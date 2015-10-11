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

NCSFile::NCSFile(Common::SeekableReadStream &ncs) : _size(0) {
	load(ncs);
}

NCSFile::~NCSFile() {
}

size_t NCSFile::size() const {
	return _size;
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

void NCSFile::setAddressType(Instruction *instr, AddressType type) {
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
				setAddressType(&*branch, kAddressTypeStateStore);
			else
				setAddressType(&*branch, kAddressTypeJumpLabel);

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
		}
	}
}

void NCSFile::findBlocks() {
	_subRoutines.push_back(SubRoutine(_instructions.front().address));
	_blocks.push_back(Block(_instructions.front().address, _subRoutines.back()));

	_subRoutines.back().blocks.push_back(&_blocks.back());

	addBlock(&_subRoutines.back(), _blocks.back(), &_instructions.front());
}

bool NCSFile::addBranchBlock(SubRoutine *&sub, Block &block, const Instruction *branchDestination,
                             Block *&branchBlock, BlockEdgeType type) {
	bool needAdd = false;

	branchBlock = const_cast<Block *>(branchDestination->block);
	if (!branchBlock) {
		if (!sub) {
			_subRoutines.push_back(SubRoutine(branchDestination->address));
			sub = &_subRoutines.back();
		}

		_blocks.push_back(Block(branchDestination->address, *sub));

		sub->blocks.push_back(&_blocks.back());

		branchBlock = &_blocks.back();
		needAdd     = true;
	}

	branchBlock->parents.push_back(&block);
	block.children.push_back(branchBlock);

	block.childrenTypes.push_back(type);

	return needAdd;
}

void NCSFile::addBlock(SubRoutine *sub, Block &block, const Instruction *instr) {
	Block *branchBlock = 0;

	while (instr) {
		if (instr->block) {
			const_cast<Block *>(instr->block)->parents.push_back(&block);
			block.children.push_back(instr->block);

			block.childrenTypes.push_back(kBlockEdgeTypeUnconditional);

			instr = 0;
			break;
		}

		if ((instr->addressType != kAddressTypeNone) && !block.instructions.empty()) {
			if (addBranchBlock(sub, block, instr, branchBlock, kBlockEdgeTypeUnconditional))
				addBlock(sub, *branchBlock, instr);

			instr = 0;
			break;
		}

		block.instructions.push_back(instr);
		const_cast<Instruction *>(instr)->block = &block;

		if ((instr->opcode == kOpcodeJMP) || (instr->opcode == kOpcodeJSR) ||
		    (instr->opcode == kOpcodeJZ ) || (instr->opcode == kOpcodeJNZ) ||
		    (instr->opcode == kOpcodeRETN) || (instr->opcode == kOpcodeSTORESTATE))
			break;

		instr = instr->follower;
	}

	if (!instr)
		return;

	SubRoutine *newSub = 0;

	switch (instr->opcode) {
		case kOpcodeJMP:
			assert(instr->branches.size() == 1);

			if (addBranchBlock(sub, block, instr->branches[0], branchBlock, kBlockEdgeTypeUnconditional))
				addBlock(sub, *branchBlock, instr->branches[0]);

			break;

		case kOpcodeJZ:
		case kOpcodeJNZ:
			assert(instr->branches.size() == 2);

			if (addBranchBlock(sub, block, instr->branches[0], branchBlock, kBlockEdgeTypeConditionalTrue))
				addBlock(sub, *branchBlock, instr->branches[0]);
			if (addBranchBlock(sub, block, instr->branches[1], branchBlock, kBlockEdgeTypeConditionalFalse))
				addBlock(sub, *branchBlock, instr->branches[1]);

			break;

		case kOpcodeJSR:
			assert(instr->branches.size() == 1);
			assert(instr->follower);

			if (addBranchBlock(newSub, block, instr->branches[0], branchBlock, kBlockEdgeTypeFunctionCall))
				addBlock(newSub, *branchBlock, instr->branches[0]);

			if (addBranchBlock(sub, block, instr->follower, branchBlock, kBlockEdgeTypeFunctionReturn))
				addBlock(sub, *branchBlock, instr->follower);

			break;

		case kOpcodeSTORESTATE:
			assert(instr->branches.size() == 1);
			assert(instr->follower);

			if (addBranchBlock(newSub, block, instr->branches[0], branchBlock, kBlockEdgeTypeStoreState))
				addBlock(newSub, *branchBlock, instr->branches[0]);

			if (addBranchBlock(sub, block, instr->follower, branchBlock, kBlockEdgeTypeFunctionReturn))
				addBlock(sub, *branchBlock, instr->follower);

			break;

		default:
			break;
	}
}

} // End of namespace NWScript
