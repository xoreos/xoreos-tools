#include <iostream>
#include <string>

std::string fixLine(std::string line);
std::string parseLine(std::string line);
std::string trim(std::string line);
std::string fixOpenQuotes(std::string line);
std::string escapeInnerQuotes(std::string line);
std::string replaceString(std::string& origStr, std::string& oldText, std::string newText);
void replaceAll(std::string& str, const std::string& from, const std::string& to);
int countOccurances(std::string line, char find);
std::string fixCopyright(std::string line);
std::string fixXMLTag(std::string line);
std::string doubleDashFix(std::string line);
std::string tripleQuoteFix(std::string line);
void countComments(std::string line);
std::string quotedCloseFix(std:: string line);
std::string escapeSpacedStrings(std::string line, bool undo);
std::string fixMismatchedParen(std::string line);
std::string fixCloseBraceQuote(std::string line);
std::string fixUnclosedQuote(std::string line);
std::string fixUnevenQuotes(std::string line);
std::string fixUnclosedNodes(std::string line);
