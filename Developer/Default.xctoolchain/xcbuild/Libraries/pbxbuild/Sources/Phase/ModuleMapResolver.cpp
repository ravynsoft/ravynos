/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Phase/ModuleMapResolver.h>
#include <pbxbuild/Phase/Environment.h>
#include <pbxbuild/Phase/Context.h>
#include <pbxbuild/Tool/CopyResolver.h>
#include <pbxbuild/Tool/ModuleMapResolver.h>
#include <pbxsetting/Environment.h>
#include <pbxsetting/Type.h>
#include <libutil/FSUtil.h>

namespace Phase = pbxbuild::Phase;
namespace Tool = pbxbuild::Tool;
using libutil::FSUtil;

Phase::ModuleMapResolver::
ModuleMapResolver()
{
}

Phase::ModuleMapResolver::
~ModuleMapResolver()
{
}

static void
ProcessModuleMap(
    Tool::Context *toolContext,
    pbxsetting::Environment const &environment,
    Tool::CopyResolver const *copyResolver,
    Tool::ModuleMapInfo::Entry const &entry)
{
    /* Define source module map. */
    auto auxiliaryFile = Tool::AuxiliaryFile(entry.intermediatePath(), { entry.contents() });
    toolContext->auxiliaryFiles().push_back(auxiliaryFile);

    /* Copy in the module map as part of the build. */
    Tool::Input input = Tool::Input(entry.intermediatePath(), nullptr);
    copyResolver->resolve(toolContext, environment, { input }, FSUtil::GetDirectoryName(entry.finalPath()), "Ditto");
}

bool Phase::ModuleMapResolver::
resolve(Phase::Environment const &phaseEnvironment, Phase::Context *phaseContext) const
{
    pbxsetting::Environment const &environment = phaseEnvironment.targetEnvironment().environment();

    std::unique_ptr<Tool::ModuleMapResolver> moduleMapResolver = Tool::ModuleMapResolver::Create();
    if (moduleMapResolver == nullptr) {
        return false;
    }

    Tool::CopyResolver const *copyResolver = phaseContext->copyResolver(phaseEnvironment);
    if (copyResolver == nullptr) {
        fprintf(stderr, "warning: failed to get copy tool for module map\n");
        return false;
    }

    /*
     * Create the module map info. Note this is required even if a module is not defined, in order to
     * have the module map info populated for other consumers of it.
     */
    moduleMapResolver->resolve(&phaseContext->toolContext(), environment, phaseEnvironment.target());

    /*
     * Check if creating module maps is even requested.
     */
    if (pbxsetting::Type::ParseBoolean(environment.resolve("DEFINES_MODULE"))) {
        /*
         * Write the module maps as auxiliary temporary files, then copy them to the product..
         */
        Tool::ModuleMapInfo const &moduleMapInfo = phaseContext->toolContext().moduleMapInfo();

        if (ext::optional<Tool::ModuleMapInfo::Entry> const &entry = moduleMapInfo.moduleMap()) {
            ProcessModuleMap(&phaseContext->toolContext(), environment, copyResolver, *entry);
        } else {
            fprintf(stderr, "warning: target defines module, but has no umbrella header\n");
        }

        if (ext::optional<Tool::ModuleMapInfo::Entry> const &entry = moduleMapInfo.privateModuleMap()) {
            ProcessModuleMap(&phaseContext->toolContext(), environment, copyResolver, *entry);
        }
    }

    return true;
}

