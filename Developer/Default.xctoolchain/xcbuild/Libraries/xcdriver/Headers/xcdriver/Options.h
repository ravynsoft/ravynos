/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcdriver_Options_h
#define __xcdriver_Options_h

#include <pbxsetting/Setting.h>
#include <libutil/Options.h>

#include <string>
#include <vector>
#include <utility>

namespace xcdriver {

class Options {
private:
    ext::optional<bool>        _usage;
    ext::optional<bool>        _help;
    ext::optional<bool>        _license;
    ext::optional<bool>        _checkFirstLaunchStatus;
    ext::optional<bool>        _version;

private:
    ext::optional<bool>        _verbose;
    ext::optional<bool>        _quiet;
    ext::optional<bool>        _json;

private:
    ext::optional<std::string> _project;
    std::vector<std::string>   _target;
    ext::optional<bool>        _allTargets;
    ext::optional<std::string> _workspace;
    ext::optional<std::string> _scheme;
    ext::optional<std::string> _configuration;
    ext::optional<std::string> _arch;
    ext::optional<std::string> _sdk;
    ext::optional<std::string> _toolchain;
    ext::optional<std::string> _destination;
    ext::optional<std::string> _destinationTimeout;

private:
    ext::optional<std::string> _formatter;
    ext::optional<std::string> _executor;
    ext::optional<bool>        _generate;

private:
    ext::optional<bool>        _parallelizeTargets;
    ext::optional<int>         _jobs;
    ext::optional<bool>        _dryRun;
    ext::optional<bool>        _hideShellScriptEnvironment;

private:
    ext::optional<bool>        _list;
    ext::optional<bool>        _showSDKs;
    ext::optional<bool>        _showBuildSettings;

private:
    ext::optional<std::string> _xcconfig;
    std::vector<pbxsetting::Setting> _settings;

private:
    ext::optional<std::string> _findExecutable;
    ext::optional<std::string> _findLibrary;

private:
    ext::optional<bool>        _enableAddressSanitizer;
    ext::optional<bool>        _enableThreadSanitizer;
    ext::optional<bool>        _enableCodeCoverage;

private:
    ext::optional<std::string> _resultBundlePath;
    ext::optional<std::string> _derivedDataPath;

private:
    ext::optional<std::string> _xctestrun;
    std::vector<std::string>   _onlyTesting;
    std::vector<std::string>   _skipTesting;

private:
    ext::optional<bool>        _exportArchive;
    ext::optional<std::string> _archivePath;
    ext::optional<std::string> _exportPath;
    ext::optional<std::string> _exportOptionsPlist;

private:
    ext::optional<bool>        _skipUnavailableActions;
    std::vector<std::string>   _actions;

private:
    ext::optional<bool>        _exportLocalizations;
    ext::optional<bool>        _importLocalizations;
    ext::optional<std::string> _localizationPath;
    ext::optional<std::string> _exportLanguage;
    ext::optional<bool>        _forceImport;

public:
    Options();
    ~Options();

public:
    bool usage() const
    { return _usage.value_or(false); }
    bool help() const
    { return _help.value_or(false); }
    bool license() const
    { return _license.value_or(false); }
    bool checkFirstLaunchStatus() const
    { return _checkFirstLaunchStatus.value_or(false); }
    bool version() const
    { return _version.value_or(false); }

public:
    bool verbose() const
    { return _verbose.value_or(false); }
    bool quiet() const
    { return _quiet.value_or(false); }
    bool json() const
    { return _json.value_or(false); }

public:
    ext::optional<std::string> const &project() const
    { return _project; }
    std::vector<std::string> const &target() const
    { return _target; }
    bool allTargets() const
    { return _allTargets.value_or(false); }
    ext::optional<std::string> const &workspace() const
    { return _workspace; }
    ext::optional<std::string> const &scheme() const
    { return _scheme; }
    ext::optional<std::string> const &configuration() const
    { return _configuration; }
    ext::optional<std::string> const &arch() const
    { return _arch; }
    ext::optional<std::string> const &sdk() const
    { return _sdk; }
    ext::optional<std::string> const &toolchain() const
    { return _toolchain; }
    ext::optional<std::string> const &destination() const
    { return _destination; }
    ext::optional<std::string> const &destinationTimeout() const
    { return _destinationTimeout; }

public:
    /* Extension. */
    ext::optional<std::string> const &formatter() const
    { return _formatter; }
    /* Extension. */
    ext::optional<std::string> const &executor() const
    { return _executor; }
    /* Extension. */
    bool generate() const
    { return _generate.value_or(false); }

public:
    bool parallelizeTargets() const
    { return _parallelizeTargets.value_or(false); }
    ext::optional<int> jobs() const
    { return _jobs; }
    bool dryRun() const
    { return _dryRun.value_or(false); }
    bool hideShellScriptEnvironment() const
    { return _hideShellScriptEnvironment.value_or(false); }

public:
    bool list() const
    { return _list.value_or(false); }
    bool showSDKs() const
    { return _showSDKs.value_or(false); }
    bool showBuildSettings() const
    { return _showBuildSettings.value_or(false); }

public:
    ext::optional<std::string> const &xcconfig() const
    { return _xcconfig; }
    std::vector<pbxsetting::Setting> const &settings() const
    { return _settings; }

public:
    ext::optional<std::string> const &findExecutable() const
    { return _findExecutable; }
    ext::optional<std::string> const &findLibrary() const
    { return _findLibrary; }

public:
    bool enableAddressSanitizer() const
    { return _enableAddressSanitizer.value_or(false); }
    bool enableThreadSanitizer() const
    { return _enableThreadSanitizer.value_or(false); }
    bool enableCodeCoverage() const
    { return _enableCodeCoverage.value_or(false); }

public:
    ext::optional<std::string> const &resultBundlePath() const
    { return _resultBundlePath; }
    ext::optional<std::string> const &derivedDataPath() const
    { return _derivedDataPath; }

public:
    ext::optional<std::string> const &xctestrun() const
    { return _xctestrun; }
    std::vector<std::string> const &onlyTesting() const
    { return _onlyTesting; }
    std::vector<std::string> const &skipTesting() const
    { return _skipTesting; }

public:
    bool exportArchive() const
    { return _exportArchive.value_or(false); }
    ext::optional<std::string> const &archivePath() const
    { return _archivePath; }
    ext::optional<std::string> const &exportPath() const
    { return _exportPath; }
    ext::optional<std::string> const &exportOptionsPlist() const
    { return _exportOptionsPlist; }

public:
    bool skipUnavailableActions() const
    { return _skipUnavailableActions.value_or(false); }
    std::vector<std::string> const &actions() const
    { return _actions; }

public:
    bool exportLocalizations() const
    { return _exportLocalizations.value_or(false); }
    bool importLocalizations() const
    { return _importLocalizations.value_or(false); }
    ext::optional<std::string> const &localizationPath() const
    { return _localizationPath; }
    ext::optional<std::string> const &exportLanguage() const
    { return _exportLanguage; }
    bool forceImport() const
    { return _forceImport.value_or(false); }

private:
    friend class libutil::Options;
    std::pair<bool, std::string>
    parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it);
};

}

#endif // !__xcdriver_Options_h
