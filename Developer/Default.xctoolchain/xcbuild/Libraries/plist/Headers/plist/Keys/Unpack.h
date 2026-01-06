
/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Keys_Unpack_h
#define __plist_Keys_Unpack_h

#include <plist/Object.h>
#include <plist/Dictionary.h>

#include <string>
#include <vector>
#include <unordered_set>

namespace plist {
namespace Keys {

/*
 * Unpack values from a dictionary with validation. Warn about unknown keys and
 * incorrect types, and support separate parsing for subtype-based parsing.
 */
class Unpack {
private:
    std::string                      _name;
    Dictionary const                *_dict;
    std::unordered_set<std::string> *_seen;
    std::vector<std::string>         _errors;

public:
    /*
     * Create an unpack for a type with the specified name, unpacking the given
     * dictionary. The seen set is keys that have and will be unpacked from it.
     */
    Unpack(std::string const &name, Dictionary const *dict, std::unordered_set<std::string> *seen);

public:
    /*
     * The errors seen so far from unpacking.
     */
    std::vector<std::string> const &errors() const
    { return _errors; }

public:
    /*
     * Fetch an untyped object from the dictionary.
     */
    Object const *value(std::string const &key);

    /*
     * Unpack an object by casting it to the template type.
     */
    template<typename T>
    T const *cast(std::string const &key)
    {
        Object const *object = value(key);
        if (object == nullptr) {
            return nullptr;
        }

        if (T const *value = CastTo<T>(object)) {
            return value;
        }

        _errors.push_back(_name + " key " + key + " was type " + ObjectTypes::Name(object->type()) + "; expected " + ObjectTypes::Name(T::Type()));
        return nullptr;
    }

    /*
     * Unpack an object by coercing it to the template type.
     */
    template<typename T>
    std::unique_ptr<T> coerce(std::string const &key)
    {
        Object const *object = value(key);
        if (object == nullptr) {
            return nullptr;
        }

        if (std::unique_ptr<T> value = T::Coerce(object)) {
            return value;
        }

        _errors.push_back(_name + " key " + key + " was type " + ObjectTypes::Name(object->type()) + "; could not coerce to " + ObjectTypes::Name(T::Type()));
        return nullptr;
    }

public:
    /*
     * Check if unpacking was complete. If false, there are errors. Only check if check
     * is passed as true: to simplify parsing via subtypes, pass true only for leaves.
     */
    bool complete(bool check);

    /*
     * Error text to print for any seen errors.
     */
    std::string errorText() const;
};


}
}

#endif  // !__plist_Keys_Unpack_h
