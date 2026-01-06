/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <stdlib.h>

#include <pbxspec/Manager.h>
#include <pbxspec/Context.h>
#include <plist/Array.h>
#include <plist/Dictionary.h>
#include <plist/Object.h>
#include <plist/Format/Any.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>

using pbxspec::Manager;
using pbxspec::Context;
using pbxspec::SpecificationType;
using pbxspec::SpecificationTypes;
namespace PBX = pbxspec::PBX;
using libutil::Filesystem;
using libutil::FSUtil;

Manager::
Manager()
{
}

Manager::
~Manager()
{
}

template <typename T>
typename T::vector Manager::
findSpecifications(std::vector<std::string> const &domains, SpecificationType type) const
{
    typename T::vector specifications;

    for (std::string const &domain : domains) {
        if (domain == AnyDomain()) {
            for (auto const &entry : _specifications) {
                auto const &it = entry.second.find(type);
                if (it != entry.second.end()) {
                    for (auto const &s : it->second) {
                        specifications.emplace_back(std::static_pointer_cast<T>(s));
                    }
                }
            }
        } else {
            auto const &doit = _specifications.find(domain);
            if (doit != _specifications.end()) {
                auto const &it = doit->second.find(type);
                if (it != doit->second.end()) {
                    for (auto const &s : it->second) {
                        specifications.emplace_back(std::static_pointer_cast<T>(s));
                    }
                }
            }
        }
    }

    return specifications;
}

template <typename T>
typename T::shared_ptr Manager::
findSpecification(std::vector<std::string> const &domains, std::string const &identifier, SpecificationType type) const
{
    typename T::vector vector = findSpecifications <T> (domains, type);

    auto I = std::find_if(vector.begin(), vector.end(), [&identifier](PBX::Specification::shared_ptr const &spec) -> bool {
        return identifier == spec->identifier();
    });

    if (I != vector.end()) {
        return *I;
    }

    return nullptr;
}

PBX::Specification::shared_ptr Manager::
specification(SpecificationType type, std::string const &identifier, std::vector<std::string> const &domains) const
{
    return findSpecification <PBX::Specification> (domains, identifier, type);
}

PBX::Specification::vector Manager::
specifications(SpecificationType type, std::vector<std::string> const &domains) const
{
    return findSpecifications <PBX::Specification> (domains, type);
}

PBX::Architecture::shared_ptr Manager::
architecture(std::string const &identifier, std::vector<std::string> const &domains) const
{
    return findSpecification <PBX::Architecture> (domains, identifier);
}

PBX::Architecture::vector Manager::
architectures(std::vector<std::string> const &domains) const
{
    return findSpecifications <PBX::Architecture> (domains);
}

PBX::BuildPhase::shared_ptr Manager::
buildPhase(std::string const &identifier, std::vector<std::string> const &domains) const
{
    return findSpecification <PBX::BuildPhase> (domains, identifier);
}

PBX::BuildPhase::vector Manager::
buildPhases(std::vector<std::string> const &domains) const
{
    return findSpecifications <PBX::BuildPhase> (domains);
}

PBX::BuildSettings::shared_ptr Manager::
buildSettings(std::string const &identifier, std::vector<std::string> const &domains) const
{
    return findSpecification <PBX::BuildSettings> (domains, identifier);
}

PBX::BuildSettings::vector Manager::
buildSettingses(std::vector<std::string> const &domains) const
{
    return findSpecifications <PBX::BuildSettings> (domains);
}

PBX::BuildStep::shared_ptr Manager::
buildStep(std::string const &identifier, std::vector<std::string> const &domains) const
{
    return findSpecification <PBX::BuildStep> (domains, identifier);
}

PBX::BuildStep::vector Manager::
buildSteps(std::vector<std::string> const &domains) const
{
    return findSpecifications <PBX::BuildStep> (domains);
}

PBX::BuildSystem::shared_ptr Manager::
buildSystem(std::string const &identifier, std::vector<std::string> const &domains) const
{
    return findSpecification <PBX::BuildSystem> (domains, identifier);
}

PBX::BuildSystem::vector Manager::
buildSystems(std::vector<std::string> const &domains) const
{
    return findSpecifications <PBX::BuildSystem> (domains);
}

PBX::Compiler::shared_ptr Manager::
compiler(std::string const &identifier, std::vector<std::string> const &domains) const
{
    return findSpecification <PBX::Compiler> (domains, identifier);
}

PBX::Compiler::vector Manager::
compilers(std::vector<std::string> const &domains) const
{
    return findSpecifications <PBX::Compiler> (domains);
}

PBX::FileType::shared_ptr Manager::
fileType(std::string const &identifier, std::vector<std::string> const &domains) const
{
    return findSpecification <PBX::FileType> (domains, identifier);
}

PBX::FileType::vector Manager::
fileTypes(std::vector<std::string> const &domains) const
{
    return findSpecifications <PBX::FileType> (domains);
}

PBX::Linker::shared_ptr Manager::
linker(std::string const &identifier, std::vector<std::string> const &domains) const
{
    return findSpecification <PBX::Linker> (domains, identifier);
}

PBX::Linker::vector Manager::
linkers(std::vector<std::string> const &domains) const
{
    return findSpecifications <PBX::Linker> (domains);
}

PBX::PackageType::shared_ptr Manager::
packageType(std::string const &identifier, std::vector<std::string> const &domains) const
{
    return findSpecification <PBX::PackageType> (domains, identifier);
}

PBX::PackageType::vector Manager::
packageTypes(std::vector<std::string> const &domains) const
{
    return findSpecifications <PBX::PackageType> (domains);
}

PBX::ProductType::shared_ptr Manager::
productType(std::string const &identifier, std::vector<std::string> const &domains) const
{
    return findSpecification <PBX::ProductType> (domains, identifier);
}

PBX::ProductType::vector Manager::
productTypes(std::vector<std::string> const &domains) const
{
    return findSpecifications <PBX::ProductType> (domains);
}

PBX::Tool::shared_ptr Manager::
tool(std::string const &identifier, std::vector<std::string> const &domains) const
{
    return findSpecification <PBX::Tool> (domains, identifier);
}

PBX::Tool::vector Manager::
tools(std::vector<std::string> const &domains) const
{
    return findSpecifications <PBX::Tool> (domains);
}

PBX::BuildRule::vector Manager::
synthesizedBuildRules(std::vector<std::string> const &domains) const
{
    PBX::BuildRule::vector buildRules;

    for (PBX::Compiler::shared_ptr const &compiler : compilers(domains)) {
        if (compiler->synthesizeBuildRule() && compiler->inputFileTypes()) {
            PBX::BuildRule::shared_ptr buildRule = std::make_shared <PBX::BuildRule> (PBX::BuildRule(*compiler->inputFileTypes(), compiler->identifier()));
            buildRules.push_back(buildRule);
        }
    }

    for (PBX::Linker::shared_ptr const &linker : linkers(domains)) {
        if (linker->synthesizeBuildRule() && linker->inputFileTypes()) {
            PBX::BuildRule::shared_ptr buildRule = std::make_shared <PBX::BuildRule> (PBX::BuildRule(*linker->inputFileTypes(), linker->identifier()));
            buildRules.push_back(buildRule);
        }
    }

    for (PBX::Tool::shared_ptr const &tool : tools(domains)) {
        if (tool->synthesizeBuildRule() && tool->inputFileTypes()) {
            PBX::BuildRule::shared_ptr buildRule = std::make_shared <PBX::BuildRule> (PBX::BuildRule(*tool->inputFileTypes(), tool->identifier()));
            buildRules.push_back(buildRule);
        }
    }

    return buildRules;
}

void Manager::
addSpecification(PBX::Specification::shared_ptr const &spec)
{
    if (spec == nullptr) {
        fprintf(stderr, "error: registering null specification\n");
        return;
    }

    if (auto ospec = specification(spec->type(), spec->identifier(), { spec->domain() })) {
        /*
         * Workaround for a typo in the default specifications; don't warn.
         */
        if (spec->identifier() == "PlatformStandardUniversal") {
            return;
        }

        fprintf(stderr, "error: registering %s specification '%s' in domain %s twice\n",
                SpecificationTypes::Name(spec->type()).c_str(), spec->identifier().c_str(), spec->domain().c_str());
        return;
    }

#if 0
    fprintf(stderr, "adding %s spec to domain %s '%s'\n",
            spec->type(), spec->domain().c_str(), spec->identifier().c_str());
#endif
    _specifications[spec->domain()][spec->type()].push_back(spec);
}

bool Manager::
inheritSpecification(PBX::Specification::shared_ptr const &specification, std::vector<PBX::Specification::shared_ptr> visited)
{
    if (specification->basedOnIdentifier() && specification->basedOnDomain() && specification->base() == nullptr) {
        /* Find the base specification. */
        PBX::Specification::shared_ptr base = this->specification(specification->type(), *specification->basedOnIdentifier(), { *specification->basedOnDomain() });

        /*
         * If searching the specified domain didn't find anything, search all
         * domains.
         * Some specifications inherit from domain/identifier pairs that don't
         * exist in practice, but by using any domain they can successfully
         * inherit from something potentially relevant.
         */
        if (!base) {
            PBX::Specification::vector candidates = this->specifications(specification->type(), { *specification->basedOnIdentifier() });

            /*
             * Make sure that we're not trying to inherit from ourselves. This
             * can happen if foo:identifier inherits from bar:identifer, and
             * bar:identifier is not found; we'd would search for
             * <any>:identifier and potentially find foo:identifier again.
             */
            auto I = std::find_if(candidates.begin(), candidates.end(),
                                  [&specification, &visited](PBX::Specification::shared_ptr const &spec) -> bool {
                                      for (auto duplicate : visited) {
                                          // check if specification exists in current inheritance chain.
                                          if (specification == duplicate) {
                                              return false;
                                          }
                                          // check if base candidate already inherited from specification in question.
                                          auto base = duplicate->base();
                                          while (base) {
                                              if (specification == base) {
                                                  return false;
                                              }
                                              base = base->base();
                                          }
                                      }
                                      return true;
                                  });
            if (I != candidates.end()) {
                base = *I;
            }
        }

        /* Find the base specification. */
        if (base == nullptr) {
            fprintf(stderr, "error: cannot find base %s specification '%s:%s'\n", SpecificationTypes::Name(specification->type()).c_str(), specification->basedOnDomain()->c_str(), specification->basedOnIdentifier()->c_str());
            return false;
        }

#if 0
        fprintf(stderr, "debug: inheriting %s:%s (%s) from %s:%s (%s)\n", specification->domain().c_str(), specification->identifier().c_str(), SpecificationTypes::Name(specification->type()).c_str(), base->domain().c_str(), base->identifier().c_str(), SpecificationTypes::Name(base->type()).c_str());
#endif

        /*
         * Ensure the base specification itself has been inherited. Since inheritance
         * copies values from the base, the base must have its own values set first.
         */
        visited.push_back(base);
        bool success = inheritSpecification(base, visited);
        visited.pop_back();
        if (!success) {
            return false;
        }

        /* Perform inheritance. */
        if (!specification->inherit(base)) {
            fprintf(stderr, "error: could not inherit from base %s specification '%s:%s'\n", SpecificationTypes::Name(specification->type()).c_str(), specification->basedOnDomain()->c_str(), specification->basedOnIdentifier()->c_str());
            return false;
        }
    }

    return true;
}

void Manager::
registerDomains(Filesystem const *filesystem, std::vector<std::pair<std::string, std::string>> const &domains)
{
    PBX::Specification::vector specifications;

    for (auto const &domain : domains) {
        /*
         * Avoid double domain registration. Unncessary and causes warnings.
         */
        if (_domains.find(domain.first) != _domains.end()) {
            continue;
        }

        Context context;
        context.domain = domain.first;

        std::string realPath = filesystem->resolvePath(domain.second);
        if (realPath.empty()) {
            continue;
        }

        ext::optional<Filesystem::Type> type = filesystem->type(realPath);
        if (!type) {
            continue;
        }

        switch (*type) {
            case Filesystem::Type::Directory: {
                filesystem->readDirectory(realPath, true, [&](std::string const &filename) -> bool {
                    std::string path = realPath + "/" + filename;

                    /* Support both *.xcspec and *.pbfilespec as a few of the latter remain in use. */
                    if (FSUtil::GetFileExtension(path) != "xcspec" && FSUtil::GetFileExtension(path) != "pbfilespec") {
                        return true;
                    }

                    /* For *.pbfilespec files, default to FileType specifications. */
                    ext::optional<SpecificationType> defaultType;
                    if (FSUtil::GetFileExtension(path) == "pbfilespec") {
                        defaultType = SpecificationType::FileType;
                    }

                    if (filesystem->type(path) != Filesystem::Type::Directory) {
#if 0
                        fprintf(stderr, "importing specification '%s'\n", path.c_str());
#endif

                        ext::optional<PBX::Specification::vector> fileSpecifications = PBX::Specification::Open(filesystem, &context, path, defaultType);
                        if (fileSpecifications) {
                            specifications.insert(specifications.end(), fileSpecifications->begin(), fileSpecifications->end());
                        } else {
                            fprintf(stderr, "warning: failed to import specification '%s'\n", path.c_str());
                        }
                    }
                    return true;
                });
                break;
            }
            case Filesystem::Type::SymbolicLink:
            case Filesystem::Type::File: {
#if 0
                fprintf(stderr, "importing specification '%s'\n", realPath.c_str());
#endif
                ext::optional<PBX::Specification::vector> fileSpecifications = PBX::Specification::Open(filesystem, &context, realPath);
                if (fileSpecifications) {
                    specifications.insert(specifications.end(), fileSpecifications->begin(), fileSpecifications->end());
                } else {
                    fprintf(stderr, "warning: failed to import specification '%s'\n", realPath.c_str());
                }
                break;
            }
        }
    }

    /*
     * Mark all of the domains regsitered. This is after all of the inputs so the
     * same domain can be registered multiple times (loaded from multiple paths)
     * in a single registration, but can't be registered twice outside that.
     */
    for (auto const &domain : domains) {
        _domains.insert(domain.first);
    }

    /*
     * Register all specifications. Must be before inheritance in case specifications
     * inherit from other specifications also being registered at the same time.
     */
    for (PBX::Specification::shared_ptr const &specification : specifications) {
        addSpecification(specification);
    }

    /*
     * Inherit from existing and newly added specifications.
     */
    for (PBX::Specification::shared_ptr const &specification : specifications) {
        std::vector<PBX::Specification::shared_ptr> visited { specification };
        if (!inheritSpecification(specification, visited)) {
            /* Unfortunately, not much useful error handling to do here. */
            continue;
        }
    }
}

bool Manager::
registerBuildRules(Filesystem const *filesystem, std::string const &path)
{
    std::vector<uint8_t> contents;
    if (!filesystem->read(&contents, path)) {
        return false;
    }

    std::unique_ptr<plist::Object> plist = plist::Format::Any::Deserialize(contents).first;
    if (plist == nullptr) {
        return false;
    }

    if (auto array = plist::CastTo <plist::Array> (plist.get())) {
        size_t count  = array->count();
        for (size_t n = 0; n < count; n++) {
            if (auto dict = array->value <plist::Dictionary> (n)) {
                PBX::BuildRule::shared_ptr buildRule = std::make_shared <PBX::BuildRule> (PBX::BuildRule());
                if (buildRule->parse(dict)) {
                    _buildRules.push_back(buildRule);
                }
            }
        }
    }

    return true;
}

Manager::shared_ptr Manager::
Create(void)
{
    return std::make_shared <Manager> ();
}

std::string const &Manager::
AnyDomain()
{
    static std::string any = "<<domain>>";
    return any;
}

std::vector<std::pair<std::string, std::string>> Manager::
DefaultDomains(std::string const &developerRoot)
{
    std::string root       = developerRoot + "/../PlugIns/Xcode3Core.ideplugin";
    std::string frameworks = root + "/Contents/Frameworks";
    std::string plugins    = root + "/Contents/SharedSupport/Developer/Library/Xcode/Plug-ins";
    std::string embedded   = developerRoot + "/../PlugIns/IDEiOSSupportCore.ideplugin/Contents/Resources";

    char *darlingC = getenv("RUNTIME_SPEC_PATH");
    std::string darling = "";
    if (darlingC == NULL) {
        fprintf(stderr, "RUNTIME_SPEC_PATH not set, using /\n");
    } else {
        darling = std::string(darlingC);
    }

    printf("%s, %s\n", developerRoot.c_str(), (darling + "/" + "BuildSystem").c_str());

    return {
        { "default", developerRoot + "/Library/Xcode/Specifications" },
        { "default", frameworks + "/" + "DevToolsCore.framework" },
        { "default", plugins + "/" + "Clang LLVM 1.0.xcplugin" },
        { "default", plugins + "/" + "Core Data.xcplugin" },
        { "default", plugins + "/" + "CoreBuildTasks.xcplugin" },
        { "default", plugins + "/" + "IBCompilerPlugin.xcplugin" },
        { "default", plugins + "/" + "Metal.xcplugin" },
        { "default", plugins + "/" + "SceneKit.xcplugin" },
        { "default", plugins + "/" + "SpriteKit.xcplugin" },
        { "default", plugins + "/" + "XCLanguageSupport.xcplugin" },
        { "default", darling + "/" + "FileType" },
        { "default", darling + "/" + "Tool" },
        { "default", darling + "/" + "BuildPhase" },
        { "default", darling + "/" + "BuildSystem" },
        { "default", darling + "/" + "Compiler" },
        { "default", darling + "/" + "Linker" },
        { "embedded-shared", embedded + "/" + "Embedded-Shared.xcspec" },
        { "embedded", embedded + "/" + "Embedded-Device.xcspec" },
        { "embedded-simulator", embedded + "/" + "Embedded-Simulator.xcspec" },
    };
}

std::vector<std::pair<std::string, std::string>> Manager::
PlatformDomains(std::unordered_map<std::string, std::string> const &platforms)
{
    std::vector<std::pair<std::string, std::string>> domains;

    auto iphoneos = platforms.find("iphoneos");
    if (iphoneos != platforms.end()) {
        std::string root = iphoneos->second + "/Developer/Library/Xcode/PrivatePlugIns/IDEiOSPlatformSupportCore.ideplugin/Contents/Resources";
        domains.push_back({ "iphoneos-shared", root + "/" + "Shared.xcspec" });
        domains.push_back({ "iphoneos", root + "/" + "Device.xcspec" });
        domains.push_back({ "iphonesimulator", root + "/" + "Simulator.xcspec" });
    }

    auto appletvos = platforms.find("appletvos");
    if (appletvos != platforms.end()) {
        std::string root = appletvos->second + "/Developer/Library/Xcode/PrivatePlugIns/IDEAppleTVSupportCore.ideplugin/Contents/Resources";
        domains.push_back({ "appletvos-shared", root + "/" + "Shared.xcspec" });
        domains.push_back({ "appletvos", root + "/" + "Device.xcspec" });
        domains.push_back({ "appletvsimulator", root + "/" + "Simulator.xcspec" });
    }

    auto watchos = platforms.find("watchos");
    if (watchos != platforms.end()) {
        std::string root = watchos->second + "/Developer/Library/Xcode/PrivatePlugIns/IDEWatchSupportCore.ideplugin/Contents/Resources";
        domains.push_back({ "watchos-shared", root + "/" + "Shared.xcspec" });
        domains.push_back({ "watchos", root + "/" + "Device.xcspec" });
        domains.push_back({ "watchsimulator", root + "/" + "Simulator.xcspec" });
    }

    /* The standard platform specifications directory. */
    for (auto const &platform : platforms) {
        domains.push_back({ platform.first, platform.second + "/Developer/Library/Xcode/Specifications" });
    }

    return domains;
}

std::vector<std::pair<std::string, std::string>> Manager::
PlatformDependentDomains(std::string const &developerRoot)
{
    std::vector<std::pair<std::string, std::string>> domains;

    std::string path = developerRoot + "/../PlugIns/Xcode3Core.ideplugin/Contents/SharedSupport/Developer/Library/Xcode/Plug-ins/XCWatchKit1Support.xcplugin";
    domains.push_back({ "default", path });

    return domains;
}

std::vector<std::string> Manager::
DeveloperBuildRules(std::string const &developerRoot)
{
    char *darlingC = getenv("RUNTIME_SPEC_PATH");
    std::string darling = "";
    if (darlingC == NULL) {
        fprintf(stderr, "RUNTIME_SPEC_PATH not set, using /\n");
    } else {
        darling = std::string(darlingC);
    }

    return {
        developerRoot + "/../PlugIns/Xcode3Core.ideplugin/Contents/Frameworks/DevToolsCore.framework/Resources/BuiltInBuildRules.plist",
        developerRoot + "/Library/Xcode/Specifications/BuiltInBuildRules.plist",
        darling + "/BuildRules/BuiltInBuildRules.plist",
    };
}
