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

#include <boost/scope_exit.hpp>

#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/readstream.h"
#include "src/common/writestream.h"

#include "src/xml/xmlwriter.h"
#include "src/xml/gff4dumper.h"
#include "src/xml/gff4fields.h"

namespace XML {

GFF4Dumper::GFF4Field::GFF4Field(const Aurora::GFF4Struct &s, uint32_t f, bool g) :
	strct(&s), field(f), isGeneric(g) {

	if (!strct->getFieldProperties(field, type, label, isList))
		throw Common::Exception(Common::kReadError);
}


GFF4Dumper::GFF4Dumper() {
	for (size_t i = 0; i < ARRAYSIZE(kGFF4FieldName); i++)
		_fieldNames[kGFF4FieldName[i].label] = kGFF4FieldName[i].name;
}

GFF4Dumper::~GFF4Dumper() {
}

Common::UString GFF4Dumper::findFieldName(uint32_t label) const {
	FieldNames::const_iterator n = _fieldNames.find(label);
	if (n == _fieldNames.end())
		return "";

	return n->second;
}

void GFF4Dumper::dump(Common::WriteStream &output, Common::SeekableReadStream *input,
                      Common::Encoding encoding, bool UNUSED(allowNWNPremium)) {

	_encoding = encoding;

	BOOST_SCOPE_EXIT( (&_gff4) (&_xml) ) {
		_gff4.reset();
		_xml.reset();
	} BOOST_SCOPE_EXIT_END

	_gff4 = std::make_unique<Aurora::GFF4File>(input);
	_xml = std::make_unique<XMLWriter>(output);

	if (_encoding == Common::kEncodingInvalid)
			_encoding = _gff4->getNativeEncoding();

	_xml->openTag("gff4");
	_xml->addProperty("type"    , Common::tagToString(_gff4->getType()       , true));
	_xml->addProperty("version" , Common::tagToString(_gff4->getTypeVersion(), true));
	_xml->addProperty("platform", Common::tagToString(_gff4->getPlatform()   , true));
	_xml->breakLine();

	dumpStruct(&_gff4->getTopLevel(), false, 0, false, 0, false);

	_xml->closeTag();
	_xml->breakLine();

	_xml->flush();
}

bool GFF4Dumper::insertID(uint64_t id) {
	std::pair<IDSet::iterator, bool> result = _structIDs.insert(id);

	return result.second;
}

void GFF4Dumper::dumpStruct(const Aurora::GFF4Struct *strct, bool hasLabel, uint32_t label,
                            bool hasIndex, size_t index, bool isGeneric) {

	if (index >= 0xFFFFFFFF)
		throw Common::Exception("GFF4 struct index overflow");

	_xml->openTag("struct");
	_xml->addProperty("name", strct ? Common::tagToString(strct->getLabel()) : "");

	if (hasLabel) {
		_xml->addProperty("label", Common::composeString(label));

		if (!isGeneric) {
			Common::UString alias = findFieldName(label);
			if (!alias.empty())
				_xml->addProperty("alias", alias);
		}
	}

	if (hasIndex)
		_xml->addProperty("index", Common::composeString(index));

	if (strct) {
		if (insertID(strct->getID())) {
			if (strct->getRefCount() > 1)
				_xml->addProperty("id", Common::composeString(strct->getID()));

			_xml->breakLine();

			const std::vector<uint32_t> &fields = strct->getFieldLabels();

			for (std::vector<uint32_t>::const_iterator f = fields.begin(); f != fields.end(); ++f)
				dumpField(*strct, *f, false);
		} else
			_xml->addProperty("ref_id", Common::composeString(strct->getID()));
	}

	_xml->closeTag();
	_xml->breakLine();
}

static const char * const kGFF4FieldTypeNames[] = {
	"uint8_t",
	"sint8",
	"uint16_t",
	"sint16",
	"uint32_t",
	"sint32",
	"uint64_t",
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
	"tlkstring",
	"ndsfixed",
	"fieldtype19",
	"ascii"
};

Common::UString GFF4Dumper::getFieldTypeName(uint32_t type, bool isList) const {
	const char *listString = isList ? "_list" : "";
	const char *typeString = "invalid";
	if      (type == Aurora::GFF4Struct::kFieldTypeStruct)
		typeString = "struct";
	else if (type == Aurora::GFF4Struct::kFieldTypeGeneric)
		typeString = "generic";
	else if (type < ARRAYSIZE(kGFF4FieldTypeNames))
		typeString = kGFF4FieldTypeNames[type];

	return Common::UString::format("%s%s", typeString, listString);
}

void GFF4Dumper::openFieldTag(uint32_t type, bool typeList, bool hasLabel, uint32_t label,
                              bool hasIndex, size_t index, bool isGenericElement) {

	if (index >= 0xFFFFFFFF)
		throw Common::Exception("GFF4 field index overflow");

	_xml->openTag(getFieldTypeName(type, typeList));

	if (hasLabel) {
		_xml->addProperty("label", Common::composeString(label));

		if (!isGenericElement) {
			Common::UString alias = findFieldName(label);
			if (!alias.empty())
				_xml->addProperty("alias", alias);
		}
	}

	if (hasIndex)
		_xml->addProperty("index", Common::composeString(index));
}

void GFF4Dumper::closeFieldTag(bool doBreak) {
	_xml->closeTag();
	if (doBreak)
		_xml->breakLine();
}

void GFF4Dumper::dumpFieldUint(const GFF4Field &field, bool isGenericElement) {
	std::vector<uint64_t> values;
	if (!field.strct->getUint(field.field, values))
		throw Common::Exception(Common::kReadError);

	if (field.isList && !values.empty())
		_xml->breakLine();

	for (size_t i = 0; i < values.size(); i++) {
		openFieldTag(field.type, false, !field.isList, field.label, field.isList, i, isGenericElement);
		_xml->setContents(Common::composeString(values[i]));
		closeFieldTag();
	}
}

void GFF4Dumper::dumpFieldSint(const GFF4Field &field, bool isGenericElement) {
	std::vector<int64_t> values;
	if (!field.strct->getSint(field.field, values))
		throw Common::Exception(Common::kReadError);

	if (field.isList && !values.empty())
		_xml->breakLine();

	for (size_t i = 0; i < values.size(); i++) {
		openFieldTag(field.type, false, !field.isList, field.label, field.isList, i, isGenericElement);
		_xml->setContents(Common::composeString(values[i]));
		closeFieldTag();
	}
}

void GFF4Dumper::dumpFieldDouble(const GFF4Field &field, bool isGenericElement) {
	std::vector<double> values;
	if (!field.strct->getDouble(field.field, values))
		throw Common::Exception(Common::kReadError);

	if (field.isList && !values.empty())
		_xml->breakLine();

	for (size_t i = 0; i < values.size(); i++) {
		openFieldTag(field.type, false, !field.isList, field.label, field.isList, i, isGenericElement);
		_xml->setContents(Common::UString::format("%.6f", values[i]));
		closeFieldTag();
	}
}

void GFF4Dumper::dumpFieldString(const GFF4Field &field, bool isGenericElement) {
	std::vector<Common::UString> values;
	if (!field.strct->getString(field.field, _encoding, values))
		throw Common::Exception(Common::kReadError);

	if (field.isList && !values.empty())
		_xml->breakLine();

	for (size_t i = 0; i < values.size(); i++) {
		openFieldTag(field.type, false, !field.isList, field.label, field.isList, i, isGenericElement);
		_xml->setContents(values[i]);
		closeFieldTag();
	}
}

void GFF4Dumper::dumpFieldTlk(const GFF4Field &field, bool isGenericElement) {
	std::vector<uint32_t> strRefs;
	std::vector<Common::UString> strs;

	if (!field.strct->getTalkString(field.field, _encoding, strRefs, strs))
		throw Common::Exception(Common::kReadError);

	if (field.isList && !strRefs.empty())
		_xml->breakLine();

	for (size_t i = 0; i < strRefs.size(); i++) {
		openFieldTag(field.type, false, !field.isList, field.label, field.isList, i, isGenericElement);

		openFieldTag(Aurora::GFF4Struct::kFieldTypeUint32, false, false, 0, false, 0);
		_xml->setContents(Common::composeString(strRefs[i]));
		closeFieldTag(false);

		openFieldTag(Aurora::GFF4Struct::kFieldTypeString, false, false, 0, false, 0);
		_xml->setContents(strs[i]);
		closeFieldTag(false);

		closeFieldTag();
	}
}

void GFF4Dumper::dumpFieldVector(const GFF4Field &field, bool isGenericElement) {
	std::vector< std::vector<double> > values;
	if (!field.strct->getVectorMatrix(field.field, values))
		throw Common::Exception(Common::kReadError);

	if (field.isList && !values.empty())
		_xml->breakLine();

	for (size_t i = 0; i < values.size(); i++) {
		openFieldTag(field.type, false, !field.isList, field.label, field.isList, i, isGenericElement);
		_xml->breakLine();

		for (size_t j = 0; j < values[i].size(); j++) {

			openFieldTag(Aurora::GFF4Struct::kFieldTypeFloat32, false, false, 0, false, 0);
			_xml->setContents(Common::UString::format("%.6f", values[i][j]));
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

	for (size_t i = 0; i < lst.size(); i++)
		dumpStruct(lst[i], !field.isList, field.label, field.isList, i, field.isGeneric);
}

void GFF4Dumper::dumpFieldGeneric(const GFF4Field &field) {
	const Aurora::GFF4Struct *generic = field.strct->getGeneric(field.field);
	if (!generic)
		return;

	const std::vector<uint32_t> &fields = generic->getFieldLabels();

	for (std::vector<uint32_t>::const_iterator f = fields.begin(); f != fields.end(); ++f) {
		if (f == fields.begin())
			_xml->breakLine();

		dumpField(*generic, *f, true);
	}
}

void GFF4Dumper::dumpField(const Aurora::GFF4Struct &strct, uint32_t field, bool isGeneric) {
	GFF4Field f(strct, field, isGeneric);

	if (f.isList)
		openFieldTag(f.type, true, true, f.label, false, 0);

	switch (f.type) {
		case Aurora::GFF4Struct::kFieldTypeUint8:
		case Aurora::GFF4Struct::kFieldTypeUint16:
		case Aurora::GFF4Struct::kFieldTypeUint32:
		case Aurora::GFF4Struct::kFieldTypeUint64:
			dumpFieldUint(f, isGeneric);
			break;

		case Aurora::GFF4Struct::kFieldTypeSint8:
		case Aurora::GFF4Struct::kFieldTypeSint16:
		case Aurora::GFF4Struct::kFieldTypeSint32:
		case Aurora::GFF4Struct::kFieldTypeSint64:
			dumpFieldSint(f, isGeneric);
			break;

		case Aurora::GFF4Struct::kFieldTypeFloat32:
		case Aurora::GFF4Struct::kFieldTypeFloat64:
		case Aurora::GFF4Struct::kFieldTypeNDSFixed:
			dumpFieldDouble(f, isGeneric);
			break;

		case Aurora::GFF4Struct::kFieldTypeString:
		case Aurora::GFF4Struct::kFieldTypeASCIIString:
			dumpFieldString(f, isGeneric);
			break;

		case Aurora::GFF4Struct::kFieldTypeTlkString:
			dumpFieldTlk(f, isGeneric);
			break;

		case Aurora::GFF4Struct::kFieldTypeVector3f:
		case Aurora::GFF4Struct::kFieldTypeVector4f:
		case Aurora::GFF4Struct::kFieldTypeQuaternionf:
		case Aurora::GFF4Struct::kFieldTypeColor4f:
		case Aurora::GFF4Struct::kFieldTypeMatrix4x4f:
			dumpFieldVector(f, isGeneric);
			break;

		case Aurora::GFF4Struct::kFieldTypeStruct:
			dumpFieldList(f);
			break;

		case Aurora::GFF4Struct::kFieldTypeGeneric:
			if (!f.isList)
				openFieldTag(f.type, false, true, f.label, false, 0);

			dumpFieldGeneric(f);

			if (!f.isList)
				closeFieldTag();
			break;

		default:
			if (f.isList)
				_xml->breakLine();

			openFieldTag(f.type, false, !f.isList, f.label, f.isList, 0, isGeneric);
			closeFieldTag();
			break;
	}

	if (f.isList)
		closeFieldTag();
}

} // End of namespace XML
