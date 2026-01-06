/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcscheme_XC_BuildAction_h
#define __xcscheme_XC_BuildAction_h

#include <xcscheme/XC/Action.h>
#include <xcscheme/XC/BuildActionEntry.h>

#include <memory>

namespace plist { class Dictionary; }

namespace xcscheme { namespace XC {

class BuildAction : public Action {
public:
    typedef std::shared_ptr <BuildAction> shared_ptr;

private:
    bool                     _buildImplicitDependencies;
    bool                     _parallelizeBuildables;
    BuildActionEntry::vector _buildActionEntries;

public:
    BuildAction();

public:
    inline bool buildImplicitDependencies() const
    { return _buildImplicitDependencies; }

public:
    inline bool parallelizeBuildables() const
    { return _parallelizeBuildables; }

public:
    inline BuildActionEntry::vector const &buildActionEntries() const
    { return _buildActionEntries; }
    inline BuildActionEntry::vector &buildActionEntries()
    { return _buildActionEntries; }

public:
    bool parse(plist::Dictionary const *dict) override;
};

} }

#endif  // !__xcscheme_XC_BuildAction_h
