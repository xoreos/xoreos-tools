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

#ifndef IMAGES_XOREOSITEX_H
#define IMAGES_XOREOSITEX_H

#include "src/images/decoder.h"

namespace Common {
	class SeekableReadStream;
}

namespace Images {

class XEOSITEX : public Decoder {
public:
	XEOSITEX(Common::SeekableReadStream &xeositex);
	~XEOSITEX();

private:
	bool _wrapX;
	bool _wrapY;
	bool _flipX;
	bool _flipY;

	uint8_t _coordTransform;

	void load(Common::SeekableReadStream &xeositex);
	void readHeader(Common::SeekableReadStream &xeositex);
	void readMipMaps(Common::SeekableReadStream &xeositex);
};

} // End of namespace Images

#endif // IMAGES_XOREOSITEX_H
