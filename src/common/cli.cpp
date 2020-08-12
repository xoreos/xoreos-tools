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

#include <cstring>

#include <list>
#include <set>

#include "src/version/version.h"

#include "src/common/cli.h"
#include "src/common/string.h"

namespace Common {

namespace CLI {

static void cliSetHelp(UString &helpStr, const char *longName,
                       char shortName, const char *optionArgs,
                       uint32_t maxArgLength, const char *help) {
	helpStr += "  ";
	if (shortName) {
		helpStr += '-';
		helpStr += shortName;
	} else {
		helpStr += "  ";
	}
	helpStr += "      --";
	helpStr += longName;
	helpStr += "  ";
	helpStr += optionArgs;
	for (int i = (maxArgLength - strlen(longName) - strlen(optionArgs) - 4);
	     i > 0; --i) {
		helpStr += " ";
	}
	if (help) {
		helpStr += help;
	}
	helpStr += "\n";
}

template<>
int ValGetter<UString &>::get(const std::vector<UString> &args, int i, int) {
	_val = args[i];
	return 0;
}

template<>
int ValGetter<uint32_t &>::get(const std::vector<UString> &args, int i, int) {
	const char *str = args[i].c_str();

	for (int j = 0;j < str[j]; ++j) {
		if (!String::isDigit(str[j]))
			return -1;
	}
	_val = atoi(str);
	return 0;
}

template<>
int ValGetter<int32_t &>::get(const std::vector<UString> &args, int i, int) {
	const char *str = args[i].c_str();
	int j = 0;

	if (str[0] == '-')
		++j;
	for (;j < str[j]; ++j) {
		if (!String::isDigit(str[j]))
			return -1;
	}
	_val = atoi(str);
	return 0;
}

template<>
int ValGetter<std::vector<UString> &>::get(const std::vector<UString> &args,
                                                   int i, int size) {
	int j = i;

	for (; j < size; ++j)
		_val.push_back(args[j]);
	return j - i;
}

template<>
int ValGetter<std::list<UString> &>::get(const std::vector<UString> &args,
                                                 int i, int size) {
	int j = i;

	for (; j < size; ++j)
		_val.push_back(args[j]);
	return j - i;
}

template<>
int ValGetter<std::set<UString> &>::get(const std::vector<UString> &args,
                                                int i, int size) {
	int j = i;

	for (; j < size; ++j)
		_val.insert(args[j]);
	return j - i;
}

int Option::doOption(const std::vector<UString> &args, int i, int size) {
	switch (_type) {
		case kAssigner:
			assign();
			break;
		case kPrinter:
			if (this->_printer.print()) {
				_printer.print()(_printer.printerStr());
			} else {
				_printer.vPrint()();
			}
			break;
		case kCallback:
			if (i + 1 < size)
				_callback->process(args[i + 1]);
			return 1;
		case kGetter:
			if (i + 1 < size)
				return _getter->get(args, i + 1, size) + 1;
			break;
		default:
			break;
	}
	return 0;
}

void Option::assign() {
	for (int i = 0, end = _assigners.size(); i < end; ++i)
		_assigners[i]->assign();
}

void Option::free() {
	switch (_type) {
		case kGetter:
			delete _getter;
			break;
		case kCallback:
			delete _callback;
			break;
		case kAssigner:
			for (int i = 0, end = _assigners.size(); i < end; ++i)
				delete _assigners[i];
			break;
		default:
			break;
	}
}

void NoOption::free() {
	delete _getter;
}

Parser::Parser(const UString &name, const char *description, const char *bottom,
               int &returnVal, std::vector<NoOption> endCli) :
	_bottom(bottom), _helpStr(description), _options(), _noOptions(endCli), _returnVal(returnVal) {

	_helpStr += "\n\nUsage: ";
	_helpStr += name;
	_helpStr += " [<options>]";
	for (unsigned int i = 0; i < endCli.size(); ++i) {
		if (!endCli[i].isOptional()) {
			_helpStr += " <";
			_helpStr += endCli[i].getter()->name();
			_helpStr += ">";
		} else {
			_helpStr += " [";
			_helpStr += endCli[i].getter()->name();
			_helpStr += "]";
		}
	}
	_helpStr += "\n\n";
	this->addOption("help", 'h', "This help text", kEndSucess, printUsage, _helpStr);
	this->addOption("version", 0, "Display version information",
			kEndSucess, Version::printVersion);
}

Parser::~Parser() {
	for (int i = 0, end  = _options.size(); i < end; ++i) {
		if (_options[i])
			_options[i]->free();
		delete _options[i];
	}
	for (int i = 0, end  = _noOptions.size(); i < end; ++i) {
		_noOptions[i].free();
	}
}

void Parser::addOption(const char *longName, char shortName, const char *help,
                       OptionRet ret, void (*printer)()) {
	_options.push_back(new Option(longName, shortName, help, ret, printer));
}

void Parser::addOption(const char *longName, char shortName, const char *help,
                       OptionRet ret, void (*printer)(UString &),
                       UString &str) {
	_options.push_back(new Option(longName, shortName, help, ret, printer, str));
}

void Parser::addOption(const char *longName, char shortName, const char *help,
                       OptionRet ret, std::vector<Assigner *> assigners) {
	_options.push_back(new Option(longName, shortName, help, ret, assigners));
}

void Parser::addOption(const char *longName, char shortName, const char *help,
                       OptionRet ret, Getter *getter) {
	_options.push_back(new Option(longName, shortName, help, ret, getter));
}

void Parser::addOption(const char *longName, char shortName, const char *help,
                       OptionRet ret, CallbackBase *callback) {
	_options.push_back(new Option(longName, shortName, help, ret, callback));
}


Option *findMatchOptionShortName(std::vector<Option *> options,
                                 const UString &str) {
	for (int i = 0, size = options.size(); i < size; ++i) {
		Option *option = options[i];

		if ((UString('-') + option->shortName()) == str)
			return option;
	}
	return 0;
}

Option *findMatchOptionName(std::vector<Option *> options,
                            const UString &str) {
	for (int i = 0, size = options.size(); i < size; ++i) {
		Option *option = options[i];

		if ((UString("--") + option->name()) == str)
			return option;
	}
	return 0;
}

bool Parser::process(const std::vector<UString> &argv) {
	// Build usage string
	int maxArgLength = 17;

	/*
	 * I need to iterated twice here, first time to determine maxArgLength
	 * 2nd time to build the help.
	 */
	for (size_t i = 0; i < _options.size(); ++i) {
		Option &option = *_options[i];
		int tmpLen = strlen(option.name().c_str());

		if (option.type() == Option::kGetter)
			tmpLen += strlen(option.getter()->name()) + 8;
		else if (option.type() == Option::kCallback)
			tmpLen += strlen(option.callback()->argName()) + 8;
		if (tmpLen > maxArgLength)
			maxArgLength = tmpLen + 2;
	}
	_helpStr += "Options:\n";
	for (size_t i = 0; i < _options.size(); ++i) {
		Option &option = *_options[i];
		UString str;

		if (option.type() == Option::kSpace) {
			_helpStr += '\n';
			continue;
		} else if (option.type() == Option::kGetter) {
			str += "<";
			str += option.getter()->name();
			str += ">";
		} else if (option.type() == Option::kCallback) {
			str += "<";
			str += option.callback()->argName();
			str += ">";
		}
		cliSetHelp(_helpStr, option.name().c_str(), option.shortName(),
			     str.c_str(), maxArgLength, option.help().c_str());
	}
	if (_bottom.size())
		_helpStr += "\n";
	_helpStr += _bottom;

	for (size_t nbArgs = argv.size(), i = 1; i < nbArgs; ++i) {
		Option *option;

		if ((isShorOption(argv[i]) && (option = findMatchOptionShortName(_options, argv[i])) != 0) ||
		    (isLongOption(argv[i]) && (option = findMatchOptionName(_options, argv[i])) != 0)) {
			int ret = option->doOption(argv, i, nbArgs);

			i += ret;
			if (ret < 0)
				goto fail;
			else if (option->returnVal() == kContinueParsing)
				continue;
			else if (option->returnVal() == kEndSucess) {
				_returnVal = 0;
				return false;
			}
			goto fail;
		} else if (_noOptions.size()) {
			int ret = _noOptions[0].getter()->get(argv, i, nbArgs);
			if (ret < 0)
				goto fail;
			i += ret;
			_noOptions[0].free();
			_noOptions.erase(_noOptions.begin());
			continue;
		} else {
			goto fail;
		}
	}
	if (_noOptions.size() > 0 && !_noOptions[0].isOptional())
		goto fail;
	return true;
fail:
	_returnVal = 1;
	this->printUsage(_helpStr);
	return false;
}

} // End of namespace CLI

} // End of namespace Common
