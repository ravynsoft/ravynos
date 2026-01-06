/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcworkspace/XC/Workspace.h>
#include <xcworkspace/XC/Group.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Format/SimpleXML.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>
#include <process/Context.h>

using xcworkspace::XC::Workspace;
using libutil::Filesystem;
using libutil::FSUtil;

Workspace::
Workspace()
{
}

bool Workspace::
parse(plist::Dictionary const *dict)
{
    //
    // The workspace is a group of itself, we add some extra checks
    // like "version" shall be 1.0.
    //
    Group G;

    auto W = dict->value <plist::Dictionary> ("Workspace");
    if (W == nullptr)
        return false;

    auto V = W->value <plist::String> ("version");
    if (V != nullptr && V->value() != "1.0") {
        fprintf(stderr, "warning: xcode workspace version %s may be unsupported\n",
                V->value().c_str());
    }

    if (!G.parse(W))
        return false;

    //
    // Now move the contents
    //
    _items = std::move(G.items());
    for (GroupItem::shared_ptr &item : _items) {
        item->_parent = nullptr;
    }

    return true;
}

Workspace::shared_ptr Workspace::
Open(Filesystem const *filesystem, std::string const &path)
{
    if (path.empty()) {
        return nullptr;
    }

    std::string projectFileName = path + "/contents.xcworkspacedata";
    if (!filesystem->isReadable(projectFileName)) {
        return nullptr;
    }

    std::string realPath = filesystem->resolvePath(projectFileName);
    if (realPath.empty()) {
        return nullptr;
    }

    std::vector<uint8_t> contents;
    if (!filesystem->read(&contents, realPath)) {
        return nullptr;
    }

    //
    // Parse property list
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
    // Create the workspace object and parse the property list
    //
    auto workspace = std::make_shared <Workspace> ();
    if (workspace->parse(plist)) {
        //
        // Save some useful info
        //
        workspace->_dataFile    = realPath;
        workspace->_projectFile = FSUtil::GetDirectoryName(realPath);
        workspace->_basePath    = FSUtil::GetDirectoryName(workspace->_projectFile);
        workspace->_name        = FSUtil::GetBaseNameWithoutExtension(workspace->_projectFile);
    } else {
        workspace = nullptr;
    }

    return workspace;
}
