/* xoreos-tools - Tools to help with xoreos development
 *
 * xoreos-tools is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * xoreos-tools is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * xoreos-tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xoreos-tools. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file
 *  The stack in BioWare's NWScript bytecode.
 */

#include <cassert>

#include "src/common/util.h"
#include "src/common/error.h"

#include "src/nwscript/stack.h"
#include "src/nwscript/instruction.h"
#include "src/nwscript/block.h"
#include "src/nwscript/subroutine.h"
#include "src/nwscript/util.h"
#include "src/nwscript/game.h"

namespace NWScript {

static const size_t kDummyStackFrameSize = 32;

/** The current analysis mode. */
enum AnalyzeMode {
	/** Analyze the stack of the _global method, in isolation. No subroutine call will be followed. */
	kAnalyzeStackGlobal,
	/** Analyze the stack during complete normal script control flow, starting from the main. */
	kAnalyzeStackSubRoutine
};

/** The context during stack analysis. */
struct AnalyzeStackContext {
	AnalyzeMode mode;

	SubRoutine *sub;
	Block *block;
	Instruction *instruction;

	VariableSpace *variables;

	Aurora::GameID game;
	Stack *stack;

	Stack *globals;

	size_t subStack;
	bool subRETN;

	Stack returnStack;


	AnalyzeStackContext(AnalyzeMode m, SubRoutine &s, VariableSpace &vars,
	                    Aurora::GameID g = Aurora::kGameIDUnknown) :
		mode(m), sub(&s), block(0), instruction(0), variables(&vars), game(g),
		stack(0), globals(0), subStack(0), subRETN(false) {

	}

	size_t getSubStackSize() const {
		if (!stack)
			return 0;

		return MIN(stack->size(), subStack);
	};

	Variable &addVariable(VariableType type, VariableUse use = kVariableUseUnknown) {
		assert(variables);

		const size_t id = variables->size();

		variables->push_back(Variable(id, type, use));
		variables->back().creator = instruction;

		if (type != kTypeAny)
			variables->back().typeInference.push_back(TypeInference(type, instruction));

		return variables->back();
	}

	VariableType readVariable(size_t offset) {
		assert(stack && (offset < stack->size()));

		(*stack)[offset].variable->readers.push_back(instruction);

		return (*stack)[offset].variable->type;
	}

	void setVariableType(Variable &var, VariableType type) {
		if ((type == kTypeAny) || ((var.type == kTypeResource) && (type == kTypeString)))
			return;

		var.type = type;
		var.typeInference.push_back(TypeInference(type, instruction));
	}

	void setVariableType(size_t offset, VariableType type) {
		assert(stack && (offset < stack->size()));

		setVariableType(*(*stack)[offset].variable, type);
	}

	void writeVariable(size_t offset) {
		assert(stack && (offset < stack->size()));

		(*stack)[offset].variable->writers.push_back(instruction);
	}

	void writeVariable(size_t offset, VariableType type) {
		setVariableType(offset, type);
		writeVariable(offset);
	}

	Variable &pushVariable(VariableType type, VariableUse use = kVariableUseUnknown) {
		assert(stack);

		subStack++;
		stack->push_front(StackVariable(addVariable(type, use)));

		return *stack->front().variable;
	}

	Variable &popVariable(bool reading = true) {
		assert(stack && !stack->empty() && (subStack > 0));

		if (reading)
			readVariable(0);

		Variable &var = *stack->front().variable;

		subStack--;
		stack->pop_front();

		return var;
	}

	void connectSets(const Variable *v1, const Variable *v2,
	                 std::set<const Variable *> &s1, std::set<const Variable *> &s2) {

		s1.insert(v2);
		s2.insert(v1);

		s1.insert(s2.begin(), s2.end());
		s2.insert(s1.begin(), s1.end());

		s1.erase(v1);
		s2.erase(v2);
	}

	void duplicateVariable(size_t offset, VariableUse use = kVariableUseUnknown) {
		assert(stack && (offset < stack->size()));

		Variable *var1 = (*stack)[offset].variable;

		(*stack)[offset].variable->readers.push_back(instruction);

		subStack++;
		stack->push_front(StackVariable(addVariable((*stack)[offset].variable->type, use)));

		Variable *var2 = stack->front().variable;

		connectSets(var1, var2, var1->duplicates, var2->duplicates);
	}

	bool checkVariableType(size_t offset, VariableType type) {
		assert(stack && (offset < stack->size()));

		if ((type == kTypeAny) || ((*stack)[offset].variable->type == kTypeAny))
			return true;

		if ((type == kTypeResource) && ((*stack)[offset].variable->type == kTypeString))
			return true;

		return ((*stack)[offset].variable->type == type);
	}

	void sameVariableType(Variable *var1, Variable *var2) {
		if (!var1 || !var2)
			return;

		VariableType type = var1->type;
		if (type == kTypeAny)
			type = var2->type;

		if (((var1->type == kTypeResource) && (var2->type == kTypeString)) ||
		    ((var2->type == kTypeResource) && (var1->type == kTypeString)))
			type = kTypeResource;

		setVariableType(*var1, type);
		setVariableType(*var2, type);
	}

	void sameVariableType(size_t offset1, size_t offset2) {
		assert(stack && (offset1 < stack->size()) && (offset2 < stack->size()));

		sameVariableType((*stack)[offset1].variable, (*stack)[offset2].variable);
	}

	void connectSiblings(Variable &var1, Variable &var2) {
		sameVariableType(&var1, &var2);

		connectSets(&var1, &var2, var1.siblings, var2.siblings);
	}

	void modifiesVariable(const Variable &var) {
		if (!instruction)
			return;

		instruction->variables.push_back(&var);
	}

	void modifiesVariable(size_t offset) {
		assert(stack && (offset < stack->size()));

		modifiesVariable(*(*stack)[offset].variable);
	}
};

typedef void (*AnalyzeStackFunc)(AnalyzeStackContext &ctx);

static void analyzeStackPush       (AnalyzeStackContext &ctx);
static void analyzeStackPop        (AnalyzeStackContext &ctx);
static void analyzeStackJSR        (AnalyzeStackContext &ctx);
static void analyzeStackRETN       (AnalyzeStackContext &ctx);
static void analyzeStackCPTOPSP    (AnalyzeStackContext &ctx);
static void analyzeStackCPDOWNSP   (AnalyzeStackContext &ctx);
static void analyzeStackCPTOPBP    (AnalyzeStackContext &ctx);
static void analyzeStackCPDOWNBP   (AnalyzeStackContext &ctx);
static void analyzeStackACTION     (AnalyzeStackContext &ctx);
static void analyzeStackBool       (AnalyzeStackContext &ctx);
static void analyzeStackEq         (AnalyzeStackContext &ctx);
static void analyzeStackShift      (AnalyzeStackContext &ctx);
static void analyzeStackUnArithm   (AnalyzeStackContext &ctx);
static void analyzeStackBinArithm  (AnalyzeStackContext &ctx);
static void analyzeStackCond       (AnalyzeStackContext &ctx);
static void analyzeStackDestruct   (AnalyzeStackContext &ctx);
static void analyzeStackSAVEBP     (AnalyzeStackContext &ctx);
static void analyzeStackRESTOREBP  (AnalyzeStackContext &ctx);
static void analyzeStackModifySP   (AnalyzeStackContext &ctx);
static void analyzeStackModifyBP   (AnalyzeStackContext &ctx);
static void analyzeStackREADARRAY  (AnalyzeStackContext &ctx);
static void analyzeStackWRITEARRAY (AnalyzeStackContext &ctx);
static void analyzeStackGETREF     (AnalyzeStackContext &ctx);
static void analyzeStackGETREFARRAY(AnalyzeStackContext &ctx);

static const AnalyzeStackFunc kAnalyzeStackFunc[kOpcodeMAX] = {
	// 0x00
	/*               */ 0,
	/* CPDOWNSP      */ analyzeStackCPDOWNSP,
	/* RSADD         */ analyzeStackPush,
	/* CPTOPSP       */ analyzeStackCPTOPSP,
	// 0x04
	/* CONST         */ analyzeStackPush,
	/* ACTION        */ analyzeStackACTION,
	/* LOGAND        */ analyzeStackBool,
	/* LOGAR         */ analyzeStackBool,
	// 0x08
	/* INCOR         */ analyzeStackBool,
	/* EXCOR         */ analyzeStackBool,
	/* BOOLAND       */ analyzeStackBool,
	/* EQ            */ analyzeStackEq,
	// 0x0C
	/* NEQ           */ analyzeStackEq,
	/* GEQ           */ analyzeStackEq,
	/* GT            */ analyzeStackEq,
	/* LT            */ analyzeStackEq,
	// 0x10
	/* LEQ           */ analyzeStackEq,
	/* SHLEFT        */ analyzeStackShift,
	/* SHRIGHT       */ analyzeStackShift,
	/* USHRIGHT      */ analyzeStackShift,
	// 0x14
	/* ADD           */ analyzeStackBinArithm,
	/* SUB           */ analyzeStackBinArithm,
	/* MUL           */ analyzeStackBinArithm,
	/* DIV           */ analyzeStackBinArithm,
	// 0x18
	/* MOD           */ analyzeStackBinArithm,
	/* NEG           */ analyzeStackUnArithm,
	/* COMP          */ analyzeStackUnArithm,
	/* MOVSP         */ analyzeStackPop,
	// 0x1C
	/* STORESTATEALL */ 0,
	/* JMP           */ 0,
	/* JSR           */ analyzeStackJSR,
	/* JZ            */ analyzeStackCond,
	// 0x20
	/* RETN          */ analyzeStackRETN,
	/* DESTRUCT      */ analyzeStackDestruct,
	/* NOT           */ analyzeStackUnArithm,
	/* DECSP         */ analyzeStackModifySP,
	// 0x24
	/* INCSP         */ analyzeStackModifySP,
	/* JNZ           */ analyzeStackCond,
	/* CPDOWNBP      */ analyzeStackCPDOWNBP,
	/* CPTOPBP       */ analyzeStackCPTOPBP,
	// 0x28
	/* DECBP         */ analyzeStackModifyBP,
	/* INCBP         */ analyzeStackModifyBP,
	/* SAVEBP        */ analyzeStackSAVEBP,
	/* RESTOREBP     */ analyzeStackRESTOREBP,
	// 0x2C
	/* STORESTATE    */ analyzeStackJSR,
	/* NOP           */ 0,
	/*               */ 0,
	/*               */ 0,
	// 0x30
	/* WRITEARRAY    */ analyzeStackWRITEARRAY,
	/*               */ 0,
	/* READARRAY     */ analyzeStackREADARRAY,
	/*               */ 0,
	// 0x34
	/*               */ 0,
	/*               */ 0,
	/*               */ 0,
	/* GETREF        */ analyzeStackGETREF,
	// 0x38
	/*               */ 0,
	/* GETREFARRAY   */ analyzeStackGETREFARRAY,
	/*               */ 0,
	/*               */ 0,
	// 0x3C
	/*               */ 0,
	/*               */ 0,
	/*               */ 0,
	/*               */ 0,
	// 0x40
	/*               */ 0,
	/*               */ 0,
	/* SCRIPTSIZE    */ 0
};


static void fixupDuplicateTypes(VariableSpace &variables) {
	for (VariableSpace::iterator v = variables.begin(); v != variables.end(); ++v) {
		VariableType type = v->type;

		for (std::set<const Variable *>::iterator d = v->duplicates.begin(); d != v->duplicates.end(); ++d)
			if ((*d)->type != kTypeAny)
				type = (*d)->type;

		v->type = type;
		for (std::set<const Variable *>::iterator d = v->duplicates.begin(); d != v->duplicates.end(); ++d)
			const_cast<Variable *>(*d)->type = type;
	}
}


static void analyzeStackBlock      (AnalyzeStackContext &ctx);
static void analyzeStackInstruction(AnalyzeStackContext &ctx);

static void analyzeStackSubRoutine(AnalyzeStackContext &ctx, bool ignoreRecursion = false) {
	assert(ctx.sub);

	if (ctx.sub->stackAnalyzeState == kStackAnalyzeStateFinished) {
		/* If we already analyzed this subroutine previously, don't do it again.
		 *
		 * Instead, we make sure the types of the parameters and return values
		 * are congruent between each other. */

		if (ctx.getSubStackSize() < ctx.sub->params.size())
			throw Common::Exception("analyzeStackSubRoutine(): @%08X: Stack underrun", ctx.instruction->address);

		for (size_t i = 0; i < ctx.sub->params.size(); i++) {
			Variable *var1 = const_cast<Variable *>(ctx.sub->params[i]);
			Variable *var2 = const_cast<Variable *>(ctx.stack->front().variable);

			var2->use = kVariableUseParameter;

			ctx.sameVariableType(var1, var2);
			ctx.popVariable(false);
		}

		for (size_t i = 0; i < ctx.sub->returns.size(); i++) {
			Variable *var1 = const_cast<Variable *>(ctx.sub->returns[i]);
			Variable *var2 = const_cast<Variable *>((*ctx.stack)[i].variable);

			var2->use = kVariableUseReturn;

			ctx.sameVariableType(var1, var2);
		}

		return;
	}

	// Are we currently already in the process of analyzing this very same subroutine?
	if (ctx.sub->stackAnalyzeState == kStackAnalyzeStateStart) {
		/* Are we currently already in the process of analyzing this very same
		 * subroutine?  Then we've walked into a recursing subroutine. Yay.
		 *
		 * If we've been told to ignore recursion, simply return. Note that this
		 * leaves the stack in a broken state, because the recursing subroutine may
		 * take parameters it should have cleared off the stack. So we can only do
		 * that in cases where the stack afterwards doesn't mater, namely as the
		 * very last instructions of a STORESTATE subroutine.
		 *
		 * In all other cases, this is a fatal. It is impossible to analyze
		 * recursion in the way we go about things. Unless we can find a dirty
		 * trick to guess the number of parameters the recursing subroutine
		 * takes without having to analyze it to the end, we need to error out.
		 */

		if (ignoreRecursion)
			return;

		throw Common::Exception("Recursion detected in subroutine %08X", ctx.sub->address);
	}

	ctx.sub->stackAnalyzeState = kStackAnalyzeStateStart;

	if (!ctx.sub->blocks.empty()) {
		/* Start analyzing the control flow of this subroutine with its
		 * first block. The following blocks and their subroutine calls
		 * will be recursively followed. */

		assert(ctx.sub->blocks.front());

		AnalyzeStackContext oldCtx(ctx);

		ctx.block    = const_cast<Block *>(ctx.sub->blocks.front());
		ctx.subStack = 0;
		ctx.subRETN  = false;
		ctx.returnStack.clear();

		analyzeStackBlock(ctx);

		*oldCtx.stack = ctx.returnStack;

		ctx = oldCtx;

		ctx.subStack -= ctx.sub->params.size();
	}

	ctx.sub->stackAnalyzeState = kStackAnalyzeStateFinished;

	// Now make sure the types of all variables that have been duplicated are the same
	fixupDuplicateTypes(*ctx.variables);
}

static void analyzeStackBlock(AnalyzeStackContext &ctx) {
	assert(ctx.block);

	if (ctx.block->stackAnalyzeState == kStackAnalyzeStateFinished) {
		/* If we already analyzed this block previously, don't do it again.
		 * However, we're going to connect the variables on the stack now
		 * with the variables on the stack then. Different variables on
		 * the same stack space are obviously "siblings", essentially the
		 * same logical variable. */

		if (!ctx.block->instructions.empty() && ctx.block->instructions.front()) {
			const Instruction &instr = *ctx.block->instructions.front();

			// Make sure the stack is balanced between the two merging nodes
			if (ctx.subStack != instr.stack.size())
				throw Common::Exception("Unbalanced stack in block fork merge @%08X: %u != %u",
				                        instr.address, (uint)ctx.subStack, (uint)instr.stack.size());

			// Now connect the siblings, until we reached a point where the stack is identical again
			for (size_t i = 0; i < ctx.subStack; i++) {
				Variable *var1 = instr.stack[i].variable;
				Variable *var2 = (*ctx.stack)[i].variable;

				if (!var1 || !var2 || (var1 == var2) || (var1->id == var2->id))
					break;

				ctx.connectSiblings(*var1, *var2);
			}
		}

		return;
	}

	// Are we currently already in the process of analyzing this very same block?
	if (ctx.block->stackAnalyzeState == kStackAnalyzeStateStart)
		throw Common::Exception("Recursion detected in block %08X", ctx.block->address);

	ctx.block->stackAnalyzeState = kStackAnalyzeStateStart;

	for (std::vector<const Instruction *>::const_iterator i = ctx.block->instructions.begin();
	     i != ctx.block->instructions.end(); ++i) {

		/* Analyze all the instructions in this block.
		 * Subroutine calls will be followed recursively. */

		assert(*i);

		ctx.instruction = const_cast<Instruction *>(*i);

		analyzeStackInstruction(ctx);

		ctx.instruction = 0;
	}

	ctx.block->stackAnalyzeState = kStackAnalyzeStateFinished;

	assert(ctx.block->children.size() == ctx.block->childrenTypes.size());

	for (size_t i = 0; i < ctx.block->children.size(); i++) {
		/* Recurse into the child blocks, but not into subroutines or STORESTATEs.
		 * Don't follow logically dead edges either. */

		if ((ctx.block->childrenTypes[i] == kBlockEdgeTypeSubRoutineCall ) ||
		    (ctx.block->childrenTypes[i] == kBlockEdgeTypeSubRoutineStore) ||
		    (ctx.block->childrenTypes[i] == kBlockEdgeTypeDead           ))
			continue;

		assert(ctx.block->children[i]);

		AnalyzeStackContext oldCtx(ctx);
		Stack cStack(*ctx.stack);

		ctx.block = const_cast<Block *>(ctx.block->children[i]);
		ctx.stack = &cStack;

		analyzeStackBlock(ctx);
		if (ctx.subRETN)
			oldCtx.subRETN = true;

		if (!ctx.returnStack.empty())
			oldCtx.returnStack = ctx.returnStack;

		ctx = oldCtx;
	}
}

static void analyzeStackInstruction(AnalyzeStackContext &ctx) {
	ctx.instruction->stack = *ctx.stack;

	// For the instruction stack, only keep the stack frame of the current subroutine
	if (ctx.instruction->stack.size() > ctx.subStack) {
		const size_t outsideSize = ctx.instruction->stack.size() - ctx.subStack;

		ctx.instruction->stack.erase(ctx.instruction->stack.end() - outsideSize, ctx.instruction->stack.end());
	}

	// Call the specific stack analyze function for this opcode

	if (((size_t)ctx.instruction->opcode >= ARRAYSIZE(kAnalyzeStackFunc)) ||
	    !kAnalyzeStackFunc[(size_t)ctx.instruction->opcode])
		return;

	const AnalyzeStackFunc func = kAnalyzeStackFunc[(size_t)ctx.instruction->opcode];
	(*func)(ctx);
}

static void analyzeStackPush(AnalyzeStackContext &ctx) {
	/* A stack push, from a RSADD or CONST instruction. */

	VariableType type = instructionTypeToVariableType(ctx.instruction->type);

	ctx.modifiesVariable(ctx.pushVariable(type, kVariableUseLocal));
}

static void analyzeStackPop(AnalyzeStackContext &ctx) {
	/* A stack pop, from a MOVSP instruction. */

	if ((ctx.instruction->args[0] > 0) || ((ctx.instruction->args[0] % 4) != 0))
		throw Common::Exception("analyzeStackPop(): @%08X: Invalid argument %d",
		                        ctx.instruction->address, ctx.instruction->args[0]);

	size_t size = ctx.instruction->args[0] / -4;

	while (size-- > 0) {
		if (ctx.stack->empty())
			throw Common::Exception("analyzeStackPop(): @%08X: Stack underrun", ctx.instruction->address);

		if (ctx.subStack == 0) {
			/* If we see an underrun during a MOVSP instruction, this means the subroutine is
			 * clearing its parameters from the stack. So we can now connect the parameter
			 * with the caller stack element. */

			ctx.subStack++;
			ctx.sub->params.push_back(ctx.stack->front().variable);
		}

		ctx.modifiesVariable(ctx.popVariable(false));
	}
}

static void analyzeStackJSR(AnalyzeStackContext &ctx) {
	/* A JSR instruction, calling into a subroutine, or a STORESTATE instruction, which
	   creates a functor of a subroutine. */

	assert(!ctx.instruction->branches.empty() && ctx.instruction->branches[0]);
	assert(ctx.instruction->branches[0]->block && ctx.instruction->branches[0]->block->subRoutine);

	/* Treat the main subroutine as a barrier between analyzing the globals and the
	 * analysis of the normal control flow. We'll completely ignore calls into it,
	 * but we'll recurse into normal subroutines. */

	SubRoutine *sub = const_cast<SubRoutine *>(ctx.instruction->branches[0]->block->subRoutine);
	if ((sub->type == kSubRoutineTypeMain) || (sub->type == kSubRoutineTypeStartCond))
		return;

	/* The stack of a STORESTATE subroutine is thrown away as soon as the subroutine
	 * returns; it does not contribute to the stack of the "caller". This means that
	 * we can safely ignore tail recursion in STORESTATE subroutines and don't have
	 * to error out there. */

	bool isStoreStateTail = ctx.sub && ctx.instruction->follower &&
	                       (ctx.sub->type == kSubRoutineTypeStoreState) &&
	                       (ctx.instruction->opcode == kOpcodeJSR) &&
	                       (ctx.instruction->follower->opcode == kOpcodeRETN);

	AnalyzeStackContext oldCtx(ctx);

	ctx.sub = sub;

	analyzeStackSubRoutine(ctx, isStoreStateTail);

	if ((sub->params.size() + sub->returns.size()) > oldCtx.stack->size())
		throw Common::Exception("analyzeStackJSR(): @%08X: Stack underrun", ctx.instruction->address);

	for (size_t i = 0; i < (sub->params.size() + sub->returns.size()); i++)
		oldCtx.modifiesVariable(i);

	oldCtx.subStack = ctx.subStack;

	ctx = oldCtx;

	assert(ctx.stack);
}

static void analyzeStackRETN(AnalyzeStackContext &ctx) {
	/* A RETN instruction, returning from a subroutine call. */

	if (ctx.subRETN)
		return;

	if (ctx.sub->type == kSubRoutineTypeStoreState) {
		/* A STORESTATE subroutine doesn't really take parameters per se, nor does
		 * it return any values. So we clear those, and then restore the stack back
		 * to its original state. */

		ctx.sub->params.clear();
		ctx.sub->returns.clear();

		ctx.returnStack = *ctx.stack;

		if (ctx.subStack > ctx.returnStack.size())
			throw Common::Exception("analyzeStackRETN(): @%08X: Stack underrun", ctx.instruction->address);

		ctx.returnStack.erase(ctx.returnStack.begin(), ctx.returnStack.begin() + ctx.subStack);

	} else {
		/* If the subroutine accessed return values, these are in the same stack space
		 * as the parameters, and are therefore offset by the number of parameters.
		 * To correct that, we're now removing the parameters from the return list.
		 * We save the stack frame as the canonical return stack for this subroutine. */

		const size_t subParams = MIN<size_t>(ctx.sub->params.size(), ctx.sub->returns.size());

		ctx.sub->returns.erase(ctx.sub->returns.begin(), ctx.sub->returns.begin() + subParams);

		ctx.returnStack = *ctx.stack;

		// Mark the variables uses

		for (std::vector<const Variable *>::iterator p = ctx.sub->params.begin();
		     p != ctx.sub->params.end(); ++p)
			const_cast<Variable *>(*p)->use = kVariableUseParameter;

		for (std::vector<const Variable *>::iterator r = ctx.sub->returns.begin();
		     r != ctx.sub->returns.end(); ++r) {

			if (!*r)
				throw Common::Exception("analyzeStackRETN(): @%08X: Missing return variable at positon %u",
				                        ctx.instruction->address, (uint) (r - ctx.sub->returns.begin()));

			const_cast<Variable *>(*r)->use = kVariableUseReturn;
		}
	}

	ctx.subRETN = true;
}

static void analyzeStackCPTOPSP(AnalyzeStackContext &ctx) {
	/* A CPTOPSP instruction, duplicating a stack elements onto the top of the stack. */

	int32_t offset = ctx.instruction->args[0];
	int32_t size   = ctx.instruction->args[1];

	if ((size < 0) || ((size % 4) != 0) || (offset > -4) || ((offset % 4) != 0))
		throw Common::Exception("analyzeStackCPTOPSP(): @%08X: Invalid arguments %d, %d",
		                        ctx.instruction->address, offset, size);

	offset = (offset / -4) - 1;
	size  /= 4;

	if ((size_t)offset >= ctx.stack->size())
		throw Common::Exception("analyzeStackCPTOPSP(): @%08X: Stack underrun", ctx.instruction->address);

	while (size-- > 0) {
		ctx.modifiesVariable(offset);
		ctx.duplicateVariable(offset, kVariableUseLocal);
		ctx.modifiesVariable(0);
	}
}

static void analyzeStackCPDOWNSP(AnalyzeStackContext &ctx) {
	/* A CPDOWNSP instruction, copying the value of stack elements down. */

	int32_t offset = ctx.instruction->args[0];
	int32_t size   = ctx.instruction->args[1];

	if ((size < 0) || ((size % 4) != 0) || (offset > -4) || ((offset % 4) != 0))
		throw Common::Exception("analyzeStackCPDOWNSP(): @%08X: Invalid arguments %d, %d",
		                        ctx.instruction->address, offset, size);

	offset = (offset / -4) - 1;
	size  /= 4;

	if (((size_t)size > ctx.stack->size()) || ((size_t)offset >= ctx.stack->size()) || (size > offset))
		throw Common::Exception("analyzeStackCPDOWNSP(): @%08X: Stack underrun", ctx.instruction->address);

	while (size > 0) {
		const size_t pos = size - 1;

		VariableType type = ctx.readVariable(pos);

		if (type == kTypeAny)
			type = (*ctx.stack)[pos].variable->type = (*ctx.stack)[offset].variable->type;

		ctx.writeVariable(offset, type);

		ctx.modifiesVariable(pos);
		ctx.modifiesVariable(offset);

		if (!ctx.subRETN && ((size_t)offset >= ctx.subStack)) {
			/* If we see an underrun during a CPDOWNSP instruction, this means the subroutine
			 * into either the return placeholder, or the parameters, both of which have been
			 * created by the caller.
			 *
			 * We'll treat it as a return value for now, and will remove the parameters from
			 * this list after the subroutine returned.
			 *
			 * We only want to do this once for each subroutine, though, so the analysis
			 * sets a flag when it finds a RETN instruction. We then ignore all further
			 * underruns. */

			const size_t underrun = (size_t)offset - ctx.subStack + 1;

			assert(ctx.sub);
			if (ctx.sub->returns.size() < underrun)
				ctx.sub->returns.resize(underrun, 0);

			ctx.sub->returns[underrun - 1] = (*ctx.stack)[offset].variable;
		}

		offset--;
		size--;
	}
}

static void analyzeStackCPTOPBP(AnalyzeStackContext &ctx) {
	/* A CPTOPBP instruction, duplicating a global variable onto the top of the stack. */

	int32_t offset = ctx.instruction->args[0];
	int32_t size   = ctx.instruction->args[1];

	if ((size < 0) || ((size % 4) != 0) || (offset > -4) || ((offset % 4) != 0))
		throw Common::Exception("analyzeStackCPTOPBP(): @%08X: Invalid arguments %d, %d",
		                        ctx.instruction->address, offset, size);

	offset = (offset / -4) - 1;
	size  /= 4;

	if (!ctx.globals)
		throw Common::Exception("analyzeStackCPTOPBP(): @%08X: No context globals", ctx.instruction->address);

	if (((size_t)offset >= ctx.globals->size()) || (size > (offset + 1)))
		throw Common::Exception("analyzeStackCPTOPBP(): @%08X: Globals underrun", ctx.instruction->address);

	while (size-- > 0) {
		ctx.modifiesVariable(*(*ctx.globals)[offset].variable);
		(*ctx.globals)[offset].variable->readers.push_back(ctx.instruction);

		ctx.pushVariable((*ctx.globals)[offset].variable->type, kVariableUseLocal);
		ctx.modifiesVariable(0);

		offset--;
	}
}

static void analyzeStackCPDOWNBP(AnalyzeStackContext &ctx) {
	/* A CPDOWNBP instruction, copying the value of stack elements into the global variables. */

	int32_t offset = ctx.instruction->args[0];
	int32_t size   = ctx.instruction->args[1];

	if ((size < 0) || ((size % 4) != 0) || (offset > -4) || ((offset % 4) != 0))
		throw Common::Exception("analyzeStackCPDOWNBP(): @%08X: Invalid arguments %d, %d",
		                        ctx.instruction->address, offset, size);

	offset = (offset / -4) - 1;
	size  /= 4;

	if (!ctx.globals)
		throw Common::Exception("analyzeStackCPDOWNBP(): @%08X: No context globals", ctx.instruction->address);

	if (((size_t)offset >= ctx.globals->size()) || (size > (offset + 1)))
		throw Common::Exception("analyzeStackCPDOWNBP(): @%08X: Globals underrun", ctx.instruction->address);

	while (size > 0) {
		const size_t pos = size - 1;

		VariableType type = ctx.readVariable(pos);
		if (type == kTypeAny)
			type = (*ctx.stack)[pos].variable->type = (*ctx.globals)[offset].variable->type;

		(*ctx.globals)[offset].variable->writers.push_back(ctx.instruction);

		(*ctx.globals)[offset].variable->type = type;

		ctx.modifiesVariable(pos);
		ctx.modifiesVariable(*(*ctx.globals)[offset].variable);

		offset--;
		size--;
	}
}

static void analyzeStackACTION(AnalyzeStackContext &ctx) {
	/* An ACTION instruction, calling a game-specific engine function. */

	const int32_t function   = ctx.instruction->args[0];
	const int32_t paramCount = ctx.instruction->args[1];

	if ((function < 0) || (paramCount < 0))
		throw Common::Exception("analyzeStackACTION(): @%08X: Invalid arguments %d, %d",
		                        ctx.instruction->address, function, paramCount);

	if (!hasFunction(ctx.game, function))
		throw Common::Exception("analyzeStackACTION(): @%08X: Invalid function", ctx.instruction->address);

	const size_t funcParamCount = getFunctionParameterCount(ctx.game, function);
	if (funcParamCount < (size_t)paramCount)
		throw Common::Exception("analyzeStackACTION(): @%08X: Invalid number of parameters (%u < %u)",
		                        ctx.instruction->address, (uint)funcParamCount, (uint)paramCount);

	const VariableType *types = getFunctionParameters(ctx.game, function);
	for (int32_t i = 0; i < paramCount; i++) {
		const VariableType type = (types[i] == kTypeVector) ? kTypeFloat : types[i];
		size_t n = (types[i] == kTypeVector) ? 3 : 1;

		// Script State ("action") parameters are not kept on the stack
		if (type == kTypeScriptState)
			continue;

		while (n-- > 0) {
			if (ctx.stack->empty() || !ctx.subStack)
				throw Common::Exception("analyzeStackACTION(): @%08X: Stack underrun", ctx.instruction->address);

			if (!ctx.checkVariableType(0, type))
				throw Common::Exception("analyzeStackACTION(): @%08X: Parameter type mismatch",
				                        ctx.instruction->address);

			ctx.modifiesVariable(ctx.popVariable());
		}
	}

	const VariableType returnType = getFunctionReturnType(ctx.game, function);
	if (returnType == kTypeVoid)
		return;

	if (returnType == kTypeVector) {
		// A vector is really 3 separate float variables

		ctx.modifiesVariable(ctx.pushVariable(kTypeFloat, kVariableUseLocal));
		ctx.modifiesVariable(ctx.pushVariable(kTypeFloat, kVariableUseLocal));
		ctx.modifiesVariable(ctx.pushVariable(kTypeFloat, kVariableUseLocal));
		return;
	}

	ctx.modifiesVariable(ctx.pushVariable(returnType, kVariableUseLocal));
}

static void analyzeStackBool(AnalyzeStackContext &ctx) {
	/* A simple binary boolean instruction, like a LOGAND, LOGOR or BOOLAND. */

	if (ctx.getSubStackSize() < 2)
		throw Common::Exception("analyzeStackBool(): @%08X: Stack underrun", ctx.instruction->address);

	if (!ctx.checkVariableType(0, kTypeInt) || !ctx.checkVariableType(1, kTypeInt))
		throw Common::Exception("analyzeStackBool(): @%08X: Invalid types", ctx.instruction->address);

	ctx.setVariableType(0, kTypeInt);
	ctx.setVariableType(1, kTypeInt);

	for (size_t i = 0; i < 2; i++)
		ctx.modifiesVariable(ctx.popVariable());

	ctx.modifiesVariable(ctx.pushVariable(kTypeInt, kVariableUseLocal));
}

static void analyzeStackEq(AnalyzeStackContext &ctx) {
	/* An equality checking instruction, EQ or NEQ. */

	if ((ctx.instruction->argCount == 1) &&
	    ((ctx.instruction->args[0] < 0) || ((ctx.instruction->args[0] % 4) != 0)))
		throw Common::Exception("analyzeStackEq(): @%08X: Invalid argument %d",
		                        ctx.instruction->address, ctx.instruction->args[0]);

	// If we have an argument, it specifies the number of variables to compare
	const size_t size = (ctx.instruction->argCount == 1) ? (ctx.instruction->args[0] / 4) : 1;
	if (ctx.getSubStackSize() < (2 * size))
		throw Common::Exception("analyzeStackEq(): @%08X: Stack underrun", ctx.instruction->address);

	std::vector<Variable *> vars1, vars2;
	vars1.reserve(size);
	vars2.reserve(size);

	for (size_t i = 0; i < size; i++)
		vars1.push_back(&ctx.popVariable());

	for (size_t i = 0; i < size; i++)
		vars2.push_back(&ctx.popVariable());

	for (size_t i = 0; i < size; i++) {
		VariableType type = kTypeAny;
		if      (ctx.instruction->type == kInstTypeIntInt)
			type = kTypeInt;
		else if (ctx.instruction->type == kInstTypeFloatFloat)
			type = kTypeFloat;
		else if (ctx.instruction->type == kInstTypeStringString)
			type = kTypeString;
		else if (ctx.instruction->type == kInstTypeVectorVector)
			type = kTypeFloat;

		ctx.setVariableType(*vars1[i], type);
		ctx.setVariableType(*vars2[i], type);

		ctx.sameVariableType(vars1[i], vars2[i]);

		ctx.modifiesVariable(*vars1[i]);
		ctx.modifiesVariable(*vars2[i]);
	}

	ctx.modifiesVariable(ctx.pushVariable(kTypeInt, kVariableUseLocal));
}

static void analyzeStackShift(AnalyzeStackContext &ctx) {
	/* A shift instruction. SHLEFT, SHRIGHT, USHRIGHT. */

	if (ctx.getSubStackSize() < 2)
		throw Common::Exception("analyzeStackShift(): @%08X: Stack underrun", ctx.instruction->address);

	if (!ctx.checkVariableType(0, kTypeInt) || !ctx.checkVariableType(1, kTypeInt))
		throw Common::Exception("analyzeStackShift(): @%08X: Invalid types", ctx.instruction->address);

	ctx.setVariableType(0, kTypeInt);
	ctx.setVariableType(1, kTypeInt);

	for (size_t i = 0; i < 2; i++)
		ctx.modifiesVariable(ctx.popVariable());

	ctx.modifiesVariable(ctx.pushVariable(kTypeInt, kVariableUseLocal));
}

static void analyzeStackUnArithm(AnalyzeStackContext &ctx) {
	/* A simple unary arithmetic instruction. NEG, NOT and COMP. */

	if (ctx.getSubStackSize() < 1)
		throw Common::Exception("analyzeStackUnArithm(): @%08X: Stack underrun", ctx.instruction->address);

	const VariableType type = instructionTypeToVariableType(ctx.instruction->type);
	if (type == kTypeVoid)
		throw Common::Exception("analyzeStackUnArithm(): @%08X: Invalid instruction type %u",
		                        ctx.instruction->address, (uint)ctx.instruction->type);

	if (!ctx.checkVariableType(0, type))
		throw Common::Exception("analyzeStackUnArithm(): @%08X: Invalid types", ctx.instruction->address);

	ctx.setVariableType(0, type);

	ctx.modifiesVariable(ctx.popVariable());
	ctx.modifiesVariable(ctx.pushVariable(type, kVariableUseLocal));
}

static void analyzeStackBinArithm(AnalyzeStackContext &ctx) {
	/* A simple binary arithmetic instruction, like ADD or SUB. */

	if (ctx.getSubStackSize() < 2)
		throw Common::Exception("analyzeStackArithm(): @%08X: Stack underrun", ctx.instruction->address);

	const VariableType type = instructionTypeToVariableType(ctx.instruction->type);
	if (type == kTypeVoid)
		throw Common::Exception("analyzeStackArithm(): @%08X: Invalid instruction type %u",
		                        ctx.instruction->address, (uint)ctx.instruction->type);


	switch (ctx.instruction->type) {
		case kInstTypeIntInt:
		case kInstTypeFloatFloat:
		case kInstTypeStringString:
		case kInstTypeEngineType0EngineType0:
		case kInstTypeEngineType1EngineType1:
		case kInstTypeEngineType2EngineType2:
		case kInstTypeEngineType3EngineType3:
		case kInstTypeEngineType4EngineType4:
		case kInstTypeEngineType5EngineType5:
			if (!ctx.checkVariableType(0, type) || !ctx.checkVariableType(1, type))
				throw Common::Exception("analyzeStackBinArithm(): @%08X: Invalid types", ctx.instruction->address);

			for (size_t i = 0; i < 2; i++) {
				ctx.setVariableType(0, type);
				ctx.modifiesVariable(ctx.popVariable());
			}

			ctx.modifiesVariable(ctx.pushVariable(type, kVariableUseLocal));
			break;

		case kInstTypeIntFloat:
			if (!ctx.checkVariableType(0, kTypeFloat) || !ctx.checkVariableType(1, kTypeInt))
				throw Common::Exception("analyzeStackBinArithm(): @%08X: Invalid types", ctx.instruction->address);

			ctx.setVariableType(0, kTypeFloat);
			ctx.setVariableType(1, kTypeInt);

			ctx.modifiesVariable(ctx.popVariable());
			ctx.modifiesVariable(ctx.popVariable());

			ctx.modifiesVariable(ctx.pushVariable(kTypeFloat, kVariableUseLocal));
			break;

		case kInstTypeFloatInt:
			if (!ctx.checkVariableType(0, kTypeInt) || !ctx.checkVariableType(1, kTypeFloat))
				throw Common::Exception("analyzeStackBinArithm(): @%08X: Invalid types", ctx.instruction->address);

			ctx.setVariableType(0, kTypeInt);
			ctx.setVariableType(1, kTypeFloat);

			ctx.modifiesVariable(ctx.popVariable());
			ctx.modifiesVariable(ctx.popVariable());

			ctx.modifiesVariable(ctx.pushVariable(kTypeFloat, kVariableUseLocal));
			break;

			break;

		case kInstTypeVectorVector:
			if (!ctx.checkVariableType(0, kTypeFloat) || !ctx.checkVariableType(1, kTypeFloat) ||
			    !ctx.checkVariableType(2, kTypeFloat) || !ctx.checkVariableType(3, kTypeFloat) ||
			    !ctx.checkVariableType(4, kTypeFloat) || !ctx.checkVariableType(5, kTypeFloat))
				throw Common::Exception("analyzeStackBinArithm(): @%08X: Invalid types", ctx.instruction->address);

			for (size_t i = 0; i < 6; i++) {
				ctx.setVariableType(0, kTypeFloat);
				ctx.modifiesVariable(ctx.popVariable());
			}

			ctx.modifiesVariable(ctx.pushVariable(kTypeFloat, kVariableUseLocal));
			ctx.modifiesVariable(ctx.pushVariable(kTypeFloat, kVariableUseLocal));
			ctx.modifiesVariable(ctx.pushVariable(kTypeFloat, kVariableUseLocal));
			break;

		case kInstTypeVectorFloat:
		case kInstTypeFloatVector:
			if (!ctx.checkVariableType(0, kTypeFloat) || !ctx.checkVariableType(1, kTypeFloat) ||
			    !ctx.checkVariableType(2, kTypeFloat) || !ctx.checkVariableType(3, kTypeFloat))
				throw Common::Exception("analyzeStackBinArithm(): @%08X: Invalid types", ctx.instruction->address);

			for (size_t i = 0; i < 4; i++) {
				ctx.setVariableType(0, kTypeFloat);
				ctx.modifiesVariable(ctx.popVariable());
			}

			ctx.modifiesVariable(ctx.pushVariable(kTypeFloat, kVariableUseLocal));
			ctx.modifiesVariable(ctx.pushVariable(kTypeFloat, kVariableUseLocal));
			ctx.modifiesVariable(ctx.pushVariable(kTypeFloat, kVariableUseLocal));
			break;

		default:
			throw Common::Exception("analyzeStackBinArithm(): @%08X: Invalid instruction type",
			                        ctx.instruction->address);
	}
}

static void analyzeStackCond(AnalyzeStackContext &ctx) {
	/* A conditional jump. JZ or JNZ. */

	if (ctx.getSubStackSize() < 1)
		throw Common::Exception("analyzeStackJump(): @%08X: Stack underrun", ctx.instruction->address);

	if (!ctx.checkVariableType(0, kTypeInt))
		throw Common::Exception("analyzeStackCond(): @%08X: Invalid types", ctx.instruction->address);

	ctx.setVariableType(0, kTypeInt);
	ctx.modifiesVariable(ctx.popVariable());
}

static void analyzeStackDestruct(AnalyzeStackContext &ctx) {
	/* A DESTRUCT instruction, clearing elements from the stack. */

	int16_t stackSize        = ctx.instruction->args[0];
	int16_t dontRemoveOffset = ctx.instruction->args[1];
	int16_t dontRemoveSize   = ctx.instruction->args[2];

	if (((stackSize % 4) != 0) || ((dontRemoveOffset % 4) != 0) || ((dontRemoveSize % 4) != 0) ||
	     (stackSize < 0)       ||  (dontRemoveOffset < 0)       ||  (dontRemoveSize < 0))
		throw Common::Exception("analyzeStackDestruct(): @%08X: Invalid arguments %d, %d, %d",
		                        ctx.instruction->address, stackSize, dontRemoveOffset, dontRemoveSize);

	if (ctx.getSubStackSize() < (((size_t)stackSize) / 4))
		throw Common::Exception("analyzeStackDestruct(): @%08X: Stack underrun", ctx.instruction->address);

	Stack tmp;

	while (stackSize > 0) {

		if ((stackSize <= (dontRemoveOffset + dontRemoveSize)) &&
		    (stackSize >   dontRemoveOffset))
			tmp.push_back(ctx.stack->front());

		ctx.modifiesVariable(*ctx.stack->front().variable);

		assert(ctx.subStack > 0);

		ctx.subStack--;
		ctx.stack->pop_front();

		stackSize -= 4;
	}

	for (Stack::reverse_iterator t = tmp.rbegin(); t != tmp.rend(); ++t) {
		ctx.subStack++;
		ctx.stack->push_front(*t);
	}
}

static void analyzeStackSAVEBP(AnalyzeStackContext &ctx) {
	/* A SAVEBP instruction, setting the value of BP. This finalizes the global variables. */

	if (ctx.mode != kAnalyzeStackGlobal)
		throw Common::Exception("analyzeStackSAVEBP(): @%08X: Found SAVEBP outside of globals analysis",
		                        ctx.instruction->address);

	if (!ctx.globals)
		throw Common::Exception("analyzeStackSAVEBP(): @%08X: No context globals?!?", ctx.instruction->address);

	if (!ctx.globals->empty())
		throw Common::Exception("analyzeStackSAVEBP(): @%08X: Encountered multiple SAVEBP calls",
		                        ctx.instruction->address);

	/* At this point, the current stack frame contains all global variables the
	 * script will have access to in the future. */

	*ctx.globals = *ctx.stack;

	// Remove the dummy stack frame from the globals stack
	const size_t dummySize = MIN<size_t>(ctx.globals->size(), kDummyStackFrameSize);
	ctx.globals->erase(ctx.globals->end() - dummySize, ctx.globals->end());

	for (Stack::iterator g = ctx.globals->begin(); g != ctx.globals->end(); ++g)
		g->variable->use = kVariableUseGlobal;

	// SAVEBP pushes the current BP value onto the stack
	ctx.modifiesVariable(ctx.pushVariable(kTypeInt, kVariableUseLocal));
}

static void analyzeStackRESTOREBP(AnalyzeStackContext &ctx) {
	/* A RESTOREBP instruction, restoring an old value of BP. */

	if (ctx.getSubStackSize() < 1)
		throw Common::Exception("analyzeStackRESTOREBP(): @%08X: Stack underrun", ctx.instruction->address);

	ctx.modifiesVariable(ctx.popVariable());
}

static void analyzeStackModifySP(AnalyzeStackContext &ctx) {
	/* An instruction that directly modifies a stack variable. DECSP or INCSP. */

	int32_t offset = ctx.instruction->args[0];

	if ((offset > -4) || ((offset % 4) != 0))
		throw Common::Exception("analyzeStackModifySP(): @%08X: Invalid argument %d",
		                        ctx.instruction->address, offset);

	offset = (offset / -4) - 1;

	if ((size_t)offset >= ctx.stack->size())
		throw Common::Exception("analyzeStackModifySP(): @%08X: Stack underrun", ctx.instruction->address);

	if (!ctx.checkVariableType(offset, kTypeInt))
		throw Common::Exception("analyzeStackModifySP(): @%08X: Invalid types", ctx.instruction->address);

	ctx.setVariableType(offset, kTypeInt);

	ctx.readVariable(offset);
	ctx.writeVariable(offset);
	ctx.modifiesVariable(offset);
}

static void analyzeStackModifyBP(AnalyzeStackContext &ctx) {
	/* An instruction that directly modifies a global variable. DECBP or INCBP. */

	if (!ctx.globals)
		throw Common::Exception("analyzeStackModifyBP(): @%08X: No context globals", ctx.instruction->address);

	int32_t offset = ctx.instruction->args[0];

	if ((offset > -4) || ((offset % 4) != 0))
		throw Common::Exception("analyzeStackModifyBP(): @%08X: Invalid argument %d",
		                        ctx.instruction->address, offset);

	offset = (offset / -4) - 1;

	if ((size_t)offset >= ctx.globals->size())
		throw Common::Exception("analyzeStackModifyBP(): @%08X: Globals underrun", ctx.instruction->address);

	(*ctx.globals)[offset].variable->readers.push_back(ctx.instruction);
	(*ctx.globals)[offset].variable->writers.push_back(ctx.instruction);
	ctx.modifiesVariable(*(*ctx.globals)[offset].variable);
}

static void analyzeStackREADARRAY(AnalyzeStackContext &ctx) {
	/* A READARRAY instruction, read an element out of an array variable. */

	if (ctx.instruction->type != kInstTypeDirect)
		throw Common::Exception("analyzeStackREADARRAY(): @%08X: Invalid instruction type",
		                        ctx.instruction->address);

	int32_t offset = ctx.instruction->args[0];
	int32_t size   = ctx.instruction->args[1];

	if ((size != 4) || (offset > -4) || ((offset % 4) != 0))
		throw Common::Exception("analyzeStackREADARRAY(): @%08X: Invalid arguments %d, %d",
		                        ctx.instruction->address, offset, size);

	offset = (offset / -4) - 1;

	if ((offset <= 0) || ((size_t)offset >= ctx.stack->size()))
		throw Common::Exception("analyzeStackREADARRAY(): @%08X: Stack underrun", ctx.instruction->address);

	ctx.modifiesVariable(0);
	ctx.modifiesVariable(offset);

	const VariableType type = arrayTypeToType(ctx.readVariable(offset));

	ctx.setVariableType(0, kTypeInt);
	ctx.popVariable();

	ctx.modifiesVariable(ctx.pushVariable(type, kVariableUseLocal));
}

static void analyzeStackWRITEARRAY(AnalyzeStackContext &ctx) {
	/* A WRITEARRAY instruction, writing an element of an array variable. */

	if (ctx.instruction->type != kInstTypeDirect)
		throw Common::Exception("analyzeStackWRITEARRAY(): @%08X: Invalid instruction type",
		                        ctx.instruction->address);

	int32_t offset = ctx.instruction->args[0];
	int32_t size   = ctx.instruction->args[1];

	if ((size != 4) || (offset > -4) || ((offset % 4) != 0))
		throw Common::Exception("analyzeStackWRITEARRAY(): @%08X: Invalid arguments %d, %d",
		                        ctx.instruction->address, offset, size);

	offset = (offset / -4) - 1;

	if ((offset <= 1) || ((size_t)offset >= ctx.stack->size()))
		throw Common::Exception("analyzeStackREADARRAY(): @%08X: Stack underrun", ctx.instruction->address);

	ctx.setVariableType(0, kTypeInt);
	ctx.modifiesVariable(ctx.popVariable());

	offset--;

	ctx.modifiesVariable(offset);
	ctx.modifiesVariable(0);

	VariableType arrayType = (*ctx.stack)[offset].variable->type;
	VariableType elemType  = ctx.readVariable(0);

	if (!ctx.checkVariableType(0     , arrayTypeToType(arrayType)) ||
	    !ctx.checkVariableType(offset, typeToArrayType(elemType)))
		throw Common::Exception("analyzeStackWRITEARRAY(): @%08X: Types mismatch", ctx.instruction->address);

	ctx.setVariableType(0, arrayTypeToType(arrayType));

	ctx.writeVariable(offset, typeToArrayType(elemType));
}

static void analyzeStackGETREF(AnalyzeStackContext &ctx) {
	/* A GETREF instruction, pushing a reference to another variable. */

	if (ctx.instruction->type != kInstTypeDirect)
		throw Common::Exception("analyzeStackGETREF(): @%08X: Invalid instruction type",
		                        ctx.instruction->address);

	int32_t offset = ctx.instruction->args[0];
	int32_t size   = ctx.instruction->args[1];

	if ((size != 4) || (offset > -4) || ((offset % 4) != 0))
		throw Common::Exception("analyzeStackGETREF(): @%08X: Invalid arguments %d, %d",
		                        ctx.instruction->address, offset, size);

	offset = (offset / -4) - 1;

	if ((size_t)offset >= ctx.stack->size())
		throw Common::Exception("analyzeStackGETREF(): @%08X: Stack underrun", ctx.instruction->address);

	const VariableType type = typeToRefType(ctx.readVariable(offset));

	ctx.modifiesVariable(offset);
	ctx.modifiesVariable(ctx.pushVariable(type, kVariableUseLocal));
}

static void analyzeStackGETREFARRAY(AnalyzeStackContext &ctx) {
	/* A GETREFARRAY instruction, pushing a reference to an array element. */

	if (ctx.instruction->type != kInstTypeDirect)
		throw Common::Exception("analyzeStackGETREFARRAY(): @%08X: Invalid instruction type",
		                        ctx.instruction->address);

	int32_t offset = ctx.instruction->args[0];
	int32_t size   = ctx.instruction->args[1];

	if ((size != 4) || (offset > -4) || ((offset % 4) != 0))
		throw Common::Exception("analyzeStackGETREFARRAY(): @%08X: Invalid arguments %d, %d",
		                        ctx.instruction->address, offset, size);

	offset = (offset / -4) - 1;

	if ((offset <= 0) || ((size_t)offset >= ctx.stack->size()))
		throw Common::Exception("analyzeStackGETREFARRAY(): @%08X: Stack underrun", ctx.instruction->address);

	const VariableType type = typeToRefType(arrayTypeToType(ctx.readVariable(offset)));

	ctx.setVariableType(0, kTypeInt);
	ctx.modifiesVariable(ctx.popVariable());

	ctx.modifiesVariable(offset - 1);
	ctx.modifiesVariable(ctx.pushVariable(type, kVariableUseLocal));
}


void analyzeStackGlobals(SubRoutine &sub, VariableSpace &variables, Aurora::GameID game, Stack &globals) {
	AnalyzeStackContext ctx(kAnalyzeStackGlobal, sub, variables, game);

	ctx.globals = &globals;

	Stack stack;
	ctx.stack = &stack;

	for (size_t i = 0; i < kDummyStackFrameSize; i++)
		ctx.pushVariable(kTypeAny);

	analyzeStackSubRoutine(ctx);
}

void analyzeStackSubRoutine(SubRoutine &sub, VariableSpace &variables, Aurora::GameID game, Stack *globals) {
	AnalyzeStackContext ctx(kAnalyzeStackSubRoutine, sub, variables, game);

	ctx.globals = globals;

	Stack stack;
	ctx.stack = &stack;

	for (size_t i = 0; i < kDummyStackFrameSize; i++)
		ctx.pushVariable(kTypeAny);

	analyzeStackSubRoutine(ctx);
}

} // End of namespace NWScript
