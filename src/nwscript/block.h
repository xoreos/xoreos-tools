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
 *  A block of NWScript bytecode instructions.
 */

#ifndef NWSCRIPT_BLOCK_H
#define NWSCRIPT_BLOCK_H

#include <vector>
#include <deque>

#include "src/common/types.h"

#include "src/nwscript/stack.h"

namespace NWScript {

struct Instruction;
struct Block;
struct SubRoutine;

typedef std::deque<Instruction> Instructions;

/** The types of an edge between blocks. */
enum BlockEdgeType {
	kBlockEdgeTypeUnconditional,    ///< This block follows unconditionally.
	kBlockEdgeTypeConditionalTrue,  ///< This block is a true branch of a conditional.
	kBlockEdgeTypeConditionalFalse, ///< This block is a false branch of a conditional.
	kBlockEdgeTypeSubRoutineCall,   ///< This block is a subroutine call created by JSR.
	kBlockEdgeTypeSubRoutineStore,  ///< This block is a subroutine functor created by STORESTATE.
	kBlockEdgeTypeSubRoutineTail,   ///< This block is a tail following a subroutine call or functor.
	kBlockEdgeTypeDead              ///< This edge is logically dead and will never be taken.
};

/** The type of a control structure. */
enum ControlType {
	kControlTypeNone,        ///< No control structure.
	kControlTypeDoWhileHead, ///< The head of a do-while loop.
	kControlTypeDoWhileTail, ///< The tail of a do-while loop.
	kControlTypeDoWhileNext, ///< The block directly following a do-while loop.
	kControlTypeWhileHead,   ///< The head of a while loop.
	kControlTypeWhileTail,   ///< The tail of a while loop.
	kControlTypeWhileNext,   ///< The block directly following a while loop.
	kControlTypeBreak,       ///< A loop break statement.
	kControlTypeContinue,    ///< A loop continue statement.
	kControlTypeReturn,      ///< A return statement.
	kControlTypeIfCond,      ///< The block containing the if condition.
	kControlTypeIfTrue,      ///< The block starting the true branch of an if.
	kControlTypeIfElse,      ///< The block starting the else branch of an if.
	kControlTypeIfNext       ///< The block directly following the whole if structure.
};

/** A control structure a block can be part of. */
struct ControlStructure {
	/** The type of this control structure. */
	ControlType type;

	// Loops
	const Block *loopHead; ///< The head block of a loop.
	const Block *loopTail; ///< The tail block of a loop.
	const Block *loopNext; ///< The block directly following the complete loop.

	// Return
	const Block *retn;     ///< The block that contains the RETN instruction.

	// If
	const Block *ifCond;   ///< The block containing the condition of an if.
	const Block *ifTrue;   ///< The block starting the true branch.
	const Block *ifElse;   ///< The block starting the else branch.
	const Block *ifNext;   ///< he block directly following the whole if.


	ControlStructure(ControlType t = kControlTypeNone) :
		type(t), loopHead(0), loopTail(0), loopNext(0), retn(0), ifCond(0), ifTrue(0), ifElse(0), ifNext(0) {
	}

	ControlStructure(ControlType t, const Block &blockRTN) :
		type(t), loopHead(0), loopTail(0), loopNext(0), retn(0), ifCond(0), ifTrue(0), ifElse(0), ifNext(0) {

		switch (type) {
			case kControlTypeReturn:
				retn = &blockRTN;
				break;

			default:
				break;
		}
	}

	ControlStructure(ControlType t, const Block &blockHead, const Block &blockTail, const Block &blockNext) :
		type(t), loopHead(0), loopTail(0), loopNext(0), retn(0), ifCond(0), ifTrue(0), ifElse(0), ifNext(0) {

		switch (type) {
			case kControlTypeDoWhileHead:
			case kControlTypeDoWhileTail:
			case kControlTypeDoWhileNext:
			case kControlTypeWhileHead:
			case kControlTypeWhileTail:
			case kControlTypeWhileNext:
			case kControlTypeBreak:
			case kControlTypeContinue:
				loopHead = &blockHead;
				loopTail = &blockTail;
				loopNext = &blockNext;
				break;

			default:
				break;
		}
	}

	ControlStructure(ControlType t, const Block &blockCond, const Block &blockTrue,
	                 const Block *blockElse, const Block *blockNext) :
		type(t), loopHead(0), loopTail(0), loopNext(0), retn(0), ifCond(0), ifTrue(0), ifElse(0), ifNext(0) {

		switch (type) {
			case kControlTypeIfCond:
			case kControlTypeIfTrue:
			case kControlTypeIfElse:
			case kControlTypeIfNext:
				ifCond = &blockCond;
				ifTrue = &blockTrue;
				ifElse =  blockElse;
				ifNext =  blockNext;
				break;

			default:
				break;
		}
	}
};

/** A block of NWScript instructions. */
struct Block {
	/** The address that starts this block. */
	uint32_t address;

	/** The instructions making up this block. */
	std::vector<const Instruction *> instructions;

	std::vector<const Block *> parents;  ///< The blocks leading into this block.
	std::vector<const Block *> children; ///< The blocks following this block.

	/** How this block leads into its children. */
	std::vector<BlockEdgeType> childrenTypes;

	/** The subroutine this block belongs to. */
	const SubRoutine *subRoutine;

	/** The current state of analyzing the stack of this block. */
	StackAnalyzeState stackAnalyzeState;

	/** The control structure(s) this block is part of. */
	std::vector<ControlStructure> controls;


	Block(uint32_t addr) : address(addr), subRoutine(0),
		stackAnalyzeState(kStackAnalyzeStateNone) {

	}

	/** Is this block the part of a specific control structure? */
	bool isControl(ControlType type) const {
		for (std::vector<ControlStructure>::const_iterator c = controls.begin(); c != controls.end(); ++c)
			if (c->type == type)
				return true;

		return false;
	}

	std::vector<const ControlStructure *> getControls(ControlType type) const {
		std::vector<const ControlStructure *> result;
		for (std::vector<ControlStructure>::const_iterator c = controls.begin(); c != controls.end(); ++c)
			if (c->type == type)
				result.push_back(&*c);

		return result;
	}

	const ControlStructure *getControl(ControlType type) const {
		std::vector<const ControlStructure *> result = getControls(type);

		return result.empty() ? 0 : result.front();
	}

	/** Is this block part of a do-while loop? */
	bool isDoWhile() const {
		return isControl(kControlTypeDoWhileHead) || isControl(kControlTypeDoWhileTail);
	}

	/** Is this block part of a while loop? */
	bool isWhile() const {
		return isControl(kControlTypeWhileHead)   || isControl(kControlTypeWhileTail);
	}

	/** Is this block part of loop? */
	bool isLoop() const {
		return isDoWhile() || isWhile();
	}

	/** Is this block a loop head? */
	bool isLoopHead() const {
		return isControl(kControlTypeDoWhileHead) || isControl(kControlTypeWhileHead);
	}

	/** Is this block a loop tail? */
	bool isLoopTail() const {
		return isControl(kControlTypeDoWhileTail) || isControl(kControlTypeWhileTail);
	}

	/** Is this block directly following a loop? */
	bool isLoopNext() const {
		return isControl(kControlTypeDoWhileNext) || isControl(kControlTypeWhileNext);
	}

	/** If this block is a loop or loop next control type, return all the loop blocks. */
	bool getLoop(const Block *&head, const Block *&tail, const Block *&next) const;

	/** Is this block part of an if condition? */
	bool isIfCond() const {
		return isControl(kControlTypeIfCond) || isControl(kControlTypeIfTrue) || isControl(kControlTypeIfElse);
	}

	/** Do we already have a "main" control structure for a block?
	 *
	 *  There are several control structure types that exclusively determine
	 *  the function of a block. A block can never be part of more than one
	 *  of these types.
	 */
	bool hasMainControl() const {
		return isControl(kControlTypeWhileHead) || isControl(kControlTypeDoWhileTail) ||
		       isControl(kControlTypeBreak)     || isControl(kControlTypeContinue)    ||
		       isControl(kControlTypeReturn)    || isControl(kControlTypeIfCond);
	}

	/** Does this block have child blocks that are conditionally followed? */
	bool hasConditionalChildren() const {
		for (std::vector<BlockEdgeType>::const_iterator t = childrenTypes.begin();
		     t != childrenTypes.end(); ++t)
			if (*t == kBlockEdgeTypeConditionalTrue || *t == kBlockEdgeTypeConditionalFalse)
				return true;

		return false;
	}

	/** Does this block have only children that are followed unconditionally? */
	bool hasUnconditionalChildren() const {
		if ((childrenTypes.size() == 1) && (childrenTypes[0] == kBlockEdgeTypeUnconditional))
			return true;

		if ((childrenTypes.size() == 2) &&
		    ((childrenTypes[0] == kBlockEdgeTypeDead) || (childrenTypes[1] == kBlockEdgeTypeDead)))
			return true;

		return false;
	}

	/** Is this child block jumped to by a subroutine call? */
	bool isSubRoutineChild(size_t i) const;

	/** Is this child block jumped to by a subroutine call? */
	bool isSubRoutineChild(const Block &child) const;

	/** Return all child blocks that jump backward, to an earlier, smaller address. */
	std::vector<const Block *> getEarlierChildren(bool includeSubRoutines = false) const;

	/** Return all child blocks that jump forward, to a later, larger address. */
	std::vector<const Block *> getLaterChildren(bool includeSubRoutines = false) const;

	/** Return all parent blocks that jump forward, from an earlier, smaller address. */
	std::vector<const Block *> getEarlierParents(bool includeSubRoutines = false) const;

	/** Return all parent blocks that jump backward, from a later, larger address. */
	std::vector<const Block *> getLaterParents(bool includeSubRoutines = false) const;

	/** Does this block have incoming edges from later in the script? */
	bool hasIncomingBackEdge() const {
		return !getLaterParents().empty();
	}

	/** Does this block have outgoing edges to earlier in the script? */
	bool hasOutgoingBackEdge() const {
		return !getEarlierChildren().empty();
	}

	/** Does this block have any back edges (incoming or outgoing)? */
	bool hasBackEdge() const {
		return hasIncomingBackEdge() || hasOutgoingBackEdge();
	}

	/** Does this block have incoming edges from earlier in the script? */
	bool hasIncomingForwardEdge() const {
		return !getEarlierParents().empty();
	}

	/** Does this block have outgoing edges to later in the script? */
	bool hasOutgoingForwardEdge() const {
		return !getLaterChildren().empty();
	}

	/** Does this block have any forward edges (incoming or outgoing)? */
	bool hasForwardEdge() const {
		return hasIncomingForwardEdge() || hasOutgoingForwardEdge();
	}
};

/** The whole set of blocks found in a script. */
typedef std::deque<Block> Blocks;

/** Construct a control flow graph of interconnected blocks from this complete
 *  set of script instructions.
 */
void constructBlocks(Blocks &blocks, Instructions &instructions);

/** Find the index of a block within another block's children.
 *
 *  If this child does not exist within the parent's children, return SIZE_MAX.
 */
size_t findParentChildBlock(const Block &parent, const Block &child);

/** Return the edge type that connects these two blocks. */
BlockEdgeType getParentChildEdgeType(const Block &parent, const Block &child);

/** Is this edge type a subroutine call? */
bool isSubRoutineCall(BlockEdgeType type);

/** Is there a linear path between these two blocks? */
bool hasLinearPath(const Block &block1, const Block &block2);

/** Given a complete set of script blocks, find the block directly following a block. */
const Block *getNextBlock(const Blocks &blocks, const Block &block);

/** Given a complete set of script blocks, find the block directly preceding a block. */
const Block *getPreviousBlock(const Blocks &blocks, const Block &block);

/** Given a complete set of script blocks, find edges between blocks that are logically
 *  dead and will never be taken.
 *
 *  Updates their edge type kBlockEdgeTypeDead.
 */
void findDeadBlockEdges(Blocks &blocks);

} // End of namespace NWScript

#endif // NWSCRIPT_BLOCK_H
