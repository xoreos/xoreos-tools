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

static const uint32 kNCSID     = MKTAG('N', 'C', 'S', ' ');
static const uint32 kVersion10 = MKTAG('V', '1', '.', '0');

namespace NWScript {

static size_t findParentChildBlock(const Block &parent, const Block &child) {
	/* Find the index of a block within another block's children. */

	for (size_t i = 0; i < parent.children.size(); i++)
		if (parent.children[i] == &child)
			return i;

	return SIZE_MAX;
}

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

		linkInstructionBranches(_instructions);

		constructBlocks(_blocks, _instructions);

		findSubRoutines(_subRoutines, _blocks);
		linkCallers(_subRoutines);
		findEntryExits(_subRoutines);
		findDeadEdges();

		identifySubRoutineTypes();

	} catch (Common::Exception &e) {
		e.add("Failed to load NCS file");

		throw e;
	}
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
		const size_t found = findParentChildBlock(block, *child);
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

static void addSubRoutineBlock(SubRoutine &sub, Block &block) {
	/* Recursively add a block and all its children to this subroutine.
	 * If this block is already in a subroutine, this must be the very
	 * same subroutine. If it is, we found a loop and don't have to
	 * follow its children. If it isn't we found a block that logically
	 * belongs to more than one subroutine. We can't handle that, and
	 * so we error out. */

	if (block.subRoutine) {
		if (block.subRoutine != &sub)
			throw Common::Exception("Block %08X belongs to subroutines %08X and %08X",
			                        block.address, sub.address, block.subRoutine->address);

		return;
	}

	block.subRoutine = &sub;
	sub.blocks.push_back(&block);

	assert(block.children.size() == block.childrenTypes.size());

	for (size_t i = 0; i < block.children.size(); i++)
		if ((block.childrenTypes[i] != kBlockEdgeTypeFunctionCall) &&
		    (block.childrenTypes[i] != kBlockEdgeTypeStoreState))
			addSubRoutineBlock(sub, *const_cast<Block *>(block.children[i]));
}

static bool isNewSubRoutineBlock(const Block &block) {
	/* Is this a block that starts a new subroutine?
	 * We determine that by going through all parent blocks of this block and see
	 * if any of them lead into this block through a function call or STORESTATE
	 * edge. If so, this block starts a new subroutine. */

	if (block.parents.empty())
		return true;

	for (std::vector<const Block *>::const_iterator p = block.parents.begin(); p != block.parents.end(); ++p) {
		if (!*p)
			continue;

		const size_t childIndex = findParentChildBlock(**p, block);
		if (childIndex == SIZE_MAX)
			throw Common::Exception("Child %08X does not exist in block %08X", block.address, (*p)->address);

		const BlockEdgeType edgeType = (*p)->childrenTypes[childIndex];
		if ((edgeType == kBlockEdgeTypeFunctionCall) || (edgeType == kBlockEdgeTypeStoreState))
			return true;
	}

	return false;
}

void NCSFile::findSubRoutines(SubRoutines &subs, Blocks &blocks) {
	/* Go through all blocks and see if they logically start a new subroutine.
	 * If they do, create the subroutine and recursively add the block and its
	 * children to the subroutine. */

	for (Blocks::iterator b = blocks.begin(); b != blocks.end(); ++b) {
		if (isNewSubRoutineBlock(*b)) {
			subs.push_back(SubRoutine(b->address));
			addSubRoutineBlock(subs.back(), *b);
		}
	}
}

void NCSFile::linkCallers(SubRoutines &subs) {
	/* Link all subroutines to their callers and callees. */

	for (SubRoutines::iterator s = subs.begin(); s != subs.end(); ++s) {
		for (std::vector<const Block *>::const_iterator b = s->blocks.begin(); b != s->blocks.end(); ++b) {
			for (std::vector<const Instruction *>::const_iterator i = (*b)->instructions.begin();
			     i != (*b)->instructions.end(); ++i) {

				if (!*i || ((*i)->opcode != kOpcodeJSR) || ((*i)->branches.size() != 1) || !(*i)->branches[0])
					continue;

				const Block *callerBlock = (*i)->block;
				const Block *calleeBlock = (*i)->branches[0]->block;

				if (!callerBlock || !callerBlock->subRoutine ||
						!calleeBlock || !calleeBlock->subRoutine)
					continue;

				SubRoutine *caller = const_cast<SubRoutine *>(callerBlock->subRoutine);
				SubRoutine *callee = const_cast<SubRoutine *>(calleeBlock->subRoutine);

				caller->callees.insert(callee);
				callee->callers.insert(caller);
			}
		}
	}
}

void NCSFile::findEntryExits(SubRoutines &subs) {
	/* Find the entry point and all exit points of all subroutines. */

	for (SubRoutines::iterator s = subs.begin(); s != subs.end(); ++s) {
		if (!s->blocks.empty() && s->blocks.front() && !s->blocks.front()->instructions.empty())
			s->entry = s->blocks.front()->instructions.front();

		for (std::vector<const Block *>::const_iterator b = s->blocks.begin(); b != s->blocks.end(); ++b) {
			for (std::vector<const Instruction *>::const_iterator i = (*b)->instructions.begin();
			     i != (*b)->instructions.end(); ++i) {

				if (!*i || ((*i)->opcode != kOpcodeRETN))
					continue;

				s->exits.push_back(*i);
			}
		}
	}
}

} // End of namespace NWScript
