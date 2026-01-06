/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Asset/Asset.h>
#include <xcassets/Asset/AppIconSet.h>
#include <xcassets/Asset/Catalog.h>
#include <xcassets/Asset/Group.h>
#include <xcassets/Asset/DataSet.h>
#include <xcassets/Asset/ImageSet.h>
#include <libutil/DefaultFilesystem.h>
#include <libutil/Filesystem.h>

using xcassets::MatchingStyles;
using xcassets::Asset::Asset;
using xcassets::Asset::AppIconSet;
using xcassets::Asset::Catalog;
using xcassets::Asset::Group;
using xcassets::Asset::DataSet;
using xcassets::Asset::ImageSet;
using xcassets::Slot::Idioms;
using libutil::DefaultFilesystem;
using libutil::Filesystem;

static void
Print(std::string const &string, int indent)
{
    for (int i = 0; i < indent; ++i) {
        fprintf(stdout, "  ");
    }
    fprintf(stdout, "%s\n", string.c_str());
}

static void
DumpAsset(Asset const *asset, int indent = 0)
{
    Print("name: " + asset->name().name(), indent);
    Print("identifier: " + asset->name().string(), indent);
    Print("path: " + asset->path(), indent);

    if (asset->author()) {
        Print("author: " + *asset->author(), indent);
    }
    if (asset->version()) {
        Print("version: " + std::to_string(*asset->version()), indent);
    }

    if (asset->type() == AppIconSet::Type()) {
        auto appIconSet = static_cast<AppIconSet const *>(asset);
        Print("type: AppIconSet", indent);
        Print("pre-rendered: " + std::to_string(appIconSet->preRendered()), indent);

        if (appIconSet->images()) {
            for (AppIconSet::Image const &image : *appIconSet->images()) {
                Print("", indent + 1);
                Print("icon image", indent + 1);

                if (image.fileName()) {
                    Print("file name: " + *image.fileName(), indent + 1);
                }
                if (image.idiom()) {
                    Print("idiom: " + Idioms::String(*image.idiom()), indent + 1);
                }
                if (image.unassignedOptional()) {
                    Print("unassigned: " + std::to_string(image.unassigned()), indent + 1);
                }
                if (image.matchingStyle()) {
                    Print("matching-style: " + MatchingStyles::String(*image.matchingStyle()), indent + 1);
                }
            }
        }
    } else if (asset->type() == Catalog::Type()) {
        auto catalog = static_cast<Catalog const *>(asset);
        Print("type: Catalog", indent);

        for (std::unique_ptr<Asset> const &child : catalog->children()) {
            Print("", indent);
            DumpAsset(child.get(), indent + 1);
        }
    } else if (asset->type() == Group::Type()) {
        auto group = static_cast<Group const *>(asset);
        Print("type: Group", indent);

        if (group->onDemandResourceTags()) {
            Print("on-demand-asset-tags: " + std::to_string(group->onDemandResourceTags()->size()), indent);
        }
        Print("provides namespace: " + std::to_string(group->providesNamespace()), indent);

        for (std::unique_ptr<Asset> const &child : group->children()) {
            Print("", indent);
            DumpAsset(child.get(), indent + 1);
        }
    } else if (asset->type() == ImageSet::Type()) {
        auto imageSet = static_cast<ImageSet const *>(asset);
        Print("type: ImageSet", indent);

        if (imageSet->onDemandResourceTags()) {
            Print("on-demand-asset-tags: " + std::to_string(imageSet->onDemandResourceTags()->size()), indent);
        }

        if (imageSet->images()) {
            for (ImageSet::Image const &image : *imageSet->images()) {
                Print("", indent + 1);
                Print("image", indent + 1);

                if (image.fileName()) {
                    Print("file name: " + *image.fileName(), indent + 1);
                }
                if (image.idiom()) {
                    Print("idiom: " + Idioms::String(*image.idiom()), indent + 1);
                }
                if (image.unassignedOptional()) {
                    Print("unassigned: " + std::to_string(image.unassigned()), indent + 1);
                }

                // TODO(grp): Print resizing.
            }
        }
    } else if (asset->type() == DataSet::Type()) {
        auto dataSet = static_cast<DataSet const *>(asset);
        Print("type: DataSet", indent);

        if (dataSet->onDemandResourceTags()) {
            Print("on-demand-asset-tags: " + std::to_string(dataSet->onDemandResourceTags()->size()), indent);
        }

        if (dataSet->data()) {
            for (DataSet::Data const &data : *dataSet->data()) {
                Print("", indent + 1);
                Print("data", indent + 1);

                if (data.fileName()) {
                    Print("file name: " + *data.fileName(), indent + 1);
                }
                if (data.idiom()) {
                    Print("idiom: " + Idioms::String(*data.idiom()), indent + 1);
                }

                // TODO(grp): Print more info.
            }
        }
    } else {
        Print("unknown asset type", indent);
    }
}

int
main(int argc, char **argv)
{
    DefaultFilesystem filesystem = DefaultFilesystem();

    if (argc < 2) {
        fprintf(stderr, "error: missing input\n");
        return 1;
    }

    std::unique_ptr<Asset> asset = Asset::Load(&filesystem, argv[1], { });
    DumpAsset(asset.get());

    return 0;
}

