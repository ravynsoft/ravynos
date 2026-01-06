/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/HeaderMap.h>
#include <libutil/Filesystem.h>
#include <libutil/DefaultFilesystem.h>

using pbxbuild::HeaderMap;
using libutil::Filesystem;
using libutil::DefaultFilesystem;

int
main(int argc, char **argv)
{
    DefaultFilesystem filesystem = DefaultFilesystem();

    if (argc < 2) {
        fprintf(stderr, "usage: %s <file.hmap>\n", argv[0]);
        return -1;
    }

    std::vector<uint8_t> contents;
    if (!filesystem.read(&contents, argv[1])) {
        fprintf(stderr, "error: cannot open '%s', failed to read\n", argv[1]);
        return -1;
    }

    HeaderMap hmap;
    if (!hmap.read(contents)) {
        fprintf(stderr, "error: cannot open '%s', not an hmap file\n", argv[1]);
        return -1;
    }

    hmap.dump();

    return 0;
}
