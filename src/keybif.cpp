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
 *  Tool to pack KEY/BIF archives.
 */

#include "src/aurora/keydatafile.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/cli.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/filepath.h"

#include "src/aurora/bifwriter.h"
#include "src/aurora/bzfwriter.h"
#include "src/aurora/keywriter.h"
#include "src/aurora/util.h"

#include "src/util.h"

struct BIFGroup {
	Common::UString name;
	std::list<Common::UString> files;
};

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &keyfile, std::set<Common::UString> &files);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		int returnValue = 1;
		Common::UString keyFile;
		std::set<Common::UString> files;

		if (!parseCommandLine(args, returnValue, keyFile, files))
			return returnValue;

		Aurora::KEYWriter keyWriter;

		std::list<BIFGroup> groups;

		for (const auto& file : files) {
			if (file.endsWith(".bif") || file.endsWith(".bzf")) {
				BIFGroup group;
				group.name = file;
				groups.push_back(group);
				continue;
			} else {
				if (groups.empty())
					throw Common::Exception("Files have to start with a bif or bzf archive");
				groups.back().files.push_back(file);
			}
		}

		for (const auto& group : groups) {
			std::printf("Packing %s ... \n", group.name.c_str());

			Common::WriteFile writeBIFFile(group.name);
			Common::ScopedPtr<Aurora::KEYDataWriter> dataFile;

			if (group.name.endsWith(".bzf"))
				dataFile.reset(new Aurora::BZFWriter(group.files.size(), writeBIFFile));
			else
				dataFile.reset(new Aurora::BIFWriter(group.files.size(), writeBIFFile));

			size_t i = 1;
			for (const auto& file : group.files) {
				std::printf("\tPacking %u/%u: %s ... ", (uint)i, static_cast<uint>(group.files.size()), file.c_str());
				std::fflush(stdout);

				Common::ReadFile packFile(file);
				dataFile->add(packFile, TypeMan.getFileType(file));

				++i;

				std::printf("Done\n");
			}

			keyWriter.addBIF(group.name, group.files, dataFile->size());
		}

		Common::WriteFile writeFile(keyFile);
		keyWriter.write(writeFile);
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &keyfile, std::set<Common::UString> &files) {
	using Common::CLI::NoOption;
	using Common::CLI::kContinueParsing;
	using Common::CLI::Parser;
	using Common::CLI::Callback;
	using Common::CLI::ValGetter;
	using Common::CLI::ValAssigner;
	using Common::CLI::makeEndArgs;
	using Common::CLI::makeAssigners;
	using Aurora::GameID;

	NoOption archiveOpt(false, new ValGetter<Common::UString &>(keyfile, "key file"));
	NoOption filesOpt(true, new ValGetter<std::set<Common::UString> &>(files, "files[...]"));

	Parser parser(argv[0], "BioWare KEY/BIF archive packer",
				  "",
				  returnValue,
				  makeEndArgs(&archiveOpt, &filesOpt));

	return parser.process(argv);
}
