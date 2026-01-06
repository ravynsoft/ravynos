/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxspec_PBX_BuildPhase_h
#define __pbxspec_PBX_BuildPhase_h

#include <pbxspec/PBX/Specification.h>

namespace pbxspec { namespace PBX {

class BuildPhase : public Specification {
public:
    typedef std::shared_ptr <BuildPhase> shared_ptr;
    typedef std::vector <shared_ptr> vector;

protected:
    BuildPhase();

public:
    virtual ~BuildPhase();

public:
    inline SpecificationType type() const override
    { return BuildPhase::Type(); }

public:
    inline BuildPhase::shared_ptr base() const
    { return std::static_pointer_cast<BuildPhase>(Specification::base()); }

protected:
    friend class Specification;
    bool parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

protected:
    bool inherit(Specification::shared_ptr const &base) override;
    virtual bool inherit(BuildPhase::shared_ptr const &base);

protected:
    static BuildPhase::shared_ptr Parse(Context *context, plist::Dictionary const *dict);

public:
    static inline SpecificationType Type()
    { return SpecificationType::BuildPhase; }
};

} }

#endif  // !__pbxspec_PBX_BuildPhase_h
