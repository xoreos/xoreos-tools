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

#ifndef AURORA_TALKTABLE_TLK_H
#define AURORA_TALKTABLE_TLK_H

#include <vector>
#include <memory>

#include "src/common/types.h"
#include "src/common/ustring.h"

#include "src/aurora/aurorafile.h"
#include "src/aurora/talktable.h"

namespace Common {
	class WriteStream;
}

namespace Aurora {

/** Loading BioWare's TLK talk tables.
 *
 *  See class TalkTable for a general overview how talk tables work.
 *
 *  Unlike TalkTable_GFF, a TLK talk table is its own simple binary
 *  format. It has a numerical, game-local ID of the language it
 *  contains, and stores a few more optional data points per string,
 *  like a reference to a voice-over file.
 *
 *  There are two versions of TLK files known and supported
 *  - V3.0, used by Neverwinter Nights, Neverwinter Nights 2, Knight of
 *    the Old Republic, Knight of the Old Republic II and The Witcher
 *  - V4.0, used by Jade Empire
 */
class TalkTable_TLK : public AuroraFile, public TalkTable {
public:
	TalkTable_TLK(Common::Encoding encoding, uint32_t languageID);
	/** Take over this stream and read a TLK out of it. */
	TalkTable_TLK(Common::SeekableReadStream *tlk, Common::Encoding encoding);
	~TalkTable_TLK();

	/** Return the language ID (ungendered) of the talk table. */
	uint32_t getLanguageID() const;

	/** Set the language ID (ungendered) of the talk table. */
	void setLanguageID(uint32_t id);

	const std::list<uint32_t> &getStrRefs() const;
	bool getString(uint32_t strRef, Common::UString &string, Common::UString &soundResRef) const;

	/** Return all values associated to a string references in a TLK talk table. */
	bool getEntry(uint32_t strRef, Common::UString &string, Common::UString &soundResRef,
	              uint32_t &volumeVariance, uint32_t &pitchVariance, float &soundLength,
	              uint32_t &soundID) const;

	/** Modify or add an entry to the talk table. */
	void setEntry(uint32_t strRef, const Common::UString &string, const Common::UString &soundResRef,
	              uint32_t volumeVariance, uint32_t pitchVariance, float soundLength,
	              uint32_t soundID);

	/** Write this TLK as a version V3.0 TLK into that stream. */
	void write30(Common::WriteStream &out) const;
	/** Write this TLK as a version V4.0 TLK into that stream. */
	void write40(Common::WriteStream &out) const;

	static uint32_t getLanguageID(Common::SeekableReadStream &tlk);
	static uint32_t getLanguageID(const Common::UString &file);


private:
	/** The entries' flags. */
	enum EntryFlags {
		kFlagTextPresent        = (1 << 0),
		kFlagSoundPresent       = (1 << 1),
		kFlagSoundLengthPresent = (1 << 2)
	};

	/** A talk resource entry. */
	struct Entry {
		Common::UString text;
		uint32_t offset;
		uint32_t length;

		// V3
		uint32_t flags;
		Common::UString soundResRef;
		uint32_t volumeVariance; // Unused
		uint32_t pitchVariance; // Unused
		float soundLength; // In seconds

		// V4
		uint32_t soundID;

		Entry();
	};

	typedef std::vector<Entry> Entries;


	std::unique_ptr<Common::SeekableReadStream> _tlk;

	uint32_t _languageID;

	std::list<uint32_t> _strRefs;

	Entries _entries;

	void load();

	void readEntryTableV3(uint32_t stringsOffset);
	void readEntryTableV4();

	Common::UString readString(const Entry &entry) const;

	Common::SeekableReadStream *collectEntries(Entries &entries) const;
};

} // End of namespace Aurora

#endif // AURORA_TALKTABLE_TLK_H
