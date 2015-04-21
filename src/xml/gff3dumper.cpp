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

#include "src/aurora/locstring.h"
#include "src/aurora/gfffile.h"

#include "src/xml/xmlwriter.h"
#include "src/xml/gff3dumper.h"

namespace XML {

GFF3Dumper::GFF3Dumper() : _gff(0), _xml(0) {
}

GFF3Dumper::~GFF3Dumper() {
	clear();
}

void GFF3Dumper::clear() {
	delete _gff;
	delete _xml;

	_gff = 0;
	_xml = 0;
}

void GFF3Dumper::dump(Common::WriteStream &output, Common::SeekableReadStream &input) {
	try {
		_gff = new Aurora::GFFFile(input, 0xFFFFFFFF);
		_xml = new XMLWriter(output);

		_xml->openTag("gff");
		_xml->addProperty("type", Common::tagToString(_gff->getType(), true));
		_xml->breakLine();

		dumpStruct(_gff->getTopLevel());

		_xml->closeTag();
		_xml->breakLine();

		_xml->flush();

	} catch (...) {
		clear();
		throw;
	}

	clear();
}

void GFF3Dumper::dumpLocString(const Aurora::LocString &locString) {
	std::vector<Aurora::LocString::SubLocString> str;
	locString.getStrings(str);

	for (std::vector<Aurora::LocString::SubLocString>::iterator s = str.begin(); s != str.end(); ++s) {
		_xml->openTag("string");
		_xml->addProperty("language", Common::UString::sprintf("%u", s->language));

		_xml->setContents(s->str);
		_xml->closeTag();
		_xml->breakLine();
	}
}

static const char *kGFF3FieldTypeNames[] = {
	"byte",
	"char",
	"uint16",
	"sint16",
	"uint32",
	"sint32",
	"uint64",
	"sint64",
	"float",
	"double",
	"exostring",
	"resref",
	"locstring",
	"data",
	"struct",
	"list",
	"orientation",
	"vector",
	"strref"
};

void GFF3Dumper::dumpField(const Aurora::GFFStruct &strct, const Common::UString &field) {
	Aurora::GFFStruct::FieldType type = strct.getType(field);

	Common::UString typeName;
	if ((type >= 0) && (type < ARRAYSIZE(kGFF3FieldTypeNames)))
		typeName = kGFF3FieldTypeNames[(int)type];
	else
		typeName = Common::UString::sprintf("fieldtype%d", (int)type);

	Common::UString label = field;

	// Structs already open their own tag
	if (type != Aurora::GFFStruct::kFieldTypeStruct) {
		_xml->openTag(typeName);
		_xml->addProperty("label", label);
	}

	switch (type) {
		case Aurora::GFFStruct::kFieldTypeChar:
			_xml->setContents(Common::UString::sprintf("%c", strct.getChar(field)));
			break;

		case Aurora::GFFStruct::kFieldTypeByte:
		case Aurora::GFFStruct::kFieldTypeUint16:
		case Aurora::GFFStruct::kFieldTypeUint32:
		case Aurora::GFFStruct::kFieldTypeUint64:
			_xml->setContents(Common::UString::sprintf("%"PRIu64, Cu64(strct.getUint(field))));
			break;

		case Aurora::GFFStruct::kFieldTypeSint16:
		case Aurora::GFFStruct::kFieldTypeSint32:
		case Aurora::GFFStruct::kFieldTypeSint64:
			_xml->setContents(Common::UString::sprintf("%"PRId64, Cd64(strct.getSint(field))));
			break;

		case Aurora::GFFStruct::kFieldTypeFloat:
		case Aurora::GFFStruct::kFieldTypeDouble:
			_xml->setContents(Common::UString::sprintf("%.6f", strct.getDouble(field)));
			break;

		case Aurora::GFFStruct::kFieldTypeExoString:
		case Aurora::GFFStruct::kFieldTypeResRef:
		case Aurora::GFFStruct::kFieldTypeStrRef:
			_xml->setContents(strct.getString(field));
			break;

		case Aurora::GFFStruct::kFieldTypeLocString:
			{
				Aurora::LocString locString;

				strct.getLocString(field, locString);
				_xml->addProperty("strref", Common::UString::sprintf("%u", locString.getID()));
				_xml->breakLine();

				dumpLocString(locString);
			}
			break;

		case Aurora::GFFStruct::kFieldTypeVoid:
			_xml->setContents(*strct.getData(field));
			break;

		case Aurora::GFFStruct::kFieldTypeStruct:
			dumpStruct(strct.getStruct(field), label);
			break;

		case Aurora::GFFStruct::kFieldTypeList:
			_xml->breakLine();
			dumpList(strct.getList(field));
			break;

		case Aurora::GFFStruct::kFieldTypeOrientation:
			{
				double a, b, c, d;

				strct.getOrientation(field, a, b, c, d);

				_xml->breakLine();

				_xml->openTag("double");
				_xml->setContents(Common::UString::sprintf("%.6f", a));
				_xml->closeTag();
				_xml->breakLine();

				_xml->openTag("double");
				_xml->setContents(Common::UString::sprintf("%.6f", b));
				_xml->closeTag();
				_xml->breakLine();

				_xml->openTag("double");
				_xml->setContents(Common::UString::sprintf("%.6f", c));
				_xml->closeTag();
				_xml->breakLine();

				_xml->openTag("double");
				_xml->setContents(Common::UString::sprintf("%.6f", d));
				_xml->closeTag();
				_xml->breakLine();
			}
			break;

		case Aurora::GFFStruct::kFieldTypeVector:
			{
				double x, y, z;

				strct.getVector(field, x, y, z);

				_xml->breakLine();

				_xml->openTag("double");
				_xml->setContents(Common::UString::sprintf("%.6f", x));
				_xml->closeTag();
				_xml->breakLine();

				_xml->openTag("double");
				_xml->setContents(Common::UString::sprintf("%.6f", y));
				_xml->closeTag();
				_xml->breakLine();

				_xml->openTag("double");
				_xml->setContents(Common::UString::sprintf("%.6f", z));
				_xml->closeTag();
				_xml->breakLine();
			}
			break;

		default:
			break;
	}

	// Structs already close their own tag
	if (type != Aurora::GFFStruct::kFieldTypeStruct) {
		_xml->closeTag();
		_xml->breakLine();
	}
}

void GFF3Dumper::dumpStruct(const Aurora::GFFStruct &strct, const Common::UString &label) {
	_xml->openTag("struct");
	_xml->addProperty("label", label);
	_xml->addProperty("id", Common::UString::sprintf("%u", strct.getID()));
	_xml->breakLine();

	for (Aurora::GFFStruct::iterator f = strct.begin(); f != strct.end(); ++f)
		dumpField(strct, *f);

	_xml->closeTag();
	_xml->breakLine();
}

void GFF3Dumper::dumpList(const Aurora::GFFList &list) {
	for (Aurora::GFFList::const_iterator e = list.begin(); e != list.end(); ++e)
		dumpStruct(**e);
}

} // End of namespace XML
