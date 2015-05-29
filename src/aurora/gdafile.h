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
 *  Handling BioWare's GDAs (2DAs, two-dimensional array, within V4.0 GFFs).
 */

#ifndef AURORA_GDAFILE_H
#define AURORA_GDAFILE_H

#include <vector>
#include <map>

#include "src/common/ustring.h"

#include "src/aurora/types.h"

namespace Common {
	class UString;
	class SeekableReadStream;
}

namespace Aurora {

/** Class to hold the GFF'd two-dimensional array of a GDA file. */
class GDAFile {
public:
	enum Type {
		kTypeEmpty    = -1,
		kTypeString   =  0,
		kTypeInt      =  1,
		kTypeFloat    =  2,
		kTypeBool     =  3,
		kTypeResource =  4
	};

	struct Header {
		uint32 hash;
		Type type;

		uint32 field;

		Header() : hash(0), type(kTypeEmpty), field(0xFFFFFFFF) { }
	};
	typedef std::vector<Header> Headers;


	GDAFile(Common::SeekableReadStream &gda);
	~GDAFile();

	/** Return the number of columns in the array. */
	uint32 getColumnCount() const;
	/** Return the number of rows in the array. */
	uint32 getRowCount() const;

	/** Get the column headers. */
	const Headers &getHeaders() const;

	/** Get a row as a GFF4 struct. */
	const GFF4Struct *getRow(uint32 row) const;

	/** Find a column by its name. */
	uint32 findColumn(const Common::UString &name) const;
	/** Find a column by its hash. */
	uint32 findColumn(uint32 hash) const;


private:
	typedef std::map<uint32, uint32> ColumnHashMap;
	typedef std::map<Common::UString, uint32> ColumnNameMap;


	GFF4File *_gff4;

	Headers _headers;

	const GFF4List *_columns;
	const GFF4List *_rows;

	mutable ColumnHashMap _columnHashMap;
	mutable ColumnNameMap _columnNameMap;


	void load(Common::SeekableReadStream &gda);
	void clear();
};

} // End of namespace Aurora

#endif // AURORA_GDAFILE_H
