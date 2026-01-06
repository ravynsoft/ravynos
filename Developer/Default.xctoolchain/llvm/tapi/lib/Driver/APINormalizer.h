//===- APINormalizer.h - API Normalizer -------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief API Normalizer
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/APIVisitor.h"
#include "tapi/Defines.h"

#ifndef TAPI_DRIVER_APINORMALIZER_H
#define TAPI_DRIVER_APINORMALIZER_H

TAPI_NAMESPACE_INTERNAL_BEGIN

class APINormalizer : public APIMutator {
public:
  APINormalizer(bool isPublicLibrary)
      : isPublicLibrary(isPublicLibrary), fileMap({}) {}
  void visitGlobal(GlobalRecord &record) override;
  void visitEnum(EnumRecord &record) override;
  void visitObjCInterface(ObjCInterfaceRecord &record) override;
  void visitObjCCategory(ObjCCategoryRecord &record) override;
  void visitObjCProtocol(ObjCProtocolRecord &record) override;
  void visitTypeDef(TypedefRecord &record) override;

private:
  void updateAPIRecord(APIRecord &record);
  void updateAPILoc(APIRecord &record);
  void updateContainer(ObjCContainerRecord &record);

  bool isPublicLibrary = false;
  llvm::StringMap<std::string> fileMap;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_DRIVER_APINORMALIZER_H
