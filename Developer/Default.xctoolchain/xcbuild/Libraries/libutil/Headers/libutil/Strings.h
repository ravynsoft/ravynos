/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __libutil_Strings_h
#define __libutil_Strings_h

#include <libutil/Filesystem.h>

#if defined(_WIN32)
#include <string.h>
#else
#include <strings.h>
#endif

namespace libutil {

static inline int strcasecmp(const char *s1, const char *s2) {
#if defined(_WIN32)
  return ::_stricmp(s1, s2);
#else
  return ::strcasecmp(s1, s2);
#endif
}

}

#endif  // !__libutil_Strings_h
