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
 *  Nintendo DS ROM parsing.
 */

#ifndef AURORA_NDSROM_H
#define AURORA_NDSROM_H

#include <vector>

#include "src/common/types.h"
#include "src/common/ustring.h"

#include "src/aurora/types.h"
#include "src/aurora/archive.h"

namespace Common {
	class SeekableReadStream;
	class File;
}

namespace Aurora {

/** A class encapsulating Nintendo DS ROM access. */
class NDSFile : public Archive {
public:
	NDSFile(const Common::UString &fileName);
	~NDSFile();

	/** Clear the resource list. */
	void clear();

	/** Return the list of resources. */
	const ResourceList &getResources() const;

	/** Return the size of a resource. */
	uint32 getResourceSize(uint32 index) const;

	/** Return a stream of the resource's contents. */
	Common::SeekableReadStream *getResource(uint32 index) const;

	const Common::UString &getName() const;
	const Common::UString &getCode() const;
	const Common::UString &getMaker() const;


private:
	/** Internal resource information. */
	struct IResource {
		uint32 offset; ///< The offset of the resource within the NDS.
		uint32 size;   ///< The resource's size.
	};

	typedef std::vector<IResource> IResourceList;

	/** External list of resource names and types. */
	ResourceList _resources;

	/** Internal list of resource offsets and sizes. */
	IResourceList _iResources;

	/** The name of the NDS file. */
	Common::UString _fileName;

	Common::UString _name;
	Common::UString _code;
	Common::UString _maker;

	uint32 _fileNameTableOffset;
	uint32 _fileNameTableLength;
	uint32 _fatOffset;
	uint32 _fatLength;

	uint32 _arm9CodeOffset;
	uint32 _arm9CodeSize;

	uint32 _arm7CodeOffset;
	uint32 _arm7CodeSize;

	uint32 _romSize;
	uint32 _headerSize;

	void open(Common::File &file) const;

	void load();
	bool readHeader(Common::SeekableReadStream &nds);
	void readNames(Common::SeekableReadStream &nds, uint32 offset, uint32 length);
	void readFAT(Common::SeekableReadStream &nds, uint32 offset);

	const IResource &getIResource(uint32 index) const;
};

} // End of namespace Aurora

#endif // AURORA_NDSROM_H
