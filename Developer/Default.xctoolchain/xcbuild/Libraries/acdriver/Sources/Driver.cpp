/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <acdriver/Driver.h>
#include <acdriver/Options.h>
#include <acdriver/Output.h>
#include <acdriver/Result.h>
#include <acdriver/VersionAction.h>
#include <acdriver/CompileAction.h>
#include <acdriver/ContentsAction.h>
#include <libutil/Filesystem.h>
#include <process/Context.h>

#include <algorithm>
#include <iterator>
#include <iostream>

using acdriver::Driver;
using acdriver::Options;
using acdriver::Output;
using acdriver::Result;
using acdriver::VersionAction;
using acdriver::CompileAction;
using acdriver::ContentsAction;
using libutil::Filesystem;

Driver::
Driver()
{
}

Driver::
~Driver()
{
}

static void
RunInternal(Filesystem *filesystem, Options const &options, Output *output, Result *result)
{
    if (options.version()) {
        VersionAction version;
        version.run(options, output, result);
    }

    if (options.printContents()) {
        ContentsAction contents;
        contents.run(filesystem, options, output, result);
    }

    if (options.compile()) {
        CompileAction compile;
        compile.run(filesystem, options, output, result);
    }
}

static Output::Format
OptionsOutputFormat(Options const &options, Result *result)
{
    if (!options.outputFormat() || *options.outputFormat() == "xml1") {
        return Output::Format::XML;
    } else if (*options.outputFormat() == "binary1") {
        return Output::Format::Binary;
    } else if (*options.outputFormat() == "human-readable-text") {
        return Output::Format::Text;
    } else {
        result->normal(Result::Severity::Error, "unknown output format");
        return Output::Format::XML;
    }
}

int Driver::
Run(process::Context const *processContext, Filesystem *filesystem)
{
    Result result;
    Output output;

    /*
     * Parse input options.
     */
    Options options;
    std::pair<bool, std::string> optionsResult = libutil::Options::Parse<Options>(&options, processContext->commandLineArguments());
    if (!optionsResult.first) {
        result.normal(Result::Severity::Error, optionsResult.second, std::string("unknown option"));
    }

    /*
     * Validate output format. Do this first so errors are caught early.
     */
    Output::Format outputFormat = OptionsOutputFormat(options, &result);

    if (result.success()) {
        /*
         * Perform actions specified by options.
         */
        RunInternal(filesystem, options, &output, &result);
    }

    /*
     * Serialize the result into the output.
     */
    result.write(&output);

    /*
     * Write out the output in the specified output format.
     */
    ext::optional<std::vector<uint8_t>> outputContents = output.serialize(outputFormat);
    if (outputContents) {
        std::copy(outputContents->begin(), outputContents->end(), std::ostream_iterator<char>(std::cout));
    } else {
        fprintf(stderr, "error: unable to serialize output\n");
    }

    return (result.success() ? 0 : 1);
}
