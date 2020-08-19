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
#include <memory>

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
		uint32_t field;

		uint32_t label;
		Aurora::GFF4Struct::FieldType type;
		bool isList;
		bool isGeneric;

		GFF4Field(const Aurora::GFF4Struct &s, uint32_t f, bool g);
	};

	typedef std::set<uint64_t> IDSet;
	typedef std::map<uint32_t, Common::UString> FieldNames;


	FieldNames _fieldNames;

	std::unique_ptr<Aurora::GFF4File> _gff4;
	std::unique_ptr<XMLWriter> _xml;

	Common::Encoding _encoding;

	IDSet _structIDs;


	bool insertID(uint64_t id);

	Common::UString getFieldTypeName(uint32_t type, bool isList) const;

	void openFieldTag (uint32_t type, bool typeList, bool hasLabel, uint32_t label, bool hasIndex,
	                   size_t index, bool isGenericElement = false);
	void closeFieldTag(bool doBreak = true);

	void dumpFieldUint  (const GFF4Field &field, bool isGenericElement = false);
	void dumpFieldSint  (const GFF4Field &field, bool isGenericElement = false);
	void dumpFieldDouble(const GFF4Field &field, bool isGenericElement = false);
	void dumpFieldString(const GFF4Field &field, bool isGenericElement = false);
	void dumpFieldTlk   (const GFF4Field &field, bool isGenericElement = false);
	void dumpFieldVector(const GFF4Field &field, bool isGenericElement = false);

	void dumpFieldGeneric(const GFF4Field &field);
	void dumpFieldList   (const GFF4Field &field);

	void dumpField(const Aurora::GFF4Struct &strct, uint32_t field, bool isGeneric);

	void dumpStruct(const Aurora::GFF4Struct *strct, bool hasLabel, uint32_t label,
	                bool hasIndex, size_t index, bool isGeneric);

	Common::UString findFieldName(uint32_t label) const;
};

} // End of namespace XML

#endif // XML_GFF4DUMPER_H
