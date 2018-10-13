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
 *  Creates GFFs out of XML files.
 */

#include "src/common/error.h"

#include "src/xml/xmlparser.h"
#include "src/xml/gffcreator.h"
#include "src/xml/gff3creator.h"

namespace XML {

void GFFCreator::create(Common::WriteStream &output, Common::ReadStream &input, const Common::UString &inputFileName) {
	XMLParser xml(input, true, inputFileName);

	const XMLNode &xmlRoot = xml.getRoot();

	const Common::UString type = xmlRoot.getProperty("type") + "    ";
	const uint32 typeId = MKTAG(*type.getPosition(0), *type.getPosition(1), *type.getPosition(2), *type.getPosition(3));

	if (xmlRoot.getName() == "gff3") {
		XML::GFF3Creator::create(xmlRoot, typeId, output);
	} else if (xmlRoot.getName() == "gff4") {
		throw Common::Exception("TODO: Add GFF4 writer support");
	} else {
		throw Common::Exception("GFFCreator::create() invalid root tag");
	}
}

} // End of namespace XML
