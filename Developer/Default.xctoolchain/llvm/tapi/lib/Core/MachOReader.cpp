//===- lib/Core/MachOReader - TAPI MachO Reader -----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the TAPI MachOReader.
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/MachOReader.h"
#include "tapi/ObjCMetadata/ObjCMetadata.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/BinaryFormat/Magic.h"
#include "llvm/DebugInfo/DWARF/DWARFCompileUnit.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/Object/Binary.h"
#include "llvm/Object/MachO.h"
#include "llvm/Object/MachOUniversal.h"
#include "llvm/TextAPI/Platform.h"
#include <iomanip>
#include <sstream>

using namespace llvm;
using namespace llvm::object;
using namespace llvm::MachO;

TAPI_NAMESPACE_INTERNAL_BEGIN

Expected<FileType> getMachOFileType(MemoryBufferRef bufferRef) {
  auto magic = identify_magic(bufferRef.getBuffer());
  switch (magic) {
  default:
    return FileType::Invalid;
  case file_magic::macho_bundle:
    return FileType::MachO_Bundle;
  case file_magic::macho_dynamically_linked_shared_lib:
    return FileType::MachO_DynamicLibrary;
  case file_magic::macho_dynamically_linked_shared_lib_stub:
    return FileType::MachO_DynamicLibrary_Stub;
  case file_magic::macho_universal_binary:
    break;
  }

  auto binaryOrErr = createBinary(bufferRef);
  if (!binaryOrErr)
    return binaryOrErr.takeError();

  Binary &bin = *binaryOrErr.get();
  assert(isa<MachOUniversalBinary>(&bin) && "Unexpected MachO binary");
  auto *UB = cast<MachOUniversalBinary>(&bin);

  FileType fileType = FileType::Invalid;
  // Check if any of the architecture slices are a MachO dylib.
  for (auto OI = UB->begin_objects(), OE = UB->end_objects(); OI != OE; ++OI) {
    // This can fail if the object is an archive.
    auto objOrErr = OI->getAsObjectFile();
    // Skip the archive and comsume the error.
    if (!objOrErr) {
      consumeError(objOrErr.takeError());
      continue;
    }

    auto &obj = *objOrErr.get();
    switch (obj.getHeader().filetype) {
    default:
      continue;
    case MachO::MH_BUNDLE:
      if (fileType == FileType::Invalid)
        fileType = FileType::MachO_Bundle;
      else if (fileType != FileType::MachO_Bundle)
        return FileType::Invalid;
      break;
    case MachO::MH_DYLIB:
      if (fileType == FileType::Invalid)
        fileType = FileType::MachO_DynamicLibrary;
      else if (fileType != FileType::MachO_DynamicLibrary)
        return FileType::Invalid;
      break;
    case MachO::MH_DYLIB_STUB:
      if (fileType == FileType::Invalid)
        fileType = FileType::MachO_DynamicLibrary_Stub;
      else if (fileType != FileType::MachO_DynamicLibrary_Stub)
        return FileType::Invalid;
      break;
    }
  }

  return fileType;
}

static Error readMachOHeader(MachOObjectFile *object, API &api) {
  auto H = object->getHeader();
  auto arch = getArchitectureFromCpuType(H.cputype, H.cpusubtype);
  if (arch == AK_unknown)
    return make_error<StringError>(
        "unknown/unsupported architecture",
        std::make_error_code(std::errc::not_supported));
  auto &binaryInfo = api.getBinaryInfo();

  switch (H.filetype) {
  default:
    llvm_unreachable("unsupported binary type");
  case MachO::MH_DYLIB:
    binaryInfo.fileType = FileType::MachO_DynamicLibrary;
    break;
  case MachO::MH_DYLIB_STUB:
    binaryInfo.fileType = FileType::MachO_DynamicLibrary_Stub;
    break;
  case MachO::MH_BUNDLE:
    binaryInfo.fileType = FileType::MachO_Bundle;
    break;
  }

  if (H.flags & MachO::MH_TWOLEVEL)
    binaryInfo.isTwoLevelNamespace = true;

  if (H.flags & MachO::MH_APP_EXTENSION_SAFE)
    binaryInfo.isAppExtensionSafe = true;

  for (const auto &LCI : object->load_commands()) {
    switch (LCI.C.cmd) {
    case MachO::LC_ID_DYLIB: {
      auto DLLC = object->getDylibIDLoadCommand(LCI);
      binaryInfo.installName = api.copyString(LCI.Ptr + DLLC.dylib.name);
      binaryInfo.currentVersion = DLLC.dylib.current_version;
      binaryInfo.compatibilityVersion = DLLC.dylib.compatibility_version;
      break;
    }
    case MachO::LC_REEXPORT_DYLIB: {
      auto DLLC = object->getDylibIDLoadCommand(LCI);
      binaryInfo.reexportedLibraries.emplace_back(
          api.copyString(LCI.Ptr + DLLC.dylib.name));
      break;
    }
    case MachO::LC_SUB_FRAMEWORK: {
      auto SFC = object->getSubFrameworkCommand(LCI);
      binaryInfo.parentUmbrella = api.copyString(LCI.Ptr + SFC.umbrella);
      break;
    }
    case MachO::LC_SUB_CLIENT: {
      auto SCLC = object->getSubClientCommand(LCI);
      binaryInfo.allowableClients.emplace_back(
          api.copyString(LCI.Ptr + SCLC.client));
      break;
    }
    case MachO::LC_UUID: {
      auto UUIDLC = object->getUuidCommand(LCI);
      std::stringstream stream;
      for (unsigned i = 0; i < 16; ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10)
          stream << '-';
        stream << std::setfill('0') << std::setw(2) << std::uppercase
               << std::hex << static_cast<int>(UUIDLC.uuid[i]);
      }
      binaryInfo.uuid = api.copyString(stream.str());
      break;
    }
    case MachO::LC_RPATH: {
      auto RPLC = object->getRpathCommand(LCI);
      binaryInfo.rpaths.emplace_back(api.copyString(LCI.Ptr + RPLC.path));
      break;
    }
    case MachO::LC_SEGMENT_SPLIT_INFO: {
      auto SSILC = object->getLinkeditDataLoadCommand(LCI);
      if (SSILC.datasize == 0)
        binaryInfo.isOSLibNotForSharedCache = true;
      break;
    }
    default:
      break;
    }
  }

  for (auto &section : object->sections()) {
    auto sectionName = section.getName();
    if (!sectionName)
      return sectionName.takeError();
    if (*sectionName != "__objc_imageinfo" && *sectionName != "__image_info")
      continue;
    auto content = section.getContents();
    if (!content)
      return content.takeError();

    if ((content->size() >= 8) && (content->front() == 0)) {
      uint32_t flags;
      if (object->isLittleEndian()) {
        auto *p =
            reinterpret_cast<const support::ulittle32_t *>(content->data() + 4);
        flags = *p;
      } else {
        auto *p =
            reinterpret_cast<const support::ubig32_t *>(content->data() + 4);
        flags = *p;
      }
      binaryInfo.swiftABIVersion = (flags >> 8) & 0xFF;
    }
  }
  return Error::success();
}

static void DWARFErrorHandler(Error err) { /**/
}

static SymbolToSourceLocMap
accumulateLocs(MachOObjectFile &obj,
               const std::unique_ptr<DWARFContext> &diCtx) {
  SymbolToSourceLocMap locMap;
  for (const auto &symbol : obj.symbols()) {
    auto flagsOrErr = symbol.getFlags();
    if (!flagsOrErr) {
      consumeError(flagsOrErr.takeError());
      continue;
    }

    if (!(*flagsOrErr & SymbolRef::SF_Exported))
      continue;

    auto addressOrErr = symbol.getAddress();
    if (!addressOrErr) {
      consumeError(addressOrErr.takeError());
      continue;
    }
    auto address = *addressOrErr;

    auto typeOrErr = symbol.getType();
    if (!typeOrErr) {
      consumeError(typeOrErr.takeError());
      continue;
    }
    const bool isCode = (*typeOrErr & SymbolRef::ST_Function);

    auto *dwarfCU = isCode ? diCtx->getCompileUnitForCodeAddress(address)
                           : diCtx->getCompileUnitForDataAddress(address);
    if (!dwarfCU)
      continue;

    const DWARFDie &die = isCode ? dwarfCU->getSubroutineForAddress(address)
                                 : dwarfCU->getVariableForAddress(address);
    const auto file = die.getDeclFile(
        llvm::DILineInfoSpecifier::FileLineInfoKind::AbsoluteFilePath);
    const auto line = die.getDeclLine();

    auto nameOrErr = symbol.getName();
    if (!nameOrErr) {
      consumeError(nameOrErr.takeError());
      continue;
    }
    auto name = *nameOrErr;
    auto sym = parseSymbol(name);

    if (!file.empty() && line != 0)
      locMap[sym.Name.str()] = APILoc(file, line, 0);
  }

  return locMap;
}

SymbolToSourceLocMap accumulateSourceLocFromDSYM(const StringRef dSYMFile,
                                                 const Target &target) {
  // Find sidecar file.
  auto dSYMsOrErr = MachOObjectFile::findDsymObjectMembers(dSYMFile);
  if (!dSYMsOrErr) {
    consumeError(dSYMsOrErr.takeError());
    return SymbolToSourceLocMap();
  }
  if (dSYMsOrErr->empty())
    return SymbolToSourceLocMap();

  const StringRef path = dSYMsOrErr->front();
  ErrorOr<std::unique_ptr<MemoryBuffer>> buffer = MemoryBuffer::getFile(path);
  if (auto error = buffer.getError())
    return SymbolToSourceLocMap();

  Expected<std::unique_ptr<Binary>> binOrErr = createBinary(*buffer.get());
  if (!binOrErr) {
    consumeError(binOrErr.takeError());
    return SymbolToSourceLocMap();
  }
  // Handle single arch.
  if (auto *single = dyn_cast<MachOObjectFile>(binOrErr->get())) {
    auto diCtx = DWARFContext::create(
        *single, DWARFContext::ProcessDebugRelocations::Process, nullptr, "",
        DWARFErrorHandler, DWARFErrorHandler);

    return accumulateLocs(*single, diCtx);
  }
  // Handle universal companion file.
  if (auto *fat = dyn_cast<MachOUniversalBinary>(binOrErr->get())) {
    auto objForArch = fat->getObjectForArch(getArchitectureName(target.Arch));
    if (!objForArch) {
      consumeError(objForArch.takeError());
      return SymbolToSourceLocMap();
    }
    auto machOOrErr = objForArch->getAsObjectFile();
    if (!machOOrErr) {
      consumeError(machOOrErr.takeError());
      return SymbolToSourceLocMap();
    }
    auto &obj = **machOOrErr;
    auto diCtx = DWARFContext::create(
        obj, DWARFContext::ProcessDebugRelocations::Process, nullptr, "",
        DWARFErrorHandler, DWARFErrorHandler);

    return accumulateLocs(obj, diCtx);
  }
  return SymbolToSourceLocMap();
}

static Error readSymbols(MachOObjectFile *object, API &api,
                         const MachOParseOption &options) {
  assert(getArchitectureFromCpuType(object->getHeader().cputype,
                                    object->getHeader().cpusubtype) !=
             AK_unknown &&
         "unknown architecture slice");

  auto parseExport = [](const auto exportFlags,
                        auto address) -> std::tuple<SymbolFlags, APILinkage> {
    SymbolFlags flags = SymbolFlags::None;
    switch (exportFlags & MachO::EXPORT_SYMBOL_FLAGS_KIND_MASK) {
    case MachO::EXPORT_SYMBOL_FLAGS_KIND_REGULAR:
      if (exportFlags & MachO::EXPORT_SYMBOL_FLAGS_WEAK_DEFINITION)
        flags |= SymbolFlags::WeakDefined;
      break;
    case MachO::EXPORT_SYMBOL_FLAGS_KIND_THREAD_LOCAL:
      flags |= SymbolFlags::ThreadLocalValue;
      break;
    }

    APILinkage linkage = (exportFlags & MachO::EXPORT_SYMBOL_FLAGS_REEXPORT)
                             ? APILinkage::Reexported
                             : APILinkage::Exported;

    return {flags, linkage};
  };

  Error error = Error::success();
  std::unordered_map<std::string, std::pair<SymbolFlags, APILinkage>> exports;
  for (auto &symbol : object->exports(error)) {
    auto [flags, linkage] = parseExport(symbol.flags(), symbol.address());
    exports.insert({symbol.name().str(), {flags, linkage}});
    // FIXME Workaround for: rdar://105047425
    // Add swift symbols from export trie.
    if (symbol.name().startswith("_$s") || symbol.name().startswith("_$S"))
      api.addGlobalFromBinary(symbol.name().str(), flags, APILoc(),
                              GVKind::Unknown, linkage);
  }

  for (const auto &symbol : object->symbols()) {
    auto flagsOrErr = symbol.getFlags();
    if (!flagsOrErr)
      return flagsOrErr.takeError();
    auto flags = *flagsOrErr;

    auto nameOrErr = symbol.getName();
    if (!nameOrErr)
      return nameOrErr.takeError();
    auto name = *nameOrErr;

    APILinkage linkage = APILinkage::Unknown;
    SymbolFlags apiFlags = SymbolFlags::None;

    if (options.parseUndefined && (flags & SymbolRef::SF_Undefined))
      linkage = APILinkage::External;
    else if (flags & SymbolRef::SF_Exported) {
      auto it = exports.find(name.str());
      if (it == exports.end())
        continue;
      std::tie(apiFlags, linkage) = it->second;
    } else if (flags & SymbolRef::SF_Hidden)
      linkage = APILinkage::Internal;
    else
      continue;

    auto typeOrErr = symbol.getType();
    if (!typeOrErr)
      return typeOrErr.takeError();
    auto type = *typeOrErr;

    GVKind kind =
        (type & SymbolRef::ST_Function) ? GVKind::Function : GVKind::Variable;

    if (kind == GVKind::Function)
      apiFlags |= SymbolFlags::Text;
    else
      apiFlags |= SymbolFlags::Data;

    api.addGlobalFromBinary(name, apiFlags, APILoc(), kind, linkage);
  }
  return error;
}

static ObjCPropertyRecord::AttributeKind getAttributeKind(StringRef attr) {
  unsigned attrs = ObjCPropertyRecord::NoAttr;
  SmallVector<StringRef, 4> attributes;
  attr.split(attributes, ',');
  for (auto a : attributes) {
    if (a == "R")
      attrs |= ObjCPropertyRecord::ReadOnly;
    else if (a == "D")
      attrs |= ObjCPropertyRecord::Dynamic;
  }

  return (ObjCPropertyRecord::AttributeKind)attrs;
}

static Error readObjectiveCMetadata(MachOObjectFile *object, API &api) {
  auto error = Error::success();
  ObjCMetaDataReader metadata(object, error);
  if (error)
    return std::move(error);

  ///
  /// Classes
  ///
  auto classes = metadata.classes();
  if (!classes)
    return classes.takeError();

  for (const auto &classRef : *classes) {
    auto objcClassMeta = classRef.getObjCClass();
    if (!objcClassMeta)
      return objcClassMeta.takeError();

    auto superClassName = objcClassMeta->getSuperClassName();
    if (!superClassName)
      return superClassName.takeError();

    auto className = objcClassMeta->getName();
    if (!className)
      return className.takeError();

    // FIXME: Re-adding classes should not assume additional attributes.
    auto *objcClass = api.addObjCInterface(
        *className, APILoc(), AvailabilityInfo(), APIAccess::Unknown,
        APILinkage::Exported, *superClassName, nullptr,
        ObjCIFSymbolKind::Class | ObjCIFSymbolKind::MetaClass,
        /*overrideLinkage=*/false);

    auto properties = objcClassMeta->properties();
    if (!properties)
      return properties.takeError();

    for (const auto &property : *properties) {
      auto name = property.getName();
      if (!name)
        return name.takeError();

      auto attr = property.getAttribute();
      if (!attr)
        return attr.takeError();
      auto attrs = getAttributeKind(*attr);

      auto setter = property.getSetter();
      if (!setter)
        return setter.takeError();
      auto getter = property.getGetter();
      if (!getter)
        return getter.takeError();

      api.addObjCProperty(objcClass, *name, *getter, *setter, APILoc(),
                          AvailabilityInfo(), APIAccess::Unknown, attrs,
                          /*isOptional=*/false, nullptr);
    }

    auto classMethods = objcClassMeta->classMethods();
    if (!classMethods)
      return classMethods.takeError();

    for (const auto &method : *classMethods) {
      auto name = method.getName();
      if (!name)
        return name.takeError();

      api.addObjCMethod(objcClass, *name, APILoc(), AvailabilityInfo(),
                        APIAccess::Unknown,
                        /*isInstanceMethod=*/false, /*isOptional=*/false,
                        /*isDynamic=*/false, nullptr);
    }

    auto instanceMethods = objcClassMeta->instanceMethods();
    if (!instanceMethods)
      return instanceMethods.takeError();

    for (const auto &method : *instanceMethods) {
      auto name = method.getName();
      if (!name)
        return name.takeError();

      api.addObjCMethod(objcClass, *name, APILoc(), AvailabilityInfo(),
                        APIAccess::Unknown,
                        /*isInstanceMethod=*/true, /*isOptional=*/false,
                        /*isDynamic=*/false, nullptr);
    }
  }

  ///
  /// Categories
  ///
  auto categories = metadata.categories();
  if (!categories)
    return categories.takeError();

  for (const auto &categoryRef : *categories) {
    auto category = categoryRef.getObjCCategory();
    if (!category)
      return category.takeError();

    auto categoryName = category->getName();
    if (!categoryName)
      return categoryName.takeError();

    auto baseClassName = category->getBaseClassName();
    if (!baseClassName)
      return baseClassName.takeError();

    auto *objcCategory =
        api.addObjCCategory(*baseClassName, *categoryName, APILoc(),
                            AvailabilityInfo(), APIAccess::Unknown, nullptr);

    auto properties = category->properties();
    if (!properties)
      return properties.takeError();

    for (const auto &property : *properties) {
      auto name = property.getName();
      if (!name)
        return name.takeError();

      auto attr = property.getAttribute();
      if (!attr)
        return attr.takeError();
      auto attrs = getAttributeKind(*attr);

      auto setter = property.getSetter();
      if (!setter)
        return setter.takeError();
      auto getter = property.getGetter();
      if (!getter)
        return getter.takeError();

      api.addObjCProperty(objcCategory, *name, *getter, *setter, APILoc(),
                          AvailabilityInfo(), APIAccess::Unknown, attrs,
                          /*isOptional=*/false, nullptr);
    }

    auto classMethods = category->classMethods();
    if (!classMethods)
      return classMethods.takeError();

    for (const auto &method : *classMethods) {
      auto name = method.getName();
      if (!name)
        return name.takeError();

      api.addObjCMethod(objcCategory, *name, APILoc(), AvailabilityInfo(),
                        APIAccess::Unknown,
                        /*isInstanceMethod=*/false, /*isOptional=*/false,
                        /*isDynamic=*/false, nullptr);
    }

    auto instanceMethods = category->instanceMethods();
    if (!instanceMethods)
      return instanceMethods.takeError();

    for (const auto &method : *instanceMethods) {
      auto name = method.getName();
      if (!name)
        return name.takeError();

      api.addObjCMethod(objcCategory, *name, APILoc(), AvailabilityInfo(),
                        APIAccess::Unknown,
                        /*isInstanceMethod=*/true, /*isOptional=*/false,
                        /*isDynamic=*/false, nullptr);
    }
  }

  ///
  /// Protocols
  ///
  auto protocols = metadata.protocols();
  if (!protocols)
    return protocols.takeError();

  for (const auto &protocolRef : *protocols) {
    auto protocol = protocolRef.getObjCProtocol();
    if (!protocol)
      return protocol.takeError();

    auto protocolName = protocol->getName();
    if (!protocolName)
      return protocolName.takeError();

    auto *objcProtocol =
        api.addObjCProtocol(*protocolName, APILoc(), AvailabilityInfo(),
                            APIAccess::Unknown, nullptr);

    auto properties = protocol->properties();
    if (!properties)
      return properties.takeError();

    for (const auto &property : *properties) {
      auto name = property.getName();
      if (!name)
        return name.takeError();

      auto attr = property.getAttribute();
      if (!attr)
        return attr.takeError();
      auto attrs = getAttributeKind(*attr);

      auto setter = property.getSetter();
      if (!setter)
        return setter.takeError();
      auto getter = property.getGetter();
      if (!getter)
        return getter.takeError();

      api.addObjCProperty(objcProtocol, *name, *getter, *setter, APILoc(),
                          AvailabilityInfo(), APIAccess::Unknown, attrs,
                          /*isOptional=*/false, nullptr);
    }

    auto classMethods = protocol->classMethods();
    if (!classMethods)
      return classMethods.takeError();

    for (const auto &method : *classMethods) {
      auto name = method.getName();
      if (!name)
        return name.takeError();

      api.addObjCMethod(objcProtocol, *name, APILoc(), AvailabilityInfo(),
                        APIAccess::Unknown,
                        /*isInstanceMethod=*/false, /*isOptional=*/false,
                        /*isDynamic=*/false, nullptr);
    }

    classMethods = protocol->optionalClassMethods();
    if (!classMethods)
      return classMethods.takeError();

    for (const auto &method : *classMethods) {
      auto name = method.getName();
      if (!name)
        return name.takeError();

      api.addObjCMethod(objcProtocol, *name, APILoc(), AvailabilityInfo(),
                        APIAccess::Unknown,
                        /*isInstanceMethod=*/false, /*isOptional=*/true,
                        /*isDynamic=*/false, nullptr);
    }

    auto instanceMethods = protocol->instanceMethods();
    if (!instanceMethods)
      return instanceMethods.takeError();

    for (const auto &method : *instanceMethods) {
      auto name = method.getName();
      if (!name)
        return name.takeError();

      api.addObjCMethod(objcProtocol, *name, APILoc(), AvailabilityInfo(),
                        APIAccess::Unknown,
                        /*isInstanceMethod=*/true, /*isOptional=*/false,
                        /*isDynamic=*/false, nullptr);
    }

    instanceMethods = protocol->optionalInstanceMethods();
    if (!instanceMethods)
      return instanceMethods.takeError();

    for (const auto &method : *instanceMethods) {
      auto name = method.getName();
      if (!name)
        return name.takeError();

      api.addObjCMethod(objcProtocol, *name, APILoc(), AvailabilityInfo(),
                        APIAccess::Unknown,
                        /*isInstanceMethod=*/true, /*isOptional=*/true,
                        /*isDynamic=*/false, nullptr);
    }
  }

  // Potentially defined selectors for swift.
  // Swift compiler generates certain data structure which can dynamically
  // constructing objc metadata.
  if (api.hasBinaryInfo() && api.getBinaryInfo().swiftABIVersion != 0)
    metadata.getAllPotentiallyDefinedSelectors(
        api.getPotentiallyDefinedSelectors());

  return Error::success();
}

static Error load(MachOObjectFile *object, API &api, MachOParseOption &option) {
  if (option.parseMachOHeader) {
    auto error = readMachOHeader(object, api);
    if (error)
      return error;
  }
  if (option.parseSymbolTable) {
    auto error = readSymbols(object, api, option);
    if (error)
      return error;
  }

  if (option.parseObjCMetadata) {
    auto error = readObjectiveCMetadata(object, api);
    if (error)
      return error;
  }

  return Error::success();
}

std::vector<Triple> constructTripleFromMachO(MachOObjectFile *object) {
  std::vector<Triple> triples;

  auto archType = getArchitectureFromCpuType(object->getHeader().cputype,
                                             object->getHeader().cpusubtype);
  auto arch = getArchitectureName(archType);

  bool isIntelBased = ArchitectureSet(archType).hasX86();

  auto getOSVersion = [](uint32_t version) {
    PackedVersion OSVersion(version);
    std::string vers;
    raw_string_ostream versionStream(vers);
    versionStream << OSVersion;
    return versionStream.str();
  };
  auto getOSVersionFromVersionMin =
      [&](const MachOObjectFile::LoadCommandInfo &cmd) {
        auto vers = object->getVersionMinLoadCommand(cmd);
        return getOSVersion(vers.version);
      };

  for (const auto &cmd : object->load_commands()) {
    std::string OSVersion;
    switch (cmd.C.cmd) {
    case MachO::LC_VERSION_MIN_MACOSX:
      OSVersion = getOSVersionFromVersionMin(cmd);
      triples.emplace_back(arch, "apple", "macos" + OSVersion);
      break;
    case MachO::LC_VERSION_MIN_IPHONEOS:
      OSVersion = getOSVersionFromVersionMin(cmd);
      if (isIntelBased)
        triples.emplace_back(arch, "apple", "ios" + OSVersion, "simulator");
      else
        triples.emplace_back(arch, "apple", "ios" + OSVersion);
      break;
    case MachO::LC_VERSION_MIN_TVOS:
      OSVersion = getOSVersionFromVersionMin(cmd);
      if (isIntelBased)
        triples.emplace_back(arch, "apple", "tvos" + OSVersion, "simulator");
      else
        triples.emplace_back(arch, "apple", "tvos" + OSVersion);
      break;
    case MachO::LC_VERSION_MIN_WATCHOS:
      OSVersion = getOSVersionFromVersionMin(cmd);
      if (isIntelBased)
        triples.emplace_back(arch, "apple", "watchos" + OSVersion, "simulator");
      else
        triples.emplace_back(arch, "apple", "watchos" + OSVersion);
      break;
    case MachO::LC_BUILD_VERSION: {
      OSVersion = getOSVersion(object->getBuildVersionLoadCommand(cmd).minos);
      switch (object->getBuildVersionLoadCommand(cmd).platform) {
      case MachO::PLATFORM_MACOS:
        triples.emplace_back(arch, "apple", "macos" + OSVersion);
        break;
      case MachO::PLATFORM_IOS:
        triples.emplace_back(arch, "apple", "ios" + OSVersion);
        break;
      case MachO::PLATFORM_TVOS:
        triples.emplace_back(arch, "apple", "tvos" + OSVersion);
        break;
      case MachO::PLATFORM_WATCHOS:
        triples.emplace_back(arch, "apple", "watchos" + OSVersion);
        break;
      case MachO::PLATFORM_BRIDGEOS:
        triples.emplace_back(arch, "apple", "bridgeos" + OSVersion);
        break;
      case MachO::PLATFORM_MACCATALYST:
        triples.emplace_back(arch, "apple", "ios" + OSVersion, "macabi");
        break;
      case MachO::PLATFORM_IOSSIMULATOR:
        triples.emplace_back(arch, "apple", "ios" + OSVersion, "simulator");
        break;
      case MachO::PLATFORM_TVOSSIMULATOR:
        triples.emplace_back(arch, "apple", "tvos" + OSVersion, "simulator");
        break;
      case MachO::PLATFORM_WATCHOSSIMULATOR:
        triples.emplace_back(arch, "apple", "watchos" + OSVersion, "simulator");
        break;
      case MachO::PLATFORM_DRIVERKIT:
        triples.emplace_back(arch, "apple", "driverkit" + OSVersion);
        break;
      case MachO::PLATFORM_XROS:
        triples.emplace_back(arch, "apple",
                             Triple::getOSTypeName(Triple::XROS) +
                             OSVersion);
        break;
      case MachO::PLATFORM_XROS_SIMULATOR:
        triples.emplace_back(arch, "apple",
                             Triple::getOSTypeName(Triple::XROS) +
                             OSVersion,
                             "simulator");
        break;
      default:
        break; // skip.
      }
      break;
    }
    default:
      break;
    }
  }

  // record unknown platform for older binary that does not enforece platform
  // load commands.
  if (triples.empty())
    triples.emplace_back(arch, "apple", "unknown");

  // Remove duplicates.
  sort(triples, [](const Triple &lhs, const Triple &rhs) {
    return lhs.str() < rhs.str();
  });
  auto last = std::unique(triples.begin(), triples.end());
  triples.erase(last, triples.end());

  return triples;
}

/// Read APIs from the macho buffer.
llvm::Expected<MachOParseResult> readMachOFile(MemoryBufferRef memBuffer,
                                               MachOParseOption &option) {
  MachOParseResult results;

  auto binaryOrErr = createBinary(memBuffer);
  if (!binaryOrErr)
    return binaryOrErr.takeError();

  Binary &binary = *binaryOrErr.get();
  if (auto *object = dyn_cast<MachOObjectFile>(&binary)) {
    auto arch = getArchitectureFromCpuType(object->getHeader().cputype,
                                           object->getHeader().cpusubtype);
    if (!option.arches.has(arch))
      return make_error<StringError>(
          "Requested architectures don't exist",
          std::make_error_code(std::errc::not_supported));

    auto triples = constructTripleFromMachO(object);
    for (const auto &target : triples) {
      if (mapToPlatformType(target) == PLATFORM_UNKNOWN)
        return make_error<StringError>(
            "unknown/unsupported platform",
            std::make_error_code(std::errc::not_supported));
      results.emplace_back(arch, std::make_shared<API>(API({target})));
      auto error = load(object, *results.back().second, option);
      if (error)
        return std::move(error);
    }

    return results;
  }

  // Only expecting MachO universal binaries at this point.
  assert(isa<MachOUniversalBinary>(&binary) &&
         "Expected a MachO universal binary.");
  auto *UB = cast<MachOUniversalBinary>(&binary);

  for (auto OI = UB->begin_objects(), OE = UB->end_objects(); OI != OE; ++OI) {
    // Skip the architecture that is not requested.
    auto arch =
        getArchitectureFromCpuType(OI->getCPUType(), OI->getCPUSubType());
    if (!option.arches.has(arch))
      continue;

    // Skip unknown architectures.
    if (arch == AK_unknown)
      continue;

    // This can fail if the object is an archive.
    auto objOrErr = OI->getAsObjectFile();

    // Skip the archive and comsume the error.
    if (!objOrErr) {
      consumeError(objOrErr.takeError());
      continue;
    }

    auto &object = *objOrErr.get();
    auto triples = constructTripleFromMachO(&object);
    switch (object.getHeader().filetype) {
    default:
      break;
    case MachO::MH_BUNDLE:
    case MachO::MH_DYLIB:
    case MachO::MH_DYLIB_STUB:
      for (const auto &target : triples) {
        results.emplace_back(arch, std::make_shared<API>(API({target})));
        auto error = load(&object, *results.back().second, option);
        if (error)
          return std::move(error);
      }
      break;
    }
  }

  if (results.empty())
    return make_error<StringError>(
        "Requested architectures don't exist",
        std::make_error_code(std::errc::not_supported));

  return results;
}

TAPI_NAMESPACE_INTERNAL_END
