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

#ifndef XML_GFFCREATOR_H
#define XML_GFFCREATOR_H

#include "src/common/ustring.h"

namespace Common {
	class ReadStream;
	class WriteStream;
}

namespace XML {

class GFFCreator {
public:
	static void create(Common::WriteStream &output, Common::ReadStream &input, const Common::UString &inputFileName);
};

} // End of namespace XML

#endif // XML_GFFCREATOR_H
