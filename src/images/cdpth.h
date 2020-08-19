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
 *  Compressed DePTH, a BioWare image-ish format found in Sonic.
 */

#ifndef IMAGES_CDPTH_H
#define IMAGES_CDPTH_H

#include <vector>

#include "src/common/ptrvector.h"

#include "src/images/decoder.h"

namespace Common {
	class SeekableReadStream;
}

namespace Images {

/** Loader for CDPTH, BioWare's Compressed DePTH, a format found in
 *  Sonic, used as depth information for the area background images.
 *
 *  Layout-wise, a CDPTH is stored similar to CBGT: cells of 64x64
 *  pixels, compressed using Nintendo's 0x10 LZSS algorithm. Unlike
 *  CBGT, though, the cells themselves are *not* swizzled into 8x8
 *  tiles, and the pixel value in CDPTH is a 16bit integer specifying
 *  a depth.
 *
 *  The width and height of the final image is not stored within the
 *  CDPTH file, and has to be provided from the outside, like from
 *  the dimensions of the CBGT image, the dimensions of the 2DA file
 *  for the CBGT image, or the relevant values found in areas.gda.
 */
class CDPTH : public Decoder {
public:
	CDPTH(Common::SeekableReadStream &cdpth, uint32_t width, uint32_t height);
	~CDPTH();

private:
	typedef Common::PtrVector<Common::SeekableReadStream> Cells;

	struct ReadContext {
		Common::SeekableReadStream *cdpth;

		Cells cells;

		uint32_t width;
		uint32_t height;

		ReadContext(Common::SeekableReadStream &c, uint32_t w, uint32_t h);
	};

	void load(ReadContext &ctx);

	void readCells(ReadContext &ctx);

	void checkConsistency(ReadContext &ctx);

	void createImage(uint32_t width, uint32_t height);
	void drawImage(ReadContext &ctx);
};

} // End of namespace Images

#endif // IMAGES_CDPTH_H
