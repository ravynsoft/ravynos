/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxspec_Manager_h
#define __pbxspec_Manager_h

#include <pbxspec/PBX/Architecture.h>
#include <pbxspec/PBX/BuildPhase.h>
#include <pbxspec/PBX/BuildRule.h>
#include <pbxspec/PBX/BuildSettings.h>
#include <pbxspec/PBX/BuildStep.h>
#include <pbxspec/PBX/BuildSystem.h>
#include <pbxspec/PBX/Compiler.h>
#include <pbxspec/PBX/FileType.h>
#include <pbxspec/PBX/Linker.h>
#include <pbxspec/PBX/PackageType.h>
#include <pbxspec/PBX/ProductType.h>
#include <pbxspec/PBX/Specification.h>
#include <pbxspec/PBX/Tool.h>

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
#include <utility>

namespace libutil { class Filesystem; }

namespace pbxspec {

class Manager {
public:
    typedef std::shared_ptr <Manager> shared_ptr;

private:
    std::unordered_set<std::string>                                                _domains;
    std::map<std::string, std::map<SpecificationType, PBX::Specification::vector>> _specifications;
    PBX::BuildRule::vector                                                         _buildRules;

public:
    Manager();
    ~Manager();

public:
    PBX::Specification::shared_ptr
    specification(SpecificationType type, std::string const &identifier, std::vector<std::string> const &domains) const;
    PBX::Specification::vector
    specifications(SpecificationType type, std::vector<std::string> const &domains) const;

public:
    PBX::Architecture::shared_ptr
    architecture(std::string const &identifier, std::vector<std::string> const &domains) const;
    PBX::Architecture::vector
    architectures(std::vector<std::string> const &domains) const;

public:
    PBX::BuildPhase::shared_ptr
    buildPhase(std::string const &identifier, std::vector<std::string> const &domains) const;
    PBX::BuildPhase::vector
    buildPhases(std::vector<std::string> const &domains) const;

public:
    PBX::BuildSettings::shared_ptr
    buildSettings(std::string const &identifier, std::vector<std::string> const &domains) const;
    PBX::BuildSettings::vector
    buildSettingses(std::vector<std::string> const &domains) const;

public:
    PBX::BuildStep::shared_ptr
    buildStep(std::string const &identifier, std::vector<std::string> const &domains) const;
    PBX::BuildStep::vector
    buildSteps(std::vector<std::string> const &domains) const;

public:
    PBX::BuildSystem::shared_ptr
    buildSystem(std::string const &identifier, std::vector<std::string> const &domains) const;
    PBX::BuildSystem::vector
    buildSystems(std::vector<std::string> const &domains) const;

public:
    PBX::Compiler::shared_ptr
    compiler(std::string const &identifier, std::vector<std::string> const &domains) const;
    PBX::Compiler::vector
    compilers(std::vector<std::string> const &domains) const;

public:
    PBX::FileType::shared_ptr
    fileType(std::string const &identifier, std::vector<std::string> const &domains) const;
    PBX::FileType::vector
    fileTypes(std::vector<std::string> const &domains) const;

public:
    PBX::Linker::shared_ptr
    linker(std::string const &identifier, std::vector<std::string> const &domains) const;
    PBX::Linker::vector
    linkers(std::vector<std::string> const &domains) const;

public:
    PBX::PackageType::shared_ptr
    packageType(std::string const &identifier, std::vector<std::string> const &domains) const;
    PBX::PackageType::vector
    packageTypes(std::vector<std::string> const &domains) const;

public:
    PBX::ProductType::shared_ptr
    productType(std::string const &identifier, std::vector<std::string> const &domains) const;
    PBX::ProductType::vector
    productTypes(std::vector<std::string> const &domains) const;

public:
    PBX::Tool::shared_ptr
    tool(std::string const &identifier, std::vector<std::string> const &domains) const;
    PBX::Tool::vector
    tools(std::vector<std::string> const &domains) const;

public:
    inline PBX::BuildRule::vector buildRules(void) const
    { return _buildRules; }
    PBX::BuildRule::vector synthesizedBuildRules(std::vector<std::string> const &domains) const;

public:
    void registerDomains(libutil::Filesystem const *filesystem, std::vector<std::pair<std::string, std::string>> const &domains);
    bool registerBuildRules(libutil::Filesystem const *filesystem, std::string const &path);

private:
    void addSpecification(PBX::Specification::shared_ptr const &specification);
    bool inheritSpecification(PBX::Specification::shared_ptr const &specification, std::vector<PBX::Specification::shared_ptr>);

private:
    template <typename T>
    typename T::shared_ptr
    findSpecification(std::vector<std::string> const &domains, std::string const &identifier, SpecificationType type = T::Type()) const;
    template <typename T>
    typename T::vector
    findSpecifications(std::vector<std::string> const &domains, SpecificationType type = T::Type()) const;

public:
    static Manager::shared_ptr
    Create(void);

public:
    static std::string const &
    AnyDomain();

public:
    static std::vector<std::pair<std::string, std::string>>
    DefaultDomains(std::string const &developerRoot);
    static std::vector<std::pair<std::string, std::string>>
    PlatformDomains(std::unordered_map<std::string, std::string> const &platform);
    static std::vector<std::pair<std::string, std::string>>
    PlatformDependentDomains(std::string const &developerRoot);

public:
    static std::vector<std::string>
    DeveloperBuildRules(std::string const &developerRoot);
};

}

#endif  // !__pbxspec_Manager_h
