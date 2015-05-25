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
 *  Basic type definitions for working with images.
 */

#ifndef IMAGES_TYPES_H
#define IMAGES_TYPES_H

namespace Images {

enum PixelFormat {
	kPixelFormatR8G8B8,
	kPixelFormatB8G8R8,

	kPixelFormatR8G8B8A8,
	kPixelFormatB8G8R8A8,

	kPixelFormatA1R5G5B5,

	kPixelFormatR5G6B5,

	kPixelFormatDepth16,

	kPixelFormatDXT1,
	kPixelFormatDXT3,
	kPixelFormatDXT5
};

} // End of namespace Images

#endif // IMAGES_TYPES_H
