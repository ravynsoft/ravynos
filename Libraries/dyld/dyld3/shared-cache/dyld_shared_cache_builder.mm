/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
 *
 * Copyright (c) 2016 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <signal.h>
#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/resource.h>
#include <dirent.h>
#include <libgen.h>
#include <pthread.h>
#include <fts.h>

#include <vector>
#include <array>
#include <set>
#include <map>
#include <unordered_set>
#include <algorithm>
#include <fstream>
#include <regex>

#include <spawn.h>

#include <Bom/Bom.h>
#include <Foundation/NSData.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSPropertyList.h>
#include <Foundation/NSString.h>

#include "Diagnostics.h"
#include "DyldSharedCache.h"
#include "FileUtils.h"
#include "JSONReader.h"
#include "JSONWriter.h"
#include "StringUtils.h"
#include "mrm_shared_cache_builder.h"

#if !__has_feature(objc_arc)
#error The use of libdispatch in this files requires it to be compiled with ARC in order to avoid leaks
#endif

extern char** environ;

static dispatch_queue_t build_queue;

int runCommandAndWait(Diagnostics& diags, const char* args[])
{
    pid_t pid;
    int   status;
    int   res = posix_spawn(&pid, args[0], nullptr, nullptr, (char**)args, environ);
    if (res != 0)
        diags.error("Failed to spawn %s: %s (%d)", args[0], strerror(res), res);

    do {
        res = waitpid(pid, &status, 0);
    } while (res == -1 && errno == EINTR);
    if (res != -1) {
        if (WIFEXITED(status)) {
            res = WEXITSTATUS(status);
        } else {
            res = -1;
        }
    }

    return res;
}

void processRoots(Diagnostics& diags, std::set<std::string>& roots, const char *tempRootsDir)
{
    std::set<std::string> processedRoots;
    struct stat           sb;
    int                   res = 0;
    const char*           args[8];

    for (const auto& root : roots) {
        res = stat(root.c_str(), &sb);

        if (res == 0 && S_ISDIR(sb.st_mode)) {
            processedRoots.insert(root);
            continue;
        }

        char tempRootDir[MAXPATHLEN];
        strlcpy(tempRootDir, tempRootsDir, MAXPATHLEN);
        strlcat(tempRootDir, "/XXXXXXXX", MAXPATHLEN);
        mkdtemp(tempRootDir);

        if (endsWith(root, ".cpio") || endsWith(root, ".cpio.gz") || endsWith(root, ".cpgz") || endsWith(root, ".cpio.bz2") || endsWith(root, ".cpbz2") || endsWith(root, ".pax") || endsWith(root, ".pax.gz") || endsWith(root, ".pgz") || endsWith(root, ".pax.bz2") || endsWith(root, ".pbz2")) {
            args[0] = (char*)"/usr/bin/ditto";
            args[1] = (char*)"-x";
            args[2] = (char*)root.c_str();
            args[3] = tempRootDir;
            args[4] = nullptr;
        } else if (endsWith(root, ".tar")) {
            args[0] = (char*)"/usr/bin/tar";
            args[1] = (char*)"xf";
            args[2] = (char*)root.c_str();
            args[3] = (char*)"-C";
            args[4] = tempRootDir;
            args[5] = nullptr;
        } else if (endsWith(root, ".tar.gz") || endsWith(root, ".tgz")) {
            args[0] = (char*)"/usr/bin/tar";
            args[1] = (char*)"xzf";
            args[2] = (char*)root.c_str();
            args[3] = (char*)"-C";
            args[4] = tempRootDir;
            args[5] = nullptr;
        } else if (endsWith(root, ".tar.bz2")
            || endsWith(root, ".tbz2")
            || endsWith(root, ".tbz")) {
            args[0] = (char*)"/usr/bin/tar";
            args[1] = (char*)"xjf";
            args[2] = (char*)root.c_str();
            args[3] = (char*)"-C";
            args[4] = tempRootDir;
            args[5] = nullptr;
        } else if (endsWith(root, ".zip")) {
            args[0] = (char*)"/usr/bin/ditto";
            args[1] = (char*)"-xk";
            args[2] = (char*)root.c_str();
            args[3] = tempRootDir;
            args[4] = nullptr;
        } else {
            diags.error("unknown archive type: %s", root.c_str());
            continue;
        }

        if (res != runCommandAndWait(diags, args)) {
            fprintf(stderr, "Could not expand archive %s: %s (%d)", root.c_str(), strerror(res), res);
            exit(-1);
        }
        for (auto& existingRoot : processedRoots) {
            if (existingRoot == tempRootDir)
                return;
        }

        processedRoots.insert(tempRootDir);
    }

    roots = processedRoots;
}

void writeRootList(const std::string& dstRoot, const std::set<std::string>& roots)
{
    if (roots.size() == 0)
        return;

    std::string rootFile = dstRoot + "/roots.txt";
    FILE*       froots = ::fopen(rootFile.c_str(), "w");
    if (froots == NULL)
        return;

    for (auto& root : roots) {
        fprintf(froots, "%s\n", root.c_str());
    }

    ::fclose(froots);
}

BOMCopierCopyOperation filteredCopyIncludingPaths(BOMCopier copier, const char* path, BOMFSObjType type, off_t size)
{
    std::string absolutePath = &path[1];
    void *userData = BOMCopierUserData(copier);
    std::set<std::string> *cachePaths = (std::set<std::string>*)userData;
    for (const std::string& cachePath : *cachePaths) {
        if (startsWith(cachePath, absolutePath))
            return BOMCopierContinue;
    }
    if (cachePaths->count(absolutePath)) {
        return BOMCopierContinue;
    }
    return BOMCopierSkipFile;
}

static Disposition stringToDisposition(Diagnostics& diags, const std::string& str) {
    if (diags.hasError())
        return Unknown;
    if (str == "Unknown")
        return Unknown;
    if (str == "InternalDevelopment")
        return InternalDevelopment;
    if (str == "Customer")
        return Customer;
    if (str == "InternalMinDevelopment")
        return InternalMinDevelopment;
    return Unknown;
}

static Platform stringToPlatform(Diagnostics& diags, const std::string& str) {
    if (diags.hasError())
        return unknown;
    if (str == "unknown")
        return unknown;
    if (str == "macOS")
        return macOS;
    if (str == "iOS")
        return iOS;
    if (str == "tvOS")
        return tvOS;
    if (str == "watchOS")
        return watchOS;
    if (str == "bridgeOS")
        return bridgeOS;
    if (str == "iOSMac")
        return iOSMac;
    if (str == "UIKitForMac")
        return iOSMac;
    if (str == "iOS_simulator")
        return iOS_simulator;
    if (str == "tvOS_simulator")
        return tvOS_simulator;
    if (str == "watchOS_simulator")
        return watchOS_simulator;
    return unknown;
}

static FileFlags stringToFileFlags(Diagnostics& diags, const std::string& str) {
    if (diags.hasError())
        return NoFlags;
    if (str == "NoFlags")
        return NoFlags;
    if (str == "MustBeInCache")
        return MustBeInCache;
    if (str == "ShouldBeExcludedFromCacheIfUnusedLeaf")
        return ShouldBeExcludedFromCacheIfUnusedLeaf;
    if (str == "RequiredClosure")
        return RequiredClosure;
    if (str == "DylibOrderFile")
        return DylibOrderFile;
    if (str == "DirtyDataOrderFile")
        return DirtyDataOrderFile;
    return NoFlags;
}

struct SharedCacheBuilderOptions {
    Diagnostics           diags;
    std::set<std::string> roots;
    std::string           dylibCacheDir;
    std::string           artifactDir;
    std::string           release;
    bool                  emitDevCaches = true;
    bool                  emitElidedDylibs = true;
    bool                  listConfigs = false;
    bool                  copyRoots = false;
    bool                  debug = false;
    bool                  useMRM = false;
    std::string           dstRoot;
    std::string           emitJSONPath;
    std::string           buildAllPath;
    std::string           resultPath;
    std::string           baselineDifferenceResultPath;
    std::string           baselineCacheMapPath;
    bool                  baselineCopyRoots = false;
};

static void loadMRMFiles(Diagnostics& diags,
                         MRMSharedCacheBuilder* sharedCacheBuilder,
                         const std::vector<std::tuple<std::string, std::string, FileFlags>>& inputFiles,
                         std::vector<std::pair<const void*, size_t>>& mappedFiles,
                         const std::set<std::string>& baselineCacheFiles) {

    for (const std::tuple<std::string, std::string, FileFlags>& inputFile : inputFiles) {
        const std::string& buildPath   = std::get<0>(inputFile);
        const std::string& runtimePath = std::get<1>(inputFile);
        FileFlags   fileFlags   = std::get<2>(inputFile);

        struct stat stat_buf;
        int fd = ::open(buildPath.c_str(), O_RDONLY, 0);
        if (fd == -1) {
            if (baselineCacheFiles.count(runtimePath)) {
                diags.error("can't open file '%s', errno=%d\n", buildPath.c_str(), errno);
                return;
            } else {
                diags.verbose("can't open file '%s', errno=%d\n", buildPath.c_str(), errno);
                continue;
            }
        }

        if (fstat(fd, &stat_buf) == -1) {
            if (baselineCacheFiles.count(runtimePath)) {
                diags.error("can't stat open file '%s', errno=%d\n", buildPath.c_str(), errno);
                ::close(fd);
                return;
            } else {
                diags.verbose("can't stat open file '%s', errno=%d\n", buildPath.c_str(), errno);
                ::close(fd);
                continue;
            }
        }

        const void* buffer = mmap(NULL, (size_t)stat_buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (buffer == MAP_FAILED) {
            diags.error("mmap() for file at %s failed, errno=%d\n", buildPath.c_str(), errno);
            ::close(fd);
        }
        ::close(fd);

        mappedFiles.emplace_back(buffer, (size_t)stat_buf.st_size);

        addFile(sharedCacheBuilder, runtimePath.c_str(), (uint8_t*)buffer, (size_t)stat_buf.st_size, fileFlags);
    }
}

static void unloadMRMFiles(std::vector<std::pair<const void*, size_t>>& mappedFiles) {
    for (auto mappedFile : mappedFiles)
        ::munmap((void*)mappedFile.first, mappedFile.second);
}

static void writeMRMResults(bool cacheBuildSuccess, MRMSharedCacheBuilder* sharedCacheBuilder, const SharedCacheBuilderOptions& options) {
    if (!cacheBuildSuccess) {
        uint64_t errorCount = 0;
        if (const char* const* errors = getErrors(sharedCacheBuilder, &errorCount)) {
            for (uint64_t i = 0, e = errorCount; i != e; ++i) {
                const char* errorMessage = errors[i];
                fprintf(stderr, "ERROR: %s\n", errorMessage);
            }
        }
    }

    // Now emit each cache we generated, or the errors for them.
    uint64_t cacheResultCount = 0;
    if (const CacheResult* const* cacheResults = getCacheResults(sharedCacheBuilder, &cacheResultCount)) {
        for (uint64_t i = 0, e = cacheResultCount; i != e; ++i) {
            const CacheResult& result = *(cacheResults[i]);
            // Always print the warnings if we have roots, even if there are errors
            if ( (result.numErrors == 0) || !options.roots.empty() ) {
                for (uint64_t warningIndex = 0; warningIndex != result.numWarnings; ++warningIndex) {
                    fprintf(stderr, "[%s] WARNING: %s\n", result.loggingPrefix, result.warnings[warningIndex]);
                }
            }
            if (result.numErrors) {
                for (uint64_t errorIndex = 0; errorIndex != result.numErrors; ++errorIndex) {
                    fprintf(stderr, "[%s] ERROR: %s\n", result.loggingPrefix, result.errors[errorIndex]);
                }
                cacheBuildSuccess = false;
            }
        }
    }

    if (!cacheBuildSuccess) {
        exit(-1);
    }

    // If we built caches, then write everything out.
    // TODO: Decide if we should we write any good caches anyway?
    if (cacheBuildSuccess && !options.dstRoot.empty()) {
        uint64_t fileResultCount = 0;
        if (const FileResult* const* fileResults = getFileResults(sharedCacheBuilder, &fileResultCount)) {
            for (uint64_t i = 0, e = fileResultCount; i != e; ++i) {
                const FileResult& result = *(fileResults[i]);

                switch (result.behavior) {
                    case AddFile:
                        break;
                    case ChangeFile:
                        continue;
                }

                if (!result.data)
                    continue;

                const std::string path = options.dstRoot + result.path;
                std::string pathTemplate = path + "-XXXXXX";
                size_t templateLen = strlen(pathTemplate.c_str())+2;
                char pathTemplateSpace[templateLen];
                strlcpy(pathTemplateSpace, pathTemplate.c_str(), templateLen);
                int fd = mkstemp(pathTemplateSpace);
                if ( fd != -1 ) {
                    ::ftruncate(fd, result.size);
                    uint64_t writtenSize = pwrite(fd, result.data, result.size, 0);
                    if ( writtenSize == result.size ) {
                        ::fchmod(fd, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH); // mkstemp() makes file "rw-------", switch it to "rw-r--r--"
                        if ( ::rename(pathTemplateSpace, path.c_str()) == 0) {
                            ::close(fd);
                            continue; // success
                        }
                    }
                    else {
                        fprintf(stderr, "ERROR: could not write file %s\n", pathTemplateSpace);
                        cacheBuildSuccess = false;
                    }
                    ::close(fd);
                    ::unlink(pathTemplateSpace);
                }
                else {
                    fprintf(stderr, "ERROR: could not open file %s\n", pathTemplateSpace);
                    cacheBuildSuccess = false;
                }
            }
        }
    }
}

static void buildCacheFromJSONManifest(Diagnostics& diags, const SharedCacheBuilderOptions& options,
                                       const std::string& jsonManifestPath) {
    dyld3::json::Node manifestNode = dyld3::json::readJSON(diags, jsonManifestPath.c_str());
    if (diags.hasError())
        return;

    // Top level node should be a map of the options, files, and symlinks.
    if (manifestNode.map.empty()) {
        diags.error("Expected map for JSON manifest node\n");
        return;
    }

    // Parse the nodes in the top level manifest node
    const dyld3::json::Node& versionNode          = dyld3::json::getRequiredValue(diags, manifestNode, "version");
    uint64_t manifestVersion                      = dyld3::json::parseRequiredInt(diags, versionNode);
    if (diags.hasError())
        return;

    const uint64_t supportedManifestVersion = 1;
    if (manifestVersion != supportedManifestVersion) {
        diags.error("JSON manfiest version of %lld is unsupported.  Supported version is %lld\n",
                    manifestVersion, supportedManifestVersion);
        return;
    }
    const dyld3::json::Node& buildOptionsNode     = dyld3::json::getRequiredValue(diags, manifestNode, "buildOptions");
    const dyld3::json::Node& filesNode            = dyld3::json::getRequiredValue(diags, manifestNode, "files");
    const dyld3::json::Node* symlinksNode         = dyld3::json::getOptionalValue(diags, manifestNode, "symlinks");

    // Parse the archs
    const dyld3::json::Node& archsNode = dyld3::json::getRequiredValue(diags, buildOptionsNode, "archs");
    if (diags.hasError())
        return;
    if (archsNode.array.empty()) {
        diags.error("Build options archs node is not an array\n");
        return;
    }
    const char* archs[archsNode.array.size()];
    uint64_t archIndex = 0;
    for (const dyld3::json::Node& archNode : archsNode.array) {
        archs[archIndex++] = dyld3::json::parseRequiredString(diags, archNode).c_str();
    }

    // Parse the rest of the options node.
    BuildOptions_v1 buildOptions;
    buildOptions.version                            = dyld3::json::parseRequiredInt(diags, dyld3::json::getRequiredValue(diags, buildOptionsNode, "version"));
    buildOptions.updateName                         = dyld3::json::parseRequiredString(diags, dyld3::json::getRequiredValue(diags, buildOptionsNode, "updateName")).c_str();
    buildOptions.deviceName                         = dyld3::json::parseRequiredString(diags, dyld3::json::getRequiredValue(diags, buildOptionsNode, "deviceName")).c_str();
    buildOptions.disposition                        = stringToDisposition(diags, dyld3::json::parseRequiredString(diags, dyld3::json::getRequiredValue(diags, buildOptionsNode, "disposition")));
    buildOptions.platform                           = stringToPlatform(diags, dyld3::json::parseRequiredString(diags, dyld3::json::getRequiredValue(diags, buildOptionsNode, "platform")));
    buildOptions.archs                              = archs;
    buildOptions.numArchs                           = archsNode.array.size();
    buildOptions.verboseDiagnostics                 = options.debug;
    buildOptions.isLocallyBuiltCache                = true;

    if (diags.hasError())
        return;

    // Override the disposition if we don't want certaion caches.
    if (!options.emitDevCaches) {
        switch (buildOptions.disposition) {
            case Unknown:
                // Nothing we can do here as we can't assume what caches are built here.
                break;
            case InternalDevelopment:
                // This builds both caches, but we don't want dev
                buildOptions.disposition = Customer;
                break;
            case Customer:
                // This is already only the customer cache
                break;
            case InternalMinDevelopment:
                diags.error("Cannot request no dev cache for InternalMinDevelopment as that is already only a dev cache\n");
                break;
        }
    }

    if (diags.hasError())
        return;

    struct MRMSharedCacheBuilder* sharedCacheBuilder = createSharedCacheBuilder(&buildOptions);

    // Parse the files
    if (filesNode.array.empty()) {
        diags.error("Build options files node is not an array\n");
        return;
    }

    std::vector<std::tuple<std::string, std::string, FileFlags>> inputFiles;
    for (const dyld3::json::Node& fileNode : filesNode.array) {
        const std::string& path = dyld3::json::parseRequiredString(diags, dyld3::json::getRequiredValue(diags, fileNode, "path")).c_str();
        FileFlags fileFlags     = stringToFileFlags(diags, dyld3::json::parseRequiredString(diags, dyld3::json::getRequiredValue(diags, fileNode, "flags")));

        // We can optionally have a sourcePath entry which is the path to get the source content from instead of the install path
        std::string sourcePath;
        const dyld3::json::Node* sourcePathNode = dyld3::json::getOptionalValue(diags, fileNode, "sourcePath");
        if ( sourcePathNode != nullptr ) {
            if (!sourcePathNode->array.empty()) {
                diags.error("sourcePath node cannot be an array\n");
                return;
            }
            if (!sourcePathNode->map.empty()) {
                diags.error("sourcePath node cannot be a map\n");
                return;
            }
            sourcePath = sourcePathNode->value;
        } else {
            sourcePath = path;
        }

        // Check if one of the -root's has this path
        bool foundInOverlay = false;
        for (const std::string& overlay : options.roots) {
            struct stat sb;
            std::string filePath = overlay + path;
            if (!stat(filePath.c_str(), &sb)) {
                foundInOverlay = true;
                diags.verbose("Taking '%s' from overlay instead of dylib cache\n", path.c_str());
                inputFiles.push_back({ filePath, path, fileFlags });
                break;
            }
        }

        if (foundInOverlay)
            continue;

        // Build paths are relative to the build artifact root directory.
        std::string buildPath;
        switch (fileFlags) {
            case NoFlags:
            case MustBeInCache:
            case ShouldBeExcludedFromCacheIfUnusedLeaf:
            case RequiredClosure:
                buildPath = "." + sourcePath;
                break;
            case DylibOrderFile:
            case DirtyDataOrderFile:
                buildPath = "." + sourcePath;
                break;
        }
        inputFiles.push_back({ buildPath, path, fileFlags });
    }

    if (diags.hasError())
        return;

    // Parse the baseline from the map if we have it
    std::set<std::string> baselineDylibs;
    if ( !options.baselineCacheMapPath.empty() ) {
        dyld3::json::Node mapNode = dyld3::json::readJSON(diags, options.baselineCacheMapPath.c_str());
        if (diags.hasError())
            return;

        // Top level node should be a map of the version and files
        if (mapNode.map.empty()) {
            diags.error("Expected map for JSON cache map node\n");
            return;
        }

        // Parse the nodes in the top level manifest node
        const dyld3::json::Node& versionNode     = dyld3::json::getRequiredValue(diags, mapNode, "version");
        uint64_t mapVersion                      = dyld3::json::parseRequiredInt(diags, versionNode);
        if (diags.hasError())
            return;

        const uint64_t supportedMapVersion = 1;
        if (mapVersion != supportedMapVersion) {
            diags.error("JSON map version of %lld is unsupported.  Supported version is %lld\n",
                        mapVersion, supportedMapVersion);
            return;
        }

        // Parse the images
        const dyld3::json::Node& imagesNode = dyld3::json::getRequiredValue(diags, mapNode, "images");
        if (diags.hasError())
            return;
        if (imagesNode.array.empty()) {
            diags.error("Images node is not an array\n");
            return;
        }

        for (const dyld3::json::Node& imageNode : imagesNode.array) {
            const dyld3::json::Node& pathNode = dyld3::json::getRequiredValue(diags, imageNode, "path");
            if (pathNode.value.empty()) {
                diags.error("Image path node is not a string\n");
                return;
            }
            baselineDylibs.insert(pathNode.value);
        }
    }

    std::vector<std::pair<const void*, size_t>> mappedFiles;
    loadMRMFiles(diags, sharedCacheBuilder, inputFiles, mappedFiles, baselineDylibs);

    if (diags.hasError())
        return;

    // Parse the symlinks if we have them
    if (symlinksNode) {
        if (symlinksNode->array.empty()) {
            diags.error("Build options symlinks node is not an array\n");
            return;
        }
        for (const dyld3::json::Node& symlinkNode : symlinksNode->array) {
            const std::string& fromPath = dyld3::json::parseRequiredString(diags, dyld3::json::getRequiredValue(diags, symlinkNode, "path")).c_str();
            const std::string& toPath   = dyld3::json::parseRequiredString(diags, dyld3::json::getRequiredValue(diags, symlinkNode, "target")).c_str();
            addSymlink(sharedCacheBuilder, fromPath.c_str(), toPath.c_str());
        }
    }

    if (diags.hasError())
        return;

    // Don't create a directory if we are skipping writes, which means we have no dstRoot set
    if (!options.dstRoot.empty())
        (void)mkpath_np((options.dstRoot + "/System/Library/Caches/com.apple.dyld/").c_str(), 0755);

    // Actually build the cache.
    bool cacheBuildSuccess = runSharedCacheBuilder(sharedCacheBuilder);

    // Compare this cache to the baseline cache and see if we have any roots to copy over
    if (!options.baselineDifferenceResultPath.empty() || options.baselineCopyRoots) {
        std::set<std::string> newDylibs;
        if (cacheBuildSuccess) {
            uint64_t fileResultCount = 0;
            if (const char* const* fileResults = getFilesToRemove(sharedCacheBuilder, &fileResultCount)) {
                for (uint64_t i = 0; i != fileResultCount; ++i)
                    newDylibs.insert(fileResults[i]);
            }
        }

        if (options.baselineCopyRoots) {
            // Work out the set of dylibs in the old cache but not the new one
            std::set<std::string> dylibsMissingFromNewCache;
            for (const std::string& baselineDylib : baselineDylibs) {
                if (!newDylibs.count(baselineDylib))
                    dylibsMissingFromNewCache.insert(baselineDylib);
            }

            if (!dylibsMissingFromNewCache.empty()) {
                BOMCopier copier = BOMCopierNewWithSys(BomSys_default());
                BOMCopierSetUserData(copier, (void*)&dylibsMissingFromNewCache);
                BOMCopierSetCopyFileStartedHandler(copier, filteredCopyIncludingPaths);
                std::string dylibCacheRootDir = realFilePath(options.dylibCacheDir);
                if (dylibCacheRootDir == "") {
                    fprintf(stderr, "Could not find dylib Root directory to copy baseline roots from\n");
                    exit(1);
                }
                BOMCopierCopy(copier, dylibCacheRootDir.c_str(), options.dstRoot.c_str());
                BOMCopierFree(copier);

                for (const std::string& dylibMissingFromNewCache : dylibsMissingFromNewCache) {
                    diags.verbose("Dylib missing from new cache: '%s'\n", dylibMissingFromNewCache.c_str());
                }
            }
        }

        if (!options.baselineDifferenceResultPath.empty()) {
            auto cppToObjStr = [](const std::string& str) {
                return [NSString stringWithUTF8String:str.c_str()];
            };

            // Work out the set of dylibs in the cache and taken from the -root
            NSMutableArray<NSString*>* dylibsFromRoots = [NSMutableArray array];
            for (auto& root : options.roots) {
                for (const std::string& dylibInstallName : newDylibs) {
                    struct stat sb;
                    std::string filePath = root + "/" + dylibInstallName;
                    if (!stat(filePath.c_str(), &sb)) {
                        [dylibsFromRoots addObject:cppToObjStr(dylibInstallName)];
                    }
                }
            }

            // Work out the set of dylibs in the new cache but not in the baseline cache.
            NSMutableArray<NSString*>* dylibsMissingFromBaselineCache = [NSMutableArray array];
            for (const std::string& newDylib : newDylibs) {
                if (!baselineDylibs.count(newDylib))
                    [dylibsMissingFromBaselineCache addObject:cppToObjStr(newDylib)];
            }

            NSMutableDictionary* cacheDict = [[NSMutableDictionary alloc] init];
            cacheDict[@"root-paths-in-cache"] = dylibsFromRoots;
            cacheDict[@"device-paths-to-delete"] = dylibsMissingFromBaselineCache;

            NSError* error = nil;
            NSData*  outData = [NSPropertyListSerialization dataWithPropertyList:cacheDict
                                                                          format:NSPropertyListBinaryFormat_v1_0
                                                                         options:0
                                                                           error:&error];
            (void)[outData writeToFile:cppToObjStr(options.baselineDifferenceResultPath) atomically:YES];
        }
    }

    writeMRMResults(cacheBuildSuccess, sharedCacheBuilder, options);

    destroySharedCacheBuilder(sharedCacheBuilder);

    unloadMRMFiles(mappedFiles);
}

int main(int argc, const char* argv[])
{
    @autoreleasepool {
        __block Diagnostics diags;
        SharedCacheBuilderOptions options;
        std::string jsonManifestPath;
        char* tempRootsDir = strdup("/tmp/dyld_shared_cache_builder.XXXXXX");

        mkdtemp(tempRootsDir);

        for (int i = 1; i < argc; ++i) {
            const char* arg = argv[i];
            if (arg[0] == '-') {
                if (strcmp(arg, "-debug") == 0) {
                    diags = Diagnostics(true);
                    options.debug = true;
                } else if (strcmp(arg, "-list_configs") == 0) {
                    options.listConfigs = true;
                } else if (strcmp(arg, "-root") == 0) {
                    std::string realpath = realPath(argv[++i]);
                    if ( realpath.empty() || !fileExists(realpath) ) {
                        fprintf(stderr, "-root path doesn't exist: %s\n", argv[i]);
                        exit(-1);
                    }
                    options.roots.insert(realpath);
                } else if (strcmp(arg, "-copy_roots") == 0) {
                    options.copyRoots = true;
                } else if (strcmp(arg, "-dylib_cache") == 0) {
                    options.dylibCacheDir = realPath(argv[++i]);
                } else if (strcmp(arg, "-artifact") == 0) {
                    options.artifactDir = realPath(argv[++i]);
                } else if (strcmp(arg, "-no_development_cache") == 0) {
                    options.emitDevCaches = false;
                } else if (strcmp(arg, "-no_overflow_dylibs") == 0) {
                    options.emitElidedDylibs = false;
                } else if (strcmp(arg, "-development_cache") == 0) {
                    options.emitDevCaches = true;
                } else if (strcmp(arg, "-overflow_dylibs") == 0) {
                    options.emitElidedDylibs = true;
                } else if (strcmp(arg, "-mrm") == 0) {
                    options.useMRM = true;
                } else if (strcmp(arg, "-emit_json") == 0) {
                    options.emitJSONPath = realPath(argv[++i]);
                } else if (strcmp(arg, "-json_manifest") == 0) {
                    jsonManifestPath = realPath(argv[++i]);
                } else if (strcmp(arg, "-build_all") == 0) {
                    options.buildAllPath = realPath(argv[++i]);
                } else if (strcmp(arg, "-dst_root") == 0) {
                    options.dstRoot = realPath(argv[++i]);
                } else if (strcmp(arg, "-release") == 0) {
                    options.release = argv[++i];
                } else if (strcmp(arg, "-results") == 0) {
                    options.resultPath = realPath(argv[++i]);
                } else if (strcmp(arg, "-baseline_diff_results") == 0) {
                    options.baselineDifferenceResultPath = realPath(argv[++i]);
                } else if (strcmp(arg, "-baseline_copy_roots") == 0) {
                    options.baselineCopyRoots = true;
                } else if (strcmp(arg, "-baseline_cache_map") == 0) {
                    options.baselineCacheMapPath = realPath(argv[++i]);
                } else {
                    //usage();
                    fprintf(stderr, "unknown option: %s\n", arg);
                    exit(-1);
                }
            } else {
                fprintf(stderr, "unknown option: %s\n", arg);
                exit(-1);
            }
        }
        (void)options.emitElidedDylibs; // not implemented yet

        time_t mytime = time(0);
        fprintf(stderr, "Started: %s", asctime(localtime(&mytime)));
        processRoots(diags, options.roots, tempRootsDir);

        struct rlimit rl = { OPEN_MAX, OPEN_MAX };
        (void)setrlimit(RLIMIT_NOFILE, &rl);

        if (options.dylibCacheDir.empty() && options.artifactDir.empty() && options.release.empty()) {
            fprintf(stderr, "you must specify either -dylib_cache, -artifact or -release\n");
            exit(-1);
        } else if (!options.dylibCacheDir.empty() && !options.release.empty()) {
            fprintf(stderr, "you may not use -dylib_cache and -release at the same time\n");
            exit(-1);
        } else if (!options.dylibCacheDir.empty() && !options.artifactDir.empty()) {
            fprintf(stderr, "you may not use -dylib_cache and -artifact at the same time\n");
            exit(-1);
        }

        if (jsonManifestPath.empty() && options.buildAllPath.empty()) {
            fprintf(stderr, "Must specify a -json_manifest path OR a -build_all path\n");
            exit(-1);
        }

        if (!options.buildAllPath.empty()) {
            if (!options.dstRoot.empty()) {
                fprintf(stderr, "Cannot combine -dst_root and -build_all\n");
                exit(-1);
            }
            if (!jsonManifestPath.empty()) {
                fprintf(stderr, "Cannot combine -json_manifest and -build_all\n");
                exit(-1);
            }
            if (!options.baselineDifferenceResultPath.empty()) {
                fprintf(stderr, "Cannot combine -baseline_diff_results and -build_all\n");
                exit(-1);
            }
            if (options.baselineCopyRoots) {
                fprintf(stderr, "Cannot combine -baseline_copy_roots and -build_all\n");
                exit(-1);
            }
            if (!options.baselineCacheMapPath.empty()) {
                fprintf(stderr, "Cannot combine -baseline_cache_map and -build_all\n");
                exit(-1);
            }
        } else if (!options.listConfigs) {
            if (options.dstRoot.empty()) {
                fprintf(stderr, "Must specify a valid -dst_root OR -list_configs\n");
                exit(-1);
            }

            if (jsonManifestPath.empty()) {
                fprintf(stderr, "Must specify a -json_manifest path OR -list_configs\n");
                exit(-1);
            }
        }

        if (!options.baselineDifferenceResultPath.empty() && (options.roots.size() > 1)) {
            fprintf(stderr, "Cannot use -baseline_diff_results with more that one -root\n");
            exit(-1);
        }

        // Some options don't work with a JSON manifest
        if (!jsonManifestPath.empty()) {
            if (!options.resultPath.empty()) {
                fprintf(stderr, "Cannot use -results with -json_manifest\n");
                exit(-1);
            }
            if (!options.baselineDifferenceResultPath.empty() && options.baselineCacheMapPath.empty()) {
                fprintf(stderr, "Must use -baseline_cache_map with -baseline_diff_results when using -json_manifest\n");
                exit(-1);
            }
            if (options.baselineCopyRoots && options.baselineCacheMapPath.empty()) {
                fprintf(stderr, "Must use -baseline_cache_map with -baseline_copy_roots when using -json_manifest\n");
                exit(-1);
            }
        } else {
            if (!options.baselineCacheMapPath.empty()) {
                fprintf(stderr, "Cannot use -baseline_cache_map without -json_manifest\n");
                exit(-1);
            }
        }

        if (!options.baselineCacheMapPath.empty()) {
            if (options.baselineDifferenceResultPath.empty() && options.baselineCopyRoots) {
                fprintf(stderr, "Must use -baseline_cache_map with -baseline_diff_results or -baseline_copy_roots\n");
                exit(-1);
            }
        }

        // Find all the JSON files if we use -build_all
        __block std::vector<std::string> jsonPaths;
        if (!options.buildAllPath.empty()) {
            struct stat stat_buf;
            if (stat(options.buildAllPath.c_str(), &stat_buf) != 0) {
                fprintf(stderr, "Could not find -build_all path '%s'\n", options.buildAllPath.c_str());
                exit(-1);
            }

            if ( (stat_buf.st_mode & S_IFMT) != S_IFDIR ) {
                fprintf(stderr, "-build_all path is not a directory '%s'\n", options.buildAllPath.c_str());
                exit(-1);
            }

            auto processFile = ^(const std::string& path, const struct stat& statBuf) {
                if ( !endsWith(path, ".json") )
                    return;

                jsonPaths.push_back(path);
            };

            iterateDirectoryTree("", options.buildAllPath,
                                 ^(const std::string& dirPath) { return false; },
                                 processFile, true /* process files */, false /* recurse */);

            if (jsonPaths.empty()) {
                fprintf(stderr, "Didn't find any .json files inside -build_all path: %s\n", options.buildAllPath.c_str());
                exit(-1);
            }

            if (options.listConfigs) {
                for (const std::string& path : jsonPaths) {
                    fprintf(stderr, "Found config: %s\n", path.c_str());
                }
                exit(-1);
            }
        }

        if (!options.artifactDir.empty()) {
            // Find the dylib cache dir from inside the artifact dir
            struct stat stat_buf;
            if (stat(options.artifactDir.c_str(), &stat_buf) != 0) {
                fprintf(stderr, "Could not find artifact path '%s'\n", options.artifactDir.c_str());
                exit(-1);
            }
            std::string dir = options.artifactDir + "/AppleInternal/Developer/DylibCaches";
            if (stat(dir.c_str(), &stat_buf) != 0) {
                fprintf(stderr, "Could not find artifact path '%s'\n", dir.c_str());
                exit(-1);
            }

            if (!options.release.empty()) {
                // Use the given release
                options.dylibCacheDir = dir + "/" + options.release + ".dlc";
            } else {
                // Find a release directory
                __block std::vector<std::string> subDirectories;
                iterateDirectoryTree("", dir, ^(const std::string& dirPath) {
                    subDirectories.push_back(dirPath);
                    return false;
                }, nullptr, false, false);

                if (subDirectories.empty()) {
                    fprintf(stderr, "Could not find dlc subdirectories inside '%s'\n", dir.c_str());
                    exit(-1);
                }

                if (subDirectories.size() > 1) {
                    fprintf(stderr, "Found too many subdirectories inside artifact path '%s'.  Use -release to select one\n", dir.c_str());
                    exit(-1);
                }

                options.dylibCacheDir = subDirectories.front();
            }
        }

        if (options.dylibCacheDir.empty()) {
            options.dylibCacheDir = std::string("/AppleInternal/Developer/DylibCaches/") + options.release + ".dlc";
        }

        //Move into the dir so we can use relative path manifests
        chdir(options.dylibCacheDir.c_str());

        dispatch_async(dispatch_get_main_queue(), ^{
            if (!options.buildAllPath.empty()) {
                bool requiresConcurrencyLimit = false;
                dispatch_semaphore_t concurrencyLimit = NULL;
                // Try build 1 cache per 8GB of RAM
                uint64_t memSize = 0;
                size_t sz = sizeof(memSize);
                if ( sysctlbyname("hw.memsize", &memSize, &sz, NULL, 0) == 0 ) {
                    uint64_t maxThreads = std::max(memSize / 0x200000000ULL, 1ULL);
                    fprintf(stderr, "Detected %lldGb or less of memory, limiting concurrency to %lld threads\n",
                            memSize / (1 << 30), maxThreads);
                    requiresConcurrencyLimit = true;
                    concurrencyLimit = dispatch_semaphore_create(maxThreads);
                }

                dispatch_apply(jsonPaths.size(), DISPATCH_APPLY_AUTO, ^(size_t index) {
                    // Horrible hack to limit concurrency in low spec build machines.
                    if (requiresConcurrencyLimit) { dispatch_semaphore_wait(concurrencyLimit, DISPATCH_TIME_FOREVER); }

                    const std::string& jsonPath = jsonPaths[index];
                    buildCacheFromJSONManifest(diags, options, jsonPath);

                    if (requiresConcurrencyLimit) { dispatch_semaphore_signal(concurrencyLimit); }
                });
            } else {
                buildCacheFromJSONManifest(diags, options, jsonManifestPath);
            }

            const char* args[8];
            args[0] = (char*)"/bin/rm";
            args[1] = (char*)"-rf";
            args[2] = (char*)tempRootsDir;
            args[3] = nullptr;
            (void)runCommandAndWait(diags, args);

            if (diags.hasError()) {
                fprintf(stderr, "dyld_shared_cache_builder: error: %s", diags.errorMessage().c_str());
                exit(-1);
            }

            for (const std::string& warn : diags.warnings()) {
                fprintf(stderr, "dyld_shared_cache_builder: warning: %s\n", warn.c_str());
            }

            // Finally, write the roots.txt to tell us which roots we pulled in
            if (!options.dstRoot.empty())
                writeRootList(options.dstRoot + "/System/Library/Caches/com.apple.dyld", options.roots);
            exit(0);
        });
    }

    dispatch_main();

    return 0;
}
