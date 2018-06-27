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

#include "src/version/version.h"

#include "src/common/scopedptr.h"
#include "src/common/ustring.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/readstream.h"
#include "src/common/readfile.h"
#include "src/common/cli.h"

#include "src/aurora/util.h"
#include "src/aurora/ndsrom.h"

#include "src/util.h"

enum Command {
	kCommandNone    = -1,
	kCommandInfo    =  0,
	kCommandList        ,
	kCommandExtract     ,
	kCommandMAX
};

const char *kCommandChar[kCommandMAX] = { "i", "l", "e" };

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Command &command, Common::UString &file);

void displayInfo(Aurora::NDSFile &nds);
void listFiles(Aurora::NDSFile &nds);
void extractFiles(Aurora::NDSFile &nds);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		int returnValue = 1;
		Command command = kCommandNone;
		Common::UString file;

		if (!parseCommandLine(args, returnValue, command, file))
			return returnValue;

		Aurora::NDSFile nds(new Common::ReadFile(file));

		if      (command == kCommandInfo)
			displayInfo(nds);
		else if (command == kCommandList)
			listFiles(nds);
		else if (command == kCommandExtract)
			extractFiles(nds);

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
                      Command &command, Common::UString &file) {
	using Common::CLI::NoOption;
	using Common::CLI::kContinueParsing;
	using Common::CLI::Parser;
	using Common::CLI::ValGetter;
	using Common::CLI::makeEndArgs;

	NoOption cmdOpt(false, new ValGetter<Command &>(command, "command"));
	NoOption fileOpt(false, new ValGetter<Common::UString &>(file, "file"));
	Parser parser(argv[0], "Nintendo DS archive extractor",
	              "Commands:\n"
	              "  i          Display meta-information\n"
	              "  l          List archive\n"
	              "  e          Extract files to current directory\n",
	              returnValue, makeEndArgs(&cmdOpt, &fileOpt));

	return parser.process(argv);
}

void displayInfo(Aurora::NDSFile &nds) {
	std::printf("Game name: \"%s\"\n", nds.getTitle().c_str());
	std::printf("Game code: \"%s\"\n", nds.getCode().c_str());
	std::printf("Game maker: \"%s\"\n", nds.getMaker().c_str());
}

void listFiles(Aurora::NDSFile &nds) {
	const Aurora::Archive::ResourceList &resources = nds.getResources();
	const size_t fileCount = resources.size();

	std::printf("Number of files: %u\n\n", (uint)fileCount);

	std::printf("               Filename                |    Size\n");
	std::printf("=======================================|===========\n");

	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r) {
		std::printf("%32s%-6s | %10d\n", r->name.c_str(), TypeMan.setFileType("", r->type).c_str(),
		                               nds.getResourceSize(r->index));
	}
}

void extractFiles(Aurora::NDSFile &nds) {
	const Aurora::Archive::ResourceList &resources = nds.getResources();
	const size_t fileCount = resources.size();

	std::printf("Number of files: %u\n\n", (uint)fileCount);

	size_t i = 1;
	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r, ++i) {
		const Common::UString fileName = TypeMan.setFileType(r->name, r->type);

		std::printf("Extracting %u/%u: %s ... ", (uint)i, (uint)fileCount, fileName.c_str());

		try {
			Common::ScopedPtr<Common::SeekableReadStream> stream(nds.getResource(r->index));

			dumpStream(*stream, fileName);

			std::printf("Done\n");
		} catch (Common::Exception &e) {
			Common::printException(e, "");
		}
	}

}
