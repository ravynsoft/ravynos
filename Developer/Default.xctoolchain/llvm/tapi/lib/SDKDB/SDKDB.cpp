//===- tapi/SDKDB/SDKDB.cpp - TAPI SDKDB ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the SDKDB
///
//===----------------------------------------------------------------------===//

#include "tapi/SDKDB/SDKDB.h"
#include "tapi/SDKDB/CompareConfigFileReader.h"

#include "tapi/Core/APIJSONSerializer.h"
#include "tapi/Core/APIVisitor.h"
#include "tapi/Core/Utils.h"
#include "tapi/Diagnostics/Diagnostics.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/JSON.h"
#include <vector>

using namespace llvm;
using namespace llvm::MachO;

TAPI_NAMESPACE_INTERNAL_BEGIN

class LookupMapBuilder : public APIMutator {
public:
  LookupMapBuilder(SDKDB &sdkdb, API &api) : sdkdb(sdkdb), api(api) {
    binInfo = api.hasBinaryInfo() ? &api.getBinaryInfo() : nullptr;
  }

  void visitGlobal(GlobalRecord &record) override {
    sdkdb.insertGlobal(&record, binInfo, api.getProjectName());
  }
  void visitObjCInterface(ObjCInterfaceRecord &record) override {
    sdkdb.insertObjCInterface(&record, binInfo, api.getProjectName());
  }
  void visitObjCCategory(ObjCCategoryRecord &record) override {
    sdkdb.insertObjCCategory(&record, binInfo, api.getProjectName());
  }
  void visitObjCProtocol(ObjCProtocolRecord &record) override {
    sdkdb.insertObjCProtocol(&record, binInfo, api.getProjectName());
  }
  void visitEnum(EnumRecord &record) override {
    sdkdb.insertEnum(&record, binInfo, api.getProjectName());
  }
  void visitTypeDef(TypedefRecord &record) override {
    sdkdb.insertTypeDef(&record, binInfo, api.getProjectName());
  }

private:
  SDKDB &sdkdb;
  API &api;
  const BinaryInfo *binInfo;
};

class APIAnnotator : public APIVisitor {
public:
  APIAnnotator(SDKDB &sdkdb, StringRef project)
      : sdkdb(sdkdb), project(project) {}

  void visitGlobal(const GlobalRecord &record) override {
    sdkdb.annotateGlobal(&record);
  }
  void visitObjCInterface(const ObjCInterfaceRecord &record) override {
    sdkdb.annotateObjCInterface(&record);
  }
  void visitObjCCategory(const ObjCCategoryRecord &record) override {
    sdkdb.annotateObjCCategory(&record);
  }
  void visitObjCProtocol(const ObjCProtocolRecord &record) override {
    if (sdkdb.findObjCProtocol(record.name)) {
      sdkdb.annotateObjCProtocol(&record);
      return;
    }

    // Make a copy of the record in frontendAPI, because the protocol only
    // exists in the HeaderAPI.
    auto *copy = sdkdb.addObjCProtocol(record);
    sdkdb.insertObjCProtocol(copy, nullptr, project);
  }

  void visitEnum(const EnumRecord &record) override {
    // Make a copy of the record in frontendAPI.
    // enums are in the HeaderAPIs which we do not preserve in SDKDB.
    auto *copy = sdkdb.addEnum(record);
    sdkdb.insertEnum(copy, nullptr, project);
  }

  void visitTypeDef(const TypedefRecord &record) override {
    // create a copy because typedefs are only in header APIs.
    auto *copy = sdkdb.addTypeDef(record);
    sdkdb.insertTypeDef(copy, nullptr, project);
  }

private:
  SDKDB &sdkdb;
  StringRef project;
};

bool SDKDB::areCompatibleTargets(const Triple &lhs, const Triple &rhs) {
  if (lhs != rhs)
    return false;

  // x86_64 and x86_64h are two different slices.
  if (lhs.getArchName().startswith("x86_64") &&
      lhs.getArchName() != rhs.getArchName())
    return false;

  return true;
}

API &SDKDB::recordAPI(API &&api) {
  auto name = api.getProjectName().str();
  apiCache[name].emplace_back(std::move(api));

  auto installName = api.getInstallName();
  if (!installName.value_or("").empty()) {
    auto [it, inserted] =
        installNames.try_emplace(installName.value(), api.getProjectName());
    if (!inserted)
      builder->report(diag::warn_sdkdb_conflict_install_name)
          << installName.value() << it->getValue() << api.getProjectName();

    for (const auto reexport : api.getBinaryInfo().reexportedLibraries)
      reexportGraph[reexport].emplace_back(*installName);
  }

  return apiCache[name].back();
}

void SDKDB::insertGlobal(GlobalRecord *record, const BinaryInfo *binInfo,
                         StringRef project) {
  // only insert exported and re-exported symbols into map.
  if (record->linkage < APILinkage::Reexported)
    return;

  auto key = record->name;
  auto &value = globalMap[key];

  // Check for duplicated non weak symbols.
  if (!value.empty() && !record->isWeakDefined()) {
    if (any_of(value, [](const MapEntry<GlobalRecord *> &entry) {
          return !entry.getRecord()->isWeakDefined();
        }))
      builder->report(diag::warn_sdkdb_duplicated_global)
          << key << binInfo->installName;
    for (const auto &entry : value) {
      if (entry.getRecord()->isWeakDefined())
        continue;
      builder->report(diag::note_sdkdb_previous_definition)
          << entry.getBinaryInfo()->installName;
    }
  }

  value.emplace_back(record, binInfo, project);
}

void SDKDB::insertObjCInterface(ObjCInterfaceRecord *record,
                                const BinaryInfo *binInfo, StringRef project) {
  auto key = record->name;
  auto result = interfaceMap.try_emplace(key, record, binInfo, project);
  // If emplace successful.
  if (result.second)
    return; 

  auto entry = result.first;
  // Entry has been seen before, report duplicated class.
  builder->report(diag::warn_sdkdb_duplicated_objc_class) << key;

  // If entires are "equal", set the entry to poison since we don't know which
  // to pick so we pick neither.
  MapEntry<ObjCInterfaceRecord*> current{record, binInfo, project};
  if (current == entry->getValue()) {
    entry->getValue().setPoison();
    return;
  }

  // Set the value if current value should replace the one in the map.
  if (current < entry->getValue())
    entry->setValue(current);
}

void SDKDB::insertObjCCategory(ObjCCategoryRecord *record,
                               const BinaryInfo *binInfo, StringRef project) {
  auto &catMap = categoryMap[record->interface];
  auto result = catMap.try_emplace(record->name, record, binInfo, project);
  // If emplace successful.
  if (result.second)
    return; 

  auto entry = result.first;
  builder->report(diag::warn_sdkdb_duplicated_objc_category)
      << record->interface << record->name;

  // If entires are "equal", set the entry to poison since we don't know which
  // to pick so we pick neither.
  MapEntry<ObjCCategoryRecord*> current{record, binInfo, project};
  if (current == entry->getValue()) {
    entry->getValue().setPoison();
    return;
  }

  // Set the value if current value should replace the one in the map.
  if (current < entry->getValue())
    entry->setValue(current);
}

EnumRecord *SDKDB::addEnum(const EnumRecord &record) {
  auto *copy =
      frontendAPI.addEnum(record.name, record.usr, record.loc,
                          record.availability, record.access, record.decl);
  for (auto *constant : record.constants)
    frontendAPI.addEnumConstant(copy, constant->name, constant->loc,
                                constant->availability, constant->access,
                                constant->decl);

  return copy;
}

TypedefRecord *SDKDB::addTypeDef(const TypedefRecord &record) {
  return frontendAPI.addTypeDef(record.name, record.loc, record.availability,
                                record.access, record.decl);
}

ObjCProtocolRecord *SDKDB::addObjCProtocol(const ObjCProtocolRecord &record) {
  auto *copy = frontendAPI.addObjCProtocol(
      record.name, record.loc, record.availability, record.access, record.decl);
  for (auto *method : record.methods)
    frontendAPI.addObjCMethod(copy, method->name, method->loc,
                              method->availability, method->access,
                              method->isInstanceMethod, method->isOptional,
                              method->isDynamic, method->decl);

  for (auto *property : record.properties)
    frontendAPI.addObjCProperty(
        copy, property->name, property->getterName, property->setterName,
        property->loc, property->availability, property->access,
        property->attributes, property->isOptional, property->decl);

  return copy;
}

void SDKDB::insertEnum(EnumRecord *record, const BinaryInfo *binInfo,
                       StringRef project) {
  auto key = record->name;
  auto result = enumMap.try_emplace(key, record, binInfo, project);
  // If emplace successful.
  if (result.second)
    return;

  auto entry = result.first;
  // Entry has been seen before, report duplicated enum.
  builder->report(diag::warn_sdkdb_duplicated_enum) << key;

  MapEntry<EnumRecord*> current{record, binInfo, project};
  if (current == entry->getValue()) {
    entry->getValue().setPoison();
    return;
  }

  if (current < entry->getValue())
    entry->setValue(current);
}

void SDKDB::insertTypeDef(TypedefRecord *record, const BinaryInfo *binInfo,
                          StringRef project) {
  auto key = record->name;
  auto result = typedefMap.try_emplace(key, record, binInfo, project);
  // If emplace successful.
  if (result.second)
    return;

  auto entry = result.first;
  // Entry has been seen before, report duplicated enum.
  builder->report(diag::warn_sdkdb_duplicated_typedef) << key;

  MapEntry<TypedefRecord *> current{record, binInfo, project};
  if (current == entry->getValue()) {
    entry->getValue().setPoison();
    return;
  }

  if (current < entry->getValue())
    entry->setValue(current);
}

SmallVector<GlobalRecord *, 2> SDKDB::findGlobals(StringRef name) const {
  SmallVector<GlobalRecord *, 2> globals;
  auto global = globalMap.find(name);
  if (global != globalMap.end()) {
    for (auto &g : global->getValue())
      globals.emplace_back(g.getRecord());
  }
  return globals;
}

ObjCInterfaceRecord *SDKDB::findObjCInterface(StringRef name) const {
  auto cls = interfaceMap.find(name);
  if (cls == interfaceMap.end() || cls->getValue().isPoison())
    return nullptr;

  return cls->getValue().getRecord();
}

ObjCCategoryRecord *SDKDB::findObjCCategory(StringRef categoryName,
                                            StringRef clsName) const {
  auto categories = categoryMap.find(clsName);
  if (categories == categoryMap.end())
    return nullptr;

  auto category = categories->getValue().find(categoryName);
  if (category == categories->getValue().end() ||
      category->getValue().isPoison())
    return nullptr;

  return category->getValue().getRecord();
}

EnumRecord *SDKDB::findEnum(StringRef name) const {
  auto e = enumMap.find(name);
  if (e == enumMap.end() || e->getValue().isPoison())
    return nullptr;

  return e->getValue().getRecord();
}

APIRecord *SDKDB::findTypedef(StringRef name) const {
  auto t = typedefMap.find(name);
  if (t == typedefMap.end() || t->getValue().isPoison())
    return nullptr;

  return t->getValue().getRecord();
}

SmallVector<ObjCCategoryRecord *, 4>
SDKDB::findObjCCategoryForClass(StringRef clsName) const {
  SmallVector<ObjCCategoryRecord *, 4> categories;

  auto entry = categoryMap.find(clsName);
  if (entry == categoryMap.end())
    return categories;

  for (auto &c : entry->getValue()) {
    if (c.getValue().isPoison())
      continue;
    categories.emplace_back(c.getValue().getRecord());
  }

  // Sort by category names.
  llvm::sort(categories,
             [](const ObjCCategoryRecord *lhs, const ObjCCategoryRecord *rhs) {
               return lhs->name < rhs->name;
             });

  return categories;
}

ObjCProtocolRecord *SDKDB::findObjCProtocol(StringRef protocolName) const {
  auto protocol = protocolMap.find(protocolName);
  if (protocol == protocolMap.end() || protocol->getValue().isPoison())
    return nullptr;

  return protocol->getValue().getRecord();
}

static ObjCMethodRecord *
findMethodFromContainer(StringRef name, bool isInstanceMethod,
                        const ObjCContainerRecord &record) {
  auto result = find_if(record.methods, [&](ObjCMethodRecord *method) {
    return method->name == name && method->isInstanceMethod == isInstanceMethod;
  });
  if (result == record.methods.end())
    return nullptr;

  return *result;
}

static ObjCPropertyRecord *findProperty(StringRef name,
                                        ObjCContainerRecord &record) {
  auto result = find_if(record.properties, [&](ObjCPropertyRecord *property) {
    return property->name == name;
  });
  if (result == record.properties.end())
    return nullptr;

  return *result;
}

void SDKDB::insertObjCProtocol(ObjCProtocolRecord *record,
                               const BinaryInfo *binInfo, StringRef project) {
  auto key = record->name;
  auto result = protocolMap.try_emplace(key, record, binInfo, project);
  if (result.second)
    return;

  auto entry = result.first;
  // If there exists protocol definition already, check they are the same.
  // no additional methods.
  auto *base = entry->getValue().getRecord();
  for (auto *method : record->methods) {
    if (!findMethod(method->name, method->isInstanceMethod, *base,
                    SDKDB::ObjCProtocol))
      builder->report(diag::warn_sdkdb_conflict_objc_protocol) << key;
  }
  // no additional protocol conformance.
  for (auto protocol : record->protocols) {
    if (find_if(base->protocols.begin(), base->protocols.end(),
                [&](auto baseProtocol) { return protocol == baseProtocol; }) ==
        std::end(base->protocols))
      builder->report(diag::warn_sdkdb_conflict_objc_protocol) << key;
  }

  // If entires are "equal", set the entry to poison since we don't know which
  // to pick so we pick neither.
  MapEntry<ObjCProtocolRecord*> current{record, binInfo, project};
  if (current == entry->getValue()) {
    entry->getValue().setPoison();
    return;
  }

  // Set the value if current value should replace the one in the map.
  if (current < entry->getValue())
    entry->setValue(current);
}

void SDKDB::annotateGlobal(const GlobalRecord *record) {
  auto entry = findGlobals(record->name);
  if (entry.empty()) {
    if (!record->availability._unavailable &&
        record->linkage >= APILinkage::Reexported)
      builder->report(diag::warn_sdkdb_missing_global) << record->name;
    return;
  }

  for (auto *base: entry)
    builder->updateGlobal(*base, *record);
}

void SDKDB::annotateObjCInterface(const ObjCInterfaceRecord *record) {
  auto *base = findObjCInterface(record->name);
  if (!base) {
    if (!record->availability._unavailable)
      builder->report(diag::warn_sdkdb_missing_objc_class) << record->name;
    return;
  }

  builder->updateObjCInterface(*this, *base, *record);

  // Update symbols for objc interfaces.
  if (!record->isExported())
    return;
  auto name = record->name;
  if (isObjC1())
    findAndUpdateGlobal(".objc_class_name_" + name, *record);
  else {
    findAndUpdateGlobal("_OBJC_CLASS_$_" + name, *record);
    findAndUpdateGlobal("_OBJC_METACLASS_$_" + name, *record);
    if (record->hasExceptionAttribute())
      findAndUpdateGlobal("_OBJC_EHTYPE_$_" + name, *record);
  }

  // sync ivars. ivars are not recorded in objc containers. sync ivars with
  // the symbols.
  for (auto *ivar : record->ivars) {
    if (!isObjC1() &&
        (ivar->accessControl ==
             ObjCInstanceVariableRecord::AccessControl::Public ||
         ivar->accessControl ==
             ObjCInstanceVariableRecord::AccessControl::Protected))
      findAndUpdateGlobal("_OBJC_IVAR_$_" + name + "." + ivar->name, *record);
  }
}

void SDKDB::annotateObjCProtocol(const ObjCProtocolRecord *record) {
  auto *base = findObjCProtocol(record->name);
  if (!base) {
    if (!record->availability._unavailable)
      builder->report(diag::warn_sdkdb_missing_objc_protocol) << record->name;
    return;
  }

  builder->updateObjCProtocol(*this, *base, *record);
}

void SDKDB::annotateObjCCategory(const ObjCCategoryRecord *record) {
  if (auto *base = findObjCCategory(record->name, record->interface))
    // For categories, search for category in the binary first.
    builder->updateObjCCategory(*this, *base, *record);
  else if (auto *cls = findObjCInterface(record->interface))
    // If the category is not found from mapping, annotate the base class
    // because linker might merge the category into the base class already.
    builder->updateObjCContainerItems(*this, *cls, *record, SDKDB::ObjCClass);
  else {
    if (!record->availability._unavailable)
      builder->report(diag::warn_sdkdb_missing_objc_category)
          << record->interface << record->name;
    return;
  }

  // sync ivars. ivars are not recorded in objc containers. sync ivars with
  // the symbols.
  for (auto *ivar : record->ivars) {
    if (!isObjC1() &&
        (ivar->accessControl ==
             ObjCInstanceVariableRecord::AccessControl::Public ||
         ivar->accessControl ==
             ObjCInstanceVariableRecord::AccessControl::Protected))
      findAndUpdateGlobal(
          "_OBJC_IVAR_$_" + record->interface + "." + ivar->name, *record);
  }
}

std::vector<const API *> SDKDB::api() const {
  std::vector<const API *> sortedAPIs;
  sortedAPIs.reserve(apiCache.size() + 1);
  for (const auto &apis : apiCache) {
    for (const auto &api : apis.second)
      sortedAPIs.emplace_back(&api);
  }
  if (!frontendAPI.isEmpty())
    sortedAPIs.emplace_back(&frontendAPI);

  llvm::stable_sort(sortedAPIs, [](const API *api1, const API *api2) {
    return *api1 < *api2;
  });
  return sortedAPIs;
}

std::vector<API *> SDKDB::api() {
  std::vector<API *> sortedAPIs;
  sortedAPIs.reserve(apiCache.size() + 1);
  for (auto &apis : apiCache) {
    for (auto &api : apis.second)
      sortedAPIs.emplace_back(&api);
  }
  if (!frontendAPI.isEmpty())
    sortedAPIs.emplace_back(&frontendAPI);

  llvm::stable_sort(sortedAPIs, [](const API *api1, const API *api2) {
    return *api1 < *api2;
  });
  return sortedAPIs;
}

std::vector<const EnumRecord*> SDKDB::getEnumRecords() const {
  std::vector<const EnumRecord*> sortedRecords;
  sortedRecords.reserve(enumMap.size());
  for (const auto &record : enumMap)
    sortedRecords.emplace_back(record.getValue().getRecord());

  llvm::sort(sortedRecords, [](const EnumRecord *r1, const EnumRecord *r2) {
    return r1->name < r2->name;
  });

  return sortedRecords;
}

std::vector<const APIRecord*> SDKDB::getTypedefRecords() const {
  std::vector<const APIRecord*> sortedRecords;
  sortedRecords.reserve(typedefMap.size());
  for (const auto &record : typedefMap)
    sortedRecords.emplace_back(record.getValue().getRecord());

  llvm::sort(sortedRecords, [](const APIRecord *r1, const APIRecord *r2) {
    return r1->name < r2->name;
  });

  return sortedRecords;
}

class APIFinalizer : public APIMutator {
public:
  APIFinalizer(SDKDBBuilder &builder, SDKDB &sdkdb)
      : builder(builder), sdkdb(sdkdb) {}

  void visitObjCProtocol(ObjCProtocolRecord &record) override {
    auto *protocol = sdkdb.findObjCProtocol(record.name);
    if (!protocol)
      return;

    // Patch up protocol list from all the binaries.
    builder.updateAPIRecord(record, *protocol);
    for (auto *method : record.methods) {
      if (auto *result = findMethodFromContainer(
              method->name, method->isInstanceMethod, *protocol))
        builder.updateAPIRecord(*method, *result);
    }
    for (auto *prop : record.properties) {
      if (auto *property = findProperty(prop->name, *protocol))
        builder.updateAPIRecord(*prop, *property);
    }
  }

private:
  SDKDBBuilder &builder;
  SDKDB &sdkdb;
};

template<typename MapEntryIter>
bool isPublicDylibEntry(const MapEntryIter &entry) {
  if (auto *binInfo = entry.getValue().getBinaryInfo()) {
    if (binInfo->fileType == FileType::MachO_Bundle)
      return false;
    return isWithinPublicLocation(binInfo->installName);
  }

  return false;
}

Error SDKDB::finalize() {
  // Finalize SDKDB.
  // Update the access of methods and properties to public if there exists super
  // class/protocol which declares the method/property to be public.
  // 1. Update Protocol methods and properties.
  for (auto &entry : protocolMap) {
    auto *protocol = entry.getValue().getRecord();
    if (entry.getValue().isPoison()) {
      builder->report(diag::warn_sdkdb_poison_entry)
          << ObjCContainerKind::ObjCProtocol << protocol->name;
      continue;
    }

    // Skip private records. Methods in private protocols are also private.
    if (protocol->access != APIAccess::Public)
      continue;

    for (auto *method : protocol->methods) {
      if (method->access != APIAccess::Public &&
          builder->isMaybePublicSelector(method->name))
        method->access = getAccessForObjCMethod(
            method->access, method->name, method->isInstanceMethod, protocol);
    }

    for (auto *property : protocol->properties) {
      if (property->access != APIAccess::Public &&
          builder->isMaybePublicProperty(property->name))
        property->access =
            getAccessForObjCProperty(property->access, property->name,
                                     property->isClassProperty(), protocol);
    }
  }

  // 2. Update Interface methods and properties.
  for (auto &entry : interfaceMap) {
    auto *interface = entry.getValue().getRecord();
    if (entry.getValue().isPoison()) {
      builder->report(diag::warn_sdkdb_poison_entry)
          << ObjCContainerKind::ObjCClass << interface->name;
      continue;
    }

    // Skip private records. Methods in private interfaces are also private.
    if (interface->access != APIAccess::Public)
      continue;

    for (auto *method : interface->methods) {
      if (method->access != APIAccess::Public &&
          builder->isMaybePublicSelector(method->name))
        method->access = getAccessForObjCMethod(
            method->access, method->name, method->isInstanceMethod, interface);
    }

    for (auto *property : interface->properties) {
      if (property->access != APIAccess::Public &&
          builder->isMaybePublicProperty(property->name))
        property->access =
            getAccessForObjCProperty(property->access, property->name,
                                     property->isClassProperty(), interface);
    }
  }

  // 3. Update Category methods and properties.
  for (auto &catEntry : categoryMap) {
    auto categories = catEntry.getValue();
    ObjCInterfaceRecord *interface = findObjCInterface(catEntry.getKey());
    for (auto &entry : categories) {
      auto *category = entry.getValue().getRecord();
      if (entry.getValue().isPoison()) {
        std::string diagName =
            category->interface.str() + "(" + category->name.str() + ")";
        builder->report(diag::warn_sdkdb_poison_entry)
            << ObjCContainerKind::ObjCCategory << diagName;
        continue;
      }

      // Skip private records. Methods in private categories are also private.
      if (category->access != APIAccess::Public)
        continue;

      for (auto *method : category->methods) {
        if (method->access != APIAccess::Public &&
          builder->isMaybePublicSelector(method->name)) {
          method->access =
              getAccessForObjCMethod(method->access, method->name,
                                     method->isInstanceMethod, category);
          // Look at base class if exists.
          if (method->access != APIAccess::Public && interface)
            method->access =
                getAccessForObjCMethod(method->access, method->name,
                                       method->isInstanceMethod, interface);
        }
      }

      for (auto *property : category->properties) {
        if (property->access != APIAccess::Public &&
            builder->isMaybePublicProperty(property->name)) {
          property->access =
              getAccessForObjCProperty(property->access, property->name,
                                       property->isClassProperty(), category);
          // Look at base class if exists.
          if (property->access != APIAccess::Public && interface)
            property->access = getAccessForObjCProperty(
                property->access, property->name, property->isClassProperty(),
                interface);
        }
      }
    }
  }

  // Perform SDKDB finalize.
  // 1. Fixup all the protocols in the SDKDB.
  // 2. Apply dylib promote to promote to public list to the entry.
  for (auto *api: api()) {
    if (!api->hasBinaryInfo() ||
        api->getBinaryInfo().fileType == FileType::MachO_Bundle)
      continue;

    APIFinalizer updater(*builder, *this);
    api->visit(updater);
  }

  return Error::success();
}

ObjCInterfaceRecord *
SDKDB::getSuperclass(const ObjCInterfaceRecord *record) const {
  if (record->superClass.empty())
    return nullptr;

  return findObjCInterface(record->superClass);
}

ObjCMethodRecord const *
SDKDB::resolveMethod(StringRef name, bool isInstanceMethod,
                     const ObjCInterfaceRecord &interface) const {
  // Resolve method in the current interface.
  if (auto *method = findMethodFromContainer(name, isInstanceMethod, interface))
    return method;

  // Resolve method in categories.
  for (auto *category : findObjCCategoryForClass(interface.name))
    if (auto *method =
            findMethodFromContainer(name, isInstanceMethod, *category))
      return method;

  // Resolve method in the super class.
  if (auto *base = getSuperclass(&interface))
    if (auto *method = resolveMethod(name, isInstanceMethod, *base))
      return method;

  // No need to look in the protocols here because we are not trying to
  // propagate access as in finalizing the SDKDB, but looking for the API
  // surface during diffing so we are trying to find an implementation. And we
  // should be guaranteed to find one (if the API surface is not completely
  // removed) in categories and the class hierarchy even if the method is
  // declared in a protocol.

  return nullptr;
}

APIAccess SDKDB::getAccessForObjCMethod(APIAccess access, StringRef name,
                                        bool isInstanceMethod,
                                        ObjCContainerRecord *container) {
  // check current container for the access.
  const auto methodIt =
      find_if(container->methods,
              [name, isInstanceMethod](const ObjCMethodRecord *method) {
                return method->name == name &&
                       method->isInstanceMethod == isInstanceMethod;
              });
  if (methodIt != container->methods.end() && (*methodIt)->access > access)
    access = (*methodIt)->access;

  if (access == APIAccess::Public)
    return access; // return since it is already public.

  // walk protocol hierarchy.
  for (auto protocolSymbol : container->protocols) {
    if (auto *protocol = findObjCProtocol(protocolSymbol))
      access = getAccessForObjCMethod(access, name, isInstanceMethod, protocol);
    if (access == APIAccess::Public)
      return access;
  }

  return access;
}

APIAccess SDKDB::getAccessForObjCMethod(APIAccess access, StringRef name,
                                        bool isInstanceMethod,
                                        ObjCInterfaceRecord *interface) {
  // walk the common container part first.
  access = getAccessForObjCMethod(access, name, isInstanceMethod,
                                  (ObjCContainerRecord *)interface);
  if (access == APIAccess::Public)
    return access; // return since it is already public.

  // check super class.
  if (auto *super = getSuperclass(interface))
    access = getAccessForObjCMethod(access, name, isInstanceMethod, super);
  if (access == APIAccess::Public)
    return access; // return since it is already public.

  // check categories.
  for (auto &category : findObjCCategoryForClass(interface->name)) {
    // recursively iterate protocols and methods of the category.
    access = getAccessForObjCMethod(access, name, isInstanceMethod, category);
    if (access == APIAccess::Public)
      return access;
  }

  return access;
}

APIAccess SDKDB::getAccessForObjCProperty(APIAccess access, StringRef name,
                                          bool isClassProperty,
                                          ObjCContainerRecord *container) {
  // check current container for the access.
  const auto propertyIt =
      find_if(container->properties,
              [name, isClassProperty](const ObjCPropertyRecord *property) {
                return property->name == name &&
                       property->isClassProperty() == isClassProperty;
              });
  if (propertyIt != container->properties.end() &&
      (*propertyIt)->access > access)
    access = (*propertyIt)->access;

  if (access == APIAccess::Public)
    return access; // return since it is already public.

  // walk protocol hierarchy.
  for (auto protocolSymbol : container->protocols) {
    if (auto *protocol = findObjCProtocol(protocolSymbol))
      access =
          getAccessForObjCProperty(access, name, isClassProperty, protocol);
    if (access == APIAccess::Public)
      return access;
  }

  return access;
}

APIAccess SDKDB::getAccessForObjCProperty(APIAccess access, StringRef name,
                                          bool isClassProperty,
                                          ObjCInterfaceRecord *interface) {
  // walk the common container part first.
  access = getAccessForObjCProperty(access, name, isClassProperty,
                                    (ObjCContainerRecord *)interface);
  if (access == APIAccess::Public)
    return access; // return since it is already public.

  // check super class.
  if (auto *super = getSuperclass(interface))
    access = getAccessForObjCProperty(access, name, isClassProperty, super);
  if (access == APIAccess::Public)
    return access; // return since it is already public.

  // check category. category should not overwrite the properties from interface
  // but it can introduce new procotol conformance.
  for (auto &category : findObjCCategoryForClass(interface->name)) {
    for (auto protocolSymbol : category->protocols) {
      if (auto *protocol = findObjCProtocol(protocolSymbol))
        access =
            getAccessForObjCProperty(access, name, isClassProperty, protocol);
      if (access == APIAccess::Public)
        return access;
    }
  }

  return access;
}

bool SDKDB::findAndUpdateGlobal(Twine name, const APIRecord &record) {
  auto syms = findGlobals(name.str());
  if (syms.empty())
    return false;
  for (auto *value : syms)
    builder->updateAPIRecord(*value, record);
  return true;
}

ObjCMethodRecord *SDKDB::findMethod(StringRef name, bool isInstanceMethod,
                                    ObjCContainerRecord &record,
                                    ObjCContainerKind kind,
                                    StringRef fallbackInterfaceName) {
  auto *result = findMethodFromContainer(name, isInstanceMethod, record);
  if (result)
    return result;

  if (kind == SDKDB::ObjCCategory) {
    // Find the base class and recursively search base class.
    if (auto *cls = findObjCInterface(fallbackInterfaceName))
      return findMethod(name, isInstanceMethod, *cls, SDKDB::ObjCClass);
  } else if (kind == SDKDB::ObjCClass) {
    // Only do global search for objc classes.
    for (auto &category : findObjCCategoryForClass(record.name)) {
      result = findMethodFromContainer(name, isInstanceMethod, *category);
      if (result)
        return result;
    }
  }

  return nullptr;
}

Error SDKDBBuilder::addBinaryAPI(API &&api) {
  auto &db = getSDKDBForTarget(api.getTriple());
  auto &current = db.recordAPI(std::move(api));

  // No need to put bundle into lookup map.
  if (api.getBinaryInfo().fileType == FileType::MachO_Bundle)
    return Error::success();

  // Build lookup map.
  LookupMapBuilder builder(db, current);
  current.visit(builder);

  return Error::success();
}

Error SDKDBBuilder::addHeaderAPI(const API &api) {
  auto &db = getSDKDBForTarget(api.getTriple());
  APIAnnotator annotator(db, api.getProjectName());
  api.visit(annotator);

  return Error::success();
}

void SDKDBBuilder::updateAPIRecord(APIRecord &base, const APIRecord &record) {
  // update the APLoc if the original location is invalid.
  // also moving away from clang::PresumedLoc because it is not in the same
  // context.
  if (base.loc.isInvalid() && preserveLocation())
    base.loc = APILoc(record.loc.getFilename(), record.loc.getLine(),
                      record.loc.getColumn());

  if (base.availability.isDefault())
    base.availability = record.availability;
  else if (base.availability != record.availability &&
           !record.availability.isDefault()) {
    diag.report(diag::warn_sdkdb_conflict_availability)
        << base.name << base.availability.str() << record.availability.str();
    // Pick the newer availability to be consistent.
    if (base.availability < record.availability)
      base.availability = record.availability;
  }

  if (base.access < record.access)
    base.access = record.access;

  // TODO: update USR after finding a way to properly copy the string data.
}

void SDKDBBuilder::updateObjCContainerItems(SDKDB &sdkdb,
                                            ObjCContainerRecord &base,
                                            const ObjCContainerRecord &record,
                                            SDKDB::ObjCContainerKind kind,
                                            StringRef fallbackInterfaceName) {
  auto handleMissingMethod = [&](StringRef selectorName,
                                 APIRecord &selectorInfo, bool isInstanceMethod,
                                 bool isOptional, bool isDynamic) {
    diag.report(diag::warn_sdkdb_missing_objc_method)
        << selectorName << kind << record.name;

    APILoc location;
    if (preserveLocation())
      location = selectorInfo.loc;
    auto *m = ObjCMethodRecord::create(
        sdkdb.danglingAPIAllocator, selectorName, location,
        selectorInfo.availability, selectorInfo.access, isInstanceMethod,
        isOptional, isDynamic, selectorInfo.decl);
    base.methods.push_back(m);
  };

  for (auto *method : record.methods) {
    if (auto *baseMethod =
            sdkdb.findMethod(method->name, method->isInstanceMethod, base, kind,
                             fallbackInterfaceName))
      updateAPIRecord(*baseMethod, *method);
    else if (!method->availability._unavailable)
      handleMissingMethod(method->name, *method, method->isInstanceMethod,
                          method->isOptional, method->isDynamic);

    if (method->access == APIAccess::Public)
      maybePublicSelector.insert(method->name);
  }

  // base does not have properties because it is coming from binary interface.
  // use the property in header to annotate getter and setter inside base.
  for (auto *prop : record.properties) {
    bool available = !prop->availability._unavailable;
    // dynamic is only known to implementation. Default to false.
    bool isDynamic = false;
    // use the class property attributes from header.
    bool isClassProperty = prop->isClassProperty();
    if (auto *property = findProperty(prop->name, base)) {
      updateAPIRecord(*property, *prop);
      isDynamic = property->isDynamic();
    } else if (available)
      diag.report(diag::warn_sdkdb_missing_objc_property)
          << prop->name << kind << base.name;

    if (prop->access == APIAccess::Public)
      maybePublicProperty.insert(prop->name);

    if (auto *baseMethod = sdkdb.findMethod(prop->getterName,
                                            /*instanceMethod*/ !isClassProperty,
                                            base, kind, fallbackInterfaceName))
      updateAPIRecord(*baseMethod, *prop);
    // ignore missing getter for optional, unavailable or dynamic properties
    else if (!prop->isOptional && available && !isDynamic)
      handleMissingMethod(prop->getterName, *prop,
                          /* isInstanceMethod */ !isClassProperty,
                          /* isOptional */ false, /* isDynamic */ false);

    if (prop->access == APIAccess::Public)
      maybePublicSelector.insert(prop->getterName);

    if (prop->isReadOnly())
      continue;

    if (auto *baseMethod = sdkdb.findMethod(prop->setterName,
                                            /*instanceMethod*/ !isClassProperty,
                                            base, kind, fallbackInterfaceName))
      updateAPIRecord(*baseMethod, *prop);
    // ignore missing setter for optional, unavailable or dynamic properties
    else if (!prop->isOptional && available && !isDynamic)
      handleMissingMethod(prop->setterName, *prop,
                          /* isInstanceMethod */ !isClassProperty,
                          /* isOptional */ false, /* isDynamic */ false);

    if (prop->access == APIAccess::Public)
      maybePublicSelector.insert(prop->setterName);
  }

  // TODO: Now we just sync all the conformed protocol from header to API.
  // Need to teach MachOReader to read them from binary in the future.
  for (auto protocol : record.protocols) {
    if (find_if(base.protocols.begin(), base.protocols.end(),
                [&](auto baseProtocol) { return protocol == baseProtocol; }) ==
        std::end(base.protocols))
      base.protocols.push_back(protocol);
  };
}

void SDKDBBuilder::updateGlobal(GlobalRecord &base,
                                const GlobalRecord &record) {
  updateAPIRecord(base, record);

  if (base.kind == GVKind::Unknown)
    base.kind = record.kind;
  else if (base.kind != record.kind)
    diag.report(diag::warn_sdkdb_conflict_gvkind) << base.name;
}

void SDKDBBuilder::updateObjCInterface(SDKDB &sdkdb, ObjCInterfaceRecord &base,
                                       const ObjCInterfaceRecord &record) {
  updateAPIRecord(base, record);
  updateObjCContainerItems(sdkdb, base, record, SDKDB::ObjCClass);
  if (base.superClass != record.superClass)
    diag.report(diag::warn_sdkdb_conflict_superclass)
        << base.name << base.superClass << record.superClass;
  if (record.hasExceptionAttribute())
    base.updateLinkageForSymbols(
        ObjCIFSymbolKind::EHType,
        record.getLinkageForSymbol(ObjCIFSymbolKind::EHType));
}

void SDKDBBuilder::updateObjCCategory(SDKDB &sdkdb, ObjCCategoryRecord &base,
                                      const ObjCCategoryRecord &record) {
  updateAPIRecord(base, record);
  updateObjCContainerItems(sdkdb, base, record, SDKDB::ObjCCategory,
                           record.interface);
}

void SDKDBBuilder::updateObjCProtocol(SDKDB &sdkdb, ObjCProtocolRecord &base,
                                      const ObjCProtocolRecord &record) {
  updateAPIRecord(base, record);
  updateObjCContainerItems(sdkdb, base, record, SDKDB::ObjCProtocol);
}

SDKDB &SDKDBBuilder::getSDKDBForTarget(const Triple &triple) {
  for (auto &entry : databases) {
    if (SDKDB::areCompatibleTargets(entry.getTargetTriple(), triple))
      return entry;
  }

  Triple key(triple);
  // clear the os version so it is not subjected to the first version it tries
  // to insert into SDKDB.
  key.setOSName(Triple::getOSTypeName(triple.getOS()));
  databases.emplace_back(key, this);
  return databases.back();
}

Error SDKDBBuilder::finalize() {
  for (auto &entry : databases) {
    if (auto err = entry.finalize())
      return err;
  }

  // sort projectWithError
  llvm::sort(projectWithError);
  return Error::success();
}

llvm::Error SDKDBBuilder::parse(StringRef JSON) {
  auto inputValue = json::parse(JSON);
  if (!inputValue)
    return inputValue.takeError();

  auto *root = inputValue->getAsObject();
  if (!root)
    return make_error<APIJSONError>("SDKDB is not a JSON Object");

  for (auto &target : *root) {
    const StringRef key = target.first;
    if (key == "public")
      continue;
    auto triple = Triple(key);
    auto payload = target.second.getAsArray();
    if (!payload)
      return make_error<APIJSONError>("Target Payload is not a JSON Array");

    for (auto &value : *payload) {
      auto obj = value.getAsObject();
      if (!obj)
        return make_error<APIJSONError>(
            "SDKDB doesn't include correct API format");
      auto api = APIJSONSerializer::parse(obj, isPublicOnly(), &triple);
      if (!api)
        return api.takeError();

      if (auto err = addBinaryAPI(std::move(*api)))
        return err;
    }
  }

  return Error::success();
}

std::vector<const SDKDB*> SDKDBBuilder::getDatabases() const {
  std::vector<Triple> sortedTriples;
  for (auto &entry : databases)
    sortedTriples.emplace_back(entry.getTargetTriple());
  llvm::sort(sortedTriples, [](const Triple &t1, const Triple &t2) {
    return t1.str() < t2.str();
  });

  std::vector<const SDKDB*> sortedDatabases;
  for (auto &target : sortedTriples) {
    const auto *entry = llvm::find_if(databases, [&](const SDKDB &db) {
      return db.getTargetTriple().str() == target.str();
    });
    assert(entry != databases.end() && "Target must exists.");
    sortedDatabases.emplace_back(entry);
  }
  return sortedDatabases;
}

void SDKDBBuilder::serialize(raw_ostream &os, bool compact) const {
  json::Object root;
  APIJSONOption serializeOpts = {
      compact,
      !hasUUID(),
      /*no target*/ true,
      /*external only*/ true,
      isPublicOnly(),
      /*ignore line and col*/ true,
  };
  for (auto *entry : getDatabases()) {
    auto target = entry->getTargetTriple();
    json::Array apiList;
    for (auto *api : entry->api()) {
      if (api->isEmpty())
        continue;
      APIJSONSerializer serializer(*api, serializeOpts);
      apiList.emplace_back(serializer.getJSONObject());
    }
    root[target.str()] = std::move(apiList);
  }
  if (isPublicOnly())
    root["public"] = true;

  json::Array projects;
  for (auto &proj : projectWithError)
    projects.push_back(proj);
  if (!projects.empty())
    root["projectWithError"] = std::move(projects);

  if (compact)
    os << formatv("{0}", json::Value(std::move(root))) << "\n";
  else
    os << formatv("{0:2}", json::Value(std::move(root))) << "\n";
}

template <typename LookupMapTy>
inline std::set<StringRef> allKeysFromMaps(const LookupMapTy &base,
                                           const LookupMapTy &test) {
  std::set<StringRef> keys;
  keys.insert(base.keys().begin(), base.keys().end());
  keys.insert(test.keys().begin(), test.keys().end());
  return keys;
}

template <typename ContainerTy>
inline std::set<StringRef> allKeysFromContainer(const ContainerTy &base,
                                                const ContainerTy &test) {
  std::set<StringRef> keys;
  for (auto &m : base)
    keys.insert(m->name);
  for (auto &m : test)
    keys.insert(m->name);
  return keys;
}

template <typename MethodsTy>
inline std::set<std::pair<StringRef, bool>> allMethods(const MethodsTy &base,
                                                       const MethodsTy &test) {
  std::set<std::pair<StringRef, bool>> methods;
  for (auto &method : base)
    methods.emplace(method->name, method->isInstanceMethod);
  for (auto &method : test)
    methods.emplace(method->name, method->isInstanceMethod);
  return methods;
}

template <typename LookupMapTy>
inline std::set<std::pair<StringRef, StringRef>>
allKeyPairsFromNestedMaps(const LookupMapTy &base, const LookupMapTy &test) {
  std::set<std::pair<StringRef, StringRef>> keys;
  for (auto &k1 : base) {
    for (auto &k2 : k1.getValue())
      keys.emplace(k1.getKey(), k2.getKey());
  }
  for (auto &k1 : test) {
    for (auto &k2 : k1.getValue())
      keys.emplace(k1.getKey(), k2.getKey());
  }
  return keys;
}

static bool checkAPIRecord(const APIRecord &record, const APIRecord &base,
                           std::function<void(StringRef)> handler) {
  assert(record.name == base.name && "record names are not equal");
  if (base.access != APIAccess::Public)
    return true; // allow regression for non-public API.

  if (record.access != APIAccess::Public) {
    handler("api access regression");
    return false;
  }

  if (record.linkage != base.linkage) {
    handler("linkage type is not equal");
    return false;
  }

  if (!base.availability.isDefault() &&
      record.availability != base.availability) {
    handler("availability changed");
    return false;
  }
  return true;
}

static bool checkObjCContainer(
    const ObjCContainerRecord &test, const ObjCContainerRecord &base,
    std::function<void(StringRef)> handlerForContainer,
    std::function<std::pair<ObjCMethodRecord const *, ObjCMethodRecord const *>(
        StringRef name, bool isInstanceMethod)>
        resolveMethod,
    std::function<void(const ObjCMethodRecord *)> handlerForNewMethod,
    std::function<void(const ObjCMethodRecord *)> handlerForMissingMethod,
    std::function<void(const ObjCMethodRecord *, StringRef)>
        handlerForRegressedMethod) {
  bool regression = checkAPIRecord(test, base, handlerForContainer);

  for (auto [name, isInstanceMethod] : allMethods(base.methods, test.methods)) {
    auto [bm, tm] = resolveMethod(name, isInstanceMethod);

    // regression.
    if (!tm) {
      assert(bm && "baseline should exist");
      // ignore the private APIs.
      auto *missing = bm;
      if (missing->access != APIAccess::Public)
        continue;

      handlerForMissingMethod(missing);
      continue;
    }

    // new API.
    if (!bm) {
      assert(tm && "test version should exist");
      auto *newMethod = tm;
      if (newMethod->access != APIAccess::Public)
        continue;

      handlerForNewMethod(newMethod);
      continue;
    }

    // new API case 2.
    if (bm->access != APIAccess::Public && tm->access == APIAccess::Public) {
      handlerForNewMethod(tm);
      continue;
    }

    // FIXME: Cannot directly use `tm` in the handler now:
    // Captured structured bindings are a C++20 extension
    auto *testMethod = tm;
    checkAPIRecord(*tm, *bm, [&](StringRef error) {
      handlerForRegressedMethod(testMethod, error);
    });
  }

  return regression;
}

void SDKDB::buildLookupTables() {
  assert(globalMap.empty() && interfaceMap.empty() && categoryMap.empty() &&
         protocolMap.empty() && "Lookup table should not be built yet");
  for (auto *api : api()) {
    LookupMapBuilder builder(*this, *api);
    api->visit(builder);
  }

  // sort the global entries.
  for (auto &entry : globalMap)
    llvm::sort(entry.second);
}

bool SDKDB::shouldDiagnoseProject(StringRef projectName) const {
  return !llvm::is_contained(builder->getProjectWithError(), projectName);
}

bool SDKDB::isReexportedBy(StringRef installName,
                           StringRef reexportedBy) const {
  // Don't consider a library is reexported by itself.
  SmallVector<StringRef, 8> frontier = {installName};
  while (!frontier.empty()) {
    for (const StringRef parent :
         reexportGraph.lookup(frontier.pop_back_val())) {
      if (parent == reexportedBy)
        return true;
      frontier.emplace_back(parent);
    }
  }

  return false;
}

bool SDKDB::isPubliclyExported(StringRef installName) const {
  if (isPublicDylib(installName))
    return true;

  SmallVector<StringRef, 8> frontier = {installName};
  while (!frontier.empty()) {
    for (const StringRef parent :
         reexportGraph.lookup(frontier.pop_back_val())) {
      if (isPublicDylib(parent))
        return true;
      frontier.emplace_back(parent);
    }
  }

  return false;
}

bool SDKDB::shouldDiagnoseLibrary(StringRef installName) const {
  if (!isPubliclyExported(installName))
    return false;
  if (const auto *compareConfig = builder->getCompareConfigFileReader())
    return compareConfig->ignoredLibraries().count(installName) == 0;
  return true;
}

template <typename MapEntryIter>
bool shouldDiagnoseEntry(const MapEntryIter &entry, const SDKDB &db) {
  // Skip all symbols that have public access but are not in an SDK location
  // (public or private).
  // FIXME: This is a workaround to make diffs less noisy.
  // Fix this by properly setting symbol APIAccess.
  if (entry.getRecord()->access != APIAccess::Public)
    return false;

  // Skip diffs for failed projects.
  // Projects with scanning errors are already reported for investigation.
  // Diffing failed projects does not provide valuable information, only noise.
  if (!db.shouldDiagnoseProject(entry.getProjectName()))
    return false;

  if (auto *binInfo = entry.getBinaryInfo()) {
    if (binInfo->fileType == FileType::MachO_Bundle)
      return false;
    return db.shouldDiagnoseLibrary(binInfo->installName);
  }

  return true;
}

void SDKDB::diagnoseDifferences(const SDKDB &baseline) const {
  // Diff APIs by diffing the global lookup table to see if there are
  // changes to the APIs.
  // 0. check install name differences.
  StringSet<> missingLibraries, newLibraries;

  for (auto installName :
       allKeysFromMaps(baseline.installNames, installNames)) {
    auto base = baseline.installNames.find(installName);
    auto test = installNames.find(installName);

    if (test == installNames.end()) {
      assert(base != baseline.installNames.end() && "baseline should exist");
      // Skip install names that are not part of the public SDK.
      // But still record the install name to check for moved libraries.
      missingLibraries.insert(installName);
      if (!shouldDiagnoseLibrary(installName) ||
          isExpectedChange(
              {ChangeType::Remove, EntryType::Library, installName}))
        continue;

      builder->report(diag::err_sdkdb_missing_api)
          << /*{frontend API|library}*/ 1
          << baseline.installNames.lookup(installName) << installName
          << getTargetTriple().str();
    }

    if (base == baseline.installNames.end()) {
      assert(test != installNames.end() && "test version should exist");
      // Skip install names that are not part of the public SDK.
      // But still record the install name to check for moved libraries.
      newLibraries.insert(installName);
      if (!shouldDiagnoseLibrary(installName) ||
          isExpectedChange({ChangeType::Add, EntryType::Library, installName}))
        continue;

      builder->report(diag::warn_sdkdb_new_api)
          << /*{frontend API|library}*/ 1 << installNames.lookup(installName)
          << installName << getTargetTriple().str();
    }
  }

  auto isKnownObjCSymbol = [&](StringRef symbolName) -> bool {
    auto [name, symbolType, _] = parseSymbol(symbolName);
    switch (symbolType) {
    case EncodeKind::GlobalSymbol:
      return false;
    case EncodeKind::ObjectiveCClass:
    case EncodeKind::ObjectiveCClassEHType:
      return baseline.interfaceMap.contains(name) ||
             interfaceMap.contains(name);
    case EncodeKind::ObjectiveCInstanceVariable: {
      auto [superClass, ivar] = name.split('.');
      return baseline.interfaceMap.contains(superClass) ||
             interfaceMap.contains(superClass) ||
             baseline.categoryMap.contains(superClass) ||
             categoryMap.contains(superClass);
    }
    }
    return false;
  };

  // 1. check globals.
  for (auto name : allKeysFromMaps(baseline.globalMap, globalMap)) {
    // Skip diagnosing linker directives to reduce noise.
    // Special linker symbols like `$ld$install_name` or `$ld$previous` are not
    // APIs/SPIs that fall into the scope of SDKDB diffing. They provide symbol
    // movement information that could be used to produce better diagnostics
    // for the moved APIs, but there's no need to report changes of the linker
    // directives themselves.
    if (name.startswith("$ld$"))
      continue;

    // FIXME: This is only a temporary workaround to further rinsing the
    // diagnostics to show the real issues we need to investigate with
    // priority. This hides real issues for Swift symbols but we also currently
    // allowlist all Swift symbols in SDKDB.
    // Remove this filter with rdar://109905325
    if (name.startswith("_$s"))
      continue;

    // If the global symbol is accounted for in a different classification (e.g.
    // classes or ivars), defer any potential differences to those container
    // comparisons.
    if (isKnownObjCSymbol(name))
      continue;

    auto base = baseline.globalMap.find(name);
    auto test = globalMap.find(name);

    // regression.
    if (test == globalMap.end()) {
      assert(base != baseline.globalMap.end() && "baseline should exist");
      for (auto missing : base->second) {
        // ignore the private APIs and missing libraries.
        if (!shouldDiagnoseEntry(missing, *this) ||
            missingLibraries.contains(missing.getInstallName()) ||
            isExpectedChange({ChangeType::Remove, EntryType::Global, name,
                              missing.getInstallName()}))
          continue;

        builder->report(diag::err_sdkdb_missing_global)
            << (unsigned)missing.getRecord()->kind << name
            << missing.getInstallName() << getTargetTriple().str();
      }
      continue;
    }

    // new API.
    if (base == baseline.globalMap.end()) {
      assert(test != globalMap.end() && "test version should exist");
      for (auto missing : test->second) {
        // ignore the private APIs and new libraries.
        if (!shouldDiagnoseEntry(missing, *this) ||
            newLibraries.contains(missing.getInstallName()) ||
            isExpectedChange({ChangeType::Add, EntryType::Global, name,
                              missing.getInstallName()}))
          continue;

        builder->report(diag::warn_sdkdb_new_global)
            << (unsigned)missing.getRecord()->kind << name
            << missing.getInstallName() << getTargetTriple().str();
      }
      continue;
    }

    auto isMatchingGlobalEntry = [&](const auto &baseEntry,
                                     const auto &testEntry) -> bool {
      // Considering project/library moves, two entries are matching if:
      // They have the same binary info (install name),
      return *baseEntry.getBinaryInfo() == *testEntry.getBinaryInfo() ||
             // or the library for the baseline entry is now reexporting the
             // library for the new entry,
             isReexportedBy(testEntry.getInstallName(),
                            /*reexportedBy*/ baseEntry.getInstallName()) ||
             // or the library for the new entry was reexporting the library for
             // the baseline entry.
             baseline.isReexportedBy(
                 baseEntry.getInstallName(),
                 /*reexportedBy*/ testEntry.getInstallName());
    };

    auto checkMatchingGlobalEntry = [&](const auto &baseEntry,
                                        const auto &testEntry) {
      // First, check if this is a newly added API.
      if (baseEntry.getRecord()->access != APIAccess::Public &&
          shouldDiagnoseEntry(testEntry, *this) &&
          !isExpectedChange({ChangeType::Add, EntryType::Global, name,
                             testEntry.getInstallName()})) {
        builder->report(diag::warn_sdkdb_new_global)
            << (unsigned)testEntry.getRecord()->kind << name
            << testEntry.getInstallName() << getTargetTriple().str();
      } else if (shouldDiagnoseEntry(baseEntry, *this)) {
        // Second, check annotation on the matching entries.
        auto *record = baseEntry.getRecord();
        auto baseInstallName = baseEntry.getInstallName();
        auto testInstallName = testEntry.getInstallName();
        checkAPIRecord(*testEntry.getRecord(), *record, [&](StringRef error) {
          if (!isExpectedChange({ChangeType::UpdateAccess, EntryType::Global,
                                 name, baseInstallName})) {
            builder->report(diag::err_sdkdb_global_regression)
                << name << baseInstallName << getTargetTriple().str() << error;
            if (baseInstallName != testInstallName)
              builder->report(diag::note_sdkdb_moved_symbol)
                  << baseInstallName << testInstallName;
          }
        });
      }
    };

    // Compare entries. All entries are sorted.
    unsigned baseIdx = 0, testIdx = 0;
    while (baseIdx < base->second.size()) {
      auto baseEntry = base->second[baseIdx];
      bool matchFound = false;
      // Advance testIdx if testEntry is smaller.
      while (testIdx < test->second.size() &&
             test->second[testIdx] < baseEntry) {
        auto testEntry = test->second[testIdx];
        // Two entries with the same binary info but from different projects
        // are considered the same symbol moved from one project to another.
        // Diagnose as matching entries.
        if (isMatchingGlobalEntry(baseEntry, testEntry)) {
          checkMatchingGlobalEntry(baseEntry, testEntry);
          // We've found a matching pair, which means there won't be any other
          // global record with the same name and install name in either base
          // or test. Advance both pointers and break as we are done here.
          matchFound = true;
          ++baseIdx;
          ++testIdx;
          break;
        }

        if (shouldDiagnoseEntry(testEntry, *this) &&
            !newLibraries.contains(testEntry.getInstallName()) &&
            !isExpectedChange({ChangeType::Add, EntryType::Global, name,
                               testEntry.getInstallName()})) {
          // new APIs case 2.
          builder->report(diag::warn_sdkdb_new_global)
              << (unsigned)testEntry.getRecord()->kind << name
              << testEntry.getInstallName() << getTargetTriple().str();
        }
        ++testIdx;
      }

      // If we've found a match at this point, we are done with this base
      // entry. Continue to the next.
      if (matchFound)
        continue;

      // If there isn't a matching entry in test, report regressions.
      if (testIdx >= test->second.size() ||
          baseEntry != test->second[testIdx]) {
        if (testIdx < test->second.size() &&
            isMatchingGlobalEntry(baseEntry, test->second[testIdx])) {
          checkMatchingGlobalEntry(baseEntry, test->second[testIdx]);
          // We've found a matching pair, which means there won't be any other
          // global record with the same name and install name in either base
          // or test. Advance both pointers and continue as we are done here.
          ++testIdx;
          ++baseIdx;
          continue;
        }

        if (shouldDiagnoseEntry(baseEntry, *this) &&
            !missingLibraries.contains(baseEntry.getInstallName()) &&
            !isExpectedChange({ChangeType::Remove, EntryType::Global, name,
                               baseEntry.getInstallName()})) {
          builder->report(diag::err_sdkdb_missing_global)
              << (unsigned)baseEntry.getRecord()->kind << name
              << baseEntry.getInstallName() << getTargetTriple().str();
        }
        ++baseIdx;
        continue;
      }

      // Diff two matching entries (same binary info, same project).
      auto &testEntry = test->second[testIdx];
      checkMatchingGlobalEntry(baseEntry, testEntry);
      ++baseIdx;
      ++testIdx;
    }

    // Check remaining test entries.
    auto &lastBaseEntry = base->second.back();
    while (testIdx < test->second.size()) {
      auto &testEntry = test->second[testIdx];
      // Two entries with the same binary info but from different projects
      // are considered the same symbol moved from one project to another.
      // Diagnose as matching entries.
      if (isMatchingGlobalEntry(lastBaseEntry, testEntry)) {
        checkMatchingGlobalEntry(lastBaseEntry, testEntry);
      } else if (shouldDiagnoseEntry(testEntry, *this) &&
                 !newLibraries.contains(testEntry.getInstallName()) &&
                 !isExpectedChange({ChangeType::Add, EntryType::Global, name,
                                    testEntry.getInstallName()})) {
        // new API.
        builder->report(diag::warn_sdkdb_new_global)
            << (unsigned)testEntry.getRecord()->kind << name
            << testEntry.getInstallName() << getTargetTriple().str();
      }
      ++testIdx;
    }
  }

  auto isMovedLibrary = [&](const auto &base, const auto &test) -> bool {
    // If the base symbol comes from a missing library *and* the test symbol
    // comes from a new one, it's a moved library and we've already emitted
    // at least one library-level diagnostic if it was/is a public dylib.
    // Skip individual symbol diagnostics.
    //
    // Otherwise, the symbol is either:
    //   1. staying within the same library (install name), or
    //   2. splited from an existing library to a new one, or
    //   3. merged from a removed libary into an existing one, or
    //   4. moved from one existing library into another.
    // For these cases, diagnose the symbol as usual.
    return (missingLibraries.contains(base.getInstallName()) &&
            newLibraries.contains(test.getInstallName()));
  };

  // 2. check objc classes.
  for (auto name : allKeysFromMaps(baseline.interfaceMap, interfaceMap)) {
    auto base = baseline.interfaceMap.find(name);
    auto test = interfaceMap.find(name);
    // regression.
    if (test == interfaceMap.end()) {
      assert(base != baseline.interfaceMap.end() && "baseline should exist");
      // ignore the private APIs and missing libraries.
      auto missing = base->second;
      if (!shouldDiagnoseEntry(missing, *this) ||
          missingLibraries.contains(missing.getInstallName()) ||
          isExpectedChange({ChangeType::Remove, EntryType::Interface, name,
                            missing.getInstallName()}))
        continue;

      builder->report(diag::err_sdkdb_missing_objc)
          << 0 << name << missing.getInstallName() << getTargetTriple().str();
      continue;
    }

    // new API.
    if (base == baseline.interfaceMap.end()) {
      assert(test != interfaceMap.end() && "test version should exist");
      auto missing = test->second;
      // ignore the private APIs and new libraries.
      if (!shouldDiagnoseEntry(missing, *this) ||
          newLibraries.contains(missing.getInstallName()) ||
          isExpectedChange({ChangeType::Add, EntryType::Interface, name,
                            missing.getInstallName()}))
        continue;
      builder->report(diag::warn_sdkdb_new_objc)
          << 0 << name << missing.getInstallName() << getTargetTriple().str();
      continue;
    }

    // We have a matching pair of Objective-C classes in base and test.

    if (isMovedLibrary(base->second, test->second))
      continue;

    // new API case 2. Promoted from existing class.
    if (base->second.getRecord()->access != APIAccess::Public &&
        shouldDiagnoseEntry(test->second, *this) &&
        !isExpectedChange({ChangeType::Add, EntryType::Interface, name,
                           test->second.getInstallName()})) {
      builder->report(diag::warn_sdkdb_new_objc)
          << 0 << name << test->second.getInstallName()
          << getTargetTriple().str();
      continue;
    }

    if (!shouldDiagnoseEntry(base->second, *this))
      continue;

    auto baseInstallName = base->second.getInstallName();
    auto testInstallName = test->second.getInstallName();
    checkObjCContainer(
        *test->second.getRecord(), *base->second.getRecord(),
        [&](StringRef error) {
          if (!isExpectedChange({ChangeType::UpdateAccess, EntryType::Interface,
                                 name, baseInstallName})) {
            builder->report(diag::err_sdkdb_objc_container_regression)
                << 0 << name << baseInstallName << getTargetTriple().str()
                << error;
            if (baseInstallName != testInstallName)
              builder->report(diag::note_sdkdb_moved_symbol)
                  << baseInstallName << testInstallName;
          }
        },
        [&](StringRef name, bool isInstanceMethod)
            -> std::pair<ObjCMethodRecord const * /*base*/,
                         ObjCMethodRecord const * /*test*/> {
          return {
              baseline.resolveMethod(name, isInstanceMethod,
                                     *base->second.getRecord()),
              resolveMethod(name, isInstanceMethod, *test->second.getRecord())};
        },
        [&](const ObjCMethodRecord *method) {
          if (!isExpectedChange({ChangeType::Add,
                                 method->isInstanceMethod
                                     ? EntryType::InstanceMethod
                                     : EntryType::ClassMethod,
                                 method->name, baseInstallName, name}))
            builder->report(diag::warn_sdkdb_new_objc_method)
                << method->name << method->isInstanceMethod << 0 << name
                << baseInstallName << getTargetTriple().str();
        },
        [&](const ObjCMethodRecord *method) {
          if (!isExpectedChange({ChangeType::Remove,
                                 method->isInstanceMethod
                                     ? EntryType::InstanceMethod
                                     : EntryType::ClassMethod,
                                 method->name, baseInstallName, name})) {
            builder->report(diag::err_sdkdb_objc_method_regression)
                << method->name << method->isInstanceMethod << 0 << name
                << baseInstallName << getTargetTriple().str()
                << "selector is missing";
            if (baseInstallName != testInstallName)
              builder->report(diag::note_sdkdb_moved_symbol)
                  << baseInstallName << testInstallName;
          }
        },
        [&](const ObjCMethodRecord *method, StringRef error) {
          if (!isExpectedChange({ChangeType::UpdateAccess,
                                 method->isInstanceMethod
                                     ? EntryType::InstanceMethod
                                     : EntryType::ClassMethod,
                                 method->name, baseInstallName, name})) {
            builder->report(diag::err_sdkdb_objc_method_regression)
                << method->name << method->isInstanceMethod << 0 << name
                << baseInstallName << getTargetTriple().str() << error;
            if (baseInstallName != testInstallName)
              builder->report(diag::note_sdkdb_moved_symbol)
                  << baseInstallName << testInstallName;
          }
        });
  }

  // 3. check objc categories.
  for (auto names :
       allKeyPairsFromNestedMaps(baseline.categoryMap, categoryMap)) {
    auto findCategory = [&](const SDKDB::CategoryMapType &map)
        -> std::optional<MapEntry<ObjCCategoryRecord *>> {
      auto clsRes = map.find(names.first);
      if (clsRes == map.end())
        return std::nullopt;

      auto res = clsRes->second.find(names.second);
      if (res == clsRes->second.end())
        return std::nullopt;

      return res->second;
    };

    std::string categoryName = (names.second + "(" + names.first + ")").str();

    auto base = findCategory(baseline.categoryMap);
    auto test = findCategory(categoryMap);

    // regression.
    if (!test) {
      assert(base && "baseline should exist");
      // ignore the private APIs and missing libraries.
      auto missing = *base;
      if (!shouldDiagnoseEntry(missing, *this) ||
          missingLibraries.contains(missing.getInstallName()) ||
          isExpectedChange({ChangeType::Remove, EntryType::Category,
                            categoryName, missing.getInstallName()}))
        continue;

      builder->report(diag::err_sdkdb_missing_objc)
          << 1 << categoryName << missing.getInstallName()
          << getTargetTriple().str();
      continue;
    }

    // new API.
    if (!base) {
      assert(test && "test version should exist");
      auto missing = *test;
      // ignore the private APIs and new libraries.
      if (!shouldDiagnoseEntry(missing, *this) ||
          newLibraries.contains(missing.getInstallName()) ||
          isExpectedChange({ChangeType::Add, EntryType::Category, categoryName,
                            missing.getInstallName()}))
        continue;
      builder->report(diag::warn_sdkdb_new_objc)
          << 1 << categoryName << missing.getInstallName()
          << getTargetTriple().str();
      continue;
    }

    // We have a matching pair of Objective-C categories in base and test.

    if (isMovedLibrary(*base, *test))
      continue;

    // new API case 2. Promoted from existing categories.
    if (base->getRecord()->access != APIAccess::Public &&
        shouldDiagnoseEntry(*test, *this) &&
        !isExpectedChange({ChangeType::Add, EntryType::Category, categoryName,
                           test->getInstallName()})) {
      builder->report(diag::warn_sdkdb_new_objc)
          << 1 << categoryName << test->getInstallName()
          << getTargetTriple().str();
      continue;
    }

    if (!shouldDiagnoseEntry(*base, *this))
      continue;

    auto baseInstallName = base->getInstallName();
    auto testInstallName = test->getInstallName();
    checkObjCContainer(
        *test->getRecord(), *base->getRecord(),
        [&](StringRef error) {
          if (!isExpectedChange({ChangeType::UpdateAccess, EntryType::Category,
                                 categoryName, baseInstallName})) {
            builder->report(diag::err_sdkdb_objc_container_regression)
                << 1 << categoryName << baseInstallName
                << getTargetTriple().str() << error;
            if (baseInstallName != testInstallName)
              builder->report(diag::note_sdkdb_moved_symbol)
                  << baseInstallName << testInstallName;
          }
        },
        [&](StringRef name, bool isInstanceMethod)
            -> std::pair<ObjCMethodRecord const * /*base*/,
                         ObjCMethodRecord const * /*test*/> {
          return {findMethodFromContainer(name, isInstanceMethod,
                                          *base->getRecord()),
                  findMethodFromContainer(name, isInstanceMethod,
                                          *test->getRecord())};
        },
        [&](const ObjCMethodRecord *method) {
          if (!isExpectedChange({ChangeType::Add,
                                 method->isInstanceMethod
                                     ? EntryType::InstanceMethod
                                     : EntryType::ClassMethod,
                                 method->name, baseInstallName, categoryName}))
            builder->report(diag::warn_sdkdb_new_objc_method)
                << method->name << method->isInstanceMethod << 1 << categoryName
                << baseInstallName << getTargetTriple().str();
        },
        [&](const ObjCMethodRecord *method) {
          if (!isExpectedChange(
                  {ChangeType::Remove,
                   method->isInstanceMethod ? EntryType::InstanceMethod
                                            : EntryType::ClassMethod,
                   method->name, baseInstallName, categoryName})) {
            builder->report(diag::err_sdkdb_objc_method_regression)
                << method->name << method->isInstanceMethod << 1 << categoryName
                << baseInstallName << getTargetTriple().str()
                << "selector is missing";
            if (baseInstallName != testInstallName)
              builder->report(diag::note_sdkdb_moved_symbol)
                  << baseInstallName << testInstallName;
          }
        },
        [&](const ObjCMethodRecord *method, StringRef error) {
          if (!isExpectedChange(
                  {ChangeType::UpdateAccess,
                   method->isInstanceMethod ? EntryType::InstanceMethod
                                            : EntryType::ClassMethod,
                   method->name, baseInstallName, categoryName})) {
            builder->report(diag::err_sdkdb_objc_method_regression)
                << method->name << method->isInstanceMethod << 1 << categoryName
                << baseInstallName << getTargetTriple().str() << error;
            if (baseInstallName != testInstallName)
              builder->report(diag::note_sdkdb_moved_symbol)
                  << baseInstallName << testInstallName;
          }
        });
  }

  // 4. check objc protocols.
  for (auto name : allKeysFromMaps(baseline.protocolMap, protocolMap)) {
    auto base = baseline.protocolMap.find(name);
    auto test = protocolMap.find(name);
    // regression.
    if (test == protocolMap.end()) {
      assert(base != baseline.protocolMap.end() && "baseline should exist");
      // ignore the private APIs and missing libraries.
      auto missing = base->second;
      if (!shouldDiagnoseEntry(missing, *this) ||
          missingLibraries.contains(missing.getInstallName()) ||
          isExpectedChange({ChangeType::Remove, EntryType::Protocol, name,
                            missing.getInstallName()}))
        continue;

      builder->report(diag::err_sdkdb_missing_objc)
          << 2 << name << missing.getInstallName() << getTargetTriple().str();
      continue;
    }

    // new API.
    if (base == baseline.protocolMap.end()) {
      assert(test != protocolMap.end() && "test version should exist");
      auto missing = test->second;
      // ignore the private APIs and new libraries.
      if (!shouldDiagnoseEntry(missing, *this) ||
          newLibraries.contains(missing.getInstallName()) ||
          isExpectedChange({ChangeType::Add, EntryType::Protocol, name,
                            missing.getInstallName()}))
        continue;

      builder->report(diag::warn_sdkdb_new_objc)
          << 2 << name << missing.getInstallName() << getTargetTriple().str();
      continue;
    }

    // We have a matching pair of Objective-C protocols in base and test.

    if (isMovedLibrary(base->second, test->second))
      continue;

    // new API case 2. Promoted from existing protocol.
    if (base->second.getRecord()->access != APIAccess::Public &&
        shouldDiagnoseEntry(test->second, *this) &&
        !isExpectedChange({ChangeType::Add, EntryType::Protocol, name,
                           test->second.getInstallName()})) {
      builder->report(diag::warn_sdkdb_new_objc)
          << 2 << name << test->second.getInstallName()
          << getTargetTriple().str();
      continue;
    }

    if (!shouldDiagnoseEntry(base->second, *this))
      continue;

    auto baseInstallName = base->second.getInstallName();
    auto testInstallName = test->second.getInstallName();

    checkObjCContainer(
        *test->second.getRecord(), *base->second.getRecord(),
        [&](StringRef error) {
          if (!isExpectedChange({ChangeType::UpdateAccess, EntryType::Protocol,
                                 name, baseInstallName})) {
            builder->report(diag::err_sdkdb_objc_container_regression)
                << 2 << name << baseInstallName << getTargetTriple().str()
                << error;
            if (baseInstallName != testInstallName)
              builder->report(diag::note_sdkdb_moved_symbol)
                  << baseInstallName << testInstallName;
          }
        },
        [&](StringRef name, bool isInstanceMethod)
            -> std::pair<ObjCMethodRecord const * /*base*/,
                         ObjCMethodRecord const * /*test*/> {
          return {findMethodFromContainer(name, isInstanceMethod,
                                          *base->second.getRecord()),
                  findMethodFromContainer(name, isInstanceMethod,
                                          *test->second.getRecord())};
        },
        [&](const ObjCMethodRecord *method) {
          if (!isExpectedChange({ChangeType::Add,
                                 method->isInstanceMethod
                                     ? EntryType::InstanceMethod
                                     : EntryType::ClassMethod,
                                 method->name, baseInstallName, name}))
            builder->report(diag::warn_sdkdb_new_objc_method)
                << method->name << method->isInstanceMethod << 2 << name
                << baseInstallName << getTargetTriple().str();
        },
        [&](const ObjCMethodRecord *method) {
          if (!isExpectedChange({ChangeType::Remove,
                                 method->isInstanceMethod
                                     ? EntryType::InstanceMethod
                                     : EntryType::ClassMethod,
                                 method->name, baseInstallName, name})) {
            builder->report(diag::err_sdkdb_objc_method_regression)
                << method->name << method->isInstanceMethod << 2 << name
                << baseInstallName << getTargetTriple().str()
                << "selector is missing";
            if (baseInstallName != testInstallName)
              builder->report(diag::note_sdkdb_moved_symbol)
                  << baseInstallName << testInstallName;
          }
        },
        [&](const ObjCMethodRecord *method, StringRef error) {
          if (!isExpectedChange({ChangeType::UpdateAccess,
                                 method->isInstanceMethod
                                     ? EntryType::InstanceMethod
                                     : EntryType::ClassMethod,
                                 method->name, baseInstallName, name})) {
            builder->report(diag::err_sdkdb_objc_method_regression)
                << method->name << method->isInstanceMethod << 2 << name
                << baseInstallName << getTargetTriple().str() << error;
            if (baseInstallName != testInstallName)
              builder->report(diag::note_sdkdb_moved_symbol)
                  << baseInstallName << testInstallName;
          }
        });
  }

  // Skip comparing enums and typedefs.
  if (!builder->diagnoseFrontendAPI())
    return;

  // 5. check enums.
  for (auto name : allKeysFromMaps(baseline.enumMap, enumMap)) {
    auto base = baseline.enumMap.find(name);
    auto test = enumMap.find(name);
    // regression.
    if (test == enumMap.end()) {
      assert(base != baseline.enumMap.end() && "baseline should exist");
      // ignore the private APIs.
      auto missing = base->second;
      if (missing.getRecord()->access != APIAccess::Public)
        continue;

      builder->report(diag::err_sdkdb_missing_frontend_api)
          << 0 << name << getTargetTriple().str();
      continue;
    }

    // new API.
    if (base == baseline.enumMap.end()) {
      assert(test != enumMap.end() && "test version should exist");
      auto missing = test->second;
      if (missing.getRecord()->access != APIAccess::Public)
        continue;
      builder->report(diag::warn_sdkdb_new_frontend_api)
          << 0 << name << getTargetTriple().str();
      continue;
    }

    // new API case 2. Promoted from existing enums.
    if (base->second.getRecord()->access != APIAccess::Public &&
        test->second.getRecord()->access == APIAccess::Public) {
      builder->report(diag::warn_sdkdb_new_frontend_api)
          << 0 << name << getTargetTriple().str();
      continue;
    }

    // check all the fields.
    checkAPIRecord(*test->second.getRecord(), *base->second.getRecord(),
                   [&](StringRef error) {
                     builder->report(diag::err_sdkdb_frontend_api_regression)
                         << 0 << name << getTargetTriple().str() << error;
                   });

    // check constants.
    auto &baseConsts = base->second.getRecord()->constants;
    auto &testConsts = test->second.getRecord()->constants;
    for (auto c : allKeysFromContainer(baseConsts, testConsts)) {
      auto bc =
          llvm::find_if(baseConsts, [&](const EnumConstantRecord *record) {
            return record->name == c;
          });
      auto tc =
          llvm::find_if(testConsts, [&](const EnumConstantRecord *record) {
            return record->name == c;
          });
      // regression.
      if (tc == testConsts.end()) {
        assert(bc != baseConsts.end() && "baseline should exist");
        // ignore the private APIs.
        auto *missing = *bc;
        if (missing->access != APIAccess::Public)
          continue;

        builder->report(diag::err_sdkdb_missing_frontend_api)
            << 1 << c << getTargetTriple().str();
        continue;
      }

      // new API.
      if (bc == baseConsts.end()) {
        assert(tc != testConsts.end() && "test version should exist");
        auto *missing = *tc;
        if (missing->access != APIAccess::Public)
          continue;
        builder->report(diag::warn_sdkdb_new_frontend_api)
            << 1 << c << getTargetTriple().str();
        continue;
      }

      // new API case 2.
      if ((*bc)->access != APIAccess::Public &&
          (*tc)->access == APIAccess::Public) {
        builder->report(diag::warn_sdkdb_new_frontend_api)
            << 1 << c << getTargetTriple().str();
        continue;
      }

      checkAPIRecord(**tc, **bc, [&](StringRef error) {
        builder->report(diag::err_sdkdb_frontend_api_regression)
            << 2 << c << getTargetTriple().str() << error;
      });
    }
  }

  // 6. check typedef.
  for (auto name : allKeysFromMaps(baseline.typedefMap, typedefMap)) {
    auto base = baseline.typedefMap.find(name);
    auto test = typedefMap.find(name);
    // regression.
    if (test == typedefMap.end()) {
      assert(base != baseline.typedefMap.end() && "baseline should exist");
      // ignore the private APIs.
      auto missing = base->second;
      if (missing.getRecord()->access != APIAccess::Public)
        continue;

      builder->report(diag::err_sdkdb_missing_frontend_api)
          << 2 << name << getTargetTriple().str();
      continue;
    }

    // new API.
    if (base == baseline.typedefMap.end()) {
      assert(test != typedefMap.end() && "test version should exist");
      auto missing = test->second;
      if (missing.getRecord()->access != APIAccess::Public)
        continue;
      builder->report(diag::warn_sdkdb_new_frontend_api)
          << 2 << name << getTargetTriple().str();
      continue;
    }

    // new API case 2.
    if (base->second.getRecord()->access != APIAccess::Public &&
        test->second.getRecord()->access == APIAccess::Public) {
      builder->report(diag::warn_sdkdb_new_frontend_api)
          << 2 << name << getTargetTriple().str();
      continue;
    }

    // check all the fields.
    checkAPIRecord(*test->second.getRecord(), *base->second.getRecord(),
                   [&](StringRef error) {
                     builder->report(diag::err_sdkdb_frontend_api_regression)
                         << 2 << name << getTargetTriple().str() << error;
                   });
  }
}

void SDKDBBuilder::buildLookupTables() {
  for (auto &db : databases)
    db.buildLookupTables();
}

bool SDKDBBuilder::diagnoseDifferences(SDKDBBuilder &baseline) {
  // Initialize the lookup table first.
  baseline.buildLookupTables();
  buildLookupTables();

  assert(!diag.hasErrorOccurred() && "no error should occured");
  // Compare the SDKDBs by lookup table.
  for (const auto &base : baseline.databases) {
    // Check to see if the target exists.
    auto *db = llvm::find_if(databases, [&](const SDKDB &db) {
      return SDKDB::areCompatibleTargets(db.triple, base.triple);
    });
    if (db == databases.end()) {
      report(diag::err_sdkdb_missing_target) << base.triple.str();
      continue;
    }

    if (!shouldDiagnoseTriple(db->getTargetTriple()))
      continue;

    if (compareConfigFileReader)
      db->expectedChanges =
          &compareConfigFileReader->expectedChanges(db->triple);

    db->diagnoseDifferences(base);
  }

  return !diag.hasErrorOccurred();
}

void SDKDBBuilder::setReportNewAPIasError(bool val) {
  diag.setWarningsAsErrors(val);
}

void SDKDBBuilder::setNoNewAPI(bool val) {
  if (val) {
    // TODO: Properly support diagnostic groups in tapi.
    diag.ignoreDiagnotic(diag::warn_sdkdb_new_api);
    diag.ignoreDiagnotic(diag::warn_sdkdb_new_global);
    diag.ignoreDiagnotic(diag::warn_sdkdb_new_objc);
    diag.ignoreDiagnotic(diag::warn_sdkdb_new_objc_method);
    diag.ignoreDiagnotic(diag::warn_sdkdb_new_frontend_api);
  }
}

bool SDKDBBuilder::setFilteredPlatforms(StringRef platform) {
  llvm::SmallVector<PlatformType, 5> mappedPlatforms;
  // Map base platform sdkdb to all the platforms that should represent
  // APIs to compare.
  switch (getPlatformFromName(platform)) {
  case PLATFORM_MACOS: {
    mappedPlatforms = {PLATFORM_MACOS, PLATFORM_MACCATALYST,
                       PLATFORM_DRIVERKIT};
    break;
  }
  case PLATFORM_IOS: {
    mappedPlatforms = {
        PLATFORM_IOS,
        PLATFORM_IOSSIMULATOR,
    };
    break;
  }
  case PLATFORM_TVOS: {
    mappedPlatforms = {
        PLATFORM_TVOS,
        PLATFORM_TVOSSIMULATOR,
    };
    break;
  }
  case PLATFORM_WATCHOS: {
    mappedPlatforms = {
        PLATFORM_WATCHOS,
        PLATFORM_WATCHOSSIMULATOR,
    };
    break;
  }
  case PLATFORM_XROS: {
    mappedPlatforms = {
        PLATFORM_XROS,
        PLATFORM_XROS_SIMULATOR,
    };
    break;
  }
  default:
    return false;
  }

  assert(filterPlatforms.empty() &&
         "filtered platforms should only be assigned once");
  filterPlatforms.insert(mappedPlatforms.begin(), mappedPlatforms.end());
  return true;
}

TAPI_NAMESPACE_INTERNAL_END
