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

#include "src/common/ustring.h"
#include "src/common/stream.h"

#include "src/xml/xmlwriter.h"

namespace XML {

XMLWriter::XMLWriter(Common::WriteStream &stream) : _stream(&stream), _needIndent(false) {
	writeHeader();
}

XMLWriter::~XMLWriter() {
	flush();
}

void XMLWriter::flush() {
	while (!_openTags.empty())
		closeTag();

	_stream->flush();
}

void XMLWriter::writeHeader() {
	_stream->writeString("<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\n");
	flush();
}

void XMLWriter::openTag(const Common::UString &name) {
	if (!_openTags.empty()) {
		_openTags.back().empty = false;

		indent(_openTags.size());
		writeTag();
	}

	_openTags.push_back(Tag());

	Tag &tag = _openTags.back();

	tag.name  = name;
	tag.empty = true;
}

void XMLWriter::closeTag() {
	if (_openTags.empty())
		return;

	writeTag();

	const Tag &tag = _openTags.back();

	if (!tag.empty) {
		indent(_openTags.size() - 1);
		_stream->writeString("</" + tag.name + ">");
	}

	_openTags.pop_back();
}

void XMLWriter::writeTag() {
	if (_openTags.empty() || _openTags.back().written)
		return;

	Tag &tag = _openTags.back();

	tag.written = true;

	_stream->writeString("<" + tag.name);

	for (std::list<Property>::const_iterator p = tag.properties.begin(); p != tag.properties.end(); ++p)
		_stream->writeString(" " + p->name + "=\"" + escape(p->value) + "\"");

	if (tag.empty)
		_stream->writeString("/");

	_stream->writeString(">");

	if (!tag.empty) {
		if (!tag.base64.empty()) {

			while (!tag.base64.empty()) {
				breakLine();
				indent(_openTags.size());
				_stream->writeString(tag.base64.front());
				tag.base64.pop_front();
			}
			breakLine();

		} else
			_stream->writeString(escape(tag.contents));
	}
}

void XMLWriter::indent(uint level) {
	if (!_needIndent)
		return;

	for (uint i = 0; i < level; i++)
		_stream->writeString("  ");

	_needIndent = false;
}

Common::UString XMLWriter::escape(const Common::UString &str) {
	Common::UString escaped;

	for (Common::UString::iterator s = str.begin(); s != str.end(); ++s) {
		uint32 c = *s;

		if      (c == '\"')
			escaped += "&quot;";
		else if (c == '\'')
			escaped += "&apos;";
		else if (c == '&')
			escaped += "&amp;";
		else if (c == '<')
			escaped += "&lt;";
		else if (c == '>')
			escaped += "&gt;";
		else
			escaped += c;

	}

	return escaped;
}

void XMLWriter::addProperty(const Common::UString &name, const Common::UString &value) {
	if (_openTags.empty())
		return;

	Tag &tag = _openTags.back();

	tag.properties.push_back(Property());

	Property &property = tag.properties.back();

	property.name  = name;
	property.value = value;
}

void XMLWriter::setContents(const Common::UString &contents) {
	if (_openTags.empty())
		return;

	Tag &tag = _openTags.back();

	tag.base64.clear();

	tag.contents = contents;
	tag.empty    = false;
}

void XMLWriter::setContents(const byte *data, uint32 size) {
	if (_openTags.empty())
		return;

	Tag &tag = _openTags.back();

	tag.base64.clear();
	tag.contents.clear();

	Common::MemoryReadStream stream(data, size);
	encodeBase64(tag.base64, stream);

	tag.empty = false;
}

void XMLWriter::setContents(Common::SeekableReadStream &stream) {
	if (_openTags.empty())
		return;

	Tag &tag = _openTags.back();

	tag.base64.clear();
	tag.contents.clear();

	encodeBase64(tag.base64, stream);

	tag.empty = false;
}

void XMLWriter::breakLine() {
	if (!_openTags.empty()) {
		_openTags.back().empty = false;
		writeTag();
	}

	_stream->writeString("\n");
	_needIndent = true;
}

static const char kBase64Char[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void XMLWriter::encodeBase64(std::list<Common::UString> &base64, Common::SeekableReadStream &data) {
	uint8 n;
	byte buf[3];

	base64.push_back(Common::UString());

	// Read up to 3 characters at a time
	while ((n = data.read(buf, 3)) != 0) {
		// We have a max line length of 64 characters
		if (base64.back().size() == 64)
			base64.push_back(Common::UString());

		uint32 code = 0;

		// Concat the input characters
		for (uint8 i = 0; i < n; i++)
			code |= buf[i] << (24 - i * 8);

		// Create up to 4 6-bit base64-characters out of them
		for (uint8 i = 0; i < (n + 1); i++) {
			base64.back() += kBase64Char[(code >> 26) & 0x0000003F];
			code <<= 6;
		}

		// Add padding
		for (int i = 0; i < (3 - n); i++)
			base64.back() += '=';
	}

	if (base64.back().empty())
		base64.pop_back();
}

} // End of namespace XML
