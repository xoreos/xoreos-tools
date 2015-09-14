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

#include "src/common/types.h"
#include "src/common/encoding.h"

namespace Common {
	class UString;
	class SeekableReadStream;
}

namespace Aurora {

/** Base class for BioWare's talk tables. */
class TalkTable {
public:
	virtual ~TalkTable();

	virtual uint32 getLanguageID() const;
	virtual void setLanguageID(uint32 id);

	virtual const std::list<uint32> &getStrRefs() const = 0;
	virtual bool getString(uint32 strRef, Common::UString &string, Common::UString &soundResRef) const = 0;

	virtual bool getEntry(uint32 strRef, Common::UString &string, Common::UString &soundResRef,
	                      uint32 &volumeVariance, uint32 &pitchVariance, float &soundLength,
	                      uint32 &soundID) const = 0;

	virtual void setEntry(uint32 strRef, const Common::UString &string, const Common::UString &soundResRef,
	                      uint32 volumeVariance, uint32 pitchVariance, float soundLength,
	                      uint32 soundID) = 0;

	static TalkTable *load(Common::SeekableReadStream *tlk, Common::Encoding encoding);


protected:
	TalkTable(Common::Encoding encoding);

	Common::Encoding _encoding;
};

} // End of namespace Aurora

#endif // AURORA_TALKTABLE_H
