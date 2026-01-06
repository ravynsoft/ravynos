/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PBX/BuildFile.h>
#include <pbxproj/PBX/FileReference.h>
#include <pbxproj/PBX/ReferenceProxy.h>
#include <pbxproj/PBX/Group.h>
#include <pbxproj/PBX/VariantGroup.h>
#include <pbxproj/XC/VersionGroup.h>
#include <pbxproj/Context.h>
#include <pbxsetting/Type.h>
#include <plist/Array.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxproj::PBX::BuildFile;
using pbxproj::Context;

BuildFile::
BuildFile() :
    Object(Isa())
{
}

BuildFile::
~BuildFile()
{
}

bool BuildFile::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Object::parse(context, dict, seen, false)) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("BuildFile", dict, seen);

    std::string FRID;
    std::string RPID;
    std::string GID;
    std::string VaGID;
    std::string VrGID;

    auto FR  = context.indirect <FileReference> (&unpack, "fileRef", &FRID);
    auto RP  = context.indirect <ReferenceProxy> (&unpack, "fileRef", &RPID);
    auto G   = context.indirect <Group> (&unpack, "fileRef", &GID);
    auto VaG = context.indirect <VariantGroup> (&unpack, "fileRef", &VaGID);
    auto VrG = context.indirect <XC::VersionGroup> (&unpack, "fileRef", &VrGID);
    auto S   = unpack.cast <plist::Dictionary> ("settings");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (FR != nullptr) {
        FileReference::shared_ptr fileReference = context.parseObject(context.fileReferences, FRID, FR);
        _fileRef = std::static_pointer_cast <GroupItem> (fileReference);
    } else if (RP != nullptr) {
        ReferenceProxy::shared_ptr referenceProxy = context.parseObject(context.referenceProxies, RPID, RP);
        _fileRef = std::static_pointer_cast <GroupItem> (referenceProxy);
    } else if (G != nullptr) {
        Group::shared_ptr group = context.parseObject(context.groups, GID, G);
        _fileRef = std::static_pointer_cast <GroupItem> (group);
    } else if (VaG != nullptr) {
        VariantGroup::shared_ptr variantGroup = context.parseObject(context.variantGroups, VaGID, VaG);
        _fileRef = std::static_pointer_cast <GroupItem> (variantGroup);
    } else if (VrG != nullptr) {
        XC::VersionGroup::shared_ptr versionGroup = context.parseObject(context.versionGroups, VrGID, VrG);
        _fileRef = std::static_pointer_cast <GroupItem> (versionGroup);
    }

    if (S != nullptr) {
        if (auto CF = S->value <plist::String> ("COMPILER_FLAGS")) {
            _compilerFlags = pbxsetting::Type::ParseList(CF->value());
        }

        if (auto A = S->value <plist::Array> ("ATTRIBUTES")) {
            for (size_t n = 0; n < A->count(); n++) {
                if (auto AA = A->value <plist::String> (n)) {
                    _attributes.push_back(AA->value());
                }
            }
        }
    }

    return true;
}
