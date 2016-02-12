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
 *  Dump GFF V4.0/V4.1 into XML files.
 */

#ifndef XML_GFF4DUMPER_H
#define XML_GFF4DUMPER_H

#include <set>
#include <map>

#include "src/common/ustring.h"

#include "src/aurora/gff4file.h"

#include "src/xml/gffdumper.h"

namespace XML {

class XMLWriter;

/** Dump GFF V4.0/V4.1 into XML files. */
class GFF4Dumper : public GFFDumper {
public:
	GFF4Dumper();
	~GFF4Dumper();

	/** Dump the GFF into XML. */
	void dump(Common::WriteStream &output, Common::SeekableReadStream *input,
	          Common::Encoding encoding, bool allowNWNPremium = false);

private:
	struct GFF4Field {
		const Aurora::GFF4Struct *strct;
		uint32 field;

		uint32 label;
		Aurora::GFF4Struct::FieldType type;
		bool isList;
		bool isGeneric;

		GFF4Field(const Aurora::GFF4Struct &s, uint32 f, bool g);
	};

	typedef std::set<uint64> IDSet;
	typedef std::map<uint32, Common::UString> FieldNames;


	FieldNames _fieldNames;

	Aurora::GFF4File *_gff4;
	XMLWriter *_xml;

	Common::Encoding _encoding;

	IDSet _structIDs;


	bool insertID(uint64 id);

	Common::UString getFieldTypeName(uint32 type, bool isList) const;

	void openFieldTag (uint32 type, bool typeList, bool hasLabel, uint32 label, bool hasIndex, size_t index);
	void closeFieldTag(bool doBreak = true);

	void dumpFieldUint  (const GFF4Field &field);
	void dumpFieldSint  (const GFF4Field &field);
	void dumpFieldDouble(const GFF4Field &field);
	void dumpFieldString(const GFF4Field &field);
	void dumpFieldTlk   (const GFF4Field &field);
	void dumpFieldVector(const GFF4Field &field);

	void dumpFieldGeneric(const GFF4Field &field);
	void dumpFieldList   (const GFF4Field &field);

	void dumpField(const Aurora::GFF4Struct &strct, uint32 field, bool isGeneric);

	void dumpStruct(const Aurora::GFF4Struct *strct, bool hasLabel, uint32 label,
	                bool hasIndex, size_t index, bool isGeneric);

	Common::UString findFieldName(uint32 label) const;

	void clear();
};

} // End of namespace XML

#endif // XML_GFF4DUMPER_H
