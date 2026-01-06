/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/HeaderMap.h>

#include <algorithm>
#include <iterator>
#include <map>
#include <cstring>

using pbxbuild::HeaderMap;

static std::string
CanonicalizeKey(std::string const &key)
{
    //
    // canonicalize key
    //
    std::string lower;
    std::transform(key.begin(), key.end(), std::back_inserter(lower), ::tolower);
    return lower;
}

HeaderMap::HeaderMap() :
    _modified(false)
{
    ::memset(&_header, 0, sizeof(_header));

    //
    // Initially we set to 8 buckets.
    //
    _header.NumBuckets = 8;
    _buckets.resize(_header.NumBuckets);
}

bool HeaderMap::
read(std::vector<uint8_t> const &buffer)
{
    if (buffer.size() < sizeof(HMapHeader)) {
        return false;
    }

    memcpy((void *)&_header, (void *)buffer.data(), sizeof(HMapHeader));

    //
    // TODO Reverse endian
    //
    if (_header.Magic != HMAP_HeaderMagicNumber ||
        _header.Version != HMAP_HeaderVersion ||
        _header.Reserved != 0 ||
        _header.StringsOffset > static_cast <uint32_t> (buffer.size())) {
        invalidate();
        return false;
    }

    //
    // Reset
    //
    _offsets.clear();
    _keys.clear();

    //
    // Read buckets and strings
    //
    _buckets.resize(_header.NumBuckets);

    if (buffer.size() - sizeof(HMapHeader) < sizeof(HMapBucket) * _header.NumBuckets) {
        invalidate();
        return false;
    }
    memcpy((void *)_buckets.data(), (void *)(buffer.data() + sizeof(HMapHeader)), sizeof(HMapBucket) * _header.NumBuckets);

    size_t stringSize = buffer.size() - _header.StringsOffset;
    _strings.resize(stringSize);

    memcpy((void *)_strings.data(), (void *)(buffer.data() + _header.StringsOffset), stringSize);

    //
    // Now fill _keys and _offsets so that we can manipulate this hmap.
    //
    for (auto const &bucket : _buckets) {
        if (bucket.Key == HMAP_EmptyBucketKey)
            continue;

        if (bucket.Key >= _strings.size() ||
            bucket.Suffix >= _strings.size() ||
            bucket.Prefix >= _strings.size())
            continue;

        _keys.insert(CanonicalizeKey(&_strings[bucket.Key]));

        _offsets.insert(std::make_pair(
                    &_strings[bucket.Key],
                    bucket.Key));
        _offsets.insert(std::make_pair(
                    &_strings[bucket.Prefix],
                    bucket.Prefix));
        _offsets.insert(std::make_pair(
                    &_strings[bucket.Suffix],
                    bucket.Suffix));
    }

    _modified = false;

    return true;
}

std::vector<uint8_t> HeaderMap::
write()
{
    std::vector<uint8_t> buffer;

    if (_modified) {
        rehash(_header.NumBuckets);
    }

    _header.Magic         = HMAP_HeaderMagicNumber;
    _header.Version       = HMAP_HeaderVersion;
    _header.Reserved      = 0;
    _header.StringsOffset = sizeof(_header) + _header.NumBuckets * sizeof(HMapBucket);

    buffer.resize(sizeof(HMapHeader) + _header.NumBuckets * sizeof(HMapBucket) + _strings.size());

    //
    // Write buckets and strings
    //
    memcpy((void *)(buffer.data()), (void *)&_header, sizeof(HMapHeader));
    memcpy((void *)(buffer.data() + sizeof(HMapHeader)), (void *)_buckets.data(), _header.NumBuckets * sizeof(HMapBucket));
    memcpy((void *)(buffer.data() + _header.StringsOffset), (void *)_strings.data(), _strings.size());

    return buffer;
}

void HeaderMap::
invalidate()
{
    ::memset(&_header, 0, sizeof(_header));
    _buckets.clear();
    _strings.clear();
    _offsets.clear();
    _keys.clear();
    _modified = false;
}

bool HeaderMap::
add(std::string const &key, std::string const &prefix, std::string const &suffix)
{
    if (key.empty() || prefix.empty() || suffix.empty()) {
        return false; // invalid argument
    }

    if (_keys.find(CanonicalizeKey(key)) != _keys.end()) {
        // already exists
        return false;
    }

    //
    // Lookup key offset, if none, add one.
    //
    auto KI = add(key, true);

    //
    // Lookup prefix offset, if none, add one.
    //
    auto PI = add(prefix, false);

    //
    // Lookup suffix offset, if none, add one.
    //
    auto SI = add(suffix, false);

    //
    // Now let the table grow by one item, if needed.
    //
    grow();

    //
    // Find the bucket for this key.
    //
    unsigned hash = HashHMapKey(key) % _header.NumBuckets;
    set(hash, KI->second, PI->second, SI->second, false);
    if (key.length() > _header.MaxValueLength) {
        _header.MaxValueLength = key.length();
    }

    _modified = true;

    return true;
}

void HeaderMap::
grow()
{
    //
    // Grow at 3/4 of buckets
    //
    if (_header.NumEntries + 1 >= (_header.NumBuckets * 3) / 4) {
        //
        // Resize and rehash
        //
        rehash(_header.NumBuckets << 1);
    }
}

void HeaderMap::
rehash(uint32_t newNumBuckets)
{
    std::vector <HMapBucket> buckets;

    buckets.resize(newNumBuckets);
    _buckets.swap(buckets);

    std::multimap <uint32_t, HMapBucket *> rehashed;

    for (size_t n = 0; n < _header.NumBuckets; n++) {
        if (buckets[n].Key == HMAP_EmptyBucketKey)
            continue;

        unsigned hash = HashHMapKey(&_strings[buckets[n].Key]) % newNumBuckets;
        rehashed.insert(std::make_pair(hash, &buckets[n]));
    }

    for (auto BI : rehashed) {
        set(BI.first, BI.second->Key, BI.second->Prefix, BI.second->Suffix, true);
    }

    _header.NumBuckets = newNumBuckets;

    _modified = false;
}

std::unordered_map<std::string, size_t>::iterator HeaderMap::
add(std::string const &string, bool key)
{
    auto I = _offsets.find(string);
    if (I == _offsets.end()) {
        //
        // Since 0 is reserved, if the strings table is empty,
        // add an empty slot.
        //
        if (_strings.empty()) {
            _strings.resize(1);
        }

        size_t offset = _strings.size();
        _strings.resize(offset + string.length() + 1);
        ::memcpy(&_strings[offset], &string[0], string.length() + 1);

        I = _offsets.insert(std::make_pair(string, offset)).first;

        if (key) {
            //
            // store key
            //
            _keys.insert(CanonicalizeKey(string));
        }
    }
    return I;
}

void HeaderMap::
set(unsigned hash, uint32_t koff, uint32_t poff, uint32_t soff, bool growing)
{
    for (size_t n = hash; n != (hash - 1); n = (n + 1) % _header.NumBuckets) {
        if (_buckets[n].Key == HMAP_EmptyBucketKey) {
            _buckets[n].Key    = koff;
            _buckets[n].Prefix = poff;
            _buckets[n].Suffix = soff;
            if (!growing) {
                _header.NumEntries++;
            }
            break;
        }
    }
}

void HeaderMap::
dump()
{
    fprintf(stderr, "Num Entries = %u Num Buckets = %u Strings Offset = %#x\n",
            _header.NumEntries, _header.NumBuckets, _header.StringsOffset);

    for (size_t n = 0; n < _buckets.size(); n++) {
        if (_buckets[n].Key == HMAP_EmptyBucketKey)
            continue;

        if (_buckets[n].Key >= _strings.size() ||
            _buckets[n].Suffix >= _strings.size() ||
            _buckets[n].Prefix >= _strings.size()) {
            fprintf(stderr, "Bucket #%zu: broken\n", n);
        } else {
            fprintf(stderr,
                    "Bucket #%zu: [%u] Key = '%s' Prefix = '%s' Suffix = '%s'\n",
                    n, HashHMapKey(&_strings[_buckets[n].Key]) % _header.NumBuckets,
                    &_strings[_buckets[n].Key], &_strings[_buckets[n].Prefix],
                    &_strings[_buckets[n].Suffix]);
        }
    }
}
