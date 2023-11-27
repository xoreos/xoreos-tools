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

#include <memory>

#include "src/version/version.h"

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

#include "src/images/decoder.h"
#include "src/images/dds.h"
#include "src/images/sbm.h"
#include "src/images/tga.h"
#include "src/images/tpc.h"
#include "src/images/txb.h"

#include "src/util.h"

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile,
                      Aurora::FileType &type, bool &flip, bool &deswizzle);

void convert(const Common::UString &inFile, const Common::UString &outFile,
             Aurora::FileType type, bool flip, bool deswizzle);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		int returnValue = 1;
		Common::UString inFile, outFile;
		Aurora::FileType type = Aurora::kFileTypeNone;
		bool flip = false, deswizzle = false;

		if (!parseCommandLine(args, returnValue, inFile, outFile, type, flip, deswizzle))
			return returnValue;

		convert(inFile, outFile, type, flip, deswizzle);
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile,
                      Aurora::FileType &type, bool &flip, bool &deswizzle) {

	using Common::CLI::NoOption;
	using Common::CLI::kContinueParsing;
	using Common::CLI::Parser;
	using Common::CLI::ValGetter;
	using Common::CLI::Callback;
	using Common::CLI::ValAssigner;
	using Common::CLI::makeEndArgs;
	using Common::CLI::makeAssigners;

	NoOption inFileOpt(false, new ValGetter<Common::UString &>(inFile, "input files"));
	NoOption outFileOpt(true, new ValGetter<Common::UString &>(outFile, "output files"));
	Parser parser(argv[0], "BioWare textures to TGA converter",
	              "",
	              returnValue,
	              makeEndArgs(&inFileOpt, &outFileOpt));

	parser.addSpace();
	parser.addOption("auto", "Autodetect input type (default)", kContinueParsing,
	                 makeAssigners(new ValAssigner<Aurora::FileType>(Aurora::kFileTypeNone, type)));
	parser.addOption("dds", "Input file is DDS", kContinueParsing,
	                 makeAssigners(new ValAssigner<Aurora::FileType>(Aurora::kFileTypeDDS, type)));
	parser.addOption("sbm", "Input file is SBM", kContinueParsing,
	                 makeAssigners(new ValAssigner<Aurora::FileType>(Aurora::kFileTypeSBM, type)));
	parser.addOption("tpc", "Input file is TPC", kContinueParsing,
	                 makeAssigners(new ValAssigner<Aurora::FileType>(Aurora::kFileTypeTPC, type)));
	parser.addOption("txb", "Input file is TXB", kContinueParsing,
	                 makeAssigners(new ValAssigner<Aurora::FileType>(Aurora::kFileTypeTXB, type)));
	parser.addOption("tga", "Input file is TGA", kContinueParsing,
	                 makeAssigners(new ValAssigner<Aurora::FileType>(Aurora::kFileTypeTGA, type)));
	parser.addSpace();
	parser.addOption("flip", 'f', "Flip the image vertically", kContinueParsing,
	                 makeAssigners(new ValAssigner<bool>(true, flip)));
	parser.addSpace();
	parser.addOption("deswizzle", 'd', "Input file is an Xbox SBM that needs deswizzling",
	                 kContinueParsing, makeAssigners(new ValAssigner<bool>(true, deswizzle)));
	return parser.process(argv);
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

static Images::Decoder *openImage(Common::SeekableReadStream &stream, Aurora::FileType type, bool deswizzle) {
	switch (type) {
		case Aurora::kFileTypeDDS:
			return new Images::DDS(stream);
		case Aurora::kFileTypeSBM:
			return new Images::SBM(stream, deswizzle);
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
             Aurora::FileType type, bool flip, bool deswizzle) {

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

	std::unique_ptr<Images::Decoder> image(openImage(in, type, deswizzle));
	if (flip)
		image->flipVertically();

	image->dumpTGA(outFile);
}
