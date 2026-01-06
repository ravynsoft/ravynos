//===- tapi/Core/LLVM.h - Import various common LLVM datatypes --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file forward declares and imports various common LLVM and clang
// datatypes that tapi wants to use unqualified.
//
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_LLVM_H
#define TAPI_CORE_LLVM_H

#include "llvm/BinaryFormat/MachO.h"
#include "llvm/Support/Casting.h"

namespace llvm {

// ADT's.
template <typename T> class ErrorOr;
template <typename T> class IntrusiveRefCntPtr;
template <typename T, unsigned N> class SmallPtrSet;
template <unsigned InternalLen> class SmallString;
template <typename T, unsigned N> class SmallVector;
template <typename T> class SmallVectorImpl;
template <typename T> class ArrayRef;
class raw_ostream;
class StringRef;
class Twine;
class MemoryBuffer;
class MemoryBufferRef;

namespace MachO {
enum Architecture : uint8_t;
class ArchitectureSet;
class Target;
enum class EncodeKind : uint8_t;
enum class SymbolFlags : uint8_t;
enum class ObjCIFSymbolKind : uint8_t;
enum FileType : unsigned;
class InterfaceFile;
class PackedVersion;
class SymbolSet;
using TargetList = SmallVector<Target, 5>;
class TextAPIReader;
class TextAPIWriter;
class TextAPIError;
enum class TextAPIErrorCode;
class InterfaceFileRef;
struct SimpleSymbol;

} // namespace MachO

} // end namespace llvm.

namespace clang {

class DirectoryEntry;
class FileEntry;
class DiagnosticBuilder;
class StreamingDiagnostic;

} // end namespace clang.

namespace tapi {

// Casting operators.
using llvm::isa;
using llvm::cast;
using llvm::dyn_cast;
using llvm::dyn_cast_or_null;
using llvm::cast_or_null;

// ADT's.
using llvm::ArrayRef;
using llvm::ErrorOr;
using llvm::IntrusiveRefCntPtr;
using llvm::SmallPtrSet;
using llvm::SmallString;
using llvm::SmallVector;
using llvm::SmallVectorImpl;
using llvm::raw_ostream;
using llvm::StringRef;
using llvm::Twine;
using llvm::MemoryBuffer;
using llvm::MemoryBufferRef;

// FileManager
using clang::DirectoryEntry;
using clang::FileEntry;

using clang::DiagnosticBuilder;
using clang::StreamingDiagnostic;

// BinaryFormat
using llvm::MachO::PlatformType;

// TextAPI types
using llvm::MachO::Architecture;
using llvm::MachO::ArchitectureSet;
using llvm::MachO::EncodeKind;
using llvm::MachO::FileType;
using llvm::MachO::InterfaceFile;
using llvm::MachO::InterfaceFileRef;
using llvm::MachO::ObjCIFSymbolKind;
using llvm::MachO::PackedVersion;
using llvm::MachO::SimpleSymbol;
using llvm::MachO::SymbolFlags;
using llvm::MachO::SymbolSet;
using llvm::MachO::Target;
using llvm::MachO::TargetList;
using llvm::MachO::TextAPIError;
using llvm::MachO::TextAPIErrorCode;
using llvm::MachO::TextAPIReader;
using llvm::MachO::TextAPIWriter;

} // end namespace tapi.

#endif // TAPI_CORE_LLVM_H
