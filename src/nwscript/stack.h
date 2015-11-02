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

namespace NWScript {

struct Variable;

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

} // End of namespace NWScript

#endif // NWSCRIPT_STACK_H
