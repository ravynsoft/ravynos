//===--- TAPIStructuralEquivalence.h - --------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the StructuralEquivalenceContext class which checks for
//  structural equivalence between types.
//
//===----------------------------------------------------------------------===//

#ifndef TAPI_APIVERIFIER_TAPISTRUCTURALEQUIVALENCE_H
#define TAPI_APIVERIFIER_TAPISTRUCTURALEQUIVALENCE_H

#include "tapi/APIVerifier/APIVerifier.h"
#include "tapi/Defines.h"
#include "tapi/Diagnostics/Diagnostics.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSet.h"
#include <optional>

namespace clang {

class ASTContext;
class Decl;
class DiagnosticBuilder;
class QualType;
class RecordDecl;
class SourceLocation;

} // namespace clang

using clang::ASTContext;
using clang::Decl;
using clang::DiagnosticBuilder;
using clang::QualType;
using clang::RecordDecl;
using clang::SourceLocation;
using clang::SourceManager;

TAPI_NAMESPACE_INTERNAL_BEGIN

// A list of location for diagnostics.
struct LocTrace {
  LocTrace() = default;
  ~LocTrace();

  using TraceVector = llvm::SmallVector<clang::SourceLocation, 2>;
  TraceVector L1;
  TraceVector L2;

  void extend(const LocTrace &other);
};

struct TAPIDiagParameter {
  enum ParamType {
    Unknown = 0,
    Int,
    String,
    DeclName,
    QualType
  } type;

  union ParamValue {
    int num;
    StringRef str;
    clang::DeclarationName name;
    clang::QualType type;

    ParamValue() : num(0) {}
    ParamValue(int num) : num(num) {}
    ParamValue(StringRef str) : str(str) {}
    ParamValue(clang::DeclarationName name) : name(name) {}
    ParamValue(clang::QualType type) : type(type) {}
    ~ParamValue() {}
  } value;

  TAPIDiagParameter() : type(Unknown), value(0) {}
  TAPIDiagParameter(int num) : type(Int), value(num) {}
  TAPIDiagParameter(StringRef str) : type(String) {
    strValue = str.str();
    value.str = strValue;
  }

  TAPIDiagParameter(clang::DeclarationName name)
      : type(DeclName), value(name) {}
  TAPIDiagParameter(clang::QualType type) : type(QualType), value(type) {}
  
  TAPIDiagParameter(const TAPIDiagParameter& other) {
    *this = other;
  }

  TAPIDiagParameter &operator=(const TAPIDiagParameter &other) {
    type = other.type;
    switch (type) {
    case Unknown:
      break;
    case Int:
      value.num = other.value.num;
      break;
    case String:
      strValue = other.strValue;
      value.str = strValue;
      break;
    case DeclName:
      value.name = other.value.name;
      break;
    case QualType:
      value.type = other.value.type;
      break;
    }
    return *this;
  }

  std::string strValue;
};

class TAPIDiffDiagnostic {
public:
  TAPIDiffDiagnostic(DiagnosticsEngine *engine, unsigned diagID,
                     clang::SourceLocation location)
      : diagEng(engine), diagID(diagID), location(location) {}

  void addString(StringRef string) { parameters.emplace_back(string); }

  void addDeclName(clang::DeclarationName name) {
    parameters.emplace_back(name);
  }

  void addIntValue(int num) {
    parameters.emplace_back(num);
  }

  void addQualType(clang::QualType type) {
    parameters.emplace_back(type);
  }

  void emitDiag() const {
    auto diag = diagEng->report(location, diagID);
    for (auto &param: parameters) {
      switch (param.type) {
      case TAPIDiagParameter::Int:
        diag << param.value.num;
        break;
      case TAPIDiagParameter::String:
        diag << param.value.str;
        break;
      case TAPIDiagParameter::DeclName:
        diag << param.value.name;
        break;
      case TAPIDiagParameter::QualType:
        diag << param.value.type;
        break;
      case TAPIDiagParameter::Unknown:
        llvm_unreachable("Unknown parameter for diagnostics");
      }
    }
  }

  DiagnosticsEngine *getDiagEngine() const { return diagEng; }
private:
  DiagnosticsEngine *diagEng;
  unsigned diagID;
  clang::SourceLocation location;

  using ParamVector = llvm::SmallVector<TAPIDiagParameter, 2>;
  ParamVector parameters;
};

using DiagVector = llvm::SmallVector<TAPIDiffDiagnostic, 2>;

struct DiagTrace {
  DiagVector D1;
  DiagVector D2;

  void extend(const DiagTrace &other);
  void clear() {
    D1.clear();
    D2.clear();
  }
};

class TAPIDiagBuilder {
public:
  TAPIDiagBuilder(DiagnosticsEngine *engine, SourceLocation loc,
                  unsigned diagID, DiagVector &diags) {
    diags.emplace_back(engine, diagID, loc);
    diagnostic = &diags.back();
  }

  TAPIDiffDiagnostic *getDiag() const { return diagnostic; }
private:
  TAPIDiffDiagnostic *diagnostic;
};

inline const TAPIDiagBuilder &operator<<(const TAPIDiagBuilder &DB,
                                         StringRef str) {
  DB.getDiag()->addString(str);
  return DB;
}

inline const TAPIDiagBuilder &operator<<(const TAPIDiagBuilder &DB, int num) {
  DB.getDiag()->addIntValue(num);
  return DB;
}

inline const TAPIDiagBuilder &operator<<(const TAPIDiagBuilder &DB,
                                         clang::DeclarationName name) {
  DB.getDiag()->addDeclName(name);
  return DB;
}

inline const TAPIDiagBuilder &operator<<(const TAPIDiagBuilder &DB,
                                         clang::QualType type) {
  DB.getDiag()->addQualType(type);
  return DB;
}

class StructuralEquivalenceContext {
public:
  StructuralEquivalenceContext(
      const APIVerifierConfiguration &config, DiagnosticsEngine &Diag,
      FrontendContext *FromFrontendCtx, FrontendContext *ToFrontendCtx,
      bool StrictTypeSpelling = false,
      APIVerifierDiagStyle Style = APIVerifierDiagStyle::Warning,
      bool CheckExternalHeaders = true, bool DiagMissingAPI = false,
      bool EmitCascadingDiags = false);

  using DeclPair = std::pair<const Decl *, const Decl *>;

  /// Determine whether all the DeclPairs are the same.
  bool diagnoseStructurallyEquivalent(std::vector<DeclPair> &&DeclPairs) {
    DeclsToCompare.insert(DeclPairs.begin(), DeclPairs.end());
    for (auto &it : DeclsToCompare)
      diagnoseStructurallyEquivalent(it.first, it.second);

    return hasErrorOccurred();
  }

  /// Determine whether the two declarations are structurally
  /// equivalent and emit diagnostics.
  bool diagnoseStructurallyEquivalent(const Decl *D1, const Decl *D2);

  /// Compare and cached.
  bool checkStructurallyEquivalent(const Decl *D1, const Decl *D2);

  /// Determine whether the two types are structurally equivalent and keep
  /// track of the locations.
  bool checkStructurallyEquivalent(QualType T1, QualType T2);

  TAPIDiagBuilder Diag1(SourceLocation Loc, unsigned DiagID);
  TAPIDiagBuilder Diag2(SourceLocation Loc, unsigned DiagID);

  void setDiagnosticDepth(unsigned depth) {
    if (depth)
      DiagnosticDepth = depth;
  }

  /// Find the index of the given anonymous struct/union within its
  /// context.
  ///
  /// \returns Returns the index of this anonymous struct/union in its context,
  /// including the next assigned index (if none of them match). Returns an
  /// empty option if the context is not a record, i.e.. if the anonymous
  /// struct/union is at namespace or block scope.
  ///
  /// FIXME: This is needed by ASTImporter and ASTStructureEquivalence. It
  /// probably makes more sense in some other common place then here.
  static std::optional<unsigned>
  findUntaggedStructOrUnionIndex(const RecordDecl *Anon);

  /// Check to see if the decl should be compared.
  bool shouldCheckDecls(const Decl *D1, const Decl *D2);

  void addEqualDecl(const Decl *D1, const Decl *D2);
  bool isKnowEqual(const Decl *D1, const Decl *D2) const;

  bool shouldCheckMissingAPIs() const { return CheckMissingAPIs; }

  bool hasErrorOccurred() const {
    return FromDiag.hasErrorOccurred() || ToDiag.hasErrorOccurred();
  }

  /// Reset the context for a new comparsion.
  void resetContext();

private:
  /// Check cache for equivalence.
  bool checkCacheForEquivalence(const Decl *D1, const Decl *D2);

  void pushContext();
  void popContext();
  /// Compare and not cached.
  bool isDeclEquivalent(const Decl *D1, const Decl *D2);

public:
  /// AST contexts for which we are checking structural equivalence.
  ASTContext &FromCtx, &ToCtx;

  /// Whether we're being strict about the spelling of types when
  /// unifying two types.
  bool StrictTypeSpelling;

  /// Exception lists.
  llvm::StringSet<> IgnoreObjCClasses;
  llvm::StringMap<StringRef> BridgeObjCClasses;

private:
  // FrontendContext to compare information used for
  // compiler invocation.
  FrontendContext *FromFrontendCtx;
  FrontendContext *ToFrontendCtx;

  /// DiagnosticsEngine to use for reporting issues.
  DiagnosticsEngine FromDiag, ToDiag;

  /// Declaration (from, to) pairs that are known not to be equivalent
  /// (which we have already complained about).
  llvm::DenseMap<DeclPair, DiagTrace> NonEquivalentDecls;

  /// All the decl pairs are known to be equal.
  llvm::DenseSet<DeclPair> EqualDecls;

  /// All the decl pairs that need to compared.
  llvm::SetVector<DeclPair> DeclsToCompare;

  /// Declaration (from, to) pairs that need to be compared.
  using CompareContext = llvm::SetVector<DeclPair>;
  SmallVector<CompareContext, 2> ComparsionStacks;
  CompareContext *TentativeComparsions;

  DiagTrace StoredDiagnostics;

  unsigned DiagnosticDepth = 4; // maximum ComparsionStack depth.

  /// Whether warn or error on external header content
  bool CheckExternalHeaders;

  /// Whether warn or error on missing APIs
  bool CheckMissingAPIs;

  /// Whether not to emit cascading diagnostics
  bool EmitCascadingDiags;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_APIVERIFIER_TAPISTRUCTURALEQUIVALENCE_H
