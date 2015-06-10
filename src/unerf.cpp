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
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/file.h"
#include "src/common/filepath.h"
#include "src/common/hash.h"

#include "src/aurora/util.h"
#include "src/aurora/erffile.h"

#include "src/util.h"
#include "src/files_dragonage.h"

enum Command {
	kCommandNone        = -1,
	kCommandInfo        =  0,
	kCommandList            ,
	kCommandListVerbose     ,
	kCommandExtract         ,
	kCommandMAX
};

const char *kCommandChar[kCommandMAX] = { "i", "l", "v", "e" };

void printUsage(FILE *stream, const char *name);
bool parseCommandLine(int argc, char **argv, int &returnValue,
                      Command &command, Common::UString &file, Aurora::GameID &game);

bool findHashedName(uint64 hash, Common::UString &name);

void displayInfo(Aurora::ERFFile &erf);
void listFiles(Aurora::ERFFile &rim, Aurora::GameID game);
void listVerboseFiles(Aurora::ERFFile &rim, Aurora::GameID game);
void extractFiles(Aurora::ERFFile &rim, Aurora::GameID);

int main(int argc, char **argv) {
	Aurora::GameID game = Aurora::kGameIDUnknown;

	int returnValue;
	Command command;
	Common::UString file;
	if (!parseCommandLine(argc, argv, returnValue, command, file, game))
		return returnValue;

	try {
		Aurora::ERFFile erf(new Common::File(file));

		if      (command == kCommandInfo)
			displayInfo(erf);
		else if (command == kCommandList)
			listFiles(erf, game);
		else if (command == kCommandListVerbose)
			listVerboseFiles(erf, game);
		else if (command == kCommandExtract)
			extractFiles(erf, game);

	} catch (Common::Exception &e) {
		Common::printException(e);
		return -1;
	} catch (std::exception &e) {
		error("%s", e.what());
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
	std::fprintf(stream, "Usage: %s [<options>] <command> <file>\n\n", name);
	std::fprintf(stream, "Options:\n");
	std::fprintf(stream, "  --nwn2     Alias file types according to Neverwinter Nights 2 rules\n");
	std::fprintf(stream, "  --jade     Alias file types according to Jade Empire rules\n\n");
	std::fprintf(stream, "Commands:\n");
	std::fprintf(stream, "  i          Display meta-information\n");
	std::fprintf(stream, "  l          List archive\n");
	std::fprintf(stream, "  v          List archive verbosely (show directory names)\n");
	std::fprintf(stream, "  e          Extract files to current directory\n");
}

bool findHashedName(uint64 hash, Common::UString &name) {
	const char *fileName = findDragonAgeFile(hash);
	if (fileName) {
		name = Common::FilePath::changeExtension(fileName, "");
		return true;
	}

	name = Common::formatHash(hash);
	return false;
}

void displayInfo(Aurora::ERFFile &erf) {
	std::printf("Version: %s\n", Common::debugTag(erf.getVersion()).c_str());
	std::printf("Build Year: %d\n", erf.getBuildYear());
	std::printf("Build Day: %d\n", erf.getBuildDay());
	std::printf("Number of files: %u\n", (uint)erf.getResources().size());

	const Aurora::LocString &description = erf.getDescription();
	if (description.getString().empty() && (description.getID() == Aurora::kStrRefInvalid))
		return;

	std::printf("\nDescription:\n");
	std::printf("String reference ID: %u\n", description.getID());

	std::vector<Aurora::LocString::SubLocString> str;
	description.getStrings(str);

	for (std::vector<Aurora::LocString::SubLocString>::iterator s = str.begin(); s != str.end(); ++s) {
		std::printf("\n.=== Description in language %u: ===\n", s->language);
		std::printf("%s\n", s->str.c_str());
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
		const Aurora::FileType type = TypeMan.aliasFileType(r->type, game);

		Common::UString name = r->name;
		if (name.empty())
			findHashedName(r->hash, name);

		name = Common::FilePath::getFile(name);

		std::printf("%32s%-4s | %10d\n", name.c_str(), TypeMan.setFileType("", type).c_str(),
		                                 erf.getResourceSize(r->index));
	}
}

struct FileEntry {
	Common::UString file;
	uint32 size;

	FileEntry(const Common::UString &f = "", uint32 s = -1) : file(f), size(s) { }
};

void listVerboseFiles(Aurora::ERFFile &erf, Aurora::GameID game) {
	const Aurora::Archive::ResourceList &resources = erf.getResources();
	const uint32 fileCount = resources.size();

	std::printf("Number of files: %u\n\n", fileCount);

	std::vector<FileEntry> fileEntries;
	fileEntries.reserve(fileCount);

	uint32 nameLength = 10;
	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r) {
		const Aurora::FileType type = TypeMan.aliasFileType(r->type, game);

		Common::UString name = r->name;
		if (name.empty())
			findHashedName(r->hash, name);

		name.replaceAll('\\', '/');

		name = TypeMan.addFileType(name, type);

		nameLength = MAX<uint32>(nameLength, name.size() + 1);

		fileEntries.push_back(FileEntry(name, erf.getResourceSize(r->index)));
	}

	if ((nameLength % 2) == 1)
		nameLength++;

	std::printf("%sFileName%s|    Size\n", Common::UString(' ', (nameLength - 8) / 2).c_str(),
	                                       Common::UString(' ', (nameLength - 8) / 2).c_str());
	std::printf("%s|===========\n", Common::UString('=', nameLength).c_str());

	for (std::vector<FileEntry>::const_iterator f = fileEntries.begin(); f != fileEntries.end(); ++f)
		std::printf("%-*s| %10d\n", nameLength, f->file.c_str(), f->size);
}

void extractFiles(Aurora::ERFFile &erf, Aurora::GameID game) {
	const Aurora::Archive::ResourceList &resources = erf.getResources();
	const uint32 fileCount = resources.size();

	std::printf("Number of files: %u\n\n", fileCount);

	uint i = 1;
	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r, ++i) {
		Common::UString name = r->name;
		if (name.empty())
			findHashedName(r->hash, name);

		const Aurora::FileType type     = TypeMan.aliasFileType(r->type, game);
		const Common::UString  fileName = TypeMan.addFileType(name, type);

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
