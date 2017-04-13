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
 *  Tool to convert XML files back into SSF.
 */

#include <cstring>
#include <cstdio>

#include "src/version/version.h"

#include "src/common/scopedptr.h"
#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/stdinstream.h"
#include "src/common/encoding.h"
#include "src/common/cli.h"

#include "src/aurora/types.h"

#include "src/xml/ssfcreator.h"

#include "src/util.h"

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile, Aurora::GameID &game);

void createSSF(const Common::UString &inFile, const Common::UString &outFile, Aurora::GameID game);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		Aurora::GameID game = Aurora::kGameIDUnknown;

		int returnValue = 1;
		Common::UString inFile, outFile;

		if (!parseCommandLine(args, returnValue, inFile, outFile, game))
			return returnValue;

		createSSF(inFile, outFile, game);
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile, Aurora::GameID &game) {

	using Common::CLI::NoOption;
	using Common::CLI::kContinueParsing;
	using Common::CLI::Parser;
	using Common::CLI::ValGetter;
	using Common::CLI::ValAssigner;
	using Common::CLI::makeEndArgs;
	using Common::CLI::makeAssigners;
	std::vector<Common::UString> args;
	NoOption filesOpt(false, new ValGetter<std::vector<Common::UString> &>(args,
									       "[input file] <output file>"));
	Parser parser(argv[0], "XML to BioWare SSF converter",
		      "If no input file is given, the input is read from stdin.\n\n"
		      "Since different games use different SSF file version, specifying the\n"
		      "game for which to create the SSF file is necessary.",
		      returnValue,
		      makeEndArgs(&filesOpt));

	game = Aurora::kGameIDUnknown;
	parser.addSpace();
	parser.addOption("nwn", "Create an SSF for Neverwinter Nights",
			 kContinueParsing,
			 makeAssigners(new ValAssigner<Aurora::GameID>(Aurora::kGameIDNWN, game)));
	parser.addOption("nwn2", "Create an SSF for Neverwinter Nights 2",
			 kContinueParsing,
			 makeAssigners(new ValAssigner<Aurora::GameID>(Aurora::kGameIDNWN2, game)));
	parser.addOption("kotor", "Create an SSF for Knights of the Old Republic",
			 kContinueParsing,
			 makeAssigners(new ValAssigner<Aurora::GameID>(Aurora::kGameIDKotOR, game)));
	parser.addOption("kotor2", "Create an SSF for Knights of the Old Republic II",
			 kContinueParsing,
			 makeAssigners(new ValAssigner<Aurora::GameID>(Aurora::kGameIDKotOR2, game)));


	if (!parser.process(argv))
		return false;

	if ((args.size() < 1) || (args.size() > 2) || (game == Aurora::kGameIDUnknown)) {
		parser.usage();
		returnValue = 1;

		return false;
	}

	if (args.size() == 2) {
		inFile  = args[0];
		outFile = args[1];
	} else
		outFile = args[0];

	return true;
}

void createSSF(const Common::UString &inFile, const Common::UString &outFile, Aurora::GameID game) {
	Common::WriteFile ssf(outFile);
	Common::ScopedPtr<Common::ReadStream> xml(openFileOrStdIn(inFile));

	XML::SSFCreator::create(ssf, *xml, game);

	ssf.flush();
	ssf.close();
}
