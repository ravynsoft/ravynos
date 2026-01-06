/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_PBX_TargetDependency_h
#define __pbxproj_PBX_TargetDependency_h

#include <pbxproj/PBX/FileReference.h>

namespace pbxproj { namespace PBX {

// Avoid circular dependencies
class Target;
class ContainerItemProxy;

class TargetDependency : public Object {
public:
    typedef std::shared_ptr <TargetDependency> shared_ptr;
    typedef std::vector <shared_ptr> vector;

private:
    std::string                          _name;
    std::shared_ptr <Target>             _target;
    std::shared_ptr <ContainerItemProxy> _targetProxy;

public:
    TargetDependency();

public:
    inline std::string const &name() const
    { return _name; }

public:
    inline std::shared_ptr<Target> const &target() const
    { return _target; }
    inline std::shared_ptr<ContainerItemProxy> const &targetProxy() const
    { return _targetProxy; }

protected:
    bool parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

public:
    static inline char const *Isa()
    { return ISA::PBXTargetDependency; }
};

} }

#endif  // !__pbxproj_PBX_TargetDependency_h
