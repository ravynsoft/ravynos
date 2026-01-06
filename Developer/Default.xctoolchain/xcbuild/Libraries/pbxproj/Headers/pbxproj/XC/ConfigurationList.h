/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_XC_ConfigurationList_h
#define __pbxproj_XC_ConfigurationList_h

#include <pbxproj/XC/BuildConfiguration.h>

namespace pbxproj { namespace XC {

class ConfigurationList : public PBX::Object {
public:
    typedef std::shared_ptr <ConfigurationList> shared_ptr;

private:
    BuildConfiguration::vector _buildConfigurations;
    std::string                _defaultConfigurationName;
    bool                       _defaultConfigurationIsVisible;

public:
    ConfigurationList();

public:
    inline BuildConfiguration::vector const &buildConfigurations() const
    { return _buildConfigurations; }
    inline std::string const &defaultConfigurationName() const
    { return _defaultConfigurationName; }
    inline bool defaultConfigurationIsVisible() const
    { return _defaultConfigurationIsVisible; }

protected:
    bool parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

public:
    static inline char const *Isa()
    { return ISA::XCConfigurationList; }
};

} }

#endif  // !__pbxproj_XC_ConfigurationList_h
