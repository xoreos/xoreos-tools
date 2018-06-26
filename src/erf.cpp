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
 *  Tool to pack ERF (.erf, .mod, .nwm, .sav) archives.
 */

#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/cli.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/filepath.h"

#include "src/aurora/erfwriter.h"
#include "src/aurora/util.h"

#include "src/util.h"

static const uint32 kERFID = MKTAG('E', 'R', 'F', ' ');
static const uint32 kMODID = MKTAG('M', 'O', 'D', ' ');
static const uint32 kHAKID = MKTAG('H', 'A', 'K', ' ');
static const uint32 kSAVID = MKTAG('S', 'A', 'V', ' ');

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &archive, std::set<Common::UString> &files, uint32 id);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		uint32 id = kERFID;
		int returnValue = 1;
		Common::UString archive;
		std::set<Common::UString> files;

		if (!parseCommandLine(args, returnValue, archive, files, id))
			return returnValue;

		Common::WriteFile writeFile(archive);

		size_t i = 1;
		Aurora::ERFWriter erfWriter(id, files.size(), writeFile);
		for (std::set<Common::UString>::const_iterator iter = files.begin(); iter != files.end(); ++iter, ++i) {
			std::printf("Packing %u/%u: %s ... ", (uint)i, (uint)files.size(), iter->c_str());
			std::fflush(stdout);

			Common::UString file = *iter;
			Common::ReadFile fileStream(file);

			erfWriter.add(Common::FilePath::getStem(file), TypeMan.getFileType(file), fileStream);
			std::printf("Done\n");
		}
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &archive, std::set<Common::UString> &files, uint32 id) {
	using Common::CLI::NoOption;
	using Common::CLI::kContinueParsing;
	using Common::CLI::Parser;
	using Common::CLI::Callback;
	using Common::CLI::ValGetter;
	using Common::CLI::ValAssigner;
	using Common::CLI::makeEndArgs;
	using Common::CLI::makeAssigners;

	NoOption archiveOpt(false, new ValGetter<Common::UString &>(archive, "output archive"));
	NoOption filesOpt(true, new ValGetter<std::set<Common::UString> &>(files, "files[...]"));

	Parser parser(argv[0], "BioWare ERF archive packer",
	              "",
	              returnValue,
	              makeEndArgs(&archiveOpt, &filesOpt));

	parser.addSpace();
	parser.addOption("erf", "Set ERF as archive id (default)",
	                 kContinueParsing,
	                 makeAssigners(new ValAssigner<uint32>(kERFID, id)));
	parser.addOption("mod", "Set MOD as archive id",
	                 kContinueParsing,
	                 makeAssigners(new ValAssigner<uint32>(kMODID, id)));
	parser.addOption("hak", "Set HAK as archive id",
	                 kContinueParsing,
	                 makeAssigners(new ValAssigner<uint32>(kHAKID, id)));
	parser.addOption("sav", "Set SAV as archive id",
	                 kContinueParsing,
	                 makeAssigners(new ValAssigner<uint32>(kSAVID, id)));

	return parser.process(argv);
}
