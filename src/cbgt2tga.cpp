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
 *  Tool to convert CBGT images to TGA. */

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

#include "src/images/cbgt.h"

#include "src/util.h"

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &cbgtFile , Common::UString &palFile,
                      Common::UString &twoDAFile, Common::UString &outFile);

void convert(const Common::UString &cbgtFile , const Common::UString &palFile,
             const Common::UString &twoDAFile, const Common::UString &outFile);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		int returnValue = 1;
		Common::UString cbgtFile, palFile, twoDAFile, outFile;

		if (!parseCommandLine(args, returnValue, cbgtFile, palFile, twoDAFile, outFile))
			return returnValue;

		convert(cbgtFile, palFile, twoDAFile, outFile);
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &cbgtFile , Common::UString &palFile,
                      Common::UString &twoDAFile, Common::UString &outFile) {
	using Common::CLI::Parser;
	using Common::CLI::ValGetter;
	using Common::CLI::NoOption;
	using Common::CLI::makeEndArgs;
	std::vector<Common::CLI::Getter *> getters;
	NoOption cbgtFileOpt(false, new ValGetter<Common::UString &>(cbgtFile, "cbgt"));
	NoOption palFileOpt(false, new ValGetter<Common::UString &>(palFile, "pal"));
	NoOption twoDAFileOpt(false, new ValGetter<Common::UString &>(twoDAFile, "2da"));
	NoOption outFileOpt(false, new ValGetter<Common::UString &>(outFile, "tga"));
	Parser parser(argv[0], "CBGT image to TGA converter","", returnValue,
		      makeEndArgs(&cbgtFileOpt, &palFileOpt, &twoDAFileOpt, &outFileOpt));

	return parser.process(argv);
}

void convert(const Common::UString &cbgtFile , const Common::UString &palFile,
             const Common::UString &twoDAFile, const Common::UString &outFile) {

	Common::ReadFile cbgt(cbgtFile), pal(palFile), twoDA(twoDAFile);
	Images::CBGT image(cbgt, pal, twoDA);

	image.flipVertically();

	image.dumpTGA(outFile);
}
