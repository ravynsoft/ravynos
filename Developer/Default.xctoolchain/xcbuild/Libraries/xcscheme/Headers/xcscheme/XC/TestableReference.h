/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcscheme_XC_TestableReference_h
#define __xcscheme_XC_TestableReference_h

#include <xcscheme/XC/BuildableReference.h>
#include <xcscheme/XC/Test.h>

namespace plist { class Dictionary; }

namespace xcscheme { namespace XC {

class TestableReference {
public:
    typedef std::shared_ptr <TestableReference> shared_ptr;
    typedef std::vector <shared_ptr> vector;

private:
    bool                           _skipped;
    BuildableReference::shared_ptr _buildableReference;
    Test::vector                   _skippedTests;

public:
    TestableReference();

public:
    inline bool skipped() const
    { return _skipped; }

public:
    inline BuildableReference::shared_ptr const &buildableReference() const
    { return _buildableReference; }
    inline BuildableReference::shared_ptr &buildableReference()
    { return _buildableReference; }

public:
    inline Test::vector const &skippedTests() const
    { return _skippedTests; }
    inline Test::vector &skippedTests()
    { return _skippedTests; }

public:
    bool parse(plist::Dictionary const *dict);
};

} }

#endif  // !__xcscheme_XC_TestableReference_h
