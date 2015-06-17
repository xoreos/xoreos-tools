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
 *  Handling BioWare's TLK talk tables.
 */

/* See BioWare's own specs released for Neverwinter Nights modding
 * (<https://github.com/xoreos/xoreos-docs/tree/master/specs/bioware>)
 */

#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/memreadstream.h"
#include "src/common/readfile.h"
#include "src/common/error.h"

#include "src/aurora/talktable_tlk.h"
#include "src/aurora/language.h"

static const uint32 kTLKID    = MKTAG('T', 'L', 'K', ' ');
static const uint32 kVersion3 = MKTAG('V', '3', '.', '0');
static const uint32 kVersion4 = MKTAG('V', '4', '.', '0');

namespace Aurora {

TalkTable_TLK::TalkTable_TLK(Common::SeekableReadStream &tlk, Common::Encoding encoding) :
	TalkTable(encoding), _tlk(&tlk) {

	load();
}

TalkTable_TLK::~TalkTable_TLK() {
}

void TalkTable_TLK::load() {
	try {
		readHeader(*_tlk);

		if (_id != kTLKID)
			throw Common::Exception("Not a TLK file (%s)", Common::debugTag(_id).c_str());

		if (_version != kVersion3 && _version != kVersion4)
			throw Common::Exception("Unsupported TLK file version %s", Common::debugTag(_version).c_str());

		_languageID = _tlk->readUint32LE();

		uint32 stringCount = _tlk->readUint32LE();
		_entries.resize(stringCount);

		// V4 added this field; it's right after the header in V3
		uint32 tableOffset = 20;
		if (_version == kVersion4)
			tableOffset = _tlk->readUint32LE();

		_stringsOffset = _tlk->readUint32LE();

		// Go to the table
		_tlk->seek(tableOffset);

		// Read in all the table data
		if (_version == kVersion3)
			readEntryTableV3();
		else
			readEntryTableV4();

	} catch (Common::Exception &e) {
		e.add("Failed reading TLK file");
		throw;
	}
}

void TalkTable_TLK::readEntryTableV3() {
	for (size_t i = 0; i < _entries.size(); i++) {
		Entry &entry = _entries[i];

		entry.flags          = _tlk->readUint32LE();
		entry.soundResRef    = Common::readStringFixed(*_tlk, Common::kEncodingASCII, 16);
		entry.volumeVariance = _tlk->readUint32LE();
		entry.pitchVariance  = _tlk->readUint32LE();
		entry.offset         = _tlk->readUint32LE() + _stringsOffset;
		entry.length         = _tlk->readUint32LE();
		entry.soundLength    = _tlk->readIEEEFloatLE();

		if ((entry.length > 0) && (entry.flags & kFlagTextPresent))
			_strRefs.push_back(i);
	}
}

void TalkTable_TLK::readEntryTableV4() {
	for (size_t i = 0; i < _entries.size(); i++) {
		Entry &entry = _entries[i];

		entry.soundID = _tlk->readUint32LE();
		entry.offset  = _tlk->readUint32LE();
		entry.length  = _tlk->readUint16LE();
		entry.flags   = kFlagTextPresent;

		if ((entry.length > 0) && (entry.flags & kFlagTextPresent))
			_strRefs.push_back(i);
	}
}

Common::UString TalkTable_TLK::readString(const Entry &entry) const {
	if ((entry.length == 0) || !(entry.flags & kFlagTextPresent))
		return "";

	if (_encoding == Common::kEncodingInvalid)
		return "";

	_tlk->seek(entry.offset);

	size_t length = MIN<size_t>(entry.length, _tlk->size() - _tlk->pos());
	if (length == 0)
		return "";

	Common::MemoryReadStream *data   = _tlk->readStream(length);
	Common::MemoryReadStream *parsed = LangMan.preParseColorCodes(*data);

	Common::UString str = Common::readString(*parsed, _encoding);

	delete parsed;
	delete data;

	return str;
}

uint32 TalkTable_TLK::getLanguageID() const {
	return _languageID;
}

const std::list<uint32> &TalkTable_TLK::getStrRefs() const {
	return _strRefs;
}

bool TalkTable_TLK::getString(uint32 strRef, Common::UString &string, Common::UString &soundResRef) const {
	if (strRef >= _entries.size())
		return false;

	string      = readString(_entries[strRef]);
	soundResRef = _entries[strRef].soundResRef;

	return true;
}

uint32 TalkTable_TLK::getLanguageID(Common::SeekableReadStream &tlk) {
	uint32 id, version;
	bool utf16le;

	AuroraBase::readHeader(tlk, id, version, utf16le);

	if ((id != kTLKID) || ((version != kVersion3) && (version != kVersion4)))
		return kLanguageInvalid;

	return tlk.readUint32LE();
}

uint32 TalkTable_TLK::getLanguageID(const Common::UString &file) {
	Common::ReadFile tlk;
	if (!tlk.open(file))
		return kLanguageInvalid;

	return getLanguageID(tlk);
}

} // End of namespace Aurora
