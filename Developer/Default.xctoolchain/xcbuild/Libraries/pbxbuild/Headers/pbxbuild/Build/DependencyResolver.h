/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Build_DependencyResolver_h
#define __pbxbuild_Build_DependencyResolver_h

#include <pbxbuild/Base.h>
#include <pbxbuild/Build/Environment.h>
#include <pbxbuild/Build/Context.h>
#include <pbxbuild/DirectedGraph.h>

namespace pbxbuild {
namespace Build {

/*
 * Resolves dependencies and produces a complete DAG for the targets
 * in a scheme for a build action.
 */
class DependencyResolver {
private:
    Build::Environment _buildEnvironment;

public:
    DependencyResolver(Build::Environment const &buildEnviroment);
    ~DependencyResolver();

public:
    /*
     * Resolves dependencies within a scheme. The scheme used is as specified
     * by the `Build::Context`, as well as the build action.
     */
    DirectedGraph<pbxproj::PBX::Target::shared_ptr>
    resolveSchemeDependencies(Build::Context const &context) const;

public:
    /*
     * Resolves legacy dependencies for targets in a project without a scheme.
     * If `allTargets` is specified, include all targets in the build; if `targets`
     * is specified, include just those targets; otherwise, include the first target.
     */
    DirectedGraph<pbxproj::PBX::Target::shared_ptr>
    resolveLegacyDependencies(Build::Context const &context, bool allTargets, ext::optional<std::vector<std::string>> const &targets) const;
};

}
}

#endif // !__pbxbuild_Build_DependencyResolver_h
