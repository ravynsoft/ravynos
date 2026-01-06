/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <xcassets/Asset/Group.h>
#include <libutil/Filesystem.h>
#include <libutil/MemoryFilesystem.h>

namespace Asset = xcassets::Asset;
using libutil::Filesystem;
using libutil::MemoryFilesystem;

static std::vector<uint8_t>
Contents(std::string const &string)
{
    return std::vector<uint8_t>(string.begin(), string.end());
}

#define CONTENTS(...) Contents(#__VA_ARGS__)

TEST(Group, ProvidesNamespace)
{
    /* Define asset. */
    MemoryFilesystem filesystem = MemoryFilesystem({
        MemoryFilesystem::Entry::Directory("Namespaced", {
            MemoryFilesystem::Entry::File("Contents.json", CONTENTS({
                "properties" : { "provides-namespace": true }
            })),
            MemoryFilesystem::Entry::Directory("Inner", { }),
        }),
    });

    /* Load asset. */
    auto asset = xcassets::Asset::Asset::Load(&filesystem, filesystem.path("Namespaced"), { }, xcassets::Asset::Group::Extension());
    auto group = libutil::static_unique_pointer_cast<xcassets::Asset::Group>(std::move(asset));
    ASSERT_NE(group, nullptr);

    /* Verify asset. */
    ASSERT_EQ(group->children().size(), 1);
    xcassets::Asset::Asset const *firstAsset = group->children().front().get();
    EXPECT_EQ(firstAsset->name().string(), "Namespaced/Inner");
}

TEST(Group, FlatNamespace)
{
    /* Define asset. */
    MemoryFilesystem filesystem = MemoryFilesystem({
        MemoryFilesystem::Entry::Directory("Flat", {
            MemoryFilesystem::Entry::File("Contents.json", CONTENTS({
                "properties" : { "provides-namespace": false }
            })),
            MemoryFilesystem::Entry::Directory("Inner", { }),
        }),
    });

    /* Load asset. */
    auto asset = xcassets::Asset::Asset::Load(&filesystem, filesystem.path("Flat"), { }, xcassets::Asset::Group::Extension());
    auto group = libutil::static_unique_pointer_cast<xcassets::Asset::Group>(std::move(asset));
    ASSERT_NE(group, nullptr);

    /* Verify asset. */
    ASSERT_EQ(group->children().size(), 1);
    xcassets::Asset::Asset const *firstAsset = group->children().front().get();
    EXPECT_EQ(firstAsset->name().string(), "Inner");
}
