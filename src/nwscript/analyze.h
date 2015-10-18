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
 *  Utility functions analyzing low-level NWScript structure for higher-level concepts.
 */

#ifndef NWSCRIPT_ANALYZE_H
#define NWSCRIPT_ANALYZE_H

#include "src/aurora/types.h"

#include "src/nwscript/types.h"

namespace NWScript {

/** Analyze this "_global"-type subroutine.
 *
 *  Every single instruction in every single block of this subroutine will be
 *  analyzed, and its stack information updated. Subroutines are *not* recursed
 *  into.
 *
 *  At the end, the parameter globals will be updated with information on all
 *  the global variables this "_global" subroutine defines.
 */
void analyzeGlobals(SubRoutine &sub, VariableSpace &variables, Stack &globals);

/** Analyze the stack throughout this subroutine.
 *
 *  Every single instruction in every single block of this subroutine will be
 *  analyzed, and its stack information updated. Subroutines that are called
 *  will be recursed into and also updated.
 *
 *  The game the subroutine's script is from needs to be set to a valid value.
 *
 *  Subroutines that themselves recurse are not supported and will lead to
 *  an analysis failure.
 *
 *  Should the analysis fail for any reason, an exception will be thrown.
 */
void analyzeSubRoutineStack(SubRoutine &sub, VariableSpace &variables, Aurora::GameID game,
                            Stack *globals = 0);

} // End of namespace NWScript

#endif // NWSCRIPT_ANALYZE_H
