/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxsetting/Environment.h>
#include <libutil/FSUtil.h>

#include <algorithm>
#include <sstream>

using pbxsetting::Environment;
using pbxsetting::Level;
using pbxsetting::Condition;
using pbxsetting::Setting;
using pbxsetting::Value;
using libutil::FSUtil;

Environment::
Environment() :
    _offset(0)
{
}

static std::string
ProcessOperation(std::string const &value, std::string const &operation)
{
    const std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string digits = "0123456789";

    if (operation == "identifier" || operation == "c99extidentifier") {
        // TODO(grp): Support c99extidentifier correctly. Requires Unicode handling.

        const std::string begin = alphabet + "_";
        const std::string subsequent = begin + digits;

        std::string result = value;
        std::string::size_type offset = result.find_first_not_of(begin);
        while (offset != std::string::npos) {
            result[offset] = '_';
            offset = result.find_first_not_of(subsequent, offset);
        }

        return result;
    } else if (operation == "rfc1034identifier") {
        const std::string begin = alphabet;
        const std::string subsequent = alphabet + digits + "-";
        const std::string end = alphabet + digits;

        std::string result = value;
        for (std::string::iterator it = result.begin(), prev = result.end(), next = (it == result.end() ? it : std::next(it)); it != result.end(); prev = it, ++it, next = (it == result.end() ? it : std::next(it))) {
            // Cannot start or end with a dot.
            if (prev == result.end() || next == result.end()) {
                if (*it == '.') {
                    *it = '-';
                }
            }

            // Cannot have digit or hyphen after dot, or hyphen before dot.
            if (prev == result.end() || *prev == '.') {
                if (begin.find(*it) == std::string::npos) {
                    *it = '-';
                }
            } else if (next != result.end() && *next == '.') {
                if (subsequent.find(*it) == std::string::npos) {
                    *it = '-';
                }
            } else {
                if (end.find(*it) == std::string::npos) {
                    *it = '-';
                }
            }
        }

        return result;
    } else if (operation == "quote") {
        // FIXME(grp): This is (probably) valid, but not necessarily compatible. Algorithm from Python's shlex.quote().
        if (value.find_first_not_of(alphabet + digits + "@%_-+=:,./") == std::string::npos) {
            return value;
        } else {
            std::string result = value;
            std::string::size_type offset = 0;
            while ((offset = result.find("'", offset)) != std::string::npos) {
                result.replace(offset, 1, "'\"'\"'");
                offset += 5;
            }
            return "'" + result + "'";
        }
    } else if (operation == "lower") {
        std::string result = value;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    } else if (operation == "upper") {
        std::string result = value;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    } else if (operation == "standardizepath") {
        return FSUtil::NormalizePath(value);
    } else if (operation == "base") {
        return FSUtil::GetBaseNameWithoutExtension(value);
    } else if (operation == "dir") {
        return FSUtil::GetDirectoryName(value);
    } else if (operation == "file") {
        return FSUtil::GetBaseName(value);
    } else if (operation == "suffix") {
        return "." + FSUtil::GetFileExtension(value);
    } else {
        fprintf(stderr, "warning: unknown build setting operation '%s'\n", operation.c_str());
        return value;
    }
}

std::string Environment::
resolveValue(Condition const &condition, Value const &value, InheritanceContext const &context) const
{
    std::string result;
    for (auto const &entry : value.entries()) {
        switch (entry.type()) {
            case Value::Entry::Type::String: {
                result += *entry.string();
                break;
            }
            case Value::Entry::Type::Value: {
                std::string resolved = resolveValue(condition, *entry.value(), context);
                if (context.valid && (resolved == context.setting || resolved == "inherited")) {
                    result += resolveInheritance(condition, context);
                } else {
                    std::string setting = resolved;

                    std::string::size_type colon = resolved.find(':');
                    if (colon != std::string::npos) {
                        setting = resolved.substr(0, colon);
                    }

                    std::string value = resolveAssignment(condition, setting);

                    while (colon != std::string::npos) {
                        std::string::size_type next = resolved.find(':', colon + 1);

                        std::string operation = resolved.substr(colon + 1, next == std::string::npos ? next : next - colon - 1);
                        value = ProcessOperation(value, operation);

                        colon = next;
                    }

                    result += value;
                }
                break;
            }
        }
    }
    return result;
}

std::string Environment::
resolveInheritance(Condition const &condition, InheritanceContext const &context) const
{
    InheritanceContext ctx = context;
    for (++ctx.it; ctx.it != _levels.end(); ++ctx.it) {
        auto result = ctx.it->get(ctx.setting, condition);
        if (result.first) {
            return resolveValue(condition, result.second, ctx);
        }
    }

    return "";
}

std::string Environment::
resolveAssignment(Condition const &condition, std::string const &setting) const
{
    InheritanceContext context = { true, setting };

    for (context.it = _levels.begin(); context.it != _levels.end(); ++context.it) {
        Level const &level = *context.it;
        auto result = level.get(setting, condition);
        if (result.first) {
            return resolveValue(condition, result.second, context);
        }
    }

    if (condition.values().empty()) {
        return "";
    } else {
        return resolveAssignment(Condition::Empty(), setting);
    }
}

std::string Environment::
expand(Value const &value, Condition const &condition) const
{
    return resolveValue(condition, value, { false });
}

std::string Environment::
expand(Value const &value) const
{
    return expand(value, Condition::Empty());
}

std::string Environment::
resolve(std::string const &setting, Condition const &condition) const
{
    return resolveAssignment(condition, setting);
}

std::string Environment::
resolve(std::string const &setting) const
{
    return resolve(setting, Condition::Empty());
}

std::unordered_map<std::string, std::string> Environment::
computeValues(Condition const &condition) const
{
    std::unordered_map<std::string, std::string> values;

    for (Level const &level : _levels) {
        for (Setting const &setting : level.settings()) {
            if (values.find(setting.name()) == values.end()) {
                values[setting.name()] = resolve(setting.name(), condition);
            }
        }
    }

    return values;
}

void Environment::
insertFront(Level const &level, bool isDefault)
{
    if (!isDefault) {
        _levels.push_front(level);
        ++_offset;
    } else {
        _levels.insert(std::next(_levels.begin(), _offset), level);
    }
}

void Environment::
insertBack(Level const &level, bool isDefault)
{
    if (!isDefault) {
        _levels.insert(std::next(_levels.begin(), _offset), level);
        ++_offset;
    } else {
        _levels.push_back(level);
    }
}

void Environment::
dump() const
{
    size_t offset = 0;

    for (Level const &level : _levels) {
        if (offset == _offset) {
            printf("=== Default Levels ===\n");
        } else if (offset == 0) {
            printf("=== Remaining Levels ===\n");
        }

        printf("Level:\n");
        for (Setting const &setting : level.settings()) {
            printf("    %s = %s\n", setting.name().c_str(), setting.value().raw().c_str());
        }
        printf("\n");

        ++offset;
    }
}

