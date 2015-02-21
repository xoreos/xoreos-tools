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

#include "common/stream.h"
#include "common/util.h"
#include "common/error.h"

#include "aurora/locstring.h"
#include "aurora/gfffile.h"

#include "xml/xmlwriter.h"
#include "xml/gffdumper.h"

struct GFFType {
	Aurora::FileType type;
	uint32 id;
	const char *name;
};

static const GFFType kGFFTypes[] = {
	{Aurora::kFileTypeRES , MKTAG('R', 'E', 'S', ' '), "RES"},
	{Aurora::kFileTypeARE , MKTAG('A', 'R', 'E', ' '), "ARE"},
	{Aurora::kFileTypeIFO , MKTAG('I', 'F', 'O', ' '), "IFO"},
	{Aurora::kFileTypeBIC , MKTAG('B', 'I', 'C', ' '), "BIC"},
	{Aurora::kFileTypeGIT , MKTAG('G', 'I', 'T', ' '), "GIT"},
	{Aurora::kFileTypeBTI , MKTAG('B', 'T', 'I', ' '), "BTI"},
	{Aurora::kFileTypeUTI , MKTAG('U', 'T', 'I', ' '), "UTI"},
	{Aurora::kFileTypeBTC , MKTAG('B', 'T', 'C', ' '), "BTC"},
	{Aurora::kFileTypeUTC , MKTAG('U', 'T', 'C', ' '), "UTC"},
	{Aurora::kFileTypeDLG , MKTAG('D', 'L', 'G', ' '), "DLG"},
	{Aurora::kFileTypeITP , MKTAG('I', 'T', 'P', ' '), "ITP"},
	{Aurora::kFileTypeBTT , MKTAG('B', 'T', 'T', ' '), "BTT"},
	{Aurora::kFileTypeUTT , MKTAG('U', 'T', 'T', ' '), "UTT"},
	{Aurora::kFileTypeBTS , MKTAG('B', 'T', 'S', ' '), "BTS"},
	{Aurora::kFileTypeUTS , MKTAG('U', 'T', 'S', ' '), "UTS"},
	{Aurora::kFileTypeGFF , MKTAG('G', 'F', 'F', ' '), "GFF"},
	{Aurora::kFileTypeFAC , MKTAG('F', 'A', 'C', ' '), "FAC"},
	{Aurora::kFileTypeBTE , MKTAG('B', 'T', 'E', ' '), "BTE"},
	{Aurora::kFileTypeUTE , MKTAG('U', 'T', 'E', ' '), "UTE"},
	{Aurora::kFileTypeBTD , MKTAG('B', 'T', 'D', ' '), "BTD"},
	{Aurora::kFileTypeUTD , MKTAG('U', 'T', 'D', ' '), "UTD"},
	{Aurora::kFileTypeBTP , MKTAG('B', 'T', 'P', ' '), "BTP"},
	{Aurora::kFileTypeUTP , MKTAG('U', 'T', 'P', ' '), "UTP"},
	{Aurora::kFileTypeGIC , MKTAG('G', 'I', 'C', ' '), "GIC"},
	{Aurora::kFileTypeGUI , MKTAG('G', 'U', 'I', ' '), "GUI"},
	{Aurora::kFileTypeBTM , MKTAG('B', 'T', 'M', ' '), "BTM"},
	{Aurora::kFileTypeUTM , MKTAG('U', 'T', 'M', ' '), "UTM"},
	{Aurora::kFileTypeBTG , MKTAG('B', 'T', 'G', ' '), "BTG"},
	{Aurora::kFileTypeUTG , MKTAG('U', 'T', 'G', ' '), "UTG"},
	{Aurora::kFileTypeJRL , MKTAG('J', 'R', 'L', ' '), "JRL"},
	{Aurora::kFileTypeUTW , MKTAG('U', 'T', 'W', ' '), "UTW"},
	{Aurora::kFileTypePTM , MKTAG('P', 'T', 'M', ' '), "PTM"},
	{Aurora::kFileTypePTT , MKTAG('P', 'T', 'T', ' '), "PTT"},
	{Aurora::kFileTypeCUT , MKTAG('C', 'U', 'T', ' '), "CUT"},
	{Aurora::kFileTypeQDB , MKTAG('Q', 'D', 'B', ' '), "QDB"},
	{Aurora::kFileTypeQST , MKTAG('Q', 'S', 'T', ' '), "QST"},
	{Aurora::kFileTypePTH , MKTAG('P', 'T', 'H', ' '), "PTH"},
	{Aurora::kFileTypeQST2, MKTAG('Q', 'S', 'T', ' '), "QST"},
	{Aurora::kFileTypeSTO , MKTAG('S', 'T', 'O', ' '), "STO"},
	{Aurora::kFileTypeUTR , MKTAG('U', 'T', 'R', ' '), "UTR"},
	{Aurora::kFileTypeULT , MKTAG('U', 'L', 'T', ' '), "ULT"},
	{Aurora::kFileTypeGDA , MKTAG('G', 'D', 'A', ' '), "GDA"},
	{Aurora::kFileTypeCRE , MKTAG('C', 'R', 'E', ' '), "CRE"},
	{Aurora::kFileTypeCAM , MKTAG('C', 'A', 'M', ' '), "CAM"},
	{Aurora::kFileTypeFSM , MKTAG('F', 'S', 'M', ' '), "FSM"},
	{Aurora::kFileTypePLA , MKTAG('P', 'L', 'A', ' '), "PLA"},
	{Aurora::kFileTypeTRG , MKTAG('T', 'R', 'G', ' '), "TRG"},
	{Aurora::kFileTypeWMP , MKTAG('W', 'M', 'P', ' '), "WMP"},
	{Aurora::kFileTypeMMD , MKTAG('M', 'M', 'D', ' '), "MMD"}
};

namespace XML {

GFFDumper::GFFDumper() {
}

GFFDumper::~GFFDumper() {
}

void GFFDumper::dump(Common::WriteStream &output, Common::SeekableReadStream &input) {
	Aurora::FileType type = identify(input);
	if (type == Aurora::kFileTypeNone)
		throw Common::Exception("Not a GFF file");

	Aurora::GFFFile gff(input, getID(type));

	XMLWriter xmlWriter(output);

	xmlWriter.openTag("gff");
	xmlWriter.addProperty("type", getTypeName(type));
	xmlWriter.breakLine();

	dumpStruct(xmlWriter, gff.getTopLevel());

	xmlWriter.closeTag();
	xmlWriter.breakLine();

	xmlWriter.flush();
}

Aurora::FileType GFFDumper::identify(Common::SeekableReadStream &input) {
	uint32 pos = input.pos();
	uint32 id  = input.readUint32BE();
	input.seek(pos);

	for (uint i = 0; i < ARRAYSIZE(kGFFTypes); i++)
		if (kGFFTypes[i].id == id)
			return kGFFTypes[i].type;

	return Aurora::kFileTypeNone;
}

uint32 GFFDumper::getID(Aurora::FileType type) {
	for (uint i = 0; i < ARRAYSIZE(kGFFTypes); i++)
		if (kGFFTypes[i].type == type)
			return kGFFTypes[i].id;

	return 0;
}

Common::UString GFFDumper::getTypeName(Aurora::FileType type) {
	for (uint i = 0; i < ARRAYSIZE(kGFFTypes); i++)
		if (kGFFTypes[i].type == type)
			return kGFFTypes[i].name;

	return "";
}

void GFFDumper::dumpStruct(XMLWriter &xml, const Aurora::GFFStruct &strct, const Common::UString &label) {
	xml.openTag("struct");
	xml.addProperty("label", label);
	xml.addProperty("id", Common::UString::sprintf("%u", strct.getID()));
	xml.breakLine();

	for (Aurora::GFFStruct::iterator f = strct.begin(); f != strct.end(); ++f)
		dumpField(xml, strct, *f);

	xml.closeTag();
	xml.breakLine();
}

static const char *kGFFFieldTypeNames[] = {
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

void GFFDumper::dumpField(XMLWriter &xml, const Aurora::GFFStruct &strct, const Common::UString &field) {
	Aurora::GFFStruct::FieldType type = strct.getType(field);

	Common::UString typeName;
	if ((type >= 0) && (type < ARRAYSIZE(kGFFFieldTypeNames)))
		typeName = kGFFFieldTypeNames[(int)type];
	else
		typeName = Common::UString::sprintf("fieldtype%d", (int)type);

	Common::UString label = field;

	// Structs already open their own tag
	if (type != Aurora::GFFStruct::kFieldTypeStruct) {
		xml.openTag(typeName);
		xml.addProperty("label", label);
	}

	switch (type) {
		case Aurora::GFFStruct::kFieldTypeChar:
			xml.setContents(Common::UString::sprintf("%c", strct.getChar(field)));
			break;

		case Aurora::GFFStruct::kFieldTypeByte:
		case Aurora::GFFStruct::kFieldTypeUint16:
		case Aurora::GFFStruct::kFieldTypeUint32:
		case Aurora::GFFStruct::kFieldTypeUint64:
			xml.setContents(Common::UString::sprintf("%"PRIu64, Cu64(strct.getUint(field))));
			break;

		case Aurora::GFFStruct::kFieldTypeSint16:
		case Aurora::GFFStruct::kFieldTypeSint32:
		case Aurora::GFFStruct::kFieldTypeSint64:
			xml.setContents(Common::UString::sprintf("%"PRId64, Cd64(strct.getSint(field))));
			break;

		case Aurora::GFFStruct::kFieldTypeFloat:
		case Aurora::GFFStruct::kFieldTypeDouble:
			xml.setContents(Common::UString::sprintf("%.6f", strct.getDouble(field)));
			break;

		case Aurora::GFFStruct::kFieldTypeExoString:
		case Aurora::GFFStruct::kFieldTypeResRef:
		case Aurora::GFFStruct::kFieldTypeStrRef:
			xml.setContents(strct.getString(field));
			break;

		case Aurora::GFFStruct::kFieldTypeLocString:
			{
				Aurora::LocString locString;

				strct.getLocString(field, locString);
				xml.addProperty("strref", Common::UString::sprintf("%u", locString.getID()));
				xml.breakLine();

				dumpLocString(xml, locString);
			}
			break;

		case Aurora::GFFStruct::kFieldTypeVoid:
			xml.setContents(*strct.getData(field));
			break;

		case Aurora::GFFStruct::kFieldTypeStruct:
			dumpStruct(xml, strct.getStruct(field), label);
			break;

		case Aurora::GFFStruct::kFieldTypeList:
			xml.breakLine();
			dumpList(xml, strct.getList(field));
			break;

		case Aurora::GFFStruct::kFieldTypeOrientation:
			{
				double a, b, c, d;

				strct.getOrientation(field, a, b, c, d);

				xml.breakLine();

				xml.openTag("double");
				xml.setContents(Common::UString::sprintf("%.6f", a));
				xml.closeTag();
				xml.breakLine();

				xml.openTag("double");
				xml.setContents(Common::UString::sprintf("%.6f", b));
				xml.closeTag();
				xml.breakLine();

				xml.openTag("double");
				xml.setContents(Common::UString::sprintf("%.6f", c));
				xml.closeTag();
				xml.breakLine();

				xml.openTag("double");
				xml.setContents(Common::UString::sprintf("%.6f", d));
				xml.closeTag();
				xml.breakLine();
			}
			break;

		case Aurora::GFFStruct::kFieldTypeVector:
			{
				double x, y, z;

				strct.getVector(field, x, y, z);

				xml.breakLine();

				xml.openTag("double");
				xml.setContents(Common::UString::sprintf("%.6f", x));
				xml.closeTag();
				xml.breakLine();

				xml.openTag("double");
				xml.setContents(Common::UString::sprintf("%.6f", y));
				xml.closeTag();
				xml.breakLine();

				xml.openTag("double");
				xml.setContents(Common::UString::sprintf("%.6f", z));
				xml.closeTag();
				xml.breakLine();
			}
			break;

		default:
			break;
	}

	// Structs already close their own tag
	if (type != Aurora::GFFStruct::kFieldTypeStruct) {
		xml.closeTag();
		xml.breakLine();
	}
}

void GFFDumper::dumpList(XMLWriter &xml, const Aurora::GFFList &list) {
	for (Aurora::GFFList::const_iterator e = list.begin(); e != list.end(); ++e)
		dumpStruct(xml, **e);
}

void GFFDumper::dumpLocString(XMLWriter &xml, const Aurora::LocString &locString) {
	for (int i = 0; i < Aurora::LocString::kStringCount; i++) {
		Aurora::Language language;
		Common::UString string = locString.getString(i, language);

		if (!string.empty() && (language != Aurora::kLanguageInvalid)) {
			xml.openTag("string");
			xml.addProperty("language", Common::UString::sprintf("%u", (uint)language));

			xml.setContents(string);
			xml.closeTag();
			xml.breakLine();
		}
	}
}

} // End of namespace XML
