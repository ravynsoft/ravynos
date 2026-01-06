//===- tapi/Core/API.h - TAPI API -------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the API.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_API_H
#define TAPI_CORE_API_H

#include "tapi/Core/APICommon.h"
#include "tapi/Core/AvailabilityInfo.h"
#include "tapi/Core/LLVM.h"
#include "tapi/Core/Utils.h"
#include "tapi/Defines.h"
#include "clang/AST/DeclObjC.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/Error.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/TextAPI/InterfaceFile.h"
#include "llvm/TextAPI/PackedVersion.h"
#include <iterator>
#include <optional>

using clang::Decl;

TAPI_NAMESPACE_INTERNAL_BEGIN

class API;
using SymbolFlags = llvm::MachO::SymbolFlags;

class APIRange {
public:
  APIRange() = default;
  APIRange(APILoc loc) : begin(loc), end(loc) {}
  APIRange(APILoc begin, APILoc end) : begin(begin), end(end) {}

  APILoc getBegin() const { return begin; }
  APILoc getEnd() const { return end; }

private:
  APILoc begin;
  APILoc end;
};

struct APIRecord {
  StringRef name;
  APILoc loc;
  const Decl *decl;
  AvailabilityInfo availability;
  APILinkage linkage;
  SymbolFlags flags;
  APIAccess access;
  bool verified = false;

  static APIRecord *create(llvm::BumpPtrAllocator &allocator, StringRef name,
                           APILinkage linkage, SymbolFlags flags, APILoc loc,
                           const AvailabilityInfo &availability,
                           APIAccess access, const Decl *decl);

  bool isWeakDefined() const {
    return (flags & SymbolFlags::WeakDefined) == SymbolFlags::WeakDefined;
  }

  bool isWeakReferenced() const {
    return (flags & SymbolFlags::WeakReferenced) == SymbolFlags::WeakReferenced;
  }

  bool isThreadLocalValue() const {
    return (flags & SymbolFlags::ThreadLocalValue) ==
           SymbolFlags::ThreadLocalValue;
  }

  bool isData() const {
    return (flags & SymbolFlags::Data) == SymbolFlags::Data;
  }

  bool isText() const {
    return (flags & SymbolFlags::Text) == SymbolFlags::Text;
  }

  bool isInternal() const { return linkage == APILinkage::Internal; }
  bool isExternal() const { return linkage == APILinkage::External; }
  bool isExported() const { return linkage >= APILinkage::Reexported; }
  bool isReexported() const { return linkage == APILinkage::Reexported; }

  bool operator==(const APIRecord &other) const {
    return std::tie(name, loc, availability, linkage, flags, access) ==
           std::tie(other.name, other.loc, other.availability, other.linkage,
                    other.flags, other.access);
  }
};

struct EnumConstantRecord : APIRecord {
  EnumConstantRecord(StringRef name, APILoc loc,
                     const AvailabilityInfo &availability, APIAccess access,
                     const Decl *decl)
      : APIRecord({name, loc, decl, availability, APILinkage::Unknown,
                   SymbolFlags::None, access}) {}
  static EnumConstantRecord *create(llvm::BumpPtrAllocator &allocator,
                                    StringRef name, APILoc loc,
                                    const AvailabilityInfo &availability,
                                    APIAccess access, const Decl *decl);
};

struct EnumRecord : APIRecord {
  std::vector<EnumConstantRecord *> constants;
  StringRef usr;

  EnumRecord(StringRef name, StringRef usr, APILoc loc,
             const AvailabilityInfo &availability, APIAccess access,
             const Decl *decl)
      : APIRecord({name, loc, decl, availability, APILinkage::Unknown,
                   SymbolFlags::None, access}),
        usr(usr) {}
  static EnumRecord *create(llvm::BumpPtrAllocator &allocator, StringRef name,
                            StringRef usr, APILoc loc,
                            const AvailabilityInfo &availability,
                            APIAccess access, const Decl *decl);

  bool operator==(const EnumRecord &other) const;
};

enum class GVKind : uint8_t {
  Unknown = 0,
  Variable = 1,
  Function = 2,
};

struct GlobalRecord : APIRecord {
  GVKind kind;
  bool inlined = false;

  GlobalRecord(StringRef name, SymbolFlags flags, APILoc loc,
               const AvailabilityInfo &availability, APIAccess access,
               const Decl *decl, GVKind kind, APILinkage linkage)
      : APIRecord({name, loc, decl, availability, linkage, flags, access}),
        kind(kind) {}

  static GlobalRecord *create(llvm::BumpPtrAllocator &allocator, StringRef name,
                              APILinkage linkage, SymbolFlags flags, APILoc loc,
                              const AvailabilityInfo &availability,
                              APIAccess access, const Decl *decl, GVKind kind);

  bool operator==(const GlobalRecord &other) const {
    return APIRecord::operator==(other) && kind == other.kind;
  }
};

struct ObjCPropertyRecord : APIRecord {
  enum AttributeKind : unsigned {
    NoAttr = 0,
    ReadOnly = 1,
    Class = 1 << 1,
    Dynamic = 1 << 2,
  };

  AttributeKind attributes;
  StringRef getterName;
  StringRef setterName;
  bool isOptional;

  ObjCPropertyRecord(StringRef name, StringRef getterName, StringRef setterName,
                     APILoc loc, const AvailabilityInfo &availability,
                     APIAccess access, AttributeKind attributes,
                     bool isOptional, const Decl *decl)
      : APIRecord({name, loc, decl, availability, APILinkage::Unknown,
                   SymbolFlags::None, access}),
        attributes(attributes), getterName(getterName), setterName(setterName),
        isOptional(isOptional) {}

  static ObjCPropertyRecord *create(llvm::BumpPtrAllocator &allocator,
                                    StringRef name, StringRef getterName,
                                    StringRef setterName, APILoc loc,
                                    const AvailabilityInfo &availability,
                                    APIAccess access, AttributeKind attributes,
                                    bool isOptional, const Decl *decl);

  bool isReadOnly() const { return attributes & ReadOnly; }
  bool isDynamic() const { return attributes & Dynamic; }
  bool isClassProperty() const { return attributes & Class; }
};

struct ObjCInstanceVariableRecord : APIRecord {
  using AccessControl = clang::ObjCIvarDecl::AccessControl;
  AccessControl accessControl;

  ObjCInstanceVariableRecord(StringRef name, APILinkage linkage, APILoc loc,
                             const AvailabilityInfo &availability,
                             APIAccess access, AccessControl accessControl,
                             const Decl *decl)
      : APIRecord({name, loc, decl, availability, linkage, SymbolFlags::Data,
                   access}),
        accessControl(accessControl) {}

  static ObjCInstanceVariableRecord *
  create(llvm::BumpPtrAllocator &allocator, StringRef name, APILinkage linkage,
         APILoc loc, const AvailabilityInfo &availability, APIAccess access,
         AccessControl accessControl, const Decl *decl);
  static std::string createName(StringRef superClass, StringRef ivarName) {
    return (superClass + "." + ivarName).str();
  }
};

struct ObjCMethodRecord : APIRecord {
  bool isInstanceMethod;
  bool isOptional;
  bool isDynamic;

  ObjCMethodRecord(StringRef name, APILoc loc,
                   const AvailabilityInfo &availability, APIAccess access,
                   bool isInstanceMethod, bool isOptional, bool isDynamic,
                   const Decl *decl)
      : APIRecord({name, loc, decl, availability, APILinkage::Unknown,
                   SymbolFlags::None, access}),
        isInstanceMethod(isInstanceMethod), isOptional(isOptional),
        isDynamic(isDynamic) {}

  static ObjCMethodRecord *create(llvm::BumpPtrAllocator &allocator,
                                  StringRef name, APILoc loc,
                                  const AvailabilityInfo &availability,
                                  APIAccess access, bool isInstanceMethod,
                                  bool isOptional, bool isDynamic,
                                  const Decl *decl);
};

struct ObjCContainerRecord : APIRecord {
  std::vector<ObjCMethodRecord *> methods;
  std::vector<ObjCPropertyRecord *> properties;
  std::vector<ObjCInstanceVariableRecord *> ivars;
  std::vector<StringRef> protocols;

  ObjCContainerRecord(StringRef name, APILinkage linkage, APILoc loc,
                      const AvailabilityInfo &availability, APIAccess access,
                      const Decl *decl)
      : APIRecord({name, loc, decl, availability, linkage, SymbolFlags::Data,
                   access}) {}

  bool operator==(const ObjCContainerRecord &other) const;
};

struct ObjCCategoryRecord : ObjCContainerRecord {
  StringRef interface;

  ObjCCategoryRecord(StringRef interface, StringRef name, APILoc loc,
                     const AvailabilityInfo &availability, APIAccess access,
                     const Decl *decl)
      : ObjCContainerRecord(name, APILinkage::Unknown, loc, availability,
                            access, decl),
        interface(interface) {}

  static ObjCCategoryRecord *create(llvm::BumpPtrAllocator &allocator,
                                    StringRef interface, StringRef name,
                                    APILoc loc,
                                    const AvailabilityInfo &availability,
                                    APIAccess access, const Decl *decl);

  bool operator==(const ObjCCategoryRecord &other) const {
    return ObjCContainerRecord::operator==(other) &&
           interface == other.interface;
  }
};

struct ObjCProtocolRecord : ObjCContainerRecord {
  ObjCProtocolRecord(StringRef name, APILoc loc,
                     const AvailabilityInfo &availability, APIAccess access,
                     const Decl *decl)
      : ObjCContainerRecord(name, APILinkage::Unknown, loc, availability,
                            access, decl) {}

  static ObjCProtocolRecord *create(llvm::BumpPtrAllocator &allocator,
                                    StringRef name, APILoc loc,
                                    const AvailabilityInfo &availability,
                                    APIAccess access, const Decl *decl);
};

struct ObjCInterfaceRecord : ObjCContainerRecord {
private:
  struct Linkages {
    APILinkage Class = APILinkage::Unknown;
    APILinkage MetaClass = APILinkage::Unknown;
    APILinkage EHType = APILinkage::Unknown;
    bool operator==(const Linkages &other) const {
      return std::tie(Class, MetaClass, EHType) ==
             std::tie(other.Class, other.MetaClass, other.EHType);
    }
    bool operator!=(const Linkages &other) const { return !(*this == other); }
  };
  Linkages linkages;

public:
  std::vector<const ObjCCategoryRecord *> categories;
  StringRef superClass;

  ObjCInterfaceRecord(StringRef name, APILinkage linkage, APILoc loc,
                      const AvailabilityInfo &availability, APIAccess access,
                      StringRef superClass, const Decl *decl,
                      ObjCIFSymbolKind symType)
      : ObjCContainerRecord(name, linkage, loc, availability, access, decl),
        superClass(superClass) {}

  static ObjCInterfaceRecord *
  create(llvm::BumpPtrAllocator &allocator, StringRef name, APILinkage linkage,
         APILoc loc, const AvailabilityInfo &availability, APIAccess access,
         StringRef superClass, const Decl *decl, ObjCIFSymbolKind symType);

  bool hasExceptionAttribute() const {
    return linkages.EHType != APILinkage::Unknown;
  }
  bool isCompleteInterface() const {
    return linkages.Class >= APILinkage::Reexported &&
           linkages.MetaClass >= APILinkage::Reexported;
  }

  APILinkage getLinkageForSymbol(ObjCIFSymbolKind currentType) const;
  void updateLinkageForSymbols(ObjCIFSymbolKind symType, APILinkage link);
  bool isExportedSymbol(ObjCIFSymbolKind currentType) const;

  bool operator==(const ObjCInterfaceRecord &other) const;
};

struct TypedefRecord : APIRecord {
  TypedefRecord(StringRef name, APILoc loc,
                const AvailabilityInfo &availability, APIAccess access,
                const Decl *decl)
      : APIRecord({name, loc, decl, availability, APILinkage::Unknown,
                   SymbolFlags::None, access}) {}

  static TypedefRecord *create(llvm::BumpPtrAllocator &allocator,
                               StringRef name, APILoc loc,
                               const AvailabilityInfo &availability,
                               APIAccess access, const Decl *decl);
};

class APIVisitor;
class APIMutator;

struct BinaryInfo {
  FileType fileType = FileType::Invalid;
  llvm::MachO::PackedVersion currentVersion;
  llvm::MachO::PackedVersion compatibilityVersion;
  uint8_t swiftABIVersion = 0;
  bool isTwoLevelNamespace = false;
  bool isAppExtensionSafe = false;
  bool isOSLibNotForSharedCache = false;
  StringRef parentUmbrella;
  std::vector<StringRef> allowableClients;
  std::vector<StringRef> reexportedLibraries;
  std::vector<StringRef> rpaths;
  std::vector<StringRef> relinkedLibraries;
  StringRef installName;
  StringRef uuid;
  StringRef path;
};

// Order of the BinaryInfo.
inline bool operator<(const BinaryInfo &lhs, const BinaryInfo &rhs) {
  // Invalid ones goes to the end. Otherwise, first sort by file type.
  if (lhs.fileType != rhs.fileType) {
    if (lhs.fileType == FileType::Invalid)
      return false;
    if (rhs.fileType == FileType::Invalid)
      return true;

    return lhs.fileType < rhs.fileType;
  }

  // Two level names space goes first.
  if (lhs.isTwoLevelNamespace != rhs.isTwoLevelNamespace)
    return lhs.isTwoLevelNamespace;

  // Empty paths goes in the end.
  bool lhsPathEmpty = lhs.installName.empty();
  bool rhsPathEmpty = rhs.installName.empty();
  if (lhsPathEmpty != rhsPathEmpty)
    return rhsPathEmpty;

  // RelativePath goes afterwards.
  bool lhsRelativePath = lhs.installName.starts_with("@");
  bool rhsRelativePath = rhs.installName.starts_with("@");
  if (lhsRelativePath != rhsRelativePath)
    return rhsRelativePath;

  // Public path goes first.
  bool lhsPublic = isPublicDylib(lhs.installName);
  bool rhsPublic = isPublicDylib(rhs.installName);
  if (lhsPublic != rhsPublic)
    return lhsPublic;

  // Check public location in SDK.
  bool lhsPublicLocation = isWithinPublicLocation(lhs.installName);
  bool rhsPublicLocation = isWithinPublicLocation(rhs.installName);
  if (lhsPublicLocation != rhsPublicLocation)
    return lhsPublicLocation;

  // Last sort by installName.
  return lhs.installName < rhs.installName;
}

// Compare only the bits that differentiate two binaries.
inline bool operator==(const BinaryInfo &lhs, const BinaryInfo &rhs) {
  return std::tie(lhs.fileType, lhs.installName, lhs.isTwoLevelNamespace) ==
         std::tie(rhs.fileType, rhs.installName, rhs.isTwoLevelNamespace);
}

inline bool operator!=(const BinaryInfo &lhs, const BinaryInfo &rhs) {
  return !(lhs == rhs);
}

class API {
public:
  API(const llvm::Triple &triple) : triple(triple), target(triple) {}
  const llvm::Triple &getTriple() const { return triple; }
  const Target &getTarget() const { return target; }

  StringRef getProjectName() const { return projectName; }
  void setProjectName(StringRef project) { projectName = copyString(project); }

  static bool updateAPIAccess(APIRecord *record, APIAccess access);
  static bool updateAPILinkage(APIRecord *record, APILinkage linkage);

  EnumRecord *addEnum(StringRef name, StringRef usr, APILoc loc,
                      const AvailabilityInfo &availability, APIAccess access,
                      const Decl *decl);
  EnumConstantRecord *addEnumConstant(EnumRecord *record, StringRef name,
                                      APILoc loc,
                                      const AvailabilityInfo &availability,
                                      APIAccess access, const Decl *decl);
  APIRecord *addGlobalFromBinary(StringRef name, SymbolFlags flags, APILoc loc,
                                 GVKind kind = GVKind::Unknown,
                                 APILinkage linkage = APILinkage::Unknown);
  GlobalRecord *
  addGlobal(StringRef name, APILoc loc, const AvailabilityInfo &availability,
            APIAccess access, const Decl *decl, GVKind kind = GVKind::Unknown,
            APILinkage linkage = APILinkage::Unknown,
            bool isWeakDefined = false, bool isThreadLocal = false);
  GlobalRecord *addGlobal(StringRef name, SymbolFlags flags, APILoc loc,
                          const AvailabilityInfo &availability,
                          APIAccess access, const Decl *decl,
                          GVKind kind = GVKind::Unknown,
                          APILinkage linkage = APILinkage::Unknown);
  GlobalRecord *addGlobalVariable(StringRef name, APILoc loc,
                                  const AvailabilityInfo &availability,
                                  APIAccess access, const Decl *decl,
                                  APILinkage linkage = APILinkage::Unknown,
                                  bool isWeakDefined = false,
                                  bool isThreadLocal = false);
  GlobalRecord *addFunction(StringRef name, APILoc loc,
                            const AvailabilityInfo &availability,
                            APIAccess access, const Decl *decl,
                            APILinkage linkage = APILinkage::Unknown,
                            bool isWeakDefined = false);
  ObjCInterfaceRecord *addObjCInterface(StringRef name, APILoc loc,
                                        const AvailabilityInfo &availability,
                                        APIAccess access, APILinkage linkage,
                                        StringRef superClass, const Decl *decl,
                                        ObjCIFSymbolKind symType,
                                        bool overrideLinkage = true);
  ObjCCategoryRecord *addObjCCategory(StringRef interface, StringRef name,
                                      APILoc loc,
                                      const AvailabilityInfo &availability,
                                      APIAccess access, const Decl *decl);
  ObjCProtocolRecord *addObjCProtocol(StringRef name, APILoc loc,
                                      const AvailabilityInfo &availability,
                                      APIAccess access, const Decl *decl);
  void addObjCProtocol(ObjCContainerRecord *record, StringRef protocol);
  ObjCMethodRecord *addObjCMethod(ObjCContainerRecord *record, StringRef name,
                                  APILoc loc,
                                  const AvailabilityInfo &availability,
                                  APIAccess access, bool isInstanceMethod,
                                  bool isOptional, bool isDynamic,
                                  const Decl *decl);
  ObjCPropertyRecord *
  addObjCProperty(ObjCContainerRecord *record, StringRef name,
                  StringRef getterName, StringRef setterName, APILoc loc,
                  const AvailabilityInfo &availability, APIAccess access,
                  ObjCPropertyRecord::AttributeKind attributes, bool isOptional,
                  const Decl *decl);
  ObjCInstanceVariableRecord *addObjCInstanceVariable(
      ObjCContainerRecord *record, StringRef name, APILoc loc,
      const AvailabilityInfo &availability, APIAccess access,
      ObjCInstanceVariableRecord::AccessControl accessControl,
      APILinkage linkage, const Decl *decl);

  TypedefRecord *addTypeDef(StringRef name, APILoc loc,
                            const AvailabilityInfo &availability,
                            APIAccess access, const Decl *decl);

  void addPotentiallyDefinedSelector(StringRef name) {
    potentiallyDefinedSelectors.insert(name);
  }

  llvm::StringSet<> &getPotentiallyDefinedSelectors() {
    return potentiallyDefinedSelectors;
  }

  const llvm::StringSet<> &getPotentiallyDefinedSelectors() const {
    return potentiallyDefinedSelectors;
  }

  void visit(APIMutator &visitor);
  void visit(APIVisitor &visitor) const;

  const TypedefRecord *findTypeDef(StringRef) const;
  const EnumRecord *findEnum(StringRef name) const;
  const ObjCProtocolRecord *findObjCProtocol(StringRef) const;
  // Container types can have ivars/properties added to it.
  ObjCInterfaceRecord *findObjCInterface(StringRef) const;
  ObjCCategoryRecord *findObjCCategory(StringRef, StringRef) const;
  ObjCContainerRecord *findContainer(StringRef ivar) const;
  ObjCInstanceVariableRecord *findIVar(StringRef, bool isSymbolName) const;
  GlobalRecord *findGlobalVariable(StringRef) const;
  GlobalRecord *findFunction(StringRef) const;
  // Find Global Record without concern about GVKind.
  GlobalRecord *findGlobal(StringRef) const;

  bool hasBinaryInfo() const { return binaryInfo; }
  BinaryInfo &getBinaryInfo();
  const BinaryInfo &getBinaryInfo() const {
    assert(hasBinaryInfo() && "must have binary info");
    return *binaryInfo;
  }

  std::optional<StringRef> getInstallName() const {
    if (!hasBinaryInfo())
      return std::nullopt;
    return getBinaryInfo().installName;
  }

  bool operator<(const API &other) const;
  // Expensive equality operator.
  bool operator==(const API &other) const;
  bool operator!=(const API &other) const { return !(*this == other); }

  StringRef copyString(StringRef string);
  static StringRef copyStringInto(StringRef, llvm::BumpPtrAllocator &);

  bool isEmpty() const {
    return !hasBinaryInfo() && globals.empty() && enums.empty() &&
           interfaces.empty() && categories.empty() && protocols.empty() &&
           typeDefs.empty() && potentiallyDefinedSelectors.empty();
  }

private:
  using GlobalRecordMap = llvm::MapVector<StringRef, GlobalRecord *>;
  using EnumRecordMap = llvm::MapVector<StringRef, EnumRecord *>;
  using ObjCInterfaceRecordMap =
      llvm::MapVector<StringRef, ObjCInterfaceRecord *>;
  using ObjCCategoryRecordMap =
      llvm::MapVector<std::pair<StringRef, StringRef>, ObjCCategoryRecord *>;
  using ObjCProtocolRecordMap =
      llvm::MapVector<StringRef, ObjCProtocolRecord *>;
  using TypedefMap = llvm::MapVector<StringRef, TypedefRecord *>;

  llvm::BumpPtrAllocator allocator;

  const llvm::Triple triple;
  // Hold tapi converted triple to avoid unecessary casts.
  const Target target;

  GlobalRecordMap globals;
  EnumRecordMap enums;
  ObjCInterfaceRecordMap interfaces;
  ObjCCategoryRecordMap categories;
  ObjCProtocolRecordMap protocols;
  TypedefMap typeDefs;
  llvm::StringSet<> potentiallyDefinedSelectors;

  StringRef projectName;
  BinaryInfo *binaryInfo = nullptr;

  friend class APIVerifier;
  friend class SortedAPI;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_API_H
