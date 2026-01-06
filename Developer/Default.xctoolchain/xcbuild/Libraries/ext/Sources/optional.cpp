/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

/*
 * This is a modified version of libc++'s <experimental/optional> with its dependency
 * on libc++-specific defines and includes removed to be usable with all C++11 compilers.
 * It also moves it from namespace std to namespace ext to not conflict with the real one.
 */

//===------------------------ optional.cpp --------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <ext/optional>

namespace ext {

bad_optional_access::~bad_optional_access() noexcept = default;

}
