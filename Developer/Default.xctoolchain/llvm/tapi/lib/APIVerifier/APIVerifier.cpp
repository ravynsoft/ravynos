//===- lib/APIVerifier/APIVerifier.cpp - TAPI API Verifier ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the TAPI API Verifier.
///
//===----------------------------------------------------------------------===//

#include "tapi/APIVerifier/APIVerifier.h"
#include "TAPIStructuralEquivalence.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Type.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/YAMLTraits.h"

using namespace llvm;
using namespace clang;

// YAML definition
using namespace llvm::yaml;
using namespace TAPI_INTERNAL;

namespace llvm {
namespace yaml {

template <> struct MappingTraits<APIVerifierConfiguration::BridgeTypes> {
  static void mapping(IO &io, APIVerifierConfiguration::BridgeTypes &types) {
    io.mapOptional("base", types.first);
    io.mapOptional("to", types.second);
  }
};

template <> struct MappingTraits<APIVerifierConfiguration> {
  static void mapping(IO &io, APIVerifierConfiguration &config) {
    io.mapOptional("ignore-objc-class", config.IgnoreObjCClasses);
    io.mapOptional("bridge-objc-class", config.BridgeObjCClasses);
  }
};

} // namespace yaml
} // namespace llvm

TAPI_NAMESPACE_INTERNAL_BEGIN

Error APIVerifierConfiguration::readConfig(MemoryBufferRef memBuffer) {
  Input yin(memBuffer.getBuffer());
  yin >> *this;
  if (yin.error())
    return errorCodeToError(yin.error());

  return Error::success();
}

void APIVerifierConfiguration::writeConfig(raw_ostream &os) {
  Output yout(os);
  yout << *this;
}

void APIVerifier::verify(FrontendContext &api1, FrontendContext &api2,
                         unsigned depth, bool external,
                         APIVerifierDiagStyle style, bool diagMissingAPI,
                         bool avoidCascadingDiags) {

  StructuralEquivalenceContext equivalence(
      config, diag, &api1, &api2,
      /*StrictTypeSpelling=*/false, /*DiagStyle=*/style,
      /*CheckExternalHeaders=*/external, /*DiagMissingAPI*/ diagMissingAPI,
      /*EmitCascadingDiags=*/!avoidCascadingDiags);

  equivalence.setDiagnosticDepth(depth);

  // Diagnose missing api. We only diagnose missing APIs that is required from
  // target varient (api2) but missing from the baseline (api1).
  auto diagnoseMissingAPI = [&](const APIRecord *record) {
    if (!equivalence.shouldCheckMissingAPIs() || !record->decl ||
        record->availability._unavailable)
      return;

    if (const auto *ND = dyn_cast<NamedDecl>(record->decl)) {
      equivalence.Diag2(ND->getLocation(),
                        TAPI_INTERNAL::diag::warn_api_incomplete)
          << ND->getDeclName() << api1.target.getTriple()
          << api2.target.getTriple();
      equivalence.resetContext(); // reset context to flush diagnostics.
    }
  };

  std::vector<StructuralEquivalenceContext::DeclPair> DeclToCompare;
  // Function to add APIs to be compared.
  auto addAPIToCompare = [&DeclToCompare](const APIRecord *r1,
                                          const APIRecord *r2) {
    // If APIRecord has no decl, there is nothing to compare.
    if (!r1->decl || !r2->decl)
      return;

    // If it is unavailable in the target-variant, then skip the comparsion.
    // It is not ok if it is the other way around.
    if (r2->availability.isUnavailable())
      return;

    DeclToCompare.emplace_back(r1->decl, r2->decl);
  };

  for (auto &it : api2.api->typeDefs) {
    auto *record = api1.api->findTypeDef(it.first);
    if (!record)
      continue; // allow missing typedef.

    addAPIToCompare(record, it.second);
  }

  for (auto &it : api2.api->globals) {
    if (it.second->kind != GVKind::Variable)
      continue;
    auto *record = api1.api->findGlobalVariable(it.first);
    if (!record)
      diagnoseMissingAPI(it.second);
    else
      addAPIToCompare(record, it.second);
  }

  for (auto &it : api2.api->globals) {
    if (it.second->kind != GVKind::Function)
      continue;
    auto *record = api1.api->findFunction(it.first);
    if (!record)
      diagnoseMissingAPI(it.second);
    else
      addAPIToCompare(record, it.second);
  }

  for (auto &it : api2.api->enums) {
    auto *record = api1.api->findEnum(it.first);
    // FIXME: this was missing enum *constants* before the change.
    // Do we still allow missing enum decls?
    if (!record)
      continue; // allow missing enum.
    addAPIToCompare(record, it.second);
    for (const auto *c2 : it.second->constants) {
      const auto c1 = find_if(record->constants, [&](const auto *c) {
        return c->name == c2->name;
      });
      if (c1 == record->constants.end())
        continue; // allow missing enum constant.
      addAPIToCompare(*c1, c2);
    }
  }

  for (auto &it : api2.api->interfaces) {
    auto *record = api1.api->findObjCInterface(it.first);
    if (!record)
      diagnoseMissingAPI(it.second);
    else
      addAPIToCompare(record, it.second);
  }

  for (auto &it : api2.api->protocols) {
    auto *record = api1.api->findObjCProtocol(it.first);
    if (!record)
      diagnoseMissingAPI(it.second);
    else
      addAPIToCompare(record, it.second);
  }

  for (auto &it : api2.api->categories) {
    auto *record = api1.api->findObjCCategory(it.first.first, it.first.second);
    if (!record)
      diagnoseMissingAPI(it.second);
    else
      addAPIToCompare(record, it.second);
  }

  // sort DeclToCompare in source order.
  llvm::sort(DeclToCompare.begin(), DeclToCompare.end(),
             [](const StructuralEquivalenceContext::DeclPair &L,
                const StructuralEquivalenceContext::DeclPair &R) {
               return L.first->getLocation().getRawEncoding() <
                      R.first->getLocation().getRawEncoding();
             });

  hasError |=
      equivalence.diagnoseStructurallyEquivalent(std::move(DeclToCompare));
}

TAPI_NAMESPACE_INTERNAL_END
