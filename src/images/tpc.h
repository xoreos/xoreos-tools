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
 *  TPC (BioWare's own texture format) loading.
 */

#ifndef IMAGES_TPC_H
#define IMAGES_TPC_H

#include "src/common/scopedptr.h"

#include "src/images/decoder.h"

namespace Common {
	class SeekableReadStream;
}

namespace Images {

/** BioWare's own texture format, TPC.
 *
 *  This format is used by the two Knights of the Old Republic games.
 *  In the Xbox versions, these files have a .txb extension, but
 *  are still this format (not the TXB format used in Jade Empire).
 */
class TPC : public Decoder {
public:
	TPC(Common::SeekableReadStream &tpc);
	~TPC();

	/** Return the enclosed TXI data. */
	Common::SeekableReadStream *getTXI() const;

private:
	Common::ScopedArray<byte> _txiData;
	size_t _txiDataSize;

	uint32 _offset;
	bool _isAnimated;

	// Loading helpers
	void load(Common::SeekableReadStream &tpc);
	void readHeader(Common::SeekableReadStream &tpc, byte &encoding);
	void readData(Common::SeekableReadStream &tpc, byte encoding);
	void readTXIData(Common::SeekableReadStream &tpc);

	bool checkAnimated(uint32 &width, uint32 &height, uint32 &dataSize);
	bool checkCubeMap(uint32 &width, uint32 &height);
	void fixupCubeMap();

	static void deSwizzle(byte *dst, const byte *src, uint32 width, uint32 height);
};

} // End of namespace Images

#endif // IMAGES_TPC_H
