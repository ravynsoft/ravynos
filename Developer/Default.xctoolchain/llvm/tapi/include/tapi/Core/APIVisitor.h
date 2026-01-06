//===- tapi/Core/APIVisitor.h - TAPI API Visitor ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the API Visitor.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_APIVISITOR_H
#define TAPI_CORE_APIVISITOR_H

#include "tapi/Core/API.h"
#include "tapi/Defines.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

class APIVisitor {
public:
  virtual ~APIVisitor();

  virtual void visitGlobal(const GlobalRecord &);
  virtual void visitEnum(const EnumRecord &);
  virtual void visitObjCInterface(const ObjCInterfaceRecord &);
  virtual void visitObjCCategory(const ObjCCategoryRecord &);
  virtual void visitObjCProtocol(const ObjCProtocolRecord &);
  virtual void visitTypeDef(const TypedefRecord &);
};

class APIMutator {
public:
  virtual ~APIMutator();

  virtual void visitGlobal(GlobalRecord &);
  virtual void visitEnum(EnumRecord &);
  virtual void visitObjCInterface(ObjCInterfaceRecord &);
  virtual void visitObjCCategory(ObjCCategoryRecord &);
  virtual void visitObjCProtocol(ObjCProtocolRecord &);
  virtual void visitTypeDef(TypedefRecord &);
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_APIVISITOR_H
