/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <builtin/copyPlist/Driver.h>
#include <libutil/DefaultFilesystem.h>
#include <process/DefaultContext.h>

using libutil::DefaultFilesystem;

int
main(int argc, char **argv, char **envp)
{
    DefaultFilesystem filesystem = DefaultFilesystem();
    process::DefaultContext processContext = process::DefaultContext();

    builtin::copyPlist::Driver driver;
    return driver.run(&processContext, &filesystem);
}
