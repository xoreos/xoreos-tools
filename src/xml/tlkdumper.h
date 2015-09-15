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

#ifndef XML_TLKDUMPER_H
#define XML_TLKDUMPER_H

#include "src/common/encoding.h"

namespace Common {
	class SeekableReadStream;
	class WriteStream;
}

namespace XML {

class TLKDumper {
public:
	/** Dump the TLK into XML. */
	static void dump(Common::WriteStream &output, Common::SeekableReadStream *input,
	                 Common::Encoding encoding);
};

} // End of namespace XML

#endif // XML_TLKDUMPER_H
