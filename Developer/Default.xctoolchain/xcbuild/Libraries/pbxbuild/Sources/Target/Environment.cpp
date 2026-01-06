/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Target/Environment.h>
#include <pbxbuild/Build/Context.h>
#include <pbxsetting/Environment.h>
#include <pbxsetting/Level.h>
#include <pbxsetting/Setting.h>
#include <pbxsetting/Type.h>
#include <pbxsetting/XC/Config.h>
#include <libutil/FSUtil.h>
#include <libutil/Filesystem.h>

#include <algorithm>
#include <iterator>
#include <set>

namespace Build = pbxbuild::Build;
namespace Target = pbxbuild::Target;
using pbxbuild::WorkspaceContext;
using libutil::Filesystem;
using libutil::FSUtil;

Target::Environment::
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
    std::unordered_map<pbxproj::PBX::BuildFile::shared_ptr, std::string> const &buildFileDisambiguation) :
    _sdk                     (sdk),
    _toolchains              (toolchains),
    _executablePaths         (executablePaths),
    _buildRules              (buildRules),
    _specDomains             (specDomains),
    _buildSystem             (buildSystem),
    _productType             (productType),
    _packageType             (packageType),
    _environment             (environment),
    _variants                (variants),
    _architectures           (architectures),
    _workingDirectory        (workingDirectory),
    _buildFileDisambiguation (buildFileDisambiguation)
{
}

static std::unordered_map<pbxproj::PBX::BuildFile::shared_ptr, std::string>
BuildFileDisambiguation(pbxproj::PBX::Target::shared_ptr const &target)
{
    std::unordered_multimap<std::string, pbxproj::PBX::BuildFile::shared_ptr> buildFileUnambiguous;
    std::unordered_map<pbxproj::PBX::BuildFile::shared_ptr, std::string> buildFileDisambiguation;

    for (pbxproj::PBX::BuildPhase::shared_ptr const &buildPhase : target->buildPhases()) {
        if (buildPhase->type() != pbxproj::PBX::BuildPhase::Type::Sources) {
            continue;
        }

        for (pbxproj::PBX::BuildFile::shared_ptr const &buildFile : buildPhase->files()) {
            if (buildFile->fileRef() == nullptr) {
                continue;
            }

            std::string name = FSUtil::GetBaseNameWithoutExtension(buildFile->fileRef()->name());

            /* Use a case-insensitive key to detect conflicts. */
            std::string lower;
            std::transform(name.begin(), name.end(), std::back_inserter(lower), ::tolower);

            auto range = buildFileUnambiguous.equal_range(lower);
            if (range.first != buildFileUnambiguous.end()) {
                /* Conflicts with at least one other file, add a disambiguation. */
                buildFileDisambiguation.insert({ buildFile, name + "-" + buildFile->blueprintIdentifier() });

                /* Add disambiguations for all the conflicting files. */
                for (auto it = range.first; it != range.second; ++it) {
                    pbxproj::PBX::BuildFile::shared_ptr const &otherBuildFile = it->second;
                    std::string otherName = FSUtil::GetBaseNameWithoutExtension(otherBuildFile->fileRef()->name());
                    buildFileDisambiguation.insert({ otherBuildFile, otherName + "-" + otherBuildFile->blueprintIdentifier() });
                }
            }
            buildFileUnambiguous.insert({ lower, buildFile });
        }
    }

    return buildFileDisambiguation;
}

static pbxproj::XC::BuildConfiguration::shared_ptr
ConfigurationNamed(pbxproj::XC::ConfigurationList::shared_ptr const &configurationList, std::string const &configuration)
{
    if (configurationList == nullptr) {
        return nullptr;
    }

    for (pbxproj::XC::BuildConfiguration::shared_ptr const &buildConfiguration : configurationList->buildConfigurations()) {
        if (buildConfiguration->name() == configuration) {
            return buildConfiguration;
        }
    }

    return nullptr;
}

static ext::optional<pbxsetting::XC::Config>
ConfigurationFile(WorkspaceContext const &workspaceContext, pbxproj::XC::BuildConfiguration::shared_ptr const &buildConfiguration)
{
    auto it = workspaceContext.configs().find(buildConfiguration);
    if (it != workspaceContext.configs().end()) {
        return it->second;
    }

    return ext::nullopt;
}

static std::vector<std::string>
SDKSpecificationDomains(xcsdk::SDK::Target::shared_ptr const &sdk)
{
    std::string const &platformName = sdk->platform()->name();

    std::vector<std::string> domains;
    domains.push_back(platformName);

    // TODO(grp): Find a better way to get corresponding device platform.
    std::string::size_type simulator = platformName.find("simulator");
    if (simulator != std::string::npos) {
        std::string device = platformName.substr(0, simulator) + "os";
        domains.push_back(device + "-shared");
    } else {
        domains.push_back(platformName + "-shared");
    }

    // TODO(grp): Find a better way to determine what's embedded.
    if (platformName != "macosx") {
        if (simulator != std::string::npos) {
            domains.push_back("embedded-simulator");
        } else {
            domains.push_back("embedded");
        }

        domains.push_back("embedded-shared");
    }

    domains.push_back("default");
    return domains;
}

static pbxsetting::Level
PlatformArchitecturesLevel(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains)
{
    std::vector<pbxsetting::Setting> architectureSettings;
    std::vector<std::string> platformArchitectures;

    pbxspec::PBX::Architecture::vector architectures = specManager->architectures(specDomains);
    for (pbxspec::PBX::Architecture::shared_ptr const &architecture : architectures) {
        ext::optional<pbxsetting::Setting> architectureSetting = architecture->defaultSetting();
        if (architectureSetting) {
            architectureSettings.push_back(*architectureSetting);
        }
        if (!architecture->realArchitectures()) {
            if (std::find(platformArchitectures.begin(), platformArchitectures.end(), architecture->identifier()) == platformArchitectures.end()) {
                platformArchitectures.push_back(architecture->identifier());
            }
        }
    }

    architectureSettings.push_back(pbxsetting::Setting::Create("VALID_ARCHS", pbxsetting::Type::FormatList(platformArchitectures)));

    return pbxsetting::Level(architectureSettings);
}

static pbxsetting::Level
PackageTypeLevel(pbxspec::PBX::PackageType::shared_ptr const &packageType)
{
    std::vector<pbxsetting::Setting> settings = {
        pbxsetting::Setting::Create("PACKAGE_TYPE", packageType->identifier()),
    };

    if (packageType->defaultBuildSettings()) {
        pbxsetting::Level const &packageTypeLevel = *packageType->defaultBuildSettings();
        settings.insert(settings.end(), packageTypeLevel.settings().begin(), packageTypeLevel.settings().end());
    }

    return pbxsetting::Level(settings);
}


static pbxsetting::Level
ProductTypeLevel(pbxspec::PBX::ProductType::shared_ptr const &productType)
{
    std::vector<pbxsetting::Setting> settings = {
        pbxsetting::Setting::Create("PRODUCT_TYPE", productType->identifier()),
    };

    if (productType->defaultBuildProperties()) {
        pbxsetting::Level const &productTypeLevel = *productType->defaultBuildProperties();
        settings.insert(settings.end(), productTypeLevel.settings().begin(), productTypeLevel.settings().end());
    }

    return pbxsetting::Level(settings);
}

static pbxspec::PBX::BuildSystem::shared_ptr
TargetBuildSystem(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains, pbxproj::PBX::Target::shared_ptr const &target)
{
    if (target->type() == pbxproj::PBX::Target::Type::Native) {
        return specManager->buildSystem("com.apple.build-system.native", specDomains);
    } else if (target->type() == pbxproj::PBX::Target::Type::Legacy) {
        return specManager->buildSystem("com.apple.build-system.external", specDomains);
    } else if (target->type() == pbxproj::PBX::Target::Type::Aggregate) {
       return specManager->buildSystem("com.apple.build-system.external", specDomains);
    } else {
        fprintf(stderr, "error: unknown target type\n");
        return nullptr;
    }
}

static std::vector<std::string>
ResolveArchitectures(pbxsetting::Environment const &environment)
{
    std::vector<std::string> archsVector = pbxsetting::Type::ParseList(environment.resolve("ARCHS"));
    std::set<std::string> archs = std::set<std::string>(archsVector.begin(), archsVector.end());
    std::vector<std::string> validArchsVector = pbxsetting::Type::ParseList(environment.resolve("VALID_ARCHS"));
    std::set<std::string> validArchs = std::set<std::string>(validArchsVector.begin(), validArchsVector.end());

    std::vector<std::string> architectures;
    std::set_intersection(archs.begin(), archs.end(), validArchs.begin(), validArchs.end(), std::back_inserter(architectures));
    return architectures;
}

static std::vector<std::string>
ResolveVariants(pbxsetting::Environment const &environment)
{
    return pbxsetting::Type::ParseList(environment.resolve("BUILD_VARIANTS"));
}

static pbxsetting::Level
ArchitecturesVariantsLevel(std::vector<std::string> const &architectures, std::vector<std::string> const &variants)
{
    std::vector<pbxsetting::Setting> settings;

    if (!variants.empty()) {
        settings.push_back(pbxsetting::Setting::Create("CURRENT_VARIANT", variants.front()));
        settings.push_back(pbxsetting::Setting::Create("variant", variants.front()));
    }

    if (!architectures.empty()) {
        settings.push_back(pbxsetting::Setting::Create("CURRENT_ARCH", architectures.front()));
        settings.push_back(pbxsetting::Setting::Create("arch", architectures.front()));
    }

    for (std::string const &variant : variants) {
        pbxsetting::Setting objectFileDir = pbxsetting::Setting::Parse("OBJECT_FILE_DIR_" + variant, "$(OBJECT_FILE_DIR)-" + variant);
        settings.push_back(objectFileDir);

        for (std::string const &arch : architectures) {
            std::string linkFileList = "LINK_FILE_LIST_" + variant + "_" + arch;
            std::string linkFileListPath = "$(OBJECT_FILE_DIR_" + variant + ")/" + arch + "/$(PRODUCT_NAME).LinkFileList";
            settings.push_back(pbxsetting::Setting::Parse(linkFileList, linkFileListPath));
        }
    }

    return pbxsetting::Level(settings);
}

static pbxsetting::Level
ExecutablePathsLevel(std::vector<std::string> const &executablePaths)
{
    std::string path;
    for (std::string const &executablePath : executablePaths) {
        path += executablePath;
        if (&executablePath != &executablePaths.back()) {
            path += ":";
        }
    }

    return pbxsetting::Level({
        pbxsetting::Setting::Create("PATH", path),
    });
}

ext::optional<Target::Environment> Target::Environment::
Create(Build::Environment const &buildEnvironment, Build::Context const &buildContext, pbxproj::PBX::Target::shared_ptr const &target)
{
    /* Use the source root, which could have been modified by project options, rather than the raw project path. */
    std::string workingDirectory = target->project()->sourceRoot();

    xcsdk::SDK::Target::shared_ptr sdk;
    std::vector<std::string> specDomains;
    pbxproj::XC::BuildConfiguration::shared_ptr projectConfiguration;
    pbxproj::XC::BuildConfiguration::shared_ptr targetConfiguration;
    ext::optional<pbxsetting::XC::Config> projectConfigurationFile;
    ext::optional<pbxsetting::XC::Config> targetConfigurationFile;

    {
        /*
         * Create a synthetic build setting environment to determine the SDK to use. The real build
         * setting environment will interleave in SDK build settings, but those aren't available until
         * the target SDK is itself determined.
         */
        pbxsetting::Environment determinationEnvironment = pbxsetting::Environment(buildEnvironment.baseEnvironment());

        /*
         * Add build base settings.
         */
        determinationEnvironment.insertFront(buildContext.baseSettings(), false);

        /*
         * Add project build settings.
         */
        determinationEnvironment.insertFront(target->project()->settings(), false);

        projectConfiguration = ConfigurationNamed(target->project()->buildConfigurationList(), buildContext.configuration());
        if (projectConfiguration == nullptr) {
            fprintf(stderr, "error: unable to find project configuration %s\n", buildContext.configuration().c_str());
            return ext::nullopt;
        }

        projectConfigurationFile = ConfigurationFile(buildContext.workspaceContext(), projectConfiguration);
        if (projectConfigurationFile) {
            determinationEnvironment.insertFront(projectConfigurationFile->level(), false);
        }

        determinationEnvironment.insertFront(projectConfiguration->buildSettings(), false);

        /*
         * Add target build settings.
         */
        determinationEnvironment.insertFront(target->settings(), false);

        targetConfiguration = ConfigurationNamed(target->buildConfigurationList(), buildContext.configuration());
        if (targetConfiguration == nullptr) {
            fprintf(stderr, "error: unable to find target configuration %s\n", buildContext.configuration().c_str());
            return ext::nullopt;
        }

        targetConfigurationFile = ConfigurationFile(buildContext.workspaceContext(), targetConfiguration);
        if (targetConfigurationFile) {
            determinationEnvironment.insertFront(targetConfigurationFile->level(), false);
        }

        determinationEnvironment.insertFront(targetConfiguration->buildSettings(), false);

        /*
         * Add build override settings.
         */
        determinationEnvironment.insertFront(buildContext.actionSettings(), false);
        for (pbxsetting::Level const &level : buildContext.overrideLevels()) {
            determinationEnvironment.insertFront(level, false);
        }

        /*
         * All settings added; determine target SDK.
         */
        std::string sdkroot = determinationEnvironment.resolve("SDKROOT");

        if (sdkroot == "") {
            sdkroot = "macosx.internal";
        }

        sdk = buildEnvironment.sdkManager()->findTarget(sdkroot);
        printf("sdkroot: %s\n", sdkroot.c_str());

        if (sdk == nullptr) {
            fprintf(stderr, "error: unable to find sdkroot %s\n", sdkroot.c_str());
            return ext::nullopt;
        }

        specDomains = SDKSpecificationDomains(sdk);
    }

    pbxspec::PBX::BuildSystem::shared_ptr buildSystem = TargetBuildSystem(buildEnvironment.specManager(), specDomains, target);
    if (buildSystem == nullptr) {
        fprintf(stderr, "error: unable to create build system\n");
        return ext::nullopt;
    }

    pbxspec::PBX::ProductType::shared_ptr productType = nullptr;
    pbxspec::PBX::PackageType::shared_ptr packageType = nullptr;
    if (target->type() == pbxproj::PBX::Target::Type::Native) {
        pbxproj::PBX::NativeTarget::shared_ptr nativeTarget = std::static_pointer_cast<pbxproj::PBX::NativeTarget>(target);

        productType = buildEnvironment.specManager()->productType(nativeTarget->productType(), specDomains);
        if (productType == nullptr) {
            fprintf(stderr, "error: unable to find product type %s\n", nativeTarget->productType().c_str());
            return ext::nullopt;
        }

        // FIXME(grp): Should this always use the first package type?
        if (productType->packageTypes() && !productType->packageTypes()->empty()) {
            packageType = buildEnvironment.specManager()->packageType(productType->packageTypes()->at(0), specDomains);
            if (packageType == nullptr) {
                fprintf(stderr, "error: unable to find package type %s\n", productType->packageTypes()->at(0).c_str());
                return ext::nullopt;
            }
        }
    }

    /*
     * Now we have $(SDKROOT), and can make the real levels.
     */
    pbxsetting::Environment environment = pbxsetting::Environment(buildEnvironment.baseEnvironment());
    environment.insertFront(buildSystem->defaultSettings(), true);
    environment.insertFront(buildContext.baseSettings(), false);
    environment.insertFront(pbxsetting::Level({
        pbxsetting::Setting::Parse("GCC_VERSION", "$(DEFAULT_COMPILER)"),
    }), false);

    if (sdk->platform()->defaultProperties()) {
        environment.insertFront(*sdk->platform()->defaultProperties(), false);
    }
    environment.insertFront(PlatformArchitecturesLevel(buildEnvironment.specManager(), specDomains), false);
    if (sdk->defaultProperties()) {
        environment.insertFront(*sdk->defaultProperties(), false);
    }
    environment.insertFront(sdk->platform()->settings(), false);
    environment.insertFront(sdk->settings(), false);
    if (sdk->customProperties()) {
        environment.insertFront(*sdk->customProperties(), false);
    }
    if (sdk->platform()->overrideProperties()) {
        environment.insertFront(*sdk->platform()->overrideProperties(), false);
    }

    if (packageType != nullptr) {
        environment.insertFront(PackageTypeLevel(packageType), false);
    }
    if (productType != nullptr) {
        environment.insertFront(ProductTypeLevel(productType), false);
    }

    /*
     * Add project build settings.
     */
    environment.insertFront(target->project()->settings(), false);
    if (projectConfigurationFile) {
        environment.insertFront(projectConfigurationFile->level(), false);
    }
    environment.insertFront(projectConfiguration->buildSettings(), false);

    /*
     * Add target build settings.
     */
    environment.insertFront(target->settings(), false);
    if (targetConfigurationFile) {
        environment.insertFront(targetConfigurationFile->level(), false);
    }
    environment.insertFront(targetConfiguration->buildSettings(), false);

    environment.insertFront(buildContext.actionSettings(), false);
    for (pbxsetting::Level const &level : buildContext.overrideLevels()) {
        environment.insertFront(level, false);
    }

    std::vector<std::string> architectures = ResolveArchitectures(environment);
    std::vector<std::string> variants = ResolveVariants(environment);
    environment.insertFront(ArchitecturesVariantsLevel(architectures, variants), false);

    /* Determine toolchains. Must be after the SDK levels are added, so they can be a fallback. */
    std::vector<xcsdk::SDK::Toolchain::shared_ptr> toolchains;
    std::vector<std::string> effectiveToolchainPaths;
    for (std::string const &toolchainName : pbxsetting::Type::ParseList(environment.resolve("TOOLCHAINS"))) {
        if (xcsdk::SDK::Toolchain::shared_ptr toolchain = buildEnvironment.sdkManager()->findToolchain(toolchainName)) {
            // TODO: Apply toolchain override build settings.
            toolchains.push_back(toolchain);
            effectiveToolchainPaths.push_back(toolchain->path());
        }
    }

    environment.insertFront(pbxsetting::Level({
        /* At the target level and below, the SDKROOT changes to always be a SDK path. */
        pbxsetting::Setting::Create("SDKROOT", sdk->path()),
        pbxsetting::Setting::Create("EFFECTIVE_TOOLCHAINS_DIRS", pbxsetting::Type::FormatList(effectiveToolchainPaths)),
    }), false);

    /* Tool search directories. Use the toolchains just discovered. */
    std::shared_ptr<xcsdk::SDK::Manager> const &sdkManager = buildEnvironment.sdkManager();
    std::vector<std::string> executablePaths = sdkManager->executablePaths(sdk->platform(), sdk, toolchains);
    executablePaths.insert(executablePaths.end(), buildEnvironment.baseExecutablePaths().begin(), buildEnvironment.baseExecutablePaths().end());
    environment.insertFront(ExecutablePathsLevel(executablePaths), false);

    auto buildRules = Target::BuildRules::Create(buildEnvironment.specManager(), specDomains, target);
    auto buildFileDisambiguation = BuildFileDisambiguation(target);

    return Environment(
        sdk,
        toolchains,
        executablePaths,
        buildRules,
        specDomains,
        buildSystem,
        productType,
        packageType,
        environment,
        variants,
        architectures,
        workingDirectory,
        buildFileDisambiguation);
}
