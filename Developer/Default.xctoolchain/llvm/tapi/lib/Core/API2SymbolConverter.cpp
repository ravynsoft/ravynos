//===- lib/Core/API2SymbolConverter.cpp - API2Symbol Converter ---------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements API2Symbol Converter
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/API2SymbolConverter.h"
#include "tapi/Core/LLVM.h"
#include "llvm/TextAPI/Symbol.h"

#include "clang/AST/DeclObjC.h"

using namespace llvm;
using namespace llvm::MachO;
using namespace clang;

TAPI_NAMESPACE_INTERNAL_BEGIN

namespace {
SymbolFlags getFlagsFromRecord(const APIRecord &record) {
  auto flag = SymbolFlags::None;
  if (record.isWeakDefined())
    flag |= SymbolFlags::WeakDefined;
  if (record.isThreadLocalValue())
    flag |= SymbolFlags::ThreadLocalValue;
  if (record.isReexported())
    flag |= SymbolFlags::Rexported;

  if (record.isExternal())
    flag |= SymbolFlags::Undefined;
  if (record.isWeakReferenced())
    flag |= SymbolFlags::WeakReferenced;

  if (record.isText())
    flag |= SymbolFlags::Text;
  else
    flag |= SymbolFlags::Data;

  return flag;
}
} // namespace

void API2SymbolConverter::visitGlobal(GlobalRecord &record) {

  // Skip non exported symbols unless for flat namespace symbols.
  if (!record.isExported()) {
    if (recordUndefs && record.isExternal()) {
      record.flags = getFlagsFromRecord(record);
      symbolSet->addGlobal(EncodeKind::GlobalSymbol, record.name, record.flags,
                           target);
    }
    return;
  }

  auto sym = parseSymbol(record.name);
  record.flags = getFlagsFromRecord(record);
  if (sym.ObjCInterfaceType == ObjCIFSymbolKind::None) {
    symbolSet->addGlobal(sym.Kind, sym.Name, record.flags, target);
    return;
  }
  // It is impossible to hold a complete ObjCInterface with a single
  // GlobalRecord, so continue to treat this symbol a generic global.
  symbolSet->addGlobal(EncodeKind::GlobalSymbol, record.name, record.flags,
                       target);
}

void API2SymbolConverter::visitObjCInterface(ObjCInterfaceRecord &record) {
  if (record.isExported()) {
    record.flags = getFlagsFromRecord(record);
    if (record.isCompleteInterface()) {
      symbolSet->addGlobal(EncodeKind::ObjectiveCClass, record.name,
                           record.flags, target);
      if (record.hasExceptionAttribute())
        symbolSet->addGlobal(EncodeKind::ObjectiveCClassEHType, record.name,
                             record.flags, target);
    } else {
      // Because there is not a complete interface, visit individual symbols
      // instead.
      if (record.isExportedSymbol(ObjCIFSymbolKind::EHType))
        symbolSet->addGlobal(EncodeKind::GlobalSymbol,
                             (ObjC2EHTypePrefix + record.name).str(),
                             record.flags, target);
      if (record.isExportedSymbol(ObjCIFSymbolKind::Class))
        symbolSet->addGlobal(EncodeKind::GlobalSymbol,
                             (ObjC2ClassNamePrefix + record.name).str(),
                             record.flags, target);
      if (record.isExportedSymbol(ObjCIFSymbolKind::MetaClass))
        symbolSet->addGlobal(EncodeKind::GlobalSymbol,
                             (ObjC2MetaClassNamePrefix + record.name).str(),
                             record.flags, target);
    }
  }

  auto addIvars = [&](ArrayRef<ObjCInstanceVariableRecord *> ivars) {
    for (auto *ivar : ivars) {
      if (!ivar->isExported())
        continue;
      // ObjC has an additional mechanism to specify if an ivar is exported or
      // not.
      if (ivar->accessControl == ObjCIvarDecl::Private ||
          ivar->accessControl == ObjCIvarDecl::Package)
        continue;
      std::string name =
          ObjCInstanceVariableRecord::createName(record.name, ivar->name);
      ivar->flags = getFlagsFromRecord(*ivar);
      symbolSet->addGlobal(EncodeKind::ObjectiveCInstanceVariable, name,
                           ivar->flags, target);
    }
  };
  addIvars(record.ivars);

  for (auto *category : record.categories) {
    addIvars(category->ivars);
  }
}

void API2SymbolConverter::visitObjCCategory(ObjCCategoryRecord &record) {
  auto addIvars = [&](ArrayRef<ObjCInstanceVariableRecord *> ivars) {
    for (auto *ivar : ivars) {
      if (!ivar->isExported())
        continue;
      // ObjC has an additional mechanism to specify if an ivar is exported or
      // not.
      if (ivar->accessControl == ObjCIvarDecl::Private ||
          ivar->accessControl == ObjCIvarDecl::Package)
        continue;
      std::string name =
          ObjCInstanceVariableRecord::createName(record.name, ivar->name);
      ivar->flags = getFlagsFromRecord(*ivar);
      symbolSet->addGlobal(EncodeKind::ObjectiveCInstanceVariable, name,
                           ivar->flags, target);
    }
  };

  addIvars(record.ivars);
}

TAPI_NAMESPACE_INTERNAL_END
