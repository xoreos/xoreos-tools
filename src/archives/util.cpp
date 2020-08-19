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
 *  General tool utility functions.
 */

#include <cstdio>

#include <vector>
#include <memory>

#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/hash.h"
#include "src/common/filepath.h"
#include "src/common/readstream.h"
#include "src/common/writefile.h"

#include "src/aurora/util.h"
#include "src/aurora/archive.h"
#include "src/aurora/keyfile.h"
#include "src/aurora/nsbtxfile.h"

#include "src/archives/util.h"
#include "src/archives/files_dragonage.h"
#include "src/archives/files_sonic.h"

namespace Archives {

static Common::UString findPath(const Common::UString &name, Aurora::FileType type,
                                uint64_t hash, Common::HashAlgo algo) {

	Common::UString path;

	if (!name.empty())
		path = TypeMan.addFileType(name, type);

	if (path.empty()) {
		const char * const fromDAHash = findDragonAgeFile(hash, algo);
		if (fromDAHash)
			path = TypeMan.setFileType(fromDAHash, type);
	}

	if (path.empty()) {
		const char * const fromSonicHash = findSonicFile(hash, algo);
		if (fromSonicHash)
			path = fromSonicHash;
	}

	if (path.empty())
		path = TypeMan.addFileType(Common::formatHash(hash), type);

	path.replaceAll('\\', '/');

	return path;
}

struct FileEntry {
	Common::UString file;
	Common::UString ext;
	uint32_t size;
	uint32_t bifIndex;

	FileEntry(const Common::UString &f = "", const Common::UString &e = "", uint32_t s = 0xFFFFFFFF) :
		file(f), ext(e), size(s), bifIndex(0xFFFFFFFF) { }
};

void listFiles(const Aurora::Archive &archive, Aurora::GameID game, bool directories) {
	const Aurora::Archive::ResourceList &resources = archive.getResources();
	const size_t fileCount = resources.size();

	std::printf("Number of files: %s\n\n", Common::composeString(fileCount).c_str());

	std::vector<FileEntry> fileEntries;
	fileEntries.reserve(fileCount);

	size_t nameLength = 0, extLength = 0;
	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r) {
		const Aurora::FileType type = TypeMan.aliasFileType(r->type, game);

		const Common::UString path     = findPath(r->name, type, r->hash, archive.getNameHashAlgo());
		const Common::UString fileName = Common::FilePath::getFile(path);
		const Common::UString name     = directories ? path :  Common::FilePath::getStem(fileName);
		const Common::UString ext      = directories ? "" : Common::FilePath::getExtension(path);

		nameLength = MAX<size_t>(nameLength, name.size());
		extLength = MAX<size_t>(extLength, ext.size());

		fileEntries.push_back(FileEntry(name, ext, archive.getResourceSize(r->index)));
	}

	size_t namePrintLength = MAX<size_t>(10, nameLength + extLength + 1);
	if ((namePrintLength % 2) == 1)
		namePrintLength++;

	std::printf("%sFileName%s|    Size\n", Common::UString(' ', (namePrintLength - 8) / 2).c_str(),
	                                       Common::UString(' ', (namePrintLength - 8) / 2).c_str());
	std::printf("%s|===========\n", Common::UString('=', namePrintLength).c_str());

	for (std::vector<FileEntry>::const_iterator f = fileEntries.begin(); f != fileEntries.end(); ++f) {
		if (directories)
			std::printf("%-*s| %10d\n", static_cast<int>(namePrintLength), f->file.c_str(), f->size);
		else
			std::printf("%*s%-*s | %10d\n", static_cast<int>(namePrintLength - extLength - 1), f->file.c_str(), static_cast<int>(extLength), f->ext.c_str(), f->size);
	}
}

void listFiles(const Aurora::KEYFile &key, const Common::UString &keyName, Aurora::GameID game) {
	const Aurora::KEYFile::BIFList &bifs = key.getBIFs();

	size_t maxBIFLength = 4;
	for (Aurora::KEYFile::BIFList::const_iterator b = bifs.begin(); b != bifs.end(); ++b)
		maxBIFLength = MAX<size_t>(maxBIFLength, b->size());

	const Aurora::KEYFile::ResourceList &resources = key.getResources();
	const size_t fileCount = resources.size();

	std::vector<FileEntry> fileEntries;
	fileEntries.reserve(fileCount);

	size_t nameLength = 0, extLength = 0;
	for (Aurora::KEYFile::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r) {
		const Aurora::FileType type = TypeMan.aliasFileType(r->type, game);
		const Common::UString ext = TypeMan.setFileType("", type);

		nameLength = MAX<size_t>(nameLength, r->name.size());
		extLength = MAX<size_t>(extLength, ext.size());

		fileEntries.push_back(FileEntry(r->name, ext));
		fileEntries.back().bifIndex = r->bifIndex;
	}

	size_t namePrintLength = MAX<size_t>(10, nameLength + extLength + 1);
	if ((namePrintLength % 2) == 1)
		namePrintLength++;

	std::printf("%s: Number of files: %s\n\n", keyName.c_str(), Common::composeString(fileCount).c_str());

	std::printf("%sFileName%s| BIF\n", Common::UString(' ', (namePrintLength - 8) / 2).c_str(),
	                                   Common::UString(' ', (namePrintLength - 8) / 2).c_str());
	std::printf("%s|=====", Common::UString('=', namePrintLength).c_str());

	for (size_t i = 4; i < maxBIFLength; i++)
		std::printf("=");

	std::printf("\n");

	for (std::vector<FileEntry>::const_iterator f = fileEntries.begin(); f != fileEntries.end(); ++f) {
		std::printf("%*s%-*s | %s\n", static_cast<int>(namePrintLength - extLength - 1), f->file.c_str(), static_cast<int>(extLength), f->ext.c_str(), (f->bifIndex < bifs.size()) ? bifs[f->bifIndex].c_str() : "");
	}
}

void listFiles(const Aurora::NSBTXFile &nsbtx) {
	const Aurora::Archive::ResourceList &resources = nsbtx.getResources();
	const size_t fileCount = resources.size();

	std::printf("Number of files: %s\n\n", Common::composeString(fileCount).c_str());

	std::printf("      Filename       \n");
	std::printf("=====================\n");

	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r)
		std::printf("%16s.tga\n", r->name.c_str());
}

static void dumpStream(Common::SeekableReadStream &stream, const Common::UString &fileName) {
	Common::WriteFile file;
	if (!file.open(fileName))
		throw Common::Exception(Common::kOpenError);

	file.writeStream(stream);
	file.flush();

	file.close();
}

void extractFiles(const Aurora::Archive &archive, Aurora::GameID game, bool directories,
                  const std::set<Common::UString> &files) {

	const Aurora::Archive::ResourceList &resources = archive.getResources();
	const size_t fileCount = resources.size();

	std::printf("Number of files: %s\n\n", Common::composeString(fileCount).c_str());

	size_t i = 1;
	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r, ++i) {
		const Aurora::FileType type = TypeMan.aliasFileType(r->type, game);

		const Common::UString path     = findPath(r->name, type, r->hash, archive.getNameHashAlgo());
		const Common::UString fileName = Common::FilePath::getFile(path);
		const Common::UString dirName  = Common::FilePath::getDirectory(path);
		const Common::UString name     = directories ? path : fileName;

		if (!files.empty() && (files.find(name) == files.end()))
			continue;

		if (directories && !dirName.empty())
			Common::FilePath::createDirectories(dirName);

		std::printf("Extracting %s/%s: %s ... ", Common::composeString(i).c_str(),
		                                         Common::composeString(fileCount).c_str(),
		                                         name.c_str());
		std::fflush(stdout);

		try {
			std::unique_ptr<Common::SeekableReadStream> stream(archive.getResource(r->index));

			dumpStream(*stream, name);

			std::printf("Done\n");
		} catch (Common::Exception &e) {
			Common::printException(e, "");
		}
	}
}

void extractFiles(const Aurora::NSBTXFile &nsbtx, const std::set<Common::UString> &files,
                  void (*dumper)(Common::SeekableReadStream &stream, const Common::UString &fileName)) {

	const Aurora::Archive::ResourceList &resources = nsbtx.getResources();
	const size_t fileCount = resources.size();

	std::printf("Number of files: %s\n\n", Common::composeString(fileCount).c_str());

	size_t i = 1;
	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r, ++i) {
		const Common::UString name = r->name + ".tga";

		if (!files.empty() && (files.find(name) == files.end()))
			continue;

		std::printf("Extracting %s/%s: %s ... ", Common::composeString(i).c_str(),
		                                         Common::composeString(fileCount).c_str(),
		                                         name.c_str());
		std::fflush(stdout);

		try {
			std::unique_ptr<Common::SeekableReadStream> stream(nsbtx.getResource(r->index));

			dumper(*stream, name);

			std::printf("Done\n");
		} catch (Common::Exception &e) {
			Common::printException(e, "");
		}
	}
}

std::set<Common::UString> fixPathSeparator(const std::set<Common::UString> &files) {
	std::set<Common::UString> newFiles;

	for (std::set<Common::UString>::const_iterator f = files.begin(); f != files.end(); ++f) {
		Common::UString file = *f;
		file.replaceAll('\\', '/');

		newFiles.insert(file);
	}

	return newFiles;
}

} // End of namespace Archives
