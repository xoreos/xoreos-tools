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
 *  Tool to extract Aspyr's OBB virtual filesystems / ZIP archives.
 */

#include <cstring>

#include <set>
#include <memory>

#include "src/version/version.h"

#include "src/common/ustring.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/cli.h"
#include "src/common/readfile.h"

#include "src/aurora/obbfile.h"
#include "src/aurora/zipfile.h"

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

bool isPKZIP(Common::SeekableReadStream &stream);

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

		std::unique_ptr<Common::SeekableReadStream> stream = std::make_unique<Common::ReadFile>(archive);

		std::unique_ptr<Aurora::Archive> arc;
		if (isPKZIP(*stream))
			arc = std::make_unique<Aurora::ZIPFile>(stream.release());
		else
			arc = std::make_unique<Aurora::OBBFile>(stream.release());

		files = Archives::fixPathSeparator(files);

		if      (command == kCommandList)
			Archives::listFiles(*arc, Aurora::kGameIDUnknown, false);
		else if (command == kCommandListVerbose)
			Archives::listFiles(*arc, Aurora::kGameIDUnknown, true);
		else if (command == kCommandExtract)
			Archives::extractFiles(*arc, Aurora::kGameIDUnknown, false, files);
		else if (command == kCommandExtractDir)
			Archives::extractFiles(*arc, Aurora::kGameIDUnknown, true, files);

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
	using Common::CLI::Parser;
	using Common::CLI::ValGetter;
	using Common::CLI::makeEndArgs;

	NoOption cmdOpt(false, new ValGetter<Command &>(command, "command"));
	NoOption archiveOpt(false, new ValGetter<Common::UString &>(archive, "archive"));
	NoOption filesOpt(true, new ValGetter<std::set<Common::UString> &>(files, "files[...]"));
	Parser parser(argv[0], "Aspyr OBB virtual filesystem / archive extractor",
	              "Commands:\n"
	              "  l          List files (stripping directories)\n"
	              "  v          List files verbosely (with directories)\n"
	              "  e          Extract files to current directory, stripping directories\n"
	              "  x          Extract files to current directory, creating subdirectories\n",
	              returnValue,
	              makeEndArgs(&cmdOpt, &archiveOpt, &filesOpt));

	return parser.process(argv);
}

bool isPKZIP(Common::SeekableReadStream &stream) {
	const size_t pos = stream.pos();
	const bool pkzip = stream.readUint16BE() == MKTAG_16('P', 'K');

	stream.seek(pos);
	return pkzip;
}
