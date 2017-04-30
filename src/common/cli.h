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

#ifndef COMMON_CLI_H
#define COMMON_CLI_H

#include <cstdio>
#include <cstdio>
#include <stdarg.h>
#include <stdlib.h>

#include "src/common/ustring.h"

namespace Common {

namespace CLI {

class Getter;
struct NoOption;

enum OptionRet {
	kContinueParsing = 0,
	kEndSucess,
	kEndFail
};

class Assigner {
public:
	Assigner() {}
	virtual ~Assigner() {}

	virtual void assign() = 0;
};

class Getter {
public:
	Getter(const char *aName) : _name(aName) {}
	virtual ~Getter() {}

	virtual int get(const std::vector<Common::UString> &args, int i, int size) = 0;
	const char *name() const { return _name; }
private:
	const char *_name;
};

template <typename T>
struct ValGetter : public Getter {
public:
	ValGetter(T val, const char *aName) : Getter(aName), _val(val) {}
	virtual ~ValGetter() {}

	int get(const std::vector<Common::UString> &args, int i, int size);
private:
	T _val;
};

class CallbackBase {
public:
	CallbackBase(const char *aName) : _argName(aName) {}
	virtual ~CallbackBase() {}

	virtual bool process(const UString &str) = 0;
	const char *argName() const { return _argName; }
private:
	const char *_argName;
};

template <typename U>
class Callback : public CallbackBase {
public:
	Callback(const char *name, bool (*aCallback)(const UString &, U), U anArg) :
		CallbackBase(name), callback(aCallback), _callbackArg(anArg) {}
	virtual ~Callback() {}

	bool (*callback)(const UString &str, U arg);
	bool process(const UString &str) {
		return callback(str, _callbackArg);
	}

private:
	U _callbackArg;
};

template <typename T>
class ValAssigner : public Assigner {
public:
	ValAssigner(T val, T &target) :
		_val(val), _target(target) {}

	void assign() { _target = _val; }

private:
	T _val;
	T &_target;
};

class Printer {
public:
	Printer() : _vPrint(0), _print(0), _printerStr(_useless) {}
	Printer(const Printer &other) :
		_vPrint(other._vPrint), _print(other._print), _printerStr(other._printerStr) {}
	Printer(void (*aPrinter)()) :
		_vPrint(aPrinter), _print(0), _printerStr(_useless) {}
	Printer(void (*aPrinter)(Common::UString &), Common::UString &str) :
		_vPrint(0), _print(aPrinter), _printerStr(str) {}

	void (*vPrint())() { return _vPrint; }
	void (*print())(Common::UString &)  { return _print; }

	const UString &useless() const { return _useless; }
	UString &printerStr() const { return _printerStr; }

public:
	void (*_vPrint)();
	void (*_print)(Common::UString &);

	UString _useless;
	UString &_printerStr;
};

class NoOption {
public:
	NoOption(bool optional, Getter *aGetter) :
		_getter(aGetter), _isOptional(optional) {}

	void free();
	Getter *getter() const { return _getter; }
	bool isOptional() const { return _isOptional; }
private:
	Getter *_getter;
	bool _isOptional;
};

class Option {
public:
	Option(const char *aName, char aShortName,
	       const char *anHelp, OptionRet ret, void (*aPrinter)()) :
		_type(kPrinter), _name(aName), _shortName(aShortName),
		_help(anHelp), _returnVal(ret), _getter(0), _callback(0),
		_assigners(), _printer(aPrinter) {}
	Option(const char *aName, char aShortName,
	       const char *anHelp, OptionRet ret, void (*aPrinter)(Common::UString &),
	       Common::UString &printerStr) :
		_type(kPrinter), _name(aName), _shortName(aShortName),
		_help(anHelp), _returnVal(ret), _getter(0),
		_printer(aPrinter, printerStr) {}

	Option(const char *aName, char aShortName,
	       const char *anHelp, OptionRet ret,
	       std::vector<Assigner *> &anAssigners) :
		_type(kAssigner), _name(aName), _shortName(aShortName),
		_help(anHelp), _returnVal(ret), _getter(0),
		_callback(0), _assigners(anAssigners) {}

	Option(const char *aName, char aShortName, const char *anHelp, OptionRet ret, Getter *aGetter) :
		_type(kGetter), _name(aName), _shortName(aShortName),
		_help(anHelp), _returnVal(ret), _getter(aGetter) {}

	Option(const char *aName, char aShortName, const char *anHelp,
	       OptionRet ret, CallbackBase *aCallback) :
		_type(kCallback), _name(aName), _shortName(aShortName),
		_help(anHelp), _returnVal(ret), _getter(0),
		_callback(aCallback) {}
	Option() : _type(kSpace) {}

	~Option() {}

	enum Type {
		kAssigner,
		kPrinter,
		kGetter,
		kCallback,
		kSpace
	};

	int doOption(const std::vector<Common::UString> &args, int i, int size);
	void assign();
	void free();
	Type type() const { return _type; }
	const Common::UString &name() const { return _name; }
	char shortName() const { return _shortName; }
	const Common::UString &help() const { return _help; }
	OptionRet returnVal() const { return _returnVal; }
	Getter *getter() const { return _getter; }
	CallbackBase *callback() const { return _callback; }

private:
	Type _type;
	Common::UString _name;
	char _shortName;
	Common::UString _help;
	OptionRet _returnVal;
	Getter *_getter;
	CallbackBase *_callback;
	std::vector<Assigner *> _assigners;
	Printer _printer;
};

class Parser {
public:

	Parser(const UString &name, const char *description,
	       const char *bottom, int &returnVal,
	       std::vector<NoOption> endCli);
	~Parser();

	void addSpace() { _options.push_back(new Option()); }
	void addOption(const char *longName, char shortName, const char *help,
	               OptionRet ret, void (*printer)());
	void addOption(const char *longName, char shortName, const char *help,
	               OptionRet ret,
	               void (*printer)(Common::UString &), Common::UString &str);
	void addOption(const char *longName, char shortName, const char *help,
	               OptionRet ret, std::vector<Assigner *> assigners);
	void addOption(const char *longName, char shortName, const char *help,
	               OptionRet ret, Getter *getter);
	void addOption(const char *longName, char shortName, const char *help,
	               OptionRet ret, CallbackBase *callback);

	void addOption(const char *longName, const char *help, OptionRet ret,
	               void (*printer)()) {
		addOption(longName, 0, help, ret, printer);
	}
	void addOption(const char *longName, const char *help, OptionRet ret,
	               void (*printer)(Common::UString &), Common::UString &str) {
		addOption(longName, 0, help, ret, printer, str);
	}
	void addOption(const char *longName, const char *help, OptionRet ret,
	               std::vector<Assigner *> assigners) {
		addOption(longName, 0, help, ret, assigners);
	}
	void addOption(const char *longName, const char *help,
	               OptionRet ret, Getter *getter) {
		addOption(longName, 0, help, ret, getter);
	}
	void addOption(const char *longName, const char *help,
	               OptionRet ret, CallbackBase *callback) {
		addOption(longName, 0, help, ret, callback);
	}

	void addOption(char shortName, const char *help, OptionRet ret,
	               void (*printer)()) {
		addOption(0, shortName, help, ret, printer);
	}
	void addOption(char shortName, const char *help, OptionRet ret,
	               void (*printer)(Common::UString &), Common::UString &str) {
		addOption(0, shortName, help, ret, printer, str);
	}
	void addOption(char shortName, const char *help, OptionRet ret,
	               std::vector<Assigner *> assigners) {
		addOption(0, shortName, help, ret, assigners);
	}
	void addOption(char shortName, const char *help,
	               OptionRet ret, Getter *getter) {
		addOption(0, shortName, help, ret, getter);
	}
	void addOption(char shortName, const char *help,
	               OptionRet ret, CallbackBase *callback) {
		addOption(0, shortName, help, ret, callback);
	}

	bool process(const std::vector<Common::UString> &argv);

	void usage() { printUsage(_helpStr); }

private:
	static void printUsage(Common::UString &str) {
		std::printf("%s\n", str.c_str());
	}

	inline bool isShorOption(const Common::UString &arg) const {
		return arg.c_str()[0] == '-' && arg.c_str()[1] != '-';
	}

	inline bool isLongOption(const Common::UString &arg) const {
		return arg.c_str()[0] == '-' && arg.c_str()[1] == '-';
	}

	Common::UString _bottom;
	Common::UString _helpStr;
	std::vector<Option *> _options;
	std::vector<NoOption> _noOptions;
	int &_returnVal;
};

namespace {

inline std::vector<NoOption> makeEndArgs(NoOption *noOption) {
	std::vector<NoOption> ret;

	ret.push_back(*noOption);
	return ret;
}

inline std::vector<NoOption> makeEndArgs(NoOption *noOption1, NoOption *noOption2) {
	std::vector<NoOption> ret;

	ret.push_back(*noOption1);
	ret.push_back(*noOption2);
	return ret;
}

inline std::vector<NoOption> makeEndArgs(NoOption *noOption1, NoOption *noOption2,
                                         NoOption *noOption3) {
	std::vector<NoOption> ret;

	ret.push_back(*noOption1);
	ret.push_back(*noOption2);
	ret.push_back(*noOption3);
	return ret;
}

inline std::vector<NoOption> makeEndArgs(NoOption *noOption1, NoOption *noOption2,
                                         NoOption *noOption3, NoOption *noOption4) {
	std::vector<NoOption> ret;

	ret.push_back(*noOption1);
	ret.push_back(*noOption2);
	ret.push_back(*noOption3);
	ret.push_back(*noOption4);
	return ret;
}

inline std::vector<NoOption> makeEndArgs(NoOption *noOption1, NoOption *noOption2,
                                         NoOption *noOption3, NoOption *noOption4,
                                         NoOption *noOption5) {
	std::vector<NoOption> ret;

	ret.push_back(*noOption1);
	ret.push_back(*noOption2);
	ret.push_back(*noOption3);
	ret.push_back(*noOption4);
	ret.push_back(*noOption5);
	return ret;
}


inline std::vector<Assigner *> makeAssigners(Assigner *assigner) {
	std::vector<Assigner *> ret;

	ret.push_back(assigner);
	return ret;
}

inline std::vector<Assigner *> makeAssigners(Assigner *assigner1, Assigner *assigner2) {
	std::vector<Assigner *> ret;

	ret.push_back(assigner1);
	ret.push_back(assigner2);
	return ret;
}

inline std::vector<Assigner *> makeAssigners(Assigner *assigner1, Assigner *assigner2,
                                             Assigner *assigner3) {
	std::vector<Assigner *> ret;

	ret.push_back(assigner1);
	ret.push_back(assigner2);
	ret.push_back(assigner3);
	return ret;
}

} // End of anonymous namespace

} // End of namespace CLI

} // End of namespace Common

#endif // COMMON_CLI_H
