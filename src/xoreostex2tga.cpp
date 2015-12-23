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
 *  Tool to convert BioWare's texture formats into TGA.
 */

#include <cstring>
#include <cstdio>

#include "src/common/version.h"
#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/readstream.h"
#include "src/common/readfile.h"

#include "src/aurora/types.h"
#include "src/aurora/util.h"

#include "src/images/decoder.h"
#include "src/images/dds.h"
#include "src/images/sbm.h"
#include "src/images/tga.h"
#include "src/images/tpc.h"
#include "src/images/txb.h"

void printUsage(FILE *stream, const Common::UString &name);
bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile,
                      Aurora::FileType &type, bool &flip);

void convert(const Common::UString &inFile, const Common::UString &outFile,
             Aurora::FileType type, bool flip);

int main(int argc, char **argv) {
	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		int returnValue = 1;
		Common::UString inFile, outFile;
		Aurora::FileType type = Aurora::kFileTypeNone;
		bool flip = false;

		if (!parseCommandLine(args, returnValue, inFile, outFile, type, flip))
			return returnValue;

		convert(inFile, outFile, type, flip);
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile,
                      Aurora::FileType &type, bool &flip) {

	std::vector<Common::UString> files;

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

			if        (argv[i] == "--auto") {
				isOption = true;
				type     = Aurora::kFileTypeNone;
			} else if (argv[i] == "--dds") {
				isOption = true;
				type     = Aurora::kFileTypeDDS;
			} else if (argv[i] == "--sbm") {
				isOption = true;
				type     = Aurora::kFileTypeSBM;
			} else if (argv[i] == "--tpc") {
				isOption = true;
				type     = Aurora::kFileTypeTPC;
			} else if (argv[i] == "--txb") {
				isOption = true;
				type     = Aurora::kFileTypeTXB;
			} else if (argv[i] == "--tga") {
				isOption = true;
				type     = Aurora::kFileTypeTGA;
			} else if ((argv[i] == "-f") || (argv[i] == "--flip")) {
				isOption = true;
				flip     = true;
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

		files.push_back(argv[i]);
	}

	if (files.size() != 2) {
		printUsage(stderr, argv[0]);
		returnValue = 1;

		return false;
	}

	inFile  = files[0];
	outFile = files[1];

	return true;
}

void printUsage(FILE *stream, const Common::UString &name) {
	std::fprintf(stream, "BioWare textures to TGA converter\n");
	std::fprintf(stream, "Usage: %s [<options>] <input file> <output file>\n", name.c_str());
	std::fprintf(stream, "  -h      --help              This help text\n");
	std::fprintf(stream, "          --version           Display version information\n");
	std::fprintf(stream, "  -f      --flip              Flip the image vertically\n");
	std::fprintf(stream, "          --auto              Autodetect input type (default)\n");
	std::fprintf(stream, "          --dds               Input file is DDS\n");
	std::fprintf(stream, "          --sbm               Input file is SBM\n");
	std::fprintf(stream, "          --tpc               Input file is TPC\n");
	std::fprintf(stream, "          --txb               Input file is TXB\n");
	std::fprintf(stream, "          --tga               Input file is TGA\n");
}

static bool isValidType(Aurora::FileType type) {
	switch (type) {
		case Aurora::kFileTypeDDS:
		case Aurora::kFileTypeSBM:
		case Aurora::kFileTypeTPC:
		case Aurora::kFileTypeTXB:
		case Aurora::kFileTypeTGA:
			return true;

		default:
			break;
	}

	return false;
}

static Aurora::FileType detectType(Common::SeekableReadStream &file) {
	if (Images::DDS::detect(file))
		return Aurora::kFileTypeDDS;

	return Aurora::kFileTypeNone;
}

static Aurora::FileType detectType(const Common::UString &file) {
	Aurora::FileType type = TypeMan.getFileType(file);
	if (isValidType(type))
		return type;

	return Aurora::kFileTypeNone;
}

Images::Decoder *openImage(Common::SeekableReadStream &stream, Aurora::FileType type) {
	switch (type) {
		case Aurora::kFileTypeDDS:
			return new Images::DDS(stream);
		case Aurora::kFileTypeSBM:
			return new Images::SBM(stream);
		case Aurora::kFileTypeTPC:
			return new Images::TPC(stream);
		case Aurora::kFileTypeTXB:
			return new Images::TXB(stream);
		case Aurora::kFileTypeTGA:
			return new Images::TGA(stream);

		default:
			throw Common::Exception("Invalid image type %d", (int) type);
	}
}

void convert(const Common::UString &inFile, const Common::UString &outFile,
             Aurora::FileType type, bool flip) {

	Common::ReadFile in(inFile);

	if (type == Aurora::kFileTypeNone) {
		// Detect by file contents
		type = detectType(in);

		if (type == Aurora::kFileTypeNone) {
			// Detect by file name
			type = detectType(inFile);

			if (type == Aurora::kFileTypeNone)
				throw Common::Exception("Failed to detect type of file \"%s\"", inFile.c_str());
		}
	}

	Images::Decoder *image = openImage(in, type);
	if (flip)
		image->flipVertically();

	try {
		image->dumpTGA(outFile);
	} catch (...) {
		delete image;
		throw;
	}

	delete image;
}
