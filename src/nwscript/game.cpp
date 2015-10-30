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

#include "src/common/util.h"
#include "src/common/strutil.h"

#include "src/nwscript/game.h"
#include "src/nwscript/game_nwn.h"
#include "src/nwscript/game_nwn2.h"
#include "src/nwscript/game_kotor.h"
#include "src/nwscript/game_kotor2.h"
#include "src/nwscript/game_jade.h"
#include "src/nwscript/game_witcher.h"
#include "src/nwscript/game_dragonage.h"
#include "src/nwscript/game_dragonage2.h"

namespace NWScript {

static const GameInfo kGameInfo[Aurora::kGameIDMAX] = {
	{
		ARRAYSIZE(NWN::kEngineTypeNames),
		NWN::kEngineTypeNames,
		ARRAYSIZE(NWN::kFunctionNames),
		NWN::kFunctionNames,
		NWN::kFunctionSignatures
	},
	{
		ARRAYSIZE(NWN2::kEngineTypeNames),
		NWN2::kEngineTypeNames,
		ARRAYSIZE(NWN2::kFunctionNames),
		NWN2::kFunctionNames,
		NWN2::kFunctionSignatures
	},
	{
		ARRAYSIZE(KotOR::kEngineTypeNames),
		KotOR::kEngineTypeNames,
		ARRAYSIZE(KotOR::kFunctionNames),
		KotOR::kFunctionNames,
		KotOR::kFunctionSignatures
	},
	{
		ARRAYSIZE(KotOR2::kEngineTypeNames),
		KotOR2::kEngineTypeNames,
		ARRAYSIZE(KotOR2::kFunctionNames),
		KotOR2::kFunctionNames,
		KotOR2::kFunctionSignatures
	},
	{
		ARRAYSIZE(Jade::kEngineTypeNames),
		Jade::kEngineTypeNames,
		ARRAYSIZE(Jade::kFunctionNames),
		Jade::kFunctionNames,
		Jade::kFunctionSignatures
	},
	{
		ARRAYSIZE(Witcher::kEngineTypeNames),
		Witcher::kEngineTypeNames,
		ARRAYSIZE(Witcher::kFunctionNames),
		Witcher::kFunctionNames,
		Witcher::kFunctionSignatures
	},
	{
		0,
		0,
		0,
		0,
		0
	},
	{
		ARRAYSIZE(DragonAge::kEngineTypeNames),
		DragonAge::kEngineTypeNames,
		ARRAYSIZE(DragonAge::kFunctionNames),
		DragonAge::kFunctionNames,
		DragonAge::kFunctionSignatures
	},
	{
		ARRAYSIZE(DragonAge2::kEngineTypeNames),
		DragonAge2::kEngineTypeNames,
		ARRAYSIZE(DragonAge2::kFunctionNames),
		DragonAge2::kFunctionNames,
		DragonAge2::kFunctionSignatures
	}
};

const GameInfo *getGameInfo(Aurora::GameID game) {
	if ((size_t)game >= ARRAYSIZE(kGameInfo) || (game == Aurora::kGameIDSonic))
		return 0;

	return &kGameInfo[(size_t)game];
}

size_t getEngineTypeCount(Aurora::GameID game) {
	const GameInfo *info = getGameInfo(game);
	if (!info)
		return 0;

	return info->engineTypeCount;
}

Common::UString getGenericEngineTypeName(size_t n) {
	return Common::UString::format("E") + Common::composeString(n);
}

Common::UString getEngineTypeName(Aurora::GameID game, size_t n) {
	const GameInfo *info = getGameInfo(game);
	if (!info || (n >= info->engineTypeCount))
		return getGenericEngineTypeName(n);

	return info->engineTypeNames[n];
}

size_t getFunctionCount(Aurora::GameID game) {
	const GameInfo *info = getGameInfo(game);
	if (!info)
		return 0;

	return info->functionCount;
}

bool hasFunction(Aurora::GameID game, size_t n) {
	return !getFunctionName(game, n).empty();
}

Common::UString getFunctionName(Aurora::GameID game, size_t n) {
	const GameInfo *info = getGameInfo(game);
	if (!info || (n >= info->functionCount))
		return "";

	return info->functionNames[n];
}

VariableType getFunctionReturnType(Aurora::GameID game, size_t n) {
	const GameInfo *info = getGameInfo(game);
	if (!info || (n >= info->functionCount))
		return kTypeVoid;

	return info->functionSignatures[n][0];
}

size_t getFunctionParameterCount(Aurora::GameID game, size_t n) {
	const GameInfo *info = getGameInfo(game);
	if (!info || (n >= info->functionCount))
		return 0;

	size_t count = 0;
	for (size_t i = 1; (i < GameInfo::kMaxSignatureSize) && (info->functionSignatures[n][i] != kTypeVoid); i++)
		count++;

	return count;
}

const VariableType *getFunctionParameters(Aurora::GameID game, size_t n) {
	const GameInfo *info = getGameInfo(game);
	if (!info || (n >= info->functionCount))
		return 0;

	return &info->functionSignatures[n][1];
}

} // End of namespace NWScript
