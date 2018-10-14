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

/**
 * @file
 * Unit tests for our KEY file writer
 */

#include "gtest/gtest.h"

#include "src/common/memreadstream.h"
#include "src/common/memwritestream.h"

#include "src/aurora/keywriter.h"
#include "src/aurora/keyfile.h"

GTEST_TEST(KEYWriter, writeKEY) {
	Common::MemoryWriteStreamDynamic keyWriter;
	Aurora::KEYWriter key;

	std::list<Common::UString> bifContents1, bifContents2;
	bifContents1.push_back("test1.txt");
	bifContents1.push_back("test2.txt");
	bifContents1.push_back("test3.txt");
	bifContents2.push_back("test4.txt");
	bifContents2.push_back("test5.txt");

	key.addBIF("test1.bif", bifContents1, 30);
	key.addBIF("test2.bif", bifContents2, 20);

	key.write(keyWriter);

	Common::MemoryReadStream keyReader(keyWriter.getData(), keyWriter.size(), true);
	Aurora::KEYFile keyFile(keyReader);

	Aurora::KEYFile::BIFList list = keyFile.getBIFs();
	EXPECT_EQ(list.size(), 2);
	EXPECT_STREQ(list.front().c_str(), "test1.bif");
	EXPECT_STREQ(list.back().c_str(), "test2.bif");

	Aurora::KEYFile::ResourceList resources = keyFile.getResources();
	EXPECT_EQ(resources.size(), 5);

	Aurora::KEYFile::ResourceList::iterator iter = resources.begin();
	EXPECT_STREQ((*iter).name.c_str(), "test1");
	EXPECT_EQ((*iter).type, Aurora::kFileTypeTXT);
	EXPECT_EQ((*iter).bifIndex, 0);
	EXPECT_EQ((*iter).resIndex, 0);

	std::advance(iter, 1);
	EXPECT_STREQ((*iter).name.c_str(), "test2");
	EXPECT_EQ((*iter).type, Aurora::kFileTypeTXT);
	EXPECT_EQ((*iter).bifIndex, 0);
	EXPECT_EQ((*iter).resIndex, 1);

	std::advance(iter, 1);
	EXPECT_STREQ((*iter).name.c_str(), "test3");
	EXPECT_EQ((*iter).type, Aurora::kFileTypeTXT);
	EXPECT_EQ((*iter).bifIndex, 0);
	EXPECT_EQ((*iter).resIndex, 2);

	std::advance(iter, 1);
	EXPECT_STREQ((*iter).name.c_str(), "test4");
	EXPECT_EQ((*iter).type, Aurora::kFileTypeTXT);
	EXPECT_EQ((*iter).bifIndex, 1);
	EXPECT_EQ((*iter).resIndex, 0);

	std::advance(iter, 1);
	EXPECT_STREQ((*iter).name.c_str(), "test5");
	EXPECT_EQ((*iter).type, Aurora::kFileTypeTXT);
	EXPECT_EQ((*iter).bifIndex, 1);
	EXPECT_EQ((*iter).resIndex, 1);
}
