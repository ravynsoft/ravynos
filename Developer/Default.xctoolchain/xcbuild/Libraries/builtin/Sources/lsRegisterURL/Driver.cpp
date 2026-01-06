/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <builtin/lsRegisterURL/Driver.h>
#include <builtin/lsRegisterURL/Options.h>
#include <libutil/Filesystem.h>
#include <process/Context.h>

#if defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#endif
#endif

using builtin::lsRegisterURL::Driver;
using builtin::lsRegisterURL::Options;
using libutil::Filesystem;

Driver::
Driver()
{
}

Driver::
~Driver()
{
}

std::string Driver::
name()
{
    return "builtin-lsRegisterURL";
}

int Driver::
run(process::Context const *processContext, libutil::Filesystem *filesystem)
{
    Options options;
    std::pair<bool, std::string> result = libutil::Options::Parse<Options>(&options, processContext->commandLineArguments());
    if (!result.first) {
        fprintf(stderr, "error: %s\n", result.second.c_str());
        return 1;
    }

    if (!options.input()) {
        fprintf(stderr, "error: no input specified\n");
        return 1;
    }

#if defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
    CFURLRef URL = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, reinterpret_cast<const UInt8 *>(options.input()->c_str()), options.input()->size(), false);
    if (URL == NULL) {
        fprintf(stderr, "error: failed to create URL\n");
        return 1;
    }

    OSStatus status = LSRegisterURL(URL, true);
    CFRelease(URL);
    if (status != noErr) {
        fprintf(stderr, "error: LSRegisterURL failed %ld\n", (long)status);
        return 1;
    }
#else
    fprintf(stderr, "warning: not supported on this platform\n");
#endif

    return 0;
}
