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

#include <boost/scope_exit.hpp>

#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/readstream.h"
#include "src/common/writestream.h"

#include "src/aurora/locstring.h"
#include "src/aurora/sacfile.h"
#include "src/aurora/gff3file.h"

#include "src/xml/xmlwriter.h"
#include "src/xml/gff3dumper.h"

namespace XML {

GFF3Dumper::GFF3Dumper(bool sacFile) : _sacFile(sacFile) {
}

GFF3Dumper::~GFF3Dumper() {
}

void GFF3Dumper::dump(Common::WriteStream &output, Common::SeekableReadStream *input,
                      Common::Encoding UNUSED(encoding), bool allowNWNPremium) {

	BOOST_SCOPE_EXIT( (&_gff3) (&_xml) ) {
		_gff3.reset();
		_xml.reset();
	} BOOST_SCOPE_EXIT_END

	if (_sacFile) {
		_gff3 = std::make_unique<Aurora::SACFile>(input);
	} else {
		_gff3 = std::make_unique<Aurora::GFF3File>(input, 0xFFFFFFFF, allowNWNPremium);
	}

	_xml = std::make_unique<XMLWriter>(output);

	_xml->openTag("gff3");
	_xml->addProperty("type", Common::tagToString(_gff3->getType(), true));
	_xml->breakLine();

	dumpStruct(_gff3->getTopLevel());

	_xml->closeTag();
	_xml->breakLine();

	_xml->flush();
}

void GFF3Dumper::dumpLocString(const Aurora::LocString &locString) {
	std::vector<Aurora::LocString::SubLocString> str;
	locString.getStrings(str);

	if (!str.empty())
		_xml->breakLine();

	for (std::vector<Aurora::LocString::SubLocString>::iterator s = str.begin(); s != str.end(); ++s) {
		_xml->openTag("string");
		_xml->addProperty("language", Common::composeString(s->language));

		_xml->setContents(s->str);
		_xml->closeTag();
		_xml->breakLine();
	}
}

static const char * const kGFF3FieldTypeNames[] = {
	"byte",
	"char",
	"uint16_t",
	"sint16",
	"uint32_t",
	"sint32",
	"uint64_t",
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
	Aurora::GFF3Struct::FieldType type = strct.getFieldType(field);

	Common::UString typeName;
	if (((size_t) type) < ARRAYSIZE(kGFF3FieldTypeNames))
		typeName = kGFF3FieldTypeNames[(int)type];
	else
		typeName = "filetype" + Common::composeString((uint64_t) type);

	Common::UString label = field;

	// Structs already open their own tag
	if (type != Aurora::GFF3Struct::kFieldTypeStruct) {
		_xml->openTag(typeName);
		_xml->addProperty("label", label);
	}

	switch (type) {
		case Aurora::GFF3Struct::kFieldTypeChar:
			_xml->setContents(Common::composeString(strct.getSint(field)));
			break;

		case Aurora::GFF3Struct::kFieldTypeByte:
		case Aurora::GFF3Struct::kFieldTypeUint16:
		case Aurora::GFF3Struct::kFieldTypeUint32:
		case Aurora::GFF3Struct::kFieldTypeUint64:
			_xml->setContents(Common::composeString(strct.getUint(field)));
			break;

		case Aurora::GFF3Struct::kFieldTypeSint16:
		case Aurora::GFF3Struct::kFieldTypeSint32:
		case Aurora::GFF3Struct::kFieldTypeSint64:
			_xml->setContents(Common::composeString(strct.getSint(field)));
			break;

		case Aurora::GFF3Struct::kFieldTypeFloat:
		case Aurora::GFF3Struct::kFieldTypeDouble:
			_xml->setContents(Common::UString::format("%.6f", strct.getDouble(field)));
			break;

		case Aurora::GFF3Struct::kFieldTypeStrRef:
			_xml->setContents(strct.getString(field));
			break;

		case Aurora::GFF3Struct::kFieldTypeExoString:
		case Aurora::GFF3Struct::kFieldTypeResRef:
			try {
				_xml->setContents(strct.getString(field));
			} catch (...) {
				_xml->addProperty("base64", "true");

				std::unique_ptr<Common::SeekableReadStream> data(strct.getData(field));
				_xml->setContents(*data);
			}
			break;

		case Aurora::GFF3Struct::kFieldTypeLocString:
			{
				Aurora::LocString locString;

				strct.getLocString(field, locString);
				_xml->addProperty("strref", Common::composeString(locString.getID()));

				dumpLocString(locString);
			}
			break;

		case Aurora::GFF3Struct::kFieldTypeVoid:
			{
				std::unique_ptr<Common::SeekableReadStream> data(strct.getData(field));
				_xml->setContents(*data);
			}
			break;

		case Aurora::GFF3Struct::kFieldTypeStruct:
			dumpStruct(strct.getStruct(field), label);
			break;

		case Aurora::GFF3Struct::kFieldTypeList:
			dumpList(strct.getList(field));
			break;

		case Aurora::GFF3Struct::kFieldTypeOrientation:
			{
				double a = 0.0, b = 0.0, c = 0.0, d = 0.0;

				strct.getOrientation(field, a, b, c, d);

				_xml->breakLine();

				_xml->openTag("double");
				_xml->setContents(Common::UString::format("%.6f", a));
				_xml->closeTag();
				_xml->breakLine();

				_xml->openTag("double");
				_xml->setContents(Common::UString::format("%.6f", b));
				_xml->closeTag();
				_xml->breakLine();

				_xml->openTag("double");
				_xml->setContents(Common::UString::format("%.6f", c));
				_xml->closeTag();
				_xml->breakLine();

				_xml->openTag("double");
				_xml->setContents(Common::UString::format("%.6f", d));
				_xml->closeTag();
				_xml->breakLine();
			}
			break;

		case Aurora::GFF3Struct::kFieldTypeVector:
			{
				double x = 0.0, y = 0.0, z = 0.0;

				strct.getVector(field, x, y, z);

				_xml->breakLine();

				_xml->openTag("double");
				_xml->setContents(Common::UString::format("%.6f", x));
				_xml->closeTag();
				_xml->breakLine();

				_xml->openTag("double");
				_xml->setContents(Common::UString::format("%.6f", y));
				_xml->closeTag();
				_xml->breakLine();

				_xml->openTag("double");
				_xml->setContents(Common::UString::format("%.6f", z));
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
	dumpStruct(strct, true, label);
}

void GFF3Dumper::dumpStruct(const Aurora::GFF3Struct &strct) {
	dumpStruct(strct, false);
}

void GFF3Dumper::dumpStruct(const Aurora::GFF3Struct &strct, bool hasLabel, const Common::UString &label) {
	_xml->openTag("struct");
	if (hasLabel)
		_xml->addProperty("label", label);
	_xml->addProperty("id", Common::composeString(strct.getID()));

	if (strct.getFieldCount() > 0)
		_xml->breakLine();

	const std::vector<Common::UString> &fields = strct.getFieldNames();

	for (std::vector<Common::UString>::const_iterator f = fields.begin(); f != fields.end(); ++f)
		dumpField(strct, *f);

	_xml->closeTag();
	_xml->breakLine();
}

void GFF3Dumper::dumpList(const Aurora::GFF3List &list) {
	if (!list.empty())
		_xml->breakLine();

	for (Aurora::GFF3List::const_iterator e = list.begin(); e != list.end(); ++e)
		dumpStruct(**e);
}

} // End of namespace XML
