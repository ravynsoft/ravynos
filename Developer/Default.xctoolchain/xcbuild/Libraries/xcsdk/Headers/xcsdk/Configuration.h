/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcsdk_Configuration_h
#define __xcsdk_Configuration_h

#include <string>
#include <vector>
#include <ext/optional>

namespace libutil { class Filesystem; }
namespace process { class Context; }
namespace process { class User; }

namespace xcsdk {

/*
 * Configuration for loading SDKs.
 */
class Configuration {
private:
    std::vector<std::string> _extraPlatformsPaths;
    std::vector<std::string> _extraToolchainsPaths;

public:
    Configuration(
        std::vector<std::string> const &extraPlatformsPaths,
        std::vector<std::string> const &extraToolchainsPaths);

public:
    /*
     * Paths to search for additional platforms.
     */
    std::vector<std::string> const &extraPlatformsPaths()  const
    { return _extraPlatformsPaths; }

    /*
     * Paths to search for additional toolchains.
     */
    std::vector<std::string> const &extraToolchainsPaths() const
    { return _extraToolchainsPaths; }

public:
    /*
     * The default paths for the configuration.
     */
    static std::vector<std::string> DefaultPaths(
        process::User const *user,
        process::Context const *processContext);

    /*
     * Loads a configuration from the filesystem given a list of
     * possible locations. The configuration loaded is the first valid
     * location.
     */
    static ext::optional<Configuration> Load(
        libutil::Filesystem const *filesystem,
        std::vector<std::string> const &paths);
};

}

#endif  // !__xcsdk_Configuration_h
