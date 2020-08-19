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
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/readstream.h"

#include "src/aurora/types.h"

#include "src/xml/gffdumper.h"
#include "src/xml/gff3dumper.h"
#include "src/xml/gff4dumper.h"

static const uint32_t kGFFTypes[] = {
	MKTAG('A', 'R', 'E', ' '),
	MKTAG('B', 'I', 'C', ' '),
	MKTAG('B', 'T', 'C', ' '),
	MKTAG('B', 'T', 'D', ' '),
	MKTAG('B', 'T', 'E', ' '),
	MKTAG('B', 'T', 'G', ' '),
	MKTAG('B', 'T', 'I', ' '),
	MKTAG('B', 'T', 'M', ' '),
	MKTAG('B', 'T', 'P', ' '),
	MKTAG('B', 'T', 'S', ' '),
	MKTAG('B', 'T', 'T', ' '),
	MKTAG('C', 'A', 'M', ' '),
	MKTAG('C', 'R', 'E', ' '),
	MKTAG('C', 'U', 'T', ' '),
	MKTAG('C', 'W', 'A', ' '),
	MKTAG('D', 'L', 'G', ' '),
	MKTAG('F', 'A', 'C', ' '),
	MKTAG('F', 'S', 'M', ' '),
	MKTAG('G', 'D', 'A', ' '),
	MKTAG('G', 'F', 'F', ' '),
	MKTAG('G', 'I', 'C', ' '),
	MKTAG('G', 'I', 'T', ' '),
	MKTAG('G', 'U', 'I', ' '),
	MKTAG('G', 'V', 'T', ' '),
	MKTAG('I', 'F', 'O', ' '),
	MKTAG('I', 'T', 'P', ' '),
	MKTAG('J', 'R', 'L', ' '),
	MKTAG('M', 'M', 'D', ' '),
	MKTAG('M', 'S', 'T', ' '),
	MKTAG('N', 'F', 'O', ' '),
	MKTAG('P', 'L', 'A', ' '),
	MKTAG('P', 'T', ' ', ' '),
	MKTAG('P', 'T', 'H', ' '),
	MKTAG('P', 'T', 'M', ' '),
	MKTAG('P', 'T', 'T', ' '),
	MKTAG('Q', 'D', 'B', ' '),
	MKTAG('Q', 'S', 'T', ' '),
	MKTAG('R', 'E', 'S', ' '),
	MKTAG('R', 'S', 'T', ' '),
	MKTAG('S', 'A', 'V', ' '),
	MKTAG('S', 'N', 'P', ' '),
	MKTAG('S', 'T', 'O', ' '),
	MKTAG('T', 'R', 'G', ' '),
	MKTAG('U', 'E', 'N', ' '),
	MKTAG('U', 'L', 'T', ' '),
	MKTAG('U', 'P', 'E', ' '),
	MKTAG('U', 'S', 'C', ' '),
	MKTAG('U', 'T', 'A', ' '),
	MKTAG('U', 'T', 'C', ' '),
	MKTAG('U', 'T', 'D', ' '),
	MKTAG('U', 'T', 'E', ' '),
	MKTAG('U', 'T', 'G', ' '),
	MKTAG('U', 'T', 'I', ' '),
	MKTAG('U', 'T', 'M', ' '),
	MKTAG('U', 'T', 'P', ' '),
	MKTAG('U', 'T', 'R', ' '),
	MKTAG('U', 'T', 'S', ' '),
	MKTAG('U', 'T', 'T', ' '),
	MKTAG('U', 'T', 'W', ' '),
	MKTAG('U', 'T', 'X', ' '),
	MKTAG('W', 'M', 'P', ' ')
};

static const uint32_t kVersion32 = MKTAG('V', '3', '.', '2');
static const uint32_t kVersion33 = MKTAG('V', '3', '.', '3');
static const uint32_t kVersion40 = MKTAG('V', '4', '.', '0');
static const uint32_t kVersion41 = MKTAG('V', '4', '.', '1');

enum GFFVersion {
	kGFFVersionNone,
	kGFFVersion3,
	kGFFVersion4
};

namespace XML {

GFFDumper::GFFDumper() {
}

GFFDumper::~GFFDumper() {
}

static GFFVersion identifyGFF(Common::SeekableReadStream &input, bool allowNWNPremium, bool sacFile) {
	uint32_t id = 0xFFFFFFFF, version = 0xFFFFFFFF;

	size_t pos = input.pos();

	if (sacFile) {
		input.skip(4);
		uint32_t stringLength = input.readUint32LE();
		input.skip(stringLength);
		input.skip(4);
	}

	id      = input.readUint32BE();
	version = input.readUint32BE();

	input.seek(pos);

	GFFVersion gffVersion;
	if        ((version == kVersion32) || (version == kVersion33)) {
		gffVersion      = kGFFVersion3;
		allowNWNPremium = false;
	} else if ((version == kVersion40) || (version == kVersion41)) {
		gffVersion      = kGFFVersion4;
		allowNWNPremium = false;
	} else if (allowNWNPremium && (FROM_BE_32(id) >= 0x30) && (FROM_BE_32(id) <= 0x12F)) {
		gffVersion      = kGFFVersion3;
	} else
		throw Common::Exception("Invalid GFF %s, %s",
		                        Common::debugTag(id).c_str(), Common::debugTag(version).c_str());

	size_t foundType = 0xFFFFFFFF;
	for (size_t i = 0; i < ARRAYSIZE(kGFFTypes); i++) {
		if (kGFFTypes[i] == id) {
			foundType = i;
			break;
		}
	}

	if ((foundType == 0xFFFFFFFF) && !allowNWNPremium)
		warning("Unknown GFF type %s", Common::debugTag(id).c_str());

	return gffVersion;
}

GFFDumper *GFFDumper::identify(Common::SeekableReadStream &input, bool allowNWNPremium, bool sacFile) {
	const GFFVersion version = identifyGFF(input, allowNWNPremium, sacFile);

	switch (version) {
		case kGFFVersion3:
			return new GFF3Dumper(sacFile);

		case kGFFVersion4:
			return new GFF4Dumper();

		default:
			throw Common::Exception("Invalid GFF version");
	}
}

} // End of namespace XML
