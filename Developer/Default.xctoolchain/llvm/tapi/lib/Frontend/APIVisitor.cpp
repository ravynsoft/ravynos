//===- lib/Frontend/APIVisitor.cpp - TAPI API Visitor -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the TAPI API Visitor.
///
//===----------------------------------------------------------------------===//

#include "APIVisitor.h"
#include "tapi/Core/AvailabilityInfo.h"
#include "tapi/Defines.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/AST/VTableBuilder.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Index/USRGeneration.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TextAPI/Symbol.h"

using namespace llvm;
using namespace TAPI_INTERNAL;

namespace {
enum class LinkageType {
  ExternalLinkage,
  LinkOnceODRLinkage,
  WeakODRLinkage,
  PrivateLinkage,
};
}

namespace clang {

// \brief Check if the interface itself or any of its super classes have an
// exception attribute.
//
// We need to export an additonal symbol ("OBJC_EHTYPE_$CLASS_NAME") if any of
// the classes have the exception attribute.
static bool hasObjCExceptionAttribute(const ObjCInterfaceDecl *decl) {
  for (; decl != nullptr; decl = decl->getSuperClass())
    if (decl->hasAttr<ObjCExceptionAttr>())
      return true;

  return false;
}

static bool isInlined(const ASTContext &context, const FunctionDecl *func) {
  bool hasInlineAttribute = false;
  bool noCXXAttr = (!context.getLangOpts().CPlusPlus &&
                    !context.getTargetInfo().getCXXABI().isMicrosoft() &&
                    !func->hasAttr<DLLExportAttr>());

  // Check all redeclarations to find the inline attribute / keyword.
  for (const auto *decl : func->redecls()) {
    if (!decl->isInlined())
      continue;
    hasInlineAttribute = true;
    if (!(noCXXAttr || decl->hasAttr<GNUInlineAttr>()))
      continue;
    if (decl->doesThisDeclarationHaveABody() &&
        decl->isInlineDefinitionExternallyVisible())
      return false;
  }

  if (!hasInlineAttribute)
    return false;

  return true;
}

// \brief Check if the NamedDecl is exported or not.
//
// Exported NamedDecl needs to have externally visibiliy linkage and
// default visibility from LinkageComputer.
static bool isExported(const NamedDecl *decl) {
  auto li = decl->getLinkageAndVisibility();
  return isExternallyVisible(li.getLinkage()) &&
         (li.getVisibility() == DefaultVisibility);
}

static bool hasVTable(ASTContext &context, const CXXRecordDecl *decl) {
  // Check if we need need to emit the vtable symbol. Only dynamic classes need
  // vtables.
  if (!decl->hasDefinition() || !decl->isDynamicClass())
    return false;

  assert(decl->isExternallyVisible() && "should be externally visible");
  assert(decl->isCompleteDefinition() && "only work on complete definitions");

  const auto *keyFunction = context.getCurrentKeyFunction(decl);
  // If this class has a key function, then we have a vtable (might be internal
  // only).
  if (keyFunction) {
    switch (keyFunction->getTemplateSpecializationKind()) {
    case TSK_Undeclared:
    case TSK_ExplicitSpecialization:
    case TSK_ImplicitInstantiation:
    case TSK_ExplicitInstantiationDefinition:
      return true;
    case TSK_ExplicitInstantiationDeclaration:
      llvm_unreachable("Should not have been asked to emit this");
    }
  } else if (decl->isAbstract())
    // If the class is abstract and it doesn't have a key function, it is a
    // 'pure' virtual class. It doesn't need a VTable.
    return false;

  switch (decl->getTemplateSpecializationKind()) {
  case TSK_Undeclared:
  case TSK_ExplicitSpecialization:
  case TSK_ImplicitInstantiation:
    return false;

  case TSK_ExplicitInstantiationDeclaration:
  case TSK_ExplicitInstantiationDefinition:
    return true;
  }

  llvm_unreachable("Invalid TemplateSpecializationKind!");
}

static LinkageType getVTableLinkage(ASTContext &context,
                                    const CXXRecordDecl *decl) {
  assert((decl->hasDefinition() && decl->isDynamicClass()) && "no vtable");
  assert(decl->isExternallyVisible() && "should be externally visible");

  if (decl->getVisibility() == HiddenVisibility)
    return LinkageType::PrivateLinkage;

  const CXXMethodDecl *keyFunction = context.getCurrentKeyFunction(decl);
  if (keyFunction) {
    // If this class has a key function, use that to determine the
    // linkage of the vtable.
    switch (keyFunction->getTemplateSpecializationKind()) {
    case TSK_Undeclared:
    case TSK_ExplicitSpecialization:
      if (isInlined(context, keyFunction))
        return LinkageType::LinkOnceODRLinkage;
      return LinkageType::ExternalLinkage;
    case TSK_ImplicitInstantiation:
      llvm_unreachable("no external vtable for implicit instantiation");
    case TSK_ExplicitInstantiationDefinition:
      return LinkageType::WeakODRLinkage;
    case TSK_ExplicitInstantiationDeclaration:
      llvm_unreachable("Should not have been asked to emit this");
    }
  }

  switch (decl->getTemplateSpecializationKind()) {
  case TSK_Undeclared:
  case TSK_ExplicitSpecialization:
  case TSK_ImplicitInstantiation:
    return LinkageType::LinkOnceODRLinkage;
  case TSK_ExplicitInstantiationDeclaration:
  case TSK_ExplicitInstantiationDefinition:
    return LinkageType::WeakODRLinkage;
  }

  llvm_unreachable("Invalid TemplateSpecializationKind!");
}

static bool isRTTIWeakDef(ASTContext &context, const CXXRecordDecl *decl) {
  if (decl->hasAttr<WeakAttr>())
    return true;

  if (decl->isAbstract() && context.getCurrentKeyFunction(decl) == nullptr)
    return true;

  if (decl->isDynamicClass())
    return getVTableLinkage(context, decl) != LinkageType::ExternalLinkage;

  return false;
}

static bool hasRTTI(ASTContext &context, const CXXRecordDecl *decl) {
  if (!context.getLangOpts().RTTI)
    return false;

  if (!decl->hasDefinition())
    return false;

  if (!decl->isDynamicClass())
    return false;

  // Don't emit weak-def RTTI information. We cannot reliably determine if the
  // final binary will have those weak defined RTTI symbols. This depends on the
  // optimization level and if the class has been instantiated and used.
  //
  // Luckily the static linker doesn't need those weak defined RTTI symbols for
  // linking. They are only needed by the runtime linker. That means we can
  // safely drop all of them.
  if (isRTTIWeakDef(context, decl))
    return false;

  return true;
}

static std::optional<std::pair<APIAccess, APILoc>>
getFileAttributesForLoc(FrontendContext &context, SourceLocation loc) {
  // If the loc refers to a macro expansion we need to first get the file
  // location of the expansion.
  auto fileLoc = context.sourceMgr->getFileLoc(loc);
  FileID id = context.sourceMgr->getFileID(fileLoc);
  if (id.isInvalid())
    return std::nullopt;

  const auto *file = context.sourceMgr->getFileEntryForID(id);
  if (!file)
    return std::nullopt;

  auto header = context.findAndRecordFile(file);
  if (!header.has_value())
    return std::nullopt;

  APIAccess access;
  switch (header.value()) {
  case HeaderType::Public:
    access = APIAccess::Public;
    break;
  case HeaderType::Private:
    access = APIAccess::Private;
    break;
  case HeaderType::Project:
    access = APIAccess::Project;
    break;
  }

  APILoc apiLoc = APILoc(loc, context.sourceMgr->getPresumedLoc(loc));
  return std::make_pair(access, apiLoc);
}

APIVisitor::APIVisitor(FrontendContext &frontend)
    : frontend(frontend), context(frontend.compiler->getASTContext()),
      sourceManager(context.getSourceManager()),
      mc(clang::ItaniumMangleContext::create(context,
                                             context.getDiagnostics())),
      dataLayout(context.getTargetInfo().getDataLayoutString()) {}

void APIVisitor::HandleTranslationUnit(ASTContext &context) {
  if (context.getDiagnostics().hasErrorOccurred())
    return;

  auto *decl = context.getTranslationUnitDecl();
  TraverseDecl(decl);
}

std::optional<std::pair<APIAccess, APILoc>>
APIVisitor::getFileAttributesForDecl(const NamedDecl *decl) const {
  auto loc = decl->getLocation();
  if (loc.isInvalid())
    return std::nullopt;

  return getFileAttributesForLoc(frontend, loc);
}

std::string APIVisitor::getMangledName(const NamedDecl *decl) const {
  SmallString<256> name;
  if (mc->shouldMangleDeclName(decl)) {
    raw_svector_ostream nameStream(name);
    mc->mangleName(decl, nameStream);
  } else
    name += decl->getNameAsString();

  return getBackendMangledName(name);
}

std::string APIVisitor::getBackendMangledName(Twine name) const {
  SmallString<256> finalName;
  Mangler::getNameWithPrefix(finalName, name, DataLayout(dataLayout));
  return finalName.str().str();
}

std::string
APIVisitor::getMangledCXXVTableName(const CXXRecordDecl *decl) const {
  SmallString<256> name;
  raw_svector_ostream nameStream(name);
  mc->mangleCXXVTable(decl, nameStream);

  return getBackendMangledName(name);
}

std::string APIVisitor::getMangledCXXRTTI(const CXXRecordDecl *decl) const {
  SmallString<256> name;
  raw_svector_ostream nameStream(name);
  mc->mangleCXXRTTI(QualType(decl->getTypeForDecl(), 0), nameStream);

  return getBackendMangledName(name);
}

std::string APIVisitor::getMangledCXXRTTIName(const CXXRecordDecl *decl) const {
  SmallString<256> name;
  raw_svector_ostream nameStream(name);
  mc->mangleCXXRTTIName(QualType(decl->getTypeForDecl(), 0), nameStream);

  return getBackendMangledName(name);
}

std::string APIVisitor::getMangledCXXThunk(const GlobalDecl &decl,
                                           const ThunkInfo &thunk,
                                           bool elideOverrideInfo) const {
  SmallString<256> name;
  raw_svector_ostream nameStream(name);
  const auto *method = cast<CXXMethodDecl>(decl.getDecl());
  if (const auto *dtor = dyn_cast<CXXDestructorDecl>(method))
    mc->mangleCXXDtorThunk(dtor, decl.getDtorType(), thunk.This, 
                           nameStream);
  else
    mc->mangleThunk(method, thunk, nameStream);

  return getBackendMangledName(name);
}

std::string APIVisitor::getMangledCtorDtor(const CXXMethodDecl *decl,
                                           int type) const {
  SmallString<256> name;
  raw_svector_ostream nameStream(name);
  GlobalDecl gd;
  if (const auto *ctor = dyn_cast<CXXConstructorDecl>(decl))
    gd = GlobalDecl(ctor, CXXCtorType(type));
  else {
    const auto *dtor = cast<CXXDestructorDecl>(decl);
    gd = GlobalDecl(dtor, CXXDtorType(type));
  }
  mc->mangleName(gd, nameStream);
  return getBackendMangledName(name);
}

AvailabilityInfo APIVisitor::getAvailabilityInfo(const Decl *decl) const {
  auto platformName = context.getTargetInfo().getPlatformName();

  AvailabilityInfo availability;
  for (const auto *decl : decl->redecls()) {
    for (const auto *A : decl->specific_attrs<AvailabilityAttr>()) {
      if (A->getPlatform()->getName() != platformName)
        continue;

      availability = AvailabilityInfo(A->getIntroduced(), A->getDeprecated(),
                                      A->getObsoleted(), A->getUnavailable(),
                                      false, isAvailabilitySPI(A->getLoc()));
      break;
    }

    if (const auto *attr = decl->getAttr<UnavailableAttr>()) {
      if (!attr->isImplicit())
        availability._unavailable = true;
    }
  }

  // Return default availability.
  return availability;
}

bool APIVisitor::isAvailabilitySPI(SourceLocation loc) const {
  // Check to see if spelling location is in AvailabilityInternalPrivate.h.
  auto &sm = context.getSourceManager();
  auto spellingLoc = sm.getSpellingLoc(loc);
  auto *file = sm.getFileEntryForID(sm.getFileID(spellingLoc));
  if (file && file->getName().endswith("AvailabilityInternalPrivate.h")) 
    return true;
  // If not, search up the macro expansion see if the previous expansion
  // satisfy the requirement.
  if (!loc.isMacroID())
    return false;
  auto &expansion = sm.getSLocEntry(sm.getFileID(loc)).getExpansion();
  return isAvailabilitySPI(expansion.getExpansionLocStart());
}

StringRef APIVisitor::getTypedefName(const TagDecl *decl) const {
  if (const auto *typedefDecl = decl->getTypedefNameForAnonDecl())
    return typedefDecl->getName();

  return {};
}

/// Collect all global variables.
bool APIVisitor::VisitVarDecl(const VarDecl *decl) {
  // Skip function parameters.
  if (isa<ParmVarDecl>(decl))
    return true;

  // Skip variables in records. They are already handled in VisitCXXRecordDecl.
  if (decl->getDeclContext()->isRecord())
    return true;

  // Skip VarDecl inside function or method.
  if (!decl->isDefinedOutsideFunctionOrMethod())
    return true;

  // If this is a template but not specialization or instantiation, skip.
  if (decl->getASTContext().getTemplateOrSpecializationInfo(decl) &&
      decl->getTemplateSpecializationKind() == TSK_Undeclared)
    return true;

  auto attributes = getFileAttributesForDecl(decl);
  if (!attributes)
    return true;
  APIAccess access;
  APILoc loc;
  std::tie(access, loc) = attributes.value();
  auto name = getMangledName(decl);
  auto avail = getAvailabilityInfo(decl);
  bool isWeakDef = decl->hasAttr<WeakAttr>();
  bool isThreadLocal = decl->getTLSKind() != VarDecl::TLS_None;
  auto linkage = isExported(decl) ? APILinkage::Exported : APILinkage::Internal;

  auto *record = frontend.api->addGlobalVariable(
      name, loc, avail, access, decl, linkage, isWeakDef, isThreadLocal);
  frontend.verifier->verify(record);

  return true;
}

bool APIVisitor::VisitFunctionDecl(const FunctionDecl *decl) {
  if (auto method = dyn_cast<CXXMethodDecl>(decl)) {
    // Skip member function in class templates.
    if (method->getParent()->getDescribedClassTemplate() != nullptr)
      return true;

    // Skip methods in records. They are already handled in VisitCXXRecordDecl.
    for (auto p : context.getParents(*method)) {
      if (p.get<CXXRecordDecl>())
        return true;
    }

    // ConstructorDecl and DestructorDecl are handled in CXXRecord.
    if (isa<CXXConstructorDecl>(method) || isa<CXXDestructorDecl>(method))
      return true;
  }

  // Skip templated functions.
  switch (decl->getTemplatedKind()) {
  case FunctionDecl::TK_NonTemplate:
  case FunctionDecl::TK_DependentNonTemplate:
    break;
  case FunctionDecl::TK_MemberSpecialization:
  case FunctionDecl::TK_FunctionTemplateSpecialization:
    if (auto *templateInfo = decl->getTemplateSpecializationInfo()) {
      if (!templateInfo->isExplicitInstantiationOrSpecialization())
        return true;
    }
    break;
  case FunctionDecl::TK_FunctionTemplate:
  case FunctionDecl::TK_DependentFunctionTemplateSpecialization:
    return true;
  }

  auto attributes = getFileAttributesForDecl(decl);
  if (!attributes)
    return true;
  APIAccess access;
  APILoc loc;
  auto name = getMangledName(decl);
  std::tie(access, loc) = attributes.value();
  auto avail = getAvailabilityInfo(decl);
  bool isExplicitInstantiation = decl->getTemplateSpecializationKind() ==
                                 TSK_ExplicitInstantiationDeclaration;
  bool isWeakDef = isExplicitInstantiation || decl->hasAttr<WeakAttr>();
  bool inlined = isInlined(context, decl);
  APILinkage linkage = inlined || !isExported(decl) ? APILinkage::Internal
                                                    : APILinkage::Exported;
  auto *record = frontend.api->addFunction(name, loc, avail, access, decl,
                                           linkage, isWeakDef);
  record->inlined = inlined;
  frontend.verifier->verify(record);

  return true;
}

bool APIVisitor::VisitEnumDecl(const EnumDecl *decl) {
  if (!decl->isComplete())
    return true;

  // Skip forward declaration.
  if (!decl->isThisDeclarationADefinition())
    return true;

  auto attributes = getFileAttributesForDecl(decl);
  if (!attributes)
    return true;
  APIAccess access;
  APILoc loc;
  std::tie(access, loc) = attributes.value();
  auto avail = getAvailabilityInfo(decl);
  auto name = decl->getQualifiedNameAsString();

  SmallString<128> usr;
  clang::index::generateUSRForDecl(decl, usr);

  auto *enumRecord = frontend.api->addEnum(name, usr, loc, avail, access, decl);
  recordEnumConstants(enumRecord, decl->enumerators());

  return true;
}

void APIVisitor::recordEnumConstants(
    EnumRecord *record, const EnumDecl::enumerator_range constants) {
  for (const auto *enumConstant : constants) {
    auto attributes = getFileAttributesForDecl(enumConstant);
    if (!attributes)
      continue;
    APIAccess access;
    APILoc loc;
    std::tie(access, loc) = attributes.value();
    auto avail = getAvailabilityInfo(enumConstant);
    auto name = enumConstant->getQualifiedNameAsString();

    frontend.api->addEnumConstant(record, name, loc, avail, access,
                                 enumConstant);
  }
}

/// \brief Visit all Objective-C Interface declarations.
///
/// Every Objective-C class has an interface declaration that lists all the
/// ivars, properties, and methods of the class.
///
bool APIVisitor::VisitObjCInterfaceDecl(const ObjCInterfaceDecl *decl) {
  // Skip forward declaration for classes (@class)
  if (!decl->isThisDeclarationADefinition())
    return true;

  // Get super class.
  StringRef superClassName;
  SmallString<128> superClassUSR;
  if (const auto *superClass = decl->getSuperClass())
    superClassName = superClass->getObjCRuntimeNameAsString();

  auto attributes = getFileAttributesForDecl(decl);
  if (!attributes)
    return true;

  // When the interface is not exported, then there are no linkable symbols
  // exported from the library. The Objective-C metadata for the class and
  // selectors on the other hand are always recorded.
  auto linkage = isExported(decl) ? APILinkage::Exported : APILinkage::Internal;

  // Record the ObjC Class
  auto name = decl->getObjCRuntimeNameAsString();
  APIAccess access;
  APILoc loc;
  std::tie(access, loc) = attributes.value();
  auto avail = getAvailabilityInfo(decl);

  MachO::ObjCIFSymbolKind symType =
      MachO::ObjCIFSymbolKind::Class | MachO::ObjCIFSymbolKind::MetaClass;
  if (!context.getLangOpts().ObjCRuntime.isFragile() &&
      hasObjCExceptionAttribute(decl))
    symType |= MachO::ObjCIFSymbolKind::EHType;

  auto *objcClass = frontend.api->addObjCInterface(
      name, loc, avail, access, linkage, superClassName, decl, symType);

  frontend.verifier->verify(objcClass);
  // Record all methods (selectors). This doesn't include automatically
  // synthesized property methods.
  recordObjCMethods(objcClass, decl->methods());
  recordObjCProperties(objcClass, decl->properties());
  recordObjCInstanceVariables(objcClass, objcClass->name, decl->ivars());
  recordObjCProtocols(objcClass, decl->protocols());

  return true;
}

/// \brief Visit all Objective-C Category/Extension declarations.
///
/// Objective-C classes may have category or extension declarations that list
/// additional ivars, properties, and methods for the class.
///
/// The class that is being extended might come from a different framework and
/// is therefore itself not recorded.
///
bool APIVisitor::VisitObjCCategoryDecl(const ObjCCategoryDecl *decl) {
  auto name = decl->getName();
  auto attributes = getFileAttributesForDecl(decl);
  if (!attributes)
    return true;
  APIAccess access;
  APILoc loc;
  std::tie(access, loc) = attributes.value();
  auto avail = getAvailabilityInfo(decl);

  const ObjCInterfaceDecl *interfaceDecl = decl->getClassInterface();
  StringRef interfaceName = interfaceDecl->getName();

  auto *category = frontend.api->addObjCCategory(interfaceName, name, loc, avail,
                                                access, decl);

  // Methods in the CoreDataGeneratedAccessors category are dynamically
  // generated during runtime.
  bool isDynamic = name == "CoreDataGeneratedAccessors";
  recordObjCMethods(category, decl->methods(), isDynamic);
  recordObjCProperties(category, decl->properties());
  recordObjCInstanceVariables(category, interfaceName, decl->ivars());
  recordObjCProtocols(category, decl->protocols());

  return true;
}

/// \brief Visit all Objective-C Protocol declarations.
bool APIVisitor::VisitObjCProtocolDecl(const ObjCProtocolDecl *decl) {
  // Skip forward declaration for protocols (@protocol).
  if (!decl->isThisDeclarationADefinition())
    return true;

  auto name = decl->getName();
  auto attributes = getFileAttributesForDecl(decl);
  if (!attributes)
    return true;
  APIAccess access;
  APILoc loc;
  std::tie(access, loc) = attributes.value();
  auto avail = getAvailabilityInfo(decl);

  auto *protocol = frontend.api->addObjCProtocol(name, loc, avail, access, decl);
  recordObjCMethods(protocol, decl->methods());
  recordObjCProperties(protocol, decl->properties());
  recordObjCProtocols(protocol, decl->protocols());

  return true;
}

void APIVisitor::recordObjCMethods(
    ObjCContainerRecord *record, const ObjCContainerDecl::method_range methods,
    bool isDynamic) {
  for (const auto *method : methods) {
    // Don't record selectors for properties.
    if (method->isPropertyAccessor())
      continue;
    auto name = method->getSelector().getAsString();
    auto attributes = getFileAttributesForDecl(method);
    if (!attributes)
      continue;
    APIAccess access;
    APILoc loc;
    std::tie(access, loc) = attributes.value();
    auto avail = getAvailabilityInfo(method);

    frontend.api->addObjCMethod(record, name, loc, avail, access,
                               method->isInstanceMethod(), method->isOptional(),
                               isDynamic, method);
  }
}

void APIVisitor::recordObjCProperties(
    ObjCContainerRecord *record,
    const ObjCContainerDecl::prop_range properties) {
  for (const auto *property : properties) {
    auto attributes = getFileAttributesForDecl(property);
    if (!attributes)
      continue;
    APIAccess access;
    APILoc loc;
    std::tie(access, loc) = attributes.value();
    auto name = property->getName();
    auto getter = property->getGetterName().getAsString();
    auto setter = property->getSetterName().getAsString();
    auto avail = getAvailabilityInfo(property);
    // Get the attributes for property.
    unsigned attr = ObjCPropertyRecord::NoAttr;
    if (property->getPropertyAttributes() &
        ObjCPropertyAttribute::kind_readonly)
      attr |= ObjCPropertyRecord::ReadOnly;
    if (property->getPropertyAttributes() & ObjCPropertyAttribute::kind_class)
      attr |= ObjCPropertyRecord::Class;

    frontend.api->addObjCProperty(record, name, getter, setter, loc, avail,
                                 access,
                                 (ObjCPropertyRecord::AttributeKind)attr,
                                 property->isOptional(), property);
  }
}

void APIVisitor::recordObjCInstanceVariables(
    ObjCContainerRecord *record, StringRef superClassName,
    const iterator_range<DeclContext::specific_decl_iterator<ObjCIvarDecl>>
        ivars) {
  auto linkage = APILinkage::Exported;
  if (context.getLangOpts().ObjCRuntime.isFragile())
    linkage = APILinkage::Unknown;
  // Linkage should be inherited from container, when known.
  else if (record->linkage != APILinkage::Unknown)
    linkage = record->linkage;
  for (const auto *ivar : ivars) {
    auto attributes = getFileAttributesForDecl(ivar);
    if (!attributes)
      continue;
    APIAccess access;
    APILoc loc;
    auto name = ivar->getName();
    std::tie(access, loc) = attributes.value();
    auto avail = getAvailabilityInfo(ivar);
    auto accessControl = ivar->getCanonicalAccessControl();

    auto *ivarRecord = frontend.api->addObjCInstanceVariable(
        record, name, loc, avail, access, accessControl, linkage, ivar);
    frontend.verifier->verify(ivarRecord, superClassName);
  }
}

void APIVisitor::recordObjCProtocols(
    ObjCContainerRecord *container,
    ObjCInterfaceDecl::protocol_range protocols) {
  for (const auto *protocol : protocols)
    frontend.api->addObjCProtocol(container, {protocol->getName()});
}

void APIVisitor::emitVTableSymbols(const CXXRecordDecl *decl, APILoc loc,
                                   AvailabilityInfo avail, APIAccess access,
                                   bool emittedVTable) {
  if (hasVTable(context, decl)) {
    emittedVTable = true;
    auto vtableLinkage = getVTableLinkage(context, decl);
    if (vtableLinkage == LinkageType::ExternalLinkage ||
        vtableLinkage == LinkageType::WeakODRLinkage) {
      auto name = getMangledCXXVTableName(decl);
      bool isWeakDef = vtableLinkage == LinkageType::WeakODRLinkage;
      auto *record = frontend.api->addGlobalVariable(
          name, loc, avail, access, decl, APILinkage::Exported, isWeakDef);
      frontend.verifier->verify(record);

      if (!decl->getDescribedClassTemplate() && !decl->isInvalidDecl()) {
        auto vtable = context.getVTableContext();
        auto addThunk = [&](GlobalDecl decl) {
          auto *thunks = vtable->getThunkInfo(decl);
          if (!thunks)
            return;

          for (auto &thunk : *thunks) {
            auto name =
                getMangledCXXThunk(decl, thunk, /*elideOverrideInfo*/ true);
            auto *record = frontend.api->addFunction(
                name, loc, avail, access, decl.getDecl(), APILinkage::Exported);
            frontend.verifier->verify(record);
          }
        };

        for (auto *method : decl->methods()) {
          if (isa<CXXConstructorDecl>(method) || !method->isVirtual())
            continue;

          if (auto dtor = dyn_cast<CXXDestructorDecl>(method)) {
            // Skip default destructor.
            if (dtor->isDefaulted())
              continue;
            addThunk({dtor, Dtor_Deleting});
            addThunk({dtor, Dtor_Complete});
          } else
            addThunk(method);
        }
      }
    }
  }

  if (!emittedVTable)
    return;

  if (hasRTTI(context, decl)) {
    auto name = getMangledCXXRTTI(decl);
    auto *record = frontend.api->addGlobalVariable(name, loc, avail, access,
                                                   decl, APILinkage::Exported);
    frontend.verifier->verify(record);

    name = getMangledCXXRTTIName(decl);
    record = frontend.api->addGlobalVariable(name, loc, avail, access, decl,
                                             APILinkage::Exported);
    frontend.verifier->verify(record);
  }

  for (const auto &it : decl->bases()) {
    const CXXRecordDecl *base =
        cast<CXXRecordDecl>(it.getType()->castAs<RecordType>()->getDecl());
    auto attributes = getFileAttributesForDecl(base);
    if (!attributes)
      continue;
    APIAccess baseAccess;
    APILoc baseLoc;
    std::tie(baseAccess, baseLoc) = attributes.value();
    auto baseAvail = getAvailabilityInfo(base);
    emitVTableSymbols(base, baseLoc, baseAvail, baseAccess, true);
  }
}

bool APIVisitor::VisitCXXRecordDecl(const CXXRecordDecl *decl) {
  if (!decl->isCompleteDefinition())
    return true;

  // Skip templated classes.
  if (decl->getDescribedClassTemplate() != nullptr)
    return true;

  // Skip partial templated classes too.
  if (isa<ClassTemplatePartialSpecializationDecl>(decl))
    return true;

  auto attributes = getFileAttributesForDecl(decl);
  if (!attributes)
    return true;
  APIAccess access;
  APILoc loc;
  std::tie(access, loc) = attributes.value();
  auto avail = getAvailabilityInfo(decl);

  // Check if we need to emit the vtable/rtti symbols.
  if (isExported(decl))
    emitVTableSymbols(decl, loc, avail, access);

  auto classSpecializationKind = TSK_Undeclared;
  bool keepInlineAsWeak = false;
  if (auto *templ = dyn_cast<ClassTemplateSpecializationDecl>(decl)) {
    classSpecializationKind = templ->getTemplateSpecializationKind();
    if (classSpecializationKind == TSK_ExplicitInstantiationDeclaration)
      keepInlineAsWeak = true;
  }

  // Record the class methods.
  for (const auto *method : decl->methods()) {
    // Inlined methods are usually not emitted - except it comes from a
    // specialized template.
    bool isWeakDef = false;
    if (isInlined(context, method)) {
      if (!keepInlineAsWeak)
        continue;

      isWeakDef = true;
    }

    // Skip the methods that are not exported.
    if (!isExported(method))
      continue;

    switch (method->getTemplateSpecializationKind()) {
    case TSK_Undeclared:
    case TSK_ExplicitSpecialization:
      break;
    case TSK_ImplicitInstantiation:
      continue;
    case TSK_ExplicitInstantiationDeclaration:
      if (classSpecializationKind == TSK_ExplicitInstantiationDeclaration)
        isWeakDef = true;
      break;
    case TSK_ExplicitInstantiationDefinition:
      isWeakDef = true;
      break;
    }

    if (!method->isUserProvided())
      continue;

    // Methods that are deleted are not exported.
    if (method->isDeleted())
      continue;

    auto attributes = getFileAttributesForDecl(method);
    if (!attributes)
      return true;
    APIAccess access;
    APILoc loc;
    std::tie(access, loc) = attributes.value();
    auto avail = getAvailabilityInfo(method);

    if (const auto *ctor = dyn_cast<CXXConstructorDecl>(method)) {
      // Defaulted constructors are not exported.
      if (ctor->isDefaulted())
        continue;

      auto name = getMangledCtorDtor(method, Ctor_Base);
      auto *record = frontend.api->addFunction(name, loc, avail, access, decl,
                                               APILinkage::Exported, isWeakDef);
      frontend.verifier->verify(record);

      if (!decl->isAbstract()) {
        auto name = getMangledCtorDtor(method, Ctor_Complete);
        auto *record = frontend.api->addFunction(
            name, loc, avail, access, decl, APILinkage::Exported, isWeakDef);
        frontend.verifier->verify(record);
      }

      continue;
    }

    if (const auto *dtor = dyn_cast<CXXDestructorDecl>(method)) {
      // Defaulted destructors are not exported.
      if (dtor->isDefaulted())
        continue;

      auto name = getMangledCtorDtor(method, Dtor_Base);
      auto *record = frontend.api->addFunction(name, loc, avail, access, decl,
                                               APILinkage::Exported, isWeakDef);
      frontend.verifier->verify(record);

      name = getMangledCtorDtor(method, Dtor_Complete);
      record = frontend.api->addFunction(name, loc, avail, access, decl,
                                         APILinkage::Exported, isWeakDef);
      frontend.verifier->verify(record);

      if (dtor->isVirtual()) {
        auto name = getMangledCtorDtor(method, Dtor_Deleting);
        auto *record = frontend.api->addFunction(
            name, loc, avail, access, decl, APILinkage::Exported, isWeakDef);
        frontend.verifier->verify(record);
      }

      continue;
    }

    // Though abstract methods can map to exports, this is generally unexpected.
    // Except in the case of destructors.
    // Only ignore pure virtuals after checking if the member
    // function was a destructor.
    if (method->isPure())
      continue;

    auto name = getMangledName(method);
    auto *record = frontend.api->addFunction(name, loc, avail, access, decl,
                                             APILinkage::Exported, isWeakDef);
    frontend.verifier->verify(record);
  }

  if (auto *templ = dyn_cast<ClassTemplateSpecializationDecl>(decl)) {
    if (!templ->isExplicitInstantiationOrSpecialization())
      return true;
  }

  using var_iter = CXXRecordDecl::specific_decl_iterator<VarDecl>;
  using var_range = iterator_range<var_iter>;
  for (auto *var : var_range(decl->decls())) {
    // Skip const static member variables.
    // \code
    // struct S {
    //   static const int x = 0;
    // };
    // \endcode
    if (var->isStaticDataMember() && var->hasInit())
      continue;

    // Skip unexported var decls.
    if (!isExported(var))
      continue;

    auto name = getMangledName(var);
    auto attributes = getFileAttributesForDecl(var);
    if (!attributes)
      return true;
    APIAccess access;
    APILoc loc;
    std::tie(access, loc) = attributes.value();
    auto avail = getAvailabilityInfo(var);
    bool isWeakDef = var->hasAttr<WeakAttr>() || keepInlineAsWeak;

    auto *record = frontend.api->addGlobalVariable(
        name, loc, avail, access, var, APILinkage::Exported, isWeakDef);
    frontend.verifier->verify(record);
  }

  return true;
}

bool APIVisitor::VisitTypedefNameDecl(const TypedefNameDecl *decl) {
  // Skip ObjC Type Parameter for now.
  if (isa<ObjCTypeParamDecl>(decl))
   return true;

  if (!decl->isDefinedOutsideFunctionOrMethod())
    return true;

  auto attributes = getFileAttributesForDecl(decl);
  if (!attributes)
    return true;
  APIAccess access;
  APILoc loc;
  std::tie(access, loc) = attributes.value();
  auto name = decl->getName();
  auto avail = getAvailabilityInfo(decl);

  frontend.api->addTypeDef(name, loc, avail, access, decl);
  return true;
}
} // end namespace clang.
