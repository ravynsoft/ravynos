/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/SwiftResolver.h>
#include <pbxbuild/Tool/CompilerCommon.h>
#include <pbxbuild/Tool/Environment.h>
#include <pbxbuild/Tool/OptionsResult.h>
#include <pbxbuild/Tool/Tokens.h>
#include <pbxbuild/Tool/Context.h>
#include <pbxsetting/Environment.h>
#include <pbxsetting/Type.h>
#include <xcsdk/SDK/Platform.h>
#include <xcsdk/SDK/Target.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Format/JSON.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>

namespace Tool = pbxbuild::Tool;
using libutil::Filesystem;
using libutil::FSUtil;

Tool::SwiftResolver::
SwiftResolver(pbxspec::PBX::Compiler::shared_ptr const &compiler) :
    _compiler(compiler)
{
}

static std::string
SwiftLibraryPath(pbxsetting::Environment const &environment, xcsdk::SDK::Target::shared_ptr const &sdk, std::vector<xcsdk::SDK::Toolchain::shared_ptr> const &toolchains)
{
    std::string path = environment.resolve("SWIFT_LIBRARY_PATH");
    if (!path.empty()) {
        return path;
    }

    /* What platform and library to search for. */
    std::string const &platformName = sdk->platform()->name();
    std::string swiftLibraryName = environment.resolve("SWIFT_STDLIB");

    // TODO(grp): Use a static Swift runtime if appropriate.
    std::string swiftLibraryDirectory = "swift";
    std::string swiftLibrarySuffix = ".dylib";

    /* Look in the toolchains for Swift libraries. */
    std::vector<std::string> swiftLibraryPaths = {
        swiftLibraryDirectory + "/" + platformName + "/" + "lib" + swiftLibraryName + swiftLibrarySuffix,
        swiftLibraryDirectory + "/" +                      "lib" + swiftLibraryName + swiftLibrarySuffix,
                                                           "lib" + swiftLibraryName + swiftLibrarySuffix,
    };

    for (xcsdk::SDK::Toolchain::shared_ptr const &toolchain : toolchains) {
        for (std::string const &subpath : swiftLibraryPaths) {
            /* The Swift library paths are relative to /usr/lib in the toolchain. */
            std::string path = toolchain->path() + "/" + "usr" + "/" + "lib" + "/" + subpath;

            /* If the Swift library exists, return the directory containing it. */
            if (Filesystem::GetDefaultUNSAFE()->exists(path)) {
                return FSUtil::GetDirectoryName(path);
            }
        }
    }

    /* Not found. */
    return std::string();
}

static std::string
SwiftDocPath(std::string const &moduleName, std::string const &modulePath)
{
    /* The .swiftdoc is output alongside the .swiftmodule. */
    return FSUtil::GetDirectoryName(modulePath) + "/" + moduleName + ".swiftdoc";
}

static void
AppendOutputs(
    std::vector<std::string> *args,
    std::vector<std::string> *outputs,
    std::vector<Tool::Invocation::DependencyInfo> *dependencyInfo,
    std::vector<Tool::AuxiliaryFile> *auxiliaryFiles,
    std::string const &outputDirectory,
    std::string const &moduleName,
    std::string const &modulePath,
    std::vector<Tool::Input> const &inputs,
    bool includeBitcode)
{
    std::unique_ptr<plist::Dictionary> outputInfo = plist::Dictionary::New();

    /* Top-level outputs for the entire module. */
    std::string moduleDependencies = outputDirectory + "/" + moduleName + "-master.swiftdeps";

    std::unique_ptr<plist::Dictionary> module = plist::Dictionary::New();
    module->set("swift-dependencies", plist::String::New(moduleDependencies));
    outputInfo->set("", std::move(module));

    for (Tool::Input const &input : inputs) {
        std::string name = FSUtil::GetBaseNameWithoutExtension(input.path());

        /* Add input argument. */
        args->push_back(input.path());

        /* All of the outputs for each file. */
        std::string swiftModule       = outputDirectory + "/" + name + "~partial.swiftmodule";
        std::string object            = outputDirectory + "/" + name + ".o";
        std::string llvmbc            = outputDirectory + "/" + name + ".bc";
        std::string diagnostics       = outputDirectory + "/" + name + ".dia";
        std::string dependencies      = outputDirectory + "/" + name + ".d";
        std::string swiftDependencies = outputDirectory + "/" + name + ".swiftdeps";

        /* All are outputs of the Swift invocation. */
        outputs->push_back(object);
        outputs->push_back(swiftModule);
        if (includeBitcode) {
            outputs->push_back(llvmbc);
        }
        if (false) {
            /* Diagnostics are not always output. */
            outputs->push_back(diagnostics);
        }
        outputs->push_back(dependencies);
        outputs->push_back(swiftDependencies);

        /* Add dependency info for output. */
        auto info = Tool::Invocation::DependencyInfo(dependency::DependencyInfoFormat::Makefile, dependencies);
        dependencyInfo->push_back(info);

        /* Create output map for this file */
        std::unique_ptr<plist::Dictionary> dict = plist::Dictionary::New();
        dict->set("swiftmodule", plist::String::New(swiftModule));
        dict->set("object", plist::String::New(object));
        if (includeBitcode) {
            dict->set("llvm-bc", plist::String::New(llvmbc));
        }
        dict->set("diagnostics", plist::String::New(diagnostics));
        dict->set("dependencies", plist::String::New(dependencies));
        dict->set("swift-dependencies", plist::String::New(swiftDependencies));
        outputInfo->set(input.path(), std::move(dict));
    }

    /* Serialize output map as JSON. */
    auto serialized = plist::Format::JSON::Serialize(outputInfo.get(), plist::Format::JSON::Create());
    if (!serialized.first) {
        fprintf(stderr, "error: %s\n", serialized.second.c_str());
    } else {
        /* Write output map as an auxiliary file. */
        std::string outputInfoPath = outputDirectory + "/" + moduleName + "-OutputFileMap.json";
        auto outputInfoFile = Tool::AuxiliaryFile::Data(outputInfoPath, *serialized.first);
        auxiliaryFiles->push_back(outputInfoFile);

        /* Add output info to the arguments. */
        args->push_back("-output-file-map");
        args->push_back(outputInfoPath);
    }

    /* Tell Swift to output various files. Paths are provided in the output map. */
    args->push_back("-parseable-output");
    args->push_back("-serialize-diagnostics");
    args->push_back("-emit-dependencies");

    /* Emit the resulting module. */
    args->push_back("-emit-module");
    args->push_back("-emit-module-path");
    args->push_back(modulePath);

    /* The module is an output. */
    outputs->push_back(modulePath);
    outputs->push_back(SwiftDocPath(moduleName, modulePath));
}

static void
AppendPathFlags(std::vector<std::string> *args, pbxsetting::Environment const &environment)
{
    /* Note: Swift paths are passed un-concatenated as two arguments. */

    std::vector<std::string> specialIncludes = { environment.resolve("BUILT_PRODUCTS_DIR") };
    Tool::CompilerCommon::AppendCompoundFlags(args, "-I", false, specialIncludes);

    std::vector<std::string> includes = pbxsetting::Type::ParseList(environment.resolve("SWIFT_INCLUDE_PATHS"));
    Tool::CompilerCommon::AppendCompoundFlags(args, "-I", false, includes);

    std::vector<std::string> specialFrameworks = { environment.resolve("BUILT_PRODUCTS_DIR") };
    Tool::CompilerCommon::AppendCompoundFlags(args, "-F", false, specialFrameworks);

    std::vector<std::string> frameworks = pbxsetting::Type::ParseList(environment.resolve("FRAMEWORK_SEARCH_PATHS"));
    Tool::CompilerCommon::AppendCompoundFlags(args, "-F", false, frameworks);
}

static void
AppendCcFlags(std::vector<std::string> *args, pbxsetting::Environment const &environment, Tool::SearchPaths const &searchPaths, Tool::HeadermapInfo const &headermapInfo)
{
    std::vector<std::string> arguments;

    /* Add include paths. */
    Tool::CompilerCommon::AppendIncludePathFlags(&arguments, environment, searchPaths, headermapInfo);

    /* Add definitions. */
    std::vector<std::string> definitions = pbxsetting::Type::ParseList(environment.resolve("GCC_PREPROCESSOR_DEFINITIONS"));
    Tool::CompilerCommon::AppendCompoundFlags(&arguments, "-D", true, definitions);

    /* Add flags prefixed with -Xcc. */
    for (std::string const &arg : arguments) {
        args->push_back("-Xcc");
        args->push_back(arg);
    }
}

static void
AppendObjcHeader(
    std::vector<std::string> *args,
    std::vector<std::string> *outputs,
    pbxsetting::Environment const &environment,
    std::string const &outputDirectory,
    std::string const &headerName,
    std::string const &headerPath)
{
    /* Output the generated header. */
    outputs->push_back(headerPath);

    args->push_back("-emit-objc-header");
    args->push_back("-emit-objc-header-path");
    args->push_back(headerPath);

    /* Load the bridging header. */
    std::string bridgingHeader = environment.resolve("SWIFT_OBJC_BRIDGING_HEADER");
    if (!bridgingHeader.empty()) {
        args->push_back("-import-objc-header");
        args->push_back(bridgingHeader);
    }
}

static void
AppendUnderlyingModule(
    std::vector<std::string> *args,
    std::vector<Tool::AuxiliaryFile> *auxiliaryFiles,
    Tool::Context const *toolContext,
    pbxsetting::Environment const &environment,
    std::string const &headerName)
{
    /* No module map means no underlying module to import. */
    ext::optional<Tool::ModuleMapInfo::Entry> const &moduleMap = toolContext->moduleMapInfo().moduleMap();
    if (!moduleMap) {
        return;
    }

    /*
     * Create a module map to import the underlying module. However, exclude the Swift-generated
     * header here, to avoid having this module try and import its own generated Objective-C structures.
     */
    std::string moduleName = environment.resolve("PRODUCT_MODULE_NAME");

    std::string unextendedModuleMap = "\n";
    unextendedModuleMap += "module " + moduleName + ".__Swift {\n";
    unextendedModuleMap += "  exclude header \"" + headerName + "\"\n";
    unextendedModuleMap += "}\n";

    auto unextendedModuleMapData = std::vector<uint8_t>(unextendedModuleMap.begin(), unextendedModuleMap.end());
    auto unextendedModuleMapChunk = Tool::AuxiliaryFile::Chunk::Data(unextendedModuleMapData);

    std::string unextendedModuleMapPath = environment.resolve("TARGET_TEMP_DIR") + "/" + "unextended-module.modulemap";
    auto unextendedModuleMapFile = Tool::AuxiliaryFile(unextendedModuleMapPath, { moduleMap->contents(), unextendedModuleMapChunk });
    auxiliaryFiles->push_back(unextendedModuleMapFile);

    /*
     * Create a VFS overlay to load the above module map. This replaces the module map in the product,
     * which does not contain the extra Swift rule, and may not even exist yet (depending on ordering).
     */
    std::string unextendedModuleOverlayPath = environment.resolve("TARGET_TEMP_DIR") + "/" + "unextended-module-overlay.yaml";

    std::string unextendedModuleOverlay;
    unextendedModuleOverlay += "{\n";
    unextendedModuleOverlay += "  'version': 0,\n";
    unextendedModuleOverlay += "  'case-sensitive': 'false',\n";
    unextendedModuleOverlay += "  'roots': [{\n";
    unextendedModuleOverlay += "    'type': 'directory',\n";
    unextendedModuleOverlay += "    'name': '" + FSUtil::GetDirectoryName(moduleMap->finalPath()) + "',\n";
    unextendedModuleOverlay += "    'contents': [{\n";
    unextendedModuleOverlay += "      'type': 'file',\n";
    unextendedModuleOverlay += "      'name': '" + FSUtil::GetBaseName(moduleMap->finalPath()) + "',\n";
    unextendedModuleOverlay += "      'external-contents': '" + unextendedModuleMapPath + "',\n";
    unextendedModuleOverlay += "    }]\n";
    unextendedModuleOverlay += "  }]\n";
    unextendedModuleOverlay += "}\n";

    auto unextendedModuleOverlayData = std::vector<uint8_t>(unextendedModuleOverlay.begin(), unextendedModuleOverlay.end());
    auto unextendedModuleOverlayFile = Tool::AuxiliaryFile::Data(unextendedModuleOverlayPath, unextendedModuleOverlayData);
    auxiliaryFiles->push_back(unextendedModuleOverlayFile);

    /*
     * Add flags to import the underlying module.
     */
    args->push_back("-import-underlying-module");
    args->push_back("-Xcc");
    args->push_back("-ivfsoverlay");
    args->push_back("-Xcc");
    args->push_back(unextendedModuleOverlayPath);
}

void Tool::SwiftResolver::
resolve(
    Tool::Context *toolContext,
    pbxsetting::Environment const &baseEnvironment,
    std::vector<Tool::Input> const &inputs,
    std::string const &outputDirectory) const
{
    /*
     * Resolve the tool options.
     */
    Tool::Environment toolEnvironment = Tool::Environment::Create(_compiler, baseEnvironment, toolContext->workingDirectory(), inputs);
    Tool::OptionsResult options = Tool::OptionsResult::Create(toolEnvironment, toolContext->workingDirectory(), nullptr);
    Tool::Tokens::ToolExpansions tokens = Tool::Tokens::ExpandTool(toolEnvironment, options);

    pbxsetting::Environment const &environment = toolEnvironment.environment();

    /*
     * Empty result appended to below.
     */
    std::vector<std::string> outputs;
    std::vector<Tool::Invocation::DependencyInfo> dependencyInfo;
    std::vector<Tool::AuxiliaryFile> auxiliaryFiles;
    std::vector<std::string> arguments = tokens.arguments();

    /*
     * Append standard Swift flags.
     */
    AppendPathFlags(&arguments, environment);

    /* Compile as a library if no main.swift. */
    bool hasMain = false;
    for (Tool::Input const &input : inputs) {
        if (FSUtil::GetBaseName(input.path()) == "main.swift") {
            hasMain = true;
            break;
        }
    }
    if (!hasMain) {
        /* The specification will already pass this flag if SWIFT_LIBRARIES_ONLY is YES. */
        if (!pbxsetting::Type::ParseBoolean(environment.resolve("SWIFT_LIBRARIES_ONLY"))) {
            arguments.push_back("-parse-as-library");
        }
    }

    /* Compile object files. */
    arguments.push_back("-c");

    /* Enable parallelization. */
    bool wholeModuleOptimization = (pbxsetting::Type::ParseBoolean(environment.resolve("SWIFT_WHOLE_MODULE_OPTIMIZATION")) || environment.resolve("SWIFT_OPTIMIZATION_LEVEL") == "-Owholemodule");
    if (!wholeModuleOptimization || !pbxsetting::Type::ParseBoolean(environment.resolve("SWIFT_USE_PARALLEL_WHOLE_MODULE_OPTIMIZATION"))) {
        // TODO(grp): Get the number of parallel build tasks here.
        arguments.push_back("-j8");
    } else {
        // TODO(grp): Get the number of parallel build tasks here.
        arguments.push_back("-num-threads");
        arguments.push_back("8");
    }

    /*
     * Add inputs and outputs to the invocation.
     */
    std::string moduleName = environment.resolve("SWIFT_MODULE_NAME");
    std::string modulePath = outputDirectory + "/" + moduleName + ".swiftmodule";
    bool includeBitcode = pbxsetting::Type::ParseBoolean(environment.resolve("ENABLE_BITCODE"));
    AppendOutputs(&arguments, &outputs, &dependencyInfo, &auxiliaryFiles, outputDirectory, moduleName, modulePath, inputs, includeBitcode);

    /*
     * Add flags for interacting with Objective-C code.
     */
    AppendCcFlags(&arguments, environment, toolContext->searchPaths(), toolContext->headermapInfo());

    /*
     * Add the Objective-C bridging headers.
     */
    std::string headerName = environment.resolve("SWIFT_OBJC_INTERFACE_HEADER_NAME");
    std::string headerPath = outputDirectory + "/" + headerName;
    AppendObjcHeader(&arguments, &outputs, environment, outputDirectory, headerName, headerPath);

    /*
     * Automatically import the underlying Objective-C module if available.
     */
    AppendUnderlyingModule(&arguments, &auxiliaryFiles, toolContext, environment, headerName);

    /* Compiler working directory. */
    arguments.push_back("-Xcc");
    arguments.push_back("-working-directory" + toolContext->workingDirectory());

    /* Log message for the outer compile step. */
    std::string logMessage = "CompileSwiftSources " + environment.resolve("variant") + " " + environment.resolve("arch") + " " + _compiler->identifier();

    /*
     * Add the invocation.
     */
    Tool::Invocation invocation;
    invocation.executable() = Tool::Invocation::Executable::Determine(tokens.executable());
    invocation.arguments() = arguments;
    invocation.environment() = options.environment();
    invocation.workingDirectory() = toolContext->workingDirectory();
    invocation.inputs() = toolEnvironment.inputs(toolContext->workingDirectory());
    invocation.outputs() = outputs;
    invocation.dependencyInfo() = dependencyInfo;
    invocation.logMessage() = logMessage;
    invocation.priority() = toolContext->currentPhaseInvocationPriority();
    toolContext->invocations().push_back(invocation);

    auto variantArchitectureKey = std::make_pair(environment.resolve("variant"), environment.resolve("arch"));
    toolContext->variantArchitectureInvocations()[variantArchitectureKey].push_back(invocation);

    /*
     * Add the auxiliary files.
     */
    toolContext->auxiliaryFiles().insert(toolContext->auxiliaryFiles().end(), auxiliaryFiles.begin(), auxiliaryFiles.end());

    /*
     * Add the Swift module info so the module can be copied.
     */
    auto swiftModuleInfo = Tool::SwiftModuleInfo(
        environment.resolve("arch"),
        moduleName,
        modulePath,
        SwiftDocPath(moduleName, modulePath),
        headerPath,
        pbxsetting::Type::ParseBoolean(environment.resolve("SWIFT_INSTALL_OBJC_HEADER")));
    toolContext->swiftModuleInfo().push_back(swiftModuleInfo);

    /*
     * Record compilation information for linking.
     */
    Tool::CompilationInfo *compilationInfo = &toolContext->compilationInfo();

    /* Default to Clang as a linker. */
    if (compilationInfo->linkerDriver().empty()) {
        compilationInfo->linkerDriver() = "clang";
    }

    // TODO(grp): For multi-arch builds the below flags get added twice.

    /* Add Swift libraries to linker arguments. */
    std::string swiftLibraryPath = SwiftLibraryPath(environment, toolContext->sdk(), toolContext->toolchains());
    if (!swiftLibraryPath.empty()) {
        compilationInfo->linkerArguments().push_back("-L" + swiftLibraryPath);
    } else {
        fprintf(stderr, "warning: unable to find Swift libraries\n");
    }

    /* Add Swift module to the linked result to allow debugging. */
    if (pbxsetting::Type::ParseBoolean(environment.resolve("GCC_GENERATE_DEBUGGING_SYMBOLS"))) {
        compilationInfo->linkerArguments().push_back("-Xlinker");
        compilationInfo->linkerArguments().push_back("-add_ast_path");
        compilationInfo->linkerArguments().push_back("-Xlinker");
        compilationInfo->linkerArguments().push_back(modulePath);
    }
}

std::unique_ptr<Tool::SwiftResolver> Tool::SwiftResolver::
Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains)
{
    pbxspec::PBX::Compiler::shared_ptr swiftTool = specManager->compiler(Tool::SwiftResolver::ToolIdentifier(), specDomains);
    if (swiftTool == nullptr) {
        fprintf(stderr, "warning: could not find asset catalog compiler\n");
        return nullptr;
    }

    return std::unique_ptr<Tool::SwiftResolver>(new Tool::SwiftResolver(swiftTool));
}

