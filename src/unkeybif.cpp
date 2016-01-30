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
 *  Tool to extract KEY/BIF archives.
 */

#include <cstring>
#include <cstdio>
#include <list>
#include <vector>

#include "src/common/version.h"
#include "src/common/util.h"
#include "src/common/ustring.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/readstream.h"
#include "src/common/readfile.h"
#include "src/common/filepath.h"

#include "src/aurora/util.h"
#include "src/aurora/keyfile.h"
#include "src/aurora/biffile.h"

#include "src/util.h"

enum Command {
	kCommandNone    = -1,
	kCommandList    =  0,
	kCommandExtract     ,
	kCommandMAX
};

const char *kCommandChar[kCommandMAX] = { "l", "e" };

void printUsage(FILE *stream, const Common::UString &name);
bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Command &command, std::list<Common::UString> &files, Aurora::GameID &game);

uint32 getFileID(const Common::UString &fileName);
void identifyFiles(const std::list<Common::UString> &files, std::vector<Common::UString> &keyFiles,
                   std::vector<Common::UString> &bifFiles);

void openKEYs(const std::vector<Common::UString> &keyFiles, std::vector<Aurora::KEYFile *> &keys);
void openBIFs(const std::vector<Common::UString> &bifFiles, std::vector<Aurora::BIFFile *> &bifs);

void mergeKEYBIF(std::vector<Aurora::KEYFile *> &keys, std::vector<Aurora::BIFFile *> &bifs,
                 const std::vector<Common::UString> &bifFiles);

void listFiles(const Aurora::KEYFile &key, Aurora::GameID game);
void listFiles(const std::vector<Aurora::KEYFile *> &keys, const std::vector<Common::UString> &keyFiles, Aurora::GameID game);
void extractFiles(const Aurora::BIFFile &bif, Aurora::GameID game);
void extractFiles(const std::vector<Aurora::BIFFile *> &bifs, const std::vector<Common::UString> &bifFiles, Aurora::GameID game);

int main(int argc, char **argv) {
	std::vector<Aurora::KEYFile *> keys;
	std::vector<Aurora::BIFFile *> bifs;

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		Aurora::GameID game = Aurora::kGameIDUnknown;

		int returnValue = 1;
		Command command = kCommandNone;
		std::list<Common::UString> files;

		if (!parseCommandLine(args, returnValue, command, files, game))
			return returnValue;

		std::vector<Common::UString> keyFiles, bifFiles;
		identifyFiles(files, keyFiles, bifFiles);

		openKEYs(keyFiles, keys);

		openBIFs(bifFiles, bifs);

		mergeKEYBIF(keys, bifs, bifFiles);

		if      (command == kCommandList)
			listFiles(keys, keyFiles, game);
		else if (command == kCommandExtract)
			extractFiles(bifs, bifFiles, game);

	} catch (...) {
		for (std::vector<Aurora::KEYFile *>::iterator k = keys.begin(); k != keys.end(); ++k)
			delete *k;
		for (std::vector<Aurora::BIFFile *>::iterator b = bifs.begin(); b != bifs.end(); ++b)
			delete *b;

		Common::exceptionDispatcherError();
	}

	for (std::vector<Aurora::KEYFile *>::iterator k = keys.begin(); k != keys.end(); ++k)
		delete *k;
	for (std::vector<Aurora::BIFFile *>::iterator b = bifs.begin(); b != bifs.end(); ++b)
		delete *b;

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Command &command, std::list<Common::UString> &files, Aurora::GameID &game) {

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
	files.insert(files.end(), arg, args.end());

	return true;
}

void printUsage(FILE *stream, const Common::UString &name) {
	std::fprintf(stream, "BioWare KEY/BIF archive extractor\n\n");
	std::fprintf(stream, "Usage: %s [<options>] <command> <file> [...]\n\n", name.c_str());
	std::fprintf(stream, "Options:\n");
	std::fprintf(stream, "  -h      --help     This help text\n");
	std::fprintf(stream, "          --version  Display version information\n");
	std::fprintf(stream, "          --nwn2     Alias file types according to Neverwinter Nights 2 rules\n");
	std::fprintf(stream, "          --jade     Alias file types according to Jade Empire rules\n\n");
	std::fprintf(stream, "Commands:\n");
	std::fprintf(stream, "  l          List files indexed in KEY archive(s)\n");
	std::fprintf(stream, "  e          Extract BIF archive(s). Needs KEY file(s) indexing these BIF.\n\n");
	std::fprintf(stream, "Examples:\n");
	std::fprintf(stream, "%s l foo.key\n", name.c_str());
	std::fprintf(stream, "%s l foo.key bar.key\n", name.c_str());
	std::fprintf(stream, "%s e foo.bif bar.key\n", name.c_str());
	std::fprintf(stream, "%s e foo.bif quux.bif bar.key\n", name.c_str());
	std::fprintf(stream, "%s e foo.bif quux.bif bar.key foobar.key\n", name.c_str());
}

uint32 getFileID(const Common::UString &fileName) {
	Common::ReadFile file(fileName);

	return file.readUint32BE();
}

void identifyFiles(const std::list<Common::UString> &files, std::vector<Common::UString> &keyFiles,
                   std::vector<Common::UString> &bifFiles) {

	keyFiles.reserve(files.size());
	bifFiles.reserve(files.size());

	for (std::list<Common::UString>::const_iterator f = files.begin(); f != files.end(); ++f) {
		uint32 id = getFileID(*f);

		if      (id == MKTAG('K', 'E', 'Y', ' '))
			keyFiles.push_back(*f);
		else if (id == MKTAG('B', 'I', 'F', 'F'))
			bifFiles.push_back(*f);
		else
			throw Common::Exception("File \"%s\" is neither a KEY nor a BIF", f->c_str());
	}
}

void openKEYs(const std::vector<Common::UString> &keyFiles, std::vector<Aurora::KEYFile *> &keys) {
	keys.reserve(keyFiles.size());

	for (std::vector<Common::UString>::const_iterator f = keyFiles.begin(); f != keyFiles.end(); ++f) {
		Common::ReadFile key(*f);

		keys.push_back(new Aurora::KEYFile(key));
	}
}

void openBIFs(const std::vector<Common::UString> &bifFiles, std::vector<Aurora::BIFFile *> &bifs) {
	bifs.reserve(bifFiles.size());

	for (std::vector<Common::UString>::const_iterator f = bifFiles.begin(); f != bifFiles.end(); ++f)
		bifs.push_back(new Aurora::BIFFile(new Common::ReadFile(*f)));
}

void mergeKEYBIF(std::vector<Aurora::KEYFile *> &keys, std::vector<Aurora::BIFFile *> &bifs,
                 const std::vector<Common::UString> &bifFiles) {

	// Go over all KEYs
	for (std::vector<Aurora::KEYFile *>::iterator k = keys.begin(); k != keys.end(); ++k) {

		// Go over all BIFs handled by the KEY
		const Aurora::KEYFile::BIFList &keyBifs = (*k)->getBIFs();
		for (uint kb = 0; kb < keyBifs.size(); kb++) {

			// Go over all BIFs
			for (uint b = 0; b < bifFiles.size(); b++) {

				// If they match, merge
				if (Common::FilePath::getFile(keyBifs[kb]).equalsIgnoreCase(Common::FilePath::getFile(bifFiles[b])))
					bifs[b]->mergeKEY(**k, kb);

			}

		}

	}

}

void listFiles(const Aurora::KEYFile &key, Aurora::GameID game) {
	const Aurora::KEYFile::ResourceList &resources = key.getResources();

	std::printf("              Filename               | BIF\n");
	std::printf("=====================================|=====");

	const Aurora::KEYFile::BIFList &bifs = key.getBIFs();

	size_t maxBIFLength = 0;
	for (Aurora::KEYFile::BIFList::const_iterator b = bifs.begin(); b != bifs.end(); ++b)
		maxBIFLength = MAX<size_t>(maxBIFLength, b->size());

	for (size_t i = 4; i < maxBIFLength; i++)
		std::printf("=");

	std::printf("\n");

	for (Aurora::KEYFile::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r) {
		const Aurora::FileType type = TypeMan.aliasFileType(r->type, game);

		std::printf("%32s%s | %s\n", r->name.c_str(), TypeMan.setFileType("", type).c_str(),
		                             (r->bifIndex < bifs.size()) ? bifs[r->bifIndex].c_str() : "");
	}
}

void listFiles(const std::vector<Aurora::KEYFile *> &keys, const std::vector<Common::UString> &keyFiles, Aurora::GameID game) {
	for (uint i = 0; i < keys.size(); i++) {
		std::printf("%s: %u files\n\n", keyFiles[i].c_str(), (uint)keys[i]->getResources().size());
		listFiles(*keys[i], game);

		if (i < (keys.size() - 1))
			std::printf("\n");
	}
}

void extractFiles(const Aurora::BIFFile &bif, Aurora::GameID game) {
	const Aurora::Archive::ResourceList &resources = bif.getResources();

	uint i = 1;
	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r, ++i) {
		const Aurora::FileType type     = TypeMan.aliasFileType(r->type, game);
		const Common::UString  fileName = TypeMan.setFileType(r->name, type);

		std::printf("Extracting %u/%u: %s ... ", i, (uint) resources.size(), fileName.c_str());

		Common::SeekableReadStream *stream = 0;
		try {
			stream = bif.getResource(r->index);

			dumpStream(*stream, fileName);

			std::printf("Done\n");
		} catch (Common::Exception &e) {
			Common::printException(e, "");
		}

		delete stream;
	}

}

void extractFiles(const std::vector<Aurora::BIFFile *> &bifs, const std::vector<Common::UString> &bifFiles, Aurora::GameID game) {
	for (uint i = 0; i < bifs.size(); i++) {
		std::printf("%s: %u indexed files (of %u)\n\n", bifFiles[i].c_str(), (uint)bifs[i]->getResources().size(),
                bifs[i]->getInternalResourceCount());

		extractFiles(*bifs[i], game);

		if (i < (bifs.size() - 1))
			std::printf("\n");
	}
}
