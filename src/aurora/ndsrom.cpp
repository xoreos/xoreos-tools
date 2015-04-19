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

// Based on http://dsibrew.org/wiki/NDS_Format

#include "src/common/util.h"
#include "src/common/file.h"
#include "src/common/stream.h"
#include "src/common/encoding.h"

#include "src/aurora/ndsrom.h"
#include "src/aurora/error.h"
#include "src/aurora/util.h"

namespace Aurora {

NDSFile::NDSFile(const Common::UString &fileName) : _fileName(fileName) {
	load();
}

NDSFile::~NDSFile() {
}

void NDSFile::clear() {
	_resources.clear();
}

void NDSFile::load() {
	Common::File nds;
	open(nds);

	if (!readHeader(nds))
		throw Common::Exception("Not a valid NDS ROM file");

	try {

		readNames(nds, _fileNameTableOffset, _fileNameTableLength);
		readFAT(nds, _fatOffset);

	if (nds.err())
		throw Common::Exception(Common::kReadError);

	} catch (Common::Exception &e) {
		e.add("Failed reading NDS file");
		throw;
	}

}

bool NDSFile::readHeader(Common::SeekableReadStream &nds) {
	nds.seek(0x00);
	_name  = Common::readStringFixed(nds, Common::kEncodingASCII, 12);
	_code  = Common::readStringFixed(nds, Common::kEncodingASCII,  4);
	_maker = Common::readStringFixed(nds, Common::kEncodingASCII,  2);

	nds.seek(0x20);
	_arm9CodeOffset = nds.readUint32LE();
	nds.skip(8);
	_arm9CodeSize = nds.readUint32LE();

	nds.seek(0x30);
	_arm7CodeOffset = nds.readUint32LE();
	nds.skip(8);
	_arm7CodeSize = nds.readUint32LE();

	nds.seek(0x40);
	_fileNameTableOffset = nds.readUint32LE();
	_fileNameTableLength = nds.readUint32LE();
	_fatOffset           = nds.readUint32LE();
	_fatLength           = nds.readUint32LE();

	nds.seek(0x80);
	_romSize    = nds.readUint32LE();
	_headerSize = nds.readUint32LE();

	const uint32 size = nds.size();

	if ((_fileNameTableOffset >= size) || ((_fileNameTableOffset + _fileNameTableLength) > size))
		return false;
	if ((_fatOffset >= size) || ((_fatOffset + _fatLength) > size))
		return false;

	if ((_arm9CodeOffset >= size) || ((_arm9CodeOffset + _arm9CodeSize) > size))
		return false;
	if ((_arm7CodeOffset >= size) || ((_arm7CodeOffset + _arm7CodeSize) > size))
		return false;

	if (_romSize > size)
		return false;
	if (_headerSize > size)
		return false;

	return true;
}

void NDSFile::readNames(Common::SeekableReadStream &nds, uint32 offset, uint32 length) {
	nds.seek(offset + 8);

	uint32 index = 0;
	while (((uint32) nds.pos()) < (offset + length)) {
		Resource res;

		byte nameLength = nds.readByte();

		Common::UString name = Common::readStringFixed(nds, Common::kEncodingASCII, nameLength).toLower();

		res.name  = setFileType(name, kFileTypeNone);
		res.type  = getFileType(name);
		res.index = index++;

		_resources.push_back(res);
	}

	while (!_resources.empty() && _resources.back().name.empty())
		_resources.pop_back();
}

void NDSFile::readFAT(Common::SeekableReadStream &nds, uint32 offset) {
	nds.seek(offset);

	_iResources.resize(_resources.size());
	for (IResourceList::iterator res = _iResources.begin(); res != _iResources.end(); ++res) {
		res->offset = nds.readUint32LE();
		res->size   = nds.readUint32LE() - res->offset; // Value is the end offset
	}
}

const Archive::ResourceList &NDSFile::getResources() const {
	return _resources;
}

const NDSFile::IResource &NDSFile::getIResource(uint32 index) const {
	if (index >= _iResources.size())
		throw Common::Exception("Resource index out of range (%d/%d)", index, _iResources.size());

	return _iResources[index];
}

uint32 NDSFile::getResourceSize(uint32 index) const {
	return getIResource(index).size;
}

Common::SeekableReadStream *NDSFile::getResource(uint32 index) const {
	const IResource &res = getIResource(index);
	if (res.size == 0)
		return new Common::MemoryReadStream(0, 0);

	Common::File nds;
	open(nds);

	nds.seek(res.offset);

	Common::SeekableReadStream *resStream = nds.readStream(res.size);

	if (!resStream || (((uint32) resStream->size()) != res.size)) {
		delete resStream;
		throw Common::Exception(Common::kReadError);
	}

	return resStream;
}

void NDSFile::open(Common::File &file) const {
	if (!file.open(_fileName))
		throw Common::Exception(Common::kOpenError);
}

const Common::UString &NDSFile::getName() const {
	return _name;
}

const Common::UString &NDSFile::getCode() const {
	return _code;
}

const Common::UString &NDSFile::getMaker() const {
	return _maker;
}

} // End of namespace Aurora
