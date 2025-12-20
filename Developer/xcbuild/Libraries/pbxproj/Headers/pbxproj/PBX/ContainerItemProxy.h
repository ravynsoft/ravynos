/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_PBX_ContainerItemProxy_h
#define __pbxproj_PBX_ContainerItemProxy_h

#ifdef __linux__
#include <cstdint>
#endif

#include <pbxproj/PBX/FileReference.h>

namespace pbxproj { namespace PBX {

class ContainerItemProxy : public Object {
public:
    typedef std::shared_ptr <ContainerItemProxy> shared_ptr;

private:
    FileReference::shared_ptr _containerPortal;
    uint32_t                  _proxyType;
    std::string               _remoteGlobalIDString;
    std::string               _remoteInfo;

public:
    ContainerItemProxy();

public:
    inline FileReference::shared_ptr const &containerPortal() const
    { return _containerPortal; }

public:
    inline uint32_t proxyType() const
    { return _proxyType; }

public:
    inline std::string const &remoteGlobalIDString() const
    { return _remoteGlobalIDString; }

    inline std::string const &remoteInfo() const
    { return _remoteInfo; }

protected:
    bool parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

public:
    static inline char const *Isa()
    { return ISA::PBXContainerItemProxy; }
};

} }

#endif  // !__pbxproj_PBX_ContainerItemProxy_h
