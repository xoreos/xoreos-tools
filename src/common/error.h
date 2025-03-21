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
 *  Basic exceptions to throw.
 */

#ifndef COMMON_ERROR_H
#define COMMON_ERROR_H

#include <stack>
#include <exception>

#include "src/common/ustring.h"

namespace Common {

/** Exception that provides a stack of explanations. */
class StackException : public std::exception {
public:
	typedef std::stack<UString> Stack;

	StackException() = default;
	StackException(const char *s, ...) GCC_PRINTF(2, 3);
	StackException(const StackException &e) = default;
	StackException(const std::exception &e);
	~StackException() = default;

	StackException &operator=(const StackException &) = default;

	void add(const char *s, ...) GCC_PRINTF(2, 3);
	void add(const std::exception &e);

	const char *what() const throw();

	bool empty() const;

	Stack &getStack();

private:
	Stack _stack;
};

typedef StackException Exception;

extern const Exception kOpenError;  ///< Exception when a file couldn't be opened.
extern const Exception kReadError;  ///< Exception when reading from a stream failed.
extern const Exception kSeekError;  ///< Exception when seeking a stream failed.
extern const Exception kWriteError; ///< Exception when writing to a stream failed.

/** Print a whole exception stack to stderr and the log. */
void printException(Exception &e, const UString &prefix = "ERROR: ");

/** Default exception dispatcher that prints the exception and errors out. */
void exceptionDispatcherError(const UString &reason = UString());

/** Exception dispatcher that prints the exception as a warning and ignores it otherwise. */
void exceptionDispatcherWarnAndIgnore(const UString &reason = UString());

} // End of namespace Common

#endif // COMMON_ERROR_H
