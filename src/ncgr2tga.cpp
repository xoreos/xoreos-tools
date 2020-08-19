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
 *  Tool to convert Nintendo NCGR images into TGA.
 */

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include "src/version/version.h"

#include "src/common/ptrvector.h"
#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/readstream.h"
#include "src/common/readfile.h"
#include "src/common/cli.h"

#include "src/aurora/types.h"
#include "src/aurora/util.h"

#include "src/images/ncgr.h"

#include "src/util.h"

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      uint32_t &width, uint32_t &height, std::vector<Common::UString> &ncgrFiles,
                      Common::UString &nclrFile, Common::UString &outFile);

void convert(std::vector<Common::UString> &ncgrFiles, Common::UString &nclrFile,
             Common::UString &outFile, uint32_t width, uint32_t height);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		int returnValue = 1;
		uint32_t width, height;
		std::vector<Common::UString> ncgrFiles;
		Common::UString nclrFile, outFile;

		if (!parseCommandLine(args, returnValue, width, height, ncgrFiles, nclrFile, outFile))
			return returnValue;

		convert(ncgrFiles, nclrFile, outFile, width, height);
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      uint32_t &width, uint32_t &height, std::vector<Common::UString> &ncgrFiles,
                      Common::UString &nclrFile, Common::UString &outFile) {

	std::vector<Common::UString> args;
	using Common::CLI::NoOption;
	using Common::CLI::Parser;
	using Common::CLI::ValGetter;
	using Common::CLI::makeEndArgs;

	NoOption argsOpt(false,
	                 new ValGetter<std::vector<Common::UString> &>
	                 (args, "width> <height> <ncgr> [<ngr> [...]] <nclr> <tga"));

	Parser parser(argv[0], "Nintendo NCGR image to TGA converter",
	              "", returnValue,
	              makeEndArgs(&argsOpt));

	if (!parser.process(argv))
		return false;

	if (args.size() < 5) {
		parser.usage();
		returnValue = 1;

		return false;
	}

	width  = (args.size() >= 2) ? atoi(args[0].c_str()) : 0;
	height = (args.size() >= 2) ? atoi(args[1].c_str()) : 0;

	if ((width == 0) || (height == 0) || (args.size() != (width * height + 4))) {
		parser.usage();
		returnValue = 1;

		return false;
	}

	ncgrFiles.resize(width * height);
	for (uint32_t i = 0; i < (width * height); i++)
		ncgrFiles[i] = args[i + 2];

	nclrFile = args[2 + width * height];
	outFile  = args[3 + width * height];

	return true;
}

void convert(std::vector<Common::UString> &ncgrFiles, Common::UString &nclrFile,
             Common::UString &outFile, uint32_t width, uint32_t height) {

	Common::ReadFile nclr(nclrFile);

	Common::PtrVector<Common::SeekableReadStream> ncgrs;
	ncgrs.resize(ncgrFiles.size(), 0);

	for (size_t i = 0; i < ncgrFiles.size(); i++)
		if (!ncgrFiles[i].empty() && (ncgrFiles[i] != "\"\"") && (ncgrFiles[i] != "\'\'"))
			ncgrs[i] = new Common::ReadFile(ncgrFiles[i]);

	Images::NCGR image(ncgrs, width, height, nclr);

	image.flipVertically();

	image.dumpTGA(outFile);
}
