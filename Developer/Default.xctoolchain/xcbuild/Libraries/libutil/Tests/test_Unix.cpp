/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <libutil/Relative.h>
#include <libutil/Absolute.h>
#include <libutil/Unix.h>

namespace Path = libutil::Path;

TEST(Path, Absolute)
{
    EXPECT_EQ(ext::nullopt, Path::BaseRelative<Path::Unix>("").absolute());
    EXPECT_EQ(ext::nullopt, Path::BaseRelative<Path::Unix>("a").absolute());
    EXPECT_EQ(ext::nullopt, Path::BaseRelative<Path::Unix>("a/b").absolute());
    EXPECT_EQ(ext::nullopt, Path::BaseRelative<Path::Unix>("./a/b").absolute());
    EXPECT_NE(ext::nullopt, Path::BaseRelative<Path::Unix>("/").absolute());
    EXPECT_NE(ext::nullopt, Path::BaseRelative<Path::Unix>("//").absolute());
    EXPECT_NE(ext::nullopt, Path::BaseRelative<Path::Unix>("/a").absolute());
    EXPECT_NE(ext::nullopt, Path::BaseRelative<Path::Unix>("/a/b/c.ext").absolute());
}

TEST(Path, Normalized)
{
    EXPECT_EQ("", Path::BaseRelative<Path::Unix>("").normalized());

    EXPECT_EQ("/", Path::BaseRelative<Path::Unix>("//").normalized());
    EXPECT_EQ("/", Path::BaseRelative<Path::Unix>("/.").normalized());
    EXPECT_EQ("/", Path::BaseRelative<Path::Unix>("/./").normalized());
    EXPECT_EQ("/", Path::BaseRelative<Path::Unix>("/..").normalized());
    EXPECT_EQ("/", Path::BaseRelative<Path::Unix>("/../").normalized());
    EXPECT_EQ("/", Path::BaseRelative<Path::Unix>("/../..").normalized());
    EXPECT_EQ("/a", Path::BaseRelative<Path::Unix>("/a").normalized());
    EXPECT_EQ("/a", Path::BaseRelative<Path::Unix>("/a/").normalized());
    EXPECT_EQ("/a", Path::BaseRelative<Path::Unix>("//a").normalized());
    EXPECT_EQ("/a/b", Path::BaseRelative<Path::Unix>("/a/b").normalized());
    EXPECT_EQ("/a/b", Path::BaseRelative<Path::Unix>("/a/b/c/..").normalized());
    EXPECT_EQ("/a/c", Path::BaseRelative<Path::Unix>("/a/b/../c").normalized());
    EXPECT_EQ("/a/b/c", Path::BaseRelative<Path::Unix>("/a/b/c").normalized());
    EXPECT_EQ("/a/b/c", Path::BaseRelative<Path::Unix>("/a/b/./c").normalized());

    EXPECT_EQ("", Path::BaseRelative<Path::Unix>(".").normalized());
    EXPECT_EQ("", Path::BaseRelative<Path::Unix>("./").normalized());
    EXPECT_EQ("", Path::BaseRelative<Path::Unix>("a/..").normalized());
    EXPECT_EQ("a", Path::BaseRelative<Path::Unix>("./a").normalized());
    EXPECT_EQ("a", Path::BaseRelative<Path::Unix>("a").normalized());
    EXPECT_EQ("a", Path::BaseRelative<Path::Unix>("a/.").normalized());
    EXPECT_EQ("b", Path::BaseRelative<Path::Unix>("a/../b").normalized());
    EXPECT_EQ("a/c", Path::BaseRelative<Path::Unix>("a/b/../c").normalized());
    EXPECT_EQ("../a/c", Path::BaseRelative<Path::Unix>(".///../a/b/././/../c").normalized());
}

TEST(Path, From)
{
    EXPECT_EQ("", Path::BaseAbsolute<Path::Unix>::Create("/")->from(*Path::BaseAbsolute<Path::Unix>::Create("/"))->raw());
    EXPECT_EQ("", Path::BaseAbsolute<Path::Unix>::Create("/a")->from(*Path::BaseAbsolute<Path::Unix>::Create("/a"))->raw());
    EXPECT_EQ("", Path::BaseAbsolute<Path::Unix>::Create("/a/b")->from(*Path::BaseAbsolute<Path::Unix>::Create("/a/b"))->raw());
    EXPECT_EQ("a", Path::BaseAbsolute<Path::Unix>::Create("/a")->from(*Path::BaseAbsolute<Path::Unix>::Create("/"))->raw());
    EXPECT_EQ("a/b", Path::BaseAbsolute<Path::Unix>::Create("/a/b")->from(*Path::BaseAbsolute<Path::Unix>::Create("/"))->raw());
    EXPECT_EQ("b", Path::BaseAbsolute<Path::Unix>::Create("/a/b")->from(*Path::BaseAbsolute<Path::Unix>::Create("/a"))->raw());
    EXPECT_EQ("../a", Path::BaseAbsolute<Path::Unix>::Create("/a")->from(*Path::BaseAbsolute<Path::Unix>::Create("/b"))->raw());
    EXPECT_EQ("../a/b", Path::BaseAbsolute<Path::Unix>::Create("/a/b")->from(*Path::BaseAbsolute<Path::Unix>::Create("/b"))->raw());
    EXPECT_EQ("../b", Path::BaseAbsolute<Path::Unix>::Create("/a/b")->from(*Path::BaseAbsolute<Path::Unix>::Create("/a/c"))->raw());
    EXPECT_EQ("../bbb", Path::BaseAbsolute<Path::Unix>::Create("/aaa/bbb")->from(*Path::BaseAbsolute<Path::Unix>::Create("/aaa/ccc"))->raw());
}

TEST(Path, Parent)
{
    EXPECT_EQ("", Path::BaseRelative<Path::Unix>("").parent().raw());
    EXPECT_EQ("", Path::BaseRelative<Path::Unix>("a").parent().raw());
    EXPECT_EQ("/", Path::BaseRelative<Path::Unix>("/a").parent().raw());
    EXPECT_EQ("/a", Path::BaseRelative<Path::Unix>("/a/b").parent().raw());
    EXPECT_EQ("/a", Path::BaseRelative<Path::Unix>("/a/b/").parent().raw());
    EXPECT_EQ("/a/b", Path::BaseRelative<Path::Unix>("/a/b/c").parent().raw());
}

TEST(Path, Base)
{
    EXPECT_EQ("", Path::BaseRelative<Path::Unix>("").base());
    EXPECT_EQ("a", Path::BaseRelative<Path::Unix>("a").base());
    EXPECT_EQ("a.ext", Path::BaseRelative<Path::Unix>("a.ext").base());
    EXPECT_EQ("b", Path::BaseRelative<Path::Unix>("/a/b").base());
    EXPECT_EQ("c", Path::BaseRelative<Path::Unix>("/a/b/c").base());
    EXPECT_EQ("c.ext", Path::BaseRelative<Path::Unix>("/a/b/c.ext").base());

    EXPECT_EQ("", Path::BaseRelative<Path::Unix>("").base(false));
    EXPECT_EQ("a", Path::BaseRelative<Path::Unix>("a").base(false));
    EXPECT_EQ("b", Path::BaseRelative<Path::Unix>("/a/b").base(false));
    EXPECT_EQ("b", Path::BaseRelative<Path::Unix>("/a/b.ext").base(false));
    EXPECT_EQ("c.sub", Path::BaseRelative<Path::Unix>("/a/b/c.sub.ext").base(false));
}

TEST(Path, Extension)
{
    EXPECT_EQ("", Path::BaseRelative<Path::Unix>("").extension());
    EXPECT_EQ("", Path::BaseRelative<Path::Unix>("a").extension());
    EXPECT_EQ("", Path::BaseRelative<Path::Unix>("/a/b").extension());
    EXPECT_EQ("ext", Path::BaseRelative<Path::Unix>("/a/b.ext").extension());
    EXPECT_EQ("ext", Path::BaseRelative<Path::Unix>("/a/b/c.sub.ext").extension());
}

TEST(Path, IsExtension)
{
    EXPECT_TRUE(Path::BaseRelative<Path::Unix>("").extension(""));
    EXPECT_TRUE(Path::BaseRelative<Path::Unix>("a").extension(""));
    EXPECT_TRUE(Path::BaseRelative<Path::Unix>("/a/b").extension(""));

    EXPECT_TRUE(Path::BaseRelative<Path::Unix>("a/b.ext").extension("ext"));
    EXPECT_TRUE(Path::BaseRelative<Path::Unix>("a/b.ext").extension("Ext", true));
    EXPECT_FALSE(Path::BaseRelative<Path::Unix>("a/b.ext").extension("Ext", false));

    EXPECT_TRUE(Path::BaseRelative<Path::Unix>("/a/b.ext").extension("ext"));
    EXPECT_TRUE(Path::BaseRelative<Path::Unix>("/a/b.ext").extension("Ext", true));
    EXPECT_FALSE(Path::BaseRelative<Path::Unix>("/a/b.ext").extension("Ext", false));

    EXPECT_FALSE(Path::BaseRelative<Path::Unix>("/a/b/c.sub.ext").extension(""));
    EXPECT_TRUE(Path::BaseRelative<Path::Unix>("/a/b/c.sub.ext").extension("ext"));
}

TEST(Path, Resolved)
{
    EXPECT_EQ("/", Path::BaseRelative<Path::Unix>("").resolved(*Path::BaseAbsolute<Path::Unix>::Create("/")).raw());
    EXPECT_EQ("/a", Path::BaseRelative<Path::Unix>("").resolved(*Path::BaseAbsolute<Path::Unix>::Create("/a")).raw());
    EXPECT_EQ("/a/b", Path::BaseRelative<Path::Unix>("a/b").resolved(*Path::BaseAbsolute<Path::Unix>::Create("/")).raw());
    EXPECT_EQ("/c/a/b", Path::BaseRelative<Path::Unix>("a/b").resolved(*Path::BaseAbsolute<Path::Unix>::Create("/c")).raw());
    EXPECT_EQ("/a/b", Path::BaseRelative<Path::Unix>("/a/b").resolved(*Path::BaseAbsolute<Path::Unix>::Create("/c")).raw());
    EXPECT_EQ("/b/../a", Path::BaseRelative<Path::Unix>("../a").resolved(*Path::BaseAbsolute<Path::Unix>::Create("/b")).raw());
}
