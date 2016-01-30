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
 *  Tool to repair GFF files found in encrypted archives used by
 *  Neverwinter Nights premium modules.
 */

#include <cstring>
#include <cstdio>

#include <vector>

#include "src/common/version.h"
#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/filepath.h"
#include "src/common/memreadstream.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"

#include "src/aurora/types.h"

void printUsage(FILE *stream, const Common::UString &name);
bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile, Common::UString &id);

void fixPremiumGFF(Common::UString &inFile, Common::UString &outFile, Common::UString &id);

int main(int argc, char **argv) {
	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		int returnValue = 1;
		Common::UString inFile, outFile, id;

		if (!parseCommandLine(args, returnValue, inFile, outFile, id))
			return returnValue;

		fixPremiumGFF(inFile, outFile, id);
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile, Common::UString &id) {

	inFile.clear();
	outFile.clear();
	std::vector<Common::UString> args;

	bool parseFail  = false;
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

			if        (argv[i] == "--id") {
				isOption = true;

				// Needs the ID as the next parameter
				if (i++ == (argv.size() - 1)) {
					parseFail = true;
					break;
				}

				id = argv[i];

			} else if (argv[i].beginsWith("-") || argv[i].beginsWith("--")) {
			  // An options, but we already checked for all known ones
				parseFail = true;
				break;
			}
		}

		// Was this a valid option? If so, don't try to use it as a file
		if (isOption)
			continue;

		// This is a file to use
		args.push_back(argv[i]);
	}

	if (parseFail || (args.size() != 2)) {
		printUsage(stderr, argv[0]);
		returnValue = 1;

		return false;
	}

	inFile  = args[0];
	outFile = args[1];

	return true;
}

void printUsage(FILE *stream, const Common::UString &name) {
	std::fprintf(stream, "Repair BioWare GFF files found in encrypted NWN premium module HAKs\n\n");
	std::fprintf(stream, "Usage: %s [<options>] <input file> <output file>\n", name.c_str());
	std::fprintf(stream, "  -h      --help              This help text\n");
	std::fprintf(stream, "          --version           Display version information\n\n");
	std::fprintf(stream, "          --id <id>           Write this GFF ID into the output file\n\n");
	std::fprintf(stream, "If no ID is given is given, it is guessed from the file name.\n");
}

static const uint32 kVersion32 = MKTAG('V', '3', '.', '2');
static const uint32 kVersion33 = MKTAG('V', '3', '.', '3');
static const uint32 kVersion40 = MKTAG('V', '4', '.', '0');
static const uint32 kVersion41 = MKTAG('V', '4', '.', '1');

void fixPremiumGFF(Common::UString &inFile, Common::UString &outFile, Common::UString &id) {
	if (id.empty()) {
		const Common::UString ext = Common::FilePath::getExtension(inFile);
		if (ext.size() != 4)
			throw Common::Exception("Failed to auto-detect the ID for file \"%s\"", inFile.c_str());

		id = Common::UString(++ext.begin(), ext.end()).toUpper();
	}

	if (id.size() > 4)
		throw Common::Exception("\"%s\" is not a valid GFF id", id.c_str());

	Common::SeekableReadStream *in = Common::ReadFile::readIntoMemory(inFile);

	try {
		const uint32 inID      = in->readUint32BE();
		const uint32 inVersion = in->readUint32BE();

		if ((inVersion == kVersion32) || (inVersion == kVersion33) ||
		    (inVersion == kVersion40) || (inVersion == kVersion41)) {

			status("\"%s\" is already a standard GFF file", inFile.c_str());

			if (inFile == outFile)
				return;

			Common::WriteFile out(outFile);

			in->seek(0);
			out.writeStream(*in);

			out.flush();
			out.close();
			return;
		}

		if ((FROM_BE_32(inID) < 0x30) || (FROM_BE_32(inID) > 0x12F))
			throw Common::Exception("File \"%s\" is neither a standard, nor a premium GFF file", inFile.c_str());

		const uint32 value = FROM_BE_32(inID) - 0x30;

		status("Repairing \"%s\" to a GFF with an ID of \"%s\" a correction value of %u",
		       inFile.c_str(), id.c_str(), value);

		in->seek(0);

		Common::WriteFile out(outFile);

		out.writeString(id);

		if (id.size() < 4)
			out.writeString(Common::UString(' ', 4 - id.size()));

		out.writeUint32BE(kVersion32);

		for (size_t i = 0; i < 6; i++) {
			const uint32 offset = in->readUint32LE();
			const uint32 count  = in->readUint32LE();

			if (offset < value)
				throw Common::Exception("File \"%s\" is neither a standard, nor a premium GFF file", inFile.c_str());

			out.writeUint32LE(offset - value + 0x08);
			out.writeUint32LE(count);
		}

		out.writeStream(*in);

		out.flush();
		out.close();
		return;

	} catch (...) {
		delete in;
		throw;
	}
}
