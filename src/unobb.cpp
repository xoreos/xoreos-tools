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
 *  Tool to extract Aspyr's OBB virtual filesystems.
 */

#include <cstring>
#include <cstdio>

#include "src/version/version.h"

#include "src/common/scopedptr.h"
#include "src/common/ustring.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/filepath.h"
#include "src/common/readfile.h"
#include "src/common/cli.h"

#include "src/aurora/util.h"
#include "src/aurora/obbfile.h"

#include "src/archives/util.h"

#include "src/util.h"

enum Command {
	kCommandNone        = -1,
	kCommandList        =  0,
	kCommandListVerbose     ,
	kCommandExtract         ,
	kCommandExtractDir      ,
	kCommandMAX
};

const char *kCommandChar[kCommandMAX] = { "l", "v", "e", "x" };

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Command &command, Common::UString &archive, std::set<Common::UString> &files);

void extractFiles(Aurora::OBBFile &obb, bool directories, std::set<Common::UString> &files);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		int returnValue = 1;
		Command command = kCommandNone;
		Common::UString archive;
		std::set<Common::UString> files;

		if (!parseCommandLine(args, returnValue, command, archive, files))
			return returnValue;

		Aurora::OBBFile obb(new Common::ReadFile(archive));

		if      (command == kCommandList)
			Archives::listFiles(obb, Aurora::kGameIDUnknown, false);
		else if (command == kCommandListVerbose)
			Archives::listFiles(obb, Aurora::kGameIDUnknown, true);
		else if (command == kCommandExtract)
			extractFiles(obb, false, files);
		else if (command == kCommandExtractDir)
			extractFiles(obb, true, files);

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
                      Command &command, Common::UString &archive, std::set<Common::UString> &files) {

	using Common::CLI::NoOption;
	using Common::CLI::kContinueParsing;
	using Common::CLI::Parser;
	using Common::CLI::ValGetter;
	using Common::CLI::ValAssigner;
	using Common::CLI::makeEndArgs;
	using Common::CLI::makeAssigners;

	NoOption cmdOpt(false, new ValGetter<Command &>(command, "command"));
	NoOption archiveOpt(false, new ValGetter<Common::UString &>(archive, "archive"));
	NoOption filesOpt(true, new ValGetter<std::set<Common::UString> &>(files, "files[...]"));
	Parser parser(argv[0], "Aspyr OBB virtual filesystem extractor",
	              "Commands:\n"
	              "  l          List files (stripping directories)\n"
	              "  v          List files verbosely (with directories)\n"
	              "  e          Extract files to current directory, stripping directories\n"
	              "  x          Extract files to current directory, creating subdirectories\n",
	              returnValue,
	              makeEndArgs(&cmdOpt, &archiveOpt, &filesOpt));

	return parser.process(argv);
}

void extractFiles(Aurora::OBBFile &obb, bool directories, std::set<Common::UString> &files) {
	const Aurora::Archive::ResourceList &resources = obb.getResources();
	const size_t fileCount = resources.size();

	std::printf("Number of files: %u\n\n", (uint)fileCount);

	size_t i = 1;
	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r, ++i) {
		const Common::UString path     = TypeMan.addFileType(r->name, r->type);
		const Common::UString fileName = Common::FilePath::getFile(path);
		const Common::UString dirName  = Common::FilePath::getDirectory(path);
		const Common::UString name     = directories ? path : fileName;

		if (!files.empty() && (files.find(path) == files.end()))
			continue;

		if (directories && !dirName.empty())
			Common::FilePath::createDirectories(dirName);

		std::printf("Extracting %u/%u: %s ... ", (uint)i, (uint)fileCount, name.c_str());
		std::fflush(stdout);

		try {
			Common::ScopedPtr<Common::SeekableReadStream> stream(obb.getResource(r->index));

			dumpStream(*stream, name);

			std::printf("Done\n");
		} catch (Common::Exception &e) {
			Common::printException(e, "");
		}
	}

}
