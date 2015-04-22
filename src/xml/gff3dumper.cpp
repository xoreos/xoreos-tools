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
 *  Dump GFF V3.2/V3.3 into XML files.
 */

#include "src/common/stream.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"

#include "src/aurora/locstring.h"
#include "src/aurora/gff3file.h"

#include "src/xml/xmlwriter.h"
#include "src/xml/gff3dumper.h"

namespace XML {

GFF3Dumper::GFF3Dumper() : _gff3(0), _xml(0) {
}

GFF3Dumper::~GFF3Dumper() {
	clear();
}

void GFF3Dumper::clear() {
	delete _gff3;
	delete _xml;

	_gff3 = 0;
	_xml  = 0;
}

void GFF3Dumper::dump(Common::WriteStream &output, Common::SeekableReadStream &input,
                      Common::Encoding UNUSED(encoding)) {

	try {
		_gff3 = new Aurora::GFF3File(input, 0xFFFFFFFF);
		_xml  = new XMLWriter(output);

		_xml->openTag("gff3");
		_xml->addProperty("type", Common::tagToString(_gff3->getType(), true));
		_xml->breakLine();

		dumpStruct(_gff3->getTopLevel());

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

void GFF3Dumper::dumpField(const Aurora::GFF3Struct &strct, const Common::UString &field) {
	Aurora::GFF3Struct::FieldType type = strct.getType(field);

	Common::UString typeName;
	if ((type >= 0) && (type < ARRAYSIZE(kGFF3FieldTypeNames)))
		typeName = kGFF3FieldTypeNames[(int)type];
	else
		typeName = Common::UString::sprintf("fieldtype%d", (int)type);

	Common::UString label = field;

	// Structs already open their own tag
	if (type != Aurora::GFF3Struct::kFieldTypeStruct) {
		_xml->openTag(typeName);
		_xml->addProperty("label", label);
	}

	switch (type) {
		case Aurora::GFF3Struct::kFieldTypeChar:
			_xml->setContents(Common::UString::sprintf("%c", strct.getChar(field)));
			break;

		case Aurora::GFF3Struct::kFieldTypeByte:
		case Aurora::GFF3Struct::kFieldTypeUint16:
		case Aurora::GFF3Struct::kFieldTypeUint32:
		case Aurora::GFF3Struct::kFieldTypeUint64:
			_xml->setContents(Common::UString::sprintf("%"PRIu64, Cu64(strct.getUint(field))));
			break;

		case Aurora::GFF3Struct::kFieldTypeSint16:
		case Aurora::GFF3Struct::kFieldTypeSint32:
		case Aurora::GFF3Struct::kFieldTypeSint64:
			_xml->setContents(Common::UString::sprintf("%"PRId64, Cd64(strct.getSint(field))));
			break;

		case Aurora::GFF3Struct::kFieldTypeFloat:
		case Aurora::GFF3Struct::kFieldTypeDouble:
			_xml->setContents(Common::UString::sprintf("%.6f", strct.getDouble(field)));
			break;

		case Aurora::GFF3Struct::kFieldTypeExoString:
		case Aurora::GFF3Struct::kFieldTypeResRef:
		case Aurora::GFF3Struct::kFieldTypeStrRef:
			_xml->setContents(strct.getString(field));
			break;

		case Aurora::GFF3Struct::kFieldTypeLocString:
			{
				Aurora::LocString locString;

				strct.getLocString(field, locString);
				_xml->addProperty("strref", Common::UString::sprintf("%u", locString.getID()));
				_xml->breakLine();

				dumpLocString(locString);
			}
			break;

		case Aurora::GFF3Struct::kFieldTypeVoid:
			_xml->setContents(*strct.getData(field));
			break;

		case Aurora::GFF3Struct::kFieldTypeStruct:
			dumpStruct(strct.getStruct(field), label);
			break;

		case Aurora::GFF3Struct::kFieldTypeList:
			_xml->breakLine();
			dumpList(strct.getList(field));
			break;

		case Aurora::GFF3Struct::kFieldTypeOrientation:
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

		case Aurora::GFF3Struct::kFieldTypeVector:
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
	if (type != Aurora::GFF3Struct::kFieldTypeStruct) {
		_xml->closeTag();
		_xml->breakLine();
	}
}

void GFF3Dumper::dumpStruct(const Aurora::GFF3Struct &strct, const Common::UString &label) {
	_xml->openTag("struct");
	_xml->addProperty("label", label);
	_xml->addProperty("id", Common::UString::sprintf("%u", strct.getID()));
	_xml->breakLine();

	for (Aurora::GFF3Struct::iterator f = strct.begin(); f != strct.end(); ++f)
		dumpField(strct, *f);

	_xml->closeTag();
	_xml->breakLine();
}

void GFF3Dumper::dumpList(const Aurora::GFF3List &list) {
	for (Aurora::GFF3List::const_iterator e = list.begin(); e != list.end(); ++e)
		dumpStruct(**e);
}

} // End of namespace XML
