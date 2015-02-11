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

/** @file unrim.cpp
 *  Tool to extract RIM archives.
 */

#include <cstring>
#include <cstdio>

#include "common/ustring.h"
#include "common/error.h"
#include "common/file.h"

#include "aurora/util.h"
#include "aurora/rimfile.h"

#include "util.h"

enum Command {
	kCommandNone    = -1,
	kCommandList    =  0,
	kCommandExtract     ,
	kCommandMAX
};

const char *kCommandChar[kCommandMAX] = { "l", "e" };

void printUsage(FILE *stream, const char *name);
bool parseCommandLine(int argc, char **argv, int &returnValue,
                      Command &command, Common::UString &file, Aurora::GameID &game);

void listFiles(Aurora::RIMFile &rim, Aurora::GameID game);
void extractFiles(Aurora::RIMFile &rim, Aurora::GameID);

int main(int argc, char **argv) {
	Aurora::GameID game = Aurora::kGameIDUnknown;

	int returnValue;
	Command command;
	Common::UString file;
	if (!parseCommandLine(argc, argv, returnValue, command, file, game))
		return returnValue;

	try {
		Aurora::RIMFile rim(file);

		if      (command == kCommandList)
			listFiles(rim, game);
		else if (command == kCommandExtract)
			extractFiles(rim, game);

	} catch (Common::Exception &e) {
		Common::printException(e);
		return -1;
	}

	return 0;
}

bool parseCommandLine(int argc, char **argv, int &returnValue,
                      Command &command, Common::UString &file, Aurora::GameID &game) {

	file.clear();

	// No command, just display the help
	if (argc == 1) {
		printUsage(stdout, argv[0]);
		returnValue = 0;

		return false;
	}

	// Parse options
	int n;
	for (n = 1; (n < argc) && (argv[n][0] == '-'); n++) {
		if        (!strcmp(argv[n], "--nwn2")) {
			game = Aurora::kGameIDNWN2;
		} else if (!strcmp(argv[n], "--jade")) {
			game = Aurora::kGameIDJade;
		} else {
			// Unknown option
			printUsage(stderr, argv[0]);
			returnValue = -1;

			return false;
		}
	}

	// Wrong number of arguments, display the help
	if ((n + 2) != argc) {
		printUsage(stderr, argv[0]);
		returnValue = -1;

		return false;
	}

	// Find out what we should do
	command = kCommandNone;
	for (int i = 0; i < kCommandMAX; i++)
		if (!strcmp(argv[n], kCommandChar[i]))
			command = (Command) i;

	// Unknown command
	if (command == kCommandNone) {
		printUsage(stderr, argv[0]);
		returnValue = -1;

		return false;
	}

	// This is the file to use
	file = argv[n + 1];

	return true;
}

void printUsage(FILE *stream, const char *name) {
	std::fprintf(stream, "BioWare RIM archive extractor\n\n");
	std::fprintf(stream, "Usage: %s [<options>] <command> <file>\n\n", name);
	std::fprintf(stream, "Options:\n");
	std::fprintf(stream, "  --nwn2     Alias file types according to Neverwinter Nights 2 rules\n");
	std::fprintf(stream, "  --jade     Alias file types according to Jade Empire rules\n\n");
	std::fprintf(stream, "Commands:\n");
	std::fprintf(stream, "  l          List archive\n");
	std::fprintf(stream, "  e          Extract files to current directory\n");
}

void listFiles(Aurora::RIMFile &rim, Aurora::GameID game) {
	const Aurora::Archive::ResourceList &resources = rim.getResources();
	const uint32 fileCount = resources.size();

	std::printf("Number of files: %u\n\n", fileCount);

	std::printf("              Filename               |    Size\n");
	std::printf("=====================================|===========\n");

	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r) {
		const Aurora::FileType type = Aurora::aliasFileType(r->type, game);

		std::printf("%32s%s | %10d\n", r->name.c_str(), Aurora::setFileType("", type).c_str(),
		                               rim.getResourceSize(r->index));
	}
}

void extractFiles(Aurora::RIMFile &rim, Aurora::GameID game) {
	const Aurora::Archive::ResourceList &resources = rim.getResources();
	const uint32 fileCount = resources.size();

	std::printf("Number of files: %u\n\n", fileCount);

	uint i = 1;
	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r, ++i) {
		const Aurora::FileType type     = Aurora::aliasFileType(r->type, game);
		const Common::UString  fileName = Aurora::setFileType(r->name, type);

		std::printf("Extracting %d/%d: %s ... ", i, fileCount, fileName.c_str());

		Common::SeekableReadStream *stream = 0;
		try {
			stream = rim.getResource(r->index);

			dumpStream(*stream, fileName);

			std::printf("Done\n");
		} catch (Common::Exception &e) {
			Common::printException(e, "");
		}

		delete stream;
	}

}
