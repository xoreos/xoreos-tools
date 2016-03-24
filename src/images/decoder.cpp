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
 *  Generic image decoder interface.
 */

#include <cassert>

#include "src/common/util.h"
#include "src/common/error.h"
#include "src/common/memreadstream.h"

#include "src/images/decoder.h"
#include "src/images/util.h"
#include "src/images/s3tc.h"
#include "src/images/dumptga.h"

namespace Images {

Decoder::MipMap::MipMap() : width(0), height(0), size(0), data(0) {
}

Decoder::MipMap::MipMap(const MipMap &mipMap) : width(0), height(0), size(0), data(0) {
	*this = mipMap;
}

Decoder::MipMap::~MipMap() {
	delete[] data;
}

Decoder::MipMap &Decoder::MipMap::operator=(const MipMap &mipMap) {
	if (&mipMap == this)
		return *this;

	width  = mipMap.width;
	height = mipMap.height;
	size   = mipMap.size;

	delete[] data;
	data = new byte[size];

	std::memcpy(data, mipMap.data, size);

	return *this;
}

void Decoder::MipMap::swap(MipMap &right) {
	SWAP(width , right.width );
	SWAP(height, right.height);
	SWAP(size  , right.size  );
	SWAP(data  , right.data  );
}


Decoder::Decoder() : _format(kPixelFormatR8G8B8A8), _layerCount(1), _isCubeMap(false) {
}

Decoder::Decoder(const Decoder &decoder) {
	*this = decoder;
}

Decoder::~Decoder() {
	for (std::vector<MipMap *>::iterator m = _mipMaps.begin(); m != _mipMaps.end(); ++m)
		delete *m;
}

Decoder &Decoder::operator=(const Decoder &decoder) {
	_format = decoder._format;

	_layerCount = decoder._layerCount;
	_isCubeMap  = decoder._isCubeMap;

	_mipMaps.clear();
	_mipMaps.reserve(decoder._mipMaps.size());

	for (std::vector<MipMap *>::const_iterator m = decoder._mipMaps.begin(); m != decoder._mipMaps.end(); ++m)
		_mipMaps.push_back(new MipMap(**m));

	return *this;
}

Common::SeekableReadStream *Decoder::getTXI() const {
	return 0;
}

bool Decoder::isCompressed() const {
	return (_format == kPixelFormatDXT1) ||
	       (_format == kPixelFormatDXT3) ||
	       (_format == kPixelFormatDXT5);
}

PixelFormat Decoder::getFormat() const {
	return _format;
}

size_t Decoder::getMipMapCount() const {
	assert((_mipMaps.size() % _layerCount) == 0);

	return _mipMaps.size() / _layerCount;
}

size_t Decoder::getLayerCount() const {
	return _layerCount;
}

bool Decoder::isCubeMap() const {
	assert(!_isCubeMap || (_layerCount == 6));

	return _isCubeMap;
}

const Decoder::MipMap &Decoder::getMipMap(size_t mipMap, size_t layer) const {
	assert(layer < _layerCount);
	assert((_mipMaps.size() % _layerCount) == 0);

	const size_t index = layer * getMipMapCount() + mipMap;

	assert(index < _mipMaps.size());

	return *_mipMaps[index];
}

void Decoder::decompress(MipMap &out, const MipMap &in, PixelFormat format) {
	if ((format != kPixelFormatDXT1) &&
	    (format != kPixelFormatDXT3) &&
	    (format != kPixelFormatDXT5))
		throw Common::Exception("Unknown compressed format %d", format);

	/* The DXT algorithms work on 4x4 pixel blocks. Textures smaller than one
	 * block will be padded, but larger textures need to be correctly aligned. */
	if (!hasValidDimensions(format, in.width, in.height))
		throw Common::Exception("Invalid dimensions (%dx%d) for format %d", in.width, in.height, format);

	out.width  = in.width;
	out.height = in.height;
	out.size   = MAX(out.width * out.height * 4, 64);
	out.data   = new byte[out.size];

	Common::MemoryReadStream *stream = new Common::MemoryReadStream(in.data, in.size);

	if      (format == kPixelFormatDXT1)
		decompressDXT1(out.data, *stream, out.width, out.height, out.width * 4);
	else if (format == kPixelFormatDXT3)
		decompressDXT3(out.data, *stream, out.width, out.height, out.width * 4);
	else if (format == kPixelFormatDXT5)
		decompressDXT5(out.data, *stream, out.width, out.height, out.width * 4);

	delete stream;
}

void Decoder::decompress() {
	if (!isCompressed())
		return;

	for (std::vector<MipMap *>::iterator m = _mipMaps.begin(); m != _mipMaps.end(); ++m) {
		MipMap decompressed;

		decompress(decompressed, **m, _format);

		decompressed.swap(**m);
	}

	_format = kPixelFormatR8G8B8A8;
}

void Decoder::dumpTGA(const Common::UString &fileName) const {
	if (_mipMaps.size() < 1)
		throw Common::Exception("Image contains no mip maps");

	if (!isCompressed()) {
		Images::dumpTGA(fileName, *this);
		return;
	}

	Decoder decoder(*this);
	decoder.decompress();

	Images::dumpTGA(fileName, decoder);
}

void Decoder::flipHorizontally() {
	decompress();

	for (std::vector<MipMap *>::iterator m = _mipMaps.begin(); m != _mipMaps.end(); ++m)
		::Images::flipHorizontally((*m)->data, (*m)->width, (*m)->height, getBPP(_format));
}

void Decoder::flipVertically() {
	decompress();

	for (std::vector<MipMap *>::iterator m = _mipMaps.begin(); m != _mipMaps.end(); ++m)
		::Images::flipVertically((*m)->data, (*m)->width, (*m)->height, getBPP(_format));
}

} // End of namespace Images
