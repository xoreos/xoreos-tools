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
 *  Loading Nitro CoLoR palette files.
 */

#ifndef IMAGES_NCLR_H
#define IMAGES_NCLR_H

#include "src/common/types.h"

#include "src/aurora/nitrofile.h"

namespace Images {

/** Simple utility class to load NCLR palette files.
 *
 *  Both 4-bit and 8-bit palettes are supported, but both
 *  return a palette with 256 BGR entries.
 */
class NCLR : public Aurora::NitroFile {
public:
	static const byte *load(Common::SeekableReadStream &nclr);

private:
	static const byte *loadNCLR(Common::SeekableSubReadStreamEndian &nclr);

	static void readHeader(Common::SeekableSubReadStreamEndian &nclr);
	static const byte *readPalette(Common::SeekableSubReadStreamEndian &nclr);
};

} // End of namespace Images

#endif // IMAGES_NCLR_H
