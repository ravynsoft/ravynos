//===- tapi/SDKDB/BitcodeWriter.h - TAPI SDKDB Bitcode Writer ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief SDKDB bitcode writer.
///
//===----------------------------------------------------------------------===//
#ifndef TAPI_SDKDB_BITCODE_WRTIER_H
#define TAPI_SDKDB_BITCODE_WRTIER_H

#include "tapi/Core/API.h"
#include "tapi/Defines.h"
#include "tapi/SDKDB/SDKDB.h"
#include "llvm/Support/raw_ostream.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

class SDKDBBitcodeWriter {
public:
  SDKDBBitcodeWriter() = default;

  void writeSDKDBToStream(const SDKDBBuilder &builder, raw_ostream &os);
};

TAPI_NAMESPACE_INTERNAL_END

#endif /* TAPI_SDKDB_BITCODE_WRTIER_H */
