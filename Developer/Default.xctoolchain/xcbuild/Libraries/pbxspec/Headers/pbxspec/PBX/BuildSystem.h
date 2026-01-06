/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxspec_PBX_BuildSystem_h
#define __pbxspec_PBX_BuildSystem_h

#include <pbxspec/PBX/Specification.h>
#include <pbxspec/PBX/PropertyOption.h>

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <ext/optional>

namespace pbxsetting { class Level; }

namespace pbxspec { namespace PBX {

class BuildSystem : public Specification {
public:
    typedef std::shared_ptr <BuildSystem> shared_ptr;
    typedef std::vector <shared_ptr> vector;

protected:
    ext::optional<PropertyOption::vector>          _options;
    PropertyOption::used_map                       _optionsUsed;
    ext::optional<PropertyOption::vector>          _properties;
    PropertyOption::used_map                       _propertiesUsed;
    ext::optional<std::unordered_set<std::string>> _deletedProperties;

protected:
    BuildSystem();

public:
    virtual ~BuildSystem();

public:
    inline SpecificationType type() const override
    { return BuildSystem::Type(); }

public:
    inline BuildSystem::shared_ptr base() const
    { return std::static_pointer_cast<BuildSystem>(Specification::base()); }

public:
    inline ext::optional<PropertyOption::vector> const &options() const
    { return _options; }

public:
    inline ext::optional<PropertyOption::vector> const &properties() const
    { return _properties; }

public:
    inline ext::optional<std::unordered_set<std::string>> const &deletedProperties() const
    { return _deletedProperties; }

public:
    pbxsetting::Level defaultSettings(void) const;

protected:
    friend class Specification;
    bool parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

protected:
    bool inherit(Specification::shared_ptr const &base) override;
    virtual bool inherit(BuildSystem::shared_ptr const &base);

protected:
    static BuildSystem::shared_ptr Parse(Context *context, plist::Dictionary const *dict);

public:
    static inline SpecificationType Type()
    { return SpecificationType::BuildSystem; }
};

} }

#endif  // !__pbxspec_PBX_BuildSystem_h
