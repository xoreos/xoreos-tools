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

#include "src/version/version.h"

#include "src/common/ptrvector.h"
#include "src/common/util.h"
#include "src/common/ustring.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/readfile.h"
#include "src/common/filepath.h"
#include "src/common/cli.h"

#include "src/aurora/util.h"
#include "src/aurora/keyfile.h"
#include "src/aurora/keydatafile.h"
#include "src/aurora/biffile.h"
#include "src/aurora/bzffile.h"

#include "src/archives/util.h"

#include "src/util.h"

enum Command {
	kCommandNone    = -1,
	kCommandList    =  0,
	kCommandExtract     ,
	kCommandMAX
};

const char *kCommandChar[kCommandMAX] = { "l", "e" };

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Command &command, std::list<Common::UString> &files, Aurora::GameID &game);

uint32_t getFileID(const Common::UString &fileName);
void identifyFiles(const std::list<Common::UString> &files, std::vector<Common::UString> &keyFiles,
                   std::vector<Common::UString> &bifFiles);

void openKEYs(const std::vector<Common::UString> &keyFiles, Common::PtrVector<Aurora::KEYFile> &keys);
void openKEYDataFiles(const std::vector<Common::UString> &dataFiles, Common::PtrVector<Aurora::KEYDataFile> &keyData);

void mergeKEYDataFiles(Common::PtrVector<Aurora::KEYFile> &keys, Common::PtrVector<Aurora::KEYDataFile> &keyData,
                       const std::vector<Common::UString> &dataFiles);

void listFiles(const Common::PtrVector<Aurora::KEYFile> &keys, const std::vector<Common::UString> &keyFiles, Aurora::GameID game);
void extractFiles(const Common::PtrVector<Aurora::KEYDataFile> &keyData, const std::vector<Common::UString> &dataFiles, Aurora::GameID game);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		Aurora::GameID game = Aurora::kGameIDUnknown;

		int returnValue = 1;
		Command command = kCommandNone;
		std::list<Common::UString> files;

		if (!parseCommandLine(args, returnValue, command, files, game))
			return returnValue;

		std::vector<Common::UString> keyFiles, dataFiles;
		identifyFiles(files, keyFiles, dataFiles);

		Common::PtrVector<Aurora::KEYFile> keys;
		Common::PtrVector<Aurora::KEYDataFile> keyData;

		openKEYs(keyFiles, keys);
		openKEYDataFiles(dataFiles, keyData);

		mergeKEYDataFiles(keys, keyData, dataFiles);

		if      (command == kCommandList)
			listFiles(keys, keyFiles, game);
		else if (command == kCommandExtract)
			extractFiles(keyData, dataFiles, game);

	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

namespace Common {
namespace CLI {
template<>
int ValGetter<Command &>::get(const std::vector<Common::UString> &args, int i, int) {
	_val = kCommandNone;
	for (int j = 0; j < kCommandMAX; j++) {
		if (!strcmp(args[i].c_str(), kCommandChar[j])) {
			_val = (Command) j;
			return 0;
		}
	}
	return -1;
}
}
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Command &command, std::list<Common::UString> &files, Aurora::GameID &game) {

	using Common::CLI::NoOption;
	using Common::CLI::Parser;
	using Common::CLI::ValGetter;
	using Common::CLI::makeEndArgs;
	using Common::CLI::ValAssigner;
	using Common::CLI::makeAssigners;

	NoOption cmdOpt(false, new ValGetter<Command &>(command, "command"));
	NoOption filesOpt(false, new ValGetter<std::list<Common::UString> &>(files, "files[...]"));
	Parser parser(argv[0], "BioWare KEY/BIF archive extractor",
	              "Commands:\n"
	              "  l          List files indexed in KEY archive(s)\n"
	              "  e          Extract BIF archive(s). Needs KEY file(s) indexing these BIF.\n\n"
	              "Examples:\n"
	              "unkeybif l foo.key\n"
	              "unkeybif l foo.key bar.key\n"
	              "unkeybif e foo.bif bar.key\n"
	              "unkeybif e foo.bif quux.bif bar.key\n"
	              "unkeybif e foo.bif quux.bif bar.key foobar.key",
	              returnValue, makeEndArgs(&cmdOpt, &filesOpt));

	parser.addSpace();
	parser.addOption("nwn2", "Alias file types according to Neverwinter Nights 2 rules",
	                 Common::CLI::kContinueParsing,
	                 makeAssigners(new ValAssigner<Aurora::GameID>(Aurora::kGameIDNWN2, game)));
	parser.addOption("jade", "Alias file types according to Jade Empire rules",
	                 Common::CLI::kContinueParsing,
	                 makeAssigners(new ValAssigner<Aurora::GameID>(Aurora::kGameIDJade, game)));

	return parser.process(argv);
}

uint32_t getFileID(const Common::UString &fileName) {
	Common::ReadFile file(fileName);

	return file.readUint32BE();
}

void identifyFiles(const std::list<Common::UString> &files, std::vector<Common::UString> &keyFiles,
                   std::vector<Common::UString> &dataFiles) {

	keyFiles.reserve(files.size());
	dataFiles.reserve(files.size());

	for (std::list<Common::UString>::const_iterator f = files.begin(); f != files.end(); ++f) {
		uint32_t id = getFileID(*f);

		if      (id == MKTAG('K', 'E', 'Y', ' '))
			keyFiles.push_back(*f);
		else if (id == MKTAG('B', 'I', 'F', 'F'))
			dataFiles.push_back(*f);
		else
			throw Common::Exception("File \"%s\" is neither a KEY nor a BIF/BZF", f->c_str());
	}
}

void openKEYs(const std::vector<Common::UString> &keyFiles, Common::PtrVector<Aurora::KEYFile> &keys) {
	keys.reserve(keyFiles.size());

	for (std::vector<Common::UString>::const_iterator f = keyFiles.begin(); f != keyFiles.end(); ++f) {
		Common::ReadFile key(*f);

		keys.push_back(new Aurora::KEYFile(key));
	}
}

void openKEYDataFiles(const std::vector<Common::UString> &dataFiles, Common::PtrVector<Aurora::KEYDataFile> &keyData) {
	keyData.reserve(dataFiles.size());

	for (std::vector<Common::UString>::const_iterator f = dataFiles.begin(); f != dataFiles.end(); ++f) {
		if (Common::FilePath::getExtension(*f).equalsIgnoreCase(".bzf"))
			keyData.push_back(new Aurora::BZFFile(new Common::ReadFile(*f)));
		else
			keyData.push_back(new Aurora::BIFFile(new Common::ReadFile(*f)));
	}
}

void mergeKEYDataFiles(Common::PtrVector<Aurora::KEYFile> &keys, Common::PtrVector<Aurora::KEYDataFile> &keyData,
                       const std::vector<Common::UString> &dataFiles) {

	// Go over all KEYs
	for (Common::PtrVector<Aurora::KEYFile>::iterator k = keys.begin(); k != keys.end(); ++k) {

		// Go over all BIFs/BZFs handled by the KEY
		const Aurora::KEYFile::BIFList &keyBifs = (*k)->getBIFs();
		for (size_t kb = 0; kb < keyBifs.size(); kb++) {

			// Go over all BIFs
			for (size_t b = 0; b < dataFiles.size(); b++) {

				// If they match, merge
				if (Common::FilePath::getStem(keyBifs[kb]).equalsIgnoreCase(Common::FilePath::getStem(dataFiles[b])))
					keyData[b]->mergeKEY(**k, kb);

			}

		}

	}

}

void listFiles(const Common::PtrVector<Aurora::KEYFile> &keys,
               const std::vector<Common::UString> &keyFiles, Aurora::GameID game) {

	for (size_t i = 0; i < keys.size(); i++) {
		Archives::listFiles(*keys[i], keyFiles[i], game);

		if (i < (keys.size() - 1))
			std::printf("\n");
	}
}

void extractFiles(const Common::PtrVector<Aurora::KEYDataFile> &keyData,
                  const std::vector<Common::UString> &dataFiles, Aurora::GameID game) {

	for (size_t i = 0; i < keyData.size(); i++) {
		std::printf("%s: %s indexed files (of %u)\n\n", dataFiles[i].c_str(),
		            Common::composeString(keyData[i]->getResources().size()).c_str(),
                keyData[i]->getInternalResourceCount());

		Archives::extractFiles(*keyData[i], game, false, std::set<Common::UString>());

		if (i < (keyData.size() - 1))
			std::printf("\n");
	}
}
