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
 *  Tool to extract NSBTX textures into TGA images.
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
#include "src/aurora/nsbtxfile.h"

#include "src/images/xoreositex.h"

#include "src/util.h"

enum Command {
	kCommandNone    = -1,
	kCommandList    =  0,
	kCommandExtract     ,
	kCommandMAX
};

const char *kCommandChar[kCommandMAX] = { "l", "e" };

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Command &command, Common::UString &file);

void listFiles(Aurora::NSBTXFile &nsbtx);
void extractFiles(Aurora::NSBTXFile &nsbtx);

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

		Aurora::NSBTXFile nsbtx(new Common::ReadFile(file));

		if      (command == kCommandList)
			listFiles(nsbtx);
		else if (command == kCommandExtract)
			extractFiles(nsbtx);

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
	Parser parser(argv[0], "Nintendo NSBTX texture extractor",
		      "Commands:\n"
		      "  l          List archive\n"
		      "  e          Extract files to current directory\n",
		      returnValue, makeEndArgs(&cmdOpt, &fileOpt));

	return parser.process(argv);
}

void listFiles(Aurora::NSBTXFile &nsbtx) {
	const Aurora::Archive::ResourceList &resources = nsbtx.getResources();
	const size_t fileCount = resources.size();

	std::printf("Number of files: %u\n\n", (uint)fileCount);

	std::printf("      Filename       |    Size\n");
	std::printf("=====================|===========\n");

	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r)
		std::printf("%16s.tga | %10d\n", r->name.c_str(), nsbtx.getResourceSize(r->index));
}

void dumpImage(Common::SeekableReadStream &stream, const Common::UString &fileName) {
	Images::XEOSITEX itex(stream);

	itex.flipVertically();

	itex.dumpTGA(fileName);
}

void extractFiles(Aurora::NSBTXFile &nsbtx) {
	const Aurora::Archive::ResourceList &resources = nsbtx.getResources();
	const size_t fileCount = resources.size();

	std::printf("Number of files: %u\n\n", (uint)fileCount);

	size_t i = 1;
	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r, ++i) {
		const Common::UString fileName = r->name + ".tga";

		std::printf("Extracting %u/%u: %s ... ", (uint)i, (uint)fileCount, fileName.c_str());

		try {
			Common::ScopedPtr<Common::SeekableReadStream> stream(nsbtx.getResource(r->index));

			dumpImage(*stream, fileName);

			std::printf("Done\n");
		} catch (Common::Exception &e) {
			std::printf("\n");
			Common::printException(e, "");
		}
	}

}
