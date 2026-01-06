/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcscheme/XC/Scheme.h>
#include <xcscheme/XC/Actions.h>
#include <plist/Dictionary.h>
#include <plist/Integer.h>
#include <plist/String.h>
#include <plist/Format/SimpleXML.h>
#include <libutil/Filesystem.h>

using xcscheme::XC::Scheme;
using libutil::Filesystem;

Scheme::
Scheme(std::string const &name, std::string const &owner) :
    _name              (name),
    _owner             (owner),
    _lastUpgradeVersion(0)
{
}

bool Scheme::
parse(plist::Dictionary const *dict)
{
    auto S = dict->value <plist::Dictionary> ("Scheme");
    if (S == nullptr)
        return false;

    auto LUV = S->value <plist::Integer> ("LastUpgradeVersion");
    auto V   = S->value <plist::String> ("version");
    auto BA  = S->value <plist::Dictionary> ("BuildAction");
    auto TA  = S->value <plist::Dictionary> ("TestAction");
    auto LA  = S->value <plist::Dictionary> ("LaunchAction");
    auto PA  = S->value <plist::Dictionary> ("ProfileAction");
    auto AnA = S->value <plist::Dictionary> ("AnalyzeAction");
    auto ArA = S->value <plist::Dictionary> ("ArchiveAction");

    if (LUV != nullptr) {
        _lastUpgradeVersion = LUV->value();
    }

    if (V != nullptr) {
        _version = V->value();
    }

    if (BA != nullptr) {
        _buildAction = std::make_shared <BuildAction> ();
        if (!_buildAction->parse(BA))
            return false;
    }

    if (TA != nullptr) {
        _testAction = std::make_shared <TestAction> ();
        if (!_testAction->parse(TA))
            return false;
    }

    if (LA != nullptr) {
        _launchAction = std::make_shared <LaunchAction> ();
        if (!_launchAction->parse(LA))
            return false;
    }

    if (PA != nullptr) {
        _profileAction = std::make_shared <ProfileAction> ();
        if (!_profileAction->parse(PA))
            return false;
    }

    if (AnA != nullptr) {
        _analyzeAction = std::make_shared <AnalyzeAction> ();
        if (!_analyzeAction->parse(AnA))
            return false;
    }

    if (ArA != nullptr) {
        _archiveAction = std::make_shared <ArchiveAction> ();
        if (!_archiveAction->parse(ArA))
            return false;
    }

    return true;
}

Scheme::shared_ptr Scheme::
Open(Filesystem const *filesystem, std::string const &name, std::string const &owner, std::string const &path)
{
    if (path.empty()) {
        return nullptr;
    }

    if (!filesystem->isReadable(path)) {
        return nullptr;
    }

    std::string realPath = filesystem->resolvePath(path);
    if (realPath.empty()) {
        return nullptr;
    }

    std::vector<uint8_t> contents;
    if (!filesystem->read(&contents, realPath)) {
        return nullptr;
    }

    //
    // Parse simple XML
    //
    std::unique_ptr<plist::Object> root = plist::Format::SimpleXML::Deserialize(contents).first;
    if (root == nullptr) {
        return nullptr;
    }

    plist::Dictionary *plist = plist::CastTo<plist::Dictionary>(root.get());
    if (plist == nullptr) {
        return nullptr;
    }

    //
    // Parse the scheme dictionary and create the scheme object.
    //
    auto scheme = std::make_shared <Scheme> (name, owner);
    if (scheme->parse(plist)) {
        scheme->_path = realPath;
    } else {
        scheme = nullptr;
    }

    return scheme;
}
