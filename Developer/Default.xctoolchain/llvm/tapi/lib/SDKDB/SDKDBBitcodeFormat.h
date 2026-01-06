//===--- SDKDBBitcodeFormat.h - The internals of SDKDB bitcode --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief SDKDB bitcode format
///
//===----------------------------------------------------------------------===//
#ifndef TAPI_SDKDB_BITCODE_FORMAT_H
#define TAPI_SDKDB_BITCODE_FORMAT_H

#include "tapi/Defines.h"
#include "llvm/Bitstream/BitCodes.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

using namespace llvm;

/// Magic number for binary store files.
const unsigned char SDKDB_SIGNATURE[] = {'S', 'D', 'K', 0xDB}; // NOLINT

/// Binary store major version number.
const uint16_t VERSION_MAJOR = 1; // NOLINT

/// Binary store minor version number.
const uint16_t VERSION_MINOR = 0; // NOLINT

/// \brief The blocks that can appear in a binary store.
///
/// These IDs must \em not be renumbered or reordered without incrementing
/// VERSION_MAJOR.
enum BlockID {
  /// The control block, which contains all of the information that needs to be
  /// validated prior to loading the binary store file. This block is expected
  /// to be first in the file.
  ///
  /// \sa control_block
  CONTROL_BLOCK_ID = llvm::bitc::FIRST_APPLICATION_BLOCKID,

  /// The identifer block, which contains all the strings, such as symbols or
  /// dynamic library names.
  ///
  /// \sa identifier_block
  IDENTIFIER_BLOCK_ID,

  /// The SDKDB block, which contains SDK information about one target.
  /// There can be multiple SDKDB blocks.
  ///
  /// \sa sdkdb_block
  SDKDB_BLOCK_ID,

  /// The API block, which contains API info for one binary.
  ///
  /// \sa api_block
  API_BLOCK_ID,

  /// The Global block, which contains API info for global.
  ///
  /// \sa global_block
  GLOBAL_BLOCK_ID,

  /// The ObjC class block.
  ///
  /// \sa objc_class_block
  OBJC_CLASS_BLOCK_ID,

  /// The ObjC category block.
  ///
  /// \sa objc_category_block
  OBJC_CATEGORY_BLOCK_ID,

  /// The ObjC protocol block.
  ///
  /// \sa objc_protocol_block
  OBJC_PROTOCOL_BLOCK_ID,

  /// The ObjC method block.
  ///
  /// \sa objc_method_block
  OBJC_METHOD_BLOCK_ID,

  /// The ObjC property block.
  ///
  /// \sa objc_prperty_block
  OBJC_PROPERTY_BLOCK_ID,

  /// The ObjC ivar block
  ///
  /// \sa objc_ivar_block
  OBJC_IVAR_BLOCK_ID,

  /// The library lookup table block.
  ///
  /// \sa library_table_block
  LIBRARY_TABLE_BLOCK_ID,

  /// The Enum block, which contains API info for enum.
  ///
  /// \sa enum_block
  ENUM_BLOCK_ID,

  /// The Enum constant block, which contains API info for enum constant.
  ///
  /// \sa enum_constant_block
  ENUM_CONSTANT_BLOCK_ID,

  /// The typedef block, which contains API info for typedef.
  ///
  /// \sa typdef_block
  TYPEDEF_BLOCK_ID,
};

// clang-format off
namespace control_block {
// These IDs must \em not be renumbered or reordered without incrementing
// VERSION_MAJOR.
enum {
  /// The metadata contains version information.
  METADATA = 1,

  /// Project with errors.
  PROJECT_WITH_ERROR = 2,
};
} // end namespace control_block

namespace identifier_block {
// These IDs must \em not be renumbered or reordered without incrementing
// VERSION_MAJOR.
enum {
  /// String table of uniqued identifiers.
  STRING_TABLE = 1,
};
} // end namespace identifier_block

namespace sdkdb_block {
// These IDs must \em not be renumbered or reordered without incrementing
// VERSION_MAJOR.
enum {
  /// Target Triple for the SDKDB.
  TARGET_TRIPLE = 1,
};
} // end namespace sdkdb_block

namespace api_block {
// These IDs must \em not be renumbered or reordered without incrementing
// VERSION_MAJOR.
enum {
  /// InstallName.
  INSTALL_NAME= 1,

  /// FileType.
  FILE_TYPE = 2,

  /// UUID.
  UUID = 3,

  /// Re-exported.
  REEXPORTED = 4,

  /// Allowable clients.
  ALLOWABLE_CLIENTS = 5,

  /// Parent Umbrella.
  PARENT_UMBRELLA = 6,

  /// Compatibility and current version.
  DYLIB_VERSION = 7,

  /// Flags. For two level namespace, app extension safe.
  FLAGS = 8,

  /// Swift version.
  SWIFT_VERSION = 9,

  /// Potentially defined selectors.
  POTENTIALLY_DEFINED_SELECTOR = 10,

  /// Project name.
  PROJECT_NAME = 11,
};
} // end namespace api_block

namespace global_block {
// These IDs must \em not be renumbered or reordered without incrementing
// VERSION_MAJOR.
enum {
  /// API Record info.
  INFO = 1,

  /// Availability info.
  AVAILABILITY = 2,

  /// Filename info.
  FILENAME = 3,

  /// Location.
  LOCATION = 4,
};
} // end namespace global_block

namespace objc_class_block {
// These IDs must \em not be renumbered or reordered without incrementing
// VERSION_MAJOR.
enum {
  /// API Record info.
  INFO = 1,

  /// Availability info.
  AVAILABILITY = 2,

  /// Filename info.
  FILENAME = 3,

  /// Location.
  LOCATION = 4,

  /// Protocols.
  PROTOCOL = 5,
};
} // end namespace objc_class_block

namespace objc_category_block {
// These IDs must \em not be renumbered or reordered without incrementing
// VERSION_MAJOR.
enum {
  /// API Record info.
  INFO = 1,

  /// Availability info.
  AVAILABILITY = 2,

  /// Filename info.
  FILENAME = 3,

  /// Location.
  LOCATION = 4,

  /// Protocols.
  PROTOCOL = 5,
};
} // end namespace objc_category_block

namespace objc_protocol_block {
// These IDs must \em not be renumbered or reordered without incrementing
// VERSION_MAJOR.
enum {
  /// API Record info.
  INFO = 1,

  /// Availability info.
  AVAILABILITY = 2,

  /// Filename info.
  FILENAME = 3,

  /// Location.
  LOCATION = 4,

  /// Protocols.
  PROTOCOL = 5,
};
} // end namespace objc_protocol_block

namespace objc_method_block {
// These IDs must \em not be renumbered or reordered without incrementing
// VERSION_MAJOR.
enum {
  /// API Record info.
  INFO = 1,

  /// Availability info.
  AVAILABILITY = 2,

  /// Filename info.
  FILENAME = 3,

  /// Location.
  LOCATION = 4,
};
} // end namespace objc_method_block

namespace objc_property_block {
// These IDs must \em not be renumbered or reordered without incrementing
// VERSION_MAJOR.
enum {
  /// API Record info.
  INFO = 1,

  /// Availability info.
  AVAILABILITY = 2,

  /// Filename info.
  FILENAME = 3,

  /// Location.
  LOCATION = 4,
};
} // end namespace objc_property_block

namespace objc_ivar_block {
// These IDs must \em not be renumbered or reordered without incrementing
// VERSION_MAJOR.
enum {
  /// API Record info.
  INFO = 1,

  /// Availability info.
  AVAILABILITY = 2,

  /// Filename info.
  FILENAME = 3,

  /// Location.
  LOCATION = 4,
};
} // end namespace objc_ivar_block

namespace library_table_block {
// These IDs must \em not be renumbered or reordered without incrementing
// VERSION_MAJOR.
enum {
  // Target Triple for the lookup table.
  TARGET_TRIPLE = 1,

  // OnDiskHashTable.
  LOOKUP_TABLE = 2,
};
} // end namespace library_table_block

namespace enum_block {
// These IDs must \em not be renumbered or reordered without incrementing
// VERSION_MAJOR.
enum {
  /// API Record info.
  INFO = 1,

  /// Availability info.
  AVAILABILITY = 2,

  /// Filename info.
  FILENAME = 3,

  /// Location.
  LOCATION = 4,
};
} // end namespace enum_block

namespace enum_constant_block {
// These IDs must \em not be renumbered or reordered without incrementing
// VERSION_MAJOR.
enum {
  /// API Record info.
  INFO = 1,

  /// Availability info.
  AVAILABILITY = 2,

  /// Filename info.
  FILENAME = 3,

  /// Location.
  LOCATION = 4,
};
} // end namespace enum_constant_block

namespace typedef_block {
// These IDs must \em not be renumbered or reordered without incrementing
// VERSION_MAJOR.
enum {
  /// API Record info.
  INFO = 1,

  /// Availability info.
  AVAILABILITY = 2,

  /// Filename info.
  FILENAME = 3,

  /// Location.
  LOCATION = 4,
};
} // end namespace typedef_block

// clang-format on

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_SDKDB_BITCODE_FORMAT_H
