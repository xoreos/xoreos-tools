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

#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/readfile.h"
#include "src/common/stdoutstream.h"
#include "src/common/encoding.h"

#include "src/aurora/aurorafile.h"
#include "src/aurora/2dafile.h"
#include "src/aurora/gdafile.h"

enum Format {
	kFormat2DA,
	kFormatCSV
};

void printUsage(FILE *stream, const char *name);
bool parseCommandLine(int argc, char **argv, int &returnValue, std::vector<Common::UString> &files,
                      Format &format);

void dump2DA(Aurora::TwoDAFile &twoDA, Format format);

Aurora::TwoDAFile *get2DAGDA(Common::SeekableReadStream *stream);
void convert2DA(const Common::UString &file, Format format);
void convert2DA(const std::vector<Common::UString> &files, Format format);

int main(int argc, char **argv) {
	Format format = kFormat2DA;

	int returnValue;
	std::vector<Common::UString> files;
	if (!parseCommandLine(argc, argv, returnValue, files, format))
		return returnValue;

	try {
		convert2DA(files, format);
	} catch (Common::Exception &e) {
		Common::printException(e);
		return -1;
	} catch (std::exception &e) {
		error("%s", e.what());
	}

	return 0;
}

bool parseCommandLine(int argc, char **argv, int &returnValue, std::vector<Common::UString> &files,
                      Format &format) {
	files.clear();

	if (argc < 2) {
		printUsage(stderr, argv[0]);
		returnValue = -1;

		return false;
	}

	bool optionsEnd = false;
	for (int i = 1; i < argc; i++) {
		bool isOption = false;

		// A "--" marks an end to all options
		if (!strcmp(argv[i], "--")) {
			optionsEnd = true;
			continue;
		}

		// We're still handling options
		if (!optionsEnd) {
			// Help text
			if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
				printUsage(stdout, argv[0]);
				returnValue = 0;

				return false;
			}

			if        (!strcmp(argv[i], "--2da") || !strcmp(argv[i], "-2")) {
				isOption = true;
				format   = kFormat2DA;
			} else if (!strcmp(argv[i], "--csv") || !strcmp(argv[i], "-c")) {
				isOption = true;
				format   = kFormatCSV;
			} else if (!strncmp(argv[i], "-", 1) || !strncmp(argv[i], "--", 2)) {
			  // An options, but we already checked for all known ones

				printUsage(stderr, argv[0]);
				returnValue = -1;

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
		returnValue = -1;

		return false;
	}

	return true;
}

void printUsage(FILE *stream, const char *name) {
	std::fprintf(stream, "BioWare 2DA/GDA to 2DA/CSV converter\n\n");
	std::fprintf(stream, "Usage: %s [options] <file> [<file> [...]]\n", name);
	std::fprintf(stream, "  -h      --help              This help text\n");
	std::fprintf(stream, "  -2      --2da               Convert to ASCII 2DA\n");
	std::fprintf(stream, "  -c      --csv               Convert to CSV\n\n");
	std::fprintf(stream, "If several files are given, they must all be GDA and use the same\n");
	std::fprintf(stream, "column layout. They will be pasted together and printed as one GDA.\n");
}

static const uint32 k2DAID     = MKTAG('2', 'D', 'A', ' ');
static const uint32 k2DAIDTab  = MKTAG('2', 'D', 'A', '\t');
static const uint32 kGFFID     = MKTAG('G', 'F', 'F', ' ');

void dump2DA(Aurora::TwoDAFile &twoDA, Format format) {
	Common::StdOutStream stdOut;

	if (format == kFormat2DA)
		twoDA.dumpASCII(stdOut);
	else
		twoDA.dumpCSV(stdOut);
}

Aurora::TwoDAFile *get2DAGDA(Common::SeekableReadStream *stream) {
	uint32 id = 0;

	try {
		id = Aurora::AuroraBase::readHeaderID(*stream);
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

void convert2DA(const Common::UString &file, Format format) {
	Aurora::TwoDAFile *twoDA = get2DAGDA(new Common::ReadFile(file));

	try {
		dump2DA(*twoDA, format);
	} catch (...) {
		delete twoDA;
		throw;
	}

	delete twoDA;
}

void convert2DA(const std::vector<Common::UString> &files, Format format) {
	if (files.size() == 1) {
		convert2DA(files[0], format);
		return;
	}

	Aurora::GDAFile gda(new Common::ReadFile(files[0]));

	for (size_t i = 1; i < files.size(); i++)
		gda.add(new Common::ReadFile(files[i]));

	Aurora::TwoDAFile twoDA(gda);

	dump2DA(twoDA, format);
}
