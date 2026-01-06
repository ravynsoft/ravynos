/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_PBX_AggregateTarget_h
#define __pbxproj_PBX_AggregateTarget_h

#include <pbxproj/PBX/Target.h>

namespace pbxproj { namespace PBX {

class AggregateTarget : public Target {
public:
    typedef std::shared_ptr <AggregateTarget> shared_ptr;

private:
    std::string  _productName;

public:
    AggregateTarget();

public:
    inline std::string const &productName() const
    { return _productName; }

protected:
    bool parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

public:
    static inline char const *Isa()
    { return ISA::PBXAggregateTarget; }
};

} }

#endif  // !__pbxproj_PBX_AggregateTarget_h
