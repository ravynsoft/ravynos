/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Phase/ProductTypeResolver.h>
#include <pbxbuild/Phase/Environment.h>
#include <pbxbuild/Phase/Context.h>
#include <pbxbuild/Target/Environment.h>
#include <pbxbuild/Tool/CopyResolver.h>
#include <pbxbuild/Tool/InfoPlistResolver.h>
#include <pbxbuild/Tool/MakeDirectoryResolver.h>
#include <pbxbuild/Tool/SymlinkResolver.h>
#include <pbxbuild/Tool/TouchResolver.h>
#include <pbxbuild/Tool/ToolResolver.h>
#include <pbxsetting/Environment.h>
#include <pbxsetting/Type.h>
#include <pbxsetting/Value.h>
#include <libutil/FSUtil.h>

namespace Target = pbxbuild::Target;
namespace Phase = pbxbuild::Phase;
namespace Tool = pbxbuild::Tool;
using libutil::FSUtil;

Phase::ProductTypeResolver::
ProductTypeResolver(pbxspec::PBX::ProductType::shared_ptr const &productType) :
    _productType(productType)
{
}

Phase::ProductTypeResolver::
~ProductTypeResolver()
{
}

static bool
StartsWith(std::string const &str, std::string const &prefix)
{
    if (prefix.size() > str.size()) {
        return false;
    }

    return std::equal(prefix.begin(), prefix.end(), str.begin());
}

static std::unordered_set<std::string>
DirectoriesContainingOutputs(std::vector<Tool::Invocation> const &invocations, std::unordered_set<std::string> const &directories)
{
    std::unordered_set<std::string> populatedDirectories;

    // TODO(grp): This could use a trie. Almost like an interview question!
    for (Tool::Invocation const &invocation : invocations) {
        for (std::string const &output : invocation.outputs()) {
            for (std::string const &directory : directories) {
                if (StartsWith(output, directory + "/")) {
                    /* Found an output in this directory. */
                    populatedDirectories.insert(directory);
                }
            }
        }
    }

    return populatedDirectories;
}

static bool
ResolveBundleStructure(Phase::Environment const &phaseEnvironment, Phase::Context *phaseContext)
{
    Target::Environment const &targetEnvironment = phaseEnvironment.targetEnvironment();
    pbxsetting::Environment const &environment = targetEnvironment.environment();

    /*
     * The setting names for directories to create if they will have contents.
     */
    std::unordered_set<std::string> subdirectories = {
        "CONTENTS_FOLDER_PATH",
        "EXECUTABLE_FOLDER_PATH",
        "PUBLIC_HEADERS_FOLDER_PATH",
        "PRIVATE_HEADERS_FOLDER_PATH",
        "UNLOCALIZED_RESOURCES_FOLDER_PATH",
        "PLUGINS_FOLDER_PATH",
    };

    std::string targetBuildDirectory = environment.resolve("TARGET_BUILD_DIR");
    std::string wrapperName = environment.resolve("WRAPPER_NAME");

    /*
     * Resolve the real path for each of the subdirectories.
     */
    std::unordered_set<std::string> possibleDirectories;
    for (std::string const &subdirectory : subdirectories) {
        std::string directory = targetBuildDirectory + "/" + environment.resolve(subdirectory);
        possibleDirectories.insert(directory);
    }

    /*
     * Check which directories contain files from other invocations.
     */
    std::vector<Tool::Invocation> const &invocations = phaseContext->toolContext().invocations();
    std::unordered_set<std::string> populatedDirectories = DirectoriesContainingOutputs(invocations, possibleDirectories);

    /*
     * Create the directories that have contents.
     */
    for (std::string const &directory : populatedDirectories) {
        if (directory == targetBuildDirectory + "/" + wrapperName) {
            // TODO(grp): This output will conflict with the 'Touch', since it's just creating the bundle root.
            continue;
        }

        if (Tool::MakeDirectoryResolver const *mkdirResolver = phaseContext->makeDirectoryResolver(phaseEnvironment)) {
            mkdirResolver->resolve(&phaseContext->toolContext(), directory, true);
        }
    }

    return true;
}

static bool
ResolveFrameworkStructure(Phase::Environment const &phaseEnvironment, Phase::Context *phaseContext)
{
    pbxproj::PBX::Target::shared_ptr const &target = phaseEnvironment.target();
    Target::Environment const &targetEnvironment = phaseEnvironment.targetEnvironment();
    pbxsetting::Environment const &environment = targetEnvironment.environment();

    /* Shallow bundles don't need any contents created. */
    if (pbxsetting::Type::ParseBoolean(environment.resolve("SHALLOW_BUNDLE"))) {
        return true;
    }

    std::string targetBuildDirectory = environment.resolve("TARGET_BUILD_DIR");
    std::string wrapperName = environment.resolve("WRAPPER_NAME");

    /*
     * Define the possible symlinks that might need to be created, and where they should point to.
     */
    enum Symlink {
        PublicHeaders,
        PrivateHeaders,
        XPCServices,
        Resources,
        Plugins,
        Modules,
        InfoPlist,
        Executable,
    };

    /* Note the custom std::hash<int>. Needed as Symlink is an enum; also why Symlink is not an enum class. */
    std::unordered_map<Symlink, pbxsetting::Value, std::hash<int>> symlinkDirectories = {
        { Symlink::PublicHeaders, pbxsetting::Value::Variable("PUBLIC_HEADERS_FOLDER_PATH") },
        { Symlink::PrivateHeaders, pbxsetting::Value::Variable("PRIVATE_HEADERS_FOLDER_PATH") },
        { Symlink::XPCServices, pbxsetting::Value::Variable("XPCSERVICES_FOLDER_PATH") },
        { Symlink::Resources, pbxsetting::Value::Variable("UNLOCALIZED_RESOURCES_FOLDER_PATH") },
        { Symlink::Plugins, pbxsetting::Value::Variable("PLUGINS_FOLDER_PATH") },
        { Symlink::Modules, pbxsetting::Value::Parse("$(CONTENTS_FOLDER_PATH)/Modules") },
        { Symlink::InfoPlist, pbxsetting::Value::Variable("INFOPLIST_PATH") },
        { Symlink::Executable, pbxsetting::Value::Variable("EXECUTABLE_PATH") },
    };

    /*
     * Determine which symlinks are actually needed, based on what's in the target.
     */
    std::unordered_set<Symlink, std::hash<int>> symlinks;

    /* Defines module: needs module symlink. */
    if (pbxsetting::Type::ParseBoolean(environment.resolve("DEFINES_MODULE"))) {
        symlinks.insert(Symlink::Modules);
    }

    /* Has info plist: needs info plist & resources symlinks. */
    std::string infoPlistFile = environment.resolve("INFOPLIST_FILE");
    if (!infoPlistFile.empty()) {
        symlinks.insert(Symlink::InfoPlist);
        symlinks.insert(Symlink::Resources);
    }

    /* Has any outputs in the plugins directory: needs plugins. */
    std::vector<Tool::Invocation> const &invocations = phaseContext->toolContext().invocations();
    std::string pluginsDirectory = targetBuildDirectory + "/" + environment.resolve("PLUGINS_FOLDER_PATH");
    if (!DirectoriesContainingOutputs(invocations, { pluginsDirectory }).empty()) {
        symlinks.insert(Symlink::Plugins);
    }

    /* Build phase contents affect other symlinks. */
    for (pbxproj::PBX::BuildPhase::shared_ptr const &buildPhase : target->buildPhases()) {
        if (buildPhase->type() == pbxproj::PBX::BuildPhase::Type::CopyFiles) {
            auto copyFiles = std::static_pointer_cast<pbxproj::PBX::CopyFilesBuildPhase>(buildPhase);

            /* Has copy files into XPC services: needs XPC services. */
            std::string XPCServices = "XPCServices";
            if (copyFiles->dstSubfolderSpec() == pbxproj::PBX::CopyFilesBuildPhase::kDestinationProducts &&
                copyFiles->dstPath() == pbxsetting::Value::Parse("$(CONTENTS_FOLDER_PATH)/" + XPCServices)) {
                symlinks.insert(Symlink::XPCServices);
            }
        } else if (buildPhase->type() == pbxproj::PBX::BuildPhase::Type::Sources) {
            /* Has sources: needs executable. */
            if (!buildPhase->files().empty()) {
                symlinks.insert(Symlink::Executable);
            }
        } else if (buildPhase->type() == pbxproj::PBX::BuildPhase::Type::Resources) {
            /* Has copy resources: needs resources. */
            if (!buildPhase->files().empty()) {
                symlinks.insert(Symlink::Resources);
            }
        } else if (buildPhase->type() == pbxproj::PBX::BuildPhase::Type::Headers) {
            if (symlinks.find(Symlink::PublicHeaders) == symlinks.end() && symlinks.find(Symlink::PrivateHeaders) == symlinks.end()) {
                for (pbxproj::PBX::BuildFile::shared_ptr const &buildFile : buildPhase->files()) {
                    std::vector<std::string> const &attributes = buildFile->attributes();
                    bool isPublic  = std::find(attributes.begin(), attributes.end(), "Public") != attributes.end();
                    bool isPrivate = std::find(attributes.begin(), attributes.end(), "Private") != attributes.end();

                    /* Has a public header: needs public headers. */
                    if (isPublic) {
                        symlinks.insert(Symlink::PublicHeaders);
                    }

                    /* Has a private header: needs private headers. */
                    if (isPrivate) {
                        symlinks.insert(Symlink::PrivateHeaders);
                    }
                }
            }
        }
    }

    if (Tool::SymlinkResolver const *symlinkResolver = phaseContext->symlinkResolver(phaseEnvironment)) {
        std::string versions = environment.resolve("VERSIONS_FOLDER_PATH");
        std::string currentVersion = environment.resolve("CURRENT_VERSION");
        std::string frameworkVersion = environment.resolve("FRAMEWORK_VERSION");

        std::string frameworkDirectory = targetBuildDirectory + "/" + wrapperName;
        std::string versionsDirectory = targetBuildDirectory + "/" + versions;

        std::string currentVersionDirectory = versionsDirectory + "/" + currentVersion;
        std::string frameworkVersionDirectory = versionsDirectory + "/" + frameworkVersion;

        /*
         * The symlinks to create are now determined. Add the symlinks.
         */
        for (Symlink const &symlink : symlinks) {
            pbxsetting::Value const &value = symlinkDirectories.at(symlink);
            std::string valueDirectory = targetBuildDirectory + "/" + environment.expand(value);

            std::string currentDirectory = currentVersionDirectory + "/" + FSUtil::GetRelativePath(valueDirectory, frameworkVersionDirectory);
            std::string rootDirectory = frameworkDirectory + "/" + FSUtil::GetBaseName(valueDirectory);

            /* Symlink /path/to.framework/Resources -> Versions/Current/Resources. */
            std::string relativeCurrentDirectory = FSUtil::GetRelativePath(currentDirectory, frameworkDirectory);
            symlinkResolver->resolve(&phaseContext->toolContext(), frameworkDirectory, rootDirectory, relativeCurrentDirectory, true);
        }

        /*
         * Symlink the version directory from "Current".
         */
        if (Tool::SymlinkResolver const *symlinkResolver = phaseContext->symlinkResolver(phaseEnvironment)) {
            /* Symlink Versions/Current -> Versions/A. */
            std::string relativeFrameworkVersionDirectory = FSUtil::GetRelativePath(frameworkVersionDirectory, versionsDirectory);
            symlinkResolver->resolve(&phaseContext->toolContext(), versionsDirectory, currentVersionDirectory, relativeFrameworkVersionDirectory, true);
        }
    } else {
        return false;
    }

    return true;
}

bool Phase::ProductTypeResolver::
resolve(Phase::Environment const &phaseEnvironment, Phase::Context *phaseContext) const
{
    Target::Environment const &targetEnvironment = phaseEnvironment.targetEnvironment();
    pbxsetting::Environment const &environment = targetEnvironment.environment();

    /*
     * Create the product structure.
     */
    bool isBundle = false;
    bool isFramework = false;
    for (pbxspec::PBX::ProductType::shared_ptr productType = _productType; productType != nullptr; productType = productType->base()) {
        if (productType->identifier() == "com.apple.product-type.framework") {
            isFramework = true;
        } else if (productType->identifier() == "com.apple.product-type.bundle") {
            isBundle = true;
        }
    }
    if (isBundle) {
        if (!ResolveBundleStructure(phaseEnvironment, phaseContext)) {
            return false;
        }

        if (isFramework) {
            if (!ResolveFrameworkStructure(phaseEnvironment, phaseContext)) {
                return false;
            }
        }
    }

    /*
     * Copy and compose the info plist.
     */
    if (_productType->hasInfoPlist()) {
        /* Note that INFOPLIST_FILE is the input, and INFOPLIST_PATH is the output. */
        std::string infoPlistFile = environment.resolve("INFOPLIST_FILE");
        if (!infoPlistFile.empty()) {
            Tool::Input infoPlistInput = Tool::Input(infoPlistFile, nullptr); // TODO: use property list file type

            if (pbxsetting::Type::ParseBoolean(environment.resolve("INFOPLIST_PREPROCESS"))) {
                if (Tool::ToolResolver const *toolResolver = phaseContext->toolResolver(phaseEnvironment, "com.apple.compilers.cpp")) {
                    std::string infoPlistIntermediate = environment.resolve("TEMP_DIR") + "/" + "Preprocessed-Info.plist";

                    pbxsetting::Level level = pbxsetting::Level({
                        pbxsetting::Setting::Create("CPP_PREPROCESSOR_DEFINITIONS", pbxsetting::Value::Variable("INFOPLIST_PREPROCESSOR_DEFINITIONS")),
                        pbxsetting::Setting::Create("CPP_PREFIX_HEADER", pbxsetting::Value::Variable("INFOPLIST_PREFIX_HEADER")),
                        pbxsetting::Setting::Create("CPP_OTHER_PREPROCESSOR_FLAGS", pbxsetting::Value::Variable("CPP_OTHER_PREPROCESSOR_FLAGS")),
                    });

                    pbxsetting::Environment preprocessEnvironment = pbxsetting::Environment(environment);
                    preprocessEnvironment.insertFront(level, false);

                    toolResolver->resolve(&phaseContext->toolContext(), preprocessEnvironment, { infoPlistInput }, infoPlistIntermediate);

                    /* Use the preprocessed result as the input below. */
                    infoPlistFile = std::move(infoPlistIntermediate);
                } else {
                    fprintf(stderr, "warning: could not find preprocessor tool\n");
                }
            }

            if (Tool::InfoPlistResolver const *infoPlistResolver = phaseContext->infoPlistResolver(phaseEnvironment)) {
                infoPlistResolver->resolve(&phaseContext->toolContext(), environment, infoPlistInput);
            } else {
                fprintf(stderr, "warning: could not find info plist tool\n");
            }
        }
    }

    /*
     * Validate the product; specific checks are in the validation tool.
     */
    if (pbxsetting::Type::ParseBoolean(environment.resolve("VALIDATE_PRODUCT"))) {
        if (_productType->validation() && _productType->validation()->validationToolSpec()) {
            std::string const &validationToolIdentifier = *_productType->validation()->validationToolSpec();
            if (Tool::ToolResolver const *toolResolver = phaseContext->toolResolver(phaseEnvironment, validationToolIdentifier)) {
                // TODO(grp): Run validation tool.
                (void)toolResolver;
            } else {
                fprintf(stderr, "warning: could not find validation tool %s\n", validationToolIdentifier.c_str());
            }
        }
    }

    /*
     * Touch the final product to note the build's ultimate creation time.
     */
    if (_productType->isWrapper()) {
        std::string wrapperPath = environment.resolve("TARGET_BUILD_DIR") + "/" + environment.resolve("WRAPPER_NAME");

        /*
         * Collect all existing tool outputs that end up inside the bundle.
         */
        std::vector<std::string> outputs;
        for (Tool::Invocation const &invocation : phaseContext->toolContext().invocations()) {
            for (std::string const &output : invocation.outputs()) {
                if (output.compare(0, wrapperPath.size(), wrapperPath) == 0) {
                    outputs.push_back(output);
                }
            }
        }

        if (Tool::TouchResolver const *touchResolver = phaseContext->touchResolver(phaseEnvironment)) {
            touchResolver->resolve(&phaseContext->toolContext(), wrapperPath, outputs);
        } else {
            fprintf(stderr, "warning: could not find touch tool\n");
        }
    }

    /*
     * Register with launch services. This is only relevant for OS X.
     */
    if (_productType->identifier() == "com.apple.product-type.application" && targetEnvironment.sdk()->platform()->name() == "macosx") {
        if (Tool::ToolResolver const *launchServicesResolver = phaseContext->toolResolver(phaseEnvironment, "com.apple.build-tasks.ls-register-url")) {
            // TODO(grp): Register with launch services. Note this needs the same dependencies as touch.
            (void)launchServicesResolver;
        } else {
            fprintf(stderr, "warning: could not find register tool\n");
        }
    }

    return true;
}
