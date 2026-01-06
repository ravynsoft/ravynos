/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxspec/pbxspec.h>
#include <libutil/DefaultFilesystem.h>
#include <libutil/Filesystem.h>

#include <cstdio>

using pbxspec::Manager;
using pbxspec::PBX::Specification;
using libutil::DefaultFilesystem;

int
main(int argc, char **argv)
{
    DefaultFilesystem filesystem = DefaultFilesystem();

    if (argc < 2) {
        fprintf(stderr, "usage: %s path\n", argv[0]);
        return -1;
    }

    std::string path = argv[1];
    Manager::shared_ptr manager = Manager::Create();
    manager->registerDomains(&filesystem, { { "xcspec", path } });

    return 0;
}
