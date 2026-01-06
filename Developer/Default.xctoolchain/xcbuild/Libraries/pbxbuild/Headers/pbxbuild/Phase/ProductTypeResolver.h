/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Phase_ProductTypeResolver_h
#define __pbxbuild_Phase_ProductTypeResolver_h

#include <pbxbuild/Base.h>

namespace pbxbuild {
namespace Phase {

class Environment;
class Context;

class ProductTypeResolver {
private:
    pbxspec::PBX::ProductType::shared_ptr _productType;

public:
    explicit ProductTypeResolver(pbxspec::PBX::ProductType::shared_ptr const &productType);
    ~ProductTypeResolver();

public:
    pbxspec::PBX::ProductType::shared_ptr const &productType() const
    { return _productType; }

public:
    bool resolve(Phase::Environment const &phaseEnvironment, Phase::Context *phaseContext) const;
};

}
}

#endif // !__pbxbuild_Phase_ProductTypeResolver_h
