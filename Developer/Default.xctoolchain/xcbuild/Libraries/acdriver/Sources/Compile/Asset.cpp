/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <acdriver/Compile/Asset.h>
#include <acdriver/Compile/AppIconSet.h>
#include <acdriver/Compile/BrandAssets.h>
#include <acdriver/Compile/ComplicationSet.h>
#include <acdriver/Compile/CubeTextureSet.h>
#include <acdriver/Compile/DataSet.h>
#include <acdriver/Compile/GCDashboardImage.h>
#include <acdriver/Compile/GCLeaderboard.h>
#include <acdriver/Compile/GCLeaderboardSet.h>
#include <acdriver/Compile/IconSet.h>
#include <acdriver/Compile/ImageSet.h>
#include <acdriver/Compile/ImageStack.h>
#include <acdriver/Compile/ImageStackLayer.h>
#include <acdriver/Compile/LaunchImage.h>
#include <acdriver/Compile/MipmapSet.h>
#include <acdriver/Compile/SpriteAtlas.h>
#include <acdriver/Compile/Sticker.h>
#include <acdriver/Compile/StickerPack.h>
#include <acdriver/Compile/StickerSequence.h>
#include <acdriver/Compile/StickersIconSet.h>
#include <acdriver/Compile/TextureSet.h>
#include <acdriver/Compile/Output.h>
#include <acdriver/Result.h>
#include <xcassets/Asset/Catalog.h>
#include <xcassets/Asset/Group.h>
#include <xcassets/Asset/Stickers.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>

using acdriver::Compile::Asset;
using acdriver::Compile::AppIconSet;
using acdriver::Compile::BrandAssets;
using acdriver::Compile::ComplicationSet;
using acdriver::Compile::CubeTextureSet;
using acdriver::Compile::DataSet;
using acdriver::Compile::GCDashboardImage;
using acdriver::Compile::GCLeaderboard;
using acdriver::Compile::GCLeaderboardSet;
using acdriver::Compile::IconSet;
using acdriver::Compile::ImageSet;
using acdriver::Compile::ImageStack;
using acdriver::Compile::ImageStackLayer;
using acdriver::Compile::LaunchImage;
using acdriver::Compile::MipmapSet;
using acdriver::Compile::SpriteAtlas;
using acdriver::Compile::Sticker;
using acdriver::Compile::StickerPack;
using acdriver::Compile::StickerSequence;
using acdriver::Compile::StickersIconSet;
using acdriver::Compile::TextureSet;
using acdriver::Compile::Output;
using acdriver::Result;
using libutil::Filesystem;
using libutil::FSUtil;

template<typename T>
static bool
CompileChildren(
    std::vector<std::unique_ptr<T>> const &assets,
    xcassets::Asset::Asset const *asset,
    Filesystem *filesystem,
    Output *compileOutput,
    Result *result)
{
    bool success = true;

    for (auto const &asset : assets) {
        if (!Asset::Compile(asset.get(), filesystem, compileOutput, result)) {
            success = false;
        }
    }

    return success;
}

bool Asset::
Compile(
    xcassets::Asset::Asset const *asset,
    Filesystem *filesystem,
    Output *compileOutput,
    Result *result)
{
    std::string filename = FSUtil::GetBaseName(asset->path());
    std::string name = FSUtil::GetBaseNameWithoutExtension(filename);

    switch (asset->type()) {
        case xcassets::Asset::AssetType::AppIconSet: {
            auto appIconSet = static_cast<xcassets::Asset::AppIconSet const *>(asset);
            if (appIconSet->name().name() == compileOutput->appIcon()) {
                Compile::AppIconSet::Compile(appIconSet, compileOutput, result);
            }
            break;
        }
        case xcassets::Asset::AssetType::BrandAssets: {
            auto brandAssets = static_cast<xcassets::Asset::BrandAssets const *>(asset);
            Compile::BrandAssets::Compile(brandAssets, filesystem, compileOutput, result);
            CompileChildren(brandAssets->children(), asset, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::Catalog: {
            auto catalog = static_cast<xcassets::Asset::Catalog const *>(asset);
            CompileChildren(catalog->children(), asset, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::ComplicationSet: {
            auto complicationSet = static_cast<xcassets::Asset::ComplicationSet const *>(asset);
            Compile::ComplicationSet::Compile(complicationSet, filesystem, compileOutput, result);
            CompileChildren(complicationSet->children(), asset, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::CubeTextureSet: {
            auto cubeTextureSet = static_cast<xcassets::Asset::CubeTextureSet const *>(asset);
            Compile::CubeTextureSet::Compile(cubeTextureSet, filesystem, compileOutput, result);
            CompileChildren(cubeTextureSet->children(), asset, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::DataSet: {
            auto dataSet = static_cast<xcassets::Asset::DataSet const *>(asset);
            Compile::DataSet::Compile(dataSet, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::GCDashboardImage: {
            auto dashboardImage = static_cast<xcassets::Asset::GCDashboardImage const *>(asset);
            Compile::GCDashboardImage::Compile(dashboardImage, filesystem, compileOutput, result);
            CompileChildren(dashboardImage->children(), asset, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::GCLeaderboard: {
            auto leaderboard = static_cast<xcassets::Asset::GCLeaderboard const *>(asset);
            Compile::GCLeaderboard::Compile(leaderboard, filesystem, compileOutput, result);
            CompileChildren(leaderboard->children(), asset, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::GCLeaderboardSet: {
            auto leaderboardSet = static_cast<xcassets::Asset::GCLeaderboardSet const *>(asset);
            Compile::GCLeaderboardSet::Compile(leaderboardSet, filesystem, compileOutput, result);
            CompileChildren(leaderboardSet->children(), asset, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::Group: {
            auto group = static_cast<xcassets::Asset::Group const *>(asset);
            CompileChildren(group->children(), asset, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::IconSet: {
            auto iconSet = static_cast<xcassets::Asset::IconSet const *>(asset);
            Compile::IconSet::Compile(iconSet, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::ImageSet: {
            auto imageSet = static_cast<xcassets::Asset::ImageSet const *>(asset);
            Compile::ImageSet::Compile(imageSet, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::ImageStack: {
            auto imageStack = static_cast<xcassets::Asset::ImageStack const *>(asset);
            Compile::ImageStack::Compile(imageStack, filesystem, compileOutput, result);
            CompileChildren(imageStack->children(), asset, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::ImageStackLayer: {
            auto imageStackLayer = static_cast<xcassets::Asset::ImageStackLayer const *>(asset);
            Compile::ImageStackLayer::Compile(imageStackLayer, filesystem, compileOutput, result);
            // TODO: CompileChildren(imageStackLayer->children(), asset, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::LaunchImage: {
            auto launchImage = static_cast<xcassets::Asset::LaunchImage const *>(asset);
            if (launchImage->name().name() == compileOutput->launchImage()) {
                Compile::LaunchImage::Compile(launchImage, compileOutput, result);
            }
            break;
        }
        case xcassets::Asset::AssetType::MipmapSet: {
            auto mipmapSet = static_cast<xcassets::Asset::MipmapSet const *>(asset);
            Compile::MipmapSet::Compile(mipmapSet, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::SpriteAtlas: {
            auto spriteAtlas = static_cast<xcassets::Asset::SpriteAtlas const *>(asset);
            Compile::SpriteAtlas::Compile(spriteAtlas, filesystem, compileOutput, result);
            CompileChildren(spriteAtlas->children(), asset, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::Sticker: {
            auto sticker = static_cast<xcassets::Asset::Sticker const *>(asset);
            Compile::Sticker::Compile(sticker, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::StickerPack: {
            auto stickerPack = static_cast<xcassets::Asset::StickerPack const *>(asset);
            Compile::StickerPack::Compile(stickerPack, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::StickerSequence: {
            auto stickerSequence = static_cast<xcassets::Asset::StickerSequence const *>(asset);
            Compile::StickerSequence::Compile(stickerSequence, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::Stickers: {
            auto stickers = static_cast<xcassets::Asset::Stickers const *>(asset);
            CompileChildren(stickers->children(), asset, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::StickersIconSet: {
            auto stickersIconSet = static_cast<xcassets::Asset::StickersIconSet const *>(asset);
            Compile::StickersIconSet::Compile(stickersIconSet, filesystem, compileOutput, result);
            break;
        }
        case xcassets::Asset::AssetType::TextureSet: {
            auto textureSet = static_cast<xcassets::Asset::TextureSet const *>(asset);
            Compile::TextureSet::Compile(textureSet, filesystem, compileOutput, result);
            CompileChildren(textureSet->children(), asset, filesystem, compileOutput, result);
            break;
        }
    }

    return true;
}

