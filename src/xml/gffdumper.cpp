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

#include "src/common/util.h"
#include "src/common/error.h"
#include "src/common/readstream.h"

#include "src/aurora/types.h"

#include "src/xml/gffdumper.h"
#include "src/xml/gff3dumper.h"
#include "src/xml/gff4dumper.h"

struct GFFType {
	Aurora::FileType type;
	uint32 id;
};

static const GFFType kGFFTypes[] = {
	{Aurora::kFileTypeRES , MKTAG('R', 'E', 'S', ' ')},
	{Aurora::kFileTypeARE , MKTAG('A', 'R', 'E', ' ')},
	{Aurora::kFileTypeIFO , MKTAG('I', 'F', 'O', ' ')},
	{Aurora::kFileTypeBIC , MKTAG('B', 'I', 'C', ' ')},
	{Aurora::kFileTypeGIT , MKTAG('G', 'I', 'T', ' ')},
	{Aurora::kFileTypeBTI , MKTAG('B', 'T', 'I', ' ')},
	{Aurora::kFileTypeUTI , MKTAG('U', 'T', 'I', ' ')},
	{Aurora::kFileTypeBTC , MKTAG('B', 'T', 'C', ' ')},
	{Aurora::kFileTypeUTC , MKTAG('U', 'T', 'C', ' ')},
	{Aurora::kFileTypeDLG , MKTAG('D', 'L', 'G', ' ')},
	{Aurora::kFileTypeITP , MKTAG('I', 'T', 'P', ' ')},
	{Aurora::kFileTypeBTT , MKTAG('B', 'T', 'T', ' ')},
	{Aurora::kFileTypeUTT , MKTAG('U', 'T', 'T', ' ')},
	{Aurora::kFileTypeBTS , MKTAG('B', 'T', 'S', ' ')},
	{Aurora::kFileTypeUTS , MKTAG('U', 'T', 'S', ' ')},
	{Aurora::kFileTypeGFF , MKTAG('G', 'F', 'F', ' ')},
	{Aurora::kFileTypeFAC , MKTAG('F', 'A', 'C', ' ')},
	{Aurora::kFileTypeBTE , MKTAG('B', 'T', 'E', ' ')},
	{Aurora::kFileTypeUTE , MKTAG('U', 'T', 'E', ' ')},
	{Aurora::kFileTypeBTD , MKTAG('B', 'T', 'D', ' ')},
	{Aurora::kFileTypeUTD , MKTAG('U', 'T', 'D', ' ')},
	{Aurora::kFileTypeBTP , MKTAG('B', 'T', 'P', ' ')},
	{Aurora::kFileTypeUTP , MKTAG('U', 'T', 'P', ' ')},
	{Aurora::kFileTypeGIC , MKTAG('G', 'I', 'C', ' ')},
	{Aurora::kFileTypeGUI , MKTAG('G', 'U', 'I', ' ')},
	{Aurora::kFileTypeBTM , MKTAG('B', 'T', 'M', ' ')},
	{Aurora::kFileTypeUTM , MKTAG('U', 'T', 'M', ' ')},
	{Aurora::kFileTypeBTG , MKTAG('B', 'T', 'G', ' ')},
	{Aurora::kFileTypeUTG , MKTAG('U', 'T', 'G', ' ')},
	{Aurora::kFileTypeJRL , MKTAG('J', 'R', 'L', ' ')},
	{Aurora::kFileTypeUTW , MKTAG('U', 'T', 'W', ' ')},
	{Aurora::kFileTypePTM , MKTAG('P', 'T', 'M', ' ')},
	{Aurora::kFileTypePTT , MKTAG('P', 'T', 'T', ' ')},
	{Aurora::kFileTypeCUT , MKTAG('C', 'U', 'T', ' ')},
	{Aurora::kFileTypeQDB , MKTAG('Q', 'D', 'B', ' ')},
	{Aurora::kFileTypeQST , MKTAG('Q', 'S', 'T', ' ')},
	{Aurora::kFileTypePTH , MKTAG('P', 'T', 'H', ' ')},
	{Aurora::kFileTypeQST2, MKTAG('Q', 'S', 'T', ' ')},
	{Aurora::kFileTypeSTO , MKTAG('S', 'T', 'O', ' ')},
	{Aurora::kFileTypeUTR , MKTAG('U', 'T', 'R', ' ')},
	{Aurora::kFileTypeULT , MKTAG('U', 'L', 'T', ' ')},
	{Aurora::kFileTypeGDA , MKTAG('G', 'D', 'A', ' ')},
	{Aurora::kFileTypeCRE , MKTAG('C', 'R', 'E', ' ')},
	{Aurora::kFileTypeCAM , MKTAG('C', 'A', 'M', ' ')},
	{Aurora::kFileTypeFSM , MKTAG('F', 'S', 'M', ' ')},
	{Aurora::kFileTypePLA , MKTAG('P', 'L', 'A', ' ')},
	{Aurora::kFileTypeTRG , MKTAG('T', 'R', 'G', ' ')},
	{Aurora::kFileTypeWMP , MKTAG('W', 'M', 'P', ' ')},
	{Aurora::kFileTypeMMD , MKTAG('M', 'M', 'D', ' ')}
};

static const uint32 kVersion32 = MKTAG('V', '3', '.', '2');
static const uint32 kVersion33 = MKTAG('V', '3', '.', '3');
static const uint32 kVersion40 = MKTAG('V', '4', '.', '0');
static const uint32 kVersion41 = MKTAG('V', '4', '.', '1');

namespace XML {

GFFDumper::GFFDumper() {
}

GFFDumper::~GFFDumper() {
}

static int identifyGFF(Common::SeekableReadStream &input, uint32 &version) {
	try {
		size_t pos = input.pos();

		uint32 id;
		id      = input.readUint32BE();
		version = input.readUint32BE();

		input.seek(pos);

		for (size_t i = 0; i < ARRAYSIZE(kGFFTypes); i++)
			if (kGFFTypes[i].id == id)
				return i;
	} catch (...) {
	}

	return -1;
}

GFFDumper *GFFDumper::identify(Common::SeekableReadStream &input) {
	uint32 version;
	int type = identifyGFF(input, version);
	if (type < 0)
		throw Common::Exception("Not a GFF file");

	if      ((version == kVersion32) || (version == kVersion33))
		return new GFF3Dumper();
	else if ((version == kVersion40) || (version == kVersion41))
		return new GFF4Dumper();
	else
		throw Common::Exception("Invalid GFF version 0x%08X", version);
}

} // End of namespace XML
