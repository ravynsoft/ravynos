//===- lib/Driver/APINormalizer.cpp - API Normalizer ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements API Normalizer
///
//===----------------------------------------------------------------------===//

#include "APINormalizer.h"
#include "tapi/Core/Utils.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

void APINormalizer::visitGlobal(GlobalRecord &record) {
  updateAPIRecord(record);
}

void APINormalizer::visitEnum(EnumRecord &record) {
  updateAPIRecord(record);
  for (auto *constant : record.constants)
    updateAPIRecord(*constant);
}

void APINormalizer::visitObjCInterface(ObjCInterfaceRecord &record) {
  updateAPIRecord(record);
  updateContainer(record);
}

void APINormalizer::visitObjCCategory(ObjCCategoryRecord &record) {
  updateAPIRecord(record);
  updateContainer(record);
}

void APINormalizer::visitObjCProtocol(ObjCProtocolRecord &record) {
  updateAPIRecord(record);
  updateContainer(record);
}

void APINormalizer::visitTypeDef(TypedefRecord &record) {
  updateAPIRecord(record);
}

void APINormalizer::updateAPIRecord(APIRecord &record) {
  updateAPILoc(record);

  // If the library is not in a public location, sets the APIAccess to private.
  if (record.access == APIAccess::Public && !isPublicLibrary)
    record.access = APIAccess::Private;
}

void APINormalizer::updateAPILoc(APIRecord &record) {
  // FIXME: Recovering the real file path depends on how records were
  // resolved during AST traversal and is sensitive to header search path
  // resolution. The real path should be saved instead from initial input,
  // either from directory scanning or via filelist.
  auto path = record.loc.getFilename();
  auto cachedPath = fileMap.find(path);
  // If the realPath is cached, update the path if needed, and return.
  if (cachedPath != fileMap.end()) {
    if (path != cachedPath->getValue())
      record.loc = APILoc(cachedPath->getValue(), record.loc.getLine(),
                          record.loc.getColumn());
    return;
  }
  // Compute the realPath.
  SmallString<PATH_MAX> realPath;
  auto ec = llvm::sys::fs::real_path(path, realPath);
  if (!ec)
    path = realPath;
  auto hasRoot = path.find("/Root/");
  auto filename = hasRoot != StringRef::npos
                      ? path.drop_front(hasRoot + sizeof("Root"))
                      : llvm::sys::path::filename(path);
  auto entry = fileMap.try_emplace(record.loc.getFilename(), filename);
  record.loc = APILoc(entry.first->getValue(), record.loc.getLine(),
                      record.loc.getColumn());
}

void APINormalizer::updateContainer(ObjCContainerRecord &record) {
  for (auto *method : record.methods)
    updateAPIRecord(*method);
  for (auto *prop : record.properties)
    updateAPIRecord(*prop);
  for (auto *ivar : record.ivars)
    updateAPIRecord(*ivar);
}

TAPI_NAMESPACE_INTERNAL_END
