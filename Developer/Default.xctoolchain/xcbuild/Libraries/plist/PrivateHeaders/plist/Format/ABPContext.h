/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Format_ABPContext_h
#define __plist_Format_ABPContext_h

#include <plist/Format/abplist-format.h>

#include <vector>
#include <cstdio>
#include <cstdint>

class ABPContext {
protected:
    unsigned                    _flags;
    abplist_header_t            _header;
    abplist_trailer_t           _trailer;
    uint64_t                   *_offsets;

protected:
    off_t                       _offset;
    std::vector<uint8_t> const *_contents;

protected:
    ABPContext(std::vector<uint8_t> const *contents);
    ~ABPContext();

protected:
    off_t seek(off_t offset, int whence);
    off_t tell();

protected:
    size_t objectsCount();
};

/* Private Context Flags */
enum {
    kABPContextOpened   = (1 << 1),
    kABPContextComplete = (1 << 2),
    kABPContextFlushed  = (1 << 3)
};

#endif  /* !__plist_Format_ABPContext_h */
