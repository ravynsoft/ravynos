/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <libutil/FSUtil.h>

using libutil::FSUtil;

// TODO: remove these tests once FSUtil is unused
#if !_WIN32

TEST(FSUtil, GetDirectoryName)
{
    EXPECT_EQ("", FSUtil::GetDirectoryName(""));
    EXPECT_EQ("", FSUtil::GetDirectoryName("a"));
    EXPECT_EQ("/", FSUtil::GetDirectoryName("/a"));
    EXPECT_EQ("/a", FSUtil::GetDirectoryName("/a/b"));
    EXPECT_EQ("/a/b", FSUtil::GetDirectoryName("/a/b/c"));
}

TEST(FSUtil, GetBaseName)
{
    EXPECT_EQ("", FSUtil::GetBaseName(""));
    EXPECT_EQ("a", FSUtil::GetBaseName("a"));
    EXPECT_EQ("a.ext", FSUtil::GetBaseName("a.ext"));
    EXPECT_EQ("b", FSUtil::GetBaseName("/a/b"));
    EXPECT_EQ("c", FSUtil::GetBaseName("/a/b/c"));
    EXPECT_EQ("c.ext", FSUtil::GetBaseName("/a/b/c.ext"));
}

TEST(FSUtil, GetBaseNameWithoutExtension)
{
    EXPECT_EQ("", FSUtil::GetBaseNameWithoutExtension(""));
    EXPECT_EQ("a", FSUtil::GetBaseNameWithoutExtension("a"));
    EXPECT_EQ("b", FSUtil::GetBaseNameWithoutExtension("/a/b"));
    EXPECT_EQ("b", FSUtil::GetBaseNameWithoutExtension("/a/b.ext"));
    EXPECT_EQ("c.sub", FSUtil::GetBaseNameWithoutExtension("/a/b/c.sub.ext"));
}

TEST(FSUtil, GetRelativePath)
{
    EXPECT_EQ("", FSUtil::GetRelativePath("/", "/"));
    EXPECT_EQ("", FSUtil::GetRelativePath("/a", "/a"));
    EXPECT_EQ("", FSUtil::GetRelativePath("/a/b", "/a/b"));
    EXPECT_EQ("a", FSUtil::GetRelativePath("/a", "/"));
    EXPECT_EQ("a/b", FSUtil::GetRelativePath("/a/b", "/"));
    EXPECT_EQ("b", FSUtil::GetRelativePath("/a/b", "/a"));
    EXPECT_EQ("../a", FSUtil::GetRelativePath("/a", "/b"));
    EXPECT_EQ("../a/b", FSUtil::GetRelativePath("/a/b", "/b"));
    EXPECT_EQ("../b", FSUtil::GetRelativePath("/a/b", "/a/c"));
    EXPECT_EQ("../bbb", FSUtil::GetRelativePath("/aaa/bbb", "/aaa/ccc"));
}

TEST(FSUtil, GetFileExtension)
{
    EXPECT_EQ("", FSUtil::GetFileExtension(""));
    EXPECT_EQ("", FSUtil::GetFileExtension("a"));
    EXPECT_EQ("", FSUtil::GetFileExtension("/a/b"));
    EXPECT_EQ("ext", FSUtil::GetFileExtension("/a/b.ext"));
    EXPECT_EQ("ext", FSUtil::GetFileExtension("/a/b/c.sub.ext"));
}

TEST(FSUtil, IsAbsolutePath)
{
    EXPECT_FALSE(FSUtil::IsAbsolutePath(""));
    EXPECT_FALSE(FSUtil::IsAbsolutePath("a"));
    EXPECT_FALSE(FSUtil::IsAbsolutePath("a/b"));
    EXPECT_FALSE(FSUtil::IsAbsolutePath("./a/b"));
    EXPECT_TRUE(FSUtil::IsAbsolutePath("/"));
    EXPECT_TRUE(FSUtil::IsAbsolutePath("//"));
    EXPECT_TRUE(FSUtil::IsAbsolutePath("/a"));
    EXPECT_TRUE(FSUtil::IsAbsolutePath("/a/b/c.ext"));
}

TEST(FSUtil, ResolveRelativePath)
{
    EXPECT_EQ("/", FSUtil::ResolveRelativePath("", "/"));
    EXPECT_EQ("/a", FSUtil::ResolveRelativePath("", "/a"));
    EXPECT_EQ("/a/b", FSUtil::ResolveRelativePath("a/b", "/"));
    EXPECT_EQ("/c/a/b", FSUtil::ResolveRelativePath("a/b", "/c"));
    EXPECT_EQ("/c/a/b", FSUtil::ResolveRelativePath("a/b", "/c"));
    EXPECT_EQ("/a/b", FSUtil::ResolveRelativePath("/a/b", "/c"));
}

TEST(FSUtil, IsFileExtension)
{
    EXPECT_TRUE(FSUtil::IsFileExtension("", ""));
    EXPECT_TRUE(FSUtil::IsFileExtension("a", ""));
    EXPECT_TRUE(FSUtil::IsFileExtension("/a/b", ""));

    EXPECT_TRUE(FSUtil::IsFileExtension("a/b.ext", "ext"));
    EXPECT_TRUE(FSUtil::IsFileExtension("a/b.ext", "Ext", true));
    EXPECT_FALSE(FSUtil::IsFileExtension("a/b.ext", "Ext", false));

    EXPECT_TRUE(FSUtil::IsFileExtension("/a/b.ext", "ext"));
    EXPECT_TRUE(FSUtil::IsFileExtension("/a/b.ext", "Ext", true));
    EXPECT_FALSE(FSUtil::IsFileExtension("/a/b.ext", "Ext", false));

    EXPECT_FALSE(FSUtil::IsFileExtension("/a/b/c.sub.ext", ""));
    EXPECT_TRUE(FSUtil::IsFileExtension("/a/b/c.sub.ext", "ext"));
}

#endif
