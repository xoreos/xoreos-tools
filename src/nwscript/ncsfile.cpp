/* xoreos - A reimplementation of BioWare's Aurora engine
 *
 * xoreos is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * xoreos is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * xoreos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xoreos. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file
 *  Handling BioWare's NCS, compiled NWScript bytecode.
 */

#include <cassert>

#include <algorithm>

#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/encoding.h"
#include "src/common/readstream.h"

#include "src/nwscript/ncsfile.h"
#include "src/nwscript/util.h"
#include "src/nwscript/parse.h"

static const uint32 kNCSID     = MKTAG('N', 'C', 'S', ' ');
static const uint32 kVersion10 = MKTAG('V', '1', '.', '0');

namespace NWScript {

NCSFile::NCSFile(Common::SeekableReadStream &ncs, Aurora::GameID game) :
	_game(game), _size(0), _hasStackAnalysis(false) {

	load(ncs);
}

NCSFile::~NCSFile() {
}

Aurora::GameID NCSFile::getGame() const {
	return _game;
}

size_t NCSFile::size() const {
	return _size;
}

bool NCSFile::hasStackAnalysis() const {
	return _hasStackAnalysis;
}

const Instructions &NCSFile::getInstructions() const {
	return _instructions;
}

const Blocks &NCSFile::getBlocks() const {
	return _blocks;
}

const Block &NCSFile::getRootBlock() const {
	if (_blocks.empty())
		throw Common::Exception("This NCS file is empty!");

	return _blocks.front();
}

const SubRoutines &NCSFile::getSubRoutines() const {
	return _subRoutines;
}

const SubRoutine *NCSFile::getStartSubRoutine() const {
	return _specialSubRoutines.startSub;
}

const SubRoutine *NCSFile::getGlobalSubRoutine() const {
	return _specialSubRoutines.globalSub;
}

const SubRoutine *NCSFile::getMainSubRoutine() const {
	return _specialSubRoutines.mainSub;
}

const Instruction *NCSFile::findInstruction(uint32 address) const {
	Instructions::const_iterator it = std::lower_bound(_instructions.begin(), _instructions.end(), address);
	if ((it == _instructions.end()) || (it->address != address))
		return 0;

	return &*it;
}

void NCSFile::load(Common::SeekableReadStream &ncs) {
	try {
		readHeader(ncs);

		if (_id != kNCSID)
			throw Common::Exception("Not an NCS file (%s)", Common::debugTag(_id).c_str());

		if (_version != kVersion10)
			throw Common::Exception("Unsupported NCS file version %s", Common::debugTag(_version).c_str());

		const byte sizeOpcode = ncs.readByte();
		if (sizeOpcode != kOpcodeSCRIPTSIZE)
			throw Common::Exception("Script size opcode != 0x42 (0x%02X)", sizeOpcode);

		_size = ncs.readUint32BE();
		if (_size > ncs.size())
			throw Common::Exception("Script size %u > stream size %u", (uint)_size, (uint)ncs.size());

		if (_size < ncs.size())
			warning("Script size %u < stream size %u", (uint)_size, (uint)ncs.size());

		parse(ncs);

		linkBranches();
		findBlocks();
		findDeadEdges();

		identifySubRoutineTypes();

	} catch (Common::Exception &e) {
		e.add("Failed to load NCS file");

		throw e;
	}
}

Instructions::iterator NCSFile::findInstruction(uint32 address) {
	Instructions::iterator it = std::lower_bound(_instructions.begin(), _instructions.end(), address);
	if ((it == _instructions.end()) || (it->address != address))
		return _instructions.end();

	return it;
}

const VariableSpace &NCSFile::getVariables() const {
	return _variables;
}

const Stack &NCSFile::getGlobals() const {
	return _globals;
}

void NCSFile::parse(Common::SeekableReadStream &ncs) {
	while (parseStep(ncs))
		;
}

bool NCSFile::parseStep(Common::SeekableReadStream &ncs) {
	Instruction instr;

	if (!parseInstruction(ncs, instr))
		return false;

	_instructions.push_back(instr);

	return true;
}

static void setAddressType(Instruction *instr, AddressType type) {
	/* Only update the address type of an instruction with a higher-priority one. */

	if (!instr || ((uint)instr->addressType >= (uint)type))
		return;

	instr->addressType = type;
}

void NCSFile::linkBranches() {
	/* Go through all instructions and link them according to the flow graph.
	 *
	 * In specifics, link each instruction's follower, the instruction that
	 * naturally follows if no branches are taken. Also fill in the branches
	 * array, which contains all branches an instruction can take. This
	 * directly creates an address type for each instruction: does it start
	 * a subroutine, is it a jump destination, is it a tail of a jump or none
	 * of these?
	 */

	for (Instructions::iterator i = _instructions.begin(); i != _instructions.end(); ++i) {
		// If this is an instruction that has a natural follower, link it
		if ((i->opcode != kOpcodeJMP) && (i->opcode != kOpcodeRETN)) {
			Instructions::iterator follower = i + 1;

			i->follower = (follower != _instructions.end()) ? &*follower : 0;

			if (follower != _instructions.end())
				follower->predecessors.push_back(&*i);
		}

		// Link destinations of unconditional branches
		if ((i->opcode == kOpcodeJMP) || (i->opcode == kOpcodeJSR) || (i->opcode == kOpcodeSTORESTATE)) {
			assert(((i->opcode == kOpcodeSTORESTATE) && (i->argCount == 3)) || (i->argCount == 1));

			Instructions::iterator branch = findInstruction(i->address + i->args[0]);
			if (branch == _instructions.end())
				throw Common::Exception("Can't find destination of unconditional branch");

			i->branches.push_back(&*branch);

			if      (i->opcode == kOpcodeJSR)
				setAddressType(&*branch, kAddressTypeSubRoutine);
			else if (i->opcode == kOpcodeSTORESTATE)
				setAddressType(&*branch, kAddressTypeStoreState);
			else {
				setAddressType(&*branch, kAddressTypeJumpLabel);
				branch->predecessors.push_back(&*i);
			}

			setAddressType(const_cast<Instruction *>(i->follower), kAddressTypeTail);
		}

		// Link destinations of conditional branches
		if ((i->opcode == kOpcodeJZ) || (i->opcode == kOpcodeJNZ)) {
			assert(i->argCount == 1);

			if (!i->follower)
				throw Common::Exception("Conditional branch has no false destination");

			Instructions::iterator branch = findInstruction(i->address + i->args[0]);
			if (branch == _instructions.end())
				throw Common::Exception("Can't find destination of conditional branch");

			setAddressType(&*branch, kAddressTypeJumpLabel);

			setAddressType(const_cast<Instruction *>(i->follower), kAddressTypeTail);

			i->branches.push_back(&*branch);    // True branch
			i->branches.push_back(i->follower); // False branch

			branch->predecessors.push_back(&*i);
		}
	}
}

void NCSFile::findBlocks() {
	/* Create the first subroutine and first block containing the very first
	 * instruction in this script. Then follow the complete codeflow from this
	 * instruction onwards. */

	_subRoutines.push_back(SubRoutine(_instructions.front().address));
	_blocks.push_back(Block(_instructions.front().address, _subRoutines.back()));

	_subRoutines.back().blocks.push_back(&_blocks.back());

	constructBlocks(_subRoutines.back(), _blocks.back(), _instructions.front());
}

bool NCSFile::addBranchBlock(SubRoutine *&sub, Block &block, const Instruction &branchDestination,
                             Block *&branchBlock, BlockEdgeType type) {

	/* Prepare to follow one branch of the path.
	   Returns true if this is a completely new path we haven't handled yet. */

	bool needAdd = false;

	// See if we have already handled this branch. If not, create a new block for it.
	branchBlock = const_cast<Block *>(branchDestination.block);
	if (!branchBlock) {
		if (!sub) {
			// We weren't given a subroutine this block belongs to. Create a new one.

			_subRoutines.push_back(SubRoutine(branchDestination.address));
			sub = &_subRoutines.back();
		}

		// Create a new block and link it with its subroutine

		_blocks.push_back(Block(branchDestination.address, *sub));

		sub->blocks.push_back(&_blocks.back());

		branchBlock = &_blocks.back();
		needAdd     = true;
	}

	// Link the branch with its parent

	branchBlock->parents.push_back(&block);
	block.children.push_back(branchBlock);

	block.childrenTypes.push_back(type);

	return needAdd;
}

void NCSFile::constructBlocks(SubRoutine &sub, Block &block, const Instruction &instr) {
	/* Recursively follow the path of instructions and construct individual but linked
	 * blocks containing the path with all its branches. */

	if (!sub.entry)
		sub.entry = &instr;

	const Instruction *blockInstr = &instr;
	while (blockInstr) {
		if (blockInstr->block) {
			/* If this instruction already has a block it belongs to, we
			 * link them together. We can then stop following this path. */

			const_cast<Block *>(blockInstr->block)->parents.push_back(&block);
			block.children.push_back(blockInstr->block);

			block.childrenTypes.push_back(kBlockEdgeTypeUnconditional);

			break;
		}

		if ((blockInstr->addressType != kAddressTypeNone) && !block.instructions.empty()) {
			/* If this instruction is a jump destination or starts a subroutine,
			 * we create a new block and link them together. Since we're handing
			 * off this path, we don't need to follow it ourselves anymore. */

			Block      *branchBlock = 0;
			SubRoutine *branchSub   = &sub;

			if (addBranchBlock(branchSub, block, *blockInstr, branchBlock, kBlockEdgeTypeUnconditional))
				constructBlocks(*branchSub, *branchBlock, *blockInstr);

			break;
		}

		// Put the instruction into the block and vice versa
		block.instructions.push_back(blockInstr);
		const_cast<Instruction *>(blockInstr)->block = &block;

		if ((blockInstr->opcode == kOpcodeJMP ) || (blockInstr->opcode == kOpcodeJSR) ||
		    (blockInstr->opcode == kOpcodeJZ  ) || (blockInstr->opcode == kOpcodeJNZ) ||
		    (blockInstr->opcode == kOpcodeRETN) || (blockInstr->opcode == kOpcodeSTORESTATE)) {

			/* If this is an instruction that influences control flow, break to evaluate the branches. */

			followBranchBlock(sub, block, *blockInstr);
			break;
		}

		// Else, continue with the next instruction
		blockInstr = blockInstr->follower;
	}
}

void NCSFile::followBranchBlock(SubRoutine &sub, Block &block, const Instruction &instr) {
	/* Evaluate the branching paths of a block and follow them all. */

	Block      *branchBlock = 0;
	SubRoutine *branchSub   = &sub;

	switch (instr.opcode) {
		case kOpcodeJMP:
			// Unconditional jump: follow the one destination

			assert(instr.branches.size() == 1);

			if (addBranchBlock(branchSub, block, *instr.branches[0], branchBlock, kBlockEdgeTypeUnconditional))
				constructBlocks(*branchSub, *branchBlock, *instr.branches[0]);

			break;

		case kOpcodeJZ:
		case kOpcodeJNZ:
			// Conditional jump: follow path destinations

			assert(instr.branches.size() == 2);

			if (addBranchBlock(branchSub, block, *instr.branches[0], branchBlock, kBlockEdgeTypeConditionalTrue))
				constructBlocks(*branchSub, *branchBlock, *instr.branches[0]);
			if (addBranchBlock(branchSub, block, *instr.branches[1], branchBlock, kBlockEdgeTypeConditionalFalse))
				constructBlocks(*branchSub, *branchBlock, *instr.branches[1]);

			break;

		case kOpcodeJSR:
			// Subroutine call: follow the subroutine and the tail (the code after the call)

			assert(instr.branches.size() == 1);
			assert(instr.follower);

			branchSub = 0;
			if (addBranchBlock(branchSub, block, *instr.branches[0], branchBlock, kBlockEdgeTypeFunctionCall))
				constructBlocks(*branchSub, *branchBlock, *instr.branches[0]);

			if (branchBlock && branchBlock->subRoutine) {
				// Link the caller and the callee

				sub.callees.insert(branchBlock->subRoutine);
				const_cast<SubRoutine *>(branchBlock->subRoutine)->callers.insert(&sub);
			}

			branchSub = &sub;
			if (addBranchBlock(branchSub, block, *instr.follower   , branchBlock, kBlockEdgeTypeFunctionReturn))
				constructBlocks(*branchSub, *branchBlock, *instr.follower);

			break;

		case kOpcodeSTORESTATE:
			// STORESTATE: follow the stored subroutine and the tail (the code after the call)

			assert(instr.branches.size() == 1);
			assert(instr.follower);

			branchSub = 0;
			if (addBranchBlock(branchSub, block, *instr.branches[0], branchBlock, kBlockEdgeTypeStoreState))
				constructBlocks(*branchSub, *branchBlock, *instr.branches[0]);

			branchSub = &sub;
			if (addBranchBlock(branchSub, block, *instr.follower   , branchBlock, kBlockEdgeTypeFunctionReturn))
				constructBlocks(*branchSub, *branchBlock, *instr.follower);

			break;

		case kOpcodeRETN:
			sub.exists.push_back(&instr);
			break;

		default:
			break;
	}
}

static bool isStackDoubler(const Instruction &instr) {
	/* Is this an instruction that doubles the element on top of the stack? */

	return (instr.opcode == kOpcodeCPTOPSP) && (instr.argCount == 2) &&
	       (instr.args[0] == -4) && (instr.args[1] == 4);
}

static bool isTopStackJumper(const Block &block, const Block *child = 0, size_t *childIndex = 0) {
	/* Does this block, as its last two instructions, double the top element of
	 * the stack and then jump accordingly? If we've been given a child block,
	 * return the index of this child block within the first block's children. */

	if ((block.instructions.size() < 2) || (block.children.size() != 2))
		return false;

	const Instruction &last       = *block.instructions[block.instructions.size() - 1];
	const Instruction &secondLast = *block.instructions[block.instructions.size() - 2];

	if (!isStackDoubler(secondLast) || last.opcode != kOpcodeJZ)
		return false;

	if (child) {
		size_t found = SIZE_MAX;
		for (size_t i = 0; i < block.children.size(); i++) {
			if (block.children[i] == child) {
				found = i;
				break;
			}
		}

		if (found == SIZE_MAX)
			return false;

		if (childIndex) {
			if ((*childIndex != SIZE_MAX) && (*childIndex != found))
				return false;

			*childIndex = found;
		}
	}

	return true;
}

void NCSFile::findDeadEdges() {
	/* Run through all blocks and find edges that are logically dead and will
	 * never be taken.
	 *
	 * Currently, this is limited to one special case that occurs in scripts
	 * compiled by the original BioWare NWScript compiler (at least in NWN and
	 * KotOR): short-circuiting in if (x || y) conditionals. The original BioWare
	 * compiler has a bug where it generates a JZ instead of a JMP, creating a
	 * true branch that will never be taken and effectively disabling short-
	 * circuiting. I.e. both x and y will always be evaluated; when x is true,
	 * y will still be evaluated afterwards.
	 *
	 * We use very simple pattern-matching here. This is enough to find most
	 * occurances of this case, but not all. */

	for (Blocks::iterator b = _blocks.begin(); b != _blocks.end(); ++b) {
		if (!isTopStackJumper(*b) || (b->instructions.size() != 2) || b->parents.empty())
			continue;

		/* Look through all parents of this block and make sure they fit the
		 * pattern as well. They also all need to jump to this block with the
		 * same branch edge (true or false). */
		size_t parentEdge = SIZE_MAX;
		for (std::vector<const Block *>::const_iterator p = b->parents.begin(); p != b->parents.end(); ++p) {
			if (!isTopStackJumper(**p, &*b, &parentEdge)) {
				parentEdge = SIZE_MAX;
				break;
			}
		}
		if (parentEdge == SIZE_MAX)
			continue;

		assert(parentEdge < 2);

		/* We have now established that
		 * 1) This block checks whether the top of the stack is == 0
		 * 2) All parent blocks check whether the top of the stack is == 0
		 * 3) All parent blocks jump with the same branch edge into this block
		 *
		 * Therefore, this block must also always follow the exact same edge.
		 * This means the other edge is logically dead. */

		b->childrenTypes[1 - parentEdge] = kBlockEdgeTypeDead;
	}
}

void NCSFile::identifySubRoutineTypes() {
	try {
		_specialSubRoutines = analyzeSubRoutineTypes(_subRoutines);
	} catch (...) {
		Common::exceptionDispatcherWarnAndIgnore();
	}
}

void NCSFile::analyzeStack() {
	if ((_game == Aurora::kGameIDUnknown) || _hasStackAnalysis)
		return;

	if (!_specialSubRoutines.mainSub)
		throw Common::Exception("Failed to identify the main subroutine");

	_variables.clear();
	_globals.clear();

	if (_specialSubRoutines.globalSub)
		analyzeStackGlobals(*_specialSubRoutines.globalSub, _variables, _game, _globals);

	analyzeStackSubRoutine(*_specialSubRoutines.mainSub, _variables, _game, &_globals);

	_hasStackAnalysis = true;
}

} // End of namespace NWScript
