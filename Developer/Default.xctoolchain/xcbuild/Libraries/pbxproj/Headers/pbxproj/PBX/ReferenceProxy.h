/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_PBX_ReferenceProxy_h
#define __pbxproj_PBX_ReferenceProxy_h

#include <pbxproj/PBX/GroupItem.h>
#include <pbxproj/PBX/ContainerItemProxy.h>

namespace pbxproj { namespace PBX {

class ReferenceProxy : public GroupItem {
public:
    typedef std::shared_ptr <ReferenceProxy> shared_ptr;
    typedef std::vector <shared_ptr> vector;

private:
    std::string                    _fileType;
    ContainerItemProxy::shared_ptr _remoteRef;

public:
    ReferenceProxy();

public:
    inline std::string const &fileType() const
    { return _fileType; }
    inline ContainerItemProxy::shared_ptr const &remoteRef() const
    { return _remoteRef; }

protected:
    bool parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

public:
    static inline char const *Isa()
    { return ISA::PBXReferenceProxy; }
};

} }

#endif  // !__pbxproj_PBX_ReferenceProxy_h
