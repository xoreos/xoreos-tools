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
 *  A subroutine in BioWare's NWScript.
 */

#ifndef NWSCRIPT_SUBROUTINE_H
#define NWSCRIPT_SUBROUTINE_H

#include <vector>
#include <deque>
#include <set>

#include "src/common/types.h"
#include "src/common/ustring.h"

#include "src/nwscript/stack.h"

namespace NWScript {

struct Instruction;
struct Block;

typedef std::deque<Block> Blocks;

/** The type of a subroutine. */
enum SubRoutineType {
	kSubRoutineTypeNone,       ///< A normal subroutine.
	kSubRoutineTypeStoreState, ///< A subroutine created by a STORESTATE.
	kSubRoutineTypeStart,      ///< The _start() subroutine, where execution starts.
	kSubRoutineTypeGlobal,     ///< The _global() subroutine that sets up global variables.
	kSubRoutineTypeMain,       ///< The main() subroutine.
	kSubRoutineTypeStartCond   ///< The StartingConditional() subroutine.
};

/** A subroutine of NWScript blocks. */
struct SubRoutine {
	/** The address that starts this subroutine. */
	uint32_t address;

	/** The blocks that are inside this subroutine. */
	std::vector<const Block *> blocks;

	std::set<const SubRoutine *> callers; ///< The subroutines calling this subroutine.
	std::set<const SubRoutine *> callees; ///< The subroutines this subroutine calls.

	/** The first instruction in this subroutine. */
	const Instruction *entry;
	/** The RETN instructions that leave this subroutine. */
	std::vector<const Instruction *> exits;

	/** The type of this subroutine. */
	SubRoutineType type;

	/** The name of this subroutine, if we have identified or assigned one. */
	Common::UString name;

	/** The current state of analyzing the stack of this while subroutine. */
	StackAnalyzeState stackAnalyzeState;

	/** The types of the parameters this subroutine takes. */
	std::vector<const Variable *> params;

	/** The types of the variables this subroutine returns. */
	std::vector<const Variable *> returns;


	SubRoutine(uint32_t addr) : address(addr), entry(0), type(kSubRoutineTypeNone),
		stackAnalyzeState(kStackAnalyzeStateNone) {

	}
};

/** The whole set of subroutines found in a script. */
typedef std::deque<SubRoutine> SubRoutines;

/** A set of special subroutines found in a script. */
struct SpecialSubRoutines {
	SubRoutine *startSub;  ///< The _start() subroutine.
	SubRoutine *globalSub; ///< The _global() subroutine.
	SubRoutine *mainSub;   ///< The main subroutine (main() or StartingConditional()).

	SpecialSubRoutines() : startSub(0), globalSub(0), mainSub(0) {
	}
};

/** Given a whole set of script blocks, construct a set of subroutines
 * incorporating these blocks.
 */
void constructSubRoutines(SubRoutines &subs, Blocks &blocks);

/** Given a whole set of script subroutines, link all callers with all callees. */
void linkSubRoutineCallers(SubRoutines &subs);

/** Given a whole set of script subroutines, find their entry and exist points. */
void findSubRoutineEntryAndExits(SubRoutines &subs);

/** Given a whole set of script subroutines, analyze their types.
 *
 *  Each subroutine will have its type field updated, and a set of special
 *  subroutines that have been identified will be returned.
 */
SpecialSubRoutines analyzeSubRoutineTypes(SubRoutines &subs);

} // End of namespace NWScript

#endif // NWSCRIPT_SUBROUTINE_H
