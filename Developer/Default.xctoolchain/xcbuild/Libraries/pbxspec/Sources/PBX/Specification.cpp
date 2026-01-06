/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxspec/PBX/Specification.h>
#include <pbxspec/PBX/BuildPhase.h>
#include <pbxspec/PBX/BuildSystem.h>
#include <pbxspec/PBX/Compiler.h>
#include <pbxspec/PBX/FileType.h>
#include <pbxspec/PBX/Linker.h>
#include <pbxspec/PBX/PackageType.h>
#include <pbxspec/PBX/ProductType.h>
#include <pbxspec/PBX/Tool.h>
#include <pbxspec/Context.h>
#include <pbxspec/Inherit.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/Object.h>
#include <plist/String.h>
#include <plist/Format/Any.h>
#include <plist/Keys/Unpack.h>
#include <libutil/Filesystem.h>

using pbxspec::PBX::Specification;
using pbxspec::SpecificationType;
using pbxspec::SpecificationTypes;
using libutil::Filesystem;

Specification::
Specification()
{
}

bool Specification::
operator==(Specification const &rhs) const
{
    return _identifier.compare(rhs._identifier) == 0 && _domain.compare(rhs._domain) == 0;
}

bool Specification::
operator!=(Specification const &rhs) const
{
    return !(*this == rhs);
}

bool Specification::
inherit(Specification::shared_ptr const &base)
{
    if (base == nullptr || base.get() == this)
        return false;

    _base               = base;
    _clazz              = Inherit::Override(_clazz, base->_clazz);
    _isGlobalDomainInUI = Inherit::Override(_isGlobalDomainInUI, base->_isGlobalDomainInUI);
    _name               = Inherit::Override(_name, base->_name);
    _description        = Inherit::Override(_description, base->_description);
    _vendor             = Inherit::Override(_vendor, base->_vendor);
    _version            = Inherit::Override(_version, base->_version);

    return true;
}

bool Specification::
parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    auto unpack = plist::Keys::Unpack("Specification", dict, seen);

    auto T  = unpack.cast <plist::String> ("Type");
    auto C  = unpack.cast <plist::String> ("Class");
    auto I  = unpack.cast <plist::String> ("Identifier");
    auto BO = unpack.cast <plist::String> ("BasedOn");
    auto DO = unpack.cast <plist::String> ("Domain");
    auto GD = unpack.coerce <plist::Boolean> ("IsGlobalDomainInUI");
    auto N  = unpack.cast <plist::String> ("Name");
    auto D  = unpack.cast <plist::String> ("Description");
    auto V1 = unpack.cast <plist::String> ("Vendor");
    auto V2 = unpack.cast <plist::String> ("Version");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (T != nullptr) {
        /* Nothing to do. (If this is used, see `Context`'s `defaultType`. */
    }

    if (C != nullptr) {
        _clazz = C->value();
    }

    if (DO != nullptr) {
        _domain = DO->value();
    } else {
        _domain = context->domain;
    }

    if (I != nullptr) {
        _identifier = I->value();
    }

    if (BO != nullptr) {
        auto basedOn           = BO->value();

        _basedOnDomain     = _domain;
        _basedOnIdentifier = basedOn;

        auto colon = basedOn.find(':');
        if (colon != std::string::npos) {
            _basedOnDomain     = basedOn.substr(0, colon);
            _basedOnIdentifier = basedOn.substr(colon + 1);
        }
    }

    if (GD != nullptr) {
        _isGlobalDomainInUI = GD->value();
    }

    if (N != nullptr) {
        _name = N->value();
    }

    if (D != nullptr) {
        _description = D->value();
    }

    if (V1 != nullptr) {
        _vendor = V1->value();
    }

    if (V2 != nullptr) {
        _version = V2->value();
    }

    return true;
}

bool Specification::
ParseType(Context *context, plist::Dictionary const *dict, SpecificationType expectedType)
{
    auto T = dict->value <plist::String> ("Type");
    if (T == nullptr) {
        /* Must have been a default type. */
        return true;
    }

    ext::optional<SpecificationType> type = SpecificationTypes::Parse(T->value());
    return (type == expectedType);
}

Specification::shared_ptr Specification::
Parse(Context *context, plist::Dictionary const *dict, ext::optional<SpecificationType> defaultType)
{
    ext::optional<SpecificationType> type;
    if (auto T = dict->value<plist::String>("Type")) {
        type = SpecificationTypes::Parse(T->value());
        if (!type) {
            fprintf(stderr, "error: specification type '%s' not supported\n", T->value().c_str());
            return nullptr;
        }
    } else {
        type = defaultType;
        if (!type) {
            fprintf(stderr, "error: no specification type\n");
            return nullptr;
        }
    }

    switch (*type) {
        case SpecificationType::Architecture:
            return Architecture::Parse(context, dict);
        case SpecificationType::BuildPhase:
            return BuildPhase::Parse(context, dict);
        case SpecificationType::BuildSettings:
            return BuildSettings::Parse(context, dict);
        case SpecificationType::BuildStep:
            return BuildStep::Parse(context, dict);
        case SpecificationType::BuildSystem:
            return BuildSystem::Parse(context, dict);
        case SpecificationType::Compiler:
            return Compiler::Parse(context, dict);
        case SpecificationType::FileType:
            return FileType::Parse(context, dict);
        case SpecificationType::Linker:
            return Linker::Parse(context, dict);
        case SpecificationType::PackageType:
            return PackageType::Parse(context, dict);
        case SpecificationType::ProductType:
            return ProductType::Parse(context, dict);
        case SpecificationType::Tool:
            return Tool::Parse(context, dict);
    }

    abort();
}

ext::optional<Specification::vector> Specification::
Open(Filesystem const *filesystem, Context *context, std::string const &filename, ext::optional<SpecificationType> defaultType)
{
    if (filename.empty()) {
        fprintf(stderr, "error: empty specification path\n");
        return ext::nullopt;
    }

    std::string realPath = filesystem->resolvePath(filename);
    if (realPath.empty()) {
        fprintf(stderr, "error: invalid specification path\n");
        return ext::nullopt;
    }

    std::vector<uint8_t> contents;
    if (!filesystem->read(&contents, realPath)) {
        fprintf(stderr, "error: unable to read specification plist\n");
        return ext::nullopt;
    }

    //
    // Parse property list
    //
    std::unique_ptr<plist::Object> plist = plist::Format::Any::Deserialize(contents).first;
    if (plist == nullptr) {
        fprintf(stderr, "error: unable to parse specification plist\n");
        return ext::nullopt;
    }

    //
    // If this is a dictionary, then it's a single specification,
    // if it's an array then multiple specifications are present.
    //
    if (auto dict = plist::CastTo <plist::Dictionary> (plist.get())) {
        if (auto spec = Parse(context, dict, defaultType)) {
            return Specification::vector({ spec });
        } else {
            fprintf(stderr, "error: single specification failed to parse\n");
            return ext::nullopt;
        }
    } else if (auto array = plist::CastTo <plist::Array> (plist.get())) {
        size_t errors = 0;
        Specification::vector specifications;

        for (size_t n = 0; n < array->count(); n++) {
            if (auto dict = array->value <plist::Dictionary> (n)) {
                if (auto spec = Parse(context, dict, defaultType)) {
                    specifications.push_back(spec);
                } else {
                    fprintf(stderr, "error: specification failed to parse\n");
                    errors++;
                }
            } else {
                fprintf(stderr, "error: specification entry was not a dictionary\n");
                errors++;
            }
        }

        if (errors == 0 || array->count() == 0) {
            return specifications;
        } else {
            fprintf(stderr, "error: specification failed to parse, errors %zu\n", errors);
            return ext::nullopt;
        }
    }

    fprintf(stderr, "error: specification file '%s' does not contain a dictionary nor an array", filename.c_str());
    return ext::nullopt;
}

