/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <builtin/copyStrings/Driver.h>
#include <builtin/copyStrings/Options.h>
#include <plist/Dictionary.h>
#include <plist/Object.h>
#include <plist/String.h>
#include <plist/Format/Any.h>
#include <plist/Format/ASCII.h>
#include <plist/Format/Binary.h>
#include <plist/Format/Encoding.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>
#include <libutil/Strings.h>
#include <process/Context.h>

using builtin::copyStrings::Driver;
using builtin::copyStrings::Options;
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
    return "builtin-copyStrings";
}

static bool
ParseStringsEncoding(std::string const &string, plist::Format::Any *format)
{
    if (libutil::strcasecmp(string.c_str(), "binary") == 0) {
        *format = plist::Format::Any::Create(plist::Format::Binary::Create());
        return true;
    } else if (libutil::strcasecmp(string.c_str(), "utf-8") == 0) {
        plist::Format::Encoding encoding = plist::Format::Encoding::UTF8;
        *format = plist::Format::Any::Create(plist::Format::ASCII::Create(true, encoding));
        return true;
    } else if (libutil::strcasecmp(string.c_str(), "utf-16") == 0) {
        plist::Format::Encoding encoding = plist::Format::Encoding::UTF16LE;
        *format = plist::Format::Any::Create(plist::Format::ASCII::Create(true, encoding));
        return true;
    } else if (libutil::strcasecmp(string.c_str(), "utf-32") == 0) {
        plist::Format::Encoding encoding = plist::Format::Encoding::UTF32LE;
        *format = plist::Format::Any::Create(plist::Format::ASCII::Create(true, encoding));
        return true;
    } else {
        return false;
    }
}

static std::pair<bool, std::string>
ValidateStrings(plist::Object const *root)
{
    if (plist::Dictionary const *dictionary = plist::CastTo<plist::Dictionary>(root)) {
        for (size_t n = 0; n < dictionary->count(); n++) {
            auto key = dictionary->key(n);
            if (dictionary->value <plist::String> (key)) {
                /* Valid for .strings files. */
            } else if (dictionary->value <plist::Dictionary> (key)) {
                /* Valid for .stringdict files. */
            } else {
                return std::make_pair(false, "strings key '" + key + "' is not a string (.strings) or a dictionary (.stringsdict)");
            }
        }

        return std::make_pair(true, std::string());
    } else {
        return std::make_pair(false, "strings file is not a dictionary");
    }
}

static bool
ValidateOptions(Options const &options)
{

    /*
     * It's unclear if an output directory should be required, but require it for
     * now since the behavior without one is also unclear.
     */
    if (!options.outputDirectory()) {
        fprintf(stderr, "error: output directory not provided\n");
        return false;
    }

    /*
     * Require at least one input.
     */
    if (options.inputs().empty()) {
        fprintf(stderr, "error: no input files provided\n");
        return false;
    }

    return true;
}

int Driver::
run(process::Context const *processContext, libutil::Filesystem *filesystem)
{
    Options options;
    std::pair<bool, std::string> result = libutil::Options::Parse<Options>(&options, processContext->commandLineArguments());
    if (!result.first) {
        fprintf(stderr, "error: %s\n", result.second.c_str());
        return -1;
    }

    /*
     * Validate options.
     */
    if (!ValidateOptions(options)) {
        return -1;
    }

    /*
     * Determine output encoding. Default to UTF-16 since that's what strings files should be.
     */
    plist::Format::Any outputFormat = plist::Format::Any::Create(plist::Format::ASCII::Create(true, plist::Format::Encoding::UTF16LE));
    if (options.outputEncoding() && !ParseStringsEncoding(*options.outputEncoding(), &outputFormat)) {
        fprintf(stderr, "error: invalid output encoding '%s'\n", options.outputEncoding()->c_str());
        return -1;
    }

    /*
     * Process each input.
     */
    for (std::string const &inputPath : options.inputs()) {
        /* Read in the input. */
        std::string resolvedInputPath = FSUtil::ResolveRelativePath(inputPath, processContext->currentDirectory());
        std::vector<uint8_t> inputContents;
        if (!filesystem->read(&inputContents, resolvedInputPath)) {
            fprintf(stderr, "error: unable to read input %s\n", inputPath.c_str());
            return 1;
        }

        /* Determine the input format. */
        std::unique_ptr<plist::Format::Any> inputFormat = plist::Format::Any::Identify(inputContents);
        if (inputFormat == nullptr) {
            fprintf(stderr, "error: input %s is not a plist\n", inputPath.c_str());
            return 1;
        }

        /* If no input format was specified, use the detected strings encoding. */
        plist::Format::Any resolvedInputFormat = *inputFormat;
        if (options.inputEncoding() && !ParseStringsEncoding(*options.inputEncoding(), &resolvedInputFormat)) {
            fprintf(stderr, "error: invalid input encoding '%s'\n", options.inputEncoding()->c_str());
            return -1;
        }

        /* Deserialize the input. */
        auto deserialize = plist::Format::Any::Deserialize(inputContents, resolvedInputFormat);
        if (!deserialize.first) {
            fprintf(stderr, "error: %s: %s\n", inputPath.c_str(), deserialize.second.c_str());
            return 1;
        }

        /* If requested, validate the strings file is valid. */
        if (options.validate()) {
            auto validation = ValidateStrings(deserialize.first.get());
            if (!validation.first) {
                fprintf(stderr, "error: %s: %s\n", inputPath.c_str(), validation.second.c_str());
                return 1;
            }
        }

        /* Output to the same name as the input, but in the output directory. */
        std::string outputPath = FSUtil::ResolveRelativePath(*options.outputDirectory(), processContext->currentDirectory()) + "/" + FSUtil::GetBaseName(inputPath);

        /* Write out the output. */
        auto serialize = plist::Format::Any::Serialize(deserialize.first.get(), outputFormat);
        if (serialize.first == nullptr) {
            fprintf(stderr, "error: %s: %s\n", inputPath.c_str(), serialize.second.c_str());
            return 1;
        }

        if (!filesystem->write(*serialize.first, outputPath)) {
            fprintf(stderr, "error: %s: could not write output\n", inputPath.c_str());
            return 1;
        }
    }

    return 0;
}
