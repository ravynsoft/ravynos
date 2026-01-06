//===- lib/Core/API.cpp - TAPI API ------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "tapi/Core/API.h"
#include "tapi/Core/APIVisitor.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringSwitch.h"

using namespace llvm;
using namespace clang;

TAPI_NAMESPACE_INTERNAL_BEGIN

APILoc::APILoc(std::string file, unsigned line, unsigned col)
    : file(file), line(line), col(col) {}

APILoc::APILoc(StringRef file, unsigned line, unsigned col)
    : file(file.str()), line(line), col(col) {}

bool APILoc::isInvalid() const {
  if (presumedLoc)
    return presumedLoc->isInvalid();
  else if (file.empty())
    return true;

  return false;
}

StringRef APILoc::getFilename() const {
  if (presumedLoc)
    return presumedLoc->getFilename();
  return file;
}

unsigned APILoc::getLine() const {
  if (presumedLoc)
    return presumedLoc->getLine();
  return line;
}

unsigned APILoc::getColumn() const {
  if (presumedLoc)
    return presumedLoc->getColumn();
  return col;
}

clang::PresumedLoc APILoc::getPresumedLoc() const {
  assert(presumedLoc && "must have an underlying PresumedLoc");
  return *presumedLoc;
}

clang::SourceLocation APILoc::getSourceLocation() const { return *sourceLoc; }

APIRecord *APIRecord::create(BumpPtrAllocator &allocator, StringRef name,
                             APILinkage linkage, SymbolFlags flags, APILoc loc,
                             const AvailabilityInfo &availability,
                             APIAccess access, const Decl *decl) {
  return new (allocator)
      APIRecord{name, loc, decl, availability, linkage, flags, access};
}

GlobalRecord *GlobalRecord::create(BumpPtrAllocator &allocator, StringRef name,
                                   APILinkage linkage, SymbolFlags flags,
                                   APILoc loc,
                                   const AvailabilityInfo &availability,
                                   APIAccess access, const Decl *decl,
                                   GVKind kind) {
  return new (allocator)
      GlobalRecord{name, flags, loc, availability, access, decl, kind, linkage};
}

EnumRecord *EnumRecord::create(BumpPtrAllocator &allocator, StringRef name,
                               StringRef usr, APILoc loc,
                               const AvailabilityInfo &availability,
                               APIAccess access, const Decl *decl) {
  return new (allocator) EnumRecord{name, usr, loc, availability, access, decl};
}

template <typename InputT>
static bool areChildrenRecordsEqual(const std::vector<InputT> &lhs,
                                    const std::vector<InputT> &rhs) {
  if (lhs.size() != rhs.size())
    return false;
  std::vector<InputT> lhsChildren = lhs;
  std::vector<InputT> rhsChildren = rhs;
  llvm::sort(lhsChildren, [](const auto *lhs, const auto *rhs) {
    return lhs->name < rhs->name;
  });
  llvm::sort(rhsChildren, [](const auto *lhs, const auto *rhs) {
    return lhs->name < rhs->name;
  });
  for (auto lhsIt = lhsChildren.begin(), rhsIt = rhsChildren.begin();
       lhsIt != lhsChildren.end() && rhsIt != rhsChildren.end();
       ++lhsIt, ++rhsIt) {
    if (!(**lhsIt == **rhsIt))
      return false;
  }
  return true;
}

bool EnumRecord::operator==(const EnumRecord &other) const {
  if (!(APIRecord::operator==(other) && usr == other.usr))
    return false;
  return areChildrenRecordsEqual(constants, other.constants);
}

bool ObjCContainerRecord::operator==(const ObjCContainerRecord &other) const {
  if (!(APIRecord::operator==(other)))
    return false;
  if (!areChildrenRecordsEqual(methods, other.methods))
    return false;
  if (!areChildrenRecordsEqual(properties, other.properties))
    return false;
  if (!areChildrenRecordsEqual(ivars, other.ivars))
    return false;
  if (protocols != other.protocols)
    return false;

  return true;
}

bool ObjCInterfaceRecord::operator==(const ObjCInterfaceRecord &other) const {
  if (!ObjCContainerRecord::operator==(other))
    return false;
  if (linkages != other.linkages)
    return false;
  if (superClass != other.superClass)
    return false;
  if (!areChildrenRecordsEqual(categories, other.categories))
    return false;

  return true;
}

EnumConstantRecord *
EnumConstantRecord::create(BumpPtrAllocator &allocator, StringRef name,
                           APILoc loc, const AvailabilityInfo &availability,
                           APIAccess access, const Decl *decl) {
  return new (allocator)
      EnumConstantRecord{name, loc, availability, access, decl};
}

ObjCMethodRecord *
ObjCMethodRecord::create(BumpPtrAllocator &allocator, StringRef name,
                         APILoc loc, const AvailabilityInfo &availability,
                         APIAccess access, bool isInstanceMethod,
                         bool isOptional, bool isDynamic, const Decl *decl) {
  return new (allocator) ObjCMethodRecord{
      name,       loc,       availability, access, isInstanceMethod,
      isOptional, isDynamic, decl};
}

ObjCPropertyRecord *
ObjCPropertyRecord::create(BumpPtrAllocator &allocator, StringRef name,
                           StringRef getterName, StringRef setterName,
                           APILoc loc, const AvailabilityInfo &availability,
                           APIAccess access, AttributeKind attributes,
                           bool isOptional, const Decl *decl) {
  return new (allocator)
      ObjCPropertyRecord{name,   getterName, setterName, loc, availability,
                         access, attributes, isOptional, decl};
}

ObjCInstanceVariableRecord *ObjCInstanceVariableRecord::create(
    BumpPtrAllocator &allocator, StringRef name, APILinkage linkage, APILoc loc,
    const AvailabilityInfo &availability, APIAccess access,
    AccessControl accessControl, const Decl *decl) {
  return new (allocator) ObjCInstanceVariableRecord{
      name, linkage, loc, availability, access, accessControl, decl};
}

ObjCInterfaceRecord *ObjCInterfaceRecord::create(
    BumpPtrAllocator &allocator, StringRef name, APILinkage linkage, APILoc loc,
    const AvailabilityInfo &availability, APIAccess access,
    StringRef superClass, const Decl *decl, ObjCIFSymbolKind symType) {
  return new (allocator) ObjCInterfaceRecord{
      name, linkage, loc, availability, access, superClass, decl, symType};
}

ObjCCategoryRecord *
ObjCCategoryRecord::create(BumpPtrAllocator &allocator, StringRef interface,
                           StringRef name, APILoc loc,
                           const AvailabilityInfo &availability,
                           APIAccess access, const Decl *decl) {
  return new (allocator)
      ObjCCategoryRecord{interface, name, loc, availability, access, decl};
}

ObjCProtocolRecord *
ObjCProtocolRecord::create(BumpPtrAllocator &allocator, StringRef name,
                           APILoc loc, const AvailabilityInfo &availability,
                           APIAccess access, const Decl *decl) {
  return new (allocator)
      ObjCProtocolRecord{name, loc, availability, access, decl};
}

TypedefRecord *TypedefRecord::create(llvm::BumpPtrAllocator &allocator,
                                     StringRef name, APILoc loc,
                                     const AvailabilityInfo &availability,
                                     APIAccess access, const Decl *decl) {
  return new (allocator) TypedefRecord{name, loc, availability, access, decl};
}

bool API::updateAPIAccess(APIRecord *record, APIAccess access) {
  if (record->access <= access)
    return false;

  record->access = access;
  return true;
}

bool API::updateAPILinkage(APIRecord *record, APILinkage linkage) {
  if (record->linkage >= linkage)
    return false;

  record->linkage = linkage;
  return true;
}

bool ObjCInterfaceRecord::isExportedSymbol(ObjCIFSymbolKind currentType) const {
  return getLinkageForSymbol(currentType) >= APILinkage::Reexported;
}

APILinkage
ObjCInterfaceRecord::getLinkageForSymbol(ObjCIFSymbolKind currentType) const {
  assert(currentType <= ObjCIFSymbolKind::EHType &&
         "expected single ObjCIFSymbolKind enum value");
  if (currentType == ObjCIFSymbolKind::Class)
    return linkages.Class;

  if (currentType == ObjCIFSymbolKind::MetaClass)
    return linkages.MetaClass;

  if (currentType == ObjCIFSymbolKind::EHType)
    return linkages.EHType;

  llvm_unreachable("unexpected ObjCIFSymbolKind");
}

void ObjCInterfaceRecord::updateLinkageForSymbols(ObjCIFSymbolKind symType,
                                                  APILinkage link) {
  if ((symType & ObjCIFSymbolKind::Class) == ObjCIFSymbolKind::Class)
    linkages.Class = std::max(link, linkages.Class);
  if ((symType & ObjCIFSymbolKind::MetaClass) == ObjCIFSymbolKind::MetaClass)
    linkages.MetaClass = std::max(link, linkages.MetaClass);
  if ((symType & ObjCIFSymbolKind::EHType) == ObjCIFSymbolKind::EHType)
    linkages.EHType = std::max(link, linkages.EHType);

  // Obj-C Classes represent multiple symbols that could have competing
  // linkages, in this case assign the largest one, when querying the linkage of
  // the record itself.
  linkage =
      std::max(linkages.Class, std::max(linkages.MetaClass, linkages.EHType));
}

APIRecord *API::addGlobalFromBinary(StringRef name, SymbolFlags flags,
                                    APILoc loc, GVKind kind,
                                    APILinkage linkage) {
  // See if there is a specific APIRecord type to capture instead.
  auto [apiName, symbolKind, interfaceType] = MachO::parseSymbol(name);
  name = apiName;
  switch (symbolKind) {
  case EncodeKind::GlobalSymbol:
    return addGlobal(name, flags, loc, AvailabilityInfo(), APIAccess::Unknown,
                     nullptr, kind, linkage);
  case EncodeKind::ObjectiveCClass: {
    return addObjCInterface(name, loc, AvailabilityInfo(), APIAccess::Unknown,
                            linkage, {}, nullptr, interfaceType);
  }
  case EncodeKind::ObjectiveCClassEHType: {
    auto *record =
        addObjCInterface(name, loc, AvailabilityInfo(), APIAccess::Unknown,
                         linkage, {}, nullptr, interfaceType);

    // When classes without ehtype are used in try/catch blocks
    // a weak-defined symbol is exported.
    if ((flags & SymbolFlags::WeakDefined) == SymbolFlags::WeakDefined)
      record->flags = SymbolFlags::WeakDefined;
    return record;
  }
  case EncodeKind::ObjectiveCInstanceVariable: {
    auto [superClass, ivar] = name.split('.');
    // Attempt to find super class.
    ObjCContainerRecord *container = findObjCInterface(superClass);

    // Ivars can only exist with extensions, if they did not come from
    // class.
    if (container == nullptr)
      container = findObjCCategory(superClass, superClass);

    // If not found, create extension since there is no mapped class symbol.
    if (container == nullptr)
      container = addObjCCategory(superClass, {}, APILoc(), AvailabilityInfo(),
                                  APIAccess::Unknown, nullptr);
    return addObjCInstanceVariable(
        container, ivar, loc, AvailabilityInfo(), APIAccess::Unknown,
        ObjCInstanceVariableRecord::AccessControl::None, linkage, nullptr);
  }
  }

  llvm_unreachable("unexpected symbol kind when adding to API");
}

GlobalRecord *API::addGlobal(StringRef name, APILoc loc,
                             const AvailabilityInfo &availability,
                             APIAccess access, const Decl *decl, GVKind kind,
                             APILinkage linkage, bool isWeakDefined,
                             bool isThreadLocal) {
  auto flags = SymbolFlags::None;
  if (isWeakDefined)
    flags |= SymbolFlags::WeakDefined;
  if (isThreadLocal)
    flags |= SymbolFlags::ThreadLocalValue;
  if (kind == GVKind::Function)
    flags |= SymbolFlags::Text;
  else
    flags |= SymbolFlags::Data;
  return addGlobal(name, flags, loc, availability, access, decl, kind, linkage);
}

GlobalRecord *API::addGlobal(StringRef name, SymbolFlags flags, APILoc loc,
                             const AvailabilityInfo &availability,
                             APIAccess access, const Decl *decl, GVKind kind,
                             APILinkage linkage) {
  name = copyString(name);
  auto result = globals.insert({name, nullptr});
  if (result.second) {
    auto *record = GlobalRecord::create(allocator, name, linkage, flags, loc,
                                        availability, access, decl, kind);
    result.first->second = record;
  }

  API::updateAPIAccess(result.first->second, access);
  API::updateAPILinkage(result.first->second, linkage);
  return result.first->second;
}

GlobalRecord *API::addGlobalVariable(StringRef name, APILoc loc,
                                     const AvailabilityInfo &availability,
                                     APIAccess access, const Decl *decl,
                                     APILinkage linkage, bool isWeakDefined,
                                     bool isThreadLocal) {
  return addGlobal(name, loc, availability, access, decl, GVKind::Variable,
                   linkage, isWeakDefined, isThreadLocal);
}

GlobalRecord *API::addFunction(StringRef name, APILoc loc,
                               const AvailabilityInfo &availability,
                               APIAccess access, const Decl *decl,
                               APILinkage linkage, bool isWeakDefined) {
  return addGlobal(name, loc, availability, access, decl, GVKind::Function,
                   linkage, isWeakDefined);
}

EnumRecord *API::addEnum(StringRef name, StringRef usr, APILoc loc,
                         const AvailabilityInfo &availability, APIAccess access,
                         const Decl *decl) {
  usr = copyString(usr);
  // Use USR as the key, as all anonymous enums have the same name.
  auto result = enums.insert({usr, nullptr});
  if (result.second) {
    name = copyString(name);
    auto *record = EnumRecord::create(allocator, name, usr, loc, availability,
                                      access, decl);
    result.first->second = record;
  }
  return result.first->second;
}

EnumConstantRecord *API::addEnumConstant(EnumRecord *record, StringRef name,
                                         APILoc loc,
                                         const AvailabilityInfo &availability,
                                         APIAccess access, const Decl *decl) {
  name = copyString(name);
  auto *constant = EnumConstantRecord::create(allocator, name, loc,
                                              availability, access, decl);
  record->constants.push_back(constant);
  return constant;
}

ObjCInterfaceRecord *API::addObjCInterface(
    StringRef name, APILoc loc, const AvailabilityInfo &availability,
    APIAccess access, APILinkage linkage, StringRef superClass,
    const Decl *decl, ObjCIFSymbolKind symType, bool overrideLinkage) {
  name = copyString(name);
  superClass = copyString(superClass);
  auto result = interfaces.insert({name, nullptr});

  if (result.second) {
    auto *record =
        ObjCInterfaceRecord::create(allocator, name, linkage, loc, availability,
                                    access, superClass, decl, symType);
    result.first->second = record;
  }

  // Inheritance may not always be known when class is first added.
  if (result.first->second->superClass.empty() && !superClass.empty()) {
    result.first->second->superClass = superClass;
  }

  if (overrideLinkage || result.second)
    result.first->second->updateLinkageForSymbols(symType, linkage);
  return result.first->second;
}

ObjCCategoryRecord *API::addObjCCategory(StringRef interface, StringRef name,
                                         APILoc loc,
                                         const AvailabilityInfo &availability,
                                         APIAccess access, const Decl *decl) {
  interface = copyString(interface);
  name = copyString(name);
  auto result = categories.insert({std::make_pair(interface, name), nullptr});
  if (result.second) {
    auto *record = ObjCCategoryRecord::create(allocator, interface, name, loc,
                                              availability, access, decl);
    result.first->second = record;
  }

  auto it = interfaces.find(interface);
  if (it != interfaces.end())
    it->second->categories.push_back(result.first->second);

  return result.first->second;
}

ObjCProtocolRecord *API::addObjCProtocol(StringRef name, APILoc loc,
                                         const AvailabilityInfo &availability,
                                         APIAccess access, const Decl *decl) {
  name = copyString(name);
  auto result = protocols.insert({name, nullptr});
  if (result.second) {
    auto *record = ObjCProtocolRecord::create(allocator, name, loc,
                                              availability, access, decl);
    result.first->second = record;
  }

  return result.first->second;
}

void API::addObjCProtocol(ObjCContainerRecord *record, StringRef protocol) {
  protocol = copyString(protocol);
  record->protocols.push_back(protocol);
}

ObjCMethodRecord *API::addObjCMethod(ObjCContainerRecord *record,
                                     StringRef name, APILoc loc,
                                     const AvailabilityInfo &availability,
                                     APIAccess access, bool isInstanceMethod,
                                     bool isOptional, bool isDynamic,
                                     const Decl *decl) {
  name = copyString(name);
  auto *method =
      ObjCMethodRecord::create(allocator, name, loc, availability, access,
                               isInstanceMethod, isOptional, isDynamic, decl);
  record->methods.push_back(method);
  return method;
}

ObjCPropertyRecord *
API::addObjCProperty(ObjCContainerRecord *record, StringRef name,
                     StringRef getterName, StringRef setterName, APILoc loc,
                     const AvailabilityInfo &availability, APIAccess access,
                     ObjCPropertyRecord::AttributeKind attributes,
                     bool isOptional, const Decl *decl) {
  name = copyString(name);
  getterName = copyString(getterName);
  setterName = copyString(setterName);
  auto *property = ObjCPropertyRecord::create(
      allocator, name, getterName, setterName, loc, availability, access,
      attributes, isOptional, decl);
  record->properties.push_back(property);
  return property;
}

ObjCInstanceVariableRecord *API::addObjCInstanceVariable(
    ObjCContainerRecord *record, StringRef name, APILoc loc,
    const AvailabilityInfo &availability, APIAccess access,
    ObjCInstanceVariableRecord::AccessControl accessControl, APILinkage linkage,
    const Decl *decl) {
  name = copyString(name);
  auto *ivar = ObjCInstanceVariableRecord::create(
      allocator, name, linkage, loc, availability, access, accessControl, decl);
  record->ivars.push_back(ivar);
  return ivar;
}

TypedefRecord *API::addTypeDef(StringRef name, APILoc loc,
                               const AvailabilityInfo &availability,
                               APIAccess access, const Decl *decl) {
  name = copyString(name);
  auto result = typeDefs.insert({name, nullptr});
  if (result.second) {
    auto *record =
        TypedefRecord::create(allocator, name, loc, availability, access, decl);
    result.first->second = record;
  }
  return result.first->second;
}

GlobalRecord *API::findGlobal(StringRef name) const {
  auto it = globals.find(name);
  if (it != globals.end())
    return it->second;
  return nullptr;
}

GlobalRecord *API::findGlobalVariable(StringRef name) const {
  auto it = globals.find(name);
  if (it != globals.end() && it->second->kind == GVKind::Variable)
    return it->second;
  return nullptr;
}

GlobalRecord *API::findFunction(StringRef name) const {
  auto it = globals.find(name);
  if (it != globals.end() && it->second->kind == GVKind::Function)
    return it->second;
  return nullptr;
}

const TypedefRecord *API::findTypeDef(StringRef name) const {
  auto it = typeDefs.find(name);
  if (it != typeDefs.end())
    return it->second;
  return nullptr;
}

const EnumRecord *API::findEnum(StringRef usr) const {
  auto it = enums.find(usr);
  if (it != enums.end())
    return it->second;
  return nullptr;
}

ObjCContainerRecord *API::findContainer(StringRef ivar) const {
  auto [superClassName, _] = ivar.split('.');
  ObjCContainerRecord *container = findObjCInterface(superClassName);
  // Ivars can only exist with extensions, if they did not come from
  // class.
  if (container == nullptr)
    container = findObjCCategory(superClassName, "");
  return container;
}

ObjCInstanceVariableRecord *API::findIVar(StringRef name,
                                          bool isSymbolName) const {
  if (isSymbolName) {
    auto *container = findContainer(name);
    if (!container)
      return nullptr;

    StringRef ivarName = name.substr(name.find_first_of('.') + 1);
    auto it = find_if(container->ivars, [ivarName](auto *ivar) {
      return ivar && ivar->name == ivarName;
    });

    if (it == container->ivars.end())
      return nullptr;
    return *it;
  }

  for (const auto &[_, record] : interfaces) {
    auto it = find_if(record->ivars, [name](auto *ivar) {
      return ivar && ivar->name == name;
    });
    if (it != record->ivars.end())
      return *it;
  }

  for (const auto &[_, record] : categories) {
    auto it = find_if(record->ivars, [name](auto *ivar) {
      return ivar && ivar->name == name;
    });
    if (it != record->ivars.end())
      return *it;
  }

  return nullptr;
}

ObjCInterfaceRecord *API::findObjCInterface(StringRef name) const {
  auto it = interfaces.find(name);
  if (it != interfaces.end())
    return it->second;
  return nullptr;
}

const ObjCProtocolRecord *API::findObjCProtocol(StringRef name) const {
  auto it = protocols.find(name);
  if (it != protocols.end())
    return it->second;
  return nullptr;
}

ObjCCategoryRecord *API::findObjCCategory(StringRef interfaceName,
                                          StringRef name) const {
  auto it = categories.find({interfaceName, name});
  if (it != categories.end())
    return it->second;
  return nullptr;
}

void API::visit(APIVisitor &visitor) const {
  for (auto &it : typeDefs)
    visitor.visitTypeDef(*it.second);
  for (auto &it : globals)
    visitor.visitGlobal(*it.second);
  for (auto &it : enums)
    visitor.visitEnum(*it.second);
  for (auto &it : protocols)
    visitor.visitObjCProtocol(*it.second);
  for (auto &it : interfaces)
    visitor.visitObjCInterface(*it.second);
  for (auto &it : categories)
    visitor.visitObjCCategory(*it.second);
}

void API::visit(APIMutator &visitor) {
  for (auto &it : typeDefs)
    visitor.visitTypeDef(*it.second);
  for (auto &it : globals)
    visitor.visitGlobal(*it.second);
  for (auto &it : enums)
    visitor.visitEnum(*it.second);
  for (auto &it : protocols)
    visitor.visitObjCProtocol(*it.second);
  for (auto &it : interfaces)
    visitor.visitObjCInterface(*it.second);
  for (auto &it : categories)
    visitor.visitObjCCategory(*it.second);
}

StringRef API::copyStringInto(StringRef string,
                              llvm::BumpPtrAllocator &allocator) {
  if (string.empty())
    return {};

  if (allocator.identifyObject(string.data()))
    return string;

  void *ptr = allocator.Allocate(string.size(), 1);
  memcpy(ptr, string.data(), string.size());
  return StringRef(reinterpret_cast<const char *>(ptr), string.size());
}

StringRef API::copyString(StringRef string) {
  return API::copyStringInto(string, allocator);
}

BinaryInfo &API::getBinaryInfo() {
  if (hasBinaryInfo())
    return *binaryInfo;

  // Allocate in bumpPtrAllocator so it has a stable address.
  binaryInfo =  new (allocator) BinaryInfo{};
  return *binaryInfo;
}

bool API::operator<(const API &other) const {
  // First, let's see if we can order them based on binaryInfo.
  // 1. Put the one with binary info first.
  if (!hasBinaryInfo() && other.hasBinaryInfo())
    return false;
  if (hasBinaryInfo() && !other.hasBinaryInfo())
    return true;

  // 2. Sort by binary kind and installName.
  if (hasBinaryInfo() && other.hasBinaryInfo()) {
    if (getBinaryInfo() != other.getBinaryInfo())
      return getBinaryInfo() < other.getBinaryInfo();
  }

  // 3. Sorted by target triple.
  // Doing string comparsion here since version matters.
  if (triple.str() != other.triple.str())
    return triple.str() < other.triple.str();

  // 4. Sort by number of APIs. Pick the one has more APIs.
  if (globals.size() != other.globals.size())
    return globals.size() > other.globals.size();
  if (interfaces.size() != other.interfaces.size())
    return interfaces.size() > other.interfaces.size();
  if (protocols.size() != other.protocols.size())
    return protocols.size() > other.protocols.size();
  if (enums.size() != other.enums.size())
    return enums.size() > other.enums.size();

  // fallback plan. unstable ordering.
  return false;
}

template <typename InputT> static bool hasEqualRecords(InputT lhs, InputT rhs) {
  if (lhs.size() != rhs.size())
    return false;

  for (const auto &[key, val] : lhs) {
    auto rhsIt = rhs.find(key);
    if (rhsIt == rhs.end())
      return false;
    if (!(*(rhsIt->second) == *val))
      return false;
  }

  return true;
}

bool API::operator==(const API &other) const {
  if (triple != other.triple)
    return false;
  if (projectName != other.projectName)
    return false;

  if (!hasEqualRecords<GlobalRecordMap>(globals, other.globals))
    return false;
  if (!hasEqualRecords<EnumRecordMap>(enums, other.enums))
    return false;
  if (!hasEqualRecords<TypedefMap>(typeDefs, other.typeDefs))
    return false;
  if (!hasEqualRecords<ObjCInterfaceRecordMap>(interfaces, other.interfaces))
    return false;
  if (!hasEqualRecords<ObjCCategoryRecordMap>(categories, other.categories))
    return false;
  if (!hasEqualRecords<ObjCProtocolRecordMap>(protocols, other.protocols))
    return false;

  if (hasBinaryInfo() && !other.hasBinaryInfo())
    return false;
  if (!hasBinaryInfo() && other.hasBinaryInfo())
    return false;
  if (hasBinaryInfo() && *binaryInfo != other.getBinaryInfo())
    return false;
  return true;
}

TAPI_NAMESPACE_INTERNAL_END
