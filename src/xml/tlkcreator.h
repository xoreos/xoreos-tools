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

#ifndef XML_TLKCREATOR_H
#define XML_TLKCREATOR_H

#include "src/common/encoding.h"

namespace Common {
	class ReadStream;
	class WriteStream;
}

namespace XML {

class TLKCreator {
public:
	enum Version {
		kVersionInvalid,
		kVersion30,
		kVersion40
	};

	static void create(Common::WriteStream &output, Common::ReadStream &input,
	                   Version &version, Common::Encoding encoding,
	                   const Common::UString &inputFileName, uint32_t languageID = 0xFFFFFFFF);
};

} // End of namespace XML

#endif // XML_TLKCREATOR_H
