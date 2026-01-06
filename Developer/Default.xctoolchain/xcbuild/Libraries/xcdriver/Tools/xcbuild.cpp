/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcdriver/Driver.h>
#include <libutil/DefaultFilesystem.h>
#include <process/DefaultContext.h>
#include <process/DefaultLauncher.h>
#include <process/DefaultUser.h>

using libutil::DefaultFilesystem;

int
main(int argc, char **argv)
{
    DefaultFilesystem filesystem = DefaultFilesystem();
    process::DefaultContext processContext = process::DefaultContext();
    process::DefaultLauncher processLauncher = process::DefaultLauncher();
    process::DefaultUser user = process::DefaultUser();
    return xcdriver::Driver::Run(&user, &processContext, &processLauncher, &filesystem);
}
