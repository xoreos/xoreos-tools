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
 *  Tool to extract files in NDS (Nintendo DS) roms.
 */

#include <cstring>
#include <cstdio>

#include "common/ustring.h"
#include "common/error.h"
#include "common/file.h"

#include "aurora/util.h"
#include "aurora/ndsrom.h"

#include "util.h"

enum Command {
	kCommandNone    = -1,
	kCommandInfo    =  0,
	kCommandList        ,
	kCommandExtract     ,
	kCommandMAX
};

const char *kCommandChar[kCommandMAX] = { "i", "l", "e" };

void printUsage(FILE *stream, const char *name);
bool parseCommandLine(int argc, char **argv, int &returnValue, Command &command, Common::UString &file);

void displayInfo(Aurora::NDSFile &nds);
void listFiles(Aurora::NDSFile &nds);
void extractFiles(Aurora::NDSFile &nds);

int main(int argc, char **argv) {
	int returnValue;
	Command command;
	Common::UString file;
	if (!parseCommandLine(argc, argv, returnValue, command, file))
		return returnValue;

	try {
		Aurora::NDSFile nds(file);

		if      (command == kCommandInfo)
			displayInfo(nds);
		else if (command == kCommandList)
			listFiles(nds);
		else if (command == kCommandExtract)
			extractFiles(nds);

	} catch (Common::Exception &e) {
		Common::printException(e);
		return -1;
	}

	return 0;
}

bool parseCommandLine(int argc, char **argv, int &returnValue, Command &command, Common::UString &file) {
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
	std::fprintf(stream, "Nintendo DS archive extractor\n\n");
	std::fprintf(stream, "Usage: %s <command> <file>\n\n", name);
	std::fprintf(stream, "Commands:\n");
	std::fprintf(stream, "  i          Display meta-information\n");
	std::fprintf(stream, "  l          List archive\n");
	std::fprintf(stream, "  e          Extract files to current directory\n");
}

void displayInfo(Aurora::NDSFile &nds) {
	std::printf("Game name: \"%s\"\n", nds.getName().c_str());
	std::printf("Game code: \"%s\"\n", nds.getCode().c_str());
	std::printf("Game maker: \"%s\"\n", nds.getMaker().c_str());
}

void listFiles(Aurora::NDSFile &nds) {
	const Aurora::Archive::ResourceList &resources = nds.getResources();
	const uint32 fileCount = resources.size();

	std::printf("Number of files: %u\n\n", fileCount);

	std::printf("               Filename                |    Size\n");
	std::printf("=======================================|===========\n");

	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r)
		std::printf("%32s%-6s | %10d\n", r->name.c_str(), Aurora::setFileType("", r->type).c_str(),
		                               nds.getResourceSize(r->index));
}

void extractFiles(Aurora::NDSFile &nds) {
	const Aurora::Archive::ResourceList &resources = nds.getResources();
	const uint32 fileCount = resources.size();

	std::printf("Number of files: %u\n\n", fileCount);

	uint i = 1;
	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r, ++i) {
		const Common::UString fileName = Aurora::setFileType(r->name, r->type);

		std::printf("Extracting %d/%d: %s ... ", i, fileCount, fileName.c_str());

		Common::SeekableReadStream *stream = 0;
		try {
			stream = nds.getResource(r->index);

			dumpStream(*stream, fileName);

			std::printf("Done\n");
		} catch (Common::Exception &e) {
			Common::printException(e, "");
		}

		delete stream;
	}

}
