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
 *  Dump GFFs into XML files.
 */

#include "src/common/stream.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"

#include "src/xml/xmlwriter.h"
#include "src/xml/gff4dumper.h"

namespace XML {

GFF4Dumper::GFF4Field::GFF4Field(const Aurora::GFF4Struct &s, uint32 f) : strct(&s), field(f) {
	if (!strct->getProperties(field, type, label, isList))
		throw Common::Exception(Common::kReadError);
}


GFF4Dumper::GFF4Dumper() : _gff4(0), _xml(0) {
}

GFF4Dumper::~GFF4Dumper() {
	clear();
}

void GFF4Dumper::clear() {
	delete _gff4;
	delete _xml;

	_gff4 = 0;
	_xml  = 0;

	_structIDs.clear();
}

void GFF4Dumper::dump(Common::WriteStream &output, Common::SeekableReadStream &input) {
	try {
		_gff4 = new Aurora::GFF4File(input);
		_xml  = new XMLWriter(output);

		_xml->openTag("gff4");
		_xml->addProperty("type"    , Common::tagToString(_gff4->getType()       , true));
		_xml->addProperty("version" , Common::tagToString(_gff4->getTypeVersion(), true));
		_xml->addProperty("platform", Common::tagToString(_gff4->getPlatform()   , true));
		_xml->breakLine();

		dumpStruct(&_gff4->getTopLevel());

		_xml->closeTag();
		_xml->breakLine();

		_xml->flush();

	} catch (...) {
		clear();
		throw;
	}

	clear();
}

bool GFF4Dumper::insertID(uint32 id) {
	std::pair<IDSet::iterator, bool> result = _structIDs.insert(id);

	return result.second;
}

void GFF4Dumper::dumpStruct(const Aurora::GFF4Struct *strct, uint32 label) {
	_xml->openTag("struct");
	_xml->addProperty("name", strct ? Common::tagToString(strct->getLabel()) : "");
	_xml->addProperty("label", Common::UString::sprintf("%u", label));

	if (strct) {
		if (insertID(strct->getID())) {
			if (strct->getRefCount() > 1)
				_xml->addProperty("id", Common::UString::sprintf("%u", strct->getID()));

			_xml->breakLine();

			for (Aurora::GFF4Struct::iterator f = strct->begin(); f != strct->end(); ++f)
				dumpField(*strct, *f);
		} else
			_xml->addProperty("ref_id", Common::UString::sprintf("%u", strct->getID()));
	}

	_xml->closeTag();
	_xml->breakLine();
}

static const char *kGFF4IFieldTypeNames[] = {
	"uint8",
	"sint8",
	"uint16",
	"sint16",
	"uint32",
	"sint32",
	"uint64",
	"sint64",
	"float",
	"double",
	"vector3f",
	"fieldtype11",
	"vector4f",
	"quaternionf",
	"string",
	"color4f",
	"matrix4x4f",
	"tlkstring"
};

Common::UString GFF4Dumper::getIFieldTypeName(uint32 type, bool isList) const {
	const char *listString = isList ? "_list" : "";
	const char *typeString = "invalid";
	if      (type == Aurora::GFF4Struct::kIFieldTypeStruct)
		typeString = "struct";
	else if (type == Aurora::GFF4Struct::kIFieldTypeGeneric)
		typeString = "generic";
	else if (type < ARRAYSIZE(kGFF4IFieldTypeNames))
		typeString = kGFF4IFieldTypeNames[type];

	return Common::UString::sprintf("%s%s", typeString, listString);
}

void GFF4Dumper::openFieldTag(uint32 type, bool isList) {
	_xml->openTag(getIFieldTypeName(type, isList));
}

void GFF4Dumper::openFieldTag(uint32 type, bool isList, uint32 label) {
	_xml->openTag(getIFieldTypeName(type, isList));
	_xml->addProperty("label", Common::UString::sprintf("%u", label));
}

void GFF4Dumper::closeFieldTag(bool doBreak) {
	_xml->closeTag();
	if (doBreak)
		_xml->breakLine();
}

void GFF4Dumper::dumpFieldUint(const GFF4Field &field) {
	std::vector<uint64> values;
	if (!field.strct->getUint(field.field, values))
		throw Common::Exception(Common::kReadError);

	if (field.isList && !values.empty())
		_xml->breakLine();

	for (uint32 i = 0; i < values.size(); i++) {
		openFieldTag(field.type, false, field.isList ? i : field.label);
		_xml->setContents(Common::UString::sprintf("%"PRIu64, Cu64(values[i])));
		closeFieldTag();
	}
}

void GFF4Dumper::dumpFieldSint(const GFF4Field &field) {
	std::vector<int64> values;
	if (!field.strct->getSint(field.field, values))
		throw Common::Exception(Common::kReadError);

	if (field.isList && !values.empty())
		_xml->breakLine();

	for (uint32 i = 0; i < values.size(); i++) {
		openFieldTag(field.type, false, field.isList ? i : field.label);
		_xml->setContents(Common::UString::sprintf("%"PRId64, Cd64(values[i])));
		closeFieldTag();
	}
}

void GFF4Dumper::dumpFieldDouble(const GFF4Field &field) {
	std::vector<double> values;
	if (!field.strct->getDouble(field.field, values))
		throw Common::Exception(Common::kReadError);

	if (field.isList && !values.empty())
		_xml->breakLine();

	for (uint32 i = 0; i < values.size(); i++) {
		openFieldTag(field.type, false, field.isList ? i : field.label);
		_xml->setContents(Common::UString::sprintf("%.6f", values[i]));
		closeFieldTag();
	}
}

void GFF4Dumper::dumpFieldString(const GFF4Field &field) {
	std::vector<Common::UString> values;
	if (!field.strct->getString(field.field, values))
		throw Common::Exception(Common::kReadError);

	if (field.isList && !values.empty())
		_xml->breakLine();

	for (uint32 i = 0; i < values.size(); i++) {
		openFieldTag(field.type, false, field.isList ? i : field.label);
		_xml->setContents(values[i]);
		closeFieldTag();
	}
}

void GFF4Dumper::dumpFieldTlk(const GFF4Field &field) {
	std::vector<uint32> strRefs;
	std::vector<Common::UString> strs;

	if (!field.strct->getTalkString(field.field, strRefs, strs))
		throw Common::Exception(Common::kReadError);

	if (field.isList && !strRefs.empty())
		_xml->breakLine();

	for (uint32 i = 0; i < strRefs.size(); i++) {
		openFieldTag(field.type, false, field.isList ? i : field.label);

		openFieldTag(Aurora::GFF4Struct::kIFieldTypeUint32, false);
		_xml->setContents(Common::UString::sprintf("%u", strRefs[i]));
		closeFieldTag(false);

		openFieldTag(Aurora::GFF4Struct::kIFieldTypeString, false);
		_xml->setContents(strs[i]);
		closeFieldTag(false);

		closeFieldTag();
	}
}

void GFF4Dumper::dumpFieldVector(const GFF4Field &field) {
	std::vector< std::vector<double> > values;
	if (!field.strct->getVectorMatrix(field.field, values))
		throw Common::Exception(Common::kReadError);

	if (field.isList && !values.empty())
		_xml->breakLine();

	for (uint32 i = 0; i < values.size(); i++) {
		openFieldTag(field.type, false, field.isList ? i : field.label);
		_xml->breakLine();

		for (uint32 j = 0; j < values[i].size(); j++) {

			openFieldTag(Aurora::GFF4Struct::kIFieldTypeFloat32, false);
			_xml->setContents(Common::UString::sprintf("%.6f", values[i][j]));
			closeFieldTag(false);

			if ((j == (values[i].size() - 1)) || ((j % 4) == 3))
				_xml->breakLine();
		}

		closeFieldTag();
	}
}

void GFF4Dumper::dumpFieldList(const GFF4Field &field) {
	const Aurora::GFF4List &lst = field.strct->getList(field.field);

	if (field.isList && !lst.empty())
		_xml->breakLine();

	for (uint32 i = 0; i < lst.size(); i++)
		dumpStruct(lst[i], field.isList ? i : field.label);
}

void GFF4Dumper::dumpFieldGeneric(const GFF4Field &field) {
	const Aurora::GFF4Struct *generic = field.strct->getGeneric(field.field);
	if (!generic || !generic->getFieldCount())
		return;

	_xml->breakLine();
	for (Aurora::GFF4Struct::iterator f = generic->begin(); f != generic->end(); ++f)
		dumpField(*generic, *f);
}

void GFF4Dumper::dumpField(const Aurora::GFF4Struct &strct, uint32 field) {
	GFF4Field f(strct, field);

	if (f.isList)
		openFieldTag(f.type, true, f.label);

	switch (f.type) {
		case Aurora::GFF4Struct::kIFieldTypeUint8:
		case Aurora::GFF4Struct::kIFieldTypeUint16:
		case Aurora::GFF4Struct::kIFieldTypeUint32:
		case Aurora::GFF4Struct::kIFieldTypeUint64:
			dumpFieldUint(f);
			break;

		case Aurora::GFF4Struct::kIFieldTypeSint8:
		case Aurora::GFF4Struct::kIFieldTypeSint16:
		case Aurora::GFF4Struct::kIFieldTypeSint32:
		case Aurora::GFF4Struct::kIFieldTypeSint64:
			dumpFieldSint(f);
			break;

		case Aurora::GFF4Struct::kIFieldTypeFloat32:
		case Aurora::GFF4Struct::kIFieldTypeFloat64:
			dumpFieldDouble(f);
			break;

		case Aurora::GFF4Struct::kIFieldTypeString:
			dumpFieldString(f);
			break;

		case Aurora::GFF4Struct::kIFieldTypeTlkString:
			dumpFieldTlk(f);
			break;

		case Aurora::GFF4Struct::kIFieldTypeVector3f:
		case Aurora::GFF4Struct::kIFieldTypeVector4f:
		case Aurora::GFF4Struct::kIFieldTypeQuaternionf:
		case Aurora::GFF4Struct::kIFieldTypeColor4f:
		case Aurora::GFF4Struct::kIFieldTypeMatrix4x4f:
			dumpFieldVector(f);
			break;

		case Aurora::GFF4Struct::kIFieldTypeStruct:
			dumpFieldList(f);
			break;

		case Aurora::GFF4Struct::kIFieldTypeGeneric:
			if (!f.isList)
				openFieldTag(f.type, f.isList, f.label);

			dumpFieldGeneric(f);

			if (!f.isList)
				closeFieldTag();
			break;

		default:
			if (f.isList)
				_xml->breakLine();

			openFieldTag(f.type, false, f.isList ? 0 : f.label);
			closeFieldTag();
			break;
	}

	if (f.isList)
		closeFieldTag();
}

} // End of namespace XML
