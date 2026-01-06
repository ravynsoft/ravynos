/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxspec_PBX_BuildRule_h
#define __pbxspec_PBX_BuildRule_h

#include <pbxspec/PBX/Specification.h>

#include <ext/optional>

namespace pbxspec { class Manager; }

namespace pbxspec { namespace PBX {

class BuildRule {
public:
    typedef std::shared_ptr <BuildRule> shared_ptr;
    typedef std::vector <shared_ptr> vector;

private:
    ext::optional<std::string>              _name;
    ext::optional<std::vector<std::string>> _fileTypes;
    ext::optional<std::string>              _compilerSpec;

protected:
    friend class pbxspec::Manager;
    BuildRule();
    BuildRule(std::vector<std::string> const &fileTypes, std::string const &compilerSpec);

public:
    inline ext::optional<std::string> const &name() const
    { return _name; }
    inline ext::optional<std::vector<std::string>> const &fileTypes() const
    { return _fileTypes; }
    inline ext::optional<std::string> const &compilerSpec() const
    { return _compilerSpec; }

protected:
    bool parse(plist::Dictionary const *dict);
};

} }

#endif  // !__pbxspec_PBX_BuildRule_h
