/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcscheme_XC_Scheme_h
#define __xcscheme_XC_Scheme_h

#ifdef __linux__
#include <cstdint>
#endif
#include <xcscheme/XC/Actions.h>

namespace libutil { class Filesystem; }

namespace xcscheme { namespace PBX {

class Project;

} }

namespace xcscheme { namespace XC {

class Scheme {
public:
    typedef std::shared_ptr <Scheme> shared_ptr;
    typedef std::vector <shared_ptr> vector;

private:
    std::string               _name;
    std::string               _path;
    std::string               _owner;

private:
    uint32_t                  _lastUpgradeVersion;
    std::string               _version;
    BuildAction::shared_ptr   _buildAction;
    TestAction::shared_ptr    _testAction;
    LaunchAction::shared_ptr  _launchAction;
    ProfileAction::shared_ptr _profileAction;
    AnalyzeAction::shared_ptr _analyzeAction;
    ArchiveAction::shared_ptr _archiveAction;

public:
    Scheme(std::string const &name, std::string const &owner);

public:
    inline bool shared() const
    { return _owner.empty(); }

public:
    inline std::string const &path() const
    { return _path; }

public:
    inline std::string const &name() const
    { return _name; }

public:
    inline std::string const &owner() const
    { return _owner; }

public:
    inline uint32_t lastUpgradeVersion() const
    { return _lastUpgradeVersion; }

public:
    inline std::string const &version() const
    { return _version; }

public:
    inline BuildAction::shared_ptr const &buildAction() const
    { return _buildAction; }
    inline BuildAction::shared_ptr &buildAction()
    { return _buildAction; }

public:
    inline TestAction::shared_ptr const &testAction() const
    { return _testAction; }
    inline TestAction::shared_ptr &testAction()
    { return _testAction; }

public:
    inline LaunchAction::shared_ptr const &launchAction() const
    { return _launchAction; }
    inline LaunchAction::shared_ptr &launchAction()
    { return _launchAction; }

public:
    inline ProfileAction::shared_ptr const &profileAction() const
    { return _profileAction; }
    inline ProfileAction::shared_ptr &profileAction()
    { return _profileAction; }

public:
    inline AnalyzeAction::shared_ptr const &analyzeAction() const
    { return _analyzeAction; }
    inline AnalyzeAction::shared_ptr &analyzeAction()
    { return _analyzeAction; }

public:
    inline ArchiveAction::shared_ptr const &archiveAction() const
    { return _archiveAction; }
    inline ArchiveAction::shared_ptr &archiveAction()
    { return _archiveAction; }

public:
    bool parse(plist::Dictionary const *dict);

public:
    static shared_ptr Open(
        libutil::Filesystem const *filesystem,
        std::string const &name,
        std::string const &owner,
        std::string const &path);
};

} }

#endif  // !__xcscheme_XC_Scheme_h
