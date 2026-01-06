/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <builtin/infoPlistUtility/Driver.h>
#include <builtin/infoPlistUtility/Options.h>
#include <pbxsetting/Environment.h>
#include <pbxsetting/Setting.h>
#include <pbxsetting/Type.h>
#include <pbxsetting/Value.h>
#include <plist/Array.h>
#include <plist/Dictionary.h>
#include <plist/Integer.h>
#include <plist/Object.h>
#include <plist/String.h>
#include <plist/Format/Any.h>
#include <plist/Format/ASCII.h>
#include <plist/Format/Binary.h>
#include <plist/Format/XML.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>
#include <process/Context.h>

using builtin::infoPlistUtility::Driver;
using builtin::infoPlistUtility::Options;
using libutil::Filesystem;
using libutil::FSUtil;

Driver::
Driver()
{
}

Driver::
~Driver()
{
}

std::string Driver::
name()
{
    return "builtin-infoPlistUtility";
}

static std::pair<bool, std::string>
WritePkgInfo(Filesystem *filesystem, plist::Dictionary const *root, std::string const &path)
{
    std::string pkgInfo;

    plist::String const *packageType = root->value<plist::String>("CFBundlePackageType");
    if (packageType != nullptr && packageType->value().size() == 4) {
        pkgInfo += packageType->value();
    } else {
        pkgInfo += "????";
    }

    plist::String const *signature = root->value<plist::String>("CFBundleSignature");
    if (signature != nullptr && signature->value().size() == 4) {
        pkgInfo += signature->value();
    } else {
        pkgInfo += "????";
    }

    auto pkgInfoContents = std::vector<uint8_t>(pkgInfo.begin(), pkgInfo.end());
    if (!filesystem->write(pkgInfoContents, path)) {
        return std::make_pair(false, "could write to " + path);
    }

    return std::make_pair(true, std::string());
}

static void
ExpandBuildSettings(plist::Object *value, pbxsetting::Environment const &environment)
{
    /*
     * Recursively expand any strings in the plist. Dictionary keys are not expanded.
     */
    if (auto dictionary = plist::CastTo<plist::Dictionary>(value)) {
        for (size_t n = 0; n < dictionary->count(); n++) {
            ExpandBuildSettings(dictionary->value(n), environment);
        }
    } else if (auto array = plist::CastTo<plist::Array>(value)) {
        for (size_t n = 0; n < array->count(); n++) {
            ExpandBuildSettings(array->value(n), environment);
        }
    } else if (auto string = plist::CastTo<plist::String>(value)) {
        pbxsetting::Value parsed = pbxsetting::Value::Parse(string->value());
        string->setValue(environment.expand(parsed));
    }
}

static void
AddBuildEnvironment(plist::Dictionary *root, pbxsetting::Environment const &environment)
{
    // TODO: This should use `xcsdk::SDK::Platform::additionalInfo()`.

    root->set("DTCompiler", plist::String::New(environment.resolve("DEFAULT_COMPILER")));
    root->set("DTXcode", plist::String::New(environment.resolve("XCODE_VERSION_ACTUAL")));
    root->set("DTXcodeBuild", plist::String::New(environment.resolve("XCODE_PRODUCT_BUILD_VERSION")));
    root->set("BuildMachineOSBuild", plist::String::New(environment.resolve("MAC_OS_X_PRODUCT_BUILD_VERSION")));

    root->set("DTPlatformName", plist::String::New(environment.resolve("PLATFORM_NAME")));
    root->set("DTPlatformBuild", plist::String::New(environment.resolve("PLATFORM_PRODUCT_BUILD_VERSION")));
    root->set("DTPlatformVersion", plist::String::New("")); // TODO(grp): Not available through build settings.

    root->set("DTSDKName", plist::String::New(environment.resolve("SDK_NAME")));
    root->set("DTSDKBuild", plist::String::New(environment.resolve("SDK_PRODUCT_BUILD_VERSION")));

    std::string deploymentTarget = environment.resolve(environment.resolve("DEPLOYMENT_TARGET_SETTING_NAME"));
    if (!deploymentTarget.empty()) {
        root->set("MinimumOSVersion", plist::String::New(deploymentTarget));
    }

    std::string targetedDeviceFamily = environment.resolve("TARGETED_DEVICE_FAMILY");
    if (!targetedDeviceFamily.empty()) {
        std::unique_ptr<plist::Array> deviceFamily = plist::Array::New();

        std::string::size_type off = 0;
        do {
            std::string::size_type noff = targetedDeviceFamily.find(',', off);
            std::string entry = (noff == std::string::npos ? targetedDeviceFamily.substr(off) : targetedDeviceFamily.substr(off, noff));

            if (!entry.empty()) {
                int64_t value = pbxsetting::Type::ParseInteger(entry);
                deviceFamily->append(plist::Integer::New(value));
            }

            off = noff;
        } while ((off != std::string::npos) && (off++ < targetedDeviceFamily.size()));

        std::unique_ptr<plist::Object> deviceFamilyObject = std::unique_ptr<plist::Object>(deviceFamily.release());
        root->set("UIDeviceFamily", std::move(deviceFamilyObject));
    }
}

static pbxsetting::Environment
CreateBuildEnvironment(std::unordered_map<std::string, std::string> const &environment)
{
    std::vector<pbxsetting::Setting> settings;
    for (auto const &pair : environment) {
        pbxsetting::Setting setting = pbxsetting::Setting::Create(pair.first, pair.second);
        settings.push_back(setting);
    }

    pbxsetting::Level level = pbxsetting::Level(settings);
    pbxsetting::Environment settingsEnvironment = pbxsetting::Environment();
    settingsEnvironment.insertFront(level, false);

    return settingsEnvironment;
}

int Driver::
run(process::Context const *processContext, libutil::Filesystem *filesystem)
{
    Options options;
    std::pair<bool, std::string> result = libutil::Options::Parse<Options>(&options, processContext->commandLineArguments());
    if (!result.first) {
        fprintf(stderr, "error: %s\n", result.second.c_str());
        return 1;
    }

    /* Validate options. */
    if (!options.input()) {
        fprintf(stderr, "error: no input file specified\n");
        return 1;
    }

    if (!options.output()) {
        fprintf(stderr, "error: no output file specified\n");
        return 1;
    }

    pbxsetting::Environment settingsEnvironment = CreateBuildEnvironment(processContext->environmentVariables());

    /* Read in the input. */
    std::vector<uint8_t> inputContents;
    if (!filesystem->read(&inputContents, FSUtil::ResolveRelativePath(*options.input(), processContext->currentDirectory()))) {
        fprintf(stderr, "error: unable to read input %s\n", options.input()->c_str());
        return 1;
    }

    /* Determine the input format. */
    std::unique_ptr<plist::Format::Any> inputFormat = plist::Format::Any::Identify(inputContents);
    if (inputFormat == nullptr) {
        fprintf(stderr, "error: input %s is not a plist\n", options.input()->c_str());
        return 1;
    }

    /* Deserialize the input. */
    auto deserialize = plist::Format::Any::Deserialize(inputContents, *inputFormat);
    if (!deserialize.first) {
        fprintf(stderr, "error: %s: %s\n", options.input()->c_str(), deserialize.second.c_str());
        return 1;
    }

    plist::Dictionary *root = plist::CastTo<plist::Dictionary>(deserialize.first.get());
    if (root == nullptr) {
        fprintf(stderr, "error: info plist root is not a dictionary\n");
        return 1;
    }

    /*
     * Expand all build settings in the plist. Build settings can be in dictionary keys
     * and strings. The resolved build setting values are passed through the environment.
     */
    if (options.expandBuildSettings()) {
        ExpandBuildSettings(root, settingsEnvironment);
    }

    /*
     * Process additional content files. These are plists that get merged with the
     * main Info.plist at the top level.
     */
    for (std::string const &additionalContentFile : options.additionalContentFiles()) {
        std::vector<uint8_t> contents;
        if (!filesystem->read(&contents, FSUtil::ResolveRelativePath(additionalContentFile, processContext->currentDirectory()))) {
            fprintf(stderr, "error: unable to read additional content file: %s\n", additionalContentFile.c_str());
            return 1;
        }

        auto additionalContent = plist::Format::Any::Deserialize(contents);
        if (additionalContent.first == nullptr) {
            fprintf(stderr, "error: unable to parse additional content file %s: %s\n", additionalContentFile.c_str(), additionalContent.second.c_str());
            return 1;
        }

        if (plist::Dictionary *dictionary = plist::CastTo<plist::Dictionary>(additionalContent.first.get())) {
            /* Pass true to replace existing entries. */
            root->merge(dictionary, true);
        }
    }

    /*
     * Info file keys/values: it's unknown what these are used for. Just warn.
     */
    if (options.infoFileKeys() || options.infoFileValues()) {
        // TODO(grp): Handle info file keys and values.
        fprintf(stderr, "warning: info file keys and values are not yet implemented\n");
    }

    /*
     * Platform and required architectures: it's unknown what these are used for. Just warn.
     */
    if (options.platform() || !options.requiredArchitectures().empty()) {
        // TODO(grp): Handle platform and required architectures.
#if 0
        fprintf(stderr, "warning: platform and required architectures are not yet implemented\n");
#endif
    }

    /*
     * Add entries to the Info.plist from the build environment.
     */
    AddBuildEnvironment(root, settingsEnvironment);

    /*
     * Write the PkgInfo file. This is just the package type and signature.
     */
    if (options.genPkgInfo()) {
        auto result = WritePkgInfo(filesystem, root, FSUtil::ResolveRelativePath(*options.genPkgInfo(), processContext->currentDirectory()));
        if (!result.first) {
            fprintf(stderr, "error: %s\n", result.second.c_str());
            return 1;
        }
    }

    /*
     * Copy the resource rules file. This is used by code signing.
     */
    if (options.resourceRulesFile()) {
        std::string resourceRulesInputPath = settingsEnvironment.resolve("CODE_SIGN_RESOURCE_RULES_PATH");
        if (!resourceRulesInputPath.empty()) {
            std::vector<uint8_t> contents;
            if (!filesystem->read(&contents, FSUtil::ResolveRelativePath(resourceRulesInputPath, processContext->currentDirectory()))) {
                fprintf(stderr, "error: unable to read input %s\n", resourceRulesInputPath.c_str());
                return 1;
            }

            if (!filesystem->write(contents, FSUtil::ResolveRelativePath(*options.resourceRulesFile(), processContext->currentDirectory()))) {
                fprintf(stderr, "error: could not open output path %s to write\n", options.resourceRulesFile()->c_str());
                return 1;
            }
        }
    }

    /*
     * Determine the output format. By default, use the same as the input format.
     */
    plist::Format::Any outputFormat = *inputFormat;
    if (options.format()) {
        if (*options.format() == "binary") {
            outputFormat = plist::Format::Any::Create(plist::Format::Binary::Create());
        } else if (*options.format() == "xml") {
            outputFormat = plist::Format::Any::Create(plist::Format::XML::Create(plist::Format::Encoding::UTF8));
        } else if (*options.format() == "ascii" || *options.format() == "openstep") {
            outputFormat = plist::Format::Any::Create(plist::Format::ASCII::Create(false, plist::Format::Encoding::UTF8));
        } else {
            fprintf(stderr, "error: unknown output format %s\n", options.format()->c_str());
            return 1;
        }
    }

    /* Serialize the output. */
    auto serialize = plist::Format::Any::Serialize(root, outputFormat);
    if (serialize.first == nullptr) {
        fprintf(stderr, "error: %s\n", serialize.second.c_str());
        return 1;
    }

    /* Write out the output. */
    if (!filesystem->write(*serialize.first, FSUtil::ResolveRelativePath(*options.output(), processContext->currentDirectory()))) {
        fprintf(stderr, "error: could not open output path %s to write\n", options.output()->c_str());
        return 1;
    }

    return 0;
}
