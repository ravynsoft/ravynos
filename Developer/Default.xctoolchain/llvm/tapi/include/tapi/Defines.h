//===-- tapi/Defines.h - TAPI C++ Library Defines ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief TAPI C++ library defines.
/// \since 1.0
///
//===----------------------------------------------------------------------===//
#ifndef TAPI_DEFINES_H
#define TAPI_DEFINES_H

#define TAPI_INTERNAL tapi::internal
#define TAPI_NAMESPACE_INTERNAL_BEGIN namespace tapi { namespace internal {
#define TAPI_NAMESPACE_INTERNAL_END } }

#define TAPI_NAMESPACE_V1_BEGIN namespace tapi { inline namespace v1 {
#define TAPI_NAMESPACE_V1_END } }

#define TAPI_PUBLIC __attribute__((visibility ("default")))

#endif // TAPI_DEFINES_H

