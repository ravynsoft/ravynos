//===- tapi/SDKDB/SDKDB.h - TAPI SDKDB --------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief SDKDB interface.
///
//===----------------------------------------------------------------------===//
#ifndef TAPI_SDKDB_SDKDB_H
#define TAPI_SDKDB_SDKDB_H

#include "tapi/Core/API.h"
#include "tapi/Diagnostics/Diagnostics.h"
#include "tapi/SDKDB/CompareConfigFileReader.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/Error.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/TextAPI/Platform.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

class SDKDBBuilder;


class SDKDB {
public:
  SDKDB(const llvm::Triple &triple, SDKDBBuilder *builder)
      : triple(triple), frontendAPI(triple), builder(builder) {}

  /// ObjCContainer type for lookup.
  enum ObjCContainerKind : unsigned {
    ObjCClass = 0,
    ObjCCategory = 1,
    ObjCProtocol = 2
  };

  const llvm::Triple &getTargetTriple() const { return triple; }

  /// Return true if compatible with target.
  static bool areCompatibleTargets(const llvm::Triple &lhs,
                                   const llvm::Triple &rhs);

  /// Insert API into SDKDB and transfer the ownership.
  API &recordAPI(API &&api);

  /// Insert APIs into global lookup map.
  void insertGlobal(GlobalRecord *record, const BinaryInfo *binInfo,
                    StringRef project);
  void insertObjCInterface(ObjCInterfaceRecord *record,
                           const BinaryInfo *binInfo, StringRef project);
  void insertObjCCategory(ObjCCategoryRecord *record, const BinaryInfo *binInfo,
                          StringRef project);
  void insertObjCProtocol(ObjCProtocolRecord *record, const BinaryInfo *binInfo,
                          StringRef project);
  void insertEnum(EnumRecord *record, const BinaryInfo *binInfo,
                  StringRef project);
  void insertTypeDef(TypedefRecord *record, const BinaryInfo *binInfo,
                     StringRef project);

  /// Helper function to lookup APIs in the maps.
  SmallVector<GlobalRecord *, 2> findGlobals(StringRef name) const;
  ObjCInterfaceRecord *findObjCInterface(StringRef name) const;
  ObjCProtocolRecord *findObjCProtocol(StringRef protocolName) const;
  ObjCCategoryRecord *findObjCCategory(StringRef categoryName,
                                       StringRef clsName) const;
  SmallVector<ObjCCategoryRecord *, 4>
  findObjCCategoryForClass(StringRef clsName) const;
  EnumRecord *findEnum(StringRef name) const;
  APIRecord *findTypedef(StringRef name) const;

  /// Add information to the APIs in the SDKDB.
  void annotateGlobal(const GlobalRecord *record);
  void annotateObjCInterface(const ObjCInterfaceRecord *record);
  void annotateObjCCategory(const ObjCCategoryRecord *record);
  void annotateObjCProtocol(const ObjCProtocolRecord *record);

  std::vector<const API*> api() const;
  std::vector<API*> api();

  std::vector<const EnumRecord*> getEnumRecords() const;
  std::vector<const APIRecord*> getTypedefRecords() const;

  /// Finalize SDKDB by fixing up all API information globally.
  llvm::Error finalize();

  /// Build lookup tables.
  void buildLookupTables();

  /// Compare two SDKDBs.
  void diagnoseDifferences(const SDKDB &baseline) const;

  bool shouldDiagnoseProject(StringRef projectName) const;
  bool shouldDiagnoseLibrary(StringRef installName) const;

  /// Helper function to add to frontendAPIs.
  EnumRecord *addEnum(const EnumRecord &record);
  TypedefRecord *addTypeDef(const TypedefRecord &record);
  ObjCProtocolRecord *addObjCProtocol(const ObjCProtocolRecord &record);

private:
  friend class SDKDBBuilder;

  template <typename T> class MapEntry {
  public:
    MapEntry(T record, const BinaryInfo *info, StringRef project)
        : record(record), info(info), project(project.str()), poison(false) {}

    T getRecord() const { return record; }
    const BinaryInfo *getBinaryInfo() const { return info; }
    StringRef getInstallName() const {
      return info ? info->installName : "unknown";
    }
    StringRef getProjectName() const { return project; }

    bool isPoison() const { return poison; }
    void setPoison() { poison = true; }

    bool operator<(const MapEntry<T> &other) const {
      // Both has binInfo.
      if (info && other.info)
        return (*info < *other.info ||
                (*info == *other.info && project < other.project));

      // Both does not have binInfo
      if (!info && !other.info)
        return project < other.project;

      // The one with the binInfo is smaller and ordered first.
      return info;
    }

    bool operator==(const MapEntry<T> &other) const {
      if (info && other.info)
        return (*info == *other.info && project == other.project);

      return (info == other.info && project == other.project);
    }

    bool operator!=(const MapEntry<T> &other) const {
      return !(*this == other);
    }

  private:
    T record;
    const BinaryInfo *info;
    std::string project;
    bool poison;
  };

  template<typename T> using LookupMap = llvm::StringMap<MapEntry<T>>;

  using InterfaceMapType = LookupMap<ObjCInterfaceRecord*>;
  using ProtocolMapType = LookupMap<ObjCProtocolRecord*>;
  using CategoryMapType = llvm::StringMap<LookupMap<ObjCCategoryRecord *>>;
  using GlobalMapType =
      llvm::StringMap<llvm::SmallVector<MapEntry<GlobalRecord *>, 1>>;
  using EnumMapType = LookupMap<EnumRecord*>;
  using TypedefMapType = LookupMap<TypedefRecord *>;

  /// Lookup map from global to its api record.
  GlobalMapType globalMap;
  /// Lookup map from objc interface to its api record.
  InterfaceMapType interfaceMap;
  /// Lookup map from objc interface to all its categories' api record.
  CategoryMapType categoryMap;
  /// Lookup map from objc protocol to its api record.
  ProtocolMapType protocolMap;
  /// Lookup map for enums.
  EnumMapType enumMap;
  /// Lookup map for typedefs.
  TypedefMapType typedefMap;

  /// get super class record.
  ObjCInterfaceRecord *getSuperclass(const ObjCInterfaceRecord *record) const;

  /// get objc method access.
  APIAccess getAccessForObjCMethod(APIAccess access, StringRef name,
                                   bool isInstanceMethod,
                                   ObjCContainerRecord *container);
  APIAccess getAccessForObjCMethod(APIAccess access, StringRef name,
                                   bool isInstanceMethod,
                                   ObjCInterfaceRecord *interface);

  /// get objc property access.
  APIAccess getAccessForObjCProperty(APIAccess access, StringRef name,
                                     bool isClassProperty,
                                     ObjCContainerRecord *container);
  APIAccess getAccessForObjCProperty(APIAccess access, StringRef name,
                                     bool isClassProperty,
                                     ObjCInterfaceRecord *interface);

  /// find and update the global.
  bool findAndUpdateGlobal(Twine name, const APIRecord &record);

  /// find objc method from an interface.
  /// If nothing found in record, it will search all categories for current
  /// record interfaces to find the method.
  /// For categories, it will search fallbackInterface to find other categories
  /// might defined the method.
  ObjCMethodRecord *findMethod(StringRef name, bool isInstanceMethod,
                               ObjCContainerRecord &record,
                               ObjCContainerKind kind,
                               StringRef fallbackInterfaceName = "");

  /// Try to resolve a method given the selector and kind for an Objective-C
  /// interface. This would be the equivalent API surface for this method on
  /// this class.
  ObjCMethodRecord const *
  resolveMethod(StringRef name, bool isInstanceMethod,
                const ObjCInterfaceRecord &interface) const;

  /// get objc runtime version.
  bool isObjC1() const { return triple.isMacOSX() && triple.isArch32Bit(); }

  const llvm::Triple triple;
  API frontendAPI;
  SDKDBBuilder *builder;

  /// Map from project name to its APIs
  std::map<std::string, std::vector<API>> apiCache;

  /// Map from install name to the contributing project name
  llvm::StringMap<StringRef> installNames;

  /// Adjacent list DAG for dylib reexports. Edges go from reexported libraries
  /// to reexporting libraries.
  llvm::StringMap<SmallVector<StringRef, 3>> reexportGraph;

  /// Check whether `installName` is eventually reexported by `reexportedBy`
  bool isReexportedBy(StringRef installName, StringRef reexportedBy) const;

  /// Check whether `installName` is eventually (re)exported in the public SDK.
  bool isPubliclyExported(StringRef installName) const;

  std::set<CompareConfigFileReader::Change> const *expectedChanges = nullptr;
  bool isExpectedChange(const CompareConfigFileReader::Change &change) const {
    if (expectedChanges)
      return expectedChanges->count(change);
    return false;
  }

  llvm::BumpPtrAllocator danglingAPIAllocator;
};

// 16 bits, higher bits reserved for future use.
// This enum is streamed into bitcode so the existing entries cannot be changed.
enum class SDKDBBuilderOptions : uint16_t {
  defaultOpt = 0,
  isPublicOnly = 1,           // only has Public API, default no.
  noObjCMetadata = 1 << 1,    // no objc metadata, default no.
  preserveSourceLoc = 1 << 2, // contain source location, defualt no.
  hasUUID = 1 << 3,           // contains UUID, defualt no.
  excludeBundles = 1 << 4,     // exclude bundles from SDKDB, default no.
  excludeEnumTypes = 1 << 5,   // exclude enums and typedefs, default no.
  LLVM_MARK_AS_BITMASK_ENUM(excludeEnumTypes)
};

class SDKDBBuilder {
public:
  SDKDBBuilder(DiagnosticsEngine &diag,
               SDKDBBuilderOptions options = SDKDBBuilderOptions::defaultOpt,
               StringRef buildVersion = "")
      : diag(diag), options(options), buildVersion(buildVersion.str()) {}

  /// Add API from binary interface.
  llvm::Error addBinaryAPI(API &&api);

  /// Add API from header interface. It is used to annotate the binary from 
  /// binary interface. All the binary interfaces must be added before calling
  /// this method.
  llvm::Error addHeaderAPI(const API &api);

  /// Finalize SDKDB.
  llvm::Error finalize();

  /// Parse SDKDB JSON.
  llvm::Error parse(StringRef JSON);

  /// Set build verison.
  void setBuildVersion(StringRef version) {
    buildVersion = version.str();
  }

  /// Get build version.
  StringRef getBuildVersion() const { return buildVersion; }

  /// Write output.
  void serialize(raw_ostream &os, bool compact) const;

  /// SDKDB private interface.
  SDKDB &getSDKDBForTarget(const llvm::Triple &triple);

  void updateAPIRecord(APIRecord &base, const APIRecord &record);
  void updateGlobal(GlobalRecord &base, const GlobalRecord &record);

  /// Update items inside an ObjCContainer (not the container record itself).
  /// For categories, it need to pass interfaceName as a fallback for method
  /// searching.
  void updateObjCContainerItems(SDKDB &sdkdb, ObjCContainerRecord &base,
                                const ObjCContainerRecord &record,
                                SDKDB::ObjCContainerKind kind,
                                StringRef fallbackInterfaceName = "");
  void updateObjCInterface(SDKDB &sdkdb, ObjCInterfaceRecord &base,
                           const ObjCInterfaceRecord &record);
  void updateObjCCategory(SDKDB &sdkdb, ObjCCategoryRecord &base,
                          const ObjCCategoryRecord &record);
  void updateObjCProtocol(SDKDB &sdkdb, ObjCProtocolRecord &base,
                          const ObjCProtocolRecord &record);

  clang::DiagnosticBuilder report(unsigned diagID) {
    return diag.report(diagID);
  }

  std::vector<const SDKDB*> getDatabases() const;

  bool isMaybePublicSelector(StringRef selector) const {
    return maybePublicSelector.count(selector);
  }

  bool isMaybePublicProperty(StringRef property) const {
    return maybePublicProperty.count(property);
  }

  void addProjectWithError(StringRef project) {
    projectWithError.emplace_back(project.data(), project.size());
  }

  const std::vector<std::string> getProjectWithError() const {
    return projectWithError;
  }

  bool isPublicOnly() const {
    return (bool)(options & SDKDBBuilderOptions::isPublicOnly);
  }

  bool noObjCMetadata() const {
    return ((bool)(options & SDKDBBuilderOptions::noObjCMetadata));
  }

  bool hasUUID() const {
    return (bool)(options & SDKDBBuilderOptions::hasUUID);
  }

  bool preserveLocation() const {
    return (bool)(options & SDKDBBuilderOptions::preserveSourceLoc);
  }

  void setBuilderOptions(SDKDBBuilderOptions opts) { options = opts; }
  uint16_t getRawOptionEncoding() const { return (uint16_t)options; }

  void setRemoveObjCMetadata() {
    options |= SDKDBBuilderOptions::noObjCMetadata;
  }

  bool excludeBundles() const {
    return (bool)(options & SDKDBBuilderOptions::excludeBundles);
  }

  void setRemoveBundles() {
    options |= SDKDBBuilderOptions::excludeBundles;
  }

  bool excludeEnumTypes() const {
    return (bool)(options & SDKDBBuilderOptions::excludeEnumTypes);
  }

  void setRemoveEnumTypes() {
    options |= SDKDBBuilderOptions::excludeEnumTypes;
  }

  void buildLookupTables();
  bool diagnoseDifferences(SDKDBBuilder &baseline);
  void setReportNewAPIasError(bool val);
  void setNoNewAPI(bool val);
  void setDiagnoseFrontendAPI(bool val) { shouldDiagnoseFrontendAPI = val; }
  bool diagnoseFrontendAPI() const { return shouldDiagnoseFrontendAPI; }
  void
  setCompareConfigFileReader(std::unique_ptr<CompareConfigFileReader> reader) {
    compareConfigFileReader = std::move(reader);
  }
  CompareConfigFileReader *getCompareConfigFileReader() const {
    return compareConfigFileReader.get();
  }

  bool setFilteredPlatforms(StringRef platform);

  bool shouldDiagnoseTriple(llvm::Triple triple) {
    // Only platform is checked.
    if (filterPlatforms.empty())
      return true;
    return filterPlatforms.contains(llvm::MachO::mapToPlatformType(triple));
  }

private:
  DiagnosticsEngine &diag;
  SDKDBBuilderOptions options;
  std::string buildVersion;
  llvm::SmallVector<SDKDB, 4> databases;
  llvm::StringSet<> maybePublicSelector;
  llvm::StringSet<> maybePublicProperty;
  llvm::MachO::PlatformSet filterPlatforms;
  // Projects that contributes to the SDKDB but has errors when scanning roots.
  std::vector<std::string> projectWithError;
  // Do not try to compare enums and typedefs by default.
  bool shouldDiagnoseFrontendAPI = false;
  std::unique_ptr<CompareConfigFileReader> compareConfigFileReader;
};

TAPI_NAMESPACE_INTERNAL_END

#endif
