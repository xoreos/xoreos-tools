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
 *  Tool to extract RIM archives.
 */

#include <cstring>
#include <cstdio>

#include "src/version/version.h"

#include "src/common/scopedptr.h"
#include "src/common/ustring.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/readstream.h"
#include "src/common/readfile.h"
#include "src/common/cli.h"

#include "src/aurora/util.h"
#include "src/aurora/rimfile.h"

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
                      Command &command, Common::UString &file, Aurora::GameID &game);

void extractFiles(Aurora::RIMFile &rim, Aurora::GameID);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		Aurora::GameID game = Aurora::kGameIDUnknown;

		int returnValue = 1;
		Command command = kCommandNone;
		Common::UString file;

		if (!parseCommandLine(args, returnValue, command, file, game))
			return returnValue;

		Aurora::RIMFile rim(new Common::ReadFile(file));

		if      (command == kCommandList)
			Archives::listFiles(rim, game, false);
		else if (command == kCommandExtract)
			extractFiles(rim, game);

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
                      Command &command, Common::UString &file, Aurora::GameID &game) {

	using Common::CLI::NoOption;
	using Common::CLI::kContinueParsing;
	using Common::CLI::Parser;
	using Common::CLI::ValGetter;
	using Common::CLI::ValAssigner;
	using Common::CLI::makeEndArgs;
	using Common::CLI::makeAssigners;

	NoOption cmdOpt(false, new ValGetter<Command &>(command, "command"));
	NoOption fileOpt(false, new ValGetter<Common::UString &>(file, "file"));
	Parser parser(argv[0], "BioWare RIM archive extractor",
	              "Commands:\n"
	              "  l          List archive\n"
	              "  e          Extract files to current directory\n",
	              returnValue,
	              makeEndArgs(&cmdOpt, &fileOpt));

	parser.addSpace();
	parser.addOption("nwn2", "Alias file types according to Neverwinter Nights 2 rules",
	                 kContinueParsing,
	                 makeAssigners(new ValAssigner<Aurora::GameID>(Aurora::kGameIDNWN2, game)));
	parser.addOption("jade", "Alias file types according to Jade Empire rules",
	                 kContinueParsing,
	                 makeAssigners(new ValAssigner<Aurora::GameID>(Aurora::kGameIDJade, game)));

	return parser.process(argv);
}

void extractFiles(Aurora::RIMFile &rim, Aurora::GameID game) {
	const Aurora::Archive::ResourceList &resources = rim.getResources();
	const size_t fileCount = resources.size();

	std::printf("Number of files: %u\n\n", (uint)fileCount);

	size_t i = 1;
	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r, ++i) {
		const Aurora::FileType type     = TypeMan.aliasFileType(r->type, game);
		const Common::UString  fileName = TypeMan.setFileType(r->name, type);

		std::printf("Extracting %u/%u: %s ... ", (uint)i, (uint)fileCount, fileName.c_str());

		try {
			Common::ScopedPtr<Common::SeekableReadStream> stream(rim.getResource(r->index));

			dumpStream(*stream, fileName);

			std::printf("Done\n");
		} catch (Common::Exception &e) {
			Common::printException(e, "");
		}
	}

}
