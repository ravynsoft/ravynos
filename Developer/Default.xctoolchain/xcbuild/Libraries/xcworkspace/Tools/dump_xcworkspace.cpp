/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcscheme/xcscheme.h>
#include <pbxproj/pbxproj.h>
#include <xcworkspace/xcworkspace.h>
#include <libutil/DefaultFilesystem.h>
#include <libutil/Filesystem.h>
#include <libutil/Strings.h>
#include <process/DefaultContext.h>
#include <process/Context.h>
#include <process/DefaultUser.h>
#include <process/User.h>

#include <algorithm>
#include <cstring>
#include <cerrno>

using namespace pbxproj;
using namespace libutil;

void DumpItem(xcworkspace::XC::Workspace::shared_ptr const &W, xcworkspace::XC::GroupItem const *item, size_t indent);
void DumpItems(xcworkspace::XC::Workspace::shared_ptr const &W, xcworkspace::XC::GroupItem::vector const &items, size_t indent);

std::string
MakePath(Filesystem const *filesystem, xcworkspace::XC::Workspace const &W, xcworkspace::XC::Group const *group,
        std::string const &location, bool real = true)
{
    std::string GL;

    if (group != nullptr) {
        GL = group->location();
    }

    if (real) {
        return filesystem->resolvePath(W.basePath() + "/" + GL + "/" + location);
    } else {
        if (GL.empty()) {
            return W.basePath() + "/" + location;
        } else {
            return W.basePath() + "/" + GL + "/" + location;
        }
    }
}

void
DumpFileRef(xcworkspace::XC::Workspace::shared_ptr const &W,
            xcworkspace::XC::FileRef const *fref, size_t indent)
{
    printf("%*s%s [%s]\n", (int)(indent * 2), "",
            fref->resolve(W).c_str(),
            fref->locationType().c_str());
}

void
DumpGroup(xcworkspace::XC::Workspace::shared_ptr const &W,
          xcworkspace::XC::Group const *group, size_t indent)
{
    printf("%*s[%s] (%s [%s])\n", (int)(indent * 2), "",
            group->name().c_str(),
            group->resolve(W).c_str(),
            group->locationType().c_str());

    DumpItems(W, group->items(), indent + 1);
}

void
DumpItem(xcworkspace::XC::Workspace::shared_ptr const &W, xcworkspace::XC::GroupItem const *item, size_t indent)
{
    switch (item->type()) {
        case xcworkspace::XC::GroupItem::Type::Group:
            DumpGroup(W, static_cast <xcworkspace::XC::Group const *> (item), indent);
            break;
        case xcworkspace::XC::GroupItem::Type::FileRef:
            DumpFileRef(W, static_cast <xcworkspace::XC::FileRef const *> (item), indent);
            break;
        default:
            break;
    }
}

void
DumpItems(xcworkspace::XC::Workspace::shared_ptr const &W, xcworkspace::XC::GroupItem::vector const &items, size_t indent)
{
    for (auto item : items) {
        DumpItem(W, item.get(), indent);
    }
}

void
DumpItems(xcworkspace::XC::Workspace::shared_ptr const &W, size_t indent = 0)
{
    DumpItems(W, W->items(), indent);
}

void ForEachItems(xcworkspace::XC::Workspace::shared_ptr const &W, xcworkspace::XC::Group const *group,
        xcworkspace::XC::GroupItem::vector const &items,
        std::function<void(xcworkspace::XC::Group const *, xcworkspace::XC::FileRef const &)> const &cb);

void
ForEachFileRef(xcworkspace::XC::Workspace::shared_ptr const &W, xcworkspace::XC::Group const *group,
        xcworkspace::XC::FileRef const *fref,
        std::function<void(xcworkspace::XC::Group const *, xcworkspace::XC::FileRef const &)> const &cb)
{
    cb(group, *fref);
}

void
ForEachGroup(xcworkspace::XC::Workspace::shared_ptr const &W, xcworkspace::XC::Group const *group,
        std::function<void(xcworkspace::XC::Group const *, xcworkspace::XC::FileRef const &)> const &cb)
{
    ForEachItems(W, group, group->items(), cb);
}

void
ForEachItem(xcworkspace::XC::Workspace::shared_ptr const &W, xcworkspace::XC::Group const *group,
        xcworkspace::XC::GroupItem const *item,
        std::function<void(xcworkspace::XC::Group const *, xcworkspace::XC::FileRef const &)> const &cb)
{
    switch (item->type()) {
        case xcworkspace::XC::GroupItem::Type::Group:
            ForEachGroup(W, static_cast <xcworkspace::XC::Group const *> (item), cb);
            break;
        case xcworkspace::XC::GroupItem::Type::FileRef:
            ForEachFileRef(W, group, static_cast <xcworkspace::XC::FileRef const *> (item), cb);
            break;
        default:
            break;
    }
}

void
ForEachItems(xcworkspace::XC::Workspace::shared_ptr const &W, xcworkspace::XC::Group const *group,
        xcworkspace::XC::GroupItem::vector const &items,
        std::function<void(xcworkspace::XC::Group const *, xcworkspace::XC::FileRef const &)> const &cb)
{
    for (auto item : items) {
        ForEachItem(W, group, item.get(), cb);
    }
}

void
ForEachFileRef(xcworkspace::XC::Workspace::shared_ptr const &W,
        std::function<void(xcworkspace::XC::Group const *, xcworkspace::XC::FileRef const &)> const &cb)
{
    ForEachItems(W, nullptr, W->items(), cb);
}

int
main(int argc, char **argv)
{
    DefaultFilesystem filesystem = DefaultFilesystem();
    process::DefaultUser user = process::DefaultUser();
    process::DefaultContext processContext = process::DefaultContext();

    if (argc < 2) {
        fprintf(stderr, "usage: %s filename.xcworkspace\n", argv[0]);
        return -1;
    }

    auto workspace = xcworkspace::XC::Workspace::Open(&filesystem, argv[1]);
    if (!workspace) {
        fprintf(stderr, "error opening workspace (%s)\n",
                strerror(errno));
        return -1;
    }

#if 1
    fprintf(stderr, "parse ok\n");
    printf("Workspace File: %s\n", workspace->projectFile().c_str());
    printf("Base Path:      %s\n", workspace->basePath().c_str());
    printf("Name:           %s\n", workspace->name().c_str());

    auto workspaceGroup = xcscheme::SchemeGroup::Open(&filesystem, user.userName(), workspace->basePath(), workspace->projectFile(), workspace->name());

    printf("Schemes:\n");
    if (workspaceGroup) {
        for (auto &I : workspaceGroup->schemes()) {
            printf("\t%s [%s]%s\n", I->name().c_str(),
                    I->shared() ? "Shared" : I->owner().c_str(),
                    I == workspaceGroup->defaultScheme() ? " (DEFAULT)" : "");
        }
    }

    DumpItems(workspace);

    PBX::Project::vector projects;
    ForEachFileRef(workspace,
            [&](xcworkspace::XC::Group const *g, xcworkspace::XC::FileRef const &fref)
            {
                std::string path = MakePath(&filesystem, *workspace, g, fref.location(), false);
                printf("opening %s\n", path.c_str());

                auto project = PBX::Project::Open(&filesystem, path);
                if (!project) {
                    printf("couldn't open %s\n", path.c_str());
                }
                projects.push_back(project);
            });

    projects.clear();
#endif

    //
    // Collect all schemes
    //
    xcscheme::XC::Scheme::vector schemes;
    if (workspaceGroup) {
        schemes.insert(schemes.end(),
                       workspaceGroup->schemes().begin(),
                       workspaceGroup->schemes().end());
    }
    ForEachFileRef(workspace,
            [&](xcworkspace::XC::Group const *g, xcworkspace::XC::FileRef const &fref)
            {
                std::string path = MakePath(&filesystem, *workspace, g, fref.location(), false);
                auto project = PBX::Project::Open(&filesystem, path);
                if (project) {
                    auto projectGroup = xcscheme::SchemeGroup::Open(&filesystem, user.userName(), project->basePath(), project->projectFile(), project->name());
                    if (projectGroup) {
                        schemes.insert(schemes.end(),
                                       projectGroup->schemes().begin(),
                                       projectGroup->schemes().end());
                    }
                }
            });

    printf("Information about workspace \"%s\":\n",
            workspace->name().c_str());
    if (schemes.empty()) {
        printf("\n%4sThis workspace contains no scheme.\n", "");
    } else {
        std::sort(schemes.begin(), schemes.end(),
                [](xcscheme::XC::Scheme::shared_ptr const &a, xcscheme::XC::Scheme::shared_ptr const &b) -> bool
                {
                    return libutil::strcasecmp(a->name().c_str(), b->name().c_str()) < 0;
                });

        auto I = std::unique(schemes.begin(), schemes.end(),
                [](xcscheme::XC::Scheme::shared_ptr const &a, xcscheme::XC::Scheme::shared_ptr const &b) -> bool
                {
                    return (a->path() == b->path());
                });
        schemes.resize(std::distance(schemes.begin(), I));

        printf("%4sSchemes:\n", "");
        for (auto scheme : schemes) {
            printf("%8s%s\n", "", scheme->name().c_str());
        }
        printf("\n");
    }

    return 0;
}
