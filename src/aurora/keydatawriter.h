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

/**
 * @file
 * An abstract writer for archive files associated with KEY files.
 */

#ifndef AURORA_KEYDATAWRITER_H
#define AURORA_KEYDATAWRITER_H

#include "src/common/readstream.h"

#include "src/aurora/types.h"

namespace Aurora {

class KEYDataWriter {
public:
	virtual ~KEYDataWriter() { };

	/**
	 * Get the size of this data file, to write it into the
	 * KEY file.
	 * @return the total size of this data file
	 */
	virtual uint32_t size() = 0;

	/**
	 * Add a stream of the specified type to the data writer.
	 * @param data the data to write
	 * @param type the type of this data
	 */
	virtual void add(Common::SeekableReadStream &data, FileType type) = 0;
};

} // End of namespace Aurora

#endif // AURORA_KEYDATAWRITER_H
