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

#include <vector>
#include <set>

#include <cstring>
#include <cstdio>

#include "src/common/version.h"
#include "src/common/ustring.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/memreadstream.h"
#include "src/common/memwritestream.h"
#include "src/common/readfile.h"
#include "src/common/filepath.h"
#include "src/common/hash.h"
#include "src/common/md5.h"

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
	kCommandExtractSub      ,
	kCommandMAX
};

enum ExtractMode {
	kExtractModeStrip,
	kExtractModeSubstitute
};

const char *kCommandChar[kCommandMAX] = { "i", "l", "v", "e", "s" };

void printUsage(FILE *stream, const Common::UString &name);
bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Command &command, Common::UString &archive, std::set<Common::UString> &files,
                      Aurora::GameID &game, std::vector<byte> &password);

bool findHashedName(uint64 hash, Common::UString &name);

void parsePassword(const Common::UString &arg, std::vector<byte> &password);
void readNWMMD5   (const Common::UString &arg, std::vector<byte> &password);

void displayInfo(Aurora::ERFFile &erf);
void listFiles(Aurora::ERFFile &erf, Aurora::GameID game);
void listVerboseFiles(Aurora::ERFFile &erf, Aurora::GameID game);
void extractFiles(Aurora::ERFFile &erf, Aurora::GameID game,
                  std::set<Common::UString> &files, ExtractMode mode);

int main(int argc, char **argv) {
	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		Aurora::GameID game = Aurora::kGameIDUnknown;

		int returnValue = 1;
		Command command = kCommandNone;
		Common::UString archive;
		std::set<Common::UString> files;
		std::vector<byte> password;

		if (!parseCommandLine(args, returnValue, command, archive, files, game, password))
			return returnValue;

		Aurora::ERFFile erf(new Common::ReadFile(archive), password);

		if      (command == kCommandInfo)
			displayInfo(erf);
		else if (command == kCommandList)
			listFiles(erf, game);
		else if (command == kCommandListVerbose)
			listVerboseFiles(erf, game);
		else if (command == kCommandExtract)
			extractFiles(erf, game, files, kExtractModeStrip);
		else if (command == kCommandExtractSub)
			extractFiles(erf, game, files, kExtractModeSubstitute);

	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

void parsePassword(const Common::UString &arg, std::vector<byte> &password) {
	const size_t length = arg.size();

	password.clear();
	password.reserve(length / 2);

	size_t i = 0;
	byte c = 0x00;
	for (Common::UString::iterator s = arg.begin(); s != arg.end(); ++s, i++) {
		byte d = 0;

		if      (*s >= '0' && *s <= '9')
			d = *s - '0';
		else if (*s >= 'a' && *s <= 'f')
			d = *s - 'a' + 10;
		else if (*s >= 'A' && *s <= 'F')
			d = *s - 'A' + 10;
		else
			throw Common::Exception("0x%08X is not a valid hex digit", (uint) *s);

		if ((i % 2) == 1) {
			c |= d;

			password.push_back(c);

			c = 0x00;
		} else
			c |= d << 4;
	}
}

void readNWMMD5(const Common::UString &arg, std::vector<byte> &password) {
	Common::ReadFile keyFile(arg);

	Common::hashMD5(keyFile, password);
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Command &command, Common::UString &archive, std::set<Common::UString> &files,
                      Aurora::GameID &game, std::vector<byte> &password) {

	archive.clear();
	files.clear();

	std::vector<Common::UString> args;

	bool optionsEnd = false;
	for (size_t i = 1; i < argv.size(); i++) {
		bool isOption = false;

		// A "--" marks an end to all options
		if (argv[i] == "--") {
			optionsEnd = true;
			continue;
		}

		// We're still handling options
		if (!optionsEnd) {
			// Help text
			if ((argv[i] == "-h") || (argv[i] == "--help")) {
				printUsage(stdout, argv[0]);
				returnValue = 0;

				return false;
			}

			if (argv[i] == "--version") {
				printVersion();
				returnValue = 0;

				return false;
			}

			if        (argv[i] == "--nwn2") {
				isOption = true;
				game     = Aurora::kGameIDNWN2;
			} else if (argv[i] == "--jade") {
				isOption = true;
			  game     = Aurora::kGameIDJade;
			} else if (argv[i] == "--pass") {
				isOption = true;

				// Needs the password as the next parameter
				if (i++ == (argv.size() - 1)) {
					printUsage(stdout, argv[0]);
					returnValue = 1;

					return false;
				}

				parsePassword(argv[i], password);

			} else if (argv[i] == "--nwm") {
				isOption = true;

				// Needs the file as the next parameter
				if (i++ == (argv.size() - 1)) {
					printUsage(stdout, argv[0]);
					returnValue = 1;

					return false;
				}

				readNWMMD5(argv[i], password);

			} else if (argv[i].beginsWith("-") || argv[i].beginsWith("--")) {
			  // An options, but we already checked for all known ones

				printUsage(stderr, argv[0]);
				returnValue = 1;

				return false;
			}
		}

		// Was this a valid option? If so, don't try to use it as a file
		if (isOption)
			continue;

		args.push_back(argv[i]);
	}

	if (args.size() < 2) {
		printUsage(stderr, argv[0]);
		returnValue = 1;

		return false;
	}

	std::vector<Common::UString>::iterator arg = args.begin();

	// Find out what we should do
	command = kCommandNone;
	for (int i = 0; i < kCommandMAX; i++)
		if (!strcmp(arg->c_str(), kCommandChar[i]))
			command = (Command) i;

	// Unknown command
	if (command == kCommandNone) {
		printUsage(stderr, argv[0]);
		returnValue = 1;

		return false;
	}

	++arg;
	archive = *arg;
	++arg;

	files.insert(arg, args.end());

	return true;
}

void printUsage(FILE *stream, const Common::UString &name) {
	std::fprintf(stream, "BioWare ERF (.erf, .mod, .nwm, .sav) archive extractor\n\n");
	std::fprintf(stream, "Usage: %s [<options>] <command> <archive> [<file> [...]]\n\n", name.c_str());
	std::fprintf(stream, "Options:\n");
	std::fprintf(stream, "  -h    --help        This help text\n");
	std::fprintf(stream, "        --version     Display version information\n");
	std::fprintf(stream, "        --nwn2        Alias file types according to Neverwinter Nights 2 rules\n");
	std::fprintf(stream, "        --jade        Alias file types according to Jade Empire rules\n");
	std::fprintf(stream, "        --pass <hex>  Decryption password, if required, in hex notation\n");
	std::fprintf(stream, "                      (e.g. \"4CF223AB\")\n");
	std::fprintf(stream, "        --nwm <file>  Neverwinter Nights premium module file\n");
	std::fprintf(stream, "                      (for decrypting their HAK file\n\n");
	std::fprintf(stream, "Commands:\n");
	std::fprintf(stream, "  i          Display meta-information\n");
	std::fprintf(stream, "  l          List archive\n");
	std::fprintf(stream, "  v          List archive verbosely (show directory names)\n");
	std::fprintf(stream, "  e          Extract files to current directory\n");
	std::fprintf(stream, "  s          Extract files to current directory with full name\n");
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
	const size_t fileCount = resources.size();

	std::printf("Number of files: %u\n\n", (uint)fileCount);

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

	FileEntry(const Common::UString &f = "", uint32 s = 0xFFFFFFFF) : file(f), size(s) { }
};

void listVerboseFiles(Aurora::ERFFile &erf, Aurora::GameID game) {
	const Aurora::Archive::ResourceList &resources = erf.getResources();
	const size_t fileCount = resources.size();

	std::printf("Number of files: %u\n\n", (uint)fileCount);

	std::vector<FileEntry> fileEntries;
	fileEntries.reserve(fileCount);

	size_t nameLength = 10;
	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r) {
		const Aurora::FileType type = TypeMan.aliasFileType(r->type, game);

		Common::UString name = r->name;
		if (name.empty())
			findHashedName(r->hash, name);

		name.replaceAll('\\', '/');

		name = TypeMan.addFileType(name, type);

		nameLength = MAX<size_t>(nameLength, name.size() + 1);

		fileEntries.push_back(FileEntry(name, erf.getResourceSize(r->index)));
	}

	if ((nameLength % 2) == 1)
		nameLength++;

	std::printf("%sFileName%s|    Size\n", Common::UString(' ', (nameLength - 8) / 2).c_str(),
	                                       Common::UString(' ', (nameLength - 8) / 2).c_str());
	std::printf("%s|===========\n", Common::UString('=', nameLength).c_str());

	for (std::vector<FileEntry>::const_iterator f = fileEntries.begin(); f != fileEntries.end(); ++f)
		std::printf("%-*s| %10d\n", (int)nameLength, f->file.c_str(), f->size);
}

void extractFiles(Aurora::ERFFile &erf, Aurora::GameID game,
                  std::set<Common::UString> &files, ExtractMode mode) {

	const Aurora::Archive::ResourceList &resources = erf.getResources();
	const size_t fileCount = resources.size();

	std::printf("Number of files: %u\n\n", (uint)fileCount);

	size_t i = 1;
	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r, ++i) {
		Common::UString name = r->name;
		if (name.empty())
			findHashedName(r->hash, name);

		name.replaceAll('\\', '/');

		if (mode == kExtractModeStrip)
			name = Common::FilePath::getFile(name);

		const Aurora::FileType type = TypeMan.aliasFileType(r->type, game);
		Common::UString fileName = TypeMan.addFileType(name, type);

		if (!files.empty() && (files.find(fileName) == files.end()))
			continue;

		if (mode == kExtractModeSubstitute)
			fileName.replaceAll('/', '=');

		std::printf("Extracting %u/%u: %s ... ", (uint)i, (uint)fileCount, fileName.c_str());

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
