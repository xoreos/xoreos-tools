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
#include "src/nwscript/analyze.h"

static const uint32 kNCSID     = MKTAG('N', 'C', 'S', ' ');
static const uint32 kVersion10 = MKTAG('V', '1', '.', '0');

namespace NWScript {

NCSFile::NCSFile(Common::SeekableReadStream &ncs) : _size(0), _multipleGlobal(false),
	_startSubRoutine(0), _globalSubRoutine(0), _mainSubRoutine(0), _hasStackAnalysis(false) {

	load(ncs);
}

NCSFile::~NCSFile() {
}

size_t NCSFile::size() const {
	return _size;
}

bool NCSFile::hasStackAnalysis() const {
	return _hasStackAnalysis;
}

const NCSFile::Instructions &NCSFile::getInstructions() const {
	return _instructions;
}

const NCSFile::Blocks &NCSFile::getBlocks() const {
	return _blocks;
}

const Block &NCSFile::getRootBlock() const {
	if (_blocks.empty())
		throw Common::Exception("This NCS file is empty!");

	return _blocks.front();
}

const NCSFile::SubRoutines &NCSFile::getSubRoutines() const {
	return _subRoutines;
}

const SubRoutine *NCSFile::getStartSubRoutine() const {
	return _startSubRoutine;
}

const SubRoutine *NCSFile::getGlobalSubRoutine() const {
	return _globalSubRoutine;
}

const SubRoutine *NCSFile::getMainSubRoutine() const {
	return _mainSubRoutine;
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

		identifySubRoutineTypes();

	} catch (Common::Exception &e) {
		e.add("Failed to load NCS file");

		throw e;
	}
}

NCSFile::Instructions::iterator NCSFile::findInstruction(uint32 address) {
	Instructions::iterator it = std::lower_bound(_instructions.begin(), _instructions.end(), address);
	if ((it == _instructions.end()) || (it->address != address))
		return _instructions.end();

	return it;
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

		if (blockInstr->opcode == kOpcodeSAVEBP) {
			/* The SAVEBP instruction is used to set a reference point for global variables.
			 * When we encounter it, we remember this current subroutine as the one that
			 * initialize this global variable frame. */

			if (_globalSubRoutine && (_globalSubRoutine != &sub)) {
				warning("Found multiple subroutines that call SAVEBP: %08X, %08X",
				        _globalSubRoutine->address, sub.address);

				_multipleGlobal = true;
			} else
				_globalSubRoutine = &sub;
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

void NCSFile::identifySubRoutineTypes() {
	/* Identify special subroutine types, like _start(), _global() and main(). */

	if (_subRoutines.empty())
		return;

	// Mark all STORESTATE subroutines as such
	for (SubRoutines::iterator s = _subRoutines.begin(); s != _subRoutines.end(); ++s) {
		if (s->blocks.empty() || s->blocks.front()->instructions.empty())
			continue;

		if (s->blocks.front()->instructions.front()->addressType == kAddressTypeStoreState)
			s->type = kSubRoutineTypeStoreState;
	}

	// The very first subroutine is the _start() one
	_startSubRoutine = &_subRoutines.front();
	_startSubRoutine->type = kSubRoutineTypeStart;
	_startSubRoutine->name = "_start";

	if (!_startSubRoutine->blocks.front()->instructions.empty()) {
		Instruction *instr = const_cast<Instruction *>(_startSubRoutine->blocks.front()->instructions.front());
		assert(instr);

		instr->addressType = kAddressTypeSubRoutine;
	}

	// If we found multiple global subroutines, there's something fishy going on. Let's ignore them
	if (_multipleGlobal)
		_globalSubRoutine = 0;

	// If we have a _global() subroutine, mark it
	if (_globalSubRoutine) {
		_globalSubRoutine->type = kSubRoutineTypeGlobal;
		_globalSubRoutine->name = "_global";
	}

	// If we have a global subroutine, it calls main(). Otherwise, _start() calls main()
	SubRoutine *mainCaller = _globalSubRoutine ? _globalSubRoutine : _startSubRoutine;

	if (mainCaller->callees.size() > 1)
		warning("More than one potential main subroutine found");

	// Assume that the last subroutine the main caller calls is the main()
	if (mainCaller->callees.size() >= 1) {
		_mainSubRoutine = const_cast<SubRoutine *>(*--mainCaller->callees.end());
		assert(_mainSubRoutine);

		if (!_startSubRoutine->blocks.empty()) {
			/* Try to find out whether this script is an event script or a dialogue
			 * conditional script.
			 *
			 * Event scripts are called by events happening on objects. They don't
			 * return a value and their main function is called "main".
			 *
			 * Dialogue conditional scripts are called to evaluate whether a branch
			 * in dialogue tree is visible to the user (on a user line), or whether
			 * it should be taken (on an NPC line). They return an int that will be
			 * interpreted as a boolean value. Their main function is called
			 * "StartingConditional".
			 *
			 * Therefore, we can differentiate them by the very first instructions
			 * in the _start() subroutine. If the _start() subroutine directly jumps
			 * into the main (or the _global()) subroutine, this is an event script.
			 * If _start() adds an integer to the stack as a placeholder for the
			 * return value and then jumps, this is a dialogue conditional script.
			 * If neither is true, something we don't know about happens there. */

			const std::vector<const Instruction *> &instr = _startSubRoutine->blocks[0]->instructions;

			if        ((instr.size() >= 1) &&
			           (instr[0]->opcode == kOpcodeJSR)) {

				_mainSubRoutine->type = kSubRoutineTypeMain;
				_mainSubRoutine->name = "main";

			} else if ((instr.size() >= 2) &&
			           (instr[0]->opcode == kOpcodeRSADD) &&
			           (instr[0]->type   == kInstTypeInt) &&
			           (instr[1]->opcode == kOpcodeJSR)) {

				_mainSubRoutine->type = kSubRoutineTypeStartCond;
				_mainSubRoutine->name = "StartingConditional";
			}
		}

		if (_mainSubRoutine->type == kSubRoutineTypeNone)
			_mainSubRoutine = 0;
	}
}

void NCSFile::analyzeStack(Aurora::GameID game) {
	if (!_mainSubRoutine)
		throw Common::Exception("Failed to identify the main subroutine");

	_variables.clear();
	_globals.clear();

	if (_globalSubRoutine)
		analyzeGlobals(*_globalSubRoutine, _variables, game, _globals);

	analyzeSubRoutineStack(*_mainSubRoutine, _variables, game, &_globals);

	_hasStackAnalysis = true;
}

} // End of namespace NWScript
