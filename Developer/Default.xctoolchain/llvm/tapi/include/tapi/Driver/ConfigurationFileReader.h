//===- ConfigurationFileReader.h - Configuration File Reader ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief The configuration file reader.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_DRIVER_CONFIGURATION_FILE_READER_H
#define TAPI_DRIVER_CONFIGURATION_FILE_READER_H

#include "tapi/Defines.h"
#include "tapi/Driver/ConfigurationFile.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/MemoryBuffer.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

class ConfigurationFileReader {
  class Implementation;

  Implementation &impl;

  ConfigurationFileReader(MemoryBufferRef inputBuffer, llvm::Error &error);

public:
  static llvm::Expected<std::unique_ptr<ConfigurationFileReader>>
  get(MemoryBufferRef inputBuffer);

  ~ConfigurationFileReader();

  ConfigurationFileReader(const ConfigurationFileReader &) = delete;
  ConfigurationFileReader &operator=(const ConfigurationFileReader &) = delete;

  ConfigurationFile takeConfigurationFile();
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_DRIVER_CONFIGURATION_FILE_READER_H
