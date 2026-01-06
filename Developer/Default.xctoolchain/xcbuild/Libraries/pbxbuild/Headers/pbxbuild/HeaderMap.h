/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_HeaderMap_h
#define __pbxbuild_HeaderMap_h

#include <pbxbuild/HMapFile.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace pbxbuild {

class HeaderMap {
private:
    HMapHeader              _header;
    std::vector<HMapBucket> _buckets;
    std::vector<char>       _strings;
    std::unordered_set<std::string>         _keys;
    std::unordered_map<std::string, size_t> _offsets;
    bool                    _modified;

public:
    HeaderMap();

public:
    bool read(std::vector<uint8_t> const &buffer);
    std::vector<uint8_t> write();

public:
    void invalidate();

public:
    bool add(std::string const &key, std::string const &prefix, std::string const &suffix);

public:
    void dump();

private:
    void grow();
    void rehash(uint32_t newNumBuckets);
    void set(unsigned hash, uint32_t koff, uint32_t poff, uint32_t soff, bool growing);
    std::unordered_map<std::string, size_t>::iterator add(std::string const &string, bool key);
};

}

#endif  // !__pbxbuild_HeaderMap_h
