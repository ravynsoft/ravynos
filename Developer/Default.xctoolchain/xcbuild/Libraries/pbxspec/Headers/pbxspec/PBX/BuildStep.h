/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxspec_PBX_BuildStep_h
#define __pbxspec_PBX_BuildStep_h

#include <pbxspec/PBX/Specification.h>

#include <ext/optional>

namespace pbxspec { namespace PBX {

class BuildStep : public Specification {
public:
    typedef std::shared_ptr <BuildStep> shared_ptr;
    typedef std::vector <shared_ptr> vector;

private:
    ext::optional<std::string> _buildStepType;

protected:
    BuildStep();

public:
    virtual ~BuildStep();

public:
    inline SpecificationType type() const override
    { return BuildStep::Type(); }

public:
    inline BuildStep::shared_ptr base() const
    { return std::static_pointer_cast<BuildStep>(Specification::base()); }

public:
    inline ext::optional<std::string> const &buildStepType() const
    { return _buildStepType; }

protected:
    friend class Specification;
    bool parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

protected:
    bool inherit(Specification::shared_ptr const &base) override;
    virtual bool inherit(BuildStep::shared_ptr const &base);

protected:
    static BuildStep::shared_ptr Parse(Context *context, plist::Dictionary const *dict);

public:
    static inline SpecificationType Type()
    { return SpecificationType::BuildStep; }
};

} }

#endif  // !__pbxspec_PBX_BuildStep_h
