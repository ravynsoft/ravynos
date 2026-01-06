/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <acdriver/Version.h>

using acdriver::Version;

Version::
Version()
{
}

Version::
~Version()
{
}

int Version::
BuildVersion()
{
    return 1;
}

std::string Version::
UserVersion()
{
    return "1";
}

