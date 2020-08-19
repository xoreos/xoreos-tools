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
 *  Tool to convert raw Nintendo NBFS images into TGA.
 */

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include "src/version/version.h"

#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/readfile.h"
#include "src/common/cli.h"

#include "src/aurora/types.h"
#include "src/aurora/util.h"

#include "src/images/nbfs.h"

#include "src/util.h"

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &nbfsFile, Common::UString &nbfpFile,
                      Common::UString &outFile, uint32_t &width, uint32_t &height);

void convert(const Common::UString &nbfsFile, const Common::UString &nbfpFile,
             const Common::UString &outFile, uint32_t width, uint32_t height);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		int returnValue = 1;
		Common::UString nbfsFile, nbfpFile, outFile;
		uint32_t width = 0xFFFFFFFF, height = 0xFFFFFFFF;

		if (!parseCommandLine(args, returnValue, nbfsFile, nbfpFile, outFile, width, height))
			return returnValue;

		convert(nbfsFile, nbfpFile, outFile, width, height);
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &nbfsFile, Common::UString &nbfpFile,
                      Common::UString &outFile, uint32_t &width, uint32_t &height) {

	using Common::CLI::Parser;
	using Common::CLI::ValGetter;
	using Common::CLI::NoOption;
	using Common::CLI::makeEndArgs;
	using Common::CLI::kContinueParsing;
	std::vector<Common::CLI::Getter *> getters;
	NoOption nbfsFileOpt(false, new ValGetter<Common::UString &>(nbfsFile, "nbfs"));
	NoOption nbfpFileOpt(false, new ValGetter<Common::UString &>(nbfpFile, "nbfp"));
	NoOption outFileOpt(false, new ValGetter<Common::UString &>(outFile, "tga"));
	NoOption widthOpt(true, new ValGetter<uint32_t &>(width, "width"));
	NoOption heightOpt(true, new ValGetter<uint32_t &>(height, "height"));
	Parser parser(argv[0], "Nintendo raw NBFS image to TGA converter\n", "",
	              returnValue, makeEndArgs(&nbfsFileOpt, &nbfpFileOpt,
	              &outFileOpt, &widthOpt, &heightOpt));

	return parser.process(argv);
}

void convert(const Common::UString &nbfsFile, const Common::UString &nbfpFile,
             const Common::UString &outFile, uint32_t width, uint32_t height) {

	Common::ReadFile nbfs(nbfsFile), nbfp(nbfpFile);
	Images::NBFS image(nbfs, nbfp, width, height);

	image.flipVertically();

	image.dumpTGA(outFile);
}
