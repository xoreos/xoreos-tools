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
 *  Base class for Nitro (Nintendo DS) files.
 */

#ifndef AURORA_NITROFILE_H
#define AURORA_NITROFILE_H

namespace Common {
	class SeekableReadStream;
	class SeekableSubReadStreamEndian;
}

namespace Aurora {

class NitroFile {
protected:
	/** Treat this stream as a Nitro file and return an endian'd stream according to its BOM. */
	static Common::SeekableSubReadStreamEndian *open(Common::SeekableReadStream &stream);
	/** Treat this stream as a Nitro file, take it over and return an endian'd stream according to its BOM. */
	static Common::SeekableSubReadStreamEndian *open(Common::SeekableReadStream *stream);
};

} // End of namespace Aurora

#endif // AURORA_NITROFILE_H
