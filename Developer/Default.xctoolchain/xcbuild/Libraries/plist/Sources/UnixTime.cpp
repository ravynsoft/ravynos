/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/UnixTime.h>

#include <ctime>

using plist::UnixTime;

void UnixTime::
Decode(uint64_t in, struct tm &out)
{
#if _WIN32
    time_t const t = in;
    ::gmtime_s(&out, &t);
#else
    time_t t = in;
    ::gmtime_r(&t, &out);
#endif
}

uint64_t UnixTime::
Encode(struct tm const &in)
{
    struct tm copy = in;
    return ::mktime(&copy);
}
