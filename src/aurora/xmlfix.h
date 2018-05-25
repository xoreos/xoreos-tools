#include <iostream>
#include <string>

namespace Common {
	class Ustring;
	class SeekableReadStream;
}

namespace Aurora {

class XMLFix { 

public:
	static Common::SeekableReadStream *fixXML(Common::SeekableReadStream *xml);

private:
	Common::UString fixLine(Common::UString line);
	Common::UString parseLine(Common::UString line);
	Common::UString trim(Common::UString line);
	Common::UString fixOpenQuotes(Common::UString line);
	Common::UString escapeInnerQuotes(Common::UString line);
	Common::UString replaceString(Common::UString& origStr, Common::UString& oldText, Common::UString newText);
	void replaceAll(Common::UString& str, const Common::UString& from, const Common::UString& to);
	int countOccurances(Common::UString line, char find);
	Common::UString fixCopyright(Common::UString line);
	Common::UStringg fixXMLTag(Common::UString line);
	Common::UString doubleDashFix(Common::UString line);
	Common::UString tripleQuoteFix(Common::UString line);
	void countComments(std::string line);
	Common::UString quotedCloseFix(Common::UString line);
	Common::UString escapeSpacedStrings(Common::UString line, bool undo);
	Common::UString fixMismatchedParen(Common::UString line);
	Common::UString fixCloseBraceQuote(Common::UString line);
	Common::UString fixUnclosedQuote(Common::UString line);
	Common::UString fixUnevenQuotes(Common::UString line);
	Common::UString fixUnclosedNodes(Common::UString line);

};

} // End of namespace Aurora


