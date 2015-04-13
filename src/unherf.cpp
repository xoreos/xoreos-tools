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
 *  Tool to extract HERF archives.
 */

#include <cstring>
#include <cstdio>

#include <map>

#include "src/common/ustring.h"
#include "src/common/error.h"
#include "src/common/file.h"
#include "src/common/filepath.h"
#include "src/common/hash.h"

#include "src/aurora/util.h"
#include "src/aurora/herffile.h"

#include "src/util.h"
#include "src/files_sonic.h"

enum Command {
	kCommandNone    = -1,
	kCommandList    =  0,
	kCommandExtract     ,
	kCommandMAX
};

typedef std::map<uint32, Common::UString> HashRegistry;

const char *kCommandChar[kCommandMAX] = { "l", "e" };

void printUsage(FILE *stream, const char *name);
bool parseCommandLine(int argc, char **argv, int &returnValue,
                      Command &command, Common::UString &file);

void createHashRegistry(HashRegistry &hashRegistry);
bool findHashedName(const HashRegistry &hashRegistry, uint32 hash,
                    Common::UString &name, Common::UString &ext);

void listFiles(Aurora::HERFFile &rim, const HashRegistry &hashRegistry);
void extractFiles(Aurora::HERFFile &rim, const HashRegistry &hashRegistry);

int main(int argc, char **argv) {
	int returnValue;
	Command command;
	Common::UString file;
	if (!parseCommandLine(argc, argv, returnValue, command, file))
		return returnValue;

	try {
		Aurora::HERFFile herf(file);

		HashRegistry hashRegistry;
		createHashRegistry(hashRegistry);

		if      (command == kCommandList)
			listFiles(herf, hashRegistry);
		else if (command == kCommandExtract)
			extractFiles(herf, hashRegistry);

	} catch (Common::Exception &e) {
		Common::printException(e);
		return -1;
	}

	return 0;
}

bool parseCommandLine(int argc, char **argv, int &returnValue,
                      Command &command, Common::UString &file) {

	file.clear();

	// No command, just display the help
	if (argc == 1) {
		printUsage(stdout, argv[0]);
		returnValue = 0;

		return false;
	}

	// Wrong number of arguments, display the help
	if (argc != 3) {
		printUsage(stderr, argv[0]);
		returnValue = -1;

		return false;
	}

	// Find out what we should do
	command = kCommandNone;
	for (int i = 0; i < kCommandMAX; i++)
		if (!strcmp(argv[1], kCommandChar[i]))
			command = (Command) i;

	// Unknown command
	if (command == kCommandNone) {
		printUsage(stderr, argv[0]);
		returnValue = -1;

		return false;
	}

	// This is the file to use
	file = argv[2];

	return true;
}

void printUsage(FILE *stream, const char *name) {
	std::fprintf(stream, "BioWare HERF archive extractor\n\n");
	std::fprintf(stream, "Usage: %s <command> <file>\n\n", name);
	std::fprintf(stream, "Commands:\n");
	std::fprintf(stream, "  l          List archive\n");
	std::fprintf(stream, "  e          Extract files to current directory\n");
}

void createHashRegistry(HashRegistry &hashRegistry) {
	for (int i = 0; i < ARRAYSIZE(kFilesSonic); i++)
		hashRegistry[Common::hashStringDJB2(kFilesSonic[i])] = kFilesSonic[i];
}

bool findHashedName(const HashRegistry &hashRegistry, uint32 hash,
                    Common::UString &name, Common::UString &ext) {

	HashRegistry::const_iterator h = hashRegistry.find(hash);
	if (h != hashRegistry.end()) {
		name = Common::FilePath::getStem(h->second);
		ext  = Common::FilePath::getExtension(h->second);
		return true;
	}

	name = Common::UString::sprintf("0x%08X", hash);
	ext  = "";
	return false;
}

void listFiles(Aurora::HERFFile &herf, const HashRegistry &hashRegistry) {
	const Aurora::Archive::ResourceList &resources = herf.getResources();
	const uint32 fileCount = resources.size();

	std::printf("Number of files: %u\n\n", fileCount);

	std::printf("               Filename                |    Size\n");
	std::printf("=======================================|===========\n");

	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r) {
		Common::UString fileName = r->name, fileExt = Aurora::setFileType("", r->type);
		if (fileName.empty())
			findHashedName(hashRegistry, r->hash, fileName, fileExt);

		std::printf("%32s%-6s | %10d\n", fileName.c_str(), fileExt.c_str(), herf.getResourceSize(r->index));
	}
}

void extractFiles(Aurora::HERFFile &herf, const HashRegistry &hashRegistry) {
	const Aurora::Archive::ResourceList &resources = herf.getResources();
	const uint32 fileCount = resources.size();

	std::printf("Number of files: %u\n\n", fileCount);

	uint i = 1;
	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r, ++i) {
		Common::UString fileName = r->name, fileExt = Aurora::setFileType("", r->type);
		if (fileName.empty())
			findHashedName(hashRegistry, r->hash, fileName, fileExt);

		fileName = fileName + fileExt;

		std::printf("Extracting %d/%d: %s ... ", i, fileCount, fileName.c_str());

		Common::SeekableReadStream *stream = 0;
		try {
			stream = herf.getResource(r->index);

			dumpStream(*stream, fileName);

			std::printf("Done\n");
		} catch (Common::Exception &e) {
			Common::printException(e, "");
		}

		delete stream;
	}

}
