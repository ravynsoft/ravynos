/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <acdriver/Compile/ImageStackLayer.h>
#include <acdriver/Compile/Output.h>
#include <acdriver/Result.h>
#include <libutil/Filesystem.h>

using acdriver::Compile::ImageStackLayer;
using acdriver::Compile::Output;
using acdriver::Result;
using libutil::Filesystem;

bool ImageStackLayer::
Compile(
    xcassets::Asset::ImageStackLayer const *imageStackLayer,
    Filesystem *filesystem,
    Output *compileOutput,
    Result *result)
{
    result->document(
        Result::Severity::Warning,
        imageStackLayer->path(),
        { Output::AssetReference(imageStackLayer) },
        "Not Implemented",
        "image stack layer not yet supported");

    return false;
}
