/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <builtin/copyPlist/Driver.h>
#include <builtin/copyPlist/Options.h>
#include <plist/Object.h>
#include <plist/Format/Any.h>
#include <libutil/Filesystem.h>
#include <process/Context.h>
#include <libutil/FSUtil.h>

using builtin::copyPlist::Driver;
using builtin::copyPlist::Options;
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
    return "builtin-copyPlist";
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

    /*
     * It's unclear if an output directory should be required, but require it for
     * now since the behavior without one is also unclear.
     */
    if (!options.outputDirectory()) {
        fprintf(stderr, "error: output directory not provided\n");
        return 1;
    }

    /*
     * Require at least one input.
     */
    if (options.inputs().empty()) {
        fprintf(stderr, "error: no input files provided\n");
        return 1;
    }

    /*
     * Determine the output format. Leave null for the same as input.
     */
    std::unique_ptr<plist::Format::Any> convertFormat = nullptr;
    if (options.convertFormat()) {
        if (*options.convertFormat() == "binary1") {
            convertFormat = std::unique_ptr<plist::Format::Any>(new plist::Format::Any(plist::Format::Any::Create(
                plist::Format::Binary::Create()
            )));
        } else if (*options.convertFormat() == "xml1") {
            convertFormat = std::unique_ptr<plist::Format::Any>(new plist::Format::Any(plist::Format::Any::Create(
                plist::Format::XML::Create(plist::Format::Encoding::UTF8)
            )));
        } else if (*options.convertFormat() == "ascii1" || *options.convertFormat() == "openstep1") {
            convertFormat = std::unique_ptr<plist::Format::Any>(new plist::Format::Any(plist::Format::Any::Create(
                plist::Format::ASCII::Create(false, plist::Format::Encoding::UTF8)
            )));
        } else {
            fprintf(stderr, "error: unknown output format %s\n", options.convertFormat()->c_str());
            return 1;
        }
    }

    /*
     * Process each input.
     */
    for (std::string const &inputPath : options.inputs()) {
        /* Read in the input. */
        std::vector<uint8_t> inputContents;
        if (!filesystem->read(&inputContents, FSUtil::ResolveRelativePath(inputPath, processContext->currentDirectory()))) {
            fprintf(stderr, "error: unable to read input %s\n", inputPath.c_str());
            return 1;
        }

        std::vector<uint8_t> outputContents;

        if (convertFormat == nullptr && !options.validate()) {
            /*
             * If we aren't converting or validating, don't even bother parsing as a plist.
             */
            outputContents = inputContents;
        } else {
            /* Determine the input format. */
            std::unique_ptr<plist::Format::Any> inputFormat = plist::Format::Any::Identify(inputContents);
            if (inputFormat == nullptr) {
                fprintf(stderr, "error: input %s is not a plist\n", inputPath.c_str());
                return 1;
            }

            /* Deserialize the input. */
            auto deserialize = plist::Format::Any::Deserialize(inputContents, *inputFormat);
            if (!deserialize.first) {
                fprintf(stderr, "error: %s: %s\n", inputPath.c_str(), deserialize.second.c_str());
                return 1;
            }

            /* Use the conversion format if specified, otherwise use the same as the input. */
            plist::Format::Any outputFormat = (convertFormat != nullptr ? *convertFormat : *inputFormat);

            /* Serialize the output. */
            auto serialize = plist::Format::Any::Serialize(deserialize.first.get(), outputFormat);
            if (serialize.first == nullptr) {
                fprintf(stderr, "error: %s: %s\n", inputPath.c_str(), serialize.second.c_str());
                return 1;
            }

            outputContents = *serialize.first;
        }

        /* Output to the same name as the input, but in the output directory. */
        std::string outputPath = FSUtil::ResolveRelativePath(*options.outputDirectory(), processContext->currentDirectory()) + "/" + FSUtil::GetBaseName(inputPath);

        /* Write out the output. */
        if (!filesystem->write(outputContents, outputPath)) {
            fprintf(stderr, "error: could not open output path %s to write\n", outputPath.c_str());
            return 1;
        }
    }

    return 0;
}
