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
 *  Tool to convert 2DA/GDA files to 2DA/CSV.
 */

#include <cstring>
#include <cstdio>

#include <memory>

#include "src/version/version.h"

#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/stdoutstream.h"
#include "src/common/encoding.h"
#include "src/common/platform.h"
#include "src/common/cli.h"

#include "src/aurora/aurorafile.h"
#include "src/aurora/2dafile.h"
#include "src/aurora/gdafile.h"

#include "src/util.h"

enum Format {
	kFormat2DA,
	kFormat2DAb,
	kFormatCSV
};

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      std::vector<Common::UString> &files, Common::UString &outFile, Format &format);

void write2DA(Aurora::TwoDAFile &twoDA, Format format);

Aurora::TwoDAFile *get2DAGDA(Common::SeekableReadStream *stream);
void convert2DA(const Common::UString &file, const Common::UString &outFile, Format format);
void convert2DA(const std::vector<Common::UString> &files, const Common::UString &outFile, Format format);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		Format format = kFormat2DA;

		int returnValue = 1;
		std::vector<Common::UString> files;
		Common::UString outFile;

		if (!parseCommandLine(args, returnValue, files, outFile, format))
			return returnValue;

		convert2DA(files, outFile, format);
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      std::vector<Common::UString> &files, Common::UString &outFile,
                      Format &format) {
	using Common::CLI::NoOption;
	using Common::CLI::kContinueParsing;
	using Common::CLI::Parser;
	using Common::CLI::ValGetter;
	using Common::CLI::Callback;
	using Common::CLI::ValAssigner;
	using Common::CLI::makeEndArgs;
	using Common::CLI::makeAssigners;

	NoOption filesOpt(false, new ValGetter<std::vector<Common::UString> &>(files, "files[...]"));
	Parser parser(argv[0], "BioWare 2DA/GDA to 2DA/CSV converter\n",
	              "If several files are given, they must all be GDA and use the same\n"
	              "column layout. They will be pasted together and printed as one GDA.\n\n"
	              "If no output file is given, the output is written to stdout.",
	              returnValue,
	              makeEndArgs(&filesOpt));

	parser.addSpace();
	parser.addOption("output", 'o', "Write the output to this file",
	                 kContinueParsing,
	                 new ValGetter<Common::UString &>(outFile, "file"));
	parser.addSpace();
	parser.addOption("2da", 'a', "Convert to ASCII 2DA (default)",
	                 kContinueParsing,
	                 makeAssigners(new ValAssigner<Format>(kFormat2DA,
	                 format)));
	parser.addOption("2dab", 'b', "Convert to binary 2DA", kContinueParsing,
	                 makeAssigners(new ValAssigner<Format>(kFormat2DAb,
	                 format)));
	parser.addOption("csv", 'c', "Convert to CSV", kContinueParsing,
	                 makeAssigners(new ValAssigner<Format>(kFormatCSV,
	                 format)));
	return parser.process(argv);
}

static const uint32_t k2DAID     = MKTAG('2', 'D', 'A', ' ');
static const uint32_t k2DAIDTab  = MKTAG('2', 'D', 'A', '\t');
static const uint32_t kGFFID     = MKTAG('G', 'F', 'F', ' ');

void write2DA(Aurora::TwoDAFile &twoDA, const Common::UString &outFile, Format format) {
	std::unique_ptr<Common::WriteStream> out(openFileOrStdOut(outFile));

	if      (format == kFormat2DA)
		twoDA.writeASCII(*out);
	else if (format == kFormat2DAb)
		twoDA.writeBinary(*out);
	else
		twoDA.writeCSV(*out);

	out->flush();
}

Aurora::TwoDAFile *get2DAGDA(Common::SeekableReadStream *stream) {
	std::unique_ptr<Common::SeekableReadStream> fStream(stream);

	const uint32_t id = Aurora::AuroraFile::readHeaderID(*fStream);
	fStream->seek(0);

	if ((id == k2DAID) || (id == k2DAIDTab))
		return new Aurora::TwoDAFile(*fStream);

	if (id == kGFFID) {
		Aurora::GDAFile gda(fStream.release());

		return new Aurora::TwoDAFile(gda);
	}

	throw Common::Exception("Not a 2DA or GDA file");
}

void convert2DA(const Common::UString &file, const Common::UString &outFile, Format format) {
	std::unique_ptr<Aurora::TwoDAFile> twoDA(get2DAGDA(new Common::ReadFile(file)));

	write2DA(*twoDA, outFile, format);
}

void convert2DA(const std::vector<Common::UString> &files, const Common::UString &outFile, Format format) {
	if (files.size() == 1) {
		convert2DA(files[0], outFile, format);
		return;
	}

	Aurora::GDAFile gda(new Common::ReadFile(files[0]));

	for (size_t i = 1; i < files.size(); i++)
		gda.add(new Common::ReadFile(files[i]));

	Aurora::TwoDAFile twoDA(gda);

	write2DA(twoDA, outFile, format);
}
