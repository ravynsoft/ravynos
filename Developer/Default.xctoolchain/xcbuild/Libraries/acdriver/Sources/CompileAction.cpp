/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <acdriver/CompileAction.h>
#include <acdriver/Compile/Output.h>
#include <acdriver/Compile/Asset.h>
#include <acdriver/Version.h>
#include <acdriver/Options.h>
#include <acdriver/Output.h>
#include <acdriver/Result.h>
#include <xcassets/Asset/Catalog.h>
#include <xcassets/Slot/SystemVersion.h>
#include <bom/bom.h>
#include <car/Reader.h>
#include <car/Writer.h>
#include <bom/bom_format.h>
#include <dependency/BinaryDependencyInfo.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Format/XML.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>

using acdriver::CompileAction;
namespace Compile = acdriver::Compile;
using acdriver::Version;
using acdriver::Options;
using acdriver::Output;
using acdriver::Result;
using libutil::Filesystem;
using libutil::FSUtil;

CompileAction::
CompileAction()
{
}

CompileAction::
~CompileAction()
{
}

static bool
WriteOutput(Filesystem *filesystem, Options const &options, Compile::Output const &compileOutput, Output *output, Result *result)
{
    bool success = true;

    /*
     * Collect all inputs and outputs.
     */
    auto info = dependency::DependencyInfo(compileOutput.inputs(), compileOutput.outputs());

    /*
     * Write out compiled archive.
     */
    if (compileOutput.car()) {
        // TODO: only write if non-empty. but did mmap already create the file?
        compileOutput.car()->write();
    }

    /*
     * Copy files into output.
     */
    for (std::pair<std::string, std::string> const &copy : compileOutput.copies()) {
        std::vector<uint8_t> contents;

        if (!filesystem->read(&contents, copy.first)) {
            result->normal(Result::Severity::Error, "unable to read input: " + copy.first);
            success = false;
            continue;
        }

        if (!filesystem->write(contents, copy.second)) {
            result->normal(Result::Severity::Error, "unable to write output: " + copy.second);
            success = false;
            continue;
        }
    }

    /*
     * Write out partial info plist, if requested.
     */
    if (options.outputPartialInfoPlist()) {
        auto format = plist::Format::XML::Create(plist::Format::Encoding::UTF8);
        auto serialize = plist::Format::XML::Serialize(compileOutput.additionalInfo(), format);
        if (serialize.first == nullptr) {
            result->normal(Result::Severity::Error, "unable to serialize partial info plist");
            success = false;
        } else {
            if (!filesystem->write(*serialize.first, *options.outputPartialInfoPlist())) {
                result->normal(Result::Severity::Error, "unable to write partial info plist");
                success = false;
            }
        }

        /* Note output file. */
        info.outputs().push_back(*options.outputPartialInfoPlist());
    }

    /*
     * Write out dependency info, if requested.
     */
    if (options.exportDependencyInfo()) {
        auto binaryInfo = dependency::BinaryDependencyInfo();
        binaryInfo.version() = "actool-" + std::to_string(Version::BuildVersion());
        binaryInfo.dependencyInfo() = info;

        if (!filesystem->write(binaryInfo.serialize(), *options.exportDependencyInfo())) {
            result->normal(Result::Severity::Error, "unable to write dependency info");
            success = false;
        }
    }

    /*
     * Add output files to output.
     */
    {
        std::string text;
        auto array = plist::Array::New();

        for (std::string const &output : info.outputs()) {
            /* Array is one entry per file. */
            array->append(plist::String::New(output));

            /* Text is one line per file. */
            text += output;
            text += "\n";
        }

        auto dict = plist::Dictionary::New();
        dict->set("output-files", std::move(array));

        output->add("com.apple.actool.compilation-results", std::move(dict), text);
    }

    return success;
}

static ext::optional<Compile::Output::Format>
DetermineOutputFormat(ext::optional<std::string> const &minimumDeploymentTarget)
{
    if (minimumDeploymentTarget) {
        if (auto systemVersion = xcassets::Slot::SystemVersion::Parse(*minimumDeploymentTarget)) {
            /*
             * Only versions starting with 7 support compiled assets.
             */
            if (systemVersion->major() >= 7) {
                return Compile::Output::Format::Compiled;
            } else {
                return Compile::Output::Format::Folder;
            }
        } else {
            return ext::nullopt;
        }
    } else {
        /* If no version is specified, default to compiled. */
        return Compile::Output::Format::Compiled;
    }
}

static ext::optional<car::Writer>
CreateWriter(std::string const &path)
{
    struct bom_context_memory memory = bom_context_memory_file(path.c_str(), true, 0);
    if (memory.data == NULL) {
        return ext::nullopt;
    }

    auto bom = car::Writer::unique_ptr_bom(bom_alloc_empty(memory), bom_free);
    if (bom == nullptr) {
        return ext::nullopt;
    }

    return car::Writer::Create(std::move(bom));
}

static void
WarnUnsupportedOptions(Options const &options, Result *result)
{
    if (options.productType()) {
        result->normal(Result::Severity::Warning, "product type not supported");
    }

    if (options.optimization()) {
        result->normal(Result::Severity::Warning, "optimization not supported");
    }

    if (options.compressPNGs()) {
        result->normal(Result::Severity::Warning, "compress PNGs not supported");
    }

    if (options.platform()) {
        result->normal(Result::Severity::Warning, "platform not supported");
    }

    if (options.flattenedAppIconPath()) {
        result->normal(Result::Severity::Warning, "flattened app icon not supported");
    }

    if (options.stickerPackIdentifierPrefix() || options.stickerPackStringsFile()) {
        /* Strings file format: `sticker-pack-name:language-identifier:path`. */
        result->normal(Result::Severity::Warning, "sticker pack not supported");
    }

    if (options.leaderboardIdentifierPrefix() || options.leaderboardSetIdentifierPrefix()) {
        result->normal(Result::Severity::Warning, "leaderboard set not supportd");
    }

    if (!options.targetDevice().empty()) {
        result->normal(Result::Severity::Warning, "target device not supported");
    }

    if (options.targetName()) {
        result->normal(Result::Severity::Warning, "target name not supported");
    }

    if (options.enableOnDemandResources()) {
        result->normal(Result::Severity::Warning, "on-demand resources not supported");
    }

    if (options.enableIncrementalDistill()) {
        result->normal(Result::Severity::Warning, "incremental distill not supported");
    }

    if (options.filterForDeviceModel()) {
        result->normal(Result::Severity::Warning, "filter device model not supported");
    }

    if (options.filterForDeviceOsVersion()) {
        result->normal(Result::Severity::Warning, "filter device os version not supported");
    }
}

void CompileAction::
run(Filesystem *filesystem, Options const &options, Output *output, Result *result)
{
    // TODO: support all options
    WarnUnsupportedOptions(options, result);

    if (!options.isValid(result)) {
       return;
    }

    /*
     * Determine format to output compiled assets.
     */
    ext::optional<Compile::Output::Format> outputFormat = DetermineOutputFormat(options.minimumDeploymentTarget());
    if (!outputFormat) {
        result->normal(Result::Severity::Error, "invalid minimum deployment target");
        return;
    }

    /*
     * Create compilation output.
     */
    Compile::Output compileOutput = Compile::Output(
        *options.compile(),
        *outputFormat,
        options.appIcon(),
        options.launchImage(),
        options.nonStandardOptions().allowImageTypes());

    /*
     * If necessary, create output archive to write into.
     */
    if (compileOutput.format() == Compile::Output::Format::Compiled) {
        std::string outputFilename = options.compileOutputFilename().value_or("Assets.car");
        std::string path = compileOutput.root() + "/" + outputFilename;

        ext::optional<car::Writer> writer = CreateWriter(path);
        if (!writer) {
            result->normal(Result::Severity::Error, "unable to create compiled asset writer");
            return;
        }

        compileOutput.car() = std::move(writer);
        // TODO: should only be an output if ultimately non-empty
        compileOutput.outputs().push_back(path);
    }

    /*
     * Compile each asset catalog into the output.
     */
    for (std::string const &input : options.inputs()) {
        /*
         * Load the input asset catalog.
         */
        auto catalog = xcassets::Asset::Catalog::Load(filesystem, input);
        if (catalog == nullptr) {
            result->normal(
                Result::Severity::Error,
                "unable to load asset catalog",
                ext::nullopt,
                input);
            continue;
        }

        /*
         * Compile the asset catalog.
         */
        if (!Compile::Asset::Compile(catalog.get(), filesystem, &compileOutput, result)) {
            /* Error already printed. */
            continue;
        }

        compileOutput.inputs().push_back(input);
    }

    /*
     * Write out the output.
     */
    if (!WriteOutput(filesystem, options, compileOutput, output, result)) {
        /* Error already reported. */
        return;
    }
}

