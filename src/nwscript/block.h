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

#ifndef NWSCRIPT_BLOCK_H
#define NWSCRIPT_BLOCK_H

#include <vector>

#include "src/common/types.h"

#include "src/nwscript/stack.h"

namespace NWScript {

struct Instruction;
struct SubRoutine;

/** The types of an edge between blocks. */
enum BlockEdgeType {
	kBlockEdgeTypeUnconditional,    ///< This block follows unconditionally.
	kBlockEdgeTypeConditionalTrue,  ///< This block is a true branch of a conditional.
	kBlockEdgeTypeConditionalFalse, ///< This block is a false branch of a conditional.
	kBlockEdgeTypeFunctionCall,     ///< This block is a function call.
	kBlockEdgeTypeFunctionReturn,   ///< This block is a function return.
	kBlockEdgeTypeStoreState,       ///< This block is a subroutine create by STORESTATE.
	kBlockEdgeTypeDead              ///< This edge is logically dead and will never be taken.
};

/** A block of NWScript instructions. */
struct Block {
	/** The address that starts this block. */
	uint32 address;

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


	Block(uint32 addr, const SubRoutine &sub) : address(addr), subRoutine(&sub),
		stackAnalyzeState(kStackAnalyzeStateNone) {

	}
};

} // End of namespace NWScript

#endif // NWSCRIPT_BLOCK_H
