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
 *  Tool to extract NSBTX textures into TGA images.
 */

#include <cstring>
#include <cstdio>

#include "src/common/ustring.h"
#include "src/common/error.h"
#include "src/common/file.h"

#include "src/aurora/util.h"
#include "src/aurora/nsbtxfile.h"

#include "src/images/xoreositex.h"

#include "src/util.h"

enum Command {
	kCommandNone    = -1,
	kCommandList    =  0,
	kCommandExtract     ,
	kCommandMAX
};

const char *kCommandChar[kCommandMAX] = { "l", "e" };

void printUsage(FILE *stream, const char *name);
bool parseCommandLine(int argc, char **argv, int &returnValue, Command &command, Common::UString &file);

void listFiles(Aurora::NSBTXFile &nsbtx);
void extractFiles(Aurora::NSBTXFile &nsbtx);

int main(int argc, char **argv) {
	int returnValue;
	Command command;
	Common::UString file;
	if (!parseCommandLine(argc, argv, returnValue, command, file))
		return returnValue;

	try {
		Aurora::NSBTXFile nsbtx(new Common::File(file));

		if      (command == kCommandList)
			listFiles(nsbtx);
		else if (command == kCommandExtract)
			extractFiles(nsbtx);

	} catch (Common::Exception &e) {
		Common::printException(e);
		return -1;
	}

	return 0;
}

bool parseCommandLine(int argc, char **argv, int &returnValue, Command &command, Common::UString &file) {
	file.clear();

	// No command, just display the help
	if (argc == 1) {
		printUsage(stdout, argv[0]);
		returnValue = 0;

		return false;
	}

	// Wrong number of arguments, display the help
	if (argc != 3) {
		printUsage(stderr, argv[0]);
		returnValue = -1;

		return false;
	}

	// Find out what we should do
	command = kCommandNone;
	for (int i = 0; i < kCommandMAX; i++)
		if (!strcmp(argv[1], kCommandChar[i]))
			command = (Command) i;

	// Unknown command
	if (command == kCommandNone) {
		printUsage(stderr, argv[0]);
		returnValue = -1;

		return false;
	}

	// This is the file to use
	file = argv[2];

	return true;
}

void printUsage(FILE *stream, const char *name) {
	std::fprintf(stream, "Nintendo NSBTX texture extractor\n\n");
	std::fprintf(stream, "Usage: %s <command> <file>\n\n", name);
	std::fprintf(stream, "Commands:\n");
	std::fprintf(stream, "  l          List texture\n");
	std::fprintf(stream, "  e          Extract images to current directory\n");
}

void listFiles(Aurora::NSBTXFile &nsbtx) {
	const Aurora::Archive::ResourceList &resources = nsbtx.getResources();
	const uint32 fileCount = resources.size();

	std::printf("Number of files: %u\n\n", fileCount);

	std::printf("      Filename       |    Size\n");
	std::printf("=====================|===========\n");

	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r)
		std::printf("%16s.tga | %10d\n", r->name.c_str(), nsbtx.getResourceSize(r->index));
}

void dumpImage(Common::SeekableReadStream &stream, const Common::UString &fileName) {
	Images::XEOSITEX itex(stream);

	itex.flipVertically();

	itex.dumpTGA(fileName);
}

void extractFiles(Aurora::NSBTXFile &nsbtx) {
	const Aurora::Archive::ResourceList &resources = nsbtx.getResources();
	const uint32 fileCount = resources.size();

	std::printf("Number of files: %u\n\n", fileCount);

	uint i = 1;
	for (Aurora::Archive::ResourceList::const_iterator r = resources.begin(); r != resources.end(); ++r, ++i) {
		const Common::UString fileName = r->name + ".tga";

		std::printf("Extracting %d/%d: %s ... ", i, fileCount, fileName.c_str());

		Common::SeekableReadStream *stream = 0;
		try {
			stream = nsbtx.getResource(r->index);

			dumpImage(*stream, fileName);

			std::printf("Done\n");
		} catch (Common::Exception &e) {
			std::printf("\n");
			Common::printException(e, "");
		}

		delete stream;
	}

}
