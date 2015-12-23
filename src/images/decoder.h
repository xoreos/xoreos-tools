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

#ifndef IMAGES_DECODER_H
#define IMAGES_DECODER_H

#include <vector>

#include "src/common/types.h"

#include "src/images/types.h"

namespace Common {
	class SeekableReadStream;
	class UString;
}

namespace Images {

/** A generic interface for image decoders. */
class Decoder {
public:
	/** A mip map. */
	struct MipMap {
		int    width;  ///< The mip map's width.
		int    height; ///< The mip map's height.
		uint32 size;   ///< The mip map's size in bytes.
		byte  *data;   ///< The mip map's data.

		MipMap();
		MipMap(const MipMap &mipMap);
		~MipMap();

		MipMap &operator=(const MipMap &mipMap);

		void swap(MipMap &right);
	};

	Decoder();
	Decoder(const Decoder &decoder);
	virtual ~Decoder();

	Decoder &operator=(const Decoder &decoder);

	/** Return the image's general format. */
	PixelFormat getFormat() const;

	/** Return the number of mip maps contained in the image. */
	size_t getMipMapCount() const;
	/** Return the number of layers contained in the image. */
	size_t getLayerCount() const;

	/** Is this image a cube map? */
	bool isCubeMap() const;

	/** Return a mip map. */
	const MipMap &getMipMap(size_t mipMap, size_t layer = 0) const;

	/** Return TXI data, if embedded in the image. */
	virtual Common::SeekableReadStream *getTXI() const;

	/** Dump the image into a TGA. */
	void dumpTGA(const Common::UString &fileName) const;

	/** Flip the whole image horizontally. */
	void flipHorizontally();
	/** Flip the whole image vertically. */
	void flipVertically();

protected:
	PixelFormat _format;

	/** Number of layers in this image. For layered 3D images and cubemaps. */
	size_t _layerCount;
	/** Is this image a cube map? A cube map always needs to have 6 layers! */
	bool _isCubeMap;

	std::vector<MipMap *> _mipMaps;

	/** Is the image data compressed? */
	bool isCompressed() const;

	/** Manually decompress the texture image data. */
	void decompress();

	static void decompress(MipMap &out, const MipMap &in, PixelFormat format);
};

} // End of namespace Images

#endif // IMAGES_DECODER_H
