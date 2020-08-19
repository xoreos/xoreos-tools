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
#include <memory>

#include "src/version/version.h"

#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/filepath.h"
#include "src/common/memreadstream.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/cli.h"

#include "src/aurora/types.h"

#include "src/util.h"

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile, Common::UString &id);

void fixPremiumGFF(Common::UString &inFile, Common::UString &outFile, Common::UString &id);

int main(int argc, char **argv) {
	initPlatform();

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
	using Common::CLI::Parser;
	using Common::CLI::ValGetter;
	using Common::CLI::NoOption;
	using Common::CLI::makeEndArgs;
	using Common::CLI::kContinueParsing;
	NoOption inFileOpt(false, new ValGetter<Common::UString &>(inFile, "input files"));
	NoOption outFileOpt(false, new ValGetter<Common::UString &>(outFile, "output files"));
	Parser parser(argv[0], "Repair BioWare GFF files found in encrypted NWN premium module HAKs\n",
	              "If no ID is given is given, it is guessed from the file name.",
	              returnValue, makeEndArgs(&inFileOpt, &outFileOpt));


	parser.addSpace();
	parser.addOption("id", "Write this GFF ID into the output file",
			 kContinueParsing, new ValGetter<Common::UString &>(id, "id"));

	return parser.process(argv);
}

static const uint32_t kVersion32 = MKTAG('V', '3', '.', '2');
static const uint32_t kVersion33 = MKTAG('V', '3', '.', '3');
static const uint32_t kVersion40 = MKTAG('V', '4', '.', '0');
static const uint32_t kVersion41 = MKTAG('V', '4', '.', '1');

void fixPremiumGFF(Common::UString &inFile, Common::UString &outFile, Common::UString &id) {
	if (id.empty()) {
		const Common::UString ext = Common::FilePath::getExtension(inFile);
		if (ext.size() != 4)
			throw Common::Exception("Failed to auto-detect the ID for file \"%s\"", inFile.c_str());

		id = Common::UString(++ext.begin(), ext.end()).toUpper();
	}

	if (id.size() > 4)
		throw Common::Exception("\"%s\" is not a valid GFF id", id.c_str());

	std::unique_ptr<Common::SeekableReadStream> in(Common::ReadFile::readIntoMemory(inFile));

	const uint32_t inID      = in->readUint32BE();
	const uint32_t inVersion = in->readUint32BE();

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

	const uint32_t value = FROM_BE_32(inID) - 0x30;

	status("Repairing \"%s\" to a GFF with an ID of \"%s\" a correction value of %u",
	       inFile.c_str(), id.c_str(), value);

	in->seek(0);

	Common::WriteFile out(outFile);

	out.writeString(id);

	if (id.size() < 4)
		out.writeString(Common::UString(' ', 4 - id.size()));

	out.writeUint32BE(kVersion32);

	for (size_t i = 0; i < 6; i++) {
		const uint32_t offset = in->readUint32LE();
		const uint32_t count  = in->readUint32LE();

		if (offset < value)
			throw Common::Exception("File \"%s\" is neither a standard, nor a premium GFF file", inFile.c_str());

		out.writeUint32LE(offset - value + 0x08);
		out.writeUint32LE(count);
	}

	out.writeStream(*in);

	out.flush();
	out.close();
}
