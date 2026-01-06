/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Target_Environment_h
#define __pbxbuild_Target_Environment_h

#include <pbxbuild/Base.h>
#include <pbxbuild/Build/Environment.h>
#include <pbxbuild/Target/BuildRules.h>
#include <pbxsetting/XC/Config.h>
#include <xcsdk/SDK/Target.h>
#include <xcsdk/SDK/Toolchain.h>

#include <ext/optional>

namespace pbxbuild {

namespace Build { class Context; }

namespace Target {

/*
 * The resolved data for a specific build of a target. The build settings,
 * variants, architectures, and target SDKs are determined upon creation.
 */
class Environment {
private:
    xcsdk::SDK::Target::shared_ptr                 _sdk;
    std::vector<xcsdk::SDK::Toolchain::shared_ptr> _toolchains;
    std::vector<std::string>                       _executablePaths;

private:
    Target::BuildRules                             _buildRules;
    std::vector<std::string>                       _specDomains;
    pbxspec::PBX::BuildSystem::shared_ptr          _buildSystem;

private:
    pbxspec::PBX::ProductType::shared_ptr          _productType;
    pbxspec::PBX::PackageType::shared_ptr          _packageType;

private:
    pbxsetting::Environment                        _environment;
    std::vector<std::string>                       _variants;
    std::vector<std::string>                       _architectures;

private:
    std::string                                    _workingDirectory;
    std::unordered_map<pbxproj::PBX::BuildFile::shared_ptr, std::string> _buildFileDisambiguation;

public:
    Environment(
        xcsdk::SDK::Target::shared_ptr const &sdk,
        std::vector<xcsdk::SDK::Toolchain::shared_ptr> const &toolchains,
        std::vector<std::string> const &executablePaths,
        Target::BuildRules const &buildRules,
        std::vector<std::string> const &specDomains,
        pbxspec::PBX::BuildSystem::shared_ptr const &buildSystem,
        pbxspec::PBX::ProductType::shared_ptr const &productType,
        pbxspec::PBX::PackageType::shared_ptr const &packageType,
        pbxsetting::Environment const &environment,
        std::vector<std::string> const &variants,
        std::vector<std::string> const &architectures,
        std::string const &workingDirectory,
        std::unordered_map<pbxproj::PBX::BuildFile::shared_ptr, std::string> const &buildFileDisambiguation);

public:
    /*
     * The base SDK used for this target. Determined from the build context
     * as well as what is specified in the target.
     */
    xcsdk::SDK::Target::shared_ptr const &sdk() const
    { return _sdk; }

    /*
     * The toolchains to use for this build. The SDK also has toolchains, but
     * they might have been overridden by the TOOLCHAINS build setting.
     */
    std::vector<xcsdk::SDK::Toolchain::shared_ptr> const &toolchains() const
    { return _toolchains; }

    /*
     * Search paths to look for tools for this target. Derived from the SDK
     * and the toolchains listed above, but cached here for convenience.
     */
    std::vector<std::string> const &executablePaths() const
    { return _executablePaths; }

public:
    /*
     * The build rules applicable to this target.
     */
    Target::BuildRules const &buildRules() const
    { return _buildRules; }

    /*
     * The specification domains for this target.
     */
    std::vector<std::string> const &specDomains() const
    { return _specDomains; }

    /*
     * The base build system used for this target.
     */
    pbxspec::PBX::BuildSystem::shared_ptr const &buildSystem() const
    { return _buildSystem; }

public:
    /*
     * The target's product type, if it has one.
     */
    pbxspec::PBX::ProductType::shared_ptr const &productType() const
    { return _productType; }

    /*
     * The target's package type, if it has one.
     */
    pbxspec::PBX::PackageType::shared_ptr const &packageType() const
    { return _packageType; }

public:
    /*
     * The complete build setting environment for this build of the target.
     */
    pbxsetting::Environment const &environment() const
    { return _environment; }

    /*
     * The variants to build this target for.
     */
    std::vector<std::string> const &variants() const
    { return _variants; }

    /*
     * The architectures to build this target for.
     */
    std::vector<std::string> const &architectures() const
    { return _architectures; }

public:
    /*
     * The working directory the target should be built in.
     */
    std::string const &workingDirectory() const
    { return _workingDirectory; }

    /*
     * Diasmbiguates build files with matching names. If build files in this
     * are output, they should append the string as a suffix to the base name.
     */
    std::unordered_map<pbxproj::PBX::BuildFile::shared_ptr, std::string> const &buildFileDisambiguation() const
    { return _buildFileDisambiguation; }

public:
    /*
     * Create a target environment for a specific build of a target.
     */
    static ext::optional<Environment>
    Create(Build::Environment const &buildEnvironment, Build::Context const &buildContext, pbxproj::PBX::Target::shared_ptr const &target);
};

}
}

#endif // !__pbxbuild_Target_Environment_h
