/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Asset/Asset.h>
#include <xcassets/Asset/AppIconSet.h>
#include <xcassets/Asset/BrandAssets.h>
#include <xcassets/Asset/Catalog.h>
#include <xcassets/Asset/ComplicationSet.h>
#include <xcassets/Asset/CubeTextureSet.h>
#include <xcassets/Asset/DataSet.h>
#include <xcassets/Asset/GCDashboardImage.h>
#include <xcassets/Asset/GCLeaderboard.h>
#include <xcassets/Asset/GCLeaderboardSet.h>
#include <xcassets/Asset/Group.h>
#include <xcassets/Asset/IconSet.h>
#include <xcassets/Asset/ImageSet.h>
#include <xcassets/Asset/ImageStack.h>
#include <xcassets/Asset/ImageStackLayer.h>
#include <xcassets/Asset/LaunchImage.h>
#include <xcassets/Asset/MipmapSet.h>
#include <xcassets/Asset/Sticker.h>
#include <xcassets/Asset/StickerPack.h>
#include <xcassets/Asset/StickerSequence.h>
#include <xcassets/Asset/Stickers.h>
#include <xcassets/Asset/StickersIconSet.h>
#include <xcassets/Asset/SpriteAtlas.h>
#include <xcassets/Asset/TextureSet.h>
#include <plist/Keys/Unpack.h>
#include <plist/Format/JSON.h>
#include <plist/Boolean.h>
#include <plist/String.h>
#include <plist/Integer.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>

using xcassets::Asset::Asset;
using xcassets::Asset::AssetType;
using xcassets::FullyQualifiedName;
using libutil::Filesystem;
using libutil::FSUtil;

Asset::
Asset(FullyQualifiedName const &name, std::string const &path) :
    _name(name),
    _path(path)
{
}

Asset::
~Asset()
{
}

Asset const *Asset::
child(std::string const &fileName, ext::optional<AssetType> type) const
{
    for (std::unique_ptr<Asset> const &asset : _children) {
        if (asset->type() != type) {
            /* Wrong kind of asset. */
            continue;
        }

        if (FSUtil::GetBaseName(asset->path()) == fileName) {
            return asset.get();
        }
    }

    return nullptr;
}

Asset const *Asset::
child(ext::optional<AssetType> type) const
{
    Asset const *candidate = nullptr;

    for (std::unique_ptr<Asset> const &asset : _children) {
        if (asset->type() != type) {
            /* Wrong kind of asset. */
            continue;
        }

        if (candidate != nullptr) {
            /* Multiple assets: which one? */
            return nullptr;
        }

        candidate = asset.get();
    }

    return candidate;
}

bool Asset::
parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    /* No contents is allowed for some assets. */
    if (dict == nullptr) {
        return true;
    }

    auto unpack = plist::Keys::Unpack("Asset", dict, seen);

    auto I = unpack.cast <plist::Dictionary> ("info");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (I != nullptr) {
        std::unordered_set<std::string> seen;
        auto unpack = plist::Keys::Unpack("Info", I, &seen);

        auto A = unpack.cast <plist::String> ("author");
        auto V = unpack.cast <plist::Integer> ("version");

        if (!unpack.complete(true)) {
            fprintf(stderr, "%s", unpack.errorText().c_str());
        }

        if (A != nullptr) {
            _author = A->value();
        }

        if (V != nullptr) {
            _version = static_cast<int>(V->value());

            if (_version != 1) {
                fprintf(stderr, "warning: unknown version: %d", *_version);
                return false;
            }
        }
    }

    return true;
}

std::unique_ptr<Asset> Asset::
Load(Filesystem const *filesystem, std::string const &path, std::vector<std::string> const &groups, ext::optional<std::string> const &overrideExtension)
{
    std::string resolvedPath = filesystem->resolvePath(path);
    FullyQualifiedName name = FullyQualifiedName(groups, FSUtil::GetBaseNameWithoutExtension(path));

    /*
     * Assets are always in directories.
     */
    if (filesystem->type(resolvedPath) != Filesystem::Type::Directory) {
        return nullptr;
    }

    /*
     * Get the file extension.
     */
    ext::optional<std::string> extension = overrideExtension;
    if (!extension) {
        std::string fileExtension = FSUtil::GetFileExtension(resolvedPath);
        if (!fileExtension.empty()) {
            extension = fileExtension;
        }
    }

    /*
     * Create an asset of the right type.
     */
    std::unique_ptr<Asset> asset = nullptr;
    if (extension == AppIconSet::Extension()) {
        auto appIconSet = std::unique_ptr<AppIconSet>(new AppIconSet(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(appIconSet));
    } else if (extension == BrandAssets::Extension()) {
        auto brandAssets = std::unique_ptr<BrandAssets>(new BrandAssets(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(brandAssets));
    } else if (extension == Catalog::Extension()) {
        auto catalog = std::unique_ptr<Catalog>(new Catalog(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(catalog));
    } else if (extension == ComplicationSet::Extension()) {
        auto complicationSet = std::unique_ptr<ComplicationSet>(new ComplicationSet(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(complicationSet));
    } else if (extension == CubeTextureSet::Extension()) {
        auto cubeTextureSet = std::unique_ptr<CubeTextureSet>(new CubeTextureSet(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(cubeTextureSet));
    } else if (extension == DataSet::Extension()) {
        auto dataSet = std::unique_ptr<DataSet>(new DataSet(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(dataSet));
    } else if (extension == GCDashboardImage::Extension()) {
        auto gcDashboardImage = std::unique_ptr<GCDashboardImage>(new GCDashboardImage(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(gcDashboardImage));
    } else if (extension == GCLeaderboard::Extension()) {
        auto gcLeaderboard = std::unique_ptr<GCLeaderboard>(new GCLeaderboard(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(gcLeaderboard));
    } else if (extension == GCLeaderboardSet::Extension()) {
        auto gcLeaderboardSet = std::unique_ptr<GCLeaderboardSet>(new GCLeaderboardSet(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(gcLeaderboardSet));
    } else if (extension == Group::Extension()) {
        auto group = std::unique_ptr<Group>(new Group(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(group));
    } else if (extension == IconSet::Extension()) {
        auto iconSet = std::unique_ptr<IconSet>(new IconSet(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(iconSet));
    } else if (extension == ImageSet::Extension()) {
        auto imageSet = std::unique_ptr<ImageSet>(new ImageSet(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(imageSet));
    } else if (extension == ImageStack::Extension()) {
        auto imageStack = std::unique_ptr<ImageStack>(new ImageStack(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(imageStack));
    } else if (extension == ImageStackLayer::Extension()) {
        auto imageStackLayer = std::unique_ptr<ImageStackLayer>(new ImageStackLayer(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(imageStackLayer));
    } else if (extension == LaunchImage::Extension()) {
        auto launchImage = std::unique_ptr<LaunchImage>(new LaunchImage(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(launchImage));
    } else if (extension == MipmapSet::Extension()) {
        auto mipmapSet = std::unique_ptr<MipmapSet>(new MipmapSet(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(mipmapSet));
    } else if (extension == SpriteAtlas::Extension()) {
        auto spriteAtlas = std::unique_ptr<SpriteAtlas>(new SpriteAtlas(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(spriteAtlas));
    } else if (extension == Sticker::Extension()) {
        auto sticker = std::unique_ptr<Sticker>(new Sticker(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(sticker));
    } else if (extension == StickerPack::Extension()) {
        auto stickerPack = std::unique_ptr<StickerPack>(new StickerPack(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(stickerPack));
    } else if (extension == StickerSequence::Extension()) {
        auto stickerSequence = std::unique_ptr<StickerSequence>(new StickerSequence(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(stickerSequence));
    } else if (extension == Stickers::Extension()) {
        auto stickers = std::unique_ptr<Stickers>(new Stickers(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(stickers));
    } else if (extension == StickersIconSet::Extension()) {
        auto stickersIconSet = std::unique_ptr<StickersIconSet>(new StickersIconSet(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(stickersIconSet));
    } else if (extension == TextureSet::Extension()) {
        auto textureSet = std::unique_ptr<TextureSet>(new TextureSet(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(textureSet));
    } else {
        /*
         * Directories with unknown extensions are treated as groups.
         */
        auto group = std::unique_ptr<Group>(new Group(name, resolvedPath));
        asset = libutil::static_unique_pointer_cast<Asset>(std::move(group));
    }

    if (!asset->load(filesystem)) {
        return nullptr;
    }

    return asset;
}

static bool
LoadContents(Filesystem const *filesystem, std::string const &path, std::unique_ptr<plist::Dictionary> *contentsDictionary)
{
    /*
     * Configure the asset with the contents.
     */
    std::string contentsPath = path + "/" + "Contents.json";

    /*
     * Check if the contents file exists. Not existing is valid for some asset types.
     */
    if (filesystem->isReadable(contentsPath)) {
        /*
         * Read in the contents file.
         */
        std::vector<uint8_t> contents;
        if (!filesystem->read(&contents, contentsPath)) {
            return false;
        }

        /*
         * If the Contents.json file exists, it must be JSON.
         */
        auto deserialized = plist::Format::JSON::Deserialize(contents, plist::Format::JSON::Create());
        if (!deserialized.first) {
            return false;
        }

        /*
         * If the Contents.json file exists, it must be a dictionary.
         */
        if (deserialized.first->type() != plist::Dictionary::Type()) {
            return false;
        }

        *contentsDictionary = plist::static_unique_pointer_cast<plist::Dictionary>(std::move(deserialized.first));
    }

    return true;
}

static bool
LoadChildren(Filesystem const *filesystem, std::string const &path, FullyQualifiedName const &name, bool providesNamespace, std::vector<std::unique_ptr<Asset>> *children)
{
    bool error = false;

    filesystem->readDirectory(path, false, [&](std::string const &fileName) -> void {
        std::string child = path + "/" + fileName;

        if (filesystem->type(child) == Filesystem::Type::Directory) {
            std::vector<std::string> groups = name.groups();
            if (providesNamespace) {
                // TODO: Should fully qualified names include extensions?
                groups.push_back(name.name());
            }

            std::unique_ptr<Asset> asset = Asset::Load(filesystem, child, groups);
            if (asset == nullptr) {
                fprintf(stderr, "error: failed to load asset: %s\n", child.c_str());
                error = true;
                return;
            }

            children->push_back(std::move(asset));
        }
    });

    return error;
}

bool Asset::
load(Filesystem const *filesystem)
{
    /*
     * Load the contents. This can succeed but not load anything.
     */
    std::unique_ptr<plist::Dictionary> contentsDictionary;
    if (!LoadContents(filesystem, _path, &contentsDictionary)) {
        return false;
    }

    /*
     * Note that some asset types have a field `provides-namespace` that affects
     * how children are loaded, which won't be loaded until parsing. To avoid
     * the cyclical dependency, pre-parse the `provides-namespace` key here.
     */
    bool providesNamespace = false;
    if (contentsDictionary != nullptr) {
        if (auto properties = contentsDictionary->value<plist::Dictionary>("properties")) {
            if (auto provides = properties->value<plist::Boolean>("provides-namespace")) {
                providesNamespace = provides->value();
            }
        }
    }

    /*
     * Load children now, so parsing can reference them.
     */
    if (!LoadChildren(filesystem, _path, _name, providesNamespace, &_children)) {
        /* A child failing to load is not an error for this asset. */
    }

    /*
     * Parse the contents dictionary.
     */
    std::unordered_set<std::string> seen;
    if (!this->parse(contentsDictionary.get(), &seen, true)) {
        return false;
    }

    return true;
}
