/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Base_h
#define __plist_Base_h

#include <memory>
#include <cstdint>
#include <cstdlib>

namespace plist {

template<typename T, typename U>
static inline std::unique_ptr<T>
static_unique_pointer_cast(std::unique_ptr<U> &&p)
{
    return std::unique_ptr<T>(static_cast<T *>(p.release()));
}

}

#endif  // !__plist_Base_h
