//===- API2SymbolConverter.h - API2Symbol Converter -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief API2Symbol Converter
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/APIVisitor.h"
#include "tapi/Defines.h"
#include "llvm/TextAPI/InterfaceFile.h"

#ifndef TAPI_DRIVER_API2SYMBOLCONVERTER_H
#define TAPI_DRIVER_API2SYMBOLCONVERTER_H

TAPI_NAMESPACE_INTERNAL_BEGIN

class API2SymbolConverter : public APIMutator {
public:
  API2SymbolConverter(SymbolSet *symbolSet, const Target &triple,
                      const bool recordUndefs = false)
      : symbolSet(symbolSet), target(triple), recordUndefs(recordUndefs) {}
  void visitGlobal(GlobalRecord &) override;
  void visitObjCInterface(ObjCInterfaceRecord &) override;
  void visitObjCCategory(ObjCCategoryRecord &) override;

private:
  SymbolSet *symbolSet;
  const Target target;
  const bool recordUndefs;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_DRIVER_API2SYMBOLCONVERTER_H
