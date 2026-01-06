/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcscheme_XC_BuildActionEntry_h
#define __xcscheme_XC_BuildActionEntry_h

#include <xcscheme/XC/BuildableReference.h>

#include <memory>
#include <vector>

namespace plist { class Dictionary; }

namespace xcscheme { namespace XC {

class BuildActionEntry {
public:
    typedef std::shared_ptr <BuildActionEntry> shared_ptr;
    typedef std::vector <shared_ptr> vector;

private:
    bool                           _buildForAnalyzing;
    bool                           _buildForArchiving;
    bool                           _buildForProfiling;
    bool                           _buildForRunning;
    bool                           _buildForTesting;
    BuildableReference::shared_ptr _buildableReference;

public:
    BuildActionEntry();

public:
    inline bool buildForAnalyzing() const
    { return _buildForAnalyzing; }

    inline bool buildForArchiving() const
    { return _buildForArchiving; }

    inline bool buildForProfiling() const
    { return _buildForProfiling; }

    inline bool buildForRunning() const
    { return _buildForRunning; }

    inline bool buildForTesting() const
    { return _buildForTesting; }

public:
    inline BuildableReference::shared_ptr const &buildableReference() const
    { return _buildableReference; }
    inline BuildableReference::shared_ptr &buildableReference()
    { return _buildableReference; }

public:
    bool parse(plist::Dictionary const *dict);
};

} }

#endif  // !__xcscheme_XC_BuildActionEntry_h
