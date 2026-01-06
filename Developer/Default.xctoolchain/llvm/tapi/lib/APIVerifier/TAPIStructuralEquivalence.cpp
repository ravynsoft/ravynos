//===--- TAPIStructuralEquivalence.cpp - ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implement StructuralEquivalenceContext class and helper functions
//  for layout matching.
//
//===----------------------------------------------------------------------===//

#include "TAPIStructuralEquivalence.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTDiagnostic.h"
#include "clang/AST/ASTImporter.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclVisitor.h"
#include "clang/AST/StmtVisitor.h"
#include "clang/AST/TypeVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSet.h"

namespace {

using namespace clang;
using namespace TAPI_INTERNAL;

static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     QualType T1, QualType T2);
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const Decl *D1, const Decl *D2);
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const TemplateArgument &Arg1,
                                     const TemplateArgument &Arg2);
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const VarDecl *D1, const VarDecl *D2);
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const ParmVarDecl *D1,
                                     const ParmVarDecl *D2);
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const FunctionDecl *D1,
                                     const FunctionDecl *D2);
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const ObjCInterfaceDecl *D1,
                                     const ObjCInterfaceDecl *D2);
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const ObjCProtocolDecl *D1,
                                     const ObjCProtocolDecl *D2);
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const ObjCCategoryDecl *D1,
                                     const ObjCCategoryDecl *D2);
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const ObjCMethodDecl *D1,
                                     const ObjCMethodDecl *D2);
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const ObjCPropertyDecl *D1,
                                     const ObjCPropertyDecl *D2);
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const ObjCIvarDecl *D1,
                                     const ObjCIvarDecl *D2);
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const EnumDecl *D1, const EnumDecl *D2);
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const TypedefNameDecl *D1,
                                     const TypedefNameDecl *D2);

/// Determine structural equivalence of two expressions.
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     Expr *E1, Expr *E2) {
  if (!E1 || !E2)
    return E1 == E2;

  // FIXME: Actually perform a structural comparison!
  return true;
}

/// Determine whether two identifiers are equivalent.
static bool IsStructurallyEquivalent(const IdentifierInfo *Name1,
                                     const IdentifierInfo *Name2) {
  if (!Name1 || !Name2)
    return Name1 == Name2;

  return Name1->getName() == Name2->getName();
}

static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const IdentifierInfo *Name1,
                                     const Decl *D1,
                                     const IdentifierInfo *Name2,
                                     const Decl *D2) {
  if (!IsStructurallyEquivalent(Name1, Name2)) {
    Context.Diag1(D1->getLocation(), TAPI_INTERNAL::diag::note_api_decl_name)
        << Name1;
    Context.Diag2(D2->getLocation(), TAPI_INTERNAL::diag::note_api_decl_name)
        << Name2;
    return false;
  }
  return true;
}

static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const TypedefNameDecl *D1,
                                     const TypedefNameDecl *D2) {
  // If compare the underlying type and record the location.
  if (!Context.checkStructurallyEquivalent(D1->getUnderlyingType(),
                                           D2->getUnderlyingType())) {
    Context.Diag1(D1->getTypeSourceInfo()->getTypeLoc().getBeginLoc(),
                  TAPI_INTERNAL::diag::note_api_type_def)
        << D1->getDeclName() << D1->getUnderlyingType();
    Context.Diag2(D2->getTypeSourceInfo()->getTypeLoc().getBeginLoc(),
                  TAPI_INTERNAL::diag::note_api_type_def)
        << D2->getDeclName() << D2->getUnderlyingType();
    return false;
  }

  return true;
}

/// Determine whether two nested-name-specifiers are equivalent.
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     NestedNameSpecifier *NNS1,
                                     NestedNameSpecifier *NNS2) {
  if (NNS1->getKind() != NNS2->getKind())
    return false;

  NestedNameSpecifier *Prefix1 = NNS1->getPrefix(),
                      *Prefix2 = NNS2->getPrefix();
  if ((bool)Prefix1 != (bool)Prefix2)
    return false;

  if (Prefix1)
    if (!IsStructurallyEquivalent(Context, Prefix1, Prefix2))
      return false;

  switch (NNS1->getKind()) {
  case NestedNameSpecifier::Identifier:
    return IsStructurallyEquivalent(NNS1->getAsIdentifier(),
                                    NNS2->getAsIdentifier());
  case NestedNameSpecifier::Namespace:
    return IsStructurallyEquivalent(Context, NNS1->getAsNamespace(),
                                    NNS2->getAsNamespace());
  case NestedNameSpecifier::NamespaceAlias:
    return IsStructurallyEquivalent(Context, NNS1->getAsNamespaceAlias(),
                                    NNS2->getAsNamespaceAlias());
  case NestedNameSpecifier::TypeSpec:
  case NestedNameSpecifier::TypeSpecWithTemplate:
    return Context.checkStructurallyEquivalent(QualType(NNS1->getAsType(), 0),
                                               QualType(NNS2->getAsType(), 0));
  case NestedNameSpecifier::Global:
    return true;
  case NestedNameSpecifier::Super:
    return Context.checkStructurallyEquivalent(NNS1->getAsRecordDecl(),
                                               NNS2->getAsRecordDecl());
  }
  return false;
}

static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const TemplateName &N1,
                                     const TemplateName &N2) {
  if (N1.getKind() != N2.getKind())
    return false;
  switch (N1.getKind()) {
  case TemplateName::Template:
  case TemplateName::UsingTemplate:
    return Context.checkStructurallyEquivalent(N1.getAsTemplateDecl(),
                                               N2.getAsTemplateDecl());

  case TemplateName::OverloadedTemplate: {
    OverloadedTemplateStorage *OS1 = N1.getAsOverloadedTemplate(),
                              *OS2 = N2.getAsOverloadedTemplate();
    OverloadedTemplateStorage::iterator I1 = OS1->begin(), I2 = OS2->begin(),
                                        E1 = OS1->end(), E2 = OS2->end();
    for (; I1 != E1 && I2 != E2; ++I1, ++I2)
      if (!Context.checkStructurallyEquivalent(*I1, *I2))
        return false;
    return I1 == E1 && I2 == E2;
  }

  case TemplateName::AssumedTemplate: {
    AssumedTemplateStorage *TN1 = N1.getAsAssumedTemplateName(),
                           *TN2 = N1.getAsAssumedTemplateName();
    return TN1->getDeclName() == TN2->getDeclName();
  }

  case TemplateName::QualifiedTemplate: {
    QualifiedTemplateName *QN1 = N1.getAsQualifiedTemplateName(),
                          *QN2 = N2.getAsQualifiedTemplateName();
    return Context.checkStructurallyEquivalent(
               QN1->getUnderlyingTemplate().getAsTemplateDecl(),
               QN2->getUnderlyingTemplate().getAsTemplateDecl()) &&
           IsStructurallyEquivalent(Context, QN1->getQualifier(),
                                    QN2->getQualifier());
  }

  case TemplateName::DependentTemplate: {
    DependentTemplateName *DN1 = N1.getAsDependentTemplateName(),
                          *DN2 = N2.getAsDependentTemplateName();
    if (!IsStructurallyEquivalent(Context, DN1->getQualifier(),
                                  DN2->getQualifier()))
      return false;
    if (DN1->isIdentifier() && DN2->isIdentifier())
      return IsStructurallyEquivalent(DN1->getIdentifier(),
                                      DN2->getIdentifier());
    else if (DN1->isOverloadedOperator() && DN2->isOverloadedOperator())
      return DN1->getOperator() == DN2->getOperator();
    return false;
  }

  case TemplateName::SubstTemplateTemplateParm: {
    SubstTemplateTemplateParmStorage *TS1 = N1.getAsSubstTemplateTemplateParm(),
                                     *TS2 = N2.getAsSubstTemplateTemplateParm();
    return IsStructurallyEquivalent(Context, TS1->getParameter(),
                                    TS2->getParameter()) &&
           IsStructurallyEquivalent(Context, TS1->getReplacement(),
                                    TS2->getReplacement());
  }
  case TemplateName::SubstTemplateTemplateParmPack: {
    SubstTemplateTemplateParmPackStorage
        *P1 = N1.getAsSubstTemplateTemplateParmPack(),
        *P2 = N2.getAsSubstTemplateTemplateParmPack();
    return IsStructurallyEquivalent(Context, P1->getArgumentPack(),
                                    P2->getArgumentPack()) &&
           IsStructurallyEquivalent(Context, P1->getParameterPack(),
                                    P2->getParameterPack());
  }
  }
  return false;
}

/// Determine whether two template arguments are equivalent.
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const TemplateArgument &Arg1,
                                     const TemplateArgument &Arg2) {
  if (Arg1.getKind() != Arg2.getKind())
    return false;

  switch (Arg1.getKind()) {
  case TemplateArgument::Null:
    return true;

  case TemplateArgument::Type:
    return Context.checkStructurallyEquivalent(Arg1.getAsType(),
                                               Arg2.getAsType());

  case TemplateArgument::Integral:
    if (!Context.checkStructurallyEquivalent(Arg1.getIntegralType(),
                                             Arg2.getIntegralType()))
      return false;

    // FIXME: Location?
    return llvm::APSInt::isSameValue(Arg1.getAsIntegral(),
                                     Arg2.getAsIntegral());

  case TemplateArgument::Declaration:
    return Context.checkStructurallyEquivalent(Arg1.getAsDecl(),
                                               Arg2.getAsDecl());

  case TemplateArgument::NullPtr:
    return true; // FIXME: Is this correct?

  case TemplateArgument::Template:
    return IsStructurallyEquivalent(Context, Arg1.getAsTemplate(),
                                    Arg2.getAsTemplate());

  case TemplateArgument::TemplateExpansion:
    return IsStructurallyEquivalent(Context,
                                    Arg1.getAsTemplateOrTemplatePattern(),
                                    Arg2.getAsTemplateOrTemplatePattern());

  case TemplateArgument::Expression:
    return IsStructurallyEquivalent(Context, Arg1.getAsExpr(),
                                    Arg2.getAsExpr());

  case TemplateArgument::Pack:
    if (Arg1.pack_size() != Arg2.pack_size())
      return false;

    for (unsigned I = 0, N = Arg1.pack_size(); I != N; ++I)
      if (!IsStructurallyEquivalent(Context, Arg1.pack_begin()[I],
                                    Arg2.pack_begin()[I]))
        return false;

    return true;
  }

  llvm_unreachable("Invalid template argument kind");
}

/// Determine structural equivalence for the common part of array
/// types.
static bool IsArrayStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                          const ArrayType *Array1,
                                          const ArrayType *Array2) {
  if (!Context.checkStructurallyEquivalent(Array1->getElementType(),
                                           Array2->getElementType()))
    return false;

  // FIXME: Locations?
  if (Array1->getSizeModifier() != Array2->getSizeModifier())
    return false;
  if (Array1->getIndexTypeQualifiers() != Array2->getIndexTypeQualifiers())
    return false;

  return true;
}

static bool isDeclAtSameLocation(StructuralEquivalenceContext &Context,
                                 NamedDecl *D1, NamedDecl *D2) {
  auto Loc1 =
      Context.FromCtx.getSourceManager().getPresumedLoc(D1->getLocation());
  auto Loc2 =
      Context.ToCtx.getSourceManager().getPresumedLoc(D2->getLocation());

  if (Loc1.isInvalid() || Loc2.isInvalid())
    return false;

  if (StringRef(Loc1.getFilename()) == StringRef(Loc2.getFilename()) &&
      Loc1.getLine() == Loc2.getLine() &&
      Loc1.getColumn() == Loc2.getColumn())
    return true;

  return false;
}

static bool shouldCheckQualType(StructuralEquivalenceContext &Context,
                                QualType T1, QualType T2) {
  // If they have different qualifier, check and diag the difference.
  if (T1.getQualifiers() != T2.getQualifiers())
    return true;

  if (auto *TD1 = T1.getTypePtr()->getAs<TypedefType>()) {
    if (auto *TD2 = T2.getTypePtr()->getAs<TypedefType>()) {
      // If TD1 and TD2 has different identifier, check and diag the difference.
      if (!IsStructurallyEquivalent(TD1->getDecl()->getIdentifier(),
                                    TD2->getDecl()->getIdentifier()))
        return true;
      // For typedef chains, if the current level requires ABI checking,
      // check the next level. If the current level has the same source location
      // but the next level doesn't require checking anymore, then we don't
      // need to compare the current level as well.
      if (Context.shouldCheckDecls(TD1->getDecl(), TD2->getDecl())) {
        if (isDeclAtSameLocation(Context, TD1->getDecl(), TD2->getDecl()) &&
            !shouldCheckQualType(Context, TD1->desugar(), TD2->desugar()))
          return false;
      } else
        return false;
    }
  }
  else if (auto *TD1 = T1.getTypePtr()->getAs<PointerType>()) {
    if (auto *TD2 = T2.getTypePtr()->getAs<PointerType>()) {
      return shouldCheckQualType(Context, TD1->getPointeeType(),
                                 TD2->getPointeeType());
    }
  }
  else if (auto *TD1 = T1.getTypePtr()->getAs<ObjCObjectPointerType>()) {
    if (auto *TD2 = T2.getTypePtr()->getAs<ObjCObjectPointerType>()) {
      return shouldCheckQualType(Context, TD1->getPointeeType(),
                                 TD2->getPointeeType());
    }
  }
  else if (auto *TD1 = T1.getTypePtr()->getAs<TagType>()) {
    if (auto *TD2 = T2.getTypePtr()->getAs<TagType>()) {
      return Context.shouldCheckDecls(TD1->getDecl(), TD2->getDecl());
    }
  }
  else if (auto *TD1 = T1.getTypePtr()->getAs<ObjCInterfaceType>()) {
    if (auto *TD2 = T2.getTypePtr()->getAs<ObjCInterfaceType>()) {
      return Context.shouldCheckDecls(TD1->getDecl(), TD2->getDecl());
    }
  }
  return true;
}


/// Determine structural equivalence of two types.
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     QualType T1, QualType T2) {
  // FIXME: Locations?
  if (T1.isNull() || T2.isNull())
    return T1.isNull() && T2.isNull();

  // Check to see if we can skip the check.
  if(!shouldCheckQualType(Context, T1, T2))
    return true;

  if (!Context.StrictTypeSpelling) {
    // We aren't being strict about token-to-token equivalence of types,
    // so map down to the canonical type.
    T1 = Context.FromCtx.getCanonicalType(T1);
    T2 = Context.ToCtx.getCanonicalType(T2);
  }

  if (T1.getQualifiers() != T2.getQualifiers())
    return false;

  Type::TypeClass TC = T1->getTypeClass();

  if (T1->getTypeClass() != T2->getTypeClass()) {
    // Compare function types with prototypes vs. without prototypes as if
    // both did not have prototypes.
    if (T1->getTypeClass() == Type::FunctionProto &&
        T2->getTypeClass() == Type::FunctionNoProto)
      TC = Type::FunctionNoProto;
    else if (T1->getTypeClass() == Type::FunctionNoProto &&
             T2->getTypeClass() == Type::FunctionProto)
      TC = Type::FunctionNoProto;
    else
      return false;
  }

  switch (TC) {
  case Type::Builtin:
    // FIXME: Deal with Char_S/Char_U.
    if (cast<BuiltinType>(T1)->getKind() != cast<BuiltinType>(T2)->getKind())
      return false;
    break;

  case Type::Complex:
    if (!Context.checkStructurallyEquivalent(
            cast<ComplexType>(T1)->getElementType(),
            cast<ComplexType>(T2)->getElementType()))
      return false;
    break;

  case Type::Adjusted:
  case Type::Decayed:
    if (!Context.checkStructurallyEquivalent(
            cast<AdjustedType>(T1)->getOriginalType(),
            cast<AdjustedType>(T2)->getOriginalType()))
      return false;
    break;

  case Type::Pointer:
    if (!Context.checkStructurallyEquivalent(
            cast<PointerType>(T1)->getPointeeType(),
            cast<PointerType>(T2)->getPointeeType()))
      return false;
    break;

  case Type::BlockPointer:
    if (!Context.checkStructurallyEquivalent(
            cast<BlockPointerType>(T1)->getPointeeType(),
            cast<BlockPointerType>(T2)->getPointeeType()))
      return false;
    break;

  case Type::LValueReference:
  case Type::RValueReference: {
    const ReferenceType *Ref1 = cast<ReferenceType>(T1);
    const ReferenceType *Ref2 = cast<ReferenceType>(T2);
    if (Ref1->isSpelledAsLValue() != Ref2->isSpelledAsLValue())
      return false;
    if (Ref1->isInnerRef() != Ref2->isInnerRef())
      return false;
    if (!IsStructurallyEquivalent(Context, Ref1->getPointeeTypeAsWritten(),
                                  Ref2->getPointeeTypeAsWritten()))
      return false;
    break;
  }

  case Type::MemberPointer: {
    const MemberPointerType *MemPtr1 = cast<MemberPointerType>(T1);
    const MemberPointerType *MemPtr2 = cast<MemberPointerType>(T2);
    if (!Context.checkStructurallyEquivalent(MemPtr1->getPointeeType(),
                                             MemPtr2->getPointeeType()))
      return false;
    if (!Context.checkStructurallyEquivalent(QualType(MemPtr1->getClass(), 0),
                                             QualType(MemPtr2->getClass(), 0)))
      return false;
    break;
  }

  case Type::ConstantArray: {
    const ConstantArrayType *Array1 = cast<ConstantArrayType>(T1);
    const ConstantArrayType *Array2 = cast<ConstantArrayType>(T2);
    if (!llvm::APInt::isSameValue(Array1->getSize(), Array2->getSize()))
      return false;

    if (!IsArrayStructurallyEquivalent(Context, Array1, Array2))
      return false;
    break;
  }

  case Type::IncompleteArray:
    if (!IsArrayStructurallyEquivalent(Context, cast<ArrayType>(T1),
                                       cast<ArrayType>(T2)))
      return false;
    break;

  case Type::VariableArray: {
    const VariableArrayType *Array1 = cast<VariableArrayType>(T1);
    const VariableArrayType *Array2 = cast<VariableArrayType>(T2);
    if (!IsStructurallyEquivalent(Context, Array1->getSizeExpr(),
                                  Array2->getSizeExpr()))
      return false;

    if (!IsArrayStructurallyEquivalent(Context, Array1, Array2))
      return false;

    break;
  }

  case Type::DependentSizedArray: {
    const DependentSizedArrayType *Array1 = cast<DependentSizedArrayType>(T1);
    const DependentSizedArrayType *Array2 = cast<DependentSizedArrayType>(T2);
    if (!IsStructurallyEquivalent(Context, Array1->getSizeExpr(),
                                  Array2->getSizeExpr()))
      return false;

    if (!IsArrayStructurallyEquivalent(Context, Array1, Array2))
      return false;

    break;
  }

  case Type::DependentAddressSpace: {
    const DependentAddressSpaceType *DepAddressSpace1 =
        cast<DependentAddressSpaceType>(T1);
    const DependentAddressSpaceType *DepAddressSpace2 =
        cast<DependentAddressSpaceType>(T2);
    if (!IsStructurallyEquivalent(Context, DepAddressSpace1->getAddrSpaceExpr(),
                                  DepAddressSpace2->getAddrSpaceExpr()))
      return false;
    if (!Context.checkStructurallyEquivalent(
            DepAddressSpace1->getPointeeType(),
            DepAddressSpace2->getPointeeType()))
      return false;

    break;
  }

  case Type::DependentSizedExtVector: {
    const DependentSizedExtVectorType *Vec1 =
        cast<DependentSizedExtVectorType>(T1);
    const DependentSizedExtVectorType *Vec2 =
        cast<DependentSizedExtVectorType>(T2);
    if (!IsStructurallyEquivalent(Context, Vec1->getSizeExpr(),
                                  Vec2->getSizeExpr()))
      return false;
    if (!Context.checkStructurallyEquivalent(Vec1->getElementType(),
                                             Vec2->getElementType()))
      return false;
    break;
  }

  case Type::DependentVector: {
    const auto *Vec1 = cast<DependentVectorType>(T1);
    const auto *Vec2 = cast<DependentVectorType>(T2);
    if (Vec1->getVectorKind() != Vec2->getVectorKind())
      return false;
    if (!IsStructurallyEquivalent(Context, Vec1->getSizeExpr(),
                                  Vec2->getSizeExpr()))
      return false;
    if (!Context.checkStructurallyEquivalent(Vec1->getElementType(),
                                             Vec2->getElementType()))
      return false;
    break;
  }

  case Type::Vector:
  case Type::ExtVector: {
    const VectorType *Vec1 = cast<VectorType>(T1);
    const VectorType *Vec2 = cast<VectorType>(T2);
    if (!Context.checkStructurallyEquivalent(Vec1->getElementType(),
                                             Vec2->getElementType()))
      return false;
    if (Vec1->getNumElements() != Vec2->getNumElements())
      return false;
    if (Vec1->getVectorKind() != Vec2->getVectorKind())
      return false;
    break;
  }

  case Type::ConstantMatrix:
  case Type::DependentSizedMatrix: {
    const ConstantMatrixType *M1 = cast<ConstantMatrixType>(T1);
    const ConstantMatrixType *M2 = cast<ConstantMatrixType>(T2);
    if (M1->getNumRows() != M2->getNumRows() ||
        M1->getNumColumns() != M2->getNumColumns())
      return false;

    if (!Context.checkStructurallyEquivalent(M1->getElementType(),
                                             M2->getElementType()))
      return false;

    break;
  }

  case Type::FunctionProto: {
    const FunctionProtoType *Proto1 = cast<FunctionProtoType>(T1);
    const FunctionProtoType *Proto2 = cast<FunctionProtoType>(T2);
    if (Proto1->getNumParams() != Proto2->getNumParams())
      return false;
    for (unsigned I = 0, N = Proto1->getNumParams(); I != N; ++I) {
      if (!Context.checkStructurallyEquivalent(Proto1->getParamType(I),
                                               Proto2->getParamType(I)))
        return false;
    }
    if (Proto1->isVariadic() != Proto2->isVariadic())
      return false;
    if (Proto1->getExceptionSpecType() != Proto2->getExceptionSpecType())
      return false;
    if (Proto1->getExceptionSpecType() == EST_Dynamic) {
      if (Proto1->getNumExceptions() != Proto2->getNumExceptions())
        return false;
      for (unsigned I = 0, N = Proto1->getNumExceptions(); I != N; ++I) {
        if (!Context.checkStructurallyEquivalent(Proto1->getExceptionType(I),
                                                 Proto2->getExceptionType(I)))
          return false;
      }
    } else if (isComputedNoexcept(Proto1->getExceptionSpecType())) {
      if (!IsStructurallyEquivalent(Context, Proto1->getNoexceptExpr(),
                                    Proto2->getNoexceptExpr()))
        return false;
    }
    if (Proto1->getMethodQuals() != Proto2->getMethodQuals())
      return false;

    // Fall through to check the bits common with FunctionNoProtoType.
    LLVM_FALLTHROUGH;
  }

  case Type::FunctionNoProto: {
    const FunctionType *Function1 = cast<FunctionType>(T1);
    const FunctionType *Function2 = cast<FunctionType>(T2);
    if (!Context.checkStructurallyEquivalent(Function1->getReturnType(),
                                             Function2->getReturnType()))
      return false;
    if (Function1->getExtInfo() != Function2->getExtInfo())
      return false;
    break;
  }

  case Type::UnresolvedUsing:
    if (!Context.checkStructurallyEquivalent(
            cast<UnresolvedUsingType>(T1)->getDecl(),
            cast<UnresolvedUsingType>(T2)->getDecl()))
      return false;

    break;

  case Type::Attributed:
    if (!Context.checkStructurallyEquivalent(
            cast<AttributedType>(T1)->getModifiedType(),
            cast<AttributedType>(T2)->getModifiedType()))
      return false;
    if (!Context.checkStructurallyEquivalent(
            cast<AttributedType>(T1)->getEquivalentType(),
            cast<AttributedType>(T2)->getEquivalentType()))
      return false;
    break;

  case Type::Paren:
    if (!Context.checkStructurallyEquivalent(
            cast<ParenType>(T1)->getInnerType(),
            cast<ParenType>(T2)->getInnerType()))
      return false;
    break;

  case Type::MacroQualified:
    if (!IsStructurallyEquivalent(
            Context, cast<MacroQualifiedType>(T1)->getUnderlyingType(),
            cast<MacroQualifiedType>(T2)->getUnderlyingType()))
      return false;
    break;

  case Type::Typedef:
    if (!Context.checkStructurallyEquivalent(cast<TypedefType>(T1)->getDecl(),
                                             cast<TypedefType>(T2)->getDecl()))
      return false;
    break;

  case Type::TypeOfExpr:
    if (!IsStructurallyEquivalent(
            Context, cast<TypeOfExprType>(T1)->getUnderlyingExpr(),
            cast<TypeOfExprType>(T2)->getUnderlyingExpr()))
      return false;
    break;

  case Type::TypeOf:
    if (!Context.checkStructurallyEquivalent(
            cast<TypeOfType>(T1)->getUnmodifiedType(),
            cast<TypeOfType>(T2)->getUnmodifiedType()))
      return false;
    break;

  case Type::UnaryTransform:
    if (!Context.checkStructurallyEquivalent(
            cast<UnaryTransformType>(T1)->getUnderlyingType(),
            cast<UnaryTransformType>(T1)->getUnderlyingType()))
      return false;
    break;

  case Type::Decltype:
    if (!IsStructurallyEquivalent(Context,
                                  cast<DecltypeType>(T1)->getUnderlyingExpr(),
                                  cast<DecltypeType>(T2)->getUnderlyingExpr()))
      return false;
    break;

  case Type::Auto:
    if (!Context.checkStructurallyEquivalent(
            cast<AutoType>(T1)->getDeducedType(),
            cast<AutoType>(T2)->getDeducedType()))
      return false;
    break;

  case Type::DeducedTemplateSpecialization: {
    auto *DT1 = cast<DeducedTemplateSpecializationType>(T1);
    auto *DT2 = cast<DeducedTemplateSpecializationType>(T2);
    if (!IsStructurallyEquivalent(Context, DT1->getTemplateName(),
                                  DT2->getTemplateName()))
      return false;
    if (!Context.checkStructurallyEquivalent(DT1->getDeducedType(),
                                             DT2->getDeducedType()))
      return false;
    break;
  }

  case Type::Record:
  case Type::Enum:
    if (!Context.checkStructurallyEquivalent(cast<TagType>(T1)->getDecl(),
                                             cast<TagType>(T2)->getDecl()))
      return false;
    break;

  case Type::TemplateTypeParm: {
    const TemplateTypeParmType *Parm1 = cast<TemplateTypeParmType>(T1);
    const TemplateTypeParmType *Parm2 = cast<TemplateTypeParmType>(T2);
    if (Parm1->getDepth() != Parm2->getDepth())
      return false;
    if (Parm1->getIndex() != Parm2->getIndex())
      return false;
    if (Parm1->isParameterPack() != Parm2->isParameterPack())
      return false;

    // Names of template type parameters are never significant.
    break;
  }

  case Type::SubstTemplateTypeParm: {
    const SubstTemplateTypeParmType *Subst1 =
        cast<SubstTemplateTypeParmType>(T1);
    const SubstTemplateTypeParmType *Subst2 =
        cast<SubstTemplateTypeParmType>(T2);
    if (!Context.checkStructurallyEquivalent(Subst1->getAssociatedDecl(),
                                             Subst2->getAssociatedDecl()))
      return false;
    if (!Context.checkStructurallyEquivalent(Subst1->getReplacementType(),
                                             Subst2->getReplacementType()))
      return false;
    break;
  }

  case Type::SubstTemplateTypeParmPack: {
    const SubstTemplateTypeParmPackType *Subst1 =
        cast<SubstTemplateTypeParmPackType>(T1);
    const SubstTemplateTypeParmPackType *Subst2 =
        cast<SubstTemplateTypeParmPackType>(T2);
    if (!Context.checkStructurallyEquivalent(Subst1->getAssociatedDecl(),
                                             Subst2->getAssociatedDecl()))
      return false;
    if (!IsStructurallyEquivalent(Context, Subst1->getArgumentPack(),
                                  Subst2->getArgumentPack()))
      return false;
    break;
  }
  case Type::TemplateSpecialization: {
    const TemplateSpecializationType *Spec1 =
        cast<TemplateSpecializationType>(T1);
    const TemplateSpecializationType *Spec2 =
        cast<TemplateSpecializationType>(T2);
    if (!IsStructurallyEquivalent(Context, Spec1->getTemplateName(),
                                  Spec2->getTemplateName()))
      return false;
    if (Spec1->template_arguments().size() != Spec2->template_arguments().size())
      return false;
    for (unsigned I = 0, N = Spec1->template_arguments().size(); I != N; ++I) {
      if (!IsStructurallyEquivalent(Context, Spec1->template_arguments()[I],
                                    Spec2->template_arguments()[I]))
        return false;
    }
    break;
  }

  case Type::Elaborated: {
    const ElaboratedType *Elab1 = cast<ElaboratedType>(T1);
    const ElaboratedType *Elab2 = cast<ElaboratedType>(T2);
    // CHECKME: what if a keyword is ETK_None or ETK_typename ?
    if (Elab1->getKeyword() != Elab2->getKeyword())
      return false;

    if (!Elab1->getQualifier() != !Elab2->getQualifier())
      return false;

    // The qualifier is optional.
    if (Elab1->getQualifier() && Elab2->getQualifier()) {
      if (!IsStructurallyEquivalent(Context, Elab1->getQualifier(),
                                    Elab2->getQualifier()))
        return false;
    }
    if (!Context.checkStructurallyEquivalent(Elab1->getNamedType(),
                                             Elab2->getNamedType()))
      return false;
    break;
  }

  case Type::InjectedClassName: {
    const InjectedClassNameType *Inj1 = cast<InjectedClassNameType>(T1);
    const InjectedClassNameType *Inj2 = cast<InjectedClassNameType>(T2);
    if (!Context.checkStructurallyEquivalent(
            Inj1->getInjectedSpecializationType(),
            Inj2->getInjectedSpecializationType()))
      return false;
    break;
  }

  case Type::DependentName: {
    const DependentNameType *Typename1 = cast<DependentNameType>(T1);
    const DependentNameType *Typename2 = cast<DependentNameType>(T2);
    if (!IsStructurallyEquivalent(Context, Typename1->getQualifier(),
                                  Typename2->getQualifier()))
      return false;
    if (!IsStructurallyEquivalent(Typename1->getIdentifier(),
                                  Typename2->getIdentifier()))
      return false;

    break;
  }

  case Type::DependentTemplateSpecialization: {
    const DependentTemplateSpecializationType *Spec1 =
        cast<DependentTemplateSpecializationType>(T1);
    const DependentTemplateSpecializationType *Spec2 =
        cast<DependentTemplateSpecializationType>(T2);
    if (!IsStructurallyEquivalent(Context, Spec1->getQualifier(),
                                  Spec2->getQualifier()))
      return false;
    if (!IsStructurallyEquivalent(Spec1->getIdentifier(),
                                  Spec2->getIdentifier()))
      return false;
    if (Spec1->template_arguments().size() != Spec2->template_arguments().size())
      return false;
    for (unsigned I = 0, N = Spec1->template_arguments().size(); I != N; ++I) {
      if (!IsStructurallyEquivalent(Context, Spec1->template_arguments()[I],
                                    Spec2->template_arguments()[I]))
        return false;
    }
    break;
  }

  case Type::PackExpansion:
    if (!IsStructurallyEquivalent(Context,
                                  cast<PackExpansionType>(T1)->getPattern(),
                                  cast<PackExpansionType>(T2)->getPattern()))
      return false;
    break;

  case Type::BitInt: {
    const auto *EIT1 = cast<BitIntType>(T1);
    const auto *EIT2 = cast<BitIntType>(T2);
    if (EIT1->isUnsigned() != EIT2->isUnsigned())
      return false;
    if (EIT1->getNumBits() != EIT2->getNumBits())
      return false;
    break;
  }

  case Type::DependentBitInt: {
    const auto *EIT1 = cast<DependentBitIntType>(T1);
    const auto *EIT2 = cast<DependentBitIntType>(T2);
    if (EIT1->isUnsigned() != EIT2->isUnsigned())
      return false;
    // FIXME: Check NumBitsExpr.
    break;
  }

  case Type::ObjCInterface: {
    const ObjCInterfaceType *Iface1 = cast<ObjCInterfaceType>(T1);
    const ObjCInterfaceType *Iface2 = cast<ObjCInterfaceType>(T2);
    if (!Context.checkStructurallyEquivalent(Iface1->getDecl(),
                                             Iface2->getDecl()))
      return false;
    break;
  }

  case Type::ObjCTypeParam: {
    const ObjCTypeParamType *Obj1 = cast<ObjCTypeParamType>(T1);
    const ObjCTypeParamType *Obj2 = cast<ObjCTypeParamType>(T2);
    if (!Context.checkStructurallyEquivalent(Obj1->getDecl(), Obj2->getDecl()))
      return false;

    if (Obj1->getNumProtocols() != Obj2->getNumProtocols())
      return false;
    for (unsigned I = 0, N = Obj1->getNumProtocols(); I != N; ++I) {
      if (!Context.checkStructurallyEquivalent(Obj1->getProtocol(I),
                                               Obj2->getProtocol(I)))
        return false;
    }
    break;
  }
  case Type::ObjCObject: {
    const ObjCObjectType *Obj1 = cast<ObjCObjectType>(T1);
    const ObjCObjectType *Obj2 = cast<ObjCObjectType>(T2);
    if (!Context.checkStructurallyEquivalent(Obj1->getBaseType(),
                                             Obj2->getBaseType()))
      return false;
    if (Obj1->getNumProtocols() != Obj2->getNumProtocols())
      return false;
    for (unsigned I = 0, N = Obj1->getNumProtocols(); I != N; ++I) {
      if (!Context.checkStructurallyEquivalent(Obj1->getProtocol(I),
                                               Obj2->getProtocol(I)))
        return false;
    }
    break;
  }

  case Type::ObjCObjectPointer: {
    const ObjCObjectPointerType *Ptr1 = cast<ObjCObjectPointerType>(T1);
    const ObjCObjectPointerType *Ptr2 = cast<ObjCObjectPointerType>(T2);
    if (!Context.checkStructurallyEquivalent(Ptr1->getPointeeType(),
                                             Ptr2->getPointeeType()))
      return false;
    break;
  }

  case Type::Atomic: {
    if (!Context.checkStructurallyEquivalent(
            cast<AtomicType>(T1)->getValueType(),
            cast<AtomicType>(T2)->getValueType()))
      return false;
    break;
  }

  case Type::Pipe: {
    if (!Context.checkStructurallyEquivalent(
            cast<PipeType>(T1)->getElementType(),
            cast<PipeType>(T2)->getElementType()))
      return false;
    break;
  }

  } // end switch

  return true;
}

/// Determine structural equivalence of two fields.
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const FieldDecl *Field1,
                                     const FieldDecl *Field2) {
  // For anonymous structs/unions, match up the anonymous struct/union type
  // declarations directly, so that we don't go off searching for anonymous
  // types
  if (Field1->isAnonymousStructOrUnion() &&
      Field2->isAnonymousStructOrUnion()) {
    RecordDecl *D1 = Field1->getType()->castAs<RecordType>()->getDecl();
    RecordDecl *D2 = Field2->getType()->castAs<RecordType>()->getDecl();
    return Context.checkStructurallyEquivalent(D1, D2);
  }

  // Check for equivalent field names.
  IdentifierInfo *Name1 = Field1->getIdentifier();
  IdentifierInfo *Name2 = Field2->getIdentifier();
  if (!IsStructurallyEquivalent(Context, Name1, Field1, Name2, Field2)) {
    Context.Diag1(Field1->getLocation(),
                  TAPI_INTERNAL::diag::note_api_field_name)
        << Field1->getDeclName();
    Context.Diag2(Field2->getLocation(),
                  TAPI_INTERNAL::diag::note_api_field_name)
        << Field2->getDeclName();
    return false;
  }

  if (!Context.checkStructurallyEquivalent(Field1->getType(),
                                           Field2->getType())) {
    Context.Diag1(Field1->getLocation(),
                  TAPI_INTERNAL::diag::note_api_field_type)
        << Field1->getDeclName() << Field1->getType();
    Context.Diag2(Field2->getLocation(),
                  TAPI_INTERNAL::diag::note_api_field_type)
        << Field2->getDeclName() << Field2->getType();
    return false;
  }

  if (Field1->isBitField() != Field2->isBitField()) {
    Context.Diag1(Field1->getLocation(),
                  TAPI_INTERNAL::diag::note_api_field_bitfield)
        << Field1->getDeclName() << Field1->isBitField();
    Context.Diag2(Field2->getLocation(),
                  TAPI_INTERNAL::diag::note_api_field_bitfield)
        << Field2->getDeclName() << Field2->isBitField();
    return false;
  }

  if (Field1->isBitField()) {
    // Make sure that the bit-fields are the same length.
    unsigned Bits1 = Field1->getBitWidthValue(Context.FromCtx);
    unsigned Bits2 = Field2->getBitWidthValue(Context.ToCtx);

    if (Bits1 != Bits2) {
      Context.Diag1(Field1->getLocation(),
                    TAPI_INTERNAL::diag::note_api_field_bitfield_width)
          << Field1->getDeclName() << Bits1;
      Context.Diag2(Field2->getLocation(),
                    TAPI_INTERNAL::diag::note_api_field_bitfield_width)
          << Field2->getDeclName() << Bits2;
      return false;
    }
  }

  return true;
}

/// Determine structural equivalence of two variables.
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const VarDecl *D1, const VarDecl *D2) {
  if (!Context.checkStructurallyEquivalent(D1->getType(), D2->getType())) {
    Context.Diag1(D1->getTypeSpecStartLoc(), TAPI_INTERNAL::diag::note_api_var)
        << D1->getDeclName() << D1->getType();
    Context.Diag2(D2->getTypeSpecStartLoc(), TAPI_INTERNAL::diag::note_api_var)
        << D2->getDeclName() << D2->getType();
    return false;
  }

  return true;
}

/// Determine structural equivalence of two parameters.
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const ParmVarDecl *D1,
                                     const ParmVarDecl *D2) {
  if (!Context.checkStructurallyEquivalent(D1->getType(), D2->getType())) {
    Context.Diag1(D1->getTypeSpecStartLoc(),
                  TAPI_INTERNAL::diag::note_api_param)
        << D1->getType();
    Context.Diag2(D2->getTypeSpecStartLoc(),
                  TAPI_INTERNAL::diag::note_api_param)
        << D2->getType();
    return false;
  }
  return true;
}

/// Determine structural equivalence of two function signatures.
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const FunctionDecl *D1,
                                     const FunctionDecl *D2) {
  // Check return type.
  if (!Context.checkStructurallyEquivalent(D1->getReturnType(),
                                           D2->getReturnType())) {
    Context.Diag1(D1->getTypeSpecStartLoc(),
                  TAPI_INTERNAL::diag::note_api_return)
        << D1->getReturnType();
    Context.Diag2(D2->getTypeSpecStartLoc(),
                  TAPI_INTERNAL::diag::note_api_return)
        << D2->getReturnType();
    return false;
  }

  // Check function parameters.
  auto Param2 = D2->param_begin(), Param2End = D2->param_end();
  for (auto Param1 = D1->param_begin(), Param1End = D1->param_end();
       Param1 != Param1End; ++Param1, ++Param2) {
    if (Param2 == Param2End) {
      Context.Diag1((*Param1)->getTypeSpecStartLoc(),
                    TAPI_INTERNAL::diag::note_api_param)
          << (*Param1)->getType();
      Context.Diag2(D2->getLocation(),
                    TAPI_INTERNAL::diag::note_api_missing_param);
      return false;
    }

    if (!Context.checkStructurallyEquivalent(*Param1, *Param2))
      return false;
  }

  for (; Param2 != Param2End; ++Param2) {
    Context.Diag2((*Param2)->getTypeSpecStartLoc(),
                  TAPI_INTERNAL::diag::note_api_param)
        << (*Param2)->getType();
    Context.Diag1(D1->getLocation(),
                  TAPI_INTERNAL::diag::note_api_missing_param);
    return false;
  }

  return true;
}

static bool checkObjCPropertyListEquivalent(
    StructuralEquivalenceContext &Context, const ObjCContainerDecl *D1,
    DeclarationName N1, const ObjCContainerDecl *D2, DeclarationName N2,
    ObjCContainerDecl::prop_range R1, ObjCContainerDecl::prop_range R2) {
  // Create lookup map for properties and class properties
  llvm::StringMap<const ObjCPropertyDecl *> D1P;
  llvm::StringMap<const ObjCPropertyDecl *> D1CP;

  // Add properties in base to lookup map.
  for (const auto *I : R1) {
    if (I->isClassProperty())
      D1CP.insert({I->getNameAsString(), I});
    else
      D1P.insert({I->getNameAsString(), I});
  }

  for (const auto *P2 : R2) {
    if (P2->isUnavailable())
      continue;
    // Lookup the property with the same name in base.
    auto P1 = P2->isClassProperty() ? D1CP.find(P2->getNameAsString())
                                    : D1P.find(P2->getNameAsString());
    auto D1End = P2->isClassProperty() ? D1CP.end() : D1P.end();
    if (P1 == D1End) {
      if (Context.shouldCheckMissingAPIs()) {
        Context.Diag2(P2->getLocation(), TAPI_INTERNAL::diag::note_api_property)
            << N2 << P2->getDeclName();
        Context.Diag1(D1->getBeginLoc(),
                      TAPI_INTERNAL::diag::note_api_missing_property)
            << N1;
        return false;
      } else
        continue;
    }

    if (!Context.checkStructurallyEquivalent(P1->second, P2))
      return false;
  }

  return true;
}

template <typename RangeType>
static bool checkObjCMethodListEquivalent(StructuralEquivalenceContext &Context,
                                          const ObjCContainerDecl *D1,
                                          DeclarationName N1,
                                          const ObjCContainerDecl *D2,
                                          DeclarationName N2,
                                          RangeType R1, RangeType R2) {
  llvm::StringMap<const ObjCMethodDecl *> D1P;

  for (const auto *I : R1) {
    if (I->isPropertyAccessor())
      continue;
    D1P.insert({I->getNameAsString(), I});
  }

  for (const auto *P2 : R2) {
    if (P2->isPropertyAccessor() || P2->isUnavailable())
      continue;
    auto P1 = D1P.find(P2->getNameAsString());

    if (P1 == D1P.end()) {
      if (Context.shouldCheckMissingAPIs()) {
        Context.Diag2(P2->getLocation(), TAPI_INTERNAL::diag::note_api_method)
            << N2 << P2->getDeclName();
        Context.Diag1(D1->getBeginLoc(),
                      TAPI_INTERNAL::diag::note_api_missing_method)
            << N1;
        return false;
      } else
        continue;
    }

    if (!Context.checkStructurallyEquivalent(P1->second, P2))
      return false;
  }

  return true;
}

static std::pair<const SourceLocation *, const ObjCProtocolDecl *>
findProtocolWithName(StringRef name, const ObjCProtocolList &PL) {
  auto loc = PL.loc_begin();
  for (auto &protocol : PL) {
    if (protocol->getName() == name)
      return {loc, protocol};
    ++loc;
  }
  return {nullptr, nullptr};
}

static bool checkObjCProtocolListEquivalent(
    StructuralEquivalenceContext &Context, const ObjCContainerDecl *D1,
    DeclarationName N1, const ObjCContainerDecl *D2, DeclarationName N2,
    const ObjCProtocolList &PL1, const ObjCProtocolList &PL2) {
  llvm::StringSet<> names;
  for (auto &I : PL1)
    names.insert(I->getNameAsString());
  for (const auto *I : PL2)
    names.insert(I->getNameAsString());
  for (auto &PN : names) {
    auto P1 = findProtocolWithName(PN.first(), PL1);
    auto P2 = findProtocolWithName(PN.first(), PL2);
    if (!P1.second) {
      Context.Diag2(*P2.first, TAPI_INTERNAL::diag::note_api_protocol)
          << N2 << P2.second->getDeclName();
      Context.Diag1(D1->getBeginLoc(),
                    TAPI_INTERNAL::diag::note_api_missing_protocol)
          << N1;
      return false;
    }
    if (!P2.second) {
      Context.Diag1(*P1.first, TAPI_INTERNAL::diag::note_api_protocol)
          << N1 << P1.second->getDeclName();
      Context.Diag2(D2->getBeginLoc(),
                    TAPI_INTERNAL::diag::note_api_missing_protocol)
          << N2;
      return false;
    }
    if (!Context.checkStructurallyEquivalent(P1.second, P2.second))
      return false;
  }

  return true;
}

static bool checkObjCContainerEquivalent(StructuralEquivalenceContext &Context,
                                         const ObjCContainerDecl *D1,
                                         DeclarationName N1,
                                         const ObjCContainerDecl *D2,
                                         DeclarationName N2) {
  if (!checkObjCMethodListEquivalent(Context, D1, N1, D2, N2,
                                     D1->instance_methods(),
                                     D2->instance_methods()))
    return false;

  if (!checkObjCMethodListEquivalent(Context, D1, N1, D2, N2,
                                     D1->class_methods(), D2->class_methods()))
    return false;

  if (!checkObjCPropertyListEquivalent(Context, D1, N1, D2, N2,
                                       D1->properties(), D2->properties()))
    return false;

  return true;
}

/// Determine structural equivalence of two Objective-C interfaces.
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const ObjCInterfaceDecl *D1,
                                     const ObjCInterfaceDecl *D2) {
  // Check exception list.
  if ((Context.IgnoreObjCClasses.count(D1->getIdentifier()->getName())) ||
      (Context.BridgeObjCClasses.lookup(D1->getIdentifier()->getName()) ==
       D2->getIdentifier()->getName()))
    return true;

  if (Context.isKnowEqual(D1, D2))
    return true;

  // Compare class name.
  if (!IsStructurallyEquivalent(Context, D1->getIdentifier(), D1,
                                D2->getIdentifier(), D2))
    return false;

  // Cannot further compare implicit or forward declared types. We have to
  // assume they are equal.
  if (!D1->hasDefinition() || !D2->hasDefinition())
    return true;

  // Compare super class.
  auto S1 = D1->getSuperClass();
  auto S2 = D2->getSuperClass();
  if (S1 && !S2) {
    Context.Diag1(D1->getSuperClassLoc(),
                  TAPI_INTERNAL::diag::note_api_interface_superclass)
        << D1->getDeclName() << S1->getDeclName();
    Context.Diag2(D2->getLocation(),
                  TAPI_INTERNAL::diag::note_api_missing_superclass)
        << D2->getDeclName();
    return false;
  }
  if (!S1 && S2) {
    Context.Diag2(D2->getSuperClassLoc(),
                  TAPI_INTERNAL::diag::note_api_interface_superclass)
        << D2->getDeclName() << S2->getDeclName();
    Context.Diag1(D1->getLocation(),
                  TAPI_INTERNAL::diag::note_api_missing_superclass)
        << D1->getDeclName();
    return false;
  }
  if (S1 && S2 && (!Context.checkStructurallyEquivalent(S1, S2))) {
    if (S1->getName() == S2->getName()) {
      // If the name is same, emit a friendly diagnostic.
      Context.Diag1(D1->getSuperClassLoc(),
                    TAPI_INTERNAL::diag::note_api_inconsistent_superclass)
          << D1->getDeclName() << S1->getDeclName();
    } else {
      Context.Diag1(D1->getSuperClassLoc(),
                    TAPI_INTERNAL::diag::note_api_interface_superclass)
          << D1->getDeclName() << S1->getDeclName();
      Context.Diag2(D2->getSuperClassLoc(),
                    TAPI_INTERNAL::diag::note_api_interface_superclass)
          << D2->getDeclName() << S2->getDeclName();
    }
    return false;
  }

  if (!checkObjCProtocolListEquivalent(
          Context, D1, D1->getDeclName(), D2, D2->getDeclName(),
          D1->getReferencedProtocols(), D2->getReferencedProtocols()))
    return false;

  if (!checkObjCContainerEquivalent(Context, D1, D1->getDeclName(), D2,
                                    D2->getDeclName())) {
    return false;
  }

  // check ivar. ivar must come in order.
  auto I2 = D2->ivar_begin(), I2End = D2->ivar_end();
  for (auto I1 = D1->ivar_begin(), I1End = D1->ivar_end(); I1 != I1End;
       ++I1, ++I2) {
    if (I2 == I2End) {
      Context.Diag1(I1->getLocation(), TAPI_INTERNAL::diag::note_api_ivar)
          << D1->getDeclName() << I1->getDeclName();
      Context.Diag2(D2->getBeginLoc(),
                    TAPI_INTERNAL::diag::note_api_missing_ivar)
          << D2->getDeclName();
      return false;
    }

    if (!Context.checkStructurallyEquivalent(*I1, *I2)) {
      Context.Diag1(I1->getLocation(), TAPI_INTERNAL::diag::note_api_ivar)
          << D1->getDeclName() << I1->getDeclName();
      Context.Diag2(I2->getLocation(), TAPI_INTERNAL::diag::note_api_ivar)
          << D2->getDeclName() << I2->getDeclName();
      return false;
    }
  }
  for (; I2 != I2End; ++I2) {
    Context.Diag2(I2->getLocation(), TAPI_INTERNAL::diag::note_api_ivar)
        << D2->getDeclName() << I2->getDeclName();
    Context.Diag1(D1->getBeginLoc(), TAPI_INTERNAL::diag::note_api_missing_ivar)
        << D1->getDeclName();
    return false;
  }

  Context.addEqualDecl(D1, D2);
  return true;
}

static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const ObjCMethodDecl *D1,
                                     const ObjCMethodDecl *D2) {
  // Check return type.
  if (!Context.checkStructurallyEquivalent(D1->getReturnType(),
                                           D2->getReturnType())) {
    Context.Diag1(D1->getReturnTypeSourceRange().getBegin(),
                  TAPI_INTERNAL::diag::note_api_return)
        << D1->getReturnType();
    Context.Diag2(D2->getReturnTypeSourceRange().getBegin(),
                  TAPI_INTERNAL::diag::note_api_return)
        << D2->getReturnType();
    return false;
  }

  // Check function parameters.
  auto Param2 = D2->param_begin(), Param2End = D2->param_end();
  for (auto Param1 = D1->param_begin(), Param1End = D1->param_end();
       Param1 != Param1End; ++Param1, ++Param2) {
    if (Param2 == Param2End) {
      Context.Diag1((*Param1)->getTypeSpecStartLoc(),
                    TAPI_INTERNAL::diag::note_api_param)
          << (*Param1)->getType();
      Context.Diag2(D2->getLocation(),
                    TAPI_INTERNAL::diag::note_api_missing_param);
      return false;
    }

    if (!Context.checkStructurallyEquivalent(*Param1, *Param2))
      return false;
  }

  for (; Param2 != Param2End; ++Param2) {
    Context.Diag2((*Param2)->getTypeSpecStartLoc(),
                  TAPI_INTERNAL::diag::note_api_param)
        << (*Param2)->getType();
    Context.Diag1(D1->getLocation(),
                  TAPI_INTERNAL::diag::note_api_missing_param);
    return false;
  }

  return true;
}

static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const ObjCPropertyDecl *D1,
                                     const ObjCPropertyDecl *D2) {
  // Check property type.
  if (!Context.checkStructurallyEquivalent(D1->getType(), D2->getType())) {
    Context.Diag1(D1->getTypeSourceInfo()->getTypeLoc().getBeginLoc(),
                  TAPI_INTERNAL::diag::note_api_property_type)
        << D1->getDeclName() << D1->getType();
    Context.Diag2(D2->getTypeSourceInfo()->getTypeLoc().getBeginLoc(),
                  TAPI_INTERNAL::diag::note_api_property_type)
        << D2->getDeclName() << D2->getType();
    return false;
  }

  // Check attributes.
  if (D1->getPropertyAttributes() == D2->getPropertyAttributes())
    return true;

  // If the attributes are not equal, we allow more restrictive attributes
  // from variants.
  auto diffBits = (unsigned)D1->getPropertyAttributes() ^
                  (unsigned)D2->getPropertyAttributes();
  if (D1->isAtomic() && !D2->isAtomic()) {
    diffBits &= ~ObjCPropertyAttribute::kind_atomic;
    diffBits &= ~ObjCPropertyAttribute::kind_nonatomic;
  }
  if (!D1->isReadOnly() && D2->isReadOnly()) {
    diffBits &= ~ObjCPropertyAttribute::kind_readonly;
    diffBits &= ~ObjCPropertyAttribute::kind_readwrite;
  }

  if (diffBits) {
    auto S1 = D1->getLParenLoc() == SourceLocation()
                  ? D1->getSourceRange().getBegin()
                  : D1->getLParenLoc();
    auto S2 = D2->getLParenLoc() == SourceLocation()
                  ? D2->getSourceRange().getBegin()
                  : D2->getLParenLoc();
    Context.Diag1(S1, TAPI_INTERNAL::diag::note_api_property_attribute)
        << D1->getDeclName();
    Context.Diag2(S2, TAPI_INTERNAL::diag::note_api_property_attribute)
        << D2->getDeclName();
    return false;
  }
  return true;
}

static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const ObjCIvarDecl *D1,
                                     const ObjCIvarDecl *D2) {
  if (!IsStructurallyEquivalent(Context, D1->getIdentifier(), D1,
                                D2->getIdentifier(), D2))
    return false;

  return Context.checkStructurallyEquivalent(
      (const FieldDecl *)D1,
      (const FieldDecl *)D2); // Check FieldDecl;
}

static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const ObjCProtocolDecl *D1,
                                     const ObjCProtocolDecl *D2) {
  if (!checkObjCContainerEquivalent(Context, D1, D1->getDeclName(), D2,
                                    D2->getDeclName()))
    return false;

  // Cannot further compare forward declared types. We have to assume they are
  // equal.
  if (!D1->hasDefinition() || !D2->hasDefinition())
    return true;

  if (!checkObjCProtocolListEquivalent(
          Context, D1, D1->getDeclName(), D2, D2->getDeclName(),
          D1->getReferencedProtocols(), D2->getReferencedProtocols()))
    return false;

  return true;
}

static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const ObjCCategoryDecl *D1,
                                     const ObjCCategoryDecl *D2) {
  // When it is an extension (declaration is empty), use the base interface
  // name for diagnostics.
  auto N1 = D1->IsClassExtension() ? D1->getClassInterface()->getDeclName()
                                   : D1->getDeclName();
  auto N2 = D2->IsClassExtension() ? D2->getClassInterface()->getDeclName()
                                   : D2->getDeclName();
  if (!checkObjCContainerEquivalent(Context, D1, N1, D2, N2))
    return false;

  if (!checkObjCProtocolListEquivalent(Context, D1, N1, D2, N2,
                                       D1->getReferencedProtocols(),
                                       D2->getReferencedProtocols()))
    return false;

  return true;
}

/// Determine structural equivalence of two records.
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const RecordDecl *D1,
                                     const RecordDecl *D2) {
  if (D1->isUnion() != D2->isUnion())
    return false;

  if (D1->isAnonymousStructOrUnion() && D2->isAnonymousStructOrUnion()) {
    // If both anonymous structs/unions are in a record context, make sure
    // they occur in the same location in the context records.
    if (std::optional<unsigned> Index1 =
            StructuralEquivalenceContext::findUntaggedStructOrUnionIndex(D1)) {
      if (std::optional<unsigned> Index2 =
              StructuralEquivalenceContext::findUntaggedStructOrUnionIndex(
                  D2)) {
        if (*Index1 != *Index2)
          return false;
      }
    }
  }

  // If both declarations are class template specializations, we know
  // the ODR applies, so check the template and template arguments.
  const auto *Spec1 = dyn_cast<ClassTemplateSpecializationDecl>(D1);
  const auto *Spec2 = dyn_cast<ClassTemplateSpecializationDecl>(D2);
  if (Spec1 && Spec2) {
    // Check that the specialized templates are the same.
    if (!Context.checkStructurallyEquivalent(Spec1->getSpecializedTemplate(),
                                             Spec2->getSpecializedTemplate()))
      return false;

    // Check that the template arguments are the same.
    if (Spec1->getTemplateArgs().size() != Spec2->getTemplateArgs().size())
      return false;

    for (unsigned I = 0, N = Spec1->getTemplateArgs().size(); I != N; ++I)
      if (!IsStructurallyEquivalent(Context, Spec1->getTemplateArgs().get(I),
                                    Spec2->getTemplateArgs().get(I)))
        return false;
  }
  // If one is a class template specialization and the other is not, these
  // structures are different.
  else if (Spec1 || Spec2)
    return false;

  // Compare the definitions of these two records. If either or both are
  // incomplete, we assume that they are equivalent.
  D1 = D1->getDefinition();
  D2 = D2->getDefinition();
  if (!D1 || !D2)
    return true;

  if (auto *D1CXX = dyn_cast<CXXRecordDecl>(D1)) {
    if (auto *D2CXX = dyn_cast<CXXRecordDecl>(D2)) {
      if (D1CXX->getNumBases() != D2CXX->getNumBases()) {
        Context.Diag1(D1->getBeginLoc(),
                      TAPI_INTERNAL::diag::note_api_base_class_num)
            << D1->getDeclName() << D1CXX->getNumBases();
        Context.Diag2(D2->getBeginLoc(),
                      TAPI_INTERNAL::diag::note_api_base_class_num)
            << D2->getDeclName() << D2CXX->getNumBases();
        return false;
      }

      // Check the base classes.
      for (auto Base1 = D1CXX->bases_begin(), BaseEnd1 = D1CXX->bases_end(),
                Base2 = D2CXX->bases_begin();
           Base1 != BaseEnd1; ++Base1, ++Base2) {
        // Check virtual vs. non-virtual inheritance mismatch.
        if (Base1->isVirtual() != Base2->isVirtual()) {
          Context.Diag1(Base1->getBeginLoc(),
                        TAPI_INTERNAL::diag::note_api_base_class_virtual)
              << Base1->getType() << Base1->isVirtual();
          Context.Diag2(Base2->getBeginLoc(),
                        TAPI_INTERNAL::diag::note_api_base_class_virtual)
              << Base2->getType() << Base2->isVirtual();
          return false;
        }
        // Check AccessSpecifier
        if (Base1->getAccessSpecifier() != Base2->getAccessSpecifier()) {
          Context.Diag1(Base1->getBeginLoc(),
                        TAPI_INTERNAL::diag::note_api_base_class_access)
              << Base1->getType() << Base1->getAccessSpecifier();
          Context.Diag2(Base2->getBeginLoc(),
                        TAPI_INTERNAL::diag::note_api_base_class_access)
              << Base2->getType() << Base2->getAccessSpecifier();
          return false;
        }
        if (!Context.checkStructurallyEquivalent(Base1->getType(),
                                                 Base2->getType())) {
          Context.Diag1(Base1->getBeginLoc(),
                        TAPI_INTERNAL::diag::note_api_base_class)
              << D1->getDeclName() << Base1->getType();
          Context.Diag2(Base2->getBeginLoc(),
                        TAPI_INTERNAL::diag::note_api_base_class)
              << D2->getDeclName() << Base2->getType();
          return false;
        }
      }
    }
  }

  // Check the fields for consistency.
  // For diagnostics to print the anonymous struct/union correctly,
  // use QualType for the diagengine instead of DeclName.
  QualType TypedName1(D1->getTypeForDecl(), 0);
  QualType TypedName2(D1->getTypeForDecl(), 0);

  RecordDecl::field_iterator Field2 = D2->field_begin(),
                             Field2End = D2->field_end();
  for (RecordDecl::field_iterator Field1 = D1->field_begin(),
                                  Field1End = D1->field_end();
       Field1 != Field1End; ++Field1, ++Field2) {
    if (Field2 == Field2End) {
      Context.Diag1(Field1->getLocation(), TAPI_INTERNAL::diag::note_api_field)
          << TypedName1 << Field1->getDeclName();
      Context.Diag2(D2->getLocation(),
                    TAPI_INTERNAL::diag::note_api_field_missing)
          << TypedName2;
      return false;
    }

    if (!Context.checkStructurallyEquivalent(*Field1, *Field2))
      return false;
  }

  if (Field2 != Field2End) {
    Context.Diag1(D1->getLocation(),
                  TAPI_INTERNAL::diag::note_api_field_missing)
        << TypedName1;
    Context.Diag2(Field2->getLocation(), TAPI_INTERNAL::diag::note_api_field)
        << TypedName2 << Field2->getDeclName();
    return false;
  }

  return true;
}

/// Determine structural equivalence of two enums.
static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const EnumDecl *D1, const EnumDecl *D2) {
  // Check type.
  if (!Context.checkStructurallyEquivalent(D1->getIntegerType(),
                                           D2->getIntegerType())) {
    auto Loc1 = D1->getIntegerTypeRange().getBegin();
    if (Loc1.isInvalid())
      Loc1 = D1->getLocation();
    Context.Diag1(Loc1, TAPI_INTERNAL::diag::note_api_enum_type)
        << D1->getIntegerType();
    auto Loc2 = D2->getIntegerTypeRange().getBegin();
    if (Loc2.isInvalid())
      Loc2 = D2->getLocation();
    Context.Diag2(Loc2, TAPI_INTERNAL::diag::note_api_enum_type)
        << D2->getIntegerType();
    return false;
  }

  std::map<StringRef, EnumConstantDecl *> D2EnumConstantDecls;
  for (auto *EC2 : D2->enumerators())
    D2EnumConstantDecls.emplace(EC2->getName(), EC2);

  for (auto *EC1 : D1->enumerators()) {
    auto it = D2EnumConstantDecls.find(EC1->getName());
    // Ignore missing enumerators.
    if (it == D2EnumConstantDecls.end())
      continue;

    auto *EC2 = it->second;
    if (!Context.checkStructurallyEquivalent(EC1, EC2))
      return false;
  }

  return true;
}

static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
    const EnumConstantDecl *D1, const EnumConstantDecl *D2) {
  auto Val1 = D1->getInitVal();
  auto Val2 = D2->getInitVal();
  if (!llvm::APSInt::isSameValue(Val1, Val2)) {
    Context.Diag1(D1->getLocation(), TAPI_INTERNAL::diag::note_api_enumerator)
        << D1->getDeclName() << toString(D1->getInitVal(), 10);
    Context.Diag2(D2->getLocation(), TAPI_INTERNAL::diag::note_api_enumerator)
        << D2->getDeclName() << toString(D2->getInitVal(), 10);
    return false;
  }
  return true;
}

static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     TemplateParameterList *Params1,
                                     TemplateParameterList *Params2) {
  if (Params1->size() != Params2->size()) {
    /*if (Context.Complain) {
      Context.Diag2(Params2->getTemplateLoc(),
                    diag::err_odr_different_num_template_parameters)
          << Params1->size() << Params2->size();
      Context.Diag1(Params1->getTemplateLoc(),
                    diag::note_odr_template_parameter_list);
    }*/
    return false;
  }

  for (unsigned I = 0, N = Params1->size(); I != N; ++I) {
    if (Params1->getParam(I)->getKind() != Params2->getParam(I)->getKind()) {
      /*if (Context.Complain) {
        Context.Diag2(Params2->getParam(I)->getLocation(),
                      diag::err_odr_different_template_parameter_kind);
        Context.Diag1(Params1->getParam(I)->getLocation(),
                      diag::note_odr_template_parameter_here);
      }*/
      return false;
    }

    if (!Context.checkStructurallyEquivalent(Params1->getParam(I),
                                             Params2->getParam(I))) {

      return false;
    }
  }

  return true;
}

static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const TemplateTypeParmDecl *D1,
                                     const TemplateTypeParmDecl *D2) {
  if (D1->isParameterPack() != D2->isParameterPack()) {
    /*if (Context.Complain) {
      Context.Diag2(D2->getLocation(), diag::err_odr_parameter_pack_non_pack)
          << D2->isParameterPack();
      Context.Diag1(D1->getLocation(), diag::note_odr_parameter_pack_non_pack)
          << D1->isParameterPack();
    }*/
    return false;
  }

  return true;
}

static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const NonTypeTemplateParmDecl *D1,
                                     const NonTypeTemplateParmDecl *D2) {
  if (D1->isParameterPack() != D2->isParameterPack()) {
    /*if (Context.Complain) {
      Context.Diag2(D2->getLocation(), diag::err_odr_parameter_pack_non_pack)
          << D2->isParameterPack();
      Context.Diag1(D1->getLocation(), diag::note_odr_parameter_pack_non_pack)
          << D1->isParameterPack();
    }*/
    return false;
  }

  // Check types.
  if (!Context.checkStructurallyEquivalent(D1->getType(), D2->getType())) {
    /*if (Context.Complain) {
      Context.Diag2(D2->getLocation(),
                    diag::err_odr_non_type_parameter_type_inconsistent)
          << D2->getType() << D1->getType();
      Context.Diag1(D1->getLocation(), diag::note_odr_value_here)
          << D1->getType();
    }*/
    return false;
  }

  return true;
}

static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const TemplateTemplateParmDecl *D1,
                                     const TemplateTemplateParmDecl *D2) {
  if (D1->isParameterPack() != D2->isParameterPack()) {
    /*if (Context.Complain) {
      Context.Diag2(D2->getLocation(), diag::err_odr_parameter_pack_non_pack)
          << D2->isParameterPack();
      Context.Diag1(D1->getLocation(), diag::note_odr_parameter_pack_non_pack)
          << D1->isParameterPack();
    }*/
    return false;
  }

  // Check template parameter lists.
  return IsStructurallyEquivalent(Context, D1->getTemplateParameters(),
                                  D2->getTemplateParameters());
}

static bool
IsTemplateDeclCommonStructurallyEquivalent(StructuralEquivalenceContext &Ctx,
                                           const TemplateDecl *D1,
                                           const TemplateDecl *D2) {
  if (!IsStructurallyEquivalent(Ctx, D1->getIdentifier(), D1,
                                D2->getIdentifier(), D2))
    return false;
  if (!D1->getIdentifier()) // Special name
    if (D1->getNameAsString() != D2->getNameAsString())
      return false;
  return IsStructurallyEquivalent(Ctx, D1->getTemplateParameters(),
                                  D2->getTemplateParameters());
}

static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const ClassTemplateDecl *D1,
                                     const ClassTemplateDecl *D2) {
  // Check template parameters.
  if (!IsTemplateDeclCommonStructurallyEquivalent(Context, D1, D2))
    return false;

  // Check the templated declaration.
  return Context.checkStructurallyEquivalent(D1->getTemplatedDecl(),
                                             D2->getTemplatedDecl());
}

static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const FunctionTemplateDecl *D1,
                                     const FunctionTemplateDecl *D2) {
  // Check template parameters.
  if (!IsTemplateDeclCommonStructurallyEquivalent(Context, D1, D2))
    return false;

  // Check the templated declaration.
  return Context.checkStructurallyEquivalent(D1->getTemplatedDecl()->getType(),
                                             D2->getTemplatedDecl()->getType());
}

static bool IsStructurallyEquivalent(StructuralEquivalenceContext &Context,
                                     const Decl *D1, const Decl *D2) {
  return Context.checkStructurallyEquivalent(D1, D2);
}

} // namespace

TAPI_NAMESPACE_INTERNAL_BEGIN

/// Determine structural equivalence of two declarations by looking at the
/// cache.
bool StructuralEquivalenceContext::checkCacheForEquivalence(const Decl *D1,
                                                            const Decl *D2) {
  // Check whether we already know that these two declarations are not
  // structurally equivalent.
  auto Lookup =
      NonEquivalentDecls.find({D1->getCanonicalDecl(), D2->getCanonicalDecl()});
  // If we found a known diff, we append the diag locations for better
  // diagnostics.
  if (Lookup != NonEquivalentDecls.end()) {
    StoredDiagnostics.extend(Lookup->second);
    return false;
  }

  // Otherwise, add the decl to the list to be compared.
  TentativeComparsions->insert({D1, D2});

  return true;
}

TAPIDiagBuilder StructuralEquivalenceContext::Diag1(SourceLocation Loc,
                                                    unsigned DiagID) {
  return TAPIDiagBuilder(&FromDiag, Loc, DiagID, StoredDiagnostics.D1);
}

TAPIDiagBuilder StructuralEquivalenceContext::Diag2(SourceLocation Loc,
                                                    unsigned DiagID) {
  return TAPIDiagBuilder(&ToDiag, Loc, DiagID, StoredDiagnostics.D2);
}

void StructuralEquivalenceContext::resetContext() {
  DiagnosticsEngine *Current =
      StoredDiagnostics.D1.empty()
          ? nullptr
          : StoredDiagnostics.D1.back().getDiagEngine();
  while (!StoredDiagnostics.D1.empty()) {
    StoredDiagnostics.D1.back().emitDiag();
    StoredDiagnostics.D1.pop_back();
  }
  if (Current && !StoredDiagnostics.D2.empty())
    StoredDiagnostics.D2.back().getDiagEngine()->notePriorDiagnosticFrom(
        *Current);
  while (!StoredDiagnostics.D2.empty()) {
    StoredDiagnostics.D2.back().emitDiag();
    StoredDiagnostics.D2.pop_back();
  }

  StoredDiagnostics.clear();
  ComparsionStacks.clear();
}

void StructuralEquivalenceContext::pushContext() {
  ComparsionStacks.emplace_back();
  TentativeComparsions = &ComparsionStacks.back();
}

void StructuralEquivalenceContext::popContext() {
  ComparsionStacks.pop_back();
  if (ComparsionStacks.empty())
    TentativeComparsions = nullptr;
  else
    TentativeComparsions = &ComparsionStacks.back();
}

bool StructuralEquivalenceContext::shouldCheckDecls(const Decl *D1,
                                                    const Decl *D2) {
  if (CheckExternalHeaders)
    return true;

  auto shouldCheckDecl = [](const Decl *D, DiagnosticsEngine &DE,
                            FrontendContext *ctx) {
    // Locate the decl. If the location is invalid or the search failed,
    // return true and check the decl.
    auto loc = D->getLocation();
    if (loc.isInvalid())
      return true;
    auto fileLoc = DE.getSourceManager().getFileLoc(loc);
    FileID id = DE.getSourceManager().getFileID(fileLoc);
    if (id.isInvalid())
      return true;
    const auto *file = DE.getSourceManager().getFileEntryForID(id);
    if (!file)
      return true;

    // If the location is not found, return false and skip.
    return ctx->findAndRecordFile(file).has_value();
  };

  return shouldCheckDecl(D1, FromDiag, FromFrontendCtx) ||
         shouldCheckDecl(D2, ToDiag, ToFrontendCtx);
}

void StructuralEquivalenceContext::addEqualDecl(const Decl *D1,
                                                const Decl *D2) {
  EqualDecls.insert({D1, D2});
}

bool StructuralEquivalenceContext::isKnowEqual(const Decl *D1,
                                               const Decl *D2) const {
  return EqualDecls.count({D1, D2});
}

std::optional<unsigned>
StructuralEquivalenceContext::findUntaggedStructOrUnionIndex(
    const RecordDecl *Anon) {
  ASTContext &Context = Anon->getASTContext();
  QualType AnonTy = Context.getRecordType(Anon);

  auto *Owner = dyn_cast<RecordDecl>(Anon->getDeclContext());
  if (!Owner)
    return std::nullopt;

  unsigned Index = 0;
  for (const auto *D : Owner->noload_decls()) {
    const auto *F = dyn_cast<FieldDecl>(D);
    if (!F)
      continue;

    if (F->isAnonymousStructOrUnion()) {
      if (Context.hasSameType(F->getType(), AnonTy))
        break;
      ++Index;
      continue;
    }

    // If the field looks like this:
    // struct { ... } A;
    QualType FieldType = F->getType();
    if (const auto *RecType = dyn_cast<RecordType>(FieldType)) {
      auto *RecDecl = RecType->getDecl();
      if (RecDecl->getDeclContext() == Owner && !RecDecl->getIdentifier()) {
        if (Context.hasSameType(FieldType, AnonTy))
          break;
        ++Index;
        continue;
      }
    }
  }

  return Index;
}

bool StructuralEquivalenceContext::diagnoseStructurallyEquivalent(
    const Decl *D1, const Decl *D2) {
  if (!checkStructurallyEquivalent(D1, D2)) {
    // Push the hints for target.
    Diag1(clang::SourceLocation(), TAPI_INTERNAL::diag::note_api_target_note)
        << FromFrontendCtx->target.getTriple();
    Diag2(clang::SourceLocation(), TAPI_INTERNAL::diag::note_api_target_note)
        << ToFrontendCtx->target.getTriple();
    if (auto *ED = dyn_cast<EnumDecl>(D1)) {
      // For EnumDecl, complain about the enum type not the enum value.
      Diag1(D1->getLocation(), TAPI_INTERNAL::diag::warn_api_inconsistent)
          << FromCtx.getTypeDeclType(ED);
    } else if (auto *OCD = dyn_cast<ObjCCategoryDecl>(D1)) {
      auto name = OCD->getDeclName();
      // For extension, rather than print empty name, use interface name.
      if (name.isEmpty())
        name = OCD->getClassInterface()->getDeclName();
      Diag1(OCD->getLocation(), TAPI_INTERNAL::diag::warn_api_inconsistent)
          << name;
    } else {
      Diag1(D1->getLocation(), TAPI_INTERNAL::diag::warn_api_inconsistent)
          << cast<clang::NamedDecl>(D1)->getDeclName();
    }
  }

  resetContext();
  return true;
}

bool StructuralEquivalenceContext::checkStructurallyEquivalent(const Decl *D1,
                                                               const Decl *D2) {
  // First check if we disable cascading diagnostics and the decl pair needs
  // to be compared in the future. If so, just return true.
  if (!EmitCascadingDiags && !ComparsionStacks.empty() &&
      DeclsToCompare.count({D1, D2}))
    return true;

  // Only allow finite number of stack frame, otherwise, it will infinite
  // loop.
  bool finalize = true;
  if (ComparsionStacks.size() < DiagnosticDepth)
    pushContext();
  else
    finalize = false;

  if (!checkCacheForEquivalence(D1, D2))
    return false;

  if (!finalize)
    return true;

  // Otherwise, do comparsion.
  bool Equivalent = true;
  for (unsigned index = 0; index < TentativeComparsions->size(); ++index) {
    auto declPair = (*TentativeComparsions)[index];
    if (!isDeclEquivalent(declPair.first, declPair.second)) {
      Equivalent = false;
      break;
    }
  }

  // Record NonEquivalentDecls. And also record the DiagTrace for the
  // Decl.
  if (!Equivalent) {
    NonEquivalentDecls.try_emplace(
        {D1->getCanonicalDecl(), D2->getCanonicalDecl()}, StoredDiagnostics);
  }

  // pop the context.
  popContext();

  return Equivalent;
}

bool StructuralEquivalenceContext::checkStructurallyEquivalent(QualType T1,
                                                               QualType T2) {
  return IsStructurallyEquivalent(*this, T1, T2);
}

bool StructuralEquivalenceContext::isDeclEquivalent(const Decl *D1,
                                                    const Decl *D2) {
  if (!shouldCheckDecls(D1, D2)) {
    if (auto *ND1 = dyn_cast<NamedDecl>(D1)) {
      if (auto *ND2 = dyn_cast<NamedDecl>(D2)) {
        return IsStructurallyEquivalent(*this, ND1->getIdentifier(), ND1,
                                        ND2->getIdentifier(), ND2);
      }
    }
  }

  bool Equivalent = true;

  // FIXME: Switch on all declaration kinds. For now, we're just going to
  // check the obvious ones.
  if (auto *P1 = dyn_cast<ParmVarDecl>(D1)) {
    if (auto *P2 = dyn_cast<ParmVarDecl>(D2)) {
      if (!IsStructurallyEquivalent(*this, P1, P2))
        Equivalent = false;
    } else {
      // Kind mismatch.
      Equivalent = false;
    }
  } else if (auto *VarDecl1 = dyn_cast<VarDecl>(D1)) {
    if (auto *VarDecl2 = dyn_cast<VarDecl>(D2)) {
      if (!IsStructurallyEquivalent(*this, VarDecl1, VarDecl2))
        Equivalent = false;
    } else {
      // Decl mismatch.
      Equivalent = false;
    }
  } else if (auto *Func1 = dyn_cast<FunctionDecl>(D1)) {
    if (auto *Func2 = dyn_cast<FunctionDecl>(D2)) {
      if (!IsStructurallyEquivalent(*this, Func1, Func2))
        Equivalent = false;
    } else {
      // Record/non-record mismatch.
      Equivalent = false;
    }
  } else if (auto *Interface1 = dyn_cast<ObjCInterfaceDecl>(D1)) {
    if (auto *Interface2 = dyn_cast<ObjCInterfaceDecl>(D2)) {
      if (!IsStructurallyEquivalent(*this, Interface1, Interface2))
        Equivalent = false;
    } else {
      // Record/non-record mismatch.
      Equivalent = false;
    }
  } else if (auto *Category1 = dyn_cast<ObjCCategoryDecl>(D1)) {
    if (auto *Category2 = dyn_cast<ObjCCategoryDecl>(D2)) {
      if (!IsStructurallyEquivalent(*this, Category1, Category2))
        Equivalent = false;
    } else {
      // Record/non-record mismatch.
      Equivalent = false;
    }
  } else if (auto *Protocol1 = dyn_cast<ObjCProtocolDecl>(D1)) {
    if (auto *Protocol2 = dyn_cast<ObjCProtocolDecl>(D2)) {
      if (!IsStructurallyEquivalent(*this, Protocol1, Protocol2))
        Equivalent = false;
    } else {
      // Record/non-record mismatch.
      Equivalent = false;
    }
  } else if (auto *Record1 = dyn_cast<RecordDecl>(D1)) {
    if (auto *Record2 = dyn_cast<RecordDecl>(D2)) {
      // Check for equivalent structure names.
      IdentifierInfo *Name1 = Record1->getIdentifier();
      if (!Name1 && Record1->getTypedefNameForAnonDecl())
        Name1 = Record1->getTypedefNameForAnonDecl()->getIdentifier();
      IdentifierInfo *Name2 = Record2->getIdentifier();
      if (!Name2 && Record2->getTypedefNameForAnonDecl())
        Name2 = Record2->getTypedefNameForAnonDecl()->getIdentifier();
      if (!IsStructurallyEquivalent(*this, Name1, Record1, Name2, Record2) ||
          !IsStructurallyEquivalent(*this, Record1, Record2))
        Equivalent = false;
    } else {
      // Record/non-record mismatch.
      Equivalent = false;
    }
  } else if (auto *Enum1 = dyn_cast<EnumDecl>(D1)) {
    if (auto *Enum2 = dyn_cast<EnumDecl>(D2)) {
      // Check for equivalent enum names.
      IdentifierInfo *Name1 = Enum1->getIdentifier();
      if (!Name1 && Enum1->getTypedefNameForAnonDecl())
        Name1 = Enum1->getTypedefNameForAnonDecl()->getIdentifier();
      IdentifierInfo *Name2 = Enum2->getIdentifier();
      if (!Name2 && Enum2->getTypedefNameForAnonDecl())
        Name2 = Enum2->getTypedefNameForAnonDecl()->getIdentifier();
      if (!IsStructurallyEquivalent(*this, Name1, Enum1, Name2, Enum2) ||
          !IsStructurallyEquivalent(*this, Enum1, Enum2))
        Equivalent = false;
    } else {
      // Enum/non-enum mismatch
      Equivalent = false;
    }
  } else if (auto *Enum1 = dyn_cast<EnumConstantDecl>(D1)) {
    if (auto *Enum2 = dyn_cast<EnumConstantDecl>(D2)) {
      // Check for equivalent enum names.
      IdentifierInfo *Name1 = Enum1->getIdentifier();
      IdentifierInfo *Name2 = Enum2->getIdentifier();
      if (!IsStructurallyEquivalent(*this, Name1, Enum1, Name2, Enum2) ||
          !IsStructurallyEquivalent(*this, Enum1, Enum2))
        Equivalent = false;
    } else {
      // Enum/non-enum mismatch
      Equivalent = false;
    }
  } else if (auto *Typedef1 = dyn_cast<TypedefNameDecl>(D1)) {
    if (auto *Typedef2 = dyn_cast<TypedefNameDecl>(D2)) {
      if (!IsStructurallyEquivalent(*this, Typedef1, Typedef2))
        Equivalent = false;
    } else {
      // Typedef/non-typedef mismatch.
      Equivalent = false;
    }

  } else if (auto *ClassTemplate1 = dyn_cast<ClassTemplateDecl>(D1)) {
    if (auto *ClassTemplate2 = dyn_cast<ClassTemplateDecl>(D2)) {
      if (!IsStructurallyEquivalent(*this, ClassTemplate1, ClassTemplate2))
        Equivalent = false;
    } else {
      // Class template/non-class-template mismatch.
      Equivalent = false;
    }

  } else if (auto *FunctionTemplate1 = dyn_cast<FunctionTemplateDecl>(D1)) {
    if (auto *FunctionTemplate2 = dyn_cast<FunctionTemplateDecl>(D2)) {
      if (!IsStructurallyEquivalent(*this, FunctionTemplate1,
                                    FunctionTemplate2))
        Equivalent = false;
    } else {
      // Class template/non-class-template mismatch.
      Equivalent = false;
    }

  } else if (auto *TTP1 = dyn_cast<TemplateTypeParmDecl>(D1)) {
    if (auto *TTP2 = dyn_cast<TemplateTypeParmDecl>(D2)) {
      if (!IsStructurallyEquivalent(*this, TTP1, TTP2))
        Equivalent = false;
    } else {
      // Kind mismatch.
      Equivalent = false;
    }
  } else if (auto *NTTP1 = dyn_cast<NonTypeTemplateParmDecl>(D1)) {
    if (auto *NTTP2 = dyn_cast<NonTypeTemplateParmDecl>(D2)) {
      if (!IsStructurallyEquivalent(*this, NTTP1, NTTP2))
        Equivalent = false;
    } else {
      // Kind mismatch.
      Equivalent = false;
    }

  } else if (auto *TTP1 = dyn_cast<TemplateTemplateParmDecl>(D1)) {
    if (auto *TTP2 = dyn_cast<TemplateTemplateParmDecl>(D2)) {
      if (!IsStructurallyEquivalent(*this, TTP1, TTP2))
        Equivalent = false;
    } else {
      // Kind mismatch.
      Equivalent = false;
    }
  } else if (auto *M1 = dyn_cast<ObjCMethodDecl>(D1)) {
    if (auto *M2 = dyn_cast<ObjCMethodDecl>(D2)) {
      if (!IsStructurallyEquivalent(*this, M1, M2))
        Equivalent = false;
    } else {
      // Kind mismatch.
      Equivalent = false;
    }
  } else if (auto *P1 = dyn_cast<ObjCPropertyDecl>(D1)) {
    if (auto *P2 = dyn_cast<ObjCPropertyDecl>(D2)) {
      if (!IsStructurallyEquivalent(*this, P1, P2))
        Equivalent = false;
    } else {
      // Kind mismatch.
      Equivalent = false;
    }
  } else if (auto *I1 = dyn_cast<ObjCIvarDecl>(D1)) {
    if (auto *I2 = dyn_cast<ObjCIvarDecl>(D2)) {
      if (!IsStructurallyEquivalent(*this, I1, I2))
        Equivalent = false;
    } else {
      // Kind mismatch.
      Equivalent = false;
    }
  } else if (auto *F1 = dyn_cast<FieldDecl>(D1)) {
    if (auto *F2 = dyn_cast<FieldDecl>(D2)) {
      if (!IsStructurallyEquivalent(*this, F1, F2))
        Equivalent = false;
    } else {
      // Kind mismatch.
      Equivalent = false;
    }
  }

  return Equivalent;
}

StructuralEquivalenceContext::StructuralEquivalenceContext(
    const APIVerifierConfiguration &config, DiagnosticsEngine &diag,
    FrontendContext *FromFrontendCtx, FrontendContext *ToFrontendCtx,
    bool StrictTypeSpelling, APIVerifierDiagStyle Style,
    bool CheckExternalHeaders, bool DiagMissingAPI, bool EmitCascadingDiags)
    : FromCtx(*FromFrontendCtx->ast), ToCtx(*FromFrontendCtx->ast),
      StrictTypeSpelling(StrictTypeSpelling), FromFrontendCtx(FromFrontendCtx),
      ToFrontendCtx(ToFrontendCtx),
      FromDiag(new ForwardingDiagnosticConsumer(*diag.getClient())),
      ToDiag(new ForwardingDiagnosticConsumer(*diag.getClient())),
      CheckExternalHeaders(CheckExternalHeaders),
      CheckMissingAPIs(DiagMissingAPI), EmitCascadingDiags(EmitCascadingDiags) {
  FromDiag.setSourceManager(FromFrontendCtx->sourceMgr.get());
  FromDiag.SetArgToStringFn(&FormatASTNodeDiagnosticArgument, &FromCtx);
  ToDiag.setSourceManager(ToFrontendCtx->sourceMgr.get());
  ToDiag.SetArgToStringFn(&FormatASTNodeDiagnosticArgument, &ToCtx);
  if (!DiagMissingAPI) {
    FromDiag.ignoreDiagnotic(diag::warn_api_incomplete);
    ToDiag.ignoreDiagnotic(diag::warn_api_incomplete);
  }
  switch (Style) {
  case APIVerifierDiagStyle::Silent:
    FromDiag.ignoreDiagnotic(diag::warn_api_inconsistent);
    ToDiag.ignoreDiagnotic(diag::warn_api_inconsistent);
    break;
  case APIVerifierDiagStyle::Warning:
    break;
  case APIVerifierDiagStyle::Error:
    FromDiag.setDiagnosticAsError(diag::warn_api_inconsistent);
    ToDiag.setDiagnosticAsError(diag::warn_api_inconsistent);
    break;
  }
  for (auto &cls : config.IgnoreObjCClasses)
    IgnoreObjCClasses.insert(cls);
  for (auto &clsPair : config.BridgeObjCClasses)
    BridgeObjCClasses.insert(clsPair);
}

LocTrace::~LocTrace() {
  // TODO: check all locations are consumed in diagnostics.
  // assert(L1.empty() && "All location are consumed");
  // assert(L2.empty() && "All location are consumed");
}

void LocTrace::extend(const LocTrace &other) {
  L1.insert(L1.end(), other.L1.begin(), other.L1.end());
  L2.insert(L2.end(), other.L2.begin(), other.L2.end());
}

void DiagTrace::extend(const DiagTrace &other) {
  D1.insert(D1.end(), other.D1.begin(), other.D1.end());
  D2.insert(D2.end(), other.D2.begin(), other.D2.end());
}

TAPI_NAMESPACE_INTERNAL_END
