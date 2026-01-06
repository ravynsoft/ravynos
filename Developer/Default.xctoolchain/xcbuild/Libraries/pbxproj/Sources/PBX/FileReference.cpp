/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PBX/FileReference.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/Integer.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxproj::PBX::FileReference;
using pbxproj::Context;

FileReference::
FileReference() :
    GroupItem      (Isa(), GroupItem::Type::FileReference),
    _includeInIndex(false),
    _fileEncoding  (FileEncoding::Default),
    _lineEnding    (LineEnding::Unix)
{
}

bool FileReference::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!GroupItem::parse(context, dict, seen, false)) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("FileReference", dict, seen);

    auto LKFT = unpack.cast <plist::String> ("lastKnownFileType");
    auto EFT  = unpack.cast <plist::String> ("explicitFileType");
    auto XLSI = unpack.cast <plist::String> ("xcLanguageSpecificationIdentifier");
    auto III  = unpack.coerce <plist::Boolean> ("includeInIndex");
    auto FE   = unpack.coerce <plist::Integer> ("fileEncoding");
    auto LE   = unpack.coerce <plist::Integer> ("lineEnding");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (LKFT != nullptr) {
        _lastKnownFileType = LKFT->value();
    }

    if (EFT != nullptr) {
        _explicitFileType = EFT->value();
    }

    if (XLSI != nullptr) {
        _xcLanguageSpecificationIdentifier = XLSI->value();
    }

    if (III != nullptr) {
        _includeInIndex = III->value();
    }

    if (FE != nullptr) {
        _fileEncoding = static_cast<FileEncoding>(FE->value());
    }

    if (LE != nullptr) {
        _lineEnding = static_cast<LineEnding>(LE->value());
    }

    return true;
}
