//===- tapi/SDKDB/BitcodeReader.h - TAPI SDKDB Bitcode Reader ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief SDKDB bitcode reader.
///
//===----------------------------------------------------------------------===//
#ifndef TAPI_SDKDB_BITCODE_READER_H
#define TAPI_SDKDB_BITCODE_READER_H

#include "tapi/Core/API.h"
#include "tapi/Defines.h"
#include "tapi/SDKDB/SDKDB.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/TargetParser/Triple.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

struct SDKDBBitcodeMaterializeOption {
  // Targets to materailize. If empty, materialize all targets.
  std::vector<llvm::Triple> targets;
  // Dylibs to materialize. If empty, materialize all binaries.
  llvm::StringSet<> installNames;

  bool shouldParseTarget(llvm::Triple &target) const;
  bool shouldParseDylib(StringRef installName) const;

  static SDKDBBitcodeMaterializeOption defaultOption;
};

class SDKDBBitcodeReader {
public:
  // Helper function to create SDKDBBitcodeReader.
  static llvm::Expected<std::unique_ptr<SDKDBBitcodeReader>>
  get(llvm::MemoryBufferRef input, SDKDBBitcodeMaterializeOption &option);

  // Get available target triples for the SDKDB.
  const std::vector<llvm::Triple> &getAvailableTriples() const;

  // Materialize all information from bitcode using SDKDBBuilder.
  llvm::Error materialize(SDKDBBuilder &builder) const;

  // Get the version of SDKDB.
  std::string getSDKDBVersion() const;

  // If the SDKDB is public only.
  bool isPublicOnly() const;

  // If the SDKDB has no objc metadata.
  bool noObjCMetadata() const;

  // Get the build verison of the SDKDB.
  std::string getBuildVersion() const;

  // Perform path lookup.
  llvm::Expected<bool> dylibExistsForPath(llvm::Triple &target, StringRef path);

  // Perform API load. This will load all the dylibs from
  // SDKDBBitcodeMaterializeOption and all its re-exported frameworks.
  llvm::Error loadAPIsFromSDKDB(SDKDBBuilder &builder, llvm::Triple &target,
                                StringRef path);

  // Get a vector of all the projects that had error when producing this SDKDB.
  const std::vector<std::string> &getProjectsWithError() const;

  SDKDBBitcodeReader(llvm::MemoryBufferRef input,
                     SDKDBBitcodeMaterializeOption &option, llvm::Error &err);
  ~SDKDBBitcodeReader();

private:

  SDKDBBitcodeReader(const SDKDBBitcodeReader &) = delete;

  // Implementation.
  class Implementation;
  Implementation &impl;
};

TAPI_NAMESPACE_INTERNAL_END

#endif /* TAPI_SDKDB_BITCODE_READER_H */
