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
 *  Creates SSFs out of XML files.
 */

#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/readstream.h"
#include "src/common/writestream.h"

#include "src/aurora/ssffile.h"

#include "src/xml/ssfcreator.h"
#include "src/xml/xmlparser.h"

namespace XML {

void SSFCreator::create(Common::WriteStream &output, Common::ReadStream &input,
                        Aurora::GameID game, const Common::UString &inputFileName) {

	XMLParser xml(input, true, inputFileName);
	const XMLNode &xmlRoot = xml.getRoot();

	if (xmlRoot.getName() != "ssf")
		throw Common::Exception("XML does not describe a SSF");

	Aurora::SSFFile ssf;

	const XMLNode::Children &strings = xmlRoot.getChildren();

	for (XMLNode::Children::const_iterator s = strings.begin(); s != strings.end(); ++s) {
		if ((*s)->getName() != "sound")
			throw Common::Exception("XML tag \"sound\" expected");

		const Common::UString xmlID = (*s)->getProperty("id");
		if (xmlID.empty())
			throw Common::Exception("XML property \"id\" expected");

		size_t soundID = 0;
		Common::parseString(xmlID, soundID, false);

		Common::UString soundFile;
		const XMLNode *text = (*s)->findChild("text");
		if (text)
			soundFile = text->getContent();

		uint32_t strRef = 0xFFFFFFFF;
		Common::parseString((*s)->getProperty("strref"), strRef, true);

		ssf.setSound(soundID, soundFile, strRef);
	}

	ssf.writeSSF(output, ssf.determineVersionForGame(game));
}

} // End of namespace XML
