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
#include <memory>

#include "src/version/version.h"

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

void openKEYs(const std::vector<Common::UString> &keyFiles, std::vector<std::unique_ptr<Aurora::KEYFile>> &keys);
void openKEYDataFiles(const std::vector<Common::UString> &dataFiles, std::vector<std::unique_ptr<Aurora::KEYDataFile>> &keyData);

void mergeKEYDataFiles(std::vector<std::unique_ptr<Aurora::KEYFile>> &keys, std::vector<std::unique_ptr<Aurora::KEYDataFile>> &keyData,
                       const std::vector<Common::UString> &dataFiles);

void listFiles(const std::vector<std::unique_ptr<Aurora::KEYFile>> &keys, const std::vector<Common::UString> &keyFiles, Aurora::GameID game);
void extractFiles(const std::vector<std::unique_ptr<Aurora::KEYDataFile>> &keyData, const std::vector<Common::UString> &dataFiles, Aurora::GameID game);

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

		std::vector<std::unique_ptr<Aurora::KEYFile>> keys;
		std::vector<std::unique_ptr<Aurora::KEYDataFile>> keyData;

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

	for (const auto &file : files) {
		uint32_t id = getFileID(file);

		if      (id == MKTAG('K', 'E', 'Y', ' '))
			keyFiles.push_back(file);
		else if (id == MKTAG('B', 'I', 'F', 'F'))
			dataFiles.push_back(file);
		else
			throw Common::Exception("File \"%s\" is neither a KEY nor a BIF/BZF", file.c_str());
	}
}

void openKEYs(const std::vector<Common::UString> &keyFiles, std::vector<std::unique_ptr<Aurora::KEYFile>> &keys) {
	keys.reserve(keyFiles.size());

	for (const auto &keyFile : keyFiles) {
		Common::ReadFile key(keyFile);

		keys.emplace_back(std::make_unique<Aurora::KEYFile>(key));
	}
}

void openKEYDataFiles(const std::vector<Common::UString> &dataFiles, std::vector<std::unique_ptr<Aurora::KEYDataFile>> &keyData) {
	keyData.reserve(dataFiles.size());

	for (const auto &dataFile : dataFiles) {
		if (Common::FilePath::getExtension(dataFile).equalsIgnoreCase(".bzf"))
			keyData.emplace_back(std::make_unique<Aurora::BZFFile>(new Common::ReadFile(dataFile)));
		else
			keyData.emplace_back(std::make_unique<Aurora::BIFFile>(new Common::ReadFile(dataFile)));
	}
}

void mergeKEYDataFiles(std::vector<std::unique_ptr<Aurora::KEYFile>> &keys, std::vector<std::unique_ptr<Aurora::KEYDataFile>> &keyData,
                       const std::vector<Common::UString> &dataFiles) {

	// Go over all KEYs
	for (auto &key : keys) {

		// Go over all BIFs/BZFs handled by the KEY
		const Aurora::KEYFile::BIFList &keyBifs = key->getBIFs();
		for (size_t keyBIFIndex = 0; keyBIFIndex < keyBifs.size(); keyBIFIndex++) {
			const Common::UString &keyBIF = keyBifs[keyBIFIndex];

			// Go over all BIFs
			for (size_t dataFileIndex = 0; dataFileIndex < dataFiles.size(); dataFileIndex++) {
				const Common::UString &dataFileName = dataFiles[dataFileIndex];
				Aurora::KEYDataFile &dataFile = *keyData[dataFileIndex];

				if (Common::FilePath::getStem(keyBIF).equalsIgnoreCase(Common::FilePath::getStem(dataFileName)))
					dataFile.mergeKEY(*key, keyBIFIndex);
			}

		}

	}

}

void listFiles(const std::vector<std::unique_ptr<Aurora::KEYFile>> &keys,
               const std::vector<Common::UString> &keyFiles, Aurora::GameID game) {

	for (size_t i = 0; i < keys.size(); i++) {
		Archives::listFiles(*keys[i], keyFiles[i], game);

		if (i < (keys.size() - 1))
			std::printf("\n");
	}
}

void extractFiles(const std::vector<std::unique_ptr<Aurora::KEYDataFile>> &keyData,
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
