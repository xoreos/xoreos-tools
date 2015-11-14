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

#ifndef XML_GFF3DUMPER_H
#define XML_GFF3DUMPER_H

#include "src/common/ustring.h"

#include "src/aurora/types.h"

#include "src/xml/gffdumper.h"

namespace Aurora {
	class LocString;
}

namespace XML {

class XMLWriter;

/** Dump GFF V3.2/V3.3 into XML files. */
class GFF3Dumper : public GFFDumper {
public:
	GFF3Dumper();
	~GFF3Dumper();

	/** Dump the GFF into XML. */
	void dump(Common::WriteStream &output, Common::SeekableReadStream *input,
	          Common::Encoding encoding, bool allowNWNPremium = false);

private:
	Aurora::GFF3File *_gff3;
	XMLWriter *_xml;

	void dumpLocString(const Aurora::LocString &locString);
	void dumpField(const Aurora::GFF3Struct &strct, const Common::UString &field);
	void dumpStruct(const Aurora::GFF3Struct &strct, const Common::UString &label = "");
	void dumpList(const Aurora::GFF3List &list);

	void clear();
};

} // End of namespace XML

#endif // XML_GFF3DUMPER_H
