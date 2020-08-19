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
 *  Creates TLKs out of XML files.
 */

#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/readstream.h"
#include "src/common/writestream.h"

#include "src/aurora/language.h"
#include "src/aurora/talktable_tlk.h"

#include "src/xml/tlkcreator.h"
#include "src/xml/xmlparser.h"

namespace XML {

void TLKCreator::create(Common::WriteStream &output, Common::ReadStream &input,
                        Version &version, Common::Encoding encoding,
                        const Common::UString &inputFileName, uint32_t languageID) {

	if ((version != kVersion30) && (version != kVersion40))
		throw Common::Exception("Invalid TLK version");

	XMLParser xml(input, true, inputFileName);
	const XMLNode &xmlRoot = xml.getRoot();

	if (xmlRoot.getName() != "tlk")
		throw Common::Exception("XML does not describe a TLK");

	if (languageID == 0xFFFFFFFF) {
		const Common::UString xmlLanguage = xmlRoot.getProperty("language");

		if (!xmlLanguage.empty())
			Common::parseString(xmlLanguage, languageID, true);
	}

	if (languageID == 0xFFFFFFFF)
		throw Common::Exception("Missing language ID");

	if (encoding == Common::kEncodingInvalid)
		encoding = LangMan.getEncoding(LangMan.getLanguage(languageID));

	if (encoding == Common::kEncodingInvalid)
		throw Common::Exception("Missing encoding");

	Aurora::TalkTable_TLK tlk(encoding, languageID);

	const XMLNode::Children &strings = xmlRoot.getChildren();

	// Look at the ID of the last string entry, and create a dummy entry to speed up re-allocation
	if (!strings.empty()) {
		uint32_t lastID = 0xFFFFFFFF;

		XMLNode::Children::const_iterator last = --strings.end();
		Common::parseString((*last)->getProperty("id"), lastID, true);

		if (lastID != 0xFFFFFFFF)
			tlk.setEntry(lastID, "", "", 0, 0, -1.0f, 0xFFFFFFFF);
	}

	for (XMLNode::Children::const_iterator s = strings.begin(); s != strings.end(); ++s) {
		if ((*s)->getName() != "string")
			throw Common::Exception("XML tag \"string\" expected");

		const Common::UString xmlID = (*s)->getProperty("id");
		if (xmlID.empty())
			throw Common::Exception("XML property \"id\" expected");

		uint32_t strRef = 0xFFFFFFFF;
		Common::parseString(xmlID, strRef, false);

		Common::UString string;
		const XMLNode *text = (*s)->findChild("text");
		if (text)
			string = text->getContent();

		const Common::UString soundResRef = (*s)->getProperty("sound");

		uint32_t volumeVariance = 0, pitchVariance = 0, soundID = 0xFFFFFFFF;
		Common::parseString((*s)->getProperty("volumevariance"), volumeVariance, true);
		Common::parseString((*s)->getProperty("pitchvariance" ), pitchVariance , true);
		Common::parseString((*s)->getProperty("soundid"       ), soundID       , true);

		float soundLength = -1.0f;
		Common::parseString((*s)->getProperty("soundlength"), soundLength, true);

		tlk.setEntry(strRef, string, soundResRef, volumeVariance, pitchVariance, soundLength, soundID);
	}

	if      (version == kVersion30)
		tlk.write30(output);
	else if (version == kVersion40)
		tlk.write40(output);
}

} // End of namespace XML
