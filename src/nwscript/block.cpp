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
 *  A block of NWScript bytecode instructions.
 */

#include <cassert>

#include "src/common/error.h"

#include "src/nwscript/block.h"
#include "src/nwscript/instruction.h"

namespace NWScript {

static void followBranchBlock(Blocks &blocks, Block &block, const Instruction &instr);

static bool addBranchBlock(Blocks &blocks, Block &block,
                           const Instruction &branchDestination,
                           Block *&branchBlock, BlockEdgeType type) {

	/* Prepare to follow one branch of the path.
	   Returns true if this is a completely new path we haven't handled yet.
	   branchBlock will be filled with the block for the branch. */

	bool needAdd = false;

	// See if we have already handled this branch. If not, create a new block for it.
	branchBlock = const_cast<Block *>(branchDestination.block);
	if (!branchBlock) {
		blocks.push_back(Block(branchDestination.address));

		branchBlock = &blocks.back();
		needAdd     = true;
	}

	// Link the branch with its parent

	branchBlock->parents.push_back(&block);
	block.children.push_back(branchBlock);

	block.childrenTypes.push_back(type);

	return needAdd;
}

static void constructBlocks(Blocks &blocks, Block &block, const Instruction &instr) {
	/* Recursively follow the path of instructions and construct individual but linked
	 * blocks containing the path with all its branches. */

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

			Block *branchBlock = 0;

			if (addBranchBlock(blocks, block, *blockInstr, branchBlock, kBlockEdgeTypeUnconditional))
				constructBlocks(blocks, *branchBlock, *blockInstr);

			break;
		}

		// Put the instruction into the block and vice versa
		block.instructions.push_back(blockInstr);
		const_cast<Instruction *>(blockInstr)->block = &block;

		if ((blockInstr->opcode == kOpcodeJMP ) || (blockInstr->opcode == kOpcodeJSR) ||
		    (blockInstr->opcode == kOpcodeJZ  ) || (blockInstr->opcode == kOpcodeJNZ) ||
		    (blockInstr->opcode == kOpcodeRETN) || (blockInstr->opcode == kOpcodeSTORESTATE)) {

			/* If this is an instruction that influences control flow, break to evaluate the branches. */

			followBranchBlock(blocks, block, *blockInstr);
			break;
		}

		// Else, continue with the next instruction
		blockInstr = blockInstr->follower;
	}
}

static void followBranchBlock(Blocks &blocks, Block &block, const Instruction &instr) {
	/* Evaluate the branching paths of a block and follow them all. */

	Block *branchBlock = 0;

	switch (instr.opcode) {
		case kOpcodeJMP:
			// Unconditional jump: follow the one destination

			assert(instr.branches.size() == 1);

			if (addBranchBlock(blocks, block, *instr.branches[0], branchBlock, kBlockEdgeTypeUnconditional))
				constructBlocks(blocks, *branchBlock, *instr.branches[0]);

			break;

		case kOpcodeJZ:
		case kOpcodeJNZ:
			// Conditional jump: follow path destinations

			assert(instr.branches.size() == 2);

			if (addBranchBlock(blocks, block, *instr.branches[0], branchBlock, kBlockEdgeTypeConditionalTrue))
				constructBlocks(blocks, *branchBlock, *instr.branches[0]);
			if (addBranchBlock(blocks, block, *instr.branches[1], branchBlock, kBlockEdgeTypeConditionalFalse))
				constructBlocks(blocks, *branchBlock, *instr.branches[1]);

			break;

		case kOpcodeJSR:
			// Subroutine call: follow the subroutine and the tail (the code after the call)

			assert(instr.branches.size() == 1);
			assert(instr.follower);

			if (addBranchBlock(blocks, block, *instr.branches[0], branchBlock, kBlockEdgeTypeFunctionCall))
				constructBlocks(blocks, *branchBlock, *instr.branches[0]);
			if (addBranchBlock(blocks, block, *instr.follower   , branchBlock, kBlockEdgeTypeFunctionReturn))
				constructBlocks(blocks, *branchBlock, *instr.follower);

			break;

		case kOpcodeSTORESTATE:
			// STORESTATE: follow the stored subroutine and the tail (the code after the call)

			assert(instr.branches.size() == 1);
			assert(instr.follower);

			if (addBranchBlock(blocks, block, *instr.branches[0], branchBlock, kBlockEdgeTypeStoreState))
				constructBlocks(blocks, *branchBlock, *instr.branches[0]);
			if (addBranchBlock(blocks, block, *instr.follower   , branchBlock, kBlockEdgeTypeFunctionReturn))
				constructBlocks(blocks, *branchBlock, *instr.follower);

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


void constructBlocks(Blocks &blocks, Instructions &instructions) {
	/* Create the first block containing the very first instruction in this script.
	 * Then follow the complete codeflow from this instruction onwards. */

	assert(blocks.empty());
	if (instructions.empty())
		return;

	blocks.push_back(Block(instructions.front().address));
	constructBlocks(blocks, blocks.back(), instructions.front());
}

size_t findParentChildBlock(const Block &parent, const Block &child) {
	/* Find the index of a block within another block's children. */

	for (size_t i = 0; i < parent.children.size(); i++)
		if (parent.children[i] == &child)
			return i;

	return SIZE_MAX;
}

void findDeadBlockEdges(Blocks &blocks) {
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

	for (Blocks::iterator b = blocks.begin(); b != blocks.end(); ++b) {
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

} // End of namespace NWScript
