/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Format/ABPContext.h>

#include <cstring>

ABPContext::
ABPContext(std::vector<uint8_t> const *contents) :
    _flags   (0),
    _offsets (nullptr),
    _offset  (0),
    _contents(contents)
{
}

ABPContext::
~ABPContext()
{
    if (this->_offsets != NULL) {
        delete[] this->_offsets;
    }
}

off_t ABPContext::
seek(off_t offset, int whence)
{
    switch (whence) {
        case SEEK_SET:
            this->_offset = offset;
            break;
        case SEEK_CUR:
            this->_offset += offset;
            break;
        case SEEK_END:
            this->_offset = this->_contents->size() + offset;
        default:
            break;
    }

    /* Error if before the beginning. */
    if (this->_offset < 0) {
        this->_offset = 0;
        return -1;
    }

    /* Error if past the end. */
    if (this->_offset > static_cast<off_t>(this->_contents->size())) {
        this->_offset = static_cast<off_t>(this->_contents->size());
        return -1;
    }

    /* Success, return current offset. */
    return this->_offset;
}

off_t ABPContext::
tell()
{
    return this->seek(0, SEEK_CUR);
}

size_t ABPContext::
objectsCount()
{
    return static_cast<size_t>(this->_trailer.objectsCount);
}

