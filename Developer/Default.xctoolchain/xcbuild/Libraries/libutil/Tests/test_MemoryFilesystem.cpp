/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <libutil/MemoryFilesystem.h>

using libutil::MemoryFilesystem;
using libutil::Filesystem;

static std::vector<uint8_t>
Contents(std::string const &string)
{
    return std::vector<uint8_t>(string.begin(), string.end());
}

static MemoryFilesystem
BasicFilesystem()
{
    return MemoryFilesystem({
        MemoryFilesystem::Entry::File("file1", Contents("one")),
        MemoryFilesystem::Entry::Directory("dir1", {
            MemoryFilesystem::Entry::File("file2", Contents("two1")),
        }),
        MemoryFilesystem::Entry::Directory("dir2", {
            MemoryFilesystem::Entry::File("file2", Contents("two2")),
            MemoryFilesystem::Entry::Directory("dir3", { }),
        }),
    });
}

TEST(MemoryFilesystem, Exists)
{
    auto filesystem = BasicFilesystem();
    EXPECT_TRUE(filesystem.exists(filesystem.path("")));
    EXPECT_TRUE(filesystem.exists(filesystem.path("dir1")));
    EXPECT_TRUE(filesystem.exists(filesystem.path("dir2")));
    EXPECT_TRUE(filesystem.exists(filesystem.path("dir2/dir3")));
    EXPECT_TRUE(filesystem.exists(filesystem.path("file1")));
    EXPECT_TRUE(filesystem.exists(filesystem.path("dir2/file2")));
    EXPECT_FALSE(filesystem.exists(filesystem.path("invalid")));
    EXPECT_FALSE(filesystem.exists(filesystem.path("dir1/invalid")));
    EXPECT_FALSE(filesystem.exists(filesystem.path("invalid1/invalid2")));
}

TEST(MemoryFilesystem, Type)
{
    auto filesystem = BasicFilesystem();
    EXPECT_EQ(filesystem.type(filesystem.path("")), Filesystem::Type::Directory);
    EXPECT_EQ(filesystem.type(filesystem.path("dir1")), Filesystem::Type::Directory);
    EXPECT_EQ(filesystem.type(filesystem.path("dir2")), Filesystem::Type::Directory);
    EXPECT_EQ(filesystem.type(filesystem.path("dir2/dir3")), Filesystem::Type::Directory);
    EXPECT_EQ(filesystem.type(filesystem.path("file1")), Filesystem::Type::File);
    EXPECT_EQ(filesystem.type(filesystem.path("dir2/file2")), Filesystem::Type::File);
    EXPECT_EQ(filesystem.type(filesystem.path("invalid")), ext::nullopt);
    EXPECT_EQ(filesystem.type(filesystem.path("invalid1/invalid2")), ext::nullopt);
}

TEST(MemoryFilesystem, IsReadable)
{
    auto filesystem = BasicFilesystem();
    EXPECT_TRUE(filesystem.isReadable(filesystem.path("")));
    EXPECT_TRUE(filesystem.isReadable(filesystem.path("dir1")));
    EXPECT_TRUE(filesystem.isReadable(filesystem.path("dir2")));
    EXPECT_TRUE(filesystem.isReadable(filesystem.path("dir2/dir3")));
    EXPECT_TRUE(filesystem.isReadable(filesystem.path("file1")));
    EXPECT_TRUE(filesystem.isReadable(filesystem.path("dir2/file2")));
    EXPECT_FALSE(filesystem.isReadable(filesystem.path("invalid")));
    EXPECT_FALSE(filesystem.isReadable(filesystem.path("dir1/invalid")));
    EXPECT_FALSE(filesystem.isReadable(filesystem.path("invalid1/invalid2")));
}

TEST(MemoryFilesystem, IsWritable)
{
    auto filesystem = BasicFilesystem();
    EXPECT_TRUE(filesystem.isWritable(filesystem.path("")));
    EXPECT_TRUE(filesystem.isWritable(filesystem.path("dir1")));
    EXPECT_TRUE(filesystem.isWritable(filesystem.path("dir2")));
    EXPECT_TRUE(filesystem.isWritable(filesystem.path("dir2/dir3")));
    EXPECT_TRUE(filesystem.isWritable(filesystem.path("file1")));
    EXPECT_TRUE(filesystem.isWritable(filesystem.path("dir2/file2")));
    EXPECT_FALSE(filesystem.isWritable(filesystem.path("invalid")));
    EXPECT_FALSE(filesystem.isWritable(filesystem.path("dir1/invalid")));
    EXPECT_FALSE(filesystem.isWritable(filesystem.path("invalid1/invalid2")));
}

TEST(MemoryFilesystem, IsExecutable)
{
    auto filesystem = BasicFilesystem();
    EXPECT_TRUE(filesystem.isExecutable(filesystem.path("")));
    EXPECT_TRUE(filesystem.isExecutable(filesystem.path("dir1")));
    EXPECT_TRUE(filesystem.isExecutable(filesystem.path("dir2")));
    EXPECT_TRUE(filesystem.isExecutable(filesystem.path("dir2/dir3")));
    EXPECT_TRUE(filesystem.isExecutable(filesystem.path("file1")));
    EXPECT_TRUE(filesystem.isExecutable(filesystem.path("dir2/file2")));
    EXPECT_FALSE(filesystem.isExecutable(filesystem.path("invalid")));
    EXPECT_FALSE(filesystem.isExecutable(filesystem.path("dir1/invalid")));
    EXPECT_FALSE(filesystem.isExecutable(filesystem.path("invalid1/invalid2")));
}

TEST(MemoryFilesystem, CreateFile)
{
    auto filesystem = BasicFilesystem();

    /* Create file. */
    EXPECT_FALSE(filesystem.exists(filesystem.path("new1")));
    EXPECT_TRUE(filesystem.createFile(filesystem.path("new1")));
    EXPECT_TRUE(filesystem.exists(filesystem.path("new1")));

    /* Create existing file. */
    EXPECT_TRUE(filesystem.exists(filesystem.path("file1")));
    EXPECT_TRUE(filesystem.createFile(filesystem.path("file1")));
    EXPECT_TRUE(filesystem.exists(filesystem.path("file1")));

    /* Create file in directory. */
    EXPECT_FALSE(filesystem.exists(filesystem.path("dir1/new1")));
    EXPECT_TRUE(filesystem.createFile(filesystem.path("dir1/new1")));
    EXPECT_TRUE(filesystem.exists(filesystem.path("dir1/new1")));

    /* Can't create file in nonexistent directory. */
    EXPECT_FALSE(filesystem.exists(filesystem.path("invalid/new1")));
    EXPECT_FALSE(filesystem.createFile(filesystem.path("invalid/new1")));
    EXPECT_FALSE(filesystem.exists(filesystem.path("invalid/new1")));
}

TEST(MemoryFilesystem, Read)
{
    auto filesystem = BasicFilesystem();
    std::vector<uint8_t> contents;

    /* Read file. */
    contents.clear();
    EXPECT_TRUE(filesystem.read(&contents, filesystem.path("file1")));
    EXPECT_EQ(contents, Contents("one"));

    /* Read file in subdirectory. */
    contents.clear();
    EXPECT_TRUE(filesystem.read(&contents, filesystem.path("dir1/file2")));
    EXPECT_EQ(contents, Contents("two1"));

    /* Read file in subdirectory. */
    contents.clear();
    EXPECT_TRUE(filesystem.read(&contents, filesystem.path("dir2/file2")));
    EXPECT_EQ(contents, Contents("two2"));

    /* Can't read root. */
    contents.clear();
    EXPECT_FALSE(filesystem.read(&contents, filesystem.path("")));
    EXPECT_EQ(contents, Contents(""));

    /* Can't read directory. */
    contents.clear();
    EXPECT_FALSE(filesystem.read(&contents, filesystem.path("dir1")));
    EXPECT_EQ(contents, Contents(""));

    /* Can't read nonexistent file. */
    contents.clear();
    EXPECT_FALSE(filesystem.read(&contents, filesystem.path("invalid")));
    EXPECT_EQ(contents, Contents(""));
}

TEST(MemoryFilesystem, Write)
{
    auto filesystem = BasicFilesystem();
    std::vector<uint8_t> contents;

    /* Write new file. */
    EXPECT_FALSE(filesystem.exists(filesystem.path("new")));
    EXPECT_TRUE(filesystem.write(Contents("new"), filesystem.path("new")));
    contents.clear();
    EXPECT_TRUE(filesystem.read(&contents, filesystem.path("new")));
    EXPECT_EQ(contents, Contents("new"));

    /* Rewrite existing file. */
    contents.clear();
    EXPECT_TRUE(filesystem.read(&contents, filesystem.path("file1")));
    EXPECT_EQ(contents, Contents("one"));
    EXPECT_TRUE(filesystem.write(Contents("new"), filesystem.path("file1")));
    contents.clear();
    EXPECT_TRUE(filesystem.read(&contents, filesystem.path("file1")));
    EXPECT_EQ(contents, Contents("new"));

    /* Can't write to a directory. */
    EXPECT_EQ(filesystem.type(filesystem.path("dir1")), Filesystem::Type::Directory);
    EXPECT_FALSE(filesystem.write(Contents("new"), filesystem.path("dir1")));
    EXPECT_EQ(filesystem.type(filesystem.path("dir1")), Filesystem::Type::Directory);

    /* Can't write in nonexistent directory. */
    EXPECT_FALSE(filesystem.exists(filesystem.path("invalid/new")));
    EXPECT_FALSE(filesystem.write(Contents("new"), filesystem.path("invalid/new")));
    EXPECT_FALSE(filesystem.exists(filesystem.path("invalid/new")));
}

TEST(MemoryFilesystem, CopyFile)
{
    std::vector<uint8_t> contents;
    auto filesystem = BasicFilesystem();

    /* Must copy to a real path. */
    EXPECT_FALSE(filesystem.copyFile(filesystem.path("file1"), filesystem.path("invalid/file1")));

    /* Can't copy over a directory, even an empty one. */
    EXPECT_FALSE(filesystem.copyFile(filesystem.path("file1"), filesystem.path("dir1")));
    EXPECT_FALSE(filesystem.copyFile(filesystem.path("file1"), filesystem.path("dir2/dir3")));

    /* Can copy to a new path. */
    contents.clear();
    EXPECT_TRUE(filesystem.copyFile(filesystem.path("file1"), filesystem.path("copied")));
    EXPECT_TRUE(filesystem.read(&contents, filesystem.path("copied")));
    EXPECT_EQ(contents, Contents("one"));

    /* Can copy over an existing file. */
    contents.clear();
    EXPECT_TRUE(filesystem.copyFile(filesystem.path("file1"), filesystem.path("dir1/file2")));
    EXPECT_TRUE(filesystem.read(&contents, filesystem.path("dir1/file2")));
    EXPECT_EQ(contents, Contents("one"));
}

TEST(MemoryFilesystem, RemoveFile)
{
    auto filesystem = BasicFilesystem();

    EXPECT_TRUE(filesystem.exists(filesystem.path("file1")));
    EXPECT_TRUE(filesystem.removeFile(filesystem.path("file1")));
    EXPECT_FALSE(filesystem.exists(filesystem.path("file1")));

    EXPECT_TRUE(filesystem.exists(filesystem.path("dir1/file2")));
    EXPECT_TRUE(filesystem.removeFile(filesystem.path("dir1/file2")));
    EXPECT_FALSE(filesystem.exists(filesystem.path("dir1/file2")));

    EXPECT_FALSE(filesystem.removeFile(filesystem.path("dir1")));
    EXPECT_FALSE(filesystem.removeFile(filesystem.path("dir2/dir3")));
    EXPECT_FALSE(filesystem.removeFile(filesystem.path("invalid")));
}

TEST(MemoryFilesystem, CreateDirectory)
{
    auto filesystem = BasicFilesystem();

    /* Create existing directory, non-recursive. */
    EXPECT_EQ(filesystem.type(filesystem.path("dir1")), Filesystem::Type::Directory);
    EXPECT_TRUE(filesystem.createDirectory(filesystem.path("dir1"), false));
    EXPECT_EQ(filesystem.type(filesystem.path("dir1")), Filesystem::Type::Directory);

    /* Create existing directory, recursive. */
    EXPECT_EQ(filesystem.type(filesystem.path("dir1")), Filesystem::Type::Directory);
    EXPECT_TRUE(filesystem.createDirectory(filesystem.path("dir1"), true));
    EXPECT_EQ(filesystem.type(filesystem.path("dir1")), Filesystem::Type::Directory);

    /* Create new directory, non-recursive. */
    EXPECT_FALSE(filesystem.exists(filesystem.path("new1")));
    EXPECT_TRUE(filesystem.createDirectory(filesystem.path("new1"), false));
    EXPECT_EQ(filesystem.type(filesystem.path("new1")), Filesystem::Type::Directory);

    /* Create new directory, recursive. */
    EXPECT_FALSE(filesystem.exists(filesystem.path("new2")));
    EXPECT_TRUE(filesystem.createDirectory(filesystem.path("new2"), true));
    EXPECT_EQ(filesystem.type(filesystem.path("new2")), Filesystem::Type::Directory);

    /* Create new subdirectory, non-recursive. */
    EXPECT_FALSE(filesystem.exists(filesystem.path("dir1/new1")));
    EXPECT_TRUE(filesystem.createDirectory(filesystem.path("dir1/new1"), false));
    EXPECT_EQ(filesystem.type(filesystem.path("dir1/new1")), Filesystem::Type::Directory);

    /* Create new subdirectory, recursive. */
    EXPECT_FALSE(filesystem.exists(filesystem.path("dir1/new2")));
    EXPECT_TRUE(filesystem.createDirectory(filesystem.path("dir1/new2"), true));
    EXPECT_EQ(filesystem.type(filesystem.path("dir1/new2")), Filesystem::Type::Directory);

    /* Can't create nested directories when non-recursive. */
    EXPECT_FALSE(filesystem.exists(filesystem.path("invalid/new1")));
    EXPECT_FALSE(filesystem.createDirectory(filesystem.path("invalid/new1"), false));
    EXPECT_EQ(filesystem.type(filesystem.path("invalid/new1")), ext::nullopt);
    EXPECT_EQ(filesystem.type(filesystem.path("invalid")), ext::nullopt);

    /* Can create nested directories when recursive. */
    EXPECT_FALSE(filesystem.exists(filesystem.path("invalid/new2")));
    EXPECT_TRUE(filesystem.createDirectory(filesystem.path("invalid/new2"), true));
    EXPECT_EQ(filesystem.type(filesystem.path("invalid/new2")), Filesystem::Type::Directory);
    EXPECT_EQ(filesystem.type(filesystem.path("invalid")), Filesystem::Type::Directory);
}

TEST(MemoryFilesystem, ReadDirectory)
{
    auto filesystem = BasicFilesystem();

    std::vector<std::string> files;
    auto accumulate = [&files](std::string const &name) {
        files.push_back(name);
    };

    /* List root contents, non-recursive. */
    files.clear();
    EXPECT_TRUE(filesystem.readDirectory(filesystem.path(""), false, accumulate));
    EXPECT_EQ(files, std::vector<std::string>({ "file1", "dir1", "dir2" }));

    /* List root contents, recursive. */
    files.clear();
    EXPECT_TRUE(filesystem.readDirectory(filesystem.path(""), true, accumulate));
    EXPECT_EQ(files, std::vector<std::string>({ "file1", "dir1/file2", "dir1", "dir2/file2", "dir2/dir3", "dir2" }));

    /* List file. */
    files.clear();
    EXPECT_TRUE(filesystem.readDirectory(filesystem.path("dir1"), false, accumulate));
    EXPECT_EQ(files, std::vector<std::string>({ "file2" }));
    files.clear();
    EXPECT_TRUE(filesystem.readDirectory(filesystem.path("dir1"), true, accumulate));
    EXPECT_EQ(files, std::vector<std::string>({ "file2" }));

    /* List files and directories. */
    files.clear();
    EXPECT_TRUE(filesystem.readDirectory(filesystem.path("dir2"), false, accumulate));
    EXPECT_EQ(files, std::vector<std::string>({ "file2", "dir3" }));
    files.clear();
    EXPECT_TRUE(filesystem.readDirectory(filesystem.path("dir2"), true, accumulate));
    EXPECT_EQ(files, std::vector<std::string>({ "file2", "dir3" }));

    /* List subdirectory. */
    files.clear();
    EXPECT_TRUE(filesystem.readDirectory(filesystem.path("dir2/dir3"), false, accumulate));
    EXPECT_EQ(files, std::vector<std::string>());

    /* Can't list file. */
    files.clear();
    EXPECT_FALSE(filesystem.readDirectory(filesystem.path("file1"), false, accumulate));
    EXPECT_EQ(files, std::vector<std::string>());
    files.clear();
    EXPECT_FALSE(filesystem.readDirectory(filesystem.path("file1"), true, accumulate));
    EXPECT_EQ(files, std::vector<std::string>());

    /* Can't list nonexistent directory. */
    files.clear();
    EXPECT_FALSE(filesystem.readDirectory(filesystem.path("invalid"), false, accumulate));
    EXPECT_EQ(files, std::vector<std::string>());
    files.clear();
    EXPECT_FALSE(filesystem.readDirectory(filesystem.path("invalid"), true, accumulate));
    EXPECT_EQ(files, std::vector<std::string>());
}

TEST(MemoryFilesystem, CopyDirectory)
{
    auto filesystem = BasicFilesystem();

    /* Must copy into a real path. */
    EXPECT_FALSE(filesystem.copyDirectory(filesystem.path("dir1"), filesystem.path("invalid/dir1"), false));

    /* Can copy into a new path non-recursive. */
    EXPECT_TRUE(filesystem.copyDirectory(filesystem.path("dir1"), filesystem.path("copied1"), false));
    EXPECT_EQ(filesystem.type(filesystem.path("copied1")), Filesystem::Type::Directory);

    /* Cannot overwrite non-empty directory when non-recursive. */
    EXPECT_FALSE(filesystem.copyDirectory(filesystem.path("dir1"), filesystem.path("dir2"), false));

    /* Can overwrite empty directory when non-recursive. */
    EXPECT_TRUE(filesystem.copyDirectory(filesystem.path("dir1"), filesystem.path("dir2/dir3"), false));
    EXPECT_EQ(filesystem.type(filesystem.path("dir2/dir3")), Filesystem::Type::Directory);

    /* Can copy into a new path recursive. */
    EXPECT_TRUE(filesystem.copyDirectory(filesystem.path("dir1"), filesystem.path("copied1"), true));
    EXPECT_EQ(filesystem.type(filesystem.path("copied1")), Filesystem::Type::Directory);
    EXPECT_EQ(filesystem.type(filesystem.path("copied1/file2")), Filesystem::Type::File);

    /* Can overwrite empty directory when recursive. */
    EXPECT_TRUE(filesystem.copyDirectory(filesystem.path("dir1"), filesystem.path("dir2/dir3"), true));
    EXPECT_EQ(filesystem.type(filesystem.path("dir2/dir3")), Filesystem::Type::Directory);
    EXPECT_EQ(filesystem.type(filesystem.path("dir2/dir3/file2")), Filesystem::Type::File);

    /* Can overwrite non-empty directory when recursive. */
    EXPECT_TRUE(filesystem.copyDirectory(filesystem.path("dir1"), filesystem.path("dir2"), true));
    EXPECT_EQ(filesystem.type(filesystem.path("dir2/dir3")), ext::nullopt);
    EXPECT_EQ(filesystem.type(filesystem.path("dir2/file2")), Filesystem::Type::File);
}

TEST(MemoryFilesystem, RemoveDirectory)
{
    auto filesystem = BasicFilesystem();

    /* Can remove empty directory when non-recursive. */
    EXPECT_TRUE(filesystem.removeDirectory(filesystem.path("dir2/dir3"), false));
    EXPECT_EQ(filesystem.type(filesystem.path("dir2/dir3")), ext::nullopt);

    /* Can't remove non-empty directory when non-recursive. */
    EXPECT_FALSE(filesystem.removeDirectory(filesystem.path("dir1"), false));
    EXPECT_EQ(filesystem.type(filesystem.path("dir1/file2")), Filesystem::Type::File);
    EXPECT_EQ(filesystem.type(filesystem.path("dir1")), Filesystem::Type::Directory);

    /* Can't remove non-empty directory when non-recursive. */
    EXPECT_TRUE(filesystem.removeDirectory(filesystem.path("dir1"), true));
    EXPECT_EQ(filesystem.type(filesystem.path("dir1/file2")), ext::nullopt);
    EXPECT_EQ(filesystem.type(filesystem.path("dir1")), ext::nullopt);

    /* Can't remove files. */
    EXPECT_TRUE(filesystem.exists(filesystem.path("file1")));
    EXPECT_FALSE(filesystem.removeDirectory(filesystem.path("file1"), false));
    EXPECT_FALSE(filesystem.removeDirectory(filesystem.path("file1"), true));
    EXPECT_TRUE(filesystem.exists(filesystem.path("file1")));

    /* Can't remove invalid paths. */
    EXPECT_FALSE(filesystem.removeDirectory(filesystem.path("invalid"), false));
    EXPECT_FALSE(filesystem.removeDirectory(filesystem.path("invalid"), true));
}

TEST(MemoryFilesystem, ResolvePath)
{
    auto filesystem = BasicFilesystem();
    EXPECT_EQ(filesystem.resolvePath(filesystem.path("")), filesystem.path(""));
    EXPECT_EQ(filesystem.resolvePath(filesystem.path("/")), filesystem.path(""));
    EXPECT_EQ(filesystem.resolvePath(filesystem.path("./")), filesystem.path(""));
    EXPECT_EQ(filesystem.resolvePath(filesystem.path("/file1")), filesystem.path("file1"));
    EXPECT_EQ(filesystem.resolvePath(filesystem.path("dir1")), filesystem.path("dir1"));
    EXPECT_EQ(filesystem.resolvePath(filesystem.path("dir1/")), filesystem.path("dir1"));
    EXPECT_EQ(filesystem.resolvePath(filesystem.path("dir1/..")), filesystem.path(""));
#if _WIN32
    EXPECT_EQ(filesystem.resolvePath(filesystem.path("dir1//file2")), filesystem.path("dir1\\file2"));
    EXPECT_EQ(filesystem.resolvePath(filesystem.path("dir2/.././dir1/file2")), filesystem.path("dir1\\file2"));
#else
    EXPECT_EQ(filesystem.resolvePath(filesystem.path("dir1//file2")), filesystem.path("dir1/file2"));
    EXPECT_EQ(filesystem.resolvePath(filesystem.path("dir2/.././dir1/file2")), filesystem.path("dir1/file2"));
#endif
}

