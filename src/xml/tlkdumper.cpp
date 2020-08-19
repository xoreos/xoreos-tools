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
 *  Dump TLKs into XML files.
 */

#include <memory>

#include "src/common/strutil.h"
#include "src/common/readstream.h"
#include "src/common/writestream.h"

#include "src/aurora/language.h"
#include "src/aurora/talktable.h"

#include "src/xml/tlkdumper.h"
#include "src/xml/xmlwriter.h"

namespace XML {

void TLKDumper::dump(Common::WriteStream &output, Common::SeekableReadStream *input,
                     Common::Encoding encoding) {

	std::unique_ptr<Aurora::TalkTable> tlk(Aurora::TalkTable::load(input, encoding));
	if (!tlk)
		return;

	const uint32_t languageID = tlk->getLanguageID();

	XMLWriter xml(output);

	xml.openTag("tlk");
	if (languageID != Aurora::kLanguageInvalid)
		xml.addProperty("language", Common::composeString(languageID));
	xml.breakLine();

	const std::list<uint32_t> &strRefs = tlk->getStrRefs();

	for (std::list<uint32_t>::const_iterator s = strRefs.begin(); s != strRefs.end(); ++s) {
		const uint32_t strRef = *s;

		Common::UString str, sound;
		uint32_t volumeVariance, pitchVariance, soundID;
		float soundLength;

		tlk->getEntry(strRef, str, sound, volumeVariance, pitchVariance, soundLength, soundID);

		if (str.empty() && sound.empty() && (soundID == 0xFFFFFFFF))
			continue;

		xml.openTag("string");
		xml.addProperty("id", Common::composeString(strRef));

		if (!sound.empty())
			xml.addProperty("sound", sound);

		if (volumeVariance != 0)
			xml.addProperty("volumevariance", Common::composeString(volumeVariance));
		if (pitchVariance != 0)
			xml.addProperty("pitchvariance", Common::composeString(pitchVariance));
		if (soundLength >= 0.0f)
			xml.addProperty("soundlength", Common::composeString(soundLength));

		if (soundID != 0xFFFFFFFF)
			xml.addProperty("soundid", Common::composeString(soundID));

		xml.setContents(str);

		xml.closeTag();
		xml.breakLine();
	}

	xml.closeTag();
	xml.breakLine();

	xml.flush();
}

} // End of namespace XML
