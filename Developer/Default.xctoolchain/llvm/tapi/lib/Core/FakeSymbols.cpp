//===- lib/Core/FakeSymbols.cpp - Fake Symbols ------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Fake several LLVM method. This prevents the inclusion of static
/// initializers code that we don't need.
///
//===----------------------------------------------------------------------===//

#include "llvm/Object/Archive.h"
#include "llvm/Object/IRObjectFile.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Object/OffloadBinary.h"
#include "llvm/Object/TapiUniversal.h"
#include "llvm/Object/WindowsResource.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ErrorOr.h"

namespace llvm {
class MemoryBufferRef;
} // namespace llvm

namespace llvm {

namespace object {

Expected<std::unique_ptr<Archive>> Archive::create(MemoryBufferRef Source) {
  llvm_unreachable("not supported");
}

Expected<std::unique_ptr<WindowsResource>>
WindowsResource::createWindowsResource(MemoryBufferRef Source) {
  llvm_unreachable("not supported");
}

Expected<std::unique_ptr<COFFObjectFile>>
ObjectFile::createCOFFObjectFile(MemoryBufferRef Object) {
  llvm_unreachable("not supported");
}

Expected<std::unique_ptr<ObjectFile>>
ObjectFile::createELFObjectFile(MemoryBufferRef Obj, bool initContent) {
  llvm_unreachable("not supported");
}

Expected<std::unique_ptr<WasmObjectFile>>
ObjectFile::createWasmObjectFile(MemoryBufferRef Object) {
  llvm_unreachable("not supported");
}


Expected<MemoryBufferRef>
IRObjectFile::findBitcodeInObject(const ObjectFile &Obj) {
  llvm_unreachable("not supported");
}

Expected<std::unique_ptr<IRObjectFile>>
IRObjectFile::create(MemoryBufferRef Object, LLVMContext &Context) {
  llvm_unreachable("not supported");
}

Expected<std::unique_ptr<TapiUniversal>>
TapiUniversal::create(MemoryBufferRef Source) {
  llvm_unreachable("not supported");
}

Expected<std::unique_ptr<OffloadBinary>>
OffloadBinary::create(MemoryBufferRef Source) {
  llvm_unreachable("not supported");
}

} // end namespace object.
} // end namespace llvm.
