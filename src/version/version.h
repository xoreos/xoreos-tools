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
 *  Basic xoreos-tools version information.
 */

#ifndef VERSION_VERSION_H
#define VERSION_VERSION_H

// "xoreos-tools"
extern const char *XOREOS_NAME;

// "0.0.1+2197.g19f9c1b"
extern const char *XOREOS_VERSION;

// "xoreos-tools 0.0.1+2197.g19f9c1b"
extern const char *XOREOS_NAMEVERSION;

// "xoreos-tools 0.0.1+2197.g19f9c1b [0.0.1+2197.g19f9c1b] (2013-07-28T13:32:04)"
extern const char *XOREOS_NAMEVERSIONFULL;

// "https://..."
extern const char *XOREOS_URL;

// Very shortened authors/copyright message
extern const char *XOREOS_AUTHORS;

/** Print the full version information to stdout. */
void printVersion();

#endif // VERSION_VERSION_H
