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
 *  Utility class for writing XML files.
 */

#ifndef XML_XMLWRITER_H
#define XML_XMLWRITER_H

#include <list>

#include "src/common/ustring.h"

namespace Common {
	class SeekableReadStream;
	class WriteStream;
}

namespace XML {

class XMLWriter {
public:
	XMLWriter(Common::WriteStream &stream);
	~XMLWriter();

	/** Close all open tags and flush the stream. */
	void flush();

	/** Open a tag. */
	void openTag(const Common::UString &name);
	/** Close the last opened tag. */
	void closeTag();

	/** Add a property. The value will be properly escaped. */
	void addProperty(const Common::UString &name, const Common::UString &value);
	/** Set contents to this string, which will be properly escaped. */
	void setContents(const Common::UString &contents);
	/** Set the contents to binary data, which will be base64 encoded. */
	void setContents(const byte *data, size_t size);
	/** Set the contents to binary data, which will be base64 encoded. */
	void setContents(Common::SeekableReadStream &stream);

	/** Add a line break. */
	void breakLine();

private:
	struct Property {
		Common::UString name;
		Common::UString value;
	};

	struct Tag {
		Common::UString name;

		std::list<Property> properties;

		Common::UString contents;
		std::list<Common::UString> base64;

		bool written;
		bool empty;
	};

	Common::WriteStream *_stream;

	std::list<Tag> _openTags;
	bool _needIndent;


	void writeHeader();

	void indent(size_t level);
	void writeTag();

	Common::UString escape(const Common::UString &str);
};

} // End of namespace XML

#endif // XML_XMLWRITER_H
