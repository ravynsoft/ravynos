/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/MipmapLevel.h>

#include <cstdlib>

using xcassets::MipmapLevel;
using xcassets::MipmapLevels;

ext::optional<MipmapLevel> MipmapLevels::
Parse(std::string const &value)
{
    if (value == "base") {
        return MipmapLevel::Base;
    } else if (value == "mipmap-level-1") {
        return MipmapLevel::Level1;
    } else if (value == "mipmap-level-2") {
        return MipmapLevel::Level2;
    } else if (value == "mipmap-level-3") {
        return MipmapLevel::Level3;
    } else if (value == "mipmap-level-4") {
        return MipmapLevel::Level4;
    } else if (value == "mipmap-level-5") {
        return MipmapLevel::Level5;
    } else if (value == "mipmap-level-6") {
        return MipmapLevel::Level6;
    } else if (value == "mipmap-level-7") {
        return MipmapLevel::Level7;
    } else if (value == "mipmap-level-8") {
        return MipmapLevel::Level8;
    } else if (value == "mipmap-level-9") {
        return MipmapLevel::Level9;
    } else if (value == "mipmap-level-10") {
        return MipmapLevel::Level10;
    } else if (value == "mipmap-level-11") {
        return MipmapLevel::Level11;
    } else if (value == "mipmap-level-12") {
        return MipmapLevel::Level12;
    } else if (value == "mipmap-level-13") {
        return MipmapLevel::Level13;
    } else if (value == "mipmap-level-14") {
        return MipmapLevel::Level14;
    } else if (value == "mipmap-level-15") {
        return MipmapLevel::Level15;
    } else if (value == "mipmap-level-16") {
        return MipmapLevel::Level16;
    } else {
        fprintf(stderr, "warning: unknown mipmap level mode %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string MipmapLevels::
String(MipmapLevel mipmapLevel)
{
    switch (mipmapLevel) {
        case MipmapLevel::Base:
            return "base";
        case MipmapLevel::Level1:
            return "mipmap-level-1";
        case MipmapLevel::Level2:
            return "mipmap-level-2";
        case MipmapLevel::Level3:
            return "mipmap-level-3";
        case MipmapLevel::Level4:
            return "mipmap-level-4";
        case MipmapLevel::Level5:
            return "mipmap-level-5";
        case MipmapLevel::Level6:
            return "mipmap-level-6";
        case MipmapLevel::Level7:
            return "mipmap-level-7";
        case MipmapLevel::Level8:
            return "mipmap-level-8";
        case MipmapLevel::Level9:
            return "mipmap-level-9";
        case MipmapLevel::Level10:
            return "mipmap-level-10";
        case MipmapLevel::Level11:
            return "mipmap-level-11";
        case MipmapLevel::Level12:
            return "mipmap-level-12";
        case MipmapLevel::Level13:
            return "mipmap-level-13";
        case MipmapLevel::Level14:
            return "mipmap-level-14";
        case MipmapLevel::Level15:
            return "mipmap-level-15";
        case MipmapLevel::Level16:
            return "mipmap-level-16";
    }

    abort();
}
