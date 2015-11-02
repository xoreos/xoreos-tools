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
 *  A subroutine in BioWare's NWScript.
 */

#include <cassert>

#include "src/common/error.cpp"

#include "src/nwscript/subroutine.h"
#include "src/nwscript/block.h"
#include "src/nwscript/instruction.h"

namespace NWScript {

SpecialSubRoutines analyzeSubRoutineTypes(SubRoutines &subs) {
	SpecialSubRoutines special;

	if (subs.empty())
		return special;

	// Mark all STORESTATE subroutines as such

	for (SubRoutines::iterator s = subs.begin(); s != subs.end(); ++s) {
		if (s->blocks.empty() || s->blocks.front()->instructions.empty())
			continue;

		if (s->blocks.front()->instructions.front()->addressType == kAddressTypeStoreState)
			s->type = kSubRoutineTypeStoreState;
	}

	// The very first subroutine should be _start(), and it should have no callers

	special.startSub = &subs.front();
	special.startSub->type = kSubRoutineTypeStart;
	special.startSub->name = "_start";

	if (!special.startSub->callers.empty())
		throw Common::Exception("The _start() subroutine has a caller");

	if (!special.startSub->blocks.front()->instructions.empty()) {
		/* For consistency's sake, mark the first instruction of _start() as
		 * one that starts subroutine. */

		Instruction *instr = const_cast<Instruction *>(special.startSub->blocks.front()->instructions.front());
		assert(instr);

		instr->addressType = kAddressTypeSubRoutine;
	}

	// Look for the SAVEBP instruction to identify the _global() subroutine

	std::set<SubRoutine *> globals;
	for (SubRoutines::iterator s = subs.begin(); s != subs.end(); ++s)
		for (std::vector<const Block *>::const_iterator b = s->blocks.begin(); b != s->blocks.end(); ++b)
			for (std::vector<const Instruction *>::const_iterator i = (*b)->instructions.begin();
			     i != (*b)->instructions.end(); ++i)
				if ((*i)->opcode == kOpcodeSAVEBP)
					globals.insert(&*s);

	if (globals.size() > 1)
		throw Common::Exception("Found multiple _global() subroutines");

	if (!globals.empty()) {
		special.globalSub = *globals.begin();

		special.globalSub->type = kSubRoutineTypeGlobal;
		special.globalSub->name = "_global";
	}

	// If we have a global subroutine, it calls main(). Otherwise, _start() calls main()
	SubRoutine *mainCaller = special.globalSub ? special.globalSub : special.startSub;

	// Assume that the last subroutine the main caller calls is the main()
	if (mainCaller->callees.size() >= 1) {
		special.mainSub = const_cast<SubRoutine *>(*--mainCaller->callees.end());
		assert(special.mainSub);

		if (!special.startSub->blocks.empty()) {
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

			const std::vector<const Instruction *> &instr = special.startSub->blocks[0]->instructions;

			if        ((instr.size() >= 1) &&
			           (instr[0]->opcode == kOpcodeJSR)) {

				special.mainSub->type = kSubRoutineTypeMain;
				special.mainSub->name = "main";

			} else if ((instr.size() >= 2) &&
			           (instr[0]->opcode == kOpcodeRSADD) &&
			           (instr[0]->type   == kInstTypeInt) &&
			           (instr[1]->opcode == kOpcodeJSR)) {

				special.mainSub->type = kSubRoutineTypeStartCond;
				special.mainSub->name = "StartingConditional";
			}
		}

		if (special.mainSub->type == kSubRoutineTypeNone)
			special.mainSub = 0;
	}

	return special;
}

} // End of namespace NWScript
