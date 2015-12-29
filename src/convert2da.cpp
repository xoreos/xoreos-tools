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

#include "src/common/version.h"
#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/stdoutstream.h"
#include "src/common/encoding.h"
#include "src/common/platform.h"

#include "src/aurora/aurorafile.h"
#include "src/aurora/2dafile.h"
#include "src/aurora/gdafile.h"

enum Format {
	kFormat2DA,
	kFormat2DAb,
	kFormatCSV
};

void printUsage(FILE *stream, const Common::UString &name);
bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      std::vector<Common::UString> &files, Common::UString &outFile, Format &format);

void write2DA(Aurora::TwoDAFile &twoDA, Format format);

Aurora::TwoDAFile *get2DAGDA(Common::SeekableReadStream *stream);
void convert2DA(const Common::UString &file, const Common::UString &outFile, Format format);
void convert2DA(const std::vector<Common::UString> &files, const Common::UString &outFile, Format format);

int main(int argc, char **argv) {
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
                      std::vector<Common::UString> &files, Common::UString &outFile, Format &format) {
	files.clear();
	outFile.clear();

	bool optionsEnd = false;
	for (size_t i = 1; i < argv.size(); i++) {
		bool isOption = false;

		// A "--" marks an end to all options
		if (argv[i] == "--") {
			optionsEnd = true;
			continue;
		}

		// We're still handling options
		if (!optionsEnd) {
			// Help text
			if ((argv[i] == "-h") || (argv[i] == "--help")) {
				printUsage(stdout, argv[0]);
				returnValue = 0;

				return false;
			}

			if (argv[i] == "--version") {
				printVersion();
				returnValue = 0;

				return false;
			}

			if        ((argv[i] == "--2da") || (argv[i] == "-a")) {
				isOption = true;
				format   = kFormat2DA;
			} else if ((argv[i] == "--2dab") || (argv[i] == "-b")) {
				isOption = true;
				format   = kFormat2DAb;
			} else if ((argv[i] == "--csv") || (argv[i] == "-c")) {
				isOption = true;
				format   = kFormatCSV;
			} else if ((argv[i] == "-o") || (argv[i] == "--output")) {
				isOption = true;

				// Needs a file name as the next parameter
				if (i++ == (argv.size() - 1)) {
					printUsage(stdout, argv[0]);
					returnValue = 1;

					return false;
				}

				outFile = argv[i];

			} else if (argv[i].beginsWith("-") || argv[i].beginsWith("--")) {
			  // An options, but we already checked for all known ones

				printUsage(stderr, argv[0]);
				returnValue = 1;

				return false;
			}
		}

		// Was this a valid option? If so, don't try to use it as a file
		if (isOption)
			continue;

		// This is a file to use
		files.push_back(argv[i]);
	}

	// No files? Error.
	if (files.empty()) {
		printUsage(stderr, argv[0]);
		returnValue = 1;

		return false;
	}

	return true;
}

void printUsage(FILE *stream, const Common::UString &name) {
	std::fprintf(stream, "BioWare 2DA/GDA to 2DA/CSV converter\n\n");
	std::fprintf(stream, "Usage: %s [<options>] <file> [<file> [...]]\n", name.c_str());
	std::fprintf(stream, "  -h        --help              This help text\n");
	std::fprintf(stream, "            --version           Display version information\n");
	std::fprintf(stream, "  -o <file> --output <file>     Write the output to this file\n");
	std::fprintf(stream, "  -a        --2da               Convert to ASCII 2DA (default)\n");
	std::fprintf(stream, "  -b        --2dab              Convert to binary 2DA\n");
	std::fprintf(stream, "  -c        --csv               Convert to CSV\n\n");
	std::fprintf(stream, "If several files are given, they must all be GDA and use the same\n");
	std::fprintf(stream, "column layout. They will be pasted together and printed as one GDA.\n\n");
	std::fprintf(stream, "If no output file is given, the output is written to stdout.\n");
}

static const uint32 k2DAID     = MKTAG('2', 'D', 'A', ' ');
static const uint32 k2DAIDTab  = MKTAG('2', 'D', 'A', '\t');
static const uint32 kGFFID     = MKTAG('G', 'F', 'F', ' ');

void write2DA(Aurora::TwoDAFile &twoDA, const Common::UString &outFile, Format format) {
	Common::WriteStream *out = 0;
	if (!outFile.empty())
		out = new Common::WriteFile(outFile);
	else
		out = new Common::StdOutStream;

	try {
		if      (format == kFormat2DA)
			twoDA.writeASCII(*out);
		else if (format == kFormat2DAb)
			twoDA.writeBinary(*out);
		else
			twoDA.writeCSV(*out);

	} catch (...) {
		delete out;
		throw;
	}

	out->flush();

	delete out;
}

Aurora::TwoDAFile *get2DAGDA(Common::SeekableReadStream *stream) {
	uint32 id = 0;

	try {
		id = Aurora::AuroraFile::readHeaderID(*stream);
		stream->seek(0);
	} catch (...) {
		delete stream;
		throw;
	}

	if ((id == k2DAID) || (id == k2DAIDTab)) {
		Aurora::TwoDAFile *twoDA = new Aurora::TwoDAFile(*stream);

		delete stream;
		return twoDA;
	}

	if (id == kGFFID) {
		Aurora::GDAFile gda(stream);

		return new Aurora::TwoDAFile(gda);
	}

	delete stream;
	throw Common::Exception("Not a 2DA or GDA file");
}

void convert2DA(const Common::UString &file, const Common::UString &outFile, Format format) {
	Aurora::TwoDAFile *twoDA = get2DAGDA(new Common::ReadFile(file));

	try {
		write2DA(*twoDA, outFile, format);
	} catch (...) {
		delete twoDA;
		throw;
	}

	delete twoDA;
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
