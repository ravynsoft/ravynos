/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2008-2022 Apple Inc. All rights reserved.
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
#ifndef __CONTAINERS_ABSTRACTION_H__
#define __CONTAINERS_ABSTRACTION_H__

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <string_view>

#define USE_LLVM_CONTAINERS 1

#if USE_LLVM_CONTAINERS
#include "../llvm/llvm-DenseSet.h"
#include "../llvm/llvm-DenseMap.h"
#else
// Include DenseMapInfo helpers even when not using LLVM containers.
#include "../llvm/llvm-DenseMapInfo.h"
#endif

namespace ld {
namespace container_details {

// utility classes for using std::unordered_map with c-strings
struct CStringHash
{
	size_t operator()(const char* s) const {
		return std::hash<std::string_view>{}(s);
	};
};

struct CStringEquals
{
	bool operator()(const char* left, const char* right) const {
		return (left == right) || (strcmp(left, right) == 0);
	}
};

struct CStringCmp {

	bool operator()(const char* left, const char* right) const {
		return (strcmp(left, right) < 0);
	}
};
}

template<typename Val>
using OrderedSet = std::set<Val>;

using CStringOrderedSet = std::set<const char*, container_details::CStringCmp>;

template<typename Key, typename Val>
using OrderedMap = std::map<Key, Val>;

#if USE_LLVM_CONTAINERS

template<typename Val>
using Set = ld::llvm::DenseSet<Val>;

template<typename Key, typename Val>
using Map = ld::llvm::DenseMap<Key, Val>;

template<typename Val>
using CStringMap = ld::llvm::DenseMap<const char*, Val>;

template<typename Val>
using StringViewMap = ld::llvm::DenseMap<std::string_view, Val>;

using CStringSet = ld::llvm::DenseSet<const char*>;

using StringViewSet = ld::llvm::DenseSet<std::string_view>;

#else // USE_LLVM_CONTAINERS

template<typename Val>
using Set = std::unordered_set<Val>;

template<typename Key, typename Val>
using Map = std::unordered_map<Key, Val>;

template<typename Val>
using CStringMap = std::unordered_map<const char*, Val, container_details::CStringHash, container_details::CStringEquals>;

template<typename Val>
using StringViewMap = std::unordered_map<std::string_view, Val>;

using CStringSet = std::unordered_set<const char*, container_details::CStringHash, container_details::CStringEquals>;

using StringViewSet = std::unordered_set<std::string_view>;

#endif // USE_LLVM_CONTAINERS
}
#endif // __CONTAINERS_ABSTRACTION_H__
