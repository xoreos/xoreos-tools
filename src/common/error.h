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

/** @file common/error.h
 *  Basic exceptions to throw.
 */

#ifndef COMMON_ERROR_H
#define COMMON_ERROR_H

#include <stack>
#include <exception>

#include "common/ustring.h"

namespace Common {

/** Exeption that provides a stack of explanations. */
class StackException : public std::exception {
public:
	typedef std::stack<UString> Stack;

	StackException();
	StackException(const char *s, ...);
	StackException(const StackException &e);
	~StackException() throw();

	void add(const char *s, ...);

	const char *what() const throw();

	Stack &getStack();

private:
	Stack _stack;
};

typedef StackException Exception;

extern const Exception kOpenError;
extern const Exception kReadError;
extern const Exception kSeekError;
extern const Exception kWriteError;

void printException(Exception &e, const UString &prefix = "ERROR: ");

} // End of namespace Common

#endif // COMMON_ERROR_H
