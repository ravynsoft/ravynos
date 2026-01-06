//===- tapi/Core/YAMLReaderWriter.cpp - YAML Reader/Writer ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the YAML reader/writer.
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/YAMLReaderWriter.h"
#include "tapi/Core/Registry.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLTraits.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TextAPI/InterfaceFile.h"
#include "llvm/TextAPI/TextAPIReader.h"

using namespace llvm;
using namespace llvm::yaml;
using namespace tapi::internal;

TAPI_NAMESPACE_INTERNAL_BEGIN

namespace stub::v1 {
bool YAMLDocumentHandler::canRead(MemoryBufferRef memBufferRef,
                                  FileType types) const {
  if (!(types & FileType::TBD_V1))
    return false;

  auto result = TextAPIReader::canRead(memBufferRef);
  if (!result) {
    consumeError(result.takeError());
    return false;
  }
  return *result == FileType::TBD_V1;
}

FileType YAMLDocumentHandler::getFileType(MemoryBufferRef memBufferRef) const {
  if (canRead(memBufferRef))
    return FileType::TBD_V1;

  return FileType::Invalid;
}

bool YAMLDocumentHandler::canWrite(const InterfaceFile *file,
                                   FileType fileType) const {
  if (fileType != FileType::TBD_V1)
    return false;

  // TODO: report reason.
  if (!file->isApplicationExtensionSafe() || !file->isTwoLevelNamespace())
    return false;

  return true;
}
} // end namespace stub::v1

namespace stub::v2 {

bool YAMLDocumentHandler::canRead(MemoryBufferRef memBufferRef,
                                  FileType types) const {
  if (!(types & FileType::TBD_V2))
    return false;

  auto result = TextAPIReader::canRead(memBufferRef);
  if (!result) {
    consumeError(result.takeError());
    return false;
  }
  return *result == FileType::TBD_V2;
}

FileType YAMLDocumentHandler::getFileType(MemoryBufferRef memBufferRef) const {
  if (canRead(memBufferRef))
    return FileType::TBD_V2;

  return FileType::Invalid;
}

bool YAMLDocumentHandler::canWrite(const InterfaceFile *file,
                                   FileType fileType) const {
  if (fileType != FileType::TBD_V2)
    return false;

  return true;
}
} // end namespace stub::v2

namespace stub::v3 {
bool YAMLDocumentHandler::canRead(MemoryBufferRef memBufferRef,
                                  FileType types) const {
  if (!(types & FileType::TBD_V3))
    return false;

  auto result = TextAPIReader::canRead(memBufferRef);
  if (!result) {
    consumeError(result.takeError());
    return false;
  }
  return *result == FileType::TBD_V3;
}

FileType YAMLDocumentHandler::getFileType(MemoryBufferRef memBufferRef) const {
  if (canRead(memBufferRef))
    return FileType::TBD_V3;

  return FileType::Invalid;
}

bool YAMLDocumentHandler::canWrite(const InterfaceFile *file,
                                   FileType fileType) const {
  if (fileType != FileType::TBD_V3)
    return false;

  return true;
}

} // end namespace stub::v3

namespace stub::v4 {

bool YAMLDocumentHandler::canRead(MemoryBufferRef memBufferRef,
                                  FileType types) const {
  if (!(types & FileType::TBD_V4))
    return false;

  auto result = TextAPIReader::canRead(memBufferRef);
  if (!result) {
    consumeError(result.takeError());
    return false;
  }
  return *result == FileType::TBD_V4;
}

FileType YAMLDocumentHandler::getFileType(MemoryBufferRef memBufferRef) const {
  if (canRead(memBufferRef))
    return FileType::TBD_V4;

  return FileType::Invalid;
}

bool YAMLDocumentHandler::canWrite(const InterfaceFile *file,
                                   FileType fileType) const {
  if (fileType != FileType::TBD_V4)
    return false;

  return true;
}
} // end namespace stub::v4

bool YAMLBase::canRead(MemoryBufferRef memBufferRef, FileType types) const {
  for (const auto &handler : _documentHandlers) {
    if (handler->canRead(memBufferRef, types))
      return true;
  }
  return false;
}

bool YAMLBase::canWrite(const InterfaceFile *file, FileType fileType) const {
  for (const auto &handler : _documentHandlers) {
    if (handler->canWrite(file, fileType))
      return true;
  }
  return false;
}

FileType YAMLBase::getFileType(MemoryBufferRef bufferRef) const {
  for (const auto &handler : _documentHandlers) {
    auto fileType = handler->getFileType(bufferRef);
    if (fileType != FileType::Invalid)
      return fileType;
  }
  return FileType::Invalid;
}

bool YAMLBase::writeFile(raw_ostream &os, const InterfaceFile *file,
                         FileType fileType) const {
  for (const auto &handler : _documentHandlers) {
    if (handler->writeFile(os, file, fileType))
      return true;
  }
  return false;
}

bool YAMLReader::canRead(file_magic magic, MemoryBufferRef memBufferRef,
                         FileType types) const {
  return YAMLBase::canRead(memBufferRef, types);
}

Expected<FileType> YAMLReader::getFileType(file_magic magic,
                                           MemoryBufferRef memBufferRef) const {
  return YAMLBase::getFileType(memBufferRef);
}

void addInterfaceFileToAPIs(APIs &apis, const InterfaceFile *interface) {
  for (auto target : interface->targets()) {
    auto api = std::make_shared<API>(API(Triple(getTargetTripleName(target))));
    auto &binaryInfo = api->getBinaryInfo();
    binaryInfo.fileType = interface->getFileType();
    binaryInfo.currentVersion = interface->getCurrentVersion();
    binaryInfo.compatibilityVersion = interface->getCompatibilityVersion();
    binaryInfo.swiftABIVersion = interface->getSwiftABIVersion();
    binaryInfo.isTwoLevelNamespace = interface->isTwoLevelNamespace();
    binaryInfo.isAppExtensionSafe = interface->isApplicationExtensionSafe();
    binaryInfo.isOSLibNotForSharedCache = interface->isOSLibNotForSharedCache();
    binaryInfo.path = api->copyString(interface->getPath());
    binaryInfo.installName = api->copyString(interface->getInstallName());

    // Per target info.
    for (const auto &client : interface->allowableClients())
      if (client.hasTarget(target))
        binaryInfo.allowableClients.emplace_back(
            api->copyString(client.getInstallName()));
    for (const auto &reexport : interface->reexportedLibraries())
      if (reexport.hasTarget(target))
        binaryInfo.reexportedLibraries.emplace_back(
            api->copyString(reexport.getInstallName()));
    for (const auto &[targ, parent] : interface->umbrellas()) {
      if (targ == target) {
        binaryInfo.parentUmbrella = api->copyString(parent);
        break;
      }
    }

    apis.emplace_back(std::move(api));
  }

  // Because API relates ivar symbols to their owned class,
  // iterate through symbols in sorted order.
  std::vector<const MachO::Symbol *> orderedSyms(interface->symbols().begin(),
                                                 interface->symbols().end());
  llvm::sort(orderedSyms,
             [](const auto *lhs, const auto *rhs) { return *lhs < *rhs; });

  for (const auto &sym : orderedSyms) {
    for (auto &target : sym->targets()) {
      auto *api = find_if(apis, [&target, &interface](const auto &api) {
        auto nameOr = api->getInstallName();
        return target == api->getTarget() &&
               (nameOr && nameOr == interface->getInstallName());
      });
      if (api == apis.end())
        continue;

      const AvailabilityInfo avail;
      const APIAccess access{0};
      // Linkage from Text files can only be three possible linkages.
      APILinkage linkage;
      if (sym->isReexported())
        linkage = APILinkage::Reexported;
      else if (sym->isUndefined())
        linkage = APILinkage::External;
      else
        linkage = APILinkage::Exported;

      switch (sym->getKind()) {
      case EncodeKind::GlobalSymbol:
        (*api)->addGlobal(sym->getName(), sym->getFlags(), APILoc(), avail,
                          access, nullptr, GVKind::Unknown, linkage);
        continue;
      case EncodeKind::ObjectiveCClass:
        (*api)->addObjCInterface(
            sym->getName(), APILoc(), avail, access, linkage, {}, nullptr,
            ObjCIFSymbolKind::Class | ObjCIFSymbolKind::MetaClass);
        continue;
      case EncodeKind::ObjectiveCClassEHType: {
        (*api)->addObjCInterface(
            sym->getName(), APILoc(), avail, access, linkage, {}, nullptr,
            ObjCIFSymbolKind::Class | ObjCIFSymbolKind::MetaClass |
                ObjCIFSymbolKind::EHType);
        continue;
      }
      case EncodeKind::ObjectiveCInstanceVariable: {
        // Attempt to find super class.
        ObjCContainerRecord *container = (*api)->findContainer(sym->getName());
        auto [superClassName, ivar] = sym->getName().split('.');

        // If not found, create extension since there is no mapped class symbol.
        if (container == nullptr)
          container =
              (*api)->addObjCCategory(superClassName, {}, APILoc(),
                                      AvailabilityInfo(), access, nullptr);
        (*api)->addObjCInstanceVariable(
            container, ivar, APILoc(), avail, access,
            ObjCInstanceVariableRecord::AccessControl::None, linkage, nullptr);
      }
      }
    }
  }
}

Expected<APIs> YAMLReader::readFile(std::unique_ptr<MemoryBuffer> memBuffer,
                                    ReadFlags readFlags,
                                    llvm::MachO::ArchitectureSet arches) const {
  auto interfaceOrErr = TextAPIReader::get(memBuffer->getMemBufferRef());
  if (!interfaceOrErr)
    return interfaceOrErr.takeError();
  auto interface = std::move(*interfaceOrErr);

  APIs apis;
  addInterfaceFileToAPIs(apis, interface.get());
  for (auto &doc : interface->documents())
    addInterfaceFileToAPIs(apis, doc.get());
  return std::move(apis);
}

bool YAMLWriter::canWrite(const InterfaceFile *file, FileType fileType) const {
  return YAMLBase::canWrite(file, fileType);
}

Error YAMLWriter::writeFile(raw_ostream &os, const InterfaceFile *file,
                            FileType fileType) const {
  if (file == nullptr)
    return errorCodeToError(std::make_error_code(std::errc::invalid_argument));
  YAMLBase::writeFile(os, file, fileType);
  return Error::success();
}

TAPI_NAMESPACE_INTERNAL_END
