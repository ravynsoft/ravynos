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
#include <libutil/Windows.h>

namespace Path = libutil::Path;

TEST(Path, Absolute)
{
    EXPECT_EQ(ext::nullopt, Path::BaseRelative<Path::Windows>("").absolute());
    EXPECT_EQ(ext::nullopt, Path::BaseRelative<Path::Windows>("a").absolute());
    EXPECT_EQ(ext::nullopt, Path::BaseRelative<Path::Windows>("a/b").absolute());
    EXPECT_EQ(ext::nullopt, Path::BaseRelative<Path::Windows>(".\\a\\b").absolute());
    EXPECT_EQ(ext::nullopt, Path::BaseRelative<Path::Windows>("/").absolute());
    EXPECT_EQ(ext::nullopt, Path::BaseRelative<Path::Windows>("/a").absolute());
    EXPECT_EQ(ext::nullopt, Path::BaseRelative<Path::Windows>("\\").absolute());
    EXPECT_EQ(ext::nullopt, Path::BaseRelative<Path::Windows>("\\a").absolute());
    EXPECT_EQ(ext::nullopt, Path::BaseRelative<Path::Windows>("\\a/b/c.ext").absolute());
    EXPECT_EQ(ext::nullopt, Path::BaseRelative<Path::Windows>("C:").absolute());
    EXPECT_EQ(ext::nullopt, Path::BaseRelative<Path::Windows>("//?/").absolute());
    EXPECT_EQ(ext::nullopt, Path::BaseRelative<Path::Windows>("/" "??" "/").absolute());

    EXPECT_NE(ext::nullopt, Path::BaseRelative<Path::Windows>("C:\\").absolute());
    EXPECT_NE(ext::nullopt, Path::BaseRelative<Path::Windows>("C:/").absolute());
    EXPECT_NE(ext::nullopt, Path::BaseRelative<Path::Windows>("//UNC").absolute());
    EXPECT_NE(ext::nullopt, Path::BaseRelative<Path::Windows>("//./").absolute());
    EXPECT_NE(ext::nullopt, Path::BaseRelative<Path::Windows>("\\\\.\\").absolute());
    EXPECT_NE(ext::nullopt, Path::BaseRelative<Path::Windows>("\\\\?\\").absolute());
    EXPECT_NE(ext::nullopt, Path::BaseRelative<Path::Windows>("\\??\\").absolute());
}

TEST(Path, Normalized)
{
    /* Drive absolute path. */
    EXPECT_EQ("C:\\", Path::BaseRelative<Path::Windows>("C:\\").normalized());
    EXPECT_EQ("C:\\", Path::BaseRelative<Path::Windows>("C:/").normalized());
    EXPECT_EQ("C:\\", Path::BaseRelative<Path::Windows>("C:\\.\\").normalized());
    EXPECT_EQ("C:\\a", Path::BaseRelative<Path::Windows>("C:\\a").normalized());
    EXPECT_EQ("C:\\a", Path::BaseRelative<Path::Windows>("C:\\a\\").normalized());
    EXPECT_EQ("C:\\", Path::BaseRelative<Path::Windows>("C:\\..\\").normalized());
    EXPECT_EQ("C:\\", Path::BaseRelative<Path::Windows>("C:\\..\\..").normalized());

    /* Drive relative path. */
    EXPECT_EQ("C:a", Path::BaseRelative<Path::Windows>("C:a").normalized());
    EXPECT_EQ("C:a\\b", Path::BaseRelative<Path::Windows>("C:a\\b").normalized());
    EXPECT_EQ("C:..", Path::BaseRelative<Path::Windows>("C:..").normalized());

    /* Relative path. */
    EXPECT_EQ("", Path::BaseRelative<Path::Windows>("").normalized());
    EXPECT_EQ("", Path::BaseRelative<Path::Windows>(".").normalized());
    EXPECT_EQ("", Path::BaseRelative<Path::Windows>(".\\").normalized());
    EXPECT_EQ("..", Path::BaseRelative<Path::Windows>("..\\").normalized());
    EXPECT_EQ("..\\b", Path::BaseRelative<Path::Windows>("../a/../b/.").normalized());
    EXPECT_EQ("a", Path::BaseRelative<Path::Windows>("a").normalized());
    EXPECT_EQ("a\\b", Path::BaseRelative<Path::Windows>("a\\b").normalized());
    EXPECT_EQ("a\\b", Path::BaseRelative<Path::Windows>("a/b").normalized());
    EXPECT_EQ("a\\c", Path::BaseRelative<Path::Windows>("a/b\\..\\c").normalized());

    /* Rooted path. */
    EXPECT_EQ("\\a", Path::BaseRelative<Path::Windows>("\\a").normalized());
    EXPECT_EQ("\\a", Path::BaseRelative<Path::Windows>("/a").normalized());
    EXPECT_EQ("\\a\\b", Path::BaseRelative<Path::Windows>("/a/b").normalized());
    EXPECT_EQ("\\a\\c", Path::BaseRelative<Path::Windows>("\\a/b\\../c").normalized());

    /* NT path. */
    EXPECT_EQ("\\\\?\\", Path::BaseRelative<Path::Windows>("\\\\?\\").normalized());
    EXPECT_EQ("\\\\.\\", Path::BaseRelative<Path::Windows>("//./").normalized());
    EXPECT_EQ("\\\\.\\C:", Path::BaseRelative<Path::Windows>("\\\\.\\C:\\").normalized());
    EXPECT_EQ("\\\\.\\C:\\a\\b", Path::BaseRelative<Path::Windows>("//./C:/a/b").normalized());
    EXPECT_EQ("\\\\?\\", Path::BaseRelative<Path::Windows>("\\\\?\\C:\\..").normalized());
    EXPECT_EQ("\\\\?\\D:", Path::BaseRelative<Path::Windows>("\\\\?\\C:\\..\\D:").normalized());
}

TEST(Path, From)
{
    EXPECT_EQ("", Path::BaseAbsolute<Path::Windows>::Create("C:\\")->from(*Path::BaseAbsolute<Path::Windows>::Create("C:\\"))->raw());
    EXPECT_EQ("", Path::BaseAbsolute<Path::Windows>::Create("C:\\a")->from(*Path::BaseAbsolute<Path::Windows>::Create("C:\\a"))->raw());
    EXPECT_EQ("", Path::BaseAbsolute<Path::Windows>::Create("C:\\a\\b")->from(*Path::BaseAbsolute<Path::Windows>::Create("C:\\a\\b"))->raw());
    EXPECT_EQ("a\\b", Path::BaseAbsolute<Path::Windows>::Create("C:\\a\\b")->from(*Path::BaseAbsolute<Path::Windows>::Create("C:\\"))->raw());
    EXPECT_EQ("b", Path::BaseAbsolute<Path::Windows>::Create("C:\\a\\b")->from(*Path::BaseAbsolute<Path::Windows>::Create("C:\\a"))->raw());
    EXPECT_EQ("..\\a", Path::BaseAbsolute<Path::Windows>::Create("C:\\a")->from(*Path::BaseAbsolute<Path::Windows>::Create("C:\\b"))->raw());
    EXPECT_EQ("..\\a\\b", Path::BaseAbsolute<Path::Windows>::Create("C:\\a\\b")->from(*Path::BaseAbsolute<Path::Windows>::Create("C:\\b"))->raw());
    EXPECT_EQ("..\\b", Path::BaseAbsolute<Path::Windows>::Create("C:\\a\\b")->from(*Path::BaseAbsolute<Path::Windows>::Create("C:\\a\\c"))->raw());
    EXPECT_EQ("..\\bbb", Path::BaseAbsolute<Path::Windows>::Create("C:\\aaa\\bbb")->from(*Path::BaseAbsolute<Path::Windows>::Create("C:\\aaa\\ccc"))->raw());

    /* Different roots are not supported. */
    EXPECT_EQ(ext::nullopt, Path::BaseAbsolute<Path::Windows>::Create("C:\\a")->from(*Path::BaseAbsolute<Path::Windows>::Create("D:\\a")));

    /* But switching drives with NT paths is fine. */
    EXPECT_EQ("..\\C:\\a", Path::BaseAbsolute<Path::Windows>::Create("\\\\?\\C:\\a")->from(*Path::BaseAbsolute<Path::Windows>::Create("\\\\?\\D:\\"))->raw());
}

TEST(Path, Parent)
{
    EXPECT_EQ("", Path::BaseRelative<Path::Windows>("").parent().raw());
    EXPECT_EQ("", Path::BaseRelative<Path::Windows>("a").parent().raw());

    EXPECT_EQ("C:", Path::BaseRelative<Path::Windows>("C:").parent().raw());
    EXPECT_EQ("C:", Path::BaseRelative<Path::Windows>("C:a").parent().raw());

    EXPECT_EQ("C:\\", Path::BaseRelative<Path::Windows>("C:\\").parent().raw());
    EXPECT_EQ("C:\\", Path::BaseRelative<Path::Windows>("C:\\a").parent().raw());
    EXPECT_EQ("C:\\a", Path::BaseRelative<Path::Windows>("C:\\a\\b\\").parent().raw());

    EXPECT_EQ("\\", Path::BaseRelative<Path::Windows>("\\").parent().raw());
    EXPECT_EQ("\\", Path::BaseRelative<Path::Windows>("\\a").parent().raw());
    EXPECT_EQ("\\a", Path::BaseRelative<Path::Windows>("\\a\\b").parent().raw());
    EXPECT_EQ("/a", Path::BaseRelative<Path::Windows>("/a/b").parent().raw());
    EXPECT_EQ("/a/b", Path::BaseRelative<Path::Windows>("/a/b/c").parent().raw());

    EXPECT_EQ("\\\\?\\", Path::BaseRelative<Path::Windows>("\\\\?\\").parent().raw());
    EXPECT_EQ("\\\\?\\", Path::BaseRelative<Path::Windows>("\\\\?\\C:\\").parent().raw());
    EXPECT_EQ("\\\\a\\b", Path::BaseRelative<Path::Windows>("\\\\a\\b\\c").parent().raw());
}

TEST(Path, Base)
{
    EXPECT_EQ("", Path::BaseRelative<Path::Windows>("").base());
    EXPECT_EQ("a", Path::BaseRelative<Path::Windows>("a").base());
    EXPECT_EQ("a.ext", Path::BaseRelative<Path::Windows>("a.ext").base());
    EXPECT_EQ("b", Path::BaseRelative<Path::Windows>("/a/b").base());
    EXPECT_EQ("c", Path::BaseRelative<Path::Windows>("/a/b/c").base());
    EXPECT_EQ("c.ext", Path::BaseRelative<Path::Windows>("/a/b/c.ext").base());

    EXPECT_EQ("", Path::BaseRelative<Path::Windows>("").base(false));
    EXPECT_EQ("a", Path::BaseRelative<Path::Windows>("a").base(false));
    EXPECT_EQ("b", Path::BaseRelative<Path::Windows>("/a/b").base(false));
    EXPECT_EQ("b", Path::BaseRelative<Path::Windows>("/a/b.ext").base(false));
    EXPECT_EQ("c.sub", Path::BaseRelative<Path::Windows>("/a/b/c.sub.ext").base(false));
}

TEST(Path, Extension)
{
    EXPECT_EQ("", Path::BaseRelative<Path::Windows>("").extension());
    EXPECT_EQ("", Path::BaseRelative<Path::Windows>("a").extension());
    EXPECT_EQ("ext", Path::BaseRelative<Path::Windows>("a.ext").extension());
    EXPECT_EQ("ext", Path::BaseRelative<Path::Windows>("a.sub.ext").extension());
    EXPECT_EQ("", Path::BaseRelative<Path::Windows>("C:\\a\\b").extension());
    EXPECT_EQ("ext", Path::BaseRelative<Path::Windows>("C:\\a\\b.ext").extension());
    EXPECT_EQ("ext", Path::BaseRelative<Path::Windows>("\\\\?\\a\\b\\c.ext").extension());
}

TEST(Path, IsExtension)
{
    EXPECT_TRUE(Path::BaseRelative<Path::Windows>("").extension(""));
    EXPECT_TRUE(Path::BaseRelative<Path::Windows>("a").extension(""));
    EXPECT_TRUE(Path::BaseRelative<Path::Windows>("C:\\a\\b").extension(""));

    EXPECT_TRUE(Path::BaseRelative<Path::Windows>("a.ext").extension("ext"));
    EXPECT_TRUE(Path::BaseRelative<Path::Windows>("a.ext").extension("Ext", true));
    EXPECT_FALSE(Path::BaseRelative<Path::Windows>("a.ext").extension("Ext", false));

    EXPECT_TRUE(Path::BaseRelative<Path::Windows>("a.ext").extension("ext"));
    EXPECT_TRUE(Path::BaseRelative<Path::Windows>("a.ext").extension("Ext", true));
    EXPECT_FALSE(Path::BaseRelative<Path::Windows>("a.ext").extension("Ext", false));

    EXPECT_FALSE(Path::BaseRelative<Path::Windows>("c.sub.ext").extension(""));
    EXPECT_FALSE(Path::BaseRelative<Path::Windows>("c.sub.ext").extension("sub"));
    EXPECT_TRUE(Path::BaseRelative<Path::Windows>("c.sub.ext").extension("ext"));
}

TEST(Path, Resolved)
{
    /* Absolute path. */
    EXPECT_EQ("C:\\", Path::BaseRelative<Path::Windows>("C:\\").resolved(*Path::BaseAbsolute<Path::Windows>::Create("D:\\")).raw());
    EXPECT_EQ("//./C:/", Path::BaseRelative<Path::Windows>("//./C:/").resolved(*Path::BaseAbsolute<Path::Windows>::Create("D:\\")).raw());
    EXPECT_EQ("\\\\?\\C:\\", Path::BaseRelative<Path::Windows>("\\\\?\\C:\\").resolved(*Path::BaseAbsolute<Path::Windows>::Create("D:\\")).raw());

    /* Relative path. */
    EXPECT_EQ("C:\\", Path::BaseRelative<Path::Windows>("").resolved(*Path::BaseAbsolute<Path::Windows>::Create("C:\\")).raw());
    EXPECT_EQ("C:\\..", Path::BaseRelative<Path::Windows>("..").resolved(*Path::BaseAbsolute<Path::Windows>::Create("C:\\")).raw());
    EXPECT_EQ("C:\\a\\b", Path::BaseRelative<Path::Windows>("a\\b").resolved(*Path::BaseAbsolute<Path::Windows>::Create("C:\\")).raw());
    EXPECT_EQ("\\\\?\\a\\b", Path::BaseRelative<Path::Windows>("b").resolved(*Path::BaseAbsolute<Path::Windows>::Create("\\\\?\\a\\")).raw());

    /* Rooted path. */
    EXPECT_EQ("C:\\b\\c", Path::BaseRelative<Path::Windows>("\\b\\c").resolved(*Path::BaseAbsolute<Path::Windows>::Create("C:\\a")).raw());
    EXPECT_EQ("\\\\?\\C:\\b\\c", Path::BaseRelative<Path::Windows>("\\b\\c").resolved(*Path::BaseAbsolute<Path::Windows>::Create("\\\\?\\C:\\a")).raw());
    EXPECT_EQ("D:\\", Path::BaseRelative<Path::Windows>("\\").resolved(*Path::BaseAbsolute<Path::Windows>::Create("D:\\")).raw());

    /* Drive relative path. */
    EXPECT_EQ("C:\\", Path::BaseRelative<Path::Windows>("C:").resolved(*Path::BaseAbsolute<Path::Windows>::Create("C:\\")).raw());
    EXPECT_EQ("C:\\a", Path::BaseRelative<Path::Windows>("C:a").resolved(*Path::BaseAbsolute<Path::Windows>::Create("C:\\b")).raw());
    EXPECT_EQ("C:\\a\\b", Path::BaseRelative<Path::Windows>("C:a\\b").resolved(*Path::BaseAbsolute<Path::Windows>::Create("C:\\c\\d")).raw());
}
