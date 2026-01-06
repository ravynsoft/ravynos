/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_PBX_BuildRule_h
#define __pbxproj_PBX_BuildRule_h

#include <pbxproj/PBX/Object.h>

namespace pbxproj { namespace PBX {

class BuildRule : public Object {
public:
    typedef std::shared_ptr <BuildRule> shared_ptr;
    typedef std::vector <shared_ptr> vector;

private:
    std::string            _compilerSpec;
    std::string            _filePatterns;
    std::string            _fileType;
    std::string            _script;
    std::vector<std::string> _outputFiles;
    bool                   _isEditable;

public:
    BuildRule();

public:
    inline std::string const &compilerSpec() const
    { return _compilerSpec; }

public:
    inline std::string const &filePatterns() const
    { return _filePatterns; }

    inline std::string const &fileType() const
    { return _fileType; }

public:
    inline std::string const &script() const
    { return _script; }

public:
    inline std::vector<std::string> const &outputFiles() const
    { return _outputFiles; }

public:
    inline bool isEditable() const
    { return _isEditable; }

protected:
    bool parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

public:
    static inline char const *Isa()
    { return ISA::PBXBuildRule; }
};

} }

#endif  // !__pbxproj_PBX_BuildRule_h
