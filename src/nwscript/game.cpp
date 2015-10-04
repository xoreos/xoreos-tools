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
		NWN::kEngineTypeCount,
		NWN::kEngineTypeNames,
	},
	{
		NWN2::kEngineTypeCount,
		NWN2::kEngineTypeNames,
	},
	{
		KotOR::kEngineTypeCount,
		KotOR::kEngineTypeNames,
	},
	{
		KotOR2::kEngineTypeCount,
		KotOR2::kEngineTypeNames,
	},
	{
		Jade::kEngineTypeCount,
		Jade::kEngineTypeNames,
	},
	{
		Witcher::kEngineTypeCount,
		Witcher::kEngineTypeNames,
	},
	{
		0,
		0
	},
	{
		DragonAge::kEngineTypeCount,
		DragonAge::kEngineTypeNames,
	},
	{
		DragonAge2::kEngineTypeCount,
		DragonAge2::kEngineTypeNames,
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
	return Common::UString::format("E%u", (uint)n);
}

Common::UString getEngineTypeName(Aurora::GameID game, size_t n) {
	const GameInfo *info = getGameInfo(game);
	if (!info || (n >= info->engineTypeCount))
		return "";

	return info->engineTypeNames[n];
}

} // End of namespace NWScript
