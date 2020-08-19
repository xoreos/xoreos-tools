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
 *  Creates V3.2 GFFs out of XML files.
 */

#include "src/common/strutil.h"
#include "src/common/base64.h"

#include "src/xml/gff3creator.h"

namespace XML {

void GFF3Creator::create(const XML::XMLNode &root, uint32_t id, Common::WriteStream &file, uint32_t version) {
	Aurora::GFF3Writer gff3(id, version);

	if (root.getChildren().size() > 1)
		throw Common::Exception("GFF3Creator::create() More than one root struct");

	if (root.getChildren().size() == 0)
		throw Common::Exception("GFF3Creator::create() No root struct");

	const XML::XMLNode &rootStruct = *root.getChildren().front();

	Common::parseString(rootStruct.getProperty("id"), id);
	if (id != 0xFFFFFFFF)
		throw Common::Exception("GFF3Creator::create() Invalid root struct id");

	readStructContents(rootStruct.getChildren(), gff3.getTopLevel());

	gff3.write(file);
}

void GFF3Creator::readStructContents(const XMLNode::Children &strctNodes, Aurora::GFF3WriterStructPtr strctPtr) {
	for (const auto &strctNode : strctNodes) {
		if (strctNode->getName() == "byte") {
			uint8_t value;
			Common::parseString(strctNode->findChild("text")->getContent(), value);
			strctPtr->addByte(strctNode->getProperty("label"), value);
		} else if (strctNode->getName() == "char") {
			int8_t value;
			Common::parseString(strctNode->findChild("text")->getContent(), value);
			strctPtr->addChar(strctNode->getProperty("label"), value);
		} else if (strctNode->getName() == "sint16") {
			int16_t value;
			Common::parseString(strctNode->findChild("text")->getContent(), value);
			strctPtr->addSint16(strctNode->getProperty("label"), value);
		} else if (strctNode->getName() == "float") {
			float value;
			Common::parseString(strctNode->findChild("text")->getContent(), value);
			strctPtr->addFloat(strctNode->getProperty("label"), value);
		} else if (strctNode->getName() == "float") {
			double value;
			Common::parseString(strctNode->findChild("text")->getContent(), value);
			strctPtr->addDouble(strctNode->getProperty("label"), value);
		} else if (strctNode->getName() == "sint32") {
			int32_t value;
			Common::parseString(strctNode->findChild("text")->getContent(), value);
			strctPtr->addSint32(strctNode->getProperty("label"), value);
		} else if (strctNode->getName() == "sint64") {
			int64_t value;
			Common::parseString(strctNode->findChild("text")->getContent(), value);
			strctPtr->addSint64(strctNode->getProperty("label"), value);
		} else if (strctNode->getName() == "uint16_t") {
			uint16_t value;
			Common::parseString(strctNode->findChild("text")->getContent(), value);
			strctPtr->addUint16(strctNode->getProperty("label"), value);
		} else if (strctNode->getName() == "uint32_t") {
			uint32_t value;
			Common::parseString(strctNode->findChild("text")->getContent(), value);
			strctPtr->addUint32(strctNode->getProperty("label"), value);
		} else if (strctNode->getName() == "uint64_t") {
			uint64_t value;
			Common::parseString(strctNode->findChild("text")->getContent(), value);
			strctPtr->addUint64(strctNode->getProperty("label"), value);
		} else if (strctNode->getName() == "exostring") {
			const XMLNode *text = strctNode->findChild("text");
			if (text) {
				bool base64 = false;
				Common::parseString(strctNode->getProperty("base64"), base64, true);

				const Common::UString contents = text->getContent();
				if (base64) {
					Common::SeekableReadStream *debase64 = Common::decodeBase64(contents);
					strctPtr->addExoString(strctNode->getProperty("label"), debase64);

				} else
					strctPtr->addExoString(strctNode->getProperty("label"), contents);

			} else
				strctPtr->addExoString(strctNode->getProperty("label"), "");

		} else if (strctNode->getName() == "strref") {
			uint32_t value;
			Common::parseString(strctNode->findChild("text")->getContent(), value);
			strctPtr->addStrRef(strctNode->getProperty("label"), value);
		} else if (strctNode->getName() == "resref") {
			const XMLNode *text = strctNode->findChild("text");
			if (text) {
				bool base64 = false;
				Common::parseString(strctNode->getProperty("base64"), base64, true);

				const Common::UString contents = text->getContent();
				if (base64) {
					Common::SeekableReadStream *debase64 = Common::decodeBase64(contents);
					strctPtr->addResRef(strctNode->getProperty("label"), debase64);

				} else
					strctPtr->addResRef(strctNode->getProperty("label"), contents);

			} else
				strctPtr->addResRef(strctNode->getProperty("label"), "");

		} else if (strctNode->getName() == "data") {
			const XMLNode *textNode = strctNode->findChild("text");
			Common::UString text = textNode ? textNode->getContent() : "";

			Common::SeekableReadStream *debase64 = Common::decodeBase64(text);
			strctPtr->addVoid(strctNode->getProperty("label"), debase64);
		} else if (strctNode->getName() == "vector") {
			float x, y, z;

			if (strctNode->getChildren().size() != 3)
				throw Common::Exception("GFF3Creator::readStructContents() Invalid size of vector components");

			XMLNode::Children::const_iterator iter = strctNode->getChildren().begin();
			const XMLNode *xValue = (*iter++)->findChild("text");
			const XMLNode *yValue = (*iter++)->findChild("text");
			const XMLNode *zValue = (*iter++)->findChild("text");

			if (!xValue || !yValue || !zValue)
				throw Common::Exception("GFF3Creator::readStructContents() Vector components empty");

			Common::parseString(xValue->getContent(), x);
			Common::parseString(yValue->getContent(), y);
			Common::parseString(zValue->getContent(), z);

			strctPtr->addVector(strctNode->getProperty("label"), x, y, z);
		} else if (strctNode->getName() == "orientation") {
			float x, y, z, w;

			if (strctNode->getChildren().size() != 4)
				throw Common::Exception("GFF3Creator::readStructContents() Invalid size of orientation components");

			XMLNode::Children::const_iterator iter = strctNode->getChildren().begin();
			const XMLNode *xValue = (*iter++)->findChild("text");
			const XMLNode *yValue = (*iter++)->findChild("text");
			const XMLNode *zValue = (*iter++)->findChild("text");
			const XMLNode *wValue = (*iter++)->findChild("text");

			if (!xValue || !yValue || !zValue)
				throw Common::Exception("GFF3Creator::readStructContents() Orientation components empty");

			Common::parseString(xValue->getContent(), x);
			Common::parseString(yValue->getContent(), y);
			Common::parseString(zValue->getContent(), z);
			Common::parseString(wValue->getContent(), w);

			strctPtr->addOrientation(strctNode->getProperty("label"), x, y, z, w);
		} else if (strctNode->getName() == "locstring") {
			uint32_t strref;
			Aurora::LocString locString;

			Common::parseString(strctNode->getProperty("strref"), strref);
			locString.setID(strref);

			if (!strctNode->getChildren().empty()) {
				for (const auto &child : strctNode->getChildren()) {
					if (child->getName() != "string")
						throw Common::Exception("GFF3Creator::readStructContents() Invalid LocString string");

					const XMLNode *text = child->findChild("text");

					uint32_t id;
					Common::parseString(child->getProperty("language"), id);
					locString.setStringRawLanguageID(id, text ? text->getContent() : "");
				}
			}

			strctPtr->addLocString(strctNode->getProperty("label"), locString);
		} else if (strctNode->getName() == "struct") {
			Common::UString idText = strctNode->getProperty("id");

			Aurora::GFF3WriterStructPtr strct = nullptr;
			if (!idText.empty()) {
				uint32_t id;
				Common::parseString(idText, id);
				strct = strctPtr->addStruct(strctNode->getProperty("label"), id);
			} else
				strct = strctPtr->addStruct(strctNode->getProperty("label"));

			readStructContents(strctNode->getChildren(), strct);
		} else if (strctNode->getName() == "list") {
			Aurora::GFF3WriterListPtr list = strctPtr->addList(strctNode->getProperty("label"));
			readListContents(strctNode->getChildren(), list);
		}
	}
}

void GFF3Creator::readListContents(const XMLNode::Children &listNodes, Aurora::GFF3WriterListPtr listPtr) {
	for (const auto &node : listNodes) {
		if (node->getName() != "struct")
			throw Common::Exception("GFF3Creator::readListContents() Invalid element in list");

		Common::UString idText = node->getProperty("id");

		Aurora::GFF3WriterStructPtr strct = nullptr;
		if (!idText.empty()) {
			uint32_t id;
			Common::parseString(idText, id);
			strct = listPtr->addStruct(node->getProperty("label"), id);
		} else
			strct = listPtr->addStruct(node->getProperty("label"));

		readStructContents(node->getChildren(), strct);
	}
}

} // End of namespace XML
