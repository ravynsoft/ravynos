/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxspec_PBX_BuildSettings_h
#define __pbxspec_PBX_BuildSettings_h

#include <pbxspec/PBX/Specification.h>
#include <pbxspec/PBX/PropertyOption.h>

#include <ext/optional>

namespace pbxspec { namespace PBX {

class BuildSettings : public Specification {
public:
    typedef std::shared_ptr <BuildSettings> shared_ptr;
    typedef std::vector <shared_ptr> vector;

protected:
    ext::optional<PropertyOption::vector> _options;
    PropertyOption::used_map              _optionsUsed;

protected:
    BuildSettings();

public:
    virtual ~BuildSettings();

public:
    inline SpecificationType type() const override
    { return BuildSettings::Type(); }

public:
    inline BuildSettings::shared_ptr base() const
    { return std::static_pointer_cast<BuildSettings>(Specification::base()); }

public:
    inline ext::optional<PropertyOption::vector> const &options() const
    { return _options; }

protected:
    friend class Specification;
    bool parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

protected:
    bool inherit(Specification::shared_ptr const &base) override;
    virtual bool inherit(BuildSettings::shared_ptr const &base);

protected:
    static BuildSettings::shared_ptr Parse(Context *context, plist::Dictionary const *dict);

public:
    static inline SpecificationType Type()
    { return SpecificationType::BuildSettings; }
};

} }

#endif  // !__pbxspec_PBX_BuildSettings_h
