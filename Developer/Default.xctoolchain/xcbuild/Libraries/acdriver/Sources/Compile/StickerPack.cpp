/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <acdriver/Compile/StickerPack.h>
#include <acdriver/Compile/Output.h>
#include <acdriver/Result.h>
#include <libutil/Filesystem.h>

using acdriver::Compile::StickerPack;
using acdriver::Compile::Output;
using acdriver::Result;
using libutil::Filesystem;

bool StickerPack::
Compile(
    xcassets::Asset::StickerPack const *stickerPack,
    Filesystem *filesystem,
    Output *compileOutput,
    Result *result)
{
    result->document(
        Result::Severity::Warning,
        stickerPack->path(),
        { Output::AssetReference(stickerPack) },
        "Not Implemented",
        "sticker pack not yet supported");

    return false;
}
