/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcscheme/XC/TestableReference.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>

using xcscheme::XC::TestableReference;

TestableReference::TestableReference() :
    _skipped(false)
{
}

bool TestableReference::
parse(plist::Dictionary const *dict)
{
    auto S   = dict->value <plist::Boolean> ("skipped");
    auto BR  = dict->value <plist::Dictionary> ("BuildableReference");
    auto ST  = dict->value <plist::Dictionary> ("SkippedTests");

    if (S != nullptr) {
        _skipped = S->value();
    }

    if (BR != nullptr) {
        _buildableReference = std::make_shared <BuildableReference> ();
        if (!_buildableReference->parse(BR))
            return false;
    }

    if (ST != nullptr) {
        if (auto Td = ST->value <plist::Dictionary> ("Test")) {
            auto T = std::make_shared <Test> ();
            if (!T->parse(Td))
                return false;

            _skippedTests.push_back(T);
        } else if (auto Ta = ST->value <plist::Array> ("Test")) {
            for (size_t n = 0; n < Ta->count(); n++) {
                auto Td = Ta->value <plist::Dictionary> (n);
                if (Td == nullptr)
                    continue;

                auto T = std::make_shared <Test> ();
                if (!T->parse(Td))
                    return false;

                _skippedTests.push_back(T);
            }
        }
    }

    return true;
}
