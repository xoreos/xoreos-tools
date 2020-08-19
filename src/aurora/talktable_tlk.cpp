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

#include <cassert>

#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/memreadstream.h"
#include "src/common/memwritestream.h"
#include "src/common/readfile.h"
#include "src/common/error.h"

#include "src/aurora/talktable_tlk.h"
#include "src/aurora/language.h"

static const uint32_t kTLKID    = MKTAG('T', 'L', 'K', ' ');
static const uint32_t kVersion3 = MKTAG('V', '3', '.', '0');
static const uint32_t kVersion4 = MKTAG('V', '4', '.', '0');

namespace Aurora {

TalkTable_TLK::Entry::Entry() : offset(0xFFFFFFFF), length(0xFFFFFFFF),
	flags(0), volumeVariance(0), pitchVariance(0), soundLength(-1.0f),
	soundID(0xFFFFFFFF) {

}


TalkTable_TLK::TalkTable_TLK(Common::Encoding encoding, uint32_t languageID) :
	TalkTable(encoding), _languageID(languageID) {

}

TalkTable_TLK::TalkTable_TLK(Common::SeekableReadStream *tlk, Common::Encoding encoding) :
	TalkTable(encoding), _tlk(tlk) {

	assert(_tlk);

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

		if (_encoding == Common::kEncodingInvalid)
			_encoding = LangMan.getEncoding(LangMan.getLanguage(_languageID));
		if (_encoding == Common::kEncodingInvalid)
			_encoding = Common::kEncodingCP1252;

		uint32_t stringCount = _tlk->readUint32LE();
		_entries.resize(stringCount);

		// V4 added this field; it's right after the header in V3
		uint32_t tableOffset = 20;
		if (_version == kVersion4)
			tableOffset = _tlk->readUint32LE();

		const uint32_t stringsOffset = _tlk->readUint32LE();

		// Go to the table
		_tlk->seek(tableOffset);

		// Read in all the table data
		if (_version == kVersion3)
			readEntryTableV3(stringsOffset);
		else
			readEntryTableV4();

	} catch (Common::Exception &e) {
		e.add("Failed reading TLK file");
		throw;
	}
}

void TalkTable_TLK::readEntryTableV3(uint32_t stringsOffset) {
	for (size_t i = 0; i < _entries.size(); i++) {
		Entry &entry = _entries[i];

		entry.flags          = _tlk->readUint32LE();
		entry.soundResRef    = Common::readStringFixed(*_tlk, Common::kEncodingASCII, 16);
		entry.volumeVariance = _tlk->readUint32LE();
		entry.pitchVariance  = _tlk->readUint32LE();
		entry.offset         = _tlk->readUint32LE() + stringsOffset;
		entry.length         = _tlk->readUint32LE();
		entry.soundLength    = _tlk->readIEEEFloatLE();

		if (!(entry.flags & kFlagSoundLengthPresent))
			entry.soundLength = -1.0f;

		if (((entry.length > 0) && (entry.flags & kFlagTextPresent)) || (!entry.soundResRef.empty()))
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

		if (((entry.length > 0) && (entry.flags & kFlagTextPresent)) || (entry.soundID != 0xFFFFFFFF))
			_strRefs.push_back(i);
	}
}

Common::UString TalkTable_TLK::readString(const Entry &entry) const {
	if (!_tlk || !entry.text.empty())
		return entry.text;

	if ((entry.length == 0) || !(entry.flags & kFlagTextPresent))
		return "";

	if (_encoding == Common::kEncodingInvalid)
		return "";

	_tlk->seek(entry.offset);

	size_t length = MIN<size_t>(entry.length, _tlk->size() - _tlk->pos());
	if (length == 0)
		return "";

	std::unique_ptr<Common::MemoryReadStream> data(_tlk->readStream(length));
	std::unique_ptr<Common::MemoryReadStream> parsed(LangMan.preParseColorCodes(*data));

	return Common::readString(*parsed, _encoding);
}

uint32_t TalkTable_TLK::getLanguageID() const {
	return _languageID;
}

void TalkTable_TLK::setLanguageID(uint32_t id) {
	_languageID = id;
}

const std::list<uint32_t> &TalkTable_TLK::getStrRefs() const {
	return _strRefs;
}

bool TalkTable_TLK::getString(uint32_t strRef, Common::UString &string, Common::UString &soundResRef) const {
	if (strRef >= _entries.size())
		return false;

	string      = readString(_entries[strRef]);
	soundResRef = _entries[strRef].soundResRef;

	return true;
}

bool TalkTable_TLK::getEntry(uint32_t strRef, Common::UString &string, Common::UString &soundResRef,
                             uint32_t &volumeVariance, uint32_t &pitchVariance, float &soundLength,
                             uint32_t &soundID) const {

	if (strRef >= _entries.size())
		return false;

	const Entry &entry = _entries[strRef];

	string      = readString(entry);
	soundResRef = entry.soundResRef;

	volumeVariance = entry.volumeVariance;
	pitchVariance  = entry.pitchVariance;
	soundLength    = entry.soundLength;

	soundID = entry.soundID;

	return true;
}

void TalkTable_TLK::setEntry(uint32_t strRef, const Common::UString &string, const Common::UString &soundResRef,
                             uint32_t volumeVariance, uint32_t pitchVariance, float soundLength,
                             uint32_t soundID) {

	if (strRef >= _entries.size()) {
		for (size_t i = _entries.size(); i < strRef; i++)
			_strRefs.push_back(i);

		_strRefs.sort();
		_strRefs.unique();

		_entries.resize(strRef + 1);
	}

	Entry &entry = _entries[strRef];

	entry.text        = string;
	entry.soundResRef = soundResRef;

	entry.volumeVariance = volumeVariance;
	entry.pitchVariance  = pitchVariance;
	entry.soundLength    = soundLength;

	entry.soundID = soundID;

	entry.length = 0;
	entry.offset = 0xFFFFFFFF;

	entry.flags = 0;
	if (!entry.text.empty())
		entry.flags |= kFlagTextPresent;
	if (!entry.soundResRef.empty())
		entry.flags |= kFlagSoundPresent;
	if (entry.soundLength > 0.0f)
		entry.flags |= kFlagSoundLengthPresent;
}

Common::SeekableReadStream *TalkTable_TLK::collectEntries(Entries &entries) const {
	entries.resize(_entries.size());

	Common::MemoryWriteStreamDynamic data;

	try {
		for (size_t i = 0; i < _entries.size(); i++) {
			entries[i].length = 0;
			entries[i].offset = 0;

			const Common::UString text = readString(_entries[i]);
			if (!text.empty()) {
				entries[i].offset = data.size();
				entries[i].length = Common::writeString(data, text, _encoding, false);
			}

			entries[i].soundResRef = _entries[i].soundResRef;

			entries[i].volumeVariance = _entries[i].volumeVariance;
			entries[i].pitchVariance  = _entries[i].pitchVariance;
			entries[i].soundLength    = _entries[i].soundLength;

			entries[i].soundID = _entries[i].soundID;

			entries[i].flags = 0;
			if (entries[i].length > 0)
				entries[i].flags |= kFlagTextPresent;
			if (!entries[i].soundResRef.empty())
				entries[i].flags |= kFlagSoundPresent;
			if (entries[i].soundLength >= 0.0f)
				entries[i].flags |= kFlagSoundLengthPresent;
		}
	} catch (...) {
		data.dispose();
		throw;
	}

	return new Common::MemoryReadStream(data.getData(), data.size(), true);
}

void TalkTable_TLK::write30(Common::WriteStream &out) const {
	out.writeUint32BE(kTLKID);
	out.writeUint32BE(kVersion3);

	out.writeUint32LE(_languageID);

	Entries entries;
	std::unique_ptr<Common::SeekableReadStream> data(collectEntries(entries));

	const uint32_t stringsOffset = 20 + entries.size() * 40;

	out.writeUint32LE(entries.size());
	out.writeUint32LE(stringsOffset);

	for (Entries::const_iterator e = entries.begin(); e != entries.end(); ++e) {
		out.writeUint32LE(e->flags);

		Common::writeStringFixed(out, e->soundResRef, Common::kEncodingASCII, 16);

		out.writeUint32LE(e->volumeVariance);
		out.writeUint32LE(e->pitchVariance);
		out.writeUint32LE(e->offset);
		out.writeUint32LE(e->length);

		out.writeIEEEFloatLE(MAX(0.0f, e->soundLength));
	}

	out.writeStream(*data);
}

void TalkTable_TLK::write40(Common::WriteStream &out) const {
	out.writeUint32BE(kTLKID);
	out.writeUint32BE(kVersion4);

	out.writeUint32LE(_languageID);

	Entries entries;
	std::unique_ptr<Common::SeekableReadStream> data(collectEntries(entries));

	const uint32_t stringsOffset = 32 + entries.size() * 10;

	out.writeUint32LE(entries.size());

	// Offset to the string table. We'll put it right after the header, with some padding
	out.writeUint32LE(32);

	out.writeUint32LE(stringsOffset);

	// Padding
	out.writeUint32LE(0);
	out.writeUint32LE(0);

	for (Entries::const_iterator e = entries.begin(); e != entries.end(); ++e) {
		out.writeUint32LE(e->soundID);
		out.writeUint32LE(e->offset + stringsOffset);
		out.writeUint16LE(e->length);
	}

	out.writeStream(*data);
}

uint32_t TalkTable_TLK::getLanguageID(Common::SeekableReadStream &tlk) {
	uint32_t id, version;
	bool utf16le;

	AuroraFile::readHeader(tlk, id, version, utf16le);

	if ((id != kTLKID) || ((version != kVersion3) && (version != kVersion4)))
		return kLanguageInvalid;

	return tlk.readUint32LE();
}

uint32_t TalkTable_TLK::getLanguageID(const Common::UString &file) {
	Common::ReadFile tlk;
	if (!tlk.open(file))
		return kLanguageInvalid;

	return getLanguageID(tlk);
}

} // End of namespace Aurora
