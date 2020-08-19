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
 *  Base class for BioWare's talk tables.
 */

#ifndef AURORA_TALKTABLE_H
#define AURORA_TALKTABLE_H

#include <list>

#include <boost/noncopyable.hpp>

#include "src/common/types.h"
#include "src/common/encoding.h"

namespace Common {
	class UString;
	class SeekableReadStream;
}

namespace Aurora {

/** Base class for BioWare's talk tables.
 *
 *  A talk table contains localized string data, and optional voice-
 *  over resource names, indexed by a string reference ("StrRef").
 *
 *  A single talktable always contains strings in a single language
 *  (and for a single gender of the PC), and commonly all strings for
 *  a given context (module, campaign, ...).
 *
 *  See classes TalkTable_TLK and TalkTable_GFF for the two main
 *  formats a talk table can be found in.
 */
class TalkTable : boost::noncopyable {
public:
	virtual ~TalkTable();

	virtual uint32_t getLanguageID() const;
	virtual void setLanguageID(uint32_t id);

	virtual const std::list<uint32_t> &getStrRefs() const = 0;
	virtual bool getString(uint32_t strRef, Common::UString &string, Common::UString &soundResRef) const = 0;

	virtual bool getEntry(uint32_t strRef, Common::UString &string, Common::UString &soundResRef,
	                      uint32_t &volumeVariance, uint32_t &pitchVariance, float &soundLength,
	                      uint32_t &soundID) const = 0;

	virtual void setEntry(uint32_t strRef, const Common::UString &string, const Common::UString &soundResRef,
	                      uint32_t volumeVariance, uint32_t pitchVariance, float soundLength,
	                      uint32_t soundID) = 0;

	/** Take over this stream and read a talk table (of either format) out of it. */
	static TalkTable *load(Common::SeekableReadStream *tlk, Common::Encoding encoding);


protected:
	TalkTable(Common::Encoding encoding);

	Common::Encoding _encoding;
};

} // End of namespace Aurora

#endif // AURORA_TALKTABLE_H
