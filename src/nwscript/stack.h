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
 *  The stack in BioWare's NWScript bytecode.
 */

#ifndef NWSCRIPT_STACK_H
#define NWSCRIPT_STACK_H

#include <deque>

#include "src/aurora/types.h"

#include "src/nwscript/variable.h"

namespace NWScript {

struct SubRoutine;

/** The current state of analyzing the stack of a script. */
enum StackAnalyzeState {
	kStackAnalyzeStateNone,    ///< No stack analysis was performed.
	kStackAnalyzeStateStart,   ///< Stack analysis started.
	kStackAnalyzeStateFinished ///< Stack analysis completed.
};

/** A variable on the NWScript stack. */
struct StackVariable {
	Variable *variable; ///< The actual variable this stack elements refers to.


	StackVariable(Variable &var) : variable(&var) {
	}
};

/** A stack frame in a script. */
typedef std::deque<StackVariable> Stack;

/** Analyze the stack of this "_global"-type subroutine.
 *
 *  Every single instruction in every single block of this subroutine will be
 *  analyzed, and its stack information updated. Subroutines are *not* recursed
 *  into.
 *
 *  At the end, the parameter globals will be updated with information on all
 *  the global variables this "_global" subroutine defines, and the parameter
 *  variables will contain unique Variable objects for each variable created
 *  during the subroutine.
 */
void analyzeStackGlobals(SubRoutine &sub, VariableSpace &variables, Aurora::GameID game, Stack &globals);

/** Analyze the stack throughout this subroutine.
 *
 *  Every single instruction in every single block of this subroutine will be
 *  analyzed, and its stack information updated. Subroutines that are called
 *  will be recursed into and also updated. Each unique variable created
 *  during this process will have a Variable object added to the variables
 *  parameter.
 *
 *  The game the subroutine's script is from needs to be set to a valid value.
 *
 *  Subroutines that themselves recurse are not supported and will lead to
 *  an analysis failure.
 *
 *  Should the analysis fail for any reason, an exception will be thrown.
 */
void analyzeStackSubRoutine(SubRoutine &sub, VariableSpace &variables, Aurora::GameID game,
                            Stack *globals = 0);

} // End of namespace NWScript

#endif // NWSCRIPT_STACK_H
