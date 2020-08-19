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
 *  Creates V3.2 GFFs out of XML files.
 */

#ifndef XML_GFF3CREATOR_H
#define XML_GFF3CREATOR_H

#include <memory>

#include "src/aurora/gff3writer.h"

#include "src/xml/xmlparser.h"

namespace XML {

class GFF3Creator {
public:
	static void create(const XML::XMLNode &root, uint32_t id, Common::WriteStream &file, uint32_t version);

private:
	static void readStructContents(const XMLNode::Children &strctNodes, Aurora::GFF3WriterStructPtr strctPtr);
	static void readListContents(const XMLNode::Children &listNodes, Aurora::GFF3WriterListPtr listPtr);
};

} // End of namespace XML

#endif // XML_GFF3CREATOR_H
