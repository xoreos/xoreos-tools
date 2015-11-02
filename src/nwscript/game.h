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
 *  Game-specific NWScript information.
 */

#ifndef NWSCRIPT_GAME_H
#define NWSCRIPT_GAME_H

#include "src/common/types.h"
#include "src/common/ustring.h"

#include "src/aurora/types.h"

#include "src/nwscript/variable.h"

namespace NWScript {

/** Game-specific NWScript information. */
struct GameInfo {
	static const size_t kMaxSignatureSize = 16;

	/** The number of NWScript engine types in this game. */
	size_t engineTypeCount;
	/** The names of each NWScript engine type in this game. */
	const char * const * const engineTypeNames;

	/** The number of NWScript engine functions in this game. */
	size_t functionCount;
	/** The names of each NWScript engine function in this game. */
	const char * const * const functionNames;
	/** The signature of each NWScript engine function in this game. */
	const VariableType (*functionSignatures)[kMaxSignatureSize];
};

/** Return the game-specific NWScript information for this game. */
const GameInfo *getGameInfo(Aurora::GameID game);

/** Return the number of NWScript engine types in this game. */
size_t getEngineTypeCount(Aurora::GameID game);

/** Return the generic name of this engine type. */
Common::UString getGenericEngineTypeName(size_t n);

/** Return the name of this engine type for this game. */
Common::UString getEngineTypeName(Aurora::GameID game, size_t n);

/** Return the number of NWScript engine functions in this game. */
size_t getFunctionCount(Aurora::GameID game);

/** Does this NWScript engine function exist in this game. */
bool hasFunction(Aurora::GameID game, size_t n);

/** Return the name of this NWScript engine function for this game. */
Common::UString getFunctionName(Aurora::GameID game, size_t n);

/** Return the type of variable this NWScript engine function for this game returns. */
VariableType getFunctionReturnType(Aurora::GameID game, size_t n);

/** Return the number of parameters this NWScript engine function for this game takes at most. */
size_t getFunctionParameterCount(Aurora::GameID game, size_t n);

/** Return the types of variable this NWScript engine function for this game takes as parameters */
const VariableType *getFunctionParameters(Aurora::GameID game, size_t n);

} // End of namespace NWScript

#endif // NWSCRIPT_GAME_H
