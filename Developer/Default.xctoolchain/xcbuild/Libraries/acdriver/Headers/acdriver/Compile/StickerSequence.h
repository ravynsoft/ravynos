/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __acdriver_Compile_StickerSequence_h
#define __acdriver_Compile_StickerSequence_h

#include <xcassets/Asset/Asset.h>
#include <xcassets/Asset/StickerSequence.h>

#include <memory>

namespace libutil { class Filesystem; }

namespace acdriver {

class Result;

namespace Compile {

class Output;

class StickerSequence {
private:
    StickerSequence();
    ~StickerSequence();

public:
    static bool Compile(
        xcassets::Asset::StickerSequence const *stickerSequence,
        libutil::Filesystem *filesystem,
        Output *compileOutput,
        Result *result);
};

}
}

#endif // !__acdriver_Compile_StickerSequence_h
