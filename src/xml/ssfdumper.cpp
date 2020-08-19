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
 *  Dump SSFs into XML files.
 */

#include "src/common/strutil.h"
#include "src/common/readstream.h"
#include "src/common/writestream.h"

#include "src/aurora/ssffile.h"

#include "src/xml/ssfdumper.h"
#include "src/xml/xmlwriter.h"

namespace XML {

/** Labels for "long" SSF files with 49 entries. */
static const char * const kLabelsLong[] = {
	"Attack",
	"BattleCry1",
	"BattleCry2",
	"BattleCry3",
	"HealMe",
	"Help",
	"Enemies",
	"Flee",
	"Taunt",
	"GuardMe",
	"Hold",
	"GruntAttack1",
	"GruntAttack2",
	"GruntAttack3",
	"Pain1",
	"Pain2",
	"Pain3",
	"NearDeath",
	"Death",
	"Poisoned",
	"SpellFailed",
	"WeaponSucks",
	"FollowMe",
	"LookHere",
	"Group",
	"MoveOver",
	"PickLock",
	"Search",
	"Hide",
	"CanDo",
	"CantDo",
	"TaskComplete",
	"Encumbered",
	"Selected",
	"Hello",
	"Yes",
	"No",
	"Stop",
	"Rest",
	"Bored",
	"Goodbye",
	"Thanks",
	"Laugh",
	"Cuss",
	"Cheer",
	"TalkToMe",
	"GoodIdea",
	"BadIdea",
	"Threaten"
};

/** Labels for "short" SSF files with 49 entries. */
static const char * const kLabelsShort[] = {
	"BattleCry1",
	"BattleCry2",
	"BattleCry3",
	"BattleCry4",
	"BattleCry5",
	"BattleCry6",
	"Selected1",
	"Selected2",
	"Selected3",
	"GruntAttack1",
	"GruntAttack2",
	"GruntAttack3",
	"Pain1",
	"Pain2",
	"NearDeath",
	"Death",
	"Critical",
	"WeaponSucks",
	"FoundMine",
	"DisabledMine",
	"Hide",
	"Search",
	"PickLock",
	"CanDo",
	"CantDo",
	"Single",
	"Group",
	"Poisoned",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	""
};

void SSFDumper::dump(Common::WriteStream &output, Common::SeekableReadStream &input) {
	Aurora::SSFFile ssf(input);

	XMLWriter xml(output);

	xml.openTag("ssf");
	xml.breakLine();

	for (size_t i = 0; i < ssf.getSoundCount(); i++) {
		xml.openTag("sound");
		xml.addProperty("id", Common::composeString(i));

		if ((ssf.getSoundCount() == ARRAYSIZE(kLabelsLong)) && (i < ARRAYSIZE(kLabelsLong)))
			if (kLabelsLong[i][0] != '\0')
				xml.addProperty("label", kLabelsLong[i]);

		if ((ssf.getSoundCount() == ARRAYSIZE(kLabelsShort)) && (i < ARRAYSIZE(kLabelsShort)))
			if (kLabelsShort[i][0] != '\0')
				xml.addProperty("label", kLabelsShort[i]);

		const uint32_t strRef = ssf.getStrRef(i);
		if (strRef != Aurora::kStrRefInvalid)
			xml.addProperty("strref", Common::composeString(strRef));

		xml.setContents(ssf.getSoundFile(i));

		xml.closeTag();
		xml.breakLine();
	}

	xml.closeTag();
	xml.breakLine();

	xml.flush();
}

} // End of namespace XML
