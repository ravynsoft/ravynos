/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Build/DependencyResolver.h>
#include <pbxbuild/Target/Environment.h>

#define DEPENDENCY_RESOLVER_LOGGING 0

namespace Build = pbxbuild::Build;
namespace Target = pbxbuild::Target;
using pbxbuild::WorkspaceContext;
using pbxbuild::DirectedGraph;
using xcscheme::XC::Scheme;
using xcscheme::XC::BuildAction;
using xcscheme::XC::BuildActionEntry;

Build::DependencyResolver::
DependencyResolver(Build::Environment const &buildEnvironment) :
    _buildEnvironment(buildEnvironment)
{
}

Build::DependencyResolver::
~DependencyResolver()
{
}

static pbxproj::PBX::Target::shared_ptr
ResolveContainerItemProxy(Build::Environment const &buildEnvironment, Build::Context const &context, pbxproj::PBX::Target::shared_ptr const &target, pbxproj::PBX::ContainerItemProxy::shared_ptr const &proxy, bool productReference)
{
    pbxproj::PBX::FileReference::shared_ptr fileReference = proxy->containerPortal();
    if (fileReference == nullptr) {
        fprintf(stderr, "warning: not able to find file reference for proxy\n");
        return nullptr;
    }

    ext::optional<Target::Environment> targetEnvironment = context.targetEnvironment(buildEnvironment, target);
    if (!targetEnvironment) {
        fprintf(stderr, "warning: not able to get target environment for target %s %s\n", target->blueprintIdentifier().c_str(), target->name().c_str());
        return nullptr;
    }

    std::string path = targetEnvironment->environment().expand(fileReference->resolve());

    pbxproj::PBX::Project::shared_ptr project = context.workspaceContext().project(path);
    if (productReference) {
        auto result = context.resolveProductIdentifier(project, proxy->remoteGlobalIDString());
        if (!result) {
            fprintf(stderr, "warning: not able to resolve product identifier %s in project %s\n", proxy->remoteGlobalIDString().c_str(), project ? project->name().c_str() : path.c_str());
            return nullptr;
        }

        return result->first;
    } else {
        return context.resolveTargetIdentifier(project, proxy->remoteGlobalIDString());
    }
}

struct DependenciesContext {
    Build::Environment const *buildEnvironment;
    Build::Context     const *buildContext;
    DirectedGraph<pbxproj::PBX::Target::shared_ptr> *graph;
    BuildAction::shared_ptr buildAction;
    std::unordered_set<pbxproj::PBX::Target::shared_ptr> *positional;
    std::unordered_map<std::string, pbxproj::PBX::Target::shared_ptr> *productNameToTarget;
};

static void
AddDependencies(DependenciesContext const &context, pbxproj::PBX::Target::shared_ptr const &target);

static void
AddImplicitDependencies(DependenciesContext const &context, pbxproj::PBX::Target::shared_ptr const &target)
{
    std::unordered_set<pbxproj::PBX::Target::shared_ptr> dependencies;

    for (pbxproj::PBX::BuildPhase::shared_ptr const &buildPhase : target->buildPhases()) {
        // TODO(grp): Only include appropriate build phases for this action.
        // TODO(grp): This may be incomplete: is it possible to include proxied files in other phases?
        if (buildPhase->type() != pbxproj::PBX::BuildPhase::Type::Frameworks && buildPhase->type() != pbxproj::PBX::BuildPhase::Type::CopyFiles) {
            continue;
        }

        for (pbxproj::PBX::BuildFile::shared_ptr const &file : buildPhase->files()) {
            switch (file->fileRef()->type()) {
                case pbxproj::PBX::GroupItem::Type::ReferenceProxy: {
                    /* A implicit dependency referencing the product of another target through a direct reference to that target's product. */
                    pbxproj::PBX::ReferenceProxy::shared_ptr proxy = std::static_pointer_cast <pbxproj::PBX::ReferenceProxy> (file->fileRef());

                    pbxproj::PBX::Target::shared_ptr proxiedTarget = ResolveContainerItemProxy(*context.buildEnvironment, *context.buildContext, target, proxy->remoteRef(), true);
                    if (proxiedTarget != nullptr) {
                        dependencies.insert(proxiedTarget);

#if DEPENDENCY_RESOLVER_LOGGING
                        fprintf(stderr, "debug: implicit dependency: %s %s -> %s %s\n", target->blueprintIdentifier().c_str(), target->name().c_str(), proxiedTarget->blueprintIdentifier().c_str(), proxiedTarget->name().c_str());
#endif

                        /* Recursively search for implicit & explicit dependencies. */
                        AddDependencies(context, proxiedTarget);
                    } else {
                        fprintf(stderr, "warning: was not able to load target for implicit dependency %s (from %s)\n", proxy->remoteRef()->remoteInfo().c_str(), proxy->name().c_str());
                    }
                    break;
                }
                case pbxproj::PBX::GroupItem::Type::FileReference: {
                    /* A implicit dependency referencing the product of another target through a filesystem path. */
                    pbxproj::PBX::FileReference::shared_ptr fileReference = std::static_pointer_cast<pbxproj::PBX::FileReference>(file->fileRef());
                    std::string name = fileReference->name();

                    auto it = context.productNameToTarget->find(name);
                    if (it != context.productNameToTarget->end()) {
                        pbxproj::PBX::Target::shared_ptr dependentTarget = it->second;
                        dependencies.insert(dependentTarget);

#if DEPENDENCY_RESOLVER_LOGGING
                        fprintf(stderr, "debug: implicit dependency: %s %s -> %s %s\n", target->blueprintIdentifier().c_str(), target->name().c_str(), dependentTarget->blueprintIdentifier().c_str(), dependentTarget->name().c_str());
#endif

                        /* Recursively search for implicit & explicit dependencies. */
                        AddDependencies(context, dependentTarget);
                    }
                    break;
                }
                default: {
                    /* Any other type of build file isn't relevant. */
                    break;
                }
            }
        }
    }

    /* If there's no build action, this is a legacy context which always parallelizes builds. */
    if (context.buildAction == nullptr || context.buildAction->parallelizeBuildables()) {
        /* If builds are parallelized, use the actual dependencies for ordering. */
        context.graph->insert(target, dependencies);
    }
}

static void
AddExplicitDependencies(DependenciesContext const &context, pbxproj::PBX::Target::shared_ptr const &target)
{
    std::unordered_set<pbxproj::PBX::Target::shared_ptr> dependencies;

    for (pbxproj::PBX::TargetDependency::shared_ptr const &dependency : target->dependencies()) {
        if (dependency->target() != nullptr) {
            /* A dependency for another target in the same project. */
            dependencies.insert(dependency->target());

#if DEPENDENCY_RESOLVER_LOGGING
            fprintf(stderr, "debug: explicit dependency: %s %s -> %s %s\n", target->blueprintIdentifier().c_str(), target->name().c_str(), dependency->target()->blueprintIdentifier().c_str(), dependency->target()->name().c_str());
#endif

            /* Recursively search for implicit & explicit dependencies. */
            AddDependencies(context, dependency->target());
        } else if (dependency->targetProxy() != nullptr) {
            /* A dependency referencing a target in another project. Get that target. */
            pbxproj::PBX::Target::shared_ptr proxiedTarget = ResolveContainerItemProxy(*context.buildEnvironment, *context.buildContext, target, dependency->targetProxy(), false);
            if (proxiedTarget != nullptr) {
                dependencies.insert(proxiedTarget);

#if DEPENDENCY_RESOLVER_LOGGING
                fprintf(stderr, "debug: explicit dependency: %s %s -> %s %s\n", target->blueprintIdentifier().c_str(), target->name().c_str(), proxiedTarget->blueprintIdentifier().c_str(), proxiedTarget->name().c_str());
#endif

                /* Recursively search for implicit & explicit dependencies. */
                AddDependencies(context, proxiedTarget);
            } else {
                fprintf(stderr, "warning: was not able to load target for explicit dependency %s\n", dependency->targetProxy()->remoteInfo().c_str());
            }
        }
    }

    /* If there's no build action, this is a legacy context which always parallelizes builds. */
    if (context.buildAction == nullptr || context.buildAction->parallelizeBuildables()) {
        /* If builds are parallelized, use the actual dependencies for ordering. */
        context.graph->insert(target, dependencies);
    }
}

static void
AddDependencies(DependenciesContext const &context, pbxproj::PBX::Target::shared_ptr const &target)
{
    /* If there's no build action, this is a legacy context which always have implicit dependencies. */
    if (context.buildAction == nullptr || context.buildAction->buildImplicitDependencies()) {
        AddImplicitDependencies(context, target);
    }

    AddExplicitDependencies(context, target);

    /* If there's no build action, this is a legacy context which always parallelizes builds. */
    if (context.buildAction != nullptr && !context.buildAction->parallelizeBuildables()) {
#if DEPENDENCY_RESOLVER_LOGGING
        for (pbxproj::PBX::Target::shared_ptr const &orderTarget : *context.positional) {
            fprintf(stderr, "debug: order dependency: %s %s -> %s %s\n", target->blueprintIdentifier().c_str(), target->name().c_str(), orderTarget->blueprintIdentifier().c_str(), orderTarget->name().c_str());
        }
#endif

        if (context.positional->find(target) == context.positional->end()) {
            /*
             * Non-parallel targets are currently implemented by adding a dependency from this target
             * on all previous targets seen. This is inefficient: the space used in the graph is O(N^2).
             */
            context.graph->insert(target, *context.positional);
            context.positional->insert(target);
        }
    }
}

static std::unordered_map<std::string, pbxproj::PBX::Target::shared_ptr>
BuildProductPathsToTargets(WorkspaceContext const &workspaceContext)
{
    std::unordered_map<std::string, pbxproj::PBX::Target::shared_ptr> productNameToTarget;

    /*
     * Build a mapping of product path -> target, in order to look up implicit dependencies
     * where the product paths match, but the dependency is not through a container portal.
     */
    for (auto const &pair : workspaceContext.projects()) {
        for (pbxproj::PBX::Target::shared_ptr const &target : pair.second->targets()) {
            if (target->type() != pbxproj::PBX::Target::Type::Native) {
                /* Only native targets have products. */
                continue;
            }

            pbxproj::PBX::NativeTarget::shared_ptr nativeTarget = std::static_pointer_cast<pbxproj::PBX::NativeTarget>(target);
            pbxproj::PBX::FileReference::shared_ptr const &productReference = nativeTarget->productReference();

            if (productReference != nullptr) {
                /* Use the product name because we can't resolve the path yet. */
                std::string productName = productReference->name();
#if DEPENDENCY_RESOLVER_LOGGING
                fprintf(stderr, "debug: product output: %s %s => %s\n", target->blueprintIdentifier().c_str(), target->name().c_str(), productName.c_str());
#endif
                productNameToTarget.insert({ productName, nativeTarget });
            }
        }
    }

    return productNameToTarget;
}

DirectedGraph<pbxproj::PBX::Target::shared_ptr> Build::DependencyResolver::
resolveSchemeDependencies(Build::Context const &context) const
{
    DirectedGraph<pbxproj::PBX::Target::shared_ptr> graph;

    xcscheme::XC::Scheme::shared_ptr const &scheme = context.scheme();
    if (scheme == nullptr) {
        fprintf(stderr, "error: scheme not available\n");
        return graph;
    }

    BuildAction::shared_ptr const &buildAction = scheme->buildAction();
    if (buildAction == nullptr) {
        fprintf(stderr, "error: build action not available\n");
        return graph;
    }

    /* Only create the product path mapping if we have implicit dependencies on to use it. */
    std::unordered_map<std::string, pbxproj::PBX::Target::shared_ptr> productNameToTarget;
    if (buildAction->buildImplicitDependencies()) {
        productNameToTarget = BuildProductPathsToTargets(context.workspaceContext());
    }

    std::unordered_set<pbxproj::PBX::Target::shared_ptr> positional;
    for (BuildActionEntry::shared_ptr const &entry : buildAction->buildActionEntries()) {
        // TODO(grp): Check the buildFor* flags against the Build::Context.
        if (!entry->buildForRunning()) {
            continue;
        }

        xcscheme::XC::BuildableReference::shared_ptr const &reference = entry->buildableReference();
        if (reference == nullptr) {
            fprintf(stderr, "warning: couldn't find buildable reference in scheme\n");
            continue;
        }

        std::string projectPath = reference->resolve(context.schemeGroup());
        pbxproj::PBX::Project::shared_ptr project = context.workspaceContext().project(projectPath);
        if (project == nullptr) {
            fprintf(stderr, "warning: couldn't find project in workspace for build action entry\n");
            continue;
        }

        pbxproj::PBX::Target::shared_ptr target = context.resolveTargetIdentifier(project, reference->blueprintIdentifier());
        if (target == nullptr) {
            /*
             * If the blueprintIdentifier doesn't match, try the blueprintName. This can happen when a checked-in
             * scheme references a generated project. After regeneration, the scheme's blueprint identifier won't match.
             */
            for (pbxproj::PBX::Target::shared_ptr const &projectTarget : project->targets()) {
                if (reference->blueprintName() == projectTarget->name()) {
                    target = projectTarget;
                    break;
                }
            }

            if (target == nullptr) {
                fprintf(stderr, "warning: couldn't find buildable reference for build action entry '%s'\n", reference->blueprintName().c_str());
                continue;
            }
        }

        DependenciesContext dependenciesContext;
        dependenciesContext.buildEnvironment = &_buildEnvironment;
        dependenciesContext.buildContext = &context;
        dependenciesContext.graph = &graph;
        dependenciesContext.buildAction = buildAction;
        dependenciesContext.positional = &positional;
        dependenciesContext.productNameToTarget = &productNameToTarget;
        AddDependencies(dependenciesContext, target);
    }

#if DEPENDENCY_RESOLVER_LOGGING
    fprintf(stderr, "debug: scheme-based target ordering\n");
    ext::optional<std::vector<pbxproj::PBX::Target::shared_ptr>> result = graph.ordered();
    if (result) {
        for (pbxproj::PBX::Target::shared_ptr const &target : *result) {
            fprintf(stderr, "debug: ordered target %s %s\n", target->blueprintIdentifier().c_str(), target->name().c_str());
        }
    }
#endif

    return graph;
}

DirectedGraph<pbxproj::PBX::Target::shared_ptr> Build::DependencyResolver::
resolveLegacyDependencies(Build::Context const &context, bool allTargets, ext::optional<std::vector<std::string>> const &targetNames) const
{
    DirectedGraph<pbxproj::PBX::Target::shared_ptr> graph;

    pbxproj::PBX::Project::shared_ptr const &project = context.workspaceContext().project();
    if (project == nullptr) {
        fprintf(stderr, "error: cannot resolve legacy dependencies for workspace\n");
        return graph;
    }

    auto productNameToTarget = BuildProductPathsToTargets(context.workspaceContext());

    std::unordered_set<pbxproj::PBX::Target::shared_ptr> positional;
    for (pbxproj::PBX::Target::shared_ptr const &target : project->targets()) {
        if (!allTargets) {
            if (targetNames && std::find(targetNames->begin(), targetNames->end(), target->name()) == targetNames->end()) {
                /* Building specific targets, and not this one. */
                continue;
            } else if (!targetNames && target != project->targets().front()) {
                /* No specific target, build whatever the first target is. */
                continue;
            }
        }

        DependenciesContext dependenciesContext;
        dependenciesContext.buildEnvironment = &_buildEnvironment;
        dependenciesContext.buildContext = &context;
        dependenciesContext.graph = &graph;
        dependenciesContext.buildAction = nullptr;
        dependenciesContext.positional = &positional;
        dependenciesContext.productNameToTarget = &productNameToTarget;
        AddDependencies(dependenciesContext, target);
    }

#if DEPENDENCY_RESOLVER_LOGGING
    fprintf(stderr, "debug: legacy target ordering\n");
    ext::optional<std::vector<pbxproj::PBX::Target::shared_ptr>> result = graph.ordered();
    if (result) {
        for (pbxproj::PBX::Target::shared_ptr const &target : *result) {
            fprintf(stderr, "debug: ordered target %s %s\n", target->blueprintIdentifier().c_str(), target->name().c_str());
        }
    }
#endif

    return graph;
}

