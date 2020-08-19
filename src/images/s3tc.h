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
 *  Manual S3TC DXTn decompression methods.
 */

#ifndef IMAGES_S3TC_H
#define IMAGES_S3TC_H

#include "src/common/types.h"

namespace Common {
	class SeekableReadStream;
}

namespace Images {

void decompressDXT1(byte *dest, Common::SeekableReadStream &src, uint32_t width, uint32_t height, uint32_t pitch);
void decompressDXT3(byte *dest, Common::SeekableReadStream &src, uint32_t width, uint32_t height, uint32_t pitch);
void decompressDXT5(byte *dest, Common::SeekableReadStream &src, uint32_t width, uint32_t height, uint32_t pitch);

} // End of namespace Images

#endif // IMAGES_S3TC_H
