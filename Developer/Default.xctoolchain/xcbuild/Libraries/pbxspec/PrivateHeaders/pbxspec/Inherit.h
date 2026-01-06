/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxspec_Inherit_h
#define __pbxspec_Inherit_h

#include <pbxsetting/Level.h>
#include <pbxsetting/Setting.h>
#include <pbxspec/PBX/PropertyOption.h>

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <ext/optional>

namespace pbxspec {

/*
 * Utilities for semantically inheriting various value types in specifications.
 */
class Inherit {
private:
    Inherit();
    ~Inherit();

public:
    /*
     * Override the base value with the current value.
     */
    template<typename T>
    static ext::optional<T>
    Override(ext::optional<T> const &current, ext::optional<T> const &base)
    {
        return current ? current : base;
    }

public:
    /*
     * Merge two vectors.
     */
    template<typename U>
    static ext::optional<std::vector<U>>
    Combine(ext::optional<std::vector<U>> const &current, ext::optional<std::vector<U>> const &base)
    {
        if (base || current) {
            std::vector<U> combined;
            if (base) {
                combined.insert(combined.end(), base->begin(), base->end());
            }
            if (current) {
                combined.insert(combined.end(), current->begin(), current->end());
            }
            return combined;
        } else {
            return ext::nullopt;
        }
    }

    template<typename U>
    static ext::optional<std::unordered_set<U>>
    Combine(ext::optional<std::unordered_set<U>> const &current, ext::optional<std::unordered_set<U>> const &base)
    {
        if (base || current) {
            std::unordered_set<U> combined;
            if (base) {
                combined.insert(base->begin(), base->end());
            }
            if (current) {
                combined.insert(current->begin(), current->end());
            }
            return combined;
        } else {
            return ext::nullopt;
        }
    }

    template<typename U, typename V>
    static ext::optional<std::unordered_map<U, V>>
    Combine(ext::optional<std::unordered_map<U, V>> const &current, ext::optional<std::unordered_map<U, V>> const &base)
    {
        if (base || current) {
            std::unordered_map<U, V> combined;
            if (base) {
                combined.insert(base->begin(), base->end());
            }
            if (current) {
                combined.insert(current->begin(), current->end());
            }
            return combined;
        } else {
            return ext::nullopt;
        }
    }

    /*
     * Merge two levels of settings.
     */
    static ext::optional<pbxsetting::Level>
    Combine(ext::optional<pbxsetting::Level> const &current, ext::optional<pbxsetting::Level> const &base)
    {
        if (base || current) {
            std::vector<pbxsetting::Setting> settings;
            if (base) {
                settings.insert(settings.end(), base->settings().begin(), base->settings().end());
            }
            if (current) {
                settings.insert(settings.end(), current->settings().begin(), current->settings().end());
            }
            return pbxsetting::Level(settings);
        } else {
            return ext::nullopt;
        }
    }

    /*
     * Merge two options lists.
     */
    static ext::optional<PBX::PropertyOption::vector>
    Combine(ext::optional<PBX::PropertyOption::vector> const &current, ext::optional<PBX::PropertyOption::vector> const &base, PBX::PropertyOption::used_map *currentUsedMap, PBX::PropertyOption::used_map *baseUsedMap)
    {
        if (base || current) {
            PBX::PropertyOption::vector options = base.value_or(PBX::PropertyOption::vector());
            *currentUsedMap = *baseUsedMap;

            if (current) {
                for (PBX::PropertyOption::shared_ptr const &option : *current) {
                    PBX::PropertyOption::Insert(&options, currentUsedMap, option);
                }
            }

            return options;
        } else {
            return ext::nullopt;
        }
    }
};

}

#endif /* __pbxspec_Inherit_h */
