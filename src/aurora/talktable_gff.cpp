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
 *  Handling BioWare's GFF'd talk tables.
 */

/* See the TLK description on the Dragon Age toolset wiki
 * (<http://social.bioware.com/wiki/datoolset/index.php/TLK>).
 */

#include <cassert>

#include "src/common/util.h"
#include "src/common/error.h"
#include "src/common/readstream.h"

#include "src/aurora/talktable_gff.h"
#include "src/aurora/gff4file.h"

static const uint32 kTLKID     = MKTAG('T', 'L', 'K', ' ');
static const uint32 kVersion02 = MKTAG('V', '0', '.', '2');
static const uint32 kVersion05 = MKTAG('V', '0', '.', '5');

namespace Aurora {

TalkTable_GFF::TalkTable_GFF(Common::SeekableReadStream *tlk, Common::Encoding encoding) :
	TalkTable(encoding), _gff(0) {

	if (_encoding == Common::kEncodingInvalid)
		_encoding = Common::kEncodingUTF16LE;

	load(tlk);
}

TalkTable_GFF::~TalkTable_GFF() {
	clean();
}

void TalkTable_GFF::clean() {
	for (Entries::iterator e = _entries.begin(); e != _entries.end(); ++e)
		delete e->second;

	delete _gff;
}

const std::list<uint32> &TalkTable_GFF::getStrRefs() const {
	return _strRefs;
}

bool TalkTable_GFF::getString(uint32 strRef, Common::UString &string, Common::UString &soundResRef) const {
	Entries::const_iterator e = _entries.find(strRef);
	if (e == _entries.end())
		return false;

	string      = readString(*e->second);
	soundResRef = "";

	return true;
}

bool TalkTable_GFF::getEntry(uint32 strRef, Common::UString &string, Common::UString &soundResRef,
                             uint32 &volumeVariance, uint32 &pitchVariance, float &soundLength,
                             uint32 &soundID) const {

	Entries::const_iterator e = _entries.find(strRef);
	if (e == _entries.end())
		return false;

	string      = readString(*e->second);
	soundResRef = "";

	volumeVariance = 0;
	pitchVariance  = 0;
	soundLength    = -1.0f;

	soundID = 0xFFFFFFFF;

	return true;
}

void TalkTable_GFF::setEntry(uint32 strRef, const Common::UString &string,
                             const Common::UString &UNUSED(soundResRef),
                             uint32 UNUSED(volumeVariance), uint32 UNUSED(pitchVariance),
                             float UNUSED(soundLength), uint32 UNUSED(soundID)) {

	Entries::iterator entry = _entries.find(strRef);
	if (entry == _entries.end()) {
		std::pair<Entries::iterator, bool> result = _entries.insert(std::make_pair(strRef, new Entry));
		entry = result.first;

		_strRefs.push_back(strRef);
	}

	entry->second->text = string;
}

void TalkTable_GFF::load(Common::SeekableReadStream *tlk) {
	assert(tlk);

	try {
		_gff = new GFF4File(tlk, kTLKID);

		const GFF4Struct &top = _gff->getTopLevel();

		if      (_gff->getTypeVersion() == kVersion02)
			load02(top);
		else if (_gff->getTypeVersion() == kVersion05)
			load05(top);
		else
			throw Common::Exception("Unsupported GFF TLK file version %08X", _gff->getTypeVersion());

		_strRefs.sort();

	} catch (Common::Exception &e) {
		clean();

		e.add("Unable to load GFF TLK");
		throw;
	}
}

void TalkTable_GFF::load02(const GFF4Struct &top) {
	if (!top.hasField(kGFF4TalkStringList))
		return;

	const GFF4List &strings = top.getList(kGFF4TalkStringList);

	for (GFF4List::const_iterator s = strings.begin(); s != strings.end(); ++s) {
		if (!*s)
			continue;

		uint32 strRef = (*s)->getUint(kGFF4TalkStringID, 0xFFFFFFFF);
		if (strRef == 0xFFFFFFFF)
			continue;

		Entry *entry = new Entry(*s);

		std::pair<Entries::iterator, bool> result = _entries.insert(std::make_pair(strRef, entry));
		if (!result.second)
			delete entry;

		_strRefs.push_back(strRef);
	}
}

void TalkTable_GFF::load05(const GFF4Struct &top) {
	if (!top.hasField(kGFF4HuffTalkStringList) ||
      !top.hasField(kGFF4HuffTalkStringHuffTree) ||
	    !top.hasField(kGFF4HuffTalkStringBitStream))
		return;

	const GFF4List &strings = top.getList(kGFF4HuffTalkStringList);

	for (GFF4List::const_iterator s = strings.begin(); s != strings.end(); ++s) {
		if (!*s)
			continue;

		uint32 strRef = (*s)->getUint(kGFF4HuffTalkStringID, 0xFFFFFFFF);
		if (strRef == 0xFFFFFFFF)
			continue;

		Entry *entry = new Entry(*s);

		std::pair<Entries::iterator, bool> result = _entries.insert(std::make_pair(strRef, entry));
		if (!result.second)
			delete entry;

		_strRefs.push_back(strRef);
	}
}

Common::UString TalkTable_GFF::readString(const Entry &entry) const {
	if (!entry.text.empty())
		return entry.text;

	if (!entry.strct)
		return "";

	if      (_gff->getTypeVersion() == kVersion02)
		return readString02(entry);
	else if (_gff->getTypeVersion() == kVersion05)
		return readString05(entry);

	return "";
}

Common::UString TalkTable_GFF::readString02(const Entry &entry) const {
	if (_encoding == Common::kEncodingInvalid)
		return "[???]";

	return entry.strct->getString(kGFF4TalkString, _encoding);
}

Common::UString TalkTable_GFF::readString05(const Entry &entry) const {
	Common::SeekableReadStream *huffTree  = _gff->getTopLevel().getData(kGFF4HuffTalkStringHuffTree);
	Common::SeekableReadStream *bitStream = _gff->getTopLevel().getData(kGFF4HuffTalkStringBitStream);

	Common::UString str = readString05(huffTree, bitStream, entry);

	delete huffTree;
	delete bitStream;

	return str;
}

Common::UString TalkTable_GFF::readString05(Common::SeekableReadStream *huffTree,
                                            Common::SeekableReadStream *bitStream,
                                            const Entry &entry) const {
	if (!huffTree || !bitStream)
		return "";

	/* Read a string encoded in a Huffman'd bitstream.
	 *
	 * The Huffman tree itself is made up of signed 32bit nodes:
	 *  - Positive values are internal nodes, encoding a child index
	 *  - Negative values are leaf nodes, encoding an UTF-16 character value
	 *
	 * Kudos to Rick (gibbed) (<http://gib.me/>).
	 */

	std::vector<uint16> utf16Str;

	const uint32 startOffset = entry.strct->getUint(kGFF4HuffTalkStringBitOffset);

	uint32 index = startOffset >> 5;
	uint32 shift = startOffset & 0x1F;

	do {
		ptrdiff_t e = (huffTree->size() / 8) - 1;

		while (e >= 0) {
			bitStream->seek(index * 4);
			const ptrdiff_t offset = (bitStream->readUint32LE() >> shift) & 1;

			huffTree->seek(((e * 2) + offset) * 4);
			e = huffTree->readSint32LE();

			shift++;
			index += (shift >> 5);

			shift %= 32;
		}

		utf16Str.push_back(TO_LE_16(0xFFFF - e));

	} while (utf16Str.back() != 0);

	const byte  *data = reinterpret_cast<const byte *>(&utf16Str[0]);
	const size_t size = utf16Str.size() * 2;

	return Common::readString(data, size, Common::kEncodingUTF16LE);
}

} // End of namespace Aurora
