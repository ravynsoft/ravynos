/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_abplist_format_h
#define __plist_abplist_format_h

#include <stdint.h>

#define ABPLIST_MAGIC        "bplist"
#define ABPLIST_MAGIC_LENGTH 6
#define ABPLIST_VERSION      "00"

#pragma pack(push, 1)

typedef struct _abplist_header {
    char magic[6];
    char version[2];
} abplist_header_t;

typedef struct _abplist_trailer {
    uint8_t  __filler[6];
    uint8_t  offsetIntByteSize;
    uint8_t  objectRefByteSize;
    uint64_t objectsCount;
    uint64_t topLevelObject;
    uint64_t offsetTableEndOffset;
} abplist_trailer_t;

#pragma pack(pop)

#endif  /* !__plist_abplist_format_h */
