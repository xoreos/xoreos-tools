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
 *  Our very own intermediate texture format.
 *  Currently used by NSBTX.
 */

#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/readstream.h"
#include "src/common/error.h"

#include "src/images/xoreositex.h"

static const uint32_t kXEOSID = MKTAG('X', 'E', 'O', 'S');
static const uint32_t kITEXID = MKTAG('I', 'T', 'E', 'X');

namespace Images {

XEOSITEX::XEOSITEX(Common::SeekableReadStream &xeositex) {
	load(xeositex);
}

XEOSITEX::~XEOSITEX() {
}

void XEOSITEX::load(Common::SeekableReadStream &xeositex) {
	try {

		readHeader(xeositex);
		readMipMaps(xeositex);

	} catch (Common::Exception &e) {
		e.add("Failed reading XEOSITEX file");
		throw;
	}
}

void XEOSITEX::readHeader(Common::SeekableReadStream &xeositex) {
	const uint32_t magic1 = xeositex.readUint32BE();
	const uint32_t magic2 = xeositex.readUint32BE();
	if ((magic1 != kXEOSID) || (magic2 != kITEXID))
		throw Common::Exception("Not a valid XEOSITEX (%s, %s)",
				Common::debugTag(magic1).c_str(), Common::debugTag(magic2).c_str());

	const uint32_t version = xeositex.readUint32LE();
	if (version != 0)
		throw Common::Exception("Invalid XEOSITEX version %u", version);

	const uint32_t pixelFormat = xeositex.readUint32LE();
	if ((pixelFormat != 3) && (pixelFormat != 4))
		throw Common::Exception("Invalid XEOSITEX pixel format %u", pixelFormat);

	if      (pixelFormat == 3)
		_format = kPixelFormatB8G8R8;
	else if (pixelFormat == 4)
		_format = kPixelFormatB8G8R8A8;

	_wrapX = xeositex.readByte() != 0;
	_wrapY = xeositex.readByte() != 0;
	_flipX = xeositex.readByte() != 0;
	_flipY = xeositex.readByte() != 0;

	_coordTransform = xeositex.readByte();

	xeositex.skip(1); // Filter

	const uint32_t mipMaps = xeositex.readUint32LE();
	_mipMaps.resize(mipMaps, 0);
}

void XEOSITEX::readMipMaps(Common::SeekableReadStream &xeositex) {
	for (size_t i = 0; i < _mipMaps.size(); i++) {
		_mipMaps[i] = new MipMap;

		_mipMaps[i]->width  = xeositex.readUint32LE();
		_mipMaps[i]->height = xeositex.readUint32LE();
		_mipMaps[i]->size   = xeositex.readUint32LE();

		_mipMaps[i]->data = std::make_unique<byte[]>(_mipMaps[i]->size);

		if (xeositex.read(_mipMaps[i]->data.get(), _mipMaps[i]->size) != _mipMaps[i]->size)
			throw Common::Exception(Common::kReadError);
	}
}

} // End of namespace Images
