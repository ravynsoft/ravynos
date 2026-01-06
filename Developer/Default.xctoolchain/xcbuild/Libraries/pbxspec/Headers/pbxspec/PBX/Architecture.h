/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxspec_PBX_Architecture_h
#define __pbxspec_PBX_Architecture_h

#include <pbxspec/PBX/Specification.h>
#include <pbxsetting/Setting.h>

#include <memory>
#include <string>
#include <vector>
#include <ext/optional>

namespace pbxspec { namespace PBX {

class Architecture : public Specification {
public:
    typedef std::shared_ptr <Architecture> shared_ptr;
    typedef std::vector <shared_ptr> vector;

protected:
    ext::optional<std::vector<std::string>> _realArchitectures;
    ext::optional<std::string>              _architectureSetting;
    ext::optional<std::string>              _perArchBuildSettingName;
    ext::optional<std::string>              _byteOrder;
    ext::optional<bool>                     _listInEnum;
    ext::optional<int>                      _sortNumber;

protected:
    Architecture();

public:
    virtual ~Architecture();

public:
    inline SpecificationType type() const override
    { return Architecture::Type(); }

public:
    inline Architecture::shared_ptr base() const
    { return std::static_pointer_cast<Architecture>(Specification::base()); }

public:
    inline ext::optional<std::vector<std::string>> const &realArchitectures() const
    { return _realArchitectures; }

public:
    inline ext::optional<std::string> const &architectureSetting() const
    { return _architectureSetting; }
    inline ext::optional<std::string >const &perArchBuildSettingName() const
    { return _perArchBuildSettingName; }

public:
    inline ext::optional<std::string> const &byteOrder() const
    { return _byteOrder; }

public:
    inline bool listInEnum() const
    { return _listInEnum.value_or(false); }
    inline ext::optional<bool> listInEnumOptional() const
    { return _listInEnum; }
    inline ext::optional<int> sortNumber() const
    { return _sortNumber; }

public:
    ext::optional<pbxsetting::Setting> defaultSetting(void) const;

protected:
    friend class Specification;
    bool parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

protected:
    bool inherit(Specification::shared_ptr const &base) override;
    virtual bool inherit(Architecture::shared_ptr const &base);

protected:
    static Architecture::shared_ptr Parse(Context *context, plist::Dictionary const *dict);

public:
    static inline SpecificationType Type()
    { return SpecificationType::Architecture; }
};

} }

#endif  // !__pbxspec_PBX_Architecture_h
