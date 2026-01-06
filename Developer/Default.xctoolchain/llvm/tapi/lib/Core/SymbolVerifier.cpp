//===- lib/Core/SymbolVerifier.cpp - TAPI Symbol Verifier -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the TAPI Symbol Verification
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/SymbolVerifier.h"
#include "tapi/Core/API2SymbolConverter.h"
#include "tapi/Core/Demangler.h"
#include "tapi/Core/MachOReader.h"
#include "tapi/Defines.h"
#include "clang/AST/Attr.h"
#include "clang/AST/DeclObjC.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/TextAPI/InterfaceFile.h"
#include "llvm/TextAPI/Platform.h"
#include <type_traits>

TAPI_NAMESPACE_INTERNAL_BEGIN
using namespace llvm::MachO;

namespace {

std::string getAnnotatedName(const APIRecord *record, EncodeKind kind,
                             StringRef symbolName, bool validSourceLoc = true,
                             ObjCIFSymbolKind objCIF = ObjCIFSymbolKind::None) {
  assert(!symbolName.empty());

  std::string name;
  if (record->isWeakDefined())
    name += "(weak-def) ";
  if (record->isWeakReferenced())
    name += "(weak-ref) ";
  if (record->isThreadLocalValue())
    name += "(tlv) ";

  const bool isAnnotatedObjCClass = ((objCIF != ObjCIFSymbolKind::None) &&
                                     (objCIF <= ObjCIFSymbolKind::EHType));

  if (isAnnotatedObjCClass) {
    if (objCIF == ObjCIFSymbolKind::EHType)
      name += "Exception Type of ";
    if (objCIF == ObjCIFSymbolKind::MetaClass)
      name += "Metaclass of ";
    if (objCIF == ObjCIFSymbolKind::Class)
      name += "Class of ";
  }

  if (validSourceLoc) {
    if ((kind == EncodeKind::GlobalSymbol) && symbolName.startswith("_"))
      return name + symbolName.drop_front(1).str();
    return name + symbolName.str();
  }

  if (isAnnotatedObjCClass)
    return name + symbolName.str();

  // Only print symbol type prefix if there is no source location tied to it.
  // This can only ever happen when the location has to come from debug info.
  switch (kind) {
  case EncodeKind::GlobalSymbol:
    return name + symbolName.str();
  case EncodeKind::ObjectiveCInstanceVariable:
    return name + "(ObjC IVar) " + symbolName.str();
  case EncodeKind::ObjectiveCClass:
    return name + "(ObjC Class) " + symbolName.str();
  case EncodeKind::ObjectiveCClassEHType:
    return name + "(ObjC Class EH) " + symbolName.str();
  }

  llvm_unreachable("unexpected case for EncodeKind");
}

APIRecord *findRecordFromAPI(const API *api, StringRef name, EncodeKind kind) {
  switch (kind) {
  case EncodeKind::GlobalSymbol: {
    auto *record = api->findGlobal(name);
    return record;
  }
  case EncodeKind::ObjectiveCInstanceVariable: {
    auto *record = api->findIVar(name, name.contains('.'));
    return record;
  }
  case EncodeKind::ObjectiveCClass:
  case EncodeKind::ObjectiveCClassEHType: {
    auto *record = api->findObjCInterface(name);
    return record;
  }
  }
  llvm_unreachable("unexpected end when finding record");
}

// The existence of weak-defined RTTI can not always be inferred from the
// header files, because they can be generated as part of an implementation
// file.
// We do not warn about weak-defined RTTI, because this doesn't affect
// linking and can be ignored.
bool shouldIgnoreCXX(StringRef name, bool isWeakDef) {
  return (isWeakDef && (name.startswith("__ZTI") || name.startswith("__ZTS")));
}

// __private_extern__ is a deprecated specifier that clang does not
// respect in all contexts, it should just be ignored for installAPI.
bool shouldIgnorePrivateExternAttr(const APIRecord *record) {
  using namespace clang;

  if (const FunctionDecl *FD = cast<FunctionDecl>(record->decl))
    return FD->getStorageClass() == StorageClass::SC_PrivateExtern;
  if (const VarDecl *VD = cast<VarDecl>(record->decl))
    return VD->getStorageClass() == StorageClass::SC_PrivateExtern;

  return false;
}

SymbolVerifier::Result updateResult(const SymbolVerifier::Result prev,
                                    const SymbolVerifier::Result next) {
  if (prev == next)
    return prev;

  // Never update from Invalid state.
  if (prev == SymbolVerifier::Result::Invalid)
    return prev;

  // Don't let an ignored verification remove a valid one.
  if (prev == SymbolVerifier::Result::Valid &&
      next == SymbolVerifier::Result::Ignore)
    return prev;

  return next;
}

ObjCIFSymbolKind assignObjCIFSymbolKind(const ObjCInterfaceRecord record) {
  ObjCIFSymbolKind result = ObjCIFSymbolKind::None;
  if (record.getLinkageForSymbol(ObjCIFSymbolKind::Class) !=
      APILinkage::Unknown)
    result |= ObjCIFSymbolKind::Class;
  if (record.getLinkageForSymbol(ObjCIFSymbolKind::MetaClass) !=
      APILinkage::Unknown)
    result |= ObjCIFSymbolKind::MetaClass;
  if (record.getLinkageForSymbol(ObjCIFSymbolKind::EHType) !=
      APILinkage::Unknown)
    result |= ObjCIFSymbolKind::EHType;
  return result;
}

class DylibAPIVerifier : public APIVisitor {
private:
  struct DSYMContext {
    const StringRef path{};
    bool parsedDSYM{false};
    SymbolToSourceLocMap sourceLocs{};
  };

  SymbolVerifier::VerifierContext &ctx;
  const InterfaceFile *swiftFile;
  const std::map<SimpleSymbol, SimpleSymbol> &aliases;
  VerificationMode mode;
  bool demangle;
  Demangler &demangler;
  SymbolSet *verifiedSymbols;
  std::map<std::string, APIInfo> &ignoredZipperedRecords;
  DSYMContext dSYMCtx;
  SymbolVerifier::Result result;

  void updateState(SymbolVerifier::Result state) {
    result = updateResult(result, state);
  }

  void visitImpl(const APIRecord &record, const EncodeKind kind,
                 const StringRef name,
                 const ObjCIFSymbolKind objCIF = ObjCIFSymbolKind::None) {
    if (record.isExternal()) {
      updateState(SymbolVerifier::Result::Valid);
      return;
    }

    if (record.isInternal()) {
      updateState(SymbolVerifier::Result::Valid);
      return;
    }
    bool isLinkerSymbol = record.name.startswith("$ld$");
    // Handle zippered symbols with mismatching availability
    // between macOS and macCatalyst, if there exists an available
    // declaration, allow it.
    if (auto *sym = verifiedSymbols->findSymbol(kind, name, objCIF)) {
      for (auto &target : sym->targets()) {
        if (target.Arch != ctx.target.Arch)
          continue;
        updateState(SymbolVerifier::Result::Ignore);
        return;
      }
    }

    DemangledName demangledName = demangler.demangle(name);
    StringRef displayName = demangle ? demangledName.str : name;

    if (record.verified) {
      // Check for unavailable symbols.
      // This should only occur in the zippered case where we ignored
      // availability until all headers have been parsed.
      auto it = ignoredZipperedRecords.find(name.str());
      if (it == ignoredZipperedRecords.end()) {
        updateState(SymbolVerifier::Result::Valid);
        return;
      }

      llvm::SmallVector<std::tuple<const clang::SourceLocation,
                                   clang::SourceManager *, Target>,
                        2>
          locs;
      for (auto &[target, hRecord, isVerified, srcMgr] : it->second) {
        if (isVerified || hRecord->availability.isObsolete()) {
          updateState(SymbolVerifier::Result::Ignore);
          return;
        }
        if (target.Arch != ctx.target.Arch)
          continue;
        locs.emplace_back(hRecord->loc.getSourceLocation(), srcMgr, target);
        isVerified = true;
      }

      // Print violating declarations per platform.
      for (auto &[loc, srcMgr, target] : locs) {
        unsigned diagID = 0;
        if (mode == VerificationMode::Pedantic || isLinkerSymbol) {
          updateState(SymbolVerifier::Result::Invalid);
          diagID = diag::err_header_availability_mismatch;
        } else if (mode == VerificationMode::ErrorsAndWarnings) {
          updateState(SymbolVerifier::Result::Ignore);
          diagID = diag::warn_header_availability_mismatch;
        } else {
          updateState(SymbolVerifier::Result::Ignore);
          return;
        }
        ctx.diag->setSourceManager(srcMgr);
        ctx.diag->report(diag::warn_target) << getTargetTripleName(target);
        ctx.diag->report(diagID, loc)
            << getAnnotatedName(&record, kind, displayName)
            << !record.availability.isUnavailable()
            << !record.availability.isUnavailable();
      }

      return;
    }

    if (shouldIgnoreCXX(name, record.isWeakDefined())) {
      updateState(SymbolVerifier::Result::Valid);
      return;
    }

    if (swiftFile && swiftFile->getSymbol(kind, name, objCIF)) {
      updateState(SymbolVerifier::Result::Valid);
      return;
    }

    if (aliases.count(SimpleSymbol{name.str(), kind, objCIF})) {
      updateState(SymbolVerifier::Result::Valid);
      return;
    }

    if (ctx.coverageAPI && findRecordFromAPI(ctx.coverageAPI, name, kind)) {
      updateState(SymbolVerifier::Result::Valid);
      return;
    }

    // All checks at the point classify as some kind of violation that should be
    // reported. Request the source location for the matching symbol for error
    // reporting.
    accumulateSourceLocsFromDSYM();
    APILoc loc = dSYMCtx.sourceLocs.lookup(name);

    if (isLinkerSymbol) {
      ctx.emitDiag([&]() {
        ctx.diag->report(diag::err_header_symbol_missing, loc)
            << getAnnotatedName(&record, kind, displayName, !loc.isInvalid());
      });
      updateState(SymbolVerifier::Result::Invalid);
      return;
    }

    if (mode == VerificationMode::Pedantic) {
      if (demangledName.isSwift)
        ctx.emitDiag([&]() {
          ctx.diag->report(diag::err_swift_interface_symbol_missing, loc)
              << getAnnotatedName(&record, kind, displayName, !loc.isInvalid(),
                                  objCIF);
        });
      else
        ctx.emitDiag([&]() {
          ctx.diag->report(diag::err_header_symbol_missing, loc)
              << getAnnotatedName(&record, kind, displayName, !loc.isInvalid(),
                                  objCIF);
        });
      updateState(SymbolVerifier::Result::Invalid);
      return;
    }

    if (mode == VerificationMode::ErrorsAndWarnings) {
      if (demangledName.isSwift)
        ctx.emitDiag([&]() {
          ctx.diag->report(diag::warn_swift_interface_symbol_missing, loc)
              << getAnnotatedName(&record, kind, displayName, !loc.isInvalid(),
                                  objCIF);
        });
      else
        ctx.emitDiag([&]() {
          ctx.diag->report(diag::warn_header_symbol_missing, loc)
              << getAnnotatedName(&record, kind, displayName, !loc.isInvalid(),
                                  objCIF);
        });
    }

    updateState(SymbolVerifier::Result::Ignore);
    return;
  }

  void accumulateSourceLocsFromDSYM() {
    if (dSYMCtx.parsedDSYM)
      return;
    dSYMCtx.parsedDSYM = true;
    if (dSYMCtx.path.empty())
      return;

    dSYMCtx.sourceLocs = accumulateSourceLocFromDSYM(dSYMCtx.path, ctx.target);
  }

public:
  DylibAPIVerifier() = delete;
  DylibAPIVerifier(SymbolVerifier::VerifierContext &ctx,
                   const InterfaceFile *swiftFile,
                   const std::map<SimpleSymbol, SimpleSymbol> &aliases,
                   VerificationMode mode, bool demangle, Demangler &demangler,
                   SymbolSet *verifiedSymbols,
                   std::map<std::string, APIInfo> &ignoredZipperedRecords,
                   const StringRef dSYMPath)
      : ctx(ctx), swiftFile(swiftFile), aliases(aliases), mode(mode),
        demangle(demangle), demangler(demangler),
        verifiedSymbols(verifiedSymbols),
        ignoredZipperedRecords(ignoredZipperedRecords), dSYMCtx({dSYMPath}),
        result(SymbolVerifier::Result::Ignore) {}

  SymbolVerifier::Result getResult() { return result; }

  void visitGlobal(const GlobalRecord &record) override {
    auto sym = parseSymbol(record.name);
    visitImpl(record, sym.Kind, sym.Name);
  }

  void visitObjCInterface(const ObjCInterfaceRecord &record) override {
    ObjCIFSymbolKind symType = assignObjCIFSymbolKind(record);
    if (symType > ObjCIFSymbolKind::EHType) {
      if (record.hasExceptionAttribute())
        visitImpl(record, EncodeKind::ObjectiveCClassEHType, record.name,
                  symType);
      visitImpl(record, EncodeKind::ObjectiveCClass, record.name, symType);
    } else {
      visitImpl(record,
                record.hasExceptionAttribute()
                    ? EncodeKind::ObjectiveCClassEHType
                    : EncodeKind::ObjectiveCClass,
                record.name, symType);
    }

    for (auto *ivar : record.ivars) {
      visitImpl(
          *ivar, EncodeKind::ObjectiveCInstanceVariable,
          ObjCInstanceVariableRecord::createName(record.name, ivar->name));
    }
  }

  void visitObjCCategory(const ObjCCategoryRecord &record) override {
    for (auto *ivar : record.ivars)
      visitImpl(
          *ivar, EncodeKind::ObjectiveCInstanceVariable,
          ObjCInstanceVariableRecord::createName(record.interface, ivar->name));
  }
};

struct SimpleVisitor {
  API *api;
  void visit(APIMutator &visitor) { api->visit(visitor); }
  void visit(APIVisitor &visitor) { api->visit(visitor); }
};

} // namespace

void SymbolVerifier::lookupAPIs(const Target &target) {
  assert(target == ctx.target && "active targets should match.");

  auto coverageIt = find_if(coverageSymbols, [&target](const auto &api) {
    return target == api->getTarget();
  });

  if (coverageIt != coverageSymbols.end())
    ctx.coverageAPI = coverageIt->get();

  // Note: there are no reexport API entries with binaries so it can be
  // assumed that the target match is the active top-level library.
  if (!dylib.empty()) {
    auto it = find_if(dylib, [&target](const auto &api) {
      return target == api->getTarget();
    });
    // A matching target slice should always exist. The only exception is for
    // autozippering where verifying against matching slices
    // is disabled.
    if (autoZippered && it == dylib.end())
      return;

    assert(it != dylib.end() && "target slice should exist");
    ctx.dylibAPI = it->get();
  }
}

struct SymbolVerifier::SymbolContext {
  // Kind to map symbol type against APIRecord.
  EncodeKind kind = EncodeKind::GlobalSymbol;
  // Name to use for printing in diagnostics.
  std::string nameForPrinting{""};
  // Name to use for all querying and verification
  // purposes.
  std::string materializedName{""};
  // The ObjCInterface symbol type, if applicable.
  ObjCIFSymbolKind objCIF = ObjCIFSymbolKind::None;
  // Tracking inlined decls.
  bool inlined = false;
};

// Declarations mapped from reexported libraries should be ignored.
bool SymbolVerifier::shouldIgnoreReexport(StringRef name,
                                          EncodeKind kind) const {

  // Linker directive symbols can never be ignored.
  if (name.starts_with("$ld$"))
    return false;

  for (auto &lib : reexportsToIgnore) {
    for (auto &api : lib) {
      if (api->getTarget() != ctx.target)
        continue;
      auto *record = findRecordFromAPI(api.get(), name, kind);
      if (record)
        return true;
    }
  }
  return false;
}

bool SymbolVerifier::canVerify(const APIRecord *record, SymbolContext &symCtx) {
  if (dylib.empty())
    return false;
  if (!ctx.dylibAPI)
    return false;
  if (ctx.diag == nullptr)
    return false;
  return true;
}

void SymbolVerifier::VerifierContext::emitDiag(std::function<void()> report) {
  if (!discoveredFirstError) {
    diag->report(diag::warn_target)
        << (printArch ? getArchitectureName(target.Arch)
                      : getTargetTripleName(target));
    discoveredFirstError = true;
  }

  report();
}

SymbolVerifier::Result SymbolVerifier::checkVisibility(const APIRecord *dRecord,
                                                       const APIRecord *record,
                                                       SymbolContext &symCtx) {

  if (record->isExported() && !dRecord) {
    ctx.emitDiag([&]() {
      ctx.diag->report(diag::err_library_missing_symbol,
                       record->loc.getSourceLocation())
          << symCtx.nameForPrinting;
    });
    return Result::Invalid;
  }

  if (record->isExported() && dRecord->isInternal()) {
    ctx.emitDiag([&]() {
      ctx.diag->report(diag::err_library_hidden_symbol,
                       record->decl->getLocation())
          << symCtx.nameForPrinting;
    });
    return Result::Invalid;
  }

  // Emit a diagnostic for hidden declarations with external symbols, except
  // when its an inlined attribute.
  if ((record->isInternal() && !symCtx.inlined) && dRecord &&
      dRecord->isExported()) {

    if (mode == VerificationMode::ErrorsOnly)
      return Result::Ignore;

    if (shouldIgnorePrivateExternAttr(record))
      return Result::Ignore;

    if (shouldIgnoreZipperedSymbol(record, symCtx))
      return Result::Ignore;

    unsigned id;
    Result result;
    if (mode == VerificationMode::ErrorsAndWarnings) {
      id = diag::warn_header_hidden_symbol;
      result = Result::Ignore;
    } else {
      id = diag::err_header_hidden_symbol;
      result = Result::Invalid;
    }
    ctx.emitDiag([&]() {
      ctx.diag->report(id, record->decl->getLocation())
          << symCtx.nameForPrinting;
    });
    return result;
  }

  if (record->isInternal())
    return Result::Ignore;

  return Result::Valid;
}

bool SymbolVerifier::shouldIgnoreObsolete(const APIRecord *record,
                                          SymbolContext &symCtx,
                                          APIRecord *dRecord) {
  if (!record->availability.isObsolete())
    return false;

  if (isZippered)
    ignoredZipperedRecords[symCtx.materializedName].emplace_back(
        std::make_tuple(ctx.target, record, /*verified*/ false,
                        &ctx.diag->getSourceManager()));

  if (dRecord)
    dRecord->availability = record->availability;
  return true;
}

// Previously internal declarations got ignored if there exists a declaration
// that is exported in the macOS slice. The symbol verifier continues to do
// this to maintain that behavior.
bool SymbolVerifier::shouldIgnoreZipperedSymbol(
    const APIRecord *record, const SymbolContext &symCtx) const {
  if (!isZippered)
    return false;

  return exports->findSymbol(symCtx.kind, symCtx.materializedName,
                             symCtx.objCIF) != nullptr;
}

// Availability verification will be differed until all targets
// have been parsed.
bool SymbolVerifier::shouldIgnoreZipperedAvailability(const APIRecord *record,
                                                      SymbolContext &symCtx) {
  if (!(isZippered && record->availability.isUnavailable()))
    return false;

  ignoredZipperedRecords[symCtx.materializedName].emplace_back(std::make_tuple(
      ctx.target, record, /*verified*/ false, &ctx.diag->getSourceManager()));

  return true;
}

SymbolVerifier::Result
SymbolVerifier::checkAvailability(APIRecord *dRecord, const APIRecord *record,
                                  SymbolContext &symCtx) {
  if (!record->availability.isUnavailable())
    return Result::Valid;

  if (shouldIgnoreZipperedAvailability(record, symCtx))
    return Result::Ignore;

  // Capture missing availabilityInfo from dylib's API.
  dRecord->availability = record->availability;

  switch (mode) {
  case VerificationMode::ErrorsAndWarnings:
    ctx.emitDiag([&]() {
      ctx.diag->report(diag::warn_header_availability_mismatch,
                       record->loc.getSourceLocation())
          << symCtx.nameForPrinting << record->availability.isUnavailable()
          << record->availability.isUnavailable();
    });
    return Result::Ignore;
  case VerificationMode::Pedantic:
    ctx.emitDiag([&]() {
      ctx.diag->report(diag::err_header_availability_mismatch,
                       record->loc.getSourceLocation())
          << symCtx.nameForPrinting << record->availability.isUnavailable()
          << record->availability.isUnavailable();
    });
    return Result::Invalid;
  case VerificationMode::ErrorsOnly:
    return Result::Ignore;
  case VerificationMode::Invalid:
    llvm_unreachable("Unexpected verification mode symbol verification");
  }

  llvm_unreachable("Unexpected verification mode symbol verification");
}

// An ObjCInterfaceRecord can represent up to three symbols. When verifying,
// account for this granularity.
bool SymbolVerifier::checkObjCInterfaceSymbols(
    const ObjCInterfaceRecord *dRecord, const APIRecord *record,
    SymbolContext &symCtx) {
  const bool isDeclVersionComplete =
      ((symCtx.objCIF & ObjCIFSymbolKind::Class) == ObjCIFSymbolKind::Class) &&
      ((symCtx.objCIF & ObjCIFSymbolKind::MetaClass) ==
       ObjCIFSymbolKind::MetaClass);

  const bool isDylibVersionComplete = dRecord->isCompleteInterface();

  auto printDiagnostic = [&](auto symLinkage, auto *record, StringRef symName,
                             bool printAsWarning = false) {
    if (symLinkage == APILinkage::Unknown)
      ctx.emitDiag([&]() {
        ctx.diag->report(printAsWarning ? diag::warn_library_missing_symbol
                                        : diag::err_library_missing_symbol,
                         record->loc.getSourceLocation())
            << symName;
      });
    else
      ctx.emitDiag([&]() {
        ctx.diag->report(printAsWarning ? diag::warn_library_hidden_symbol
                                        : diag::err_library_hidden_symbol,
                         record->decl->getLocation())
            << symName;
      });
  };

  if (isDeclVersionComplete) {
    // The common case, a complete ObjCInterface.
    if (isDylibVersionComplete)
      return true;

    // The decl represents a complete ObjCInterface, but the symbols in the
    // dylib do not. Determine which symbol is missing. To keep older projects
    // building, treat this as a warning.
    if (!dRecord->isExportedSymbol(ObjCIFSymbolKind::Class))
      printDiagnostic(
          dRecord->getLinkageForSymbol(ObjCIFSymbolKind::Class), record,
          getAnnotatedName(record, symCtx.kind, symCtx.nameForPrinting,
                           /*validSourceLoc=*/true, ObjCIFSymbolKind::Class),
          /*printAsWarning=*/true);

    if (!dRecord->isExportedSymbol(ObjCIFSymbolKind::MetaClass))
      printDiagnostic(
          dRecord->getLinkageForSymbol(ObjCIFSymbolKind::MetaClass), record,
          getAnnotatedName(record, symCtx.kind, symCtx.nameForPrinting,
                           /*validSourceLoc=*/true,
                           ObjCIFSymbolKind::MetaClass),
          /*printAsWarning=*/true);
    return true;
  }

  if (dRecord->isExportedSymbol(symCtx.objCIF)) {
    if (!isDylibVersionComplete) {
      // Both the declaration and dylib have a non-complete interface.
      symCtx.kind = EncodeKind::GlobalSymbol;
      symCtx.materializedName = record->name;
    }
    return true;
  }

  // At this point that means there was not a matching class symbol
  // to represent the one discovered as a declaration.
  printDiagnostic(dRecord->getLinkageForSymbol(symCtx.objCIF), record,
                  symCtx.nameForPrinting);
  return false;
}

bool SymbolVerifier::checkSymbolFlags(APIRecord *dRecord,
                                      const APIRecord *record,
                                      SymbolContext &symCtx) {
  std::string displayName =
      demangle ? demangler.demangle(dRecord->name).str : dRecord->name.str();

  if (dRecord->isThreadLocalValue() && !record->isThreadLocalValue()) {
    ctx.emitDiag([&]() {
      ctx.diag->report(diag::err_dylib_symbol_flags_mismatch,
                       record->loc.getSourceLocation())
          << getAnnotatedName(dRecord, symCtx.kind, displayName)
          << dRecord->isThreadLocalValue();
    });
    return false;
  }
  if (!dRecord->isThreadLocalValue() && record->isThreadLocalValue()) {
    ctx.emitDiag([&]() {
      ctx.diag->report(diag::err_header_symbol_flags_mismatch,
                       record->loc.getSourceLocation())
          << symCtx.nameForPrinting << record->isThreadLocalValue();
    });
    return false;
  }

  if (dRecord->isWeakDefined() && !record->isWeakDefined()) {
    ctx.emitDiag([&]() {
      ctx.diag->report(diag::err_dylib_symbol_flags_mismatch,
                       record->loc.getSourceLocation())
          << getAnnotatedName(dRecord, symCtx.kind, displayName)
          << record->isWeakDefined();
    });
    return false;
  }
  if (!dRecord->isWeakDefined() && record->isWeakDefined()) {
    ctx.emitDiag([&]() {
      ctx.diag->report(diag::err_header_symbol_flags_mismatch,
                       record->loc.getSourceLocation())
          << symCtx.nameForPrinting << record->isWeakDefined();
    });
    return false;
  }

  return true;
}

void SymbolVerifier::updateFrontendState(Result state) {
  ctx.frontendState = updateResult(ctx.frontendState, state);
}

void SymbolVerifier::addSymbol(const APIRecord *record, SymbolContext &symCtx,
                               TargetList targets) {
  if (targets.empty())
    targets = {ctx.target};

  exports->addGlobal(symCtx.kind, symCtx.materializedName, record->flags,
                     targets);
}

SymbolVerifier::Result SymbolVerifier::verifyImpl(const APIRecord *record,
                                                  SymbolContext &symCtx) {
  if (!canVerify(record, symCtx)) {
    // Accumulate symbols when not in verifying against dylib.
    if (ctx.target != Target() && record->isExported() &&
        !record->availability.isObsolete() &&
        !record->availability.isUnavailable() &&
        !shouldIgnoreReexport(symCtx.materializedName, symCtx.kind)) {
      addSymbol(record, symCtx);
    }
    updateFrontendState(Result::Ignore);
    return ctx.frontendState;
  }

  if (shouldIgnoreReexport(symCtx.materializedName, symCtx.kind)) {
    updateFrontendState(Result::Ignore);
    return ctx.frontendState;
  }

  auto *dRecord =
      findRecordFromAPI(ctx.dylibAPI, symCtx.materializedName, symCtx.kind);
  if (dRecord)
    dRecord->verified = true;

  if (shouldIgnoreObsolete(record, symCtx, dRecord)) {
    updateFrontendState(Result::Ignore);
    return ctx.frontendState;
  }

  if (record->availability.isUnavailable() &&
      (!dRecord || dRecord->isInternal())) {
    updateFrontendState(Result::Valid);
    return ctx.frontendState;
  }

  Result visibilityCheck = checkVisibility(dRecord, record, symCtx);
  if (visibilityCheck != Result::Valid) {
    updateFrontendState(visibilityCheck);
    return ctx.frontendState;
  }

  // All missing symbol cases to diagnose have been handled.
  if (!dRecord) {
    updateFrontendState(Result::Ignore);
    return ctx.frontendState;
  }

  // Check for mismatching ObjC Interfaces.
  if (symCtx.objCIF != ObjCIFSymbolKind::None) {
    if (!checkObjCInterfaceSymbols(
            ctx.dylibAPI->findObjCInterface(dRecord->name), record, symCtx)) {
      updateFrontendState(Result::Invalid);
      return ctx.frontendState;
    }
  }

  Result availabilityCheck = checkAvailability(dRecord, record, symCtx);
  if (availabilityCheck != Result::Valid) {
    updateFrontendState(availabilityCheck);
    return ctx.frontendState;
  }

  if (!checkSymbolFlags(dRecord, record, symCtx)) {
    updateFrontendState(Result::Invalid);
    return ctx.frontendState;
  }

  addSymbol(record, symCtx);
  updateFrontendState(Result::Valid);
  return ctx.frontendState;
}

SymbolVerifier::Result SymbolVerifier::verify(const GlobalRecord *record) {
  // Global Symbol classifications could be obfusciated with `asm`
  auto sym = parseSymbol(record->name);
  SymbolContext symCtx;
  symCtx.kind = sym.Kind;
  symCtx.materializedName = sym.Name;
  symCtx.objCIF = sym.ObjCInterfaceType;
  symCtx.nameForPrinting =
      getAnnotatedName(record, symCtx.kind,
                       demangle ? demangler.demangle(sym.Name).str : sym.Name,
                       /*validSourceLoc=*/true, symCtx.objCIF);
  symCtx.inlined = record->inlined;

  return verifyImpl(record, symCtx);
}

SymbolVerifier::Result
SymbolVerifier::verify(const ObjCInterfaceRecord *record) {
  SymbolContext symCtx;
  symCtx.materializedName = record->name;
  symCtx.objCIF = assignObjCIFSymbolKind(*record);

  std::string displayName =
      demangle ? demangler.demangle(record->name).str : record->name.str();
  if (record->hasExceptionAttribute()) {
    symCtx.kind = EncodeKind::ObjectiveCClassEHType;
    symCtx.nameForPrinting = getAnnotatedName(record, symCtx.kind, displayName);
  } else {
    symCtx.kind = EncodeKind::ObjectiveCClass;
    symCtx.nameForPrinting = getAnnotatedName(record, symCtx.kind, displayName);
  }

  return verifyImpl(record, symCtx);
}

SymbolVerifier::Result
SymbolVerifier::verify(const ObjCInstanceVariableRecord *record,
                       StringRef superClass) {
  if (record->accessControl == clang::ObjCIvarDecl::Private ||
      record->accessControl == clang::ObjCIvarDecl::Package)
    return Result::Ignore;

  auto fullName =
      ObjCInstanceVariableRecord::createName(superClass, record->name);
  std::string displayName =
      demangle ? demangler.demangle(fullName).str : fullName;
  SymbolContext symCtx{EncodeKind::ObjectiveCInstanceVariable,
                       getAnnotatedName(record,
                                        EncodeKind::ObjectiveCInstanceVariable,
                                        displayName),
                       fullName};

  return verifyImpl(record, symCtx);
}

std::unique_ptr<SymbolSet> SymbolVerifier::getExports() {
  for (auto &cov : coverageSymbols) {
    SimpleVisitor visitor{cov.get()};
    API2SymbolConverter converter(exports.get(), cov->getTarget());
    visitor.visit(converter);
  }

  for (const auto &[alias, base] : aliases) {
    APIRecord record;
    record.name = alias.Name;
    TargetList targets;
    if (auto *sym = exports->findSymbol(base.Kind, base.Name)) {
      record.flags = sym->getFlags();
      targets = {sym->targets().begin(), sym->targets().end()};
    } else {
      record.linkage = APILinkage::Exported;
      record.flags = SymbolFlags::None;
      record.access = APIAccess::Private;
      for (auto &api : dylib)
        targets.emplace_back(api->getTarget());
    }

    SymbolContext symCtx;
    symCtx.materializedName = alias.Name;
    symCtx.kind = alias.Kind;
    addSymbol(&record, symCtx, targets);
  }

  return std::move(exports);
}

SymbolVerifier::Result
SymbolVerifier::verifyRemainingSymbols(Architecture arch) {
  if (dylib.empty())
    return Result::Ignore;

  auto *it = find_if(dylib, [&arch](const auto &api) {
    return arch == api->getTarget().Arch;
  });
  if (it == dylib.end())
    return Result::Ignore;
  auto api = *it;

  ctx.discoveredFirstError = false;
  ctx.printArch = true;
  DylibAPIVerifier apiVerifier(ctx, verifiedSwift ? nullptr : swiftInterface,
                               aliases, mode, demangle, demangler,
                               exports.get(), ignoredZipperedRecords, dSYMPath);
  ctx.target = api->getTarget();
  SimpleVisitor visitor{api.get()};
  visitor.visit(apiVerifier);
  return apiVerifier.getResult();
}

SymbolVerifier::Result SymbolVerifier::verifySwift() {
  if (!swiftInterface)
    return Result::Ignore;

  if (dylib.empty())
    return Result::Ignore;

  Result result = Result::Valid;

  for (auto *sym : swiftInterface->exports()) {
    for (auto &dylibAPI : dylib) {
      if (!sym->hasTarget(dylibAPI->getTarget()))
        continue;
      auto *dSym =
          findRecordFromAPI(dylibAPI.get(), sym->getName(), sym->getKind());
      if (dSym) {
        dSym->verified = true;
        continue;
      }
      std::string swiftName = demangle ? demangler.demangle(sym->getName()).str
                                       : sym->getName().str();
      result = Result::Invalid;
      ctx.diag->report(diag::err_swift_dylib_symbol_missing) << swiftName;
    }
  }
  verifiedSwift = true;
  return result;
}

TAPI_NAMESPACE_INTERNAL_END
