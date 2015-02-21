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
 *  Tool to extract ERF (.erf, .mod, .nwm, .sav) archives.
 */

#include <cstring>
#include <cstdio>

#include "src/common/ustring.h"
#include "src/common/error.h"
#include "src/common/file.h"

#include "src/aurora/util.h"
#include "src/aurora/erffile.h"

#include "src/util.h"

enum Command {
	kCommandNone    = -1,
	kCommandInfo    =  0,
	kCommandList        ,
	kCommandExtract     ,
	kCommandMAX
};

const char *kCommandChar[kCommandMAX] = { "i", "l", "e" };

void printUsage(FILE *stream, const char *name);
bool parseCommandLine(int argc, char **argv, int &returnValue,
                      Command &command, Common::UString &file, Aurora::GameID &game);

void displayInfo(Aurora::ERFFile &erf);
void listFiles(Aurora::ERFFile &rim, Aurora::GameID game);
void extractFiles(Aurora::ERFFile &rim, Aurora::GameID);

int main(int argc, char **argv) {
	Aurora::GameID game = Aurora::kGameIDUnknown;

	int returnValue;
	Command command;
	Common::UString file;
	if (!parseCommandLine(argc, argv, returnValue, command, file, game))
		return returnValue;

	try {
		Aurora::ERFFile erf(file);

		if      (command == kCommandInfo)
			displayInfo(erf);
		else if (command == kCommandList)
			listFiles(erf, game);
		else if (command == kCommandExtract)
			extractFiles(erf, game);

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
	std::fprintf(stream, "BioWare ERF (.erf, .mod, .nwm, .sav) archive extractor\n\n");
	std::fprintf(stream, "Usage: %s <command> <file>\n\n", name);
	std::fprintf(stream, "Options:\n");
	std::fprintf(stream, "  --nwn2     Alias file types according to Neverwinter Nights 2 rules\n");
	std::fprintf(stream, "  --jade     Alias file types according to Jade Empire rules\n\n");
	std::fprintf(stream, "Commands:\n");
	std::fprintf(stream, "  i          Display meta-information\n");
	std::fprintf(stream, "  l          List archive\n");
	std::fprintf(stream, "  e          Extract files to current directory\n");
}

void displayInfo(Aurora::ERFFile &erf) {
	std::printf("Build Year: %d\n", erf.getBuildYear());
	std::printf("Build Day: %d\n", erf.getBuildDay());
	std::printf("Number of files: %u\n", (uint)erf.getResources().size());

	const Aurora::LocString &description = erf.getDescription();
	if (description.getString().empty() && (description.getID() == Aurora::kStrRefInvalid))
		return;

	std::printf("\nDescription:\n");
	std::printf("String reference ID: %u\n", description.getID());

	for (int i = 0; i < Aurora::LocString::kStringCount; i++) {
		Aurora::Language language;
		const Common::UString &string = description.getString(i, language);

		if (string.empty())
			continue;

		std::printf("\n.=== Description in language %d: ===\n", (int)language);
		std::printf("%s\n", string.c_str());
		std::printf("'=== ===\n");
	}
}

void listFiles(Aurora::ERFFile &erf, Aurora::GameID game) {
	const Aurora::Archive::ResourceList &resources = erf.getResources();
	const uint32 fileCount = resources.size();

	std::printf("Number of files: %u\n\n", fileCount);

	std::printf("              Filename               |    Size\n");
	std::printf("=====================================|===========\n");

	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r) {
		const Aurora::FileType type = Aurora::aliasFileType(r->type, game);

		std::printf("%32s%s | %10d\n", r->name.c_str(), Aurora::setFileType("", type).c_str(),
		                               erf.getResourceSize(r->index));
	}
}

void extractFiles(Aurora::ERFFile &erf, Aurora::GameID game) {
	const Aurora::Archive::ResourceList &resources = erf.getResources();
	const uint32 fileCount = resources.size();

	std::printf("Number of files: %u\n\n", fileCount);

	uint i = 1;
	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r, ++i) {
		const Aurora::FileType type     = Aurora::aliasFileType(r->type, game);
		const Common::UString  fileName = Aurora::setFileType(r->name, type);

		std::printf("Extracting %d/%d: %s ... ", i, fileCount, fileName.c_str());

		Common::SeekableReadStream *stream = 0;
		try {
			stream = erf.getResource(r->index);

			dumpStream(*stream, fileName);

			std::printf("Done\n");
		} catch (Common::Exception &e) {
			Common::printException(e, "");
		}

		delete stream;
	}

}
