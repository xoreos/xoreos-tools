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
 *  Unicode string handling.
 */

#include <cstdarg>
#include <cstdio>
#include <cctype>
#include <cstring>

#include "src/common/ustring.h"
#include "src/common/error.h"
#include "src/common/util.h"

namespace Common {

UString::UString() : _size(0) {
}

UString::UString(const UString &str) {
	*this = str;
}

UString::UString(const std::string &str) {
	*this = str;
}

UString::UString(const char *str) {
	*this = str;
}

UString::UString(const char *str, size_t n) {
	*this = std::string(str, n);
}

UString::UString(uint32_t c, size_t n) : _size(0) {
	while (n-- > 0)
		*this += c;
}

UString::UString(iterator sBegin, iterator sEnd) : _size(0) {
	for (; (sBegin != sEnd) && *sBegin; ++sBegin)
		*this += *sBegin;
}

UString::~UString() {
}

UString &UString::operator=(const UString &str) {
	_string = str._string;
	_size   = str._size;

	return *this;
}

UString &UString::operator=(const std::string &str) {
	_string = str;

	recalculateSize();

	return *this;
}

UString &UString::operator=(const char *str) {
	*this = std::string(str);

	return *this;
}

bool UString::operator==(const UString &str) const {
	return _string == str._string;
}

bool UString::operator!=(const UString &str) const {
	return !(*this == str);
}

bool UString::operator<(const UString &str) const {
	return _string < str._string;
}

bool UString::operator>(const UString &str) const {
	return !(*this < str);
}

UString UString::operator+(const UString &str) const {
	UString tmp(*this);

	tmp += str;

	return tmp;
}

UString UString::operator+(const std::string &str) const {
	UString tmp(*this);

	tmp += str;

	return tmp;
}

UString UString::operator+(const char *str) const {
	UString tmp(*this);

	tmp += str;

	return tmp;
}

UString UString::operator+(uint32_t c) const {
	UString tmp(*this);

	tmp += c;

	return tmp;
}

UString &UString::operator+=(const UString &str) {
	_string += str._string;
	_size   += str._size;

	return *this;
}

UString &UString::operator+=(const std::string &str) {
	UString ustr(str);

	return *this += ustr;
}

UString &UString::operator+=(const char *str) {
	UString ustr(str);

	return *this += ustr;
}

UString &UString::operator+=(uint32_t c) {
	try {
		utf8::append(c, std::back_inserter(_string));
	} catch (const std::exception &se) {
		Exception e(se);
		throw e;
	}

	_size++;

	return *this;
}

bool UString::equals(const UString &str) const {
	return *this == str;
}

bool UString::equalsIgnoreCase(const UString &str) const {
	return String::equalsIgnoreCase(_string, str._string);
}

bool UString::less(const UString &str) const {
	return *this < str;
}

bool UString::lessIgnoreCase(const UString &str) const {
	return String::compareIgnoreCase(_string, str._string) < 0;
}

void UString::swap(UString &str) {
	_string.swap(str._string);

	std::swap(_size, str._size);
}

void UString::clear() {
	_string.clear();
	_size = 0;
}

size_t UString::size() const {
	return _size;
}

bool UString::empty() const {
	return _string.empty() || (_string[0] == '\0');
}

const char *UString::c_str() const {
	return _string.c_str();
}

const std::string &UString::toString() const {
	return _string;
}

UString::iterator UString::begin() const {
	return iterator(_string.begin(), _string.begin(), _string.end());
}

UString::iterator UString::end() const {
	return iterator(_string.end(), _string.begin(), _string.end());
}

UString::iterator UString::findFirst(uint32_t c) const {
	for (iterator it = begin(); it != end(); ++it)
		if (*it == c)
			return it;

	return end();
}

UString::iterator UString::findFirst(const UString &what) const {
	size_t index = _string.find(what._string);
	if (index != std::string::npos) {
		std::string::const_iterator it = _string.begin();
		std::advance(it, index);
		return iterator(it, _string.begin(), _string.end());
	}
	return end();
}

UString::iterator UString::findLast(uint32_t c) const {
	if (empty())
		return end();

	iterator it = end();
	do {
		--it;

		if (*it == c)
			return it;

	} while (it != begin());

	return end();
}

bool UString::beginsWith(const UString &with) const {
	if (with.empty())
		return true;

	if (empty())
		return false;

	UString::iterator myIt   = begin();
	UString::iterator withIt = with.begin();

	while ((myIt != end()) && (withIt != with.end()))
		if (*myIt++ != *withIt++)
			return false;

	if ((myIt == end()) && (withIt != with.end()))
		return false;

	return true;
}

bool UString::endsWith(const UString &with) const {
	if (with.empty())
		return true;

	if (empty())
		return false;

	UString::iterator myIt   = --end();
	UString::iterator withIt = --with.end();

	while ((myIt != begin()) && (withIt != with.begin()))
		if (*myIt-- != *withIt--)
			return false;

	if (withIt == with.begin())
		return (*myIt == *withIt);

	return false;
}

bool UString::contains(const UString &what) const {
	return _string.find(what._string) != std::string::npos;
}

bool UString::contains(uint32_t c) const {
	return findFirst(c) != end();
}

uint32_t UString::at(size_t pos) const {
	if (pos < _size)
		return _string.at(pos);
	else
		return 0;
}

void UString::truncate(const iterator &it) {
	UString temp;

	for (iterator i = begin(); i != it; ++i)
		temp += *i;

	swap(temp);
}

void UString::truncate(size_t n) {
	if (n >= _size)
		return;

	UString temp;

	for (iterator it = begin(); n > 0; ++it, n--)
		temp += *it;

	swap(temp);
}

void UString::trim() {
	if (_string.empty())
		// Nothing to do
		return;

	// Find the last space, from the end
	iterator itEnd = --end();
	for (; itEnd != begin(); --itEnd) {
		uint32_t c = *itEnd;
		if (!String::isSpace(c) && (c != '\0')) {
			++itEnd;
			break;
		}
	}

	if (itEnd == begin()) {
		uint32_t c = *itEnd;
		if (!String::isSpace(c) && (c != '\0'))
			++itEnd;
	}

	// Find the first non-space
	iterator itStart = begin();
	for (; itStart != itEnd; ++itStart)
		if (!String::isSpace(*itStart))
			break;

	_string = std::string(itStart.base(), itEnd.base());
	recalculateSize();
}

void UString::trimLeft() {
	if (_string.empty())
		// Nothing to do
		return;

	// Find the first non-space
	iterator itStart = begin();
	for (; itStart != end(); ++itStart)
		if (!String::isSpace(*itStart))
			break;

	_string = std::string(itStart.base(), end().base());
	recalculateSize();
}

void UString::trimRight() {
	if (_string.empty())
		// Nothing to do
		return;

	// Find the last space, from the end
	iterator itEnd = --end();
	for (; itEnd != begin(); --itEnd) {
		uint32_t c = *itEnd;
		if (!String::isSpace(c) && (c != '\0')) {
			++itEnd;
			break;
		}
	}

	if (itEnd == begin()) {
		uint32_t c = *itEnd;
		if (!String::isSpace(c) && (c != '\0'))
			++itEnd;
	}

	_string = std::string(begin().base(), itEnd.base());
	recalculateSize();
}

void UString::replaceAll(uint32_t what, uint32_t with) {
	try {

		// The new string with characters replaced
		std::string newString;
		newString.reserve(_string.size());

		// Run through the whole string
		std::string::iterator it = _string.begin();
		while (it != _string.end()) {
			std::string::iterator prev = it;

			// Get the codepoint
			uint32_t c = utf8::next(it, _string.end());

			if (c != what) {
				// It's not what we're looking for, copy it
				for (; prev != it; ++prev)
					newString.push_back(*prev);
			} else
				// It's what we're looking for, insert the replacement instead
				utf8::append(with, std::back_inserter(newString));

		}

		// And set the new string's contents
		_string.swap(newString);

	} catch (const std::exception &se) {
		Exception e(se);
		throw e;
	}
}

void UString::makeLower() {
	*this = toLower();
}

void UString::makeUpper() {
	*this = toUpper();
}

UString UString::toLower() const {
	UString str;

	str._string.reserve(_string.size());
	for (iterator it = begin(); it != end(); ++it)
		str += String::toLower(*it);

	return str;
}

UString UString::toUpper() const {
	UString str;

	str._string.reserve(_string.size());
	for (iterator it = begin(); it != end(); ++it)
		str += String::toUpper(*it);

	return str;
}

UString::iterator UString::getPosition(size_t n) const {
	iterator it = begin();
	for (size_t i = 0; (i < n) && (it != end()); i++, ++it);
	return it;
}

size_t UString::getPosition(iterator it) const {
	size_t n = 0;
	for (iterator i = begin(); i != it; ++i, n++);
	return n;
}

void UString::insert(iterator pos, uint32_t c) {
	if (pos == end()) {
		*this += c;
		return;
	}

	UString temp;

	iterator it;
	for (it = begin(); it != pos; ++it)
		temp += *it;

	temp += c;

	for ( ; it != end(); ++it)
		temp += *it;

	swap(temp);
}

void UString::insert(UString::iterator pos, const UString &str) {
	if (pos == end()) {
		*this += str;
		return;
	}

	UString temp;

	iterator it;
	for (it = begin(); it != pos; ++it)
		temp += *it;

	temp += str;

	for ( ; it != end(); ++it)
		temp += *it;

	swap(temp);
}

void UString::replace(iterator pos, uint32_t c) {
	if (pos == end()) {
		*this += c;
		return;
	}

	UString temp;

	iterator it;
	for (it = begin(); it != pos; ++it)
		temp += *it;

	temp += c;

	for (++it; it != end(); ++it)
		temp += *it;

	swap(temp);
}

void UString::replace(UString::iterator pos, const UString &str) {
	if (pos == end()) {
		*this += str;
		return;
	}

	UString temp;

	iterator it;
	for (it = begin(); it != pos; ++it)
		temp += *it;

	for (iterator it2 = str.begin(); it2 != str.end(); ++it2) {
		temp += *it2;

		if (it != end())
			++it;
	}

	for ( ; it != end(); ++it)
		temp += *it;

	swap(temp);
}

void UString::erase(iterator from, iterator to) {
	if (from == end())
		return;

	UString temp;

	iterator it = begin();
	for ( ; it != from; ++it)
		temp += *it;

	for ( ; it != to; ++it);

	for ( ; it != end(); ++it)
		temp += *it;

	swap(temp);
}

void UString::erase(iterator pos) {
	iterator to = pos;
	erase(pos, ++to);
}

void UString::split(iterator splitPoint, UString &left, UString &right, bool remove) const {
	left.clear();
	right.clear();

	if (splitPoint == begin()) {
		right = *this;
		return;
	}
	if (splitPoint == end()) {
		left = *this;
		return;
	}

	iterator it = begin();
	for ( ; it != splitPoint; ++it)
		left += *it;

	if (remove)
		++it;

	for ( ; it != end(); ++it)
		right += *it;
}

void UString::splitTextTokens(const UString &text, std::vector<UString> &tokens) {
	UString collect;

	int state = 0;
	for (iterator it = text.begin(); it != text.end(); ++it) {
		uint32_t c = *it;

		if (state == 0) {
			// Collecting non-tokens

			if (c == '<') {
				tokens.push_back(collect);

				collect.clear();
				collect += c;

				state = 1;
			} else
				collect += c;

		} else if (state == 1) {
			// Collecting tokens

			if        (c == '<') {
				// Start of a token within a token
				// Add what we've collected to the last non-token

				tokens.back() += collect;

				collect.clear();
				collect += c;

			} else if (c == '>') {
				// End of the token

				collect += c;
				tokens.push_back(collect);

				collect.clear();
				state = 0;

			} else {
				// Still within a token

				collect += c;
			}

		}

	}

	if (collect.empty())
		return;

	// What's now collected is no full token
	if (state == 0)
		tokens.push_back(collect);
	else if (state == 1)
		tokens.back() += collect;
}

UString UString::substr(iterator from, iterator to) const {
	UString sub;

	iterator it = begin();
	for ( ; it != from; ++it);

	for ( ; it != to; ++it)
		sub += *it;

	return sub;
}

size_t UString::split(const UString &text, uint32_t delim, std::vector<UString> &texts) {
	size_t length = 0;

	UString t = text;

	iterator point;
	while ((point = t.findFirst(delim)) != t.end()) {
		UString left, right;

		t.split(point, left, right, true);

		if (!left.empty()) {
			length = MAX(length, left.size());
			texts.push_back(left);
		}

		t = right;
	}

	if (!t.empty()) {
		length = MAX(length, t.size());
		texts.push_back(t);
	}

	return length;
}

void UString::recalculateSize() {
	try {
		// Calculate the "distance" in characters from the beginning and end
		_size = utf8::distance(_string.begin(), _string.end());
	} catch (const std::exception &se) {
		Exception e(se);
		throw e;
	}
}

} // End of namespace Common
