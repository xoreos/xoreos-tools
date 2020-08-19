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
 *  Higher-level control flow analysis on NWScript bytecode.
 */

#include <cassert>

#include <algorithm>

#include "src/common/util.h"
#include "src/common/error.h"

#include "src/nwscript/controlflow.h"
#include "src/nwscript/instruction.h"
#include "src/nwscript/block.h"
#include "src/nwscript/subroutine.h"

namespace NWScript {

/** Does this block have only one instruction?
 *
 *  For example, this block would qualify:
 *
 *            .
 *            |
 *            V
 *  .------------------.
 *  | JMP loc_00000042 |
 *  '------------------'
 *            |
 *            '
 */
static bool isSingularBlock(const Block &block) {
	return block.instructions.size() == 1;
}

/** Is this an independent block that consists of a single JMP?
 *
 *  A dependent block is one that has only parents that unconditionally, seamlessy jump
 *  to this block. Essentially, the block has only been divided because a third
 *  block jumps into its middle.
 *
 *  For example, the block at (1) would qualify:
 *
 *            .                        .
 *            |                        |
 *            V                        V
 *  .------------------.     .-----------------.
 *  | MOVSP -4         |     | EQI             |
 *  '------------------'     | JZ loc_00000023 |
 *            |              '-----------------'
 *   (1)      V              (true)|      |(false)
 *  .------------------.           |      |
 *  | JMP loc_00000042 |<----------'      |
 *  '------------------'                  |
 *            |                           |
 *            '                           '
 */
static bool isLoneJump(const Block *block) {
	if (!block)
		return false;

	const bool loneJump = isSingularBlock(*block) && (block->instructions[0]->opcode == kOpcodeJMP);

	bool independ = false;
	for (std::vector<const Block *>::const_iterator p = block->parents.begin();
	     p != block->parents.end(); ++p) {

		if ((*p)->hasConditionalChildren()) {
			independ = true;
			break;
		}
	}

	return loneJump && independ;
}

/** Is this a block that *doesn't* consist of a single JMP or isn't independent?
 *
 *  This is a convenience negation of isLoneJump(), to be used in combination with
 *  std::remove_if().
 */
static bool isNotLoneJump(const Block *block) {
	return !isLoneJump(block);
}

/** Is this a block that has a return instruction?
 *
 *  Any block with a RETN instruction qualifies. For example:
 *
 *            .
 *            |
 *            V
 *  .----------------.
 *  | CPDOWNSP -24 4 |
 *  | MOVSP -4       |
 *  | MOVSP -12      |
 *  | RETN           |
 *  '----------------'
 */
static bool isReturnBlock(const Block &block) {
	for (std::vector<const Instruction *>::const_iterator i = block.instructions.begin();
	     i != block.instructions.end(); i++)
		if ((*i)->opcode == kOpcodeRETN)
			return true;

	return false;
}

/** Is this a block that has a Return control type?
 *
 *  Any block with a RETN instruction that has been previously detected by
 *  detectReturn() qualifies. For example:
 *
 *            .
 *            |
 *            V
 *  .----------------.
 *  |     <RETN>     |
 *  | CPDOWNSP -24 4 |
 *  | MOVSP -4       |
 *  | MOVSP -12      |
 *  | RETN           |
 *  '----------------'
 */
static bool isReturnControl(const Block &block, bool checkChildren = false) {
	if (block.isControl(kControlTypeReturn))
		return true;

	if (checkChildren) {
		if (block.hasConditionalChildren())
			return false;

		for (std::vector<const Block *>::const_iterator c = block.children.begin();
		     c != block.children.end(); ++c)
			if ((*c)->isControl(kControlTypeReturn))
				return true;
	}

	return false;
}

/** Given a vector of pointers to blocks, return the block that has the earliest, lowest address. */
static const Block *getEarliestBlock(const std::vector<const Block *> &blocks) {
	const Block *result = 0;
	for (std::vector<const Block *>::const_iterator b = blocks.begin(); b != blocks.end(); ++b)
		if (!result || ((*b)->address < result->address))
			result = *b;

	return result;
}

/** Given a vector of pointers to blocks, return the block that has the latest, largest address. */
static const Block *getLatestBlock(const std::vector<const Block *> &blocks) {
	const Block *result = 0;
	for (std::vector<const Block *>::const_iterator b = blocks.begin(); b != blocks.end(); ++b)
		if (!result || ((*b)->address > result->address))
			result = *b;

	return result;
}

/** Recursive internal convenience function to be used by findPathMerge(). */
static void findPathMergeRec(std::vector<const Block *> &merges, std::set<uint32_t> &visited,
                             const Block &block1, const Block &block2) {

	/* We hold the earlier block and recursively descend into the children
	 * of the later block. If at any point, there is a linear path between
	 * the earlier block and the later block (or one of its children), we
	 * have found a merge point. */

	// Remember which blocks we already visited, so we don't process them twice
	visited.insert(block2.address);

	// We moved past the destination => no merge here
	if (block1.address > block2.address)
		return;

	// There's a linear path => we found a merge point
	if (hasLinearPath(block1, block2)) {
		merges.push_back(&block2);
		return;
	}

	// Continue along the children
	assert(block2.children.size() == block2.childrenTypes.size());

	for (size_t i = 0; i < block2.children.size(); i++) {
		const Block        &child = *block2.children[i];
		const BlockEdgeType type  =  block2.childrenTypes[i];

		// Don't follow subroutine calls, don't jump backwards and don't visit blocks twice
		if (!isSubRoutineCall(type) && (child.address > block2.address))
			if (visited.find(child.address) == visited.end())
				findPathMergeRec(merges, visited, block1, child);
	}
}

/** Find the block where the paths of these two blocks come back together.
 *
 *  For example, when given the two blocks at (1) and (2), findPathMerge()
 *  will find the block at (3).
 *
 *                .
 *                |
 *                V
 *      .-----------------.
 *      | EQI             |
 *      | JZ loc_00000023 |
 *      '-----------------'
 *      (true)|     |(false)
 *      .-----'     '-----.
 *      |                 |
 *      V  (1)       (2)  V
 * .----------.     .----------.
 * |          |     |          |
 * '----------'     '----------'
 *      |                 |
 *      V                 V
 * .----------.     .----------.
 * |          |     |          |
 * '----------'     '----------'
 *      |                 |
 *      V                 |
 * .----------.           |
 * |          |           |
 * '----------'           |
 *      |        .--------'
 *      V   (3)  V
 *    .------------.
 *    |            |
 *    '------------'
 *          |
 *          '
 */
static const Block *findPathMerge(const Block &block1, const Block &block2) {
	std::vector<const Block *> merges;
	std::set<uint32_t> visited;

	// Correctly order the two blocks we want to check
	if (block1.address < block2.address)
		findPathMergeRec(merges, visited, block1, block2);
	else
		findPathMergeRec(merges, visited, block2, block1);

	// We're only interested in the earliest merge point
	return getEarliestBlock(merges);
}


static void detectDoWhile(Blocks &blocks) {
	/* Find all do-while loops. A do-while loop has a tail block that
	 * only has a single JMP that jumps back to the loop head.
	 *
	 * For example:
	 *
	 *        .
	 *        |
	 *  (1)   V
	 * .-------------.
	 * |             |
	 * |             |<---------------------------.
	 * '-------------'                            |
	 *        |                                   |
	 *        V                                   |
	 * .-------------.                            |
	 * |             |                            |
	 * |             |                            |
	 * '-------------'                            |
	 *  (true)|   |(false)                        |
	 *        |   '--------------------.          |
	 *        |               (2)      V          |
	 *        |              .------------------. |
	 *        |              | JMP loc_00000042 | |
	 *  (3)   V              '------------------' |
	 * .-------------.                 |          |
	 * |             |                 '----------'
	 * |             |
	 * '-------------'
	 *        |
	 *        '
	 *
	 * Here, the block at (1) is the loop head, (2) is the loop tail and
	 * the block at (3) is the block immediately after the whole loop.
	 */

	for (Blocks::iterator head = blocks.begin(); head != blocks.end(); ++head) {
		// Find all parents of this block from later in the script that only consist of a single JMP.

		std::vector<const Block *> parents = head->getLaterParents();
		parents.erase(std::remove_if(parents.begin(), parents.end(), isNotLoneJump), parents.end());

		// Get the parent that has the highest address and make sure it's still undetermined
		Block *tail = const_cast<Block *>(getLatestBlock(parents));
		if (!tail || tail->hasMainControl())
			continue;

		Block *next = const_cast<Block *>(getNextBlock(blocks, *tail));
		if (!next)
			throw Common::Exception("Can't find a block following the do-while loop");

		// If such a parent exists, it's the tail of a do-while loop
		head->controls.push_back(ControlStructure(kControlTypeDoWhileHead, *head, *tail, *next));
		tail->controls.push_back(ControlStructure(kControlTypeDoWhileTail, *head, *tail, *next));
		next->controls.push_back(ControlStructure(kControlTypeDoWhileNext, *head, *tail, *next));
	}
}

static void detectWhile(Blocks &blocks) {
	/* Find all while loops. A while loop has a tail block that isn't a
	 * do-while loop tail, that jumps back to the loop head.
	 *
	 * For example:
	 *
	 *           .
	 *           |
	 *     (1)   V
	 *    .-------------.
	 *    |             |
	 *    |             |<-----.
	 *    '-------------'      |
	 *    (true)|   |(false)   |
	 * .--------'   |          |
	 * |            V          |
	 * |  .------------.       |
	 * |  |            |       |
	 * |  '------------'       |
	 * |            |          |
	 * |    (2)     V          |
	 * |  .------------------. |
	 * |  |                  | |
	 * |  |                  | |
	 * |  |                  | |
	 * |  | JMP loc_00000042 | |
	 * |  '------------------' |
	 * |            |          |
	 * |            '----------'
	 * |   (3)
	 * |  .-------------.
	 * '->|             |
	 *    |             |
	 *    '-------------'
	 *           |
	 *           '
	 *
	 * Here, the block at (1) is the loop head, (2) is the loop tail and
	 * the block at (3) is the block immediately after the whole loop.
	 */

	for (Blocks::iterator head = blocks.begin(); head != blocks.end(); ++head) {
		// Find all parents of this block from later in the script

		std::vector<const Block *> parents = head->getLaterParents();

		// Get the parent that has the highest address and make sure it's still undetermined
		Block *tail = const_cast<Block *>(getLatestBlock(parents));
		if (!tail || tail ->hasMainControl())
			continue;

		Block *next = const_cast<Block *>(getNextBlock(blocks, *tail));
		if (!next)
			throw Common::Exception("Can't find a block following the do-while loop");

		// If such a parent exists, it's the tail of a while loop
		head->controls.push_back(ControlStructure(kControlTypeWhileHead, *head, *tail, *next));
		tail->controls.push_back(ControlStructure(kControlTypeWhileTail, *head, *tail, *next));
		next->controls.push_back(ControlStructure(kControlTypeWhileNext, *head, *tail, *next));
	}
}

static void detectBreak(Blocks &blocks) {
	/* Find all "break;" statements. A break is created by a block that
	 * only contains a single JMP that jumps directly outside the loop.
	 *
	 * For example:
	 *
	 *           .
	 *           |
	 *     (1)   V
	 *    .-------------.
	 *    |             |
	 *    |             |<---------------------------.
	 *    '-------------'                            |
	 *           |                                   |
	 *           V                                   |
	 *    .-------------.                            |
	 *    |             |                            |
	 *    |             |                            |
	 *    '-------------'                            |
	 *     (true)|  |(false)                         |
	 *           |  '--------.                       |
	 *           |           V                       |
	 *           |    .-------------.                |
	 *           |    |             |                |
	 *           |    |             |                |
	 *           |    '-------------'                |
	 *           |           |                       |
	 *           |           V                       |
	 *           |    .-------------.                |
	 *           |    |             |                |
	 *           |    |             |                |
	 *           |    '-------------'                |
	 *           |     (true)|   |(false)            |
	 *           |           |   '--------.          |
	 *           |           |            |          |
	 *  (4)      V           |   (2)      V          |
	 * .------------------.  |  .------------------. |
	 * | JMP loc_0000007F |  |  | JMP loc_00000042 | |
	 * '------------------'  |  '------------------' |
	 *           |           |            |          |
	 *           |           |            '----------'
	 *           |     (3)   V
	 *           |    .-------------.
	 *           '--->|             |
	 *                |             |
	 *                '-------------'
	 *                       |
	 *                       '
	 *
	 * Here, the block at (1) is the loop head, (2) is the loop tail and
	 * the block at (3) is the block immediately after the whole loop.
	 * The block at (4) is then a break statement.
	 */

	for (Blocks::iterator b = blocks.begin(); b != blocks.end(); ++b) {
		// Find all undetermined blocks that consist of a single JMP
		if (b->hasMainControl() || !isLoneJump(&*b))
			continue;

		// Make sure they jump to a block that directly follows a loop
		if ((b->children.size() != 1) || !b->children[0]->isLoopNext())
			continue;

		// Get the loop blocks
		const Block *head = 0, *tail = 0, *next = 0;
		if (!b->children[0]->getLoop(head, tail, next))
			continue;

		// Mark the block as being a loop break
		b->controls.push_back(ControlStructure(kControlTypeBreak, *head, *tail, *next));
	}
}

static void detectContinue(Blocks &blocks) {
	/* Find all "continue;" statements. A continue is created by a block that
	 * only contains a single JMP that jumps directly to the tail of the loop.
	 *
	 * For example:
	 *
	 *        .
	 *        |
	 *  (1)   V
	 * .-------------.
	 * |             |
	 * |             |<---------------------------.
	 * '-------------'                            |
	 *        |                                   |
	 *        V                                   |
	 * .-------------.                            |
	 * |             |                            |
	 * |             |                            |
	 * '-------------'                            |
	 *  (true)|   |(false)                        |
	 *        |   '--------------------.          |
	 *        V                        |          |
	 * .-------------.         (4)     V          |
	 * |             |       .------------------. |
	 * |             |       | JMP loc_0000007F | |
	 * '-------------'       '------------------' |
	 *        |                        |          |
	 *        V                        |          |
	 * .-------------.                 |          |
	 * |             |                 |          |
	 * |             |                 |          |
	 * '-------------'        (2)      V          |
	 *  (true)|   |(false)   .------------------. |
	 *        |   '--------->| JMP loc_00000042 | |
	 *  (3)   V              '------------------' |
	 * .-------------.                 |          |
	 * |             |                 '----------'
	 * |             |
	 * '-------------'
	 *        |
	 *        '
	 *
	 * Here, the block at (1) is the loop head, (2) is the loop tail and
	 * the block at (3) is the block immediately after the whole loop.
	 * The block at (4) is then a continue statement.
	 */

	for (Blocks::iterator b = blocks.begin(); b != blocks.end(); ++b) {
		// Find all undetermined blocks that consist of a single JMP
		if (b->hasMainControl() || !isLoneJump(&*b))
			continue;

		// Make sure they jump to a loop tail
		if ((b->children.size() != 1) || !b->children[0]->isLoopTail())
			continue;

		// Get the loop blocks
		const Block *head = 0, *tail = 0, *next = 0;
		if (!b->children[0]->getLoop(head, tail, next))
			continue;

		// Mark the block as being a loop continue
		b->controls.push_back(ControlStructure(kControlTypeContinue, *head, *tail, *next));
	}
}

static void detectReturn(Blocks &blocks) {
	/* Find all "return;" (and "return $value;") statements. A return block is
	 * a block that contains a RETN statement, or that unconditionally jumps
	 * to a block with a RETN statement.
	 *
	 *           .                         .
	 *           |                         |
	 *           V                         V
	 *    .-------------.              .-------.
	 *    |             |              |       |
	 *    |             |              '-------'
	 *    '-------------'                  |
	 *     (true)|    |(false)             V
	 *     .-----'    |                .-------.
	 *     |          V                |       |
	 *     |      .-------.            '-------'
	 *     |      |       |                |
	 *     |      '-------'            (3) V
	 *     |          |                .-------.
	 *     |          V                | RETN  |
	 *     |      .-------.            '-------'
	 *     |      |       |
	 *     |      '-------'
	 *     |          |
	 * (1) |      (2) V
	 * .-------.  .-------.
	 * |       |  |       |
	 * '-------'  '-------'
	 *     |          |
	 *     |          V
	 *     |      .-------.
	 *     '----->| RETN  |
	 *            '-------'
	 *
	 *           .
	 *           |
	 *           V
	 *    .-------------.
	 *    |             |
	 *    |             |<---------------------------------------.
	 *    '-------------'                                        |
	 *           |                                               |
	 *           V                                               |
	 *    .-------------.                                        |
	 *    |             |                                        |
	 *    |             |                                        |
	 *    '-------------'                                        |
	 *     (true)|   |(false)                                    |
	 *       .---'   '-------.                                   |
	 *       |               |                                   |
	 *  (5)  V               V                                   |
	 * .-----------.  .-------------.                            |
	 * |           |  |             |                            |
	 * '           |  |             |                            |
	 * '-----------'  '-------------'                            |
	 *       |               |                                   |
	 *       |               V                                   |
	 *       |        .-------------.                            |
	 *       |        |             |                            |
	 *       |        |             |                            |
	 *       |        '-------------'                            |
	 *       |         (true)|   |(false)   .------------------. |
	 *       |               |   '--------->| JMP loc_00000042 | |
	 *       |               V              '------------------' |
	 *       |        .-------------.                 |          |
	 *       |        |             |                 '----------'
	 *       |        |             |
	 *       |        '-------------'
	 *       |               |
	 *       |         (4)   V
	 *       |        .-------------.
	 *       |        |             |
	 *       |        '-------------'
	 *       |               |
	 *       |               V
	 *       |            .------.
	 *       '----------->| RETN |
	 *                    '------'
	 *
	 * Here, the blocks at (1), (2), (3), (4) and (5) are all return statements.
	 */

	for (Blocks::iterator b = blocks.begin(); b != blocks.end(); ++b) {
		// Find all undetermined blocks with a RETN
		if (b->hasMainControl() || !isReturnBlock(*b))
			continue;

		// Make sure this is not the entry (and only) block in this subroutine
		if (!b->subRoutine || (b->subRoutine->address == b->address))
			continue;

		bool hasReturnParent = false;

		if (isSingularBlock(*b)) {
			/* If this is a block that has *only* a RETN, this block is
			 * probably a shared RETN used by several "return;" statements. */

			for (std::vector<const Block *>::const_iterator p = b->parents.begin();
			     p != b->parents.end(); ++p) {

				if ((*p)->hasUnconditionalChildren() && !(*p)->hasMainControl()) {
					hasReturnParent = true;
					const_cast<Block *>(*p)->controls.push_back(ControlStructure(kControlTypeReturn, *b));
				}
			}
		}

		// If we haven't marked any of this block's parents, mark this block instead
		if (!hasReturnParent)
			b->controls.push_back(ControlStructure(kControlTypeReturn, *b));
	}
}

static void detectIf(Blocks &blocks) {
	/* Detect if and if-else statements. An if starts with a yet undetermined block
	 * that contains a conditional jump (JZ or JNZ).
	 *
	 * For example:
	 *
	 *             .                          .
	 *             |                          |
	 *     (1)     V                  (4)     V
	 *    .-----------------.        .-----------------.
	 *    |                 |        |                 |
	 *    | EQI             |        | EQI             |
	 *    | JZ loc_00000023 |        | JZ loc_00000042 |
	 *    '-----------------'        '-----------------'
	 *     (true)|    |(false)        (true)|    |(false)
	 *      .----'    |                .----'    '----.
	 *      |         |                |              |
	 *  (2) V         |            (5) V          (6) V
	 * .---------.    |           .---------.    .---------.
	 * |         |    |           |         |    |         |
	 * '---------'    |           '---------'    '---------'
	 *      |         |                |              |
	 *      V         |                V              |
	 * .---------.    |           .---------.         |
	 * |         |    |           |         |         |
	 * '---------'    |           '---------'         |
	 *      |         |                |              |
	 *  (3) V         |                |   (7)        |
	 * .---------.    |                |  .--------.  |
	 * |         |<---'                '->|        |<-'
	 * '---------'                        '--------'
	 *      |                                  |
	 *      '                                  '
	 *
	 * Here, the blocks at (1) and (4) are conditional blocks, the blocks
	 * at (2) and (5) are the starting blocks of the true branch, the block
	 * at (6) is the starting block of the else branch, and the blocks at
	 * (3) and (7) are the blocks following the whole if construct.
	 */

	for (Blocks::iterator ifCond = blocks.begin(); ifCond != blocks.end(); ++ifCond) {
		// Find all undetermined blocks (but while heads are okay, too)
		if (ifCond->hasMainControl() && !ifCond->isControl(kControlTypeWhileHead))
			continue;

		// They do need to have conditionals, though
		if ((ifCond->children.size() != 2) || !ifCond->hasConditionalChildren())
			continue;

		// If there's no direct linear path between the two branches, this is an if-else
		const bool isIfElse = !hasLinearPath(*ifCond->children[0], *ifCond->children[1]);

		Block *ifTrue = 0, *ifElse = 0, *ifNext = 0;

		if (isIfElse) {
			// The two branches are the if and the else
			ifTrue = const_cast<Block *>(ifCond->children[0]);
			ifElse = const_cast<Block *>(ifCond->children[1]);

			// If we have both, try to find the block where the code flow unites again
			if (ifTrue && ifElse)
				ifNext = const_cast<Block *>(findPathMerge(*ifTrue, *ifElse));

		} else {
			// The if branch has the smaller address, and the flow continues at the larger address
			const bool firstSmaller = ifCond->children[0]->address < ifCond->children[1]->address;

			const Block *low  = firstSmaller ? ifCond->children[0] : ifCond->children[1];
			const Block *high = firstSmaller ? ifCond->children[1] : ifCond->children[0];

			ifTrue = const_cast<Block *>(low);
			ifNext = const_cast<Block *>(high);
		}

		assert(ifTrue);

		// Mark the conditional and the true branch
		ifCond->controls.push_back(ControlStructure(kControlTypeIfCond, *ifCond, *ifTrue, ifElse, ifNext));
		ifTrue->controls.push_back(ControlStructure(kControlTypeIfTrue, *ifCond, *ifTrue, ifElse, ifNext));

		// If we have an else and/or a next branch, mark them as well
		if (ifElse)
			ifElse->controls.push_back(ControlStructure(kControlTypeIfElse, *ifCond, *ifTrue, ifElse, ifNext));
		if (ifNext)
			ifNext->controls.push_back(ControlStructure(kControlTypeIfNext, *ifCond, *ifTrue, ifElse, ifNext));
	}
}


/** Collect all control structures of a certain type from all blocks. */
static std::vector<const ControlStructure *> collectControls(const Blocks &blocks, ControlType type) {
	std::vector<const ControlStructure *> controls;

	for (Blocks::const_iterator b = blocks.begin(); b != blocks.end(); ++b)
		for (std::vector<ControlStructure>::const_iterator c = b->controls.begin(); c != b->controls.end(); ++c)
			if (c->type == type)
				controls.push_back(&*c);

	return controls;
}


static void verifyBlocks(const Blocks &blocks) {
	/* Verify that all blocks that should have control structures attached do,
	 * in fact, have control structures attached. If we find one that doesn't,
	 * that's a fatal error. */

	for (Blocks::const_iterator b = blocks.begin(); b != blocks.end(); ++b) {
		if (b->hasBackEdge() && !b->isLoop())
			throw Common::Exception("Block %08X has back edges but is no loop", b->address);

		if (b->hasConditionalChildren()) {
			if (!b->isControl(kControlTypeIfCond))
				throw Common::Exception("Block %08X has conditional children but is no if", b->address);

			for (std::vector<const Block *>::const_iterator c = b->children.begin(); c != b->children.end(); ++c)
				if (!(*c)->isIfCond() && !(*c)->isControl(kControlTypeIfNext))
					throw Common::Exception("Block %08X is child of if %08X but is not an if type",
					                        (*c)->address, b->address);
		}
	}
}

static void verifyLoopBlocks(std::set<uint32_t> &visited, const Block &block,
                             const Block &head, const Block &tail, const Block &next) {

	/* Recursively verify that all blocks inside a jump control structure don't
	 * jump to random script locations. The only valid jump destinations for
	 * a block of a loop is to another block of the loop, the block directly
	 * following the loop (thus ending the loop), or a return block (thus
	 * returning from the subroutine entirely). */

	// Remember which blocks we already visited, so we don't process them twice
	visited.insert(block.address);

	if ((block.address > tail.address) || (block.address < head.address))
		return;

	for (size_t i = 0; i < block.children.size(); i++) {
		if (block.isSubRoutineChild(i))
			continue;

		const Block &child = *block.children[i];

		if ( (child.address < head.address) ||
		    ((child.address > tail.address) && (child.address != next.address))) {

			if (!isReturnControl(block) && !isReturnControl(child, true))
				throw Common::Exception("Loop block jumps outside loop: %08X, %08X, %08X: %08X => %08X",
				                        head.address, tail.address, next.address, block.address, child.address);
		}

		if (child.address > block.address)
			if (visited.find(child.address) == visited.end())
				verifyLoopBlocks(visited, child, head, tail, next);
	}
}

static void verifyLoop(const Block &head, const Block &tail, const Block &next) {
	/* Verify the loop assumption by making sure that the critical loop
	 * blocks are ordered correctly, that there is a path between them,
	 * and that all blocks within the loop jump to valid locations. */

	if ((head.address >= tail.address) || (next.address <= tail.address))
		throw Common::Exception("Loop blocks out of order: %08X, %08X, %08X",
		                        head.address, tail.address, next.address);

	if (!hasLinearPath(head, tail) || !hasLinearPath(head, next))
	   throw Common::Exception("Loop blocks have no linear path: %08X, %08X, %08X",
	                           head.address, tail.address, next.address);

	std::set<uint32_t> visited;
	verifyLoopBlocks(visited, head, head, tail, next);
}

static void verifyLoops(const std::vector<const ControlStructure *> &loops) {
	for (std::vector<const ControlStructure *>::const_iterator l = loops.begin(); l != loops.end(); ++l)
		verifyLoop(*(*l)->loopHead, *(*l)->loopTail, *(*l)->loopNext);
}

static void verifyLoops(const Blocks &blocks) {
	std::vector<const ControlStructure *> doWhileLoops = collectControls(blocks, kControlTypeDoWhileHead);
	verifyLoops(doWhileLoops);

	std::vector<const ControlStructure *> whileLoops   = collectControls(blocks, kControlTypeWhileHead);
	verifyLoops(whileLoops);
}

static void verifyIf(const Block *ifCond, const Block *ifTrue, const Block *ifElse, const Block *ifNext) {
	/* Verify the if assumption by making sure that there is a path between
	 * the critical blocks of the if condition. */

	assert(ifCond && ifTrue);

	if (ifTrue && ifNext)
		if (!hasLinearPath(*ifTrue, *ifNext))
			throw Common::Exception("If blocks true and next have no linear path: %08X, %08X, %08X",
			                        ifCond->address, ifTrue->address, ifNext->address);

	if (ifElse && ifNext)
		if (!hasLinearPath(*ifElse, *ifNext))
			throw Common::Exception("If blocks else and next have no linear path: %08X, %08X, %08X",
			                        ifCond->address, ifTrue->address, ifNext->address);
}

static void verifyIf(const Blocks &blocks) {
	std::vector<const ControlStructure *> ifs = collectControls(blocks, kControlTypeIfCond);
	for (std::vector<const ControlStructure *>::const_iterator i = ifs.begin(); i != ifs.end(); ++i)
		verifyIf((*i)->ifCond, (*i)->ifTrue, (*i)->ifElse, (*i)->ifNext);
}


static void detectControlFlow(Blocks &blocks) {
	// The order is important!
	detectDoWhile (blocks);
	detectWhile   (blocks);
	detectBreak   (blocks);
	detectContinue(blocks);
	detectReturn  (blocks);
	detectIf      (blocks);
}

static void verifyControlFlow(const Blocks &blocks) {
	verifyBlocks(blocks);
	verifyLoops (blocks);
	verifyIf    (blocks);
}


void analyzeControlFlow(Blocks &blocks) {
	/* Analyze the control flow to detect (and verify) different control structures. */

	detectControlFlow(blocks);
	verifyControlFlow(blocks);
}

} // End of namespace NWScript
