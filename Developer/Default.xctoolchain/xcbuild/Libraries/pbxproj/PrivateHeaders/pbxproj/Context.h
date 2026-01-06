/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_Context_h
#define __pbxproj_Context_h

#include <pbxproj/PlistHelpers.h>
#include <pbxproj/ISA.h>
#include <plist/Dictionary.h>
#include <plist/Object.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace pbxproj {

//
// Forward declarations
//
namespace PBX {

class Object;
class Project;
class Group;
class VariantGroup;
class FileReference;
class ReferenceProxy;
class NativeTarget;
class AggregateTarget;
class LegacyTarget;
class ContainerItemProxy;
class TargetDependency;
class BuildFile;
class BuildRule;
class HeadersBuildPhase;
class SourcesBuildPhase;
class ResourcesBuildPhase;
class FrameworksBuildPhase;
class CopyFilesBuildPhase;
class ShellScriptBuildPhase;
class AppleScriptBuildPhase;
class RezBuildPhase;

}

namespace XC {

class BuildConfiguration;
class ConfigurationList;
class VersionGroup;

}

class Context {
public:
    //
    // Parsing context
    //
    plist::Dictionary const *objects;

    //
    // The main project
    //
    std::shared_ptr<PBX::Project> project;

    //
    // Cached values
    //
    std::unordered_map <std::string, std::shared_ptr <PBX::Project>>               projects;
    std::unordered_map <std::string, std::shared_ptr <PBX::FileReference>>         fileReferences;
    std::unordered_map <std::string, std::shared_ptr <PBX::ReferenceProxy>>        referenceProxies;
    std::unordered_map <std::string, std::shared_ptr <PBX::Group>>                 groups;
    std::unordered_map <std::string, std::shared_ptr <PBX::VariantGroup>>          variantGroups;
    std::unordered_map <std::string, std::shared_ptr <PBX::NativeTarget>>          nativeTargets;
    std::unordered_map <std::string, std::shared_ptr <PBX::AggregateTarget>>       aggregateTargets;
    std::unordered_map <std::string, std::shared_ptr <PBX::LegacyTarget>>          legacyTargets;
    std::unordered_map <std::string, std::shared_ptr <PBX::TargetDependency>>      targetDependencies;
    std::unordered_map <std::string, std::shared_ptr <PBX::ContainerItemProxy>>    containerItemProxies;
    std::unordered_map <std::string, std::shared_ptr <PBX::BuildFile>>             buildFiles;
    std::unordered_map <std::string, std::shared_ptr <PBX::BuildRule>>             buildRules;
    std::unordered_map <std::string, std::shared_ptr <PBX::HeadersBuildPhase>>     headersBuildPhases;
    std::unordered_map <std::string, std::shared_ptr <PBX::SourcesBuildPhase>>     sourcesBuildPhases;
    std::unordered_map <std::string, std::shared_ptr <PBX::ResourcesBuildPhase>>   resourcesBuildPhases;
    std::unordered_map <std::string, std::shared_ptr <PBX::FrameworksBuildPhase>>  frameworksBuildPhases;
    std::unordered_map <std::string, std::shared_ptr <PBX::CopyFilesBuildPhase>>   copyFilesBuildPhases;
    std::unordered_map <std::string, std::shared_ptr <PBX::ShellScriptBuildPhase>> shellScriptBuildPhases;
    std::unordered_map <std::string, std::shared_ptr <PBX::AppleScriptBuildPhase>> appleScriptBuildPhases;
    std::unordered_map <std::string, std::shared_ptr <PBX::RezBuildPhase>>         rezBuildPhases;

    std::unordered_map <std::string, std::shared_ptr <XC::BuildConfiguration>>     buildConfigurations;
    std::unordered_map <std::string, std::shared_ptr <XC::ConfigurationList>>      configurationLists;
    std::unordered_map <std::string, std::shared_ptr <XC::VersionGroup>>           versionGroups;

public:
    Context()
    {
        project = nullptr;
    }

    inline void clear()
    {
        project = nullptr;
        projects.clear();
        fileReferences.clear();
        referenceProxies.clear();
        groups.clear();
        variantGroups.clear();
        nativeTargets.clear();
        targetDependencies.clear();
        containerItemProxies.clear();
        buildFiles.clear();
        buildRules.clear();
        headersBuildPhases.clear();
        sourcesBuildPhases.clear();
        resourcesBuildPhases.clear();
        frameworksBuildPhases.clear();
        copyFilesBuildPhases.clear();
        shellScriptBuildPhases.clear();
        appleScriptBuildPhases.clear();
        rezBuildPhases.clear();

        buildConfigurations.clear();
        configurationLists.clear();
        versionGroups.clear();
    }

    //
    // Helper functions
    //
private:
    inline plist::Dictionary const *get(std::string const &key,
                                        std::string const &isa,
                                        std::string *id = nullptr) const
    {
        if (id != nullptr) {
            *id = key;
        }
        return PlistDictionaryGetPBXObject(objects, key, isa);
    }

public:
    template <typename T>
    inline plist::Dictionary const *get(std::string const &key,
                                       std::string *id = nullptr) const
    { return get(key, T::Isa(), id); }

private:
    inline plist::Dictionary const *get(plist::Object const *objectKey,
                                        std::string const &isa,
                                        std::string *id = nullptr) const
    {
        plist::String const *key = plist::CastTo <plist::String> (objectKey);
        if (key == nullptr)
            return nullptr;

        if (id != nullptr) {
            *id = key->value();
        }
        return PlistDictionaryGetPBXObject(objects, key->value(), isa);
    }

public:
    template <typename T>
    inline plist::Dictionary const *get(plist::Object const *objectKey,
                                        std::string *id = nullptr) const
    { return get(objectKey, T::Isa(), id); }

private:
    inline plist::Dictionary const *indirect(plist::Keys::Unpack *unpack,
                                             std::string const &key,
                                             std::string const &isa,
                                             std::string *id = nullptr) const
    {
        return PlistDictionaryGetIndirectPBXObject(objects, unpack, key, isa, id);
    }

public:
    template <typename T>
    inline plist::Dictionary const *indirect(plist::Keys::Unpack *unpack,
                                             std::string const &key,
                                             std::string *id = nullptr) const
    {
        return indirect(unpack, key, T::Isa(), id);
    }

private:
    inline plist::Dictionary const *indirect(plist::Keys::Unpack *unpack,
                                             plist::Object const *objectKey,
                                             std::string const &isa,
                                             std::string *id = nullptr) const
    {
        plist::String const *key = plist::CastTo <plist::String> (objectKey);
        if (key == nullptr)
            return nullptr;
        else
            return PlistDictionaryGetIndirectPBXObject(objects, unpack,
                    key->value(), isa, id);
    }

public:
    template <typename T>
    inline plist::Dictionary const *indirect(plist::Keys::Unpack *unpack,
                                             plist::Object const *objectKey,
                                             std::string *id = nullptr) const
    {
        return indirect(unpack, objectKey, T::Isa(), id);
    }

public:
    template <typename T>
    inline std::shared_ptr <T> parseObject(std::unordered_map <std::string, std::shared_ptr <T>> &cache,
                                           std::string const &id,
                                           plist::Dictionary const *dict)
    {
        auto I = cache.find(id);
        if (I != cache.end())
            return I->second;

        auto O = std::make_shared <T> ();
        cacheObject(O, id); // cache inside the project
        cache[id] = O; // cache local to the context

        if (!O->parseObject(*this, dict)) {
            cache.erase(id);
            return std::shared_ptr <T> ();
        }

        return O;
    }

    template <typename T>
    inline std::shared_ptr <T> parseObject(std::unordered_map <std::string, std::shared_ptr <T>> &cache,
                                           plist::Object const *objectId,
                                           plist::Dictionary const *dict)
    {
        plist::String const *id = plist::CastTo <plist::String> (objectId);
        if (id == nullptr)
            return nullptr;

        return parseObject(cache, id->value(), dict);
    }

private:
    void cacheObject(std::shared_ptr <PBX::Object> const &O, std::string const &id);
};

}

#endif  // !__pbxproj_Context_h
