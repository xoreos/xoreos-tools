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
 *  TXB (another one of BioWare's own texture formats) loading.
 */

#include "src/common/util.h"
#include "src/common/error.h"
#include "src/common/memreadstream.h"

#include "src/images/txb.h"
#include "src/images/util.h"

static const byte kEncodingBGRA = 0x04;
static const byte kEncodingGray = 0x09;
static const byte kEncodingDXT1 = 0x0A;
static const byte kEncodingDXT5 = 0x0C;

namespace Images {

TXB::TXB(Common::SeekableReadStream &txb) : _dataSize(0), _txiDataSize(0) {
	load(txb);

	// In xoreos-tools, we always want decompressed images
	decompress();
}

TXB::~TXB() {
}

void TXB::load(Common::SeekableReadStream &txb) {
	try {

		byte encoding;

		readHeader(txb, encoding);
		readData  (txb, encoding);

		txb.seek(_dataSize + 128);

		readTXIData(txb);

	} catch (Common::Exception &e) {
		e.add("Failed reading TXB file");
		throw;
	}
}

Common::SeekableReadStream *TXB::getTXI() const {
	if (!_txiData || (_txiDataSize == 0))
		return 0;

	return new Common::MemoryReadStream(_txiData.get(), _txiDataSize);
}

static uint32_t getTXBDataSize(byte encoding, PixelFormat format, int32_t width, int32_t height) {
	switch (encoding) {
		case kEncodingBGRA:
		case kEncodingDXT1:
		case kEncodingDXT5:
			return getDataSize(format, width, height);

		case kEncodingGray:
			return width * height;
	}

	return 0;
}

void TXB::readHeader(Common::SeekableReadStream &txb, byte &encoding) {
	// Number of bytes for the pixel data in one full image
	uint32_t dataSize = txb.readUint32LE();

	_dataSize = dataSize;

	txb.skip(4); // Some float

	// Image dimensions
	uint32_t width  = txb.readUint16LE();
	uint32_t height = txb.readUint16LE();

	if ((width >= 0x8000) || (height >= 0x8000))
		throw Common::Exception("Unsupported image dimensions (%ux%u)", width, height);

	// How's the pixel data encoded?
	encoding = txb.readByte();

	// Number of mip maps in the image
	byte mipMapCount = txb.readByte();

	txb.skip(2); // Unknown (Always 0x0101 on 0x0A and 0x0C types, 0x0100 on 0x09?)
	txb.skip(4); // Some float
	txb.skip(108); // Reserved

	if        (encoding == kEncodingBGRA) {
		// Raw BGRA, swizzled

		_format = kPixelFormatB8G8R8A8;

	} else if (encoding == kEncodingGray) {
		// Raw Grayscale, swizzled. We map it to BGR

		_format = kPixelFormatB8G8R8;

	} else if (encoding == kEncodingDXT1) {
		// S3TC DXT1

		_format = kPixelFormatDXT1;

	} else if (encoding == kEncodingDXT5) {
		// S3TC DXT5

		_format = kPixelFormatDXT5;

	} else
		throw Common::Exception("Unknown TXB encoding 0x%02X (%dx%d, %d, %d)",
				encoding, width, height, mipMapCount, dataSize);

	if (!hasValidDimensions(_format, width, height))
		throw Common::Exception("Invalid dimensions (%dx%d) for format %d", width, height, _format);

	const size_t fullImageDataSize = getTXBDataSize(encoding, _format, width, height);
	if (dataSize < fullImageDataSize)
		throw Common::Exception("Image wouldn't fit into data");

	_mipMaps.reserve(mipMapCount);
	for (uint32_t i = 0; i < mipMapCount; i++) {
		std::unique_ptr<MipMap> mipMap = std::make_unique<MipMap>();

		mipMap->width  = width;
		mipMap->height = height;
		mipMap->size   = getTXBDataSize(encoding, _format, width, height);

		_mipMaps.push_back(mipMap.release());

		if (width  > 1) width  >>= 1;
		if (height > 1) height >>= 1;
	}

	if ((mipMapCount != 0) && (_mipMaps.empty()))
		throw Common::Exception("Couldn't read any mip maps");
}

void TXB::deSwizzle(byte *dst, const byte *src, uint32_t width, uint32_t height, uint8_t bpp) {
	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			const uint32_t offset = deSwizzleOffset(x, y, width, height) * bpp;

			for (uint8_t p = 0; p < bpp; p++)
				*dst++ = src[offset + p];
		}
	}
}

void TXB::readData(Common::SeekableReadStream &txb, byte encoding) {
	for (MipMaps::iterator mipMap = _mipMaps.begin(); mipMap != _mipMaps.end(); ++mipMap) {
		const bool needDeSwizzle = (encoding == kEncodingBGRA) || (encoding == kEncodingGray);

		// If the texture width is a power of two, the texture memory layout is "swizzled"
		const bool widthPOT = ((*mipMap)->width & ((*mipMap)->width - 1)) == 0;
		const bool swizzled = needDeSwizzle && widthPOT;

		(*mipMap)->data = std::make_unique<byte[]>((*mipMap)->size);
		if (txb.read((*mipMap)->data.get(), (*mipMap)->size) != (*mipMap)->size)
			throw Common::Exception(Common::kReadError);

		if (encoding == kEncodingGray) {
			// Convert grayscale into BGR

			const uint32_t oldSize = (*mipMap)->size;
			const uint32_t newSize = (*mipMap)->size * 3;

			std::unique_ptr<byte[]> tmp1 = std::make_unique<byte[]>(newSize);
			for (uint32_t i = 0; i < oldSize; i++)
				tmp1[i * 3 + 0] = tmp1[i * 3 + 1] = tmp1[i * 3 + 2] = (*mipMap)->data[i];

			if (swizzled) {
				std::unique_ptr<byte[]> tmp2 = std::make_unique<byte[]>(newSize);
				deSwizzle(tmp2.get(), tmp1.get(), (*mipMap)->width, (*mipMap)->height, 3);

				tmp1.swap(tmp2);
			}

			(*mipMap)->data.swap(tmp1);
			(*mipMap)->size = newSize;

		} else if (swizzled) {
			std::unique_ptr<byte[]> tmp = std::make_unique<byte[]>((*mipMap)->size);

			deSwizzle(tmp.get(), (*mipMap)->data.get(), (*mipMap)->width, (*mipMap)->height, 4);

			(*mipMap)->data.swap(tmp);
		}

	}
}

void TXB::readTXIData(Common::SeekableReadStream &txb) {
	// TXI data for the rest of the TXB
	_txiDataSize = txb.size() - txb.pos();

	if (_txiDataSize == 0)
		return;

	_txiData = std::make_unique<byte[]>(_txiDataSize);

	if (txb.read(_txiData.get(), _txiDataSize) != _txiDataSize)
		throw Common::Exception(Common::kReadError);
}

} // End of namespace Images
