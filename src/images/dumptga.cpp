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
 *  A simple TGA image dumper.
 */

#include <cstdio>
#include <cstring>

#include "src/common/error.h"
#include "src/common/ustring.h"
#include "src/common/writefile.h"

#include "src/images/decoder.h"

namespace Images {

static void writePixel(Common::WriteFile &file, const byte *&data, PixelFormat format) {
	if (format == kPixelFormatR8G8B8) {
		file.writeByte(data[2]);
		file.writeByte(data[1]);
		file.writeByte(data[0]);
		file.writeByte(0xFF);
		data += 3;
	} else if (format == kPixelFormatB8G8R8) {
		file.writeByte(data[0]);
		file.writeByte(data[1]);
		file.writeByte(data[2]);
		file.writeByte(0xFF);
		data += 3;
	} else if (format == kPixelFormatR8G8B8A8) {
		file.writeByte(data[2]);
		file.writeByte(data[1]);
		file.writeByte(data[0]);
		file.writeByte(data[3]);
		data += 4;
	} else if (format == kPixelFormatB8G8R8A8) {
		file.writeByte(data[0]);
		file.writeByte(data[1]);
		file.writeByte(data[2]);
		file.writeByte(data[3]);
		data += 4;
	} else if (format == kPixelFormatR5G6B5) {
		uint16 color = READ_LE_UINT16(data);
		file.writeByte( color & 0x001F);
		file.writeByte((color & 0x07E0) >>  5);
		file.writeByte((color & 0xF800) >> 11);
		file.writeByte(0xFF);
		data += 2;
	} else if (format == kPixelFormatA1R5G5B5) {
		uint16 color = READ_LE_UINT16(data);
		file.writeByte( color & 0x001F);
		file.writeByte((color & 0x03E0) >>  5);
		file.writeByte((color & 0x7C00) >> 10);
		file.writeByte((color & 0x8000) ? 0xFF : 0x00);
		data += 2;
	} else if (format == kPixelFormatDepth16) {
		uint16 color = READ_LE_UINT16(data);
		file.writeByte(color / 128);
		file.writeByte(color / 128);
		file.writeByte(color / 128);
		file.writeByte((color >= 0x7FFF) ? 0x00 : 0xFF);

		data += 2;
	} else
		throw Common::Exception("Unsupported pixel format: %d", (int) format);

}

void dumpTGA(const Common::UString &fileName, const byte *data, int width, int height, PixelFormat format) {
	if ((width <= 0) || (height <= 0) || !data)
		throw Common::Exception("Invalid image data (%dx%d %d)", width, height, data != 0);

	Common::WriteFile file(fileName);

	file.writeByte(0); // ID Length
	file.writeByte(0); // Palette size
	file.writeByte(2); // Unmapped RGB
	file.writeUint32LE(0); // Color map
	file.writeByte(0);     // Color map
	file.writeUint16LE(0); // X
	file.writeUint16LE(0); // Y

	file.writeUint16LE(width);
	file.writeUint16LE(height);

	file.writeByte(32); // Pixel depths

	file.writeByte(0);

	uint32 count = width * height;
	for (uint32 i = 0; i < count; i++)
		writePixel(file, data, format);
}

void dumpTGA(const Common::UString &fileName, const Decoder &image) {
	if (image.getLayerCount() != 1)
		throw Common::Exception("TODO: dumpTGA() with %u layers", (uint) image.getLayerCount());

	if (image.getMipMapCount() < 1)
		throw Common::Exception("No image");

	const Decoder::MipMap &mipMap = image.getMipMap(0);

	dumpTGA(fileName, mipMap.data, mipMap.width, mipMap.height, image.getFormat());
}

} // End of namespace Images
