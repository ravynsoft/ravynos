//===- Frontend/SymbolVerifier.h - TAPI Symbol Verifier ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the Symbol Verifier
///
//===----------------------------------------------------------------------===//
#ifndef TAPI_CORE_SYMBOLVERIFIER_H
#define TAPI_CORE_SYMBOLVERIFIER_H

#include "tapi/Core/API.h"
#include "tapi/Core/Demangler.h"
#include "tapi/Core/LLVM.h"
#include "tapi/Diagnostics/Diagnostics.h"
#include "llvm/TextAPI/InterfaceFile.h"
#include "llvm/TextAPI/Platform.h"
#include "llvm/TextAPI/Symbol.h"
#include "llvm/TextAPI/SymbolSet.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

/// A list of InstallAPI verification modes.
enum class VerificationMode {
  Invalid,
  ErrorsOnly,
  ErrorsAndWarnings,
  Pedantic,
};

using APIInfo = llvm::SmallVector<
    std::tuple<Target, const APIRecord *, bool, clang::SourceManager *>, 6>;

/// Service responsible to tracking state of symbol verification across the
/// lifetime of InstallAPI.
class SymbolVerifier {
public:
  enum class Result { Invalid, Ignore, Valid };

  struct VerifierContext {
    // Current target being verified against the AST.
    Target target;
    // Target specific API from binary.
    API *dylibAPI;
    // Target specific API from enabling code coverage options.
    API *coverageAPI;
    // Track whether to print target or architecture banner.
    bool printArch;
    // Track whether to print banner.
    bool discoveredFirstError;
    // Query state of verification after AST has been traversed.
    Result frontendState;
    // Report errors.
    DiagnosticsEngine *diag;

    // Handle diagnostics reporting for target level violations.
    void emitDiag(std::function<void()> report);
  };

private:
  llvm::BumpPtrAllocator allocator;
  APIs dylib;
  InterfaceFile *swiftInterface;
  VerificationMode mode;
  bool demangle;
  Demangler demangler;
  bool autoZippered;
  bool isZippered;
  std::vector<APIs> reexportsToIgnore;
  APIs coverageSymbols;
  std::map<SimpleSymbol, SimpleSymbol> aliases;
  const StringRef dSYMPath;
  std::unique_ptr<SymbolSet> exports;
  bool verifiedSwift;
  VerifierContext ctx;
  struct SymbolContext;
  std::map<std::string, APIInfo> ignoredZipperedRecords;

  bool canVerify(const APIRecord *record, SymbolContext &ctx);
  Result checkVisibility(const APIRecord *dRecord, const APIRecord *record,
                         SymbolContext &symCtx);
  bool checkSymbolFlags(APIRecord *dRecord, const APIRecord *record,
                        SymbolContext &symCtx);
  Result checkAvailability(APIRecord *dRecord, const APIRecord *record,
                           SymbolContext &symCtx);
  bool checkObjCInterfaceSymbols(const ObjCInterfaceRecord *dRecord,
                                 const APIRecord *record,
                                 SymbolContext &symCtx);
  bool shouldIgnoreObsolete(const APIRecord *record, SymbolContext &symCtx,
                            APIRecord *dRecord);
  bool shouldIgnoreReexport(StringRef name, EncodeKind kind) const;
  bool shouldIgnoreZipperedAvailability(const APIRecord *record,
                                        SymbolContext &symCtx);
  bool shouldIgnoreZipperedSymbol(const APIRecord *record,
                                  const SymbolContext &symCtx) const;
  void addSymbol(const APIRecord *record, SymbolContext &symCtx,
                 TargetList targets = {});
  Result verifyImpl(const APIRecord *record, SymbolContext &symCtx);

  void updateFrontendState(Result state);
  void lookupAPIs(const Target &target);

public:
  SymbolVerifier()
      : dylib({}), swiftInterface(nullptr), mode(VerificationMode::Invalid),
        demangle(false), demangler({}), autoZippered(false), isZippered(false),
        reexportsToIgnore({}), coverageSymbols({}), aliases({}),
        dSYMPath(StringRef{}), exports(nullptr),
        ctx({Target(), nullptr, nullptr, false, false, Result::Ignore,
             nullptr}) {}

  SymbolVerifier(DiagnosticsEngine *diag, APIs &&dylib,
                 InterfaceFile *swiftInterface, VerificationMode mode,
                 bool demangle, bool autoZippered, bool isZippered,
                 std::vector<APIs> &&reexportsToIgnore, APIs &&coverageSymbols,
                 std::map<SimpleSymbol, SimpleSymbol> &&aliases,
                 const StringRef dSYMPath = {},
                 llvm::Triple triple = llvm::Triple())
      : dylib(std::move(dylib)), swiftInterface(swiftInterface), mode(mode),
        demangle(demangle), demangler({}), autoZippered(autoZippered),
        isZippered(isZippered), reexportsToIgnore(reexportsToIgnore),
        coverageSymbols(std::move(coverageSymbols)), aliases(aliases),
        dSYMPath(dSYMPath), exports(std::make_unique<SymbolSet>()),
        verifiedSwift(false) {
    ctx = VerifierContext{Target(triple), nullptr,        nullptr, false,
                          false,          Result::Ignore, diag};
  }

  bool setSourceManager(clang::SourceManager &sourceMgr) const {
    if (!ctx.diag)
      return false;
    ctx.diag->setSourceManager(&sourceMgr);
    return true;
  }

  void setTarget(llvm::Triple triple) {
    ctx.target = Target(triple);
    ctx.discoveredFirstError = false;
    updateFrontendState(Result::Ignore);
    lookupAPIs(ctx.target);
  }

  Result getFrontendState() const { return ctx.frontendState; }

  /// Compare remaining symbols for target slice.
  Result verifyRemainingSymbols(Architecture arch);

  Result verify(const GlobalRecord *record);
  Result verify(const ObjCInterfaceRecord *record);
  Result verify(const ObjCInstanceVariableRecord *record, StringRef superClass);

  // Check all symbols in swift interface are in dylib, this is done for all
  // targets.
  Result verifySwift();

  // Release ownership over exports.
  std::unique_ptr<SymbolSet> getExports();
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_SYMBOLVERIFIER_H
