/*
 * Copyright (c) 2019 Apple Inc.  All Rights Reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef DENSEMAPEXTRAS_H
#define DENSEMAPEXTRAS_H

#include "llvm-DenseMap.h"
#include "llvm-DenseSet.h"

namespace objc {

// We cannot use a C++ static initializer to initialize certain globals because
// libc calls us before our C++ initializers run. We also don't want a global
// pointer to some globals because of the extra indirection.
//
// ExplicitInit / LazyInit wrap doing it the hard way.
template <typename Type>
class ExplicitInit {
    alignas(Type) uint8_t _storage[sizeof(Type)];

public:
    template <typename... Ts>
    void init(Ts &&... Args) {
        new (_storage) Type(std::forward<Ts>(Args)...);
    }

    Type &get() {
        return *reinterpret_cast<Type *>(_storage);
    }
};

template <typename Type>
class LazyInit {
    alignas(Type) uint8_t _storage[sizeof(Type)];
    bool _didInit;

public:
    template <typename... Ts>
    Type *get(bool allowCreate, Ts &&... Args) {
        if (!_didInit) {
            if (!allowCreate) {
                return nullptr;
            }
            new (_storage) Type(std::forward<Ts>(Args)...);
            _didInit = true;
        }
        return reinterpret_cast<Type *>(_storage);
    }
};

// Convenience class for Dense Maps & Sets
template <typename Key, typename Value>
class ExplicitInitDenseMap : public ExplicitInit<DenseMap<Key, Value>> { };

template <typename Key, typename Value>
class LazyInitDenseMap : public LazyInit<DenseMap<Key, Value>> { };

template <typename Value>
class ExplicitInitDenseSet : public ExplicitInit<DenseSet<Value>> { };

template <typename Value>
class LazyInitDenseSet : public LazyInit<DenseSet<Value>> { };

} // namespace objc

#endif /* DENSEMAPEXTRAS_H */
