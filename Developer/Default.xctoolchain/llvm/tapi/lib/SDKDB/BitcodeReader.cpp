//===- tapi/SDKDB/BitcodeReader.cpp - TAPI SDKDB Bitcode Reader -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implementes SDKDB bitcode reader.
///
//===----------------------------------------------------------------------===//

#include "tapi/SDKDB/BitcodeReader.h"
#include "SDKDBBitcodeFormat.h"

#include "llvm/Bitcode/BitcodeConvenience.h"
#include "llvm/Bitstream/BitCodes.h"
#include "llvm/Bitstream/BitstreamReader.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/OnDiskHashTable.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

TAPI_NAMESPACE_INTERNAL_BEGIN

namespace {
class LibraryTableInfo {
  const char *identifiers;

public:
  LibraryTableInfo(const char *identifiers) : identifiers(identifiers) {}

  using internal_key_type = StringRef;
  using external_key_type = StringRef;
  using data_type = uint64_t;
  using hash_value_type = uint64_t;
  using offset_type = unsigned;

  // NOLINTNEXTLINE
  internal_key_type GetInternalKey(external_key_type key) { return key; }

  // NOLINTNEXTLINE
  external_key_type GetExternalKey(internal_key_type key) { return key; }

  // NOLINTNEXTLINE
  hash_value_type ComputeHash(internal_key_type key) { return hash_value(key); }

  // NOLINTNEXTLINE
  static bool EqualKey(internal_key_type lhs, internal_key_type rhs) {
    return lhs == rhs;
  }

  static std::pair<offset_type, offset_type> // NOLINTNEXTLINE
  ReadKeyDataLength(const uint8_t *&data) {
    // StringRef: offset=32 length=16.
    offset_type keyLength = sizeof(uint32_t) + sizeof(uint16_t);
    // Offset to library block.
    offset_type dataLength = sizeof(data_type);
    return {keyLength, dataLength};
  }

  // NOLINTNEXTLINE
  internal_key_type ReadKey(const uint8_t *data, offset_type KeyLen) {
    auto offset = support::endian::readNext<uint32_t, support::little,
                                            support::unaligned>(data);
    auto size = support::endian::readNext<uint16_t, support::little,
                                          support::unaligned>(data);
    auto name = StringRef(identifiers + offset, size);
    return name;
  }

  // NOLINTNEXTLINE
  static data_type ReadData(internal_key_type key, const uint8_t *data,
                            offset_type length) {
    return support::endian::readNext<data_type, support::little,
                                     support::unaligned>(data);
  }
};
} // end anonymous namespace

SDKDBBitcodeMaterializeOption SDKDBBitcodeMaterializeOption::defaultOption =
    SDKDBBitcodeMaterializeOption();

class SDKDBBitcodeReader::Implementation {
public:
  Implementation(MemoryBufferRef input, SDKDBBitcodeMaterializeOption &option,
                 Error &err);

  const std::vector<Triple> &getAvailableTriples() const { return triples; }

  Error materialize(SDKDBBuilder &builder) const;

  std::string getSDKDBVersion() const;

  std::string getBuildVersion() const;

  Expected<bool> dylibExistsForPath(Triple &target, StringRef path);

  Error loadAPIsFromSDKDB(SDKDBBuilder &builder, Triple &target,
                          StringRef path);

  const std::vector<std::string> &getProjectsWithError() const {
    return projectWithError;
  }

  bool isPublicOnly() const {
    return (bool)(builderOpts & SDKDBBuilderOptions::isPublicOnly);
  }

  bool noObjCMetadata() const {
    return (bool)(builderOpts & SDKDBBuilderOptions::noObjCMetadata);
  }

private:
  using SerializedLibraryTable =
      OnDiskIterableChainedHashTable<LibraryTableInfo>;

  // helper functions.
  // TODO: handle newly added fields in BitCode:
  //   - USR
  //   - docComment
  Error validateSDKDB();
  Error readTripleFromSDKDB(BitstreamCursor &cursor);
  Error readSignature(BitstreamCursor &cursor) const;
  Error readBlockInfoBlock(BitstreamCursor &cursor,
                           BitstreamBlockInfo &info) const;
  Error readControlBlock(BitstreamCursor &cursor);
  Error readIdentificationBlock(BitstreamCursor &cursor) const;
  Error readSDKDBBlock(BitstreamCursor &cursor, SDKDBBuilder &builder) const;
  Expected<API *> readAPIBlock(BitstreamCursor &cursor, SDKDB &sdkdb) const;
  Error readGlobalBlock(BitstreamCursor &cursor, API &api) const;
  Error readObjCClassBlock(BitstreamCursor &cursor, API &api) const;
  Error readObjCCategoryBlock(BitstreamCursor &cursor, API &api) const;
  Error readObjCProtocolBlock(BitstreamCursor &cursor, API &api) const;
  Error readObjCMethodBlock(BitstreamCursor &cursor, API &api,
                            ObjCContainerRecord &container) const;
  Error readObjCPropertyBlock(BitstreamCursor &cursor, API &api,
                              ObjCContainerRecord &container) const;
  Error readObjCInstanceVarBlock(BitstreamCursor &cursor, API &api,
                                 ObjCContainerRecord &container) const;
  Error readEnumBlock(BitstreamCursor &cursor, API &api) const;
  Error readEnumConstantBlock(BitstreamCursor &cursor, API &api,
                              EnumRecord *record) const;
  Error readTypedefBlock(BitstreamCursor &cursor, API &api) const;

  Expected<StringRef> readStringFromTable(unsigned offset, unsigned size) const;
  Expected<APIRecord> readAPIRecordFromScratch() const;
  Error readAvailabilityInfoFromScratch(APIRecord &record) const;
  Error readFilenameFromScratch(APIRecord &record) const;
  Error readSourceLocationFromScratch(APIRecord &record) const;

  Error materializeLibraryTable() const;
  Error readLibraryTableBlock(BitstreamCursor &cursor) const;

  // Look for offset of the library in the dylibTable.
  Expected<uint64_t> getOffsetForLibrary(Triple &target, StringRef path);
  // Look for offset of the library and taking fallback targets into
  // consideration.
  Expected<uint64_t> findOffsetForLibrary(Triple &target, StringRef path);

  MemoryBufferRef input;
  SDKDBBitcodeMaterializeOption &option;
  SDKDBBuilderOptions builderOpts;
  std::vector<Triple> triples;
  unsigned majorVersion;
  unsigned minorVersion;
  std::string buildVersion;
  std::vector<std::string> projectWithError;

  // stringTable.
  mutable StringRef stringTable;

  // lookupTable for dylibs
  mutable StringMap<std::unique_ptr<SerializedLibraryTable>> dylibTable;

  // scatch space.
  mutable SmallVector<uint64_t, 64> scratch;
};

SDKDBBitcodeReader::SDKDBBitcodeReader(MemoryBufferRef input,
                                       SDKDBBitcodeMaterializeOption &option,
                                       Error &err)
    : impl(*new SDKDBBitcodeReader::Implementation(input, option, err)) {}

SDKDBBitcodeReader::~SDKDBBitcodeReader() { delete &impl; }

Expected<std::unique_ptr<SDKDBBitcodeReader>>
SDKDBBitcodeReader::get(MemoryBufferRef input,
                        SDKDBBitcodeMaterializeOption &option) {
  Error error = Error::success();
  auto reader = std::make_unique<SDKDBBitcodeReader>(input, option, error);
  if (error)
    return std::move(error);

  return reader;
}

const std::vector<Triple> &SDKDBBitcodeReader::getAvailableTriples() const {
  return impl.getAvailableTriples();
}

const std::vector<std::string> &
SDKDBBitcodeReader::getProjectsWithError() const {
  return impl.getProjectsWithError();
}

Error SDKDBBitcodeReader::materialize(SDKDBBuilder &builder) const {
  return impl.materialize(builder);
}

std::string SDKDBBitcodeReader::getSDKDBVersion() const {
  return impl.getSDKDBVersion();
}

std::string SDKDBBitcodeReader::getBuildVersion() const {
  return impl.getBuildVersion();
}

Expected<bool> SDKDBBitcodeReader::dylibExistsForPath(Triple &target,
                                                      StringRef path) {
  return impl.dylibExistsForPath(target, path);
}

Error SDKDBBitcodeReader::loadAPIsFromSDKDB(SDKDBBuilder &builder,
                                            Triple &target, StringRef path) {
  return impl.loadAPIsFromSDKDB(builder, target, path);
}

std::string SDKDBBitcodeReader::Implementation::getSDKDBVersion() const {
  std::string version;
  raw_string_ostream ss(version);
  ss << majorVersion << "." << minorVersion;

  return ss.str();
}

std::string SDKDBBitcodeReader::Implementation::getBuildVersion() const {
  return buildVersion;
}

bool SDKDBBitcodeReader::isPublicOnly() const {
  return impl.isPublicOnly();
}

bool SDKDBBitcodeReader::noObjCMetadata() const {
  return impl.noObjCMetadata();
}

Error SDKDBBitcodeReader::Implementation::readSignature(
    BitstreamCursor &cursor) const {
  // Validate signature.
  for (auto byte : SDKDB_SIGNATURE) {
    if (cursor.AtEndOfStream()) {
      return make_error<StringError>("invalid binary store file",
                                     inconvertibleErrorCode());
    }
    auto result = cursor.Read(8);
    if (!result)
      return result.takeError();

    if (result.get() != byte) {
      return make_error<StringError>("invalid signature",
                                     inconvertibleErrorCode());
    }
  }
  return Error::success();
}

Error SDKDBBitcodeReader::Implementation::readBlockInfoBlock(
    BitstreamCursor &cursor, BitstreamBlockInfo &info) const {
  if (auto maybeBlockInfo = cursor.ReadBlockInfoBlock()) {
    if (*maybeBlockInfo) {
      info = std::move(**maybeBlockInfo);
      cursor.setBlockInfo(&info);
    }
  } else
    return maybeBlockInfo.takeError();

  return Error::success();
}

Error SDKDBBitcodeReader::Implementation::readControlBlock(
    BitstreamCursor &cursor) {
  if (auto err = cursor.EnterSubBlock(CONTROL_BLOCK_ID))
    return err;

  while (true) {
    auto maybeEntry = cursor.advance();
    if (!maybeEntry)
      return maybeEntry.takeError();
    auto entry = maybeEntry.get();
    switch (entry.Kind) {
    case BitstreamEntry::EndBlock:
      return Error::success();
    case BitstreamEntry::Error:
      return make_error<StringError>("error malformed entry",
                                     inconvertibleErrorCode());
    case BitstreamEntry::SubBlock: {
      // Unknown metadata sub-block, possibly for use by a future version of the
      // format.
      if (auto err = cursor.SkipBlock())
        return err;
      continue;
    }
    case BitstreamEntry::Record:
      break;
    }

    scratch.clear();
    StringRef blob;
    auto maybeKind = cursor.readRecord(entry.ID, scratch, &blob);
    if (!maybeKind)
      return maybeKind.takeError();
    unsigned kind = maybeKind.get();
    switch (kind) {
    case control_block::METADATA:
      majorVersion = scratch[0];
      minorVersion = scratch[1];
      builderOpts = SDKDBBuilderOptions(scratch[2]);
      buildVersion = blob.str();
      if (majorVersion != VERSION_MAJOR)
        return make_error<StringError>("unsupported major version number",
                                       inconvertibleErrorCode());
      if (minorVersion > VERSION_MINOR)
        return make_error<StringError>("unsupported minor version number",
                                       inconvertibleErrorCode());
      break;

    case control_block::PROJECT_WITH_ERROR:
      projectWithError.push_back(blob.str());
      break;

    default:
      // Unknown metadata record, possibly for use by a future version of the
      // format.
      continue;
    }
  } // while
}

Error SDKDBBitcodeReader::Implementation::readIdentificationBlock(
    BitstreamCursor &cursor) const {
  // If the identification block is already read, skip block.
  if (!stringTable.empty()) {
    if (auto err = cursor.SkipBlock())
      return err;
    return Error::success();
  }

  if (auto err = cursor.EnterSubBlock(IDENTIFIER_BLOCK_ID))
    return err;

  while (true) {
    auto maybeEntry = cursor.advance();
    if (!maybeEntry)
      return maybeEntry.takeError();
    auto entry = maybeEntry.get();
    switch (entry.Kind) {
    case BitstreamEntry::EndBlock:
      return Error::success();
    case BitstreamEntry::Error:
      return make_error<StringError>("error malformed entry",
                                     inconvertibleErrorCode());
    case BitstreamEntry::SubBlock: {
      // Unknown metadata sub-block, possibly for use by a future version of the
      // format.
      if (auto err = cursor.SkipBlock())
        return err;
      continue;
    }
    case BitstreamEntry::Record:
      break;
    }

    scratch.clear();
    StringRef tempStr;
    auto maybeKind = cursor.readRecord(entry.ID, scratch, &tempStr);
    if (!maybeKind)
      return maybeKind.takeError();
    unsigned kind = maybeKind.get();
    switch (kind) {
    case identifier_block::STRING_TABLE:
      stringTable = tempStr;
      break;

    default:
      // Unknown metadata record, possibly for use by a future version of the
      // format.
      continue;
    }
  } // while
}

Error SDKDBBitcodeReader::Implementation::readTripleFromSDKDB(
    BitstreamCursor &cursor) {
  if (auto err = cursor.EnterSubBlock(SDKDB_BLOCK_ID))
    return err;

  while (true) {
    auto maybeEntry = cursor.advance();
    if (!maybeEntry)
      return maybeEntry.takeError();
    auto entry = maybeEntry.get();
    switch (entry.Kind) {
    case BitstreamEntry::EndBlock:
      return Error::success();
    case BitstreamEntry::Error:
      return make_error<StringError>("error malformed entry",
                                     inconvertibleErrorCode());
    case BitstreamEntry::SubBlock: {
      // Skip all the API blocks during triple extracting.
      if (auto err = cursor.SkipBlock())
        return err;
      continue;
    }
    case BitstreamEntry::Record:
      break;
    }

    scratch.clear();
    StringRef tripleStr;
    auto maybeKind = cursor.readRecord(entry.ID, scratch, &tripleStr);
    if (!maybeKind)
      return maybeKind.takeError();
    unsigned kind = maybeKind.get();
    switch (kind) {
    case sdkdb_block::TARGET_TRIPLE: {
      Triple triple(tripleStr);
      assert(llvm::find_if(triples,
                           [&](const Triple &target) {
                             return SDKDB::areCompatibleTargets(triple, target);
                           }) == triples.end() &&
             "Triples are not unqiue in SDKDB");
      triples.emplace_back(triple);
      break;
    }

    default:
      // Unknown metadata record, possibly for use by a future version of the
      // format.
      continue;
    }
  } // while
}

Error SDKDBBitcodeReader::Implementation::validateSDKDB() {
  BitstreamCursor cursor(input);
  BitstreamBlockInfo blockInfo;

  if (auto err = readSignature(cursor))
    return err;

  while (!cursor.AtEndOfStream()) {
    auto maybeTopLevelEntry = cursor.advance();
    if (!maybeTopLevelEntry)
      return maybeTopLevelEntry.takeError();

    auto topLevelEntry = maybeTopLevelEntry.get();
    if (topLevelEntry.Kind != BitstreamEntry::SubBlock)
      break;

    switch (topLevelEntry.ID) {
    case bitc::BLOCKINFO_BLOCK_ID: {
      if (auto err = readBlockInfoBlock(cursor, blockInfo))
        return err;

      break;
    }

    case CONTROL_BLOCK_ID: {
      if (auto err = readControlBlock(cursor))
        return err;
      break;
    }

    case SDKDB_BLOCK_ID: {
      if (auto err = readTripleFromSDKDB(cursor))
        return err;
      break;
    }

    default: { // Skip all the other blocks.
      if (auto err = cursor.SkipBlock())
        return err;
      break;
    }
    }
  }

  return Error::success();
}

Expected<API *>
SDKDBBitcodeReader::Implementation::readAPIBlock(BitstreamCursor &cursor,
                                                 SDKDB &sdkdb) const {
  if (auto err = cursor.EnterSubBlock(API_BLOCK_ID))
    return std::move(err);

  bool skipBlock = !option.installNames.empty();
  API api(sdkdb.getTargetTriple());
  while (true) {
    auto maybeEntry = cursor.advance();
    if (!maybeEntry)
      return maybeEntry.takeError();
    auto entry = maybeEntry.get();

    switch (entry.Kind) {
    case BitstreamEntry::Error:
      return make_error<StringError>("error malformed entry",
                                     inconvertibleErrorCode());
    case BitstreamEntry::Record: {
      scratch.clear();
      StringRef maybeUUID;
      auto maybeKind = cursor.readRecord(entry.ID, scratch, &maybeUUID);
      if (!maybeKind)
        return maybeKind.takeError();
      unsigned kind = maybeKind.get();
      switch (kind) {
      case api_block::INSTALL_NAME: {
        unsigned offset = scratch[0];
        unsigned size = scratch[1];
        if (auto installName = readStringFromTable(offset, size)) {
          if (option.installNames.count(*installName))
            skipBlock &= false;
          auto name = api.copyString(*installName);
          api.getBinaryInfo().installName = name;
        } else
          return installName.takeError();
        continue;
      }
      case api_block::FILE_TYPE:
        api.getBinaryInfo().fileType = (FileType)scratch[0];
        continue;
      case api_block::UUID: {
        auto uuid = api.copyString(maybeUUID);
        api.getBinaryInfo().uuid = uuid;
        continue;
      }
      case api_block::REEXPORTED: {
        unsigned offset = scratch[0];
        unsigned size = scratch[1];
        if (auto lib = readStringFromTable(offset, size)) {
          auto name = api.copyString(*lib);
          api.getBinaryInfo().reexportedLibraries.push_back(name);
        } else
          return lib.takeError();
        continue;
      }
      case api_block::PARENT_UMBRELLA: {
        unsigned offset = scratch[0];
        unsigned size = scratch[1];
        if (auto lib = readStringFromTable(offset, size)) {
          auto name = api.copyString(*lib);
          api.getBinaryInfo().parentUmbrella = name;
        } else
          return lib.takeError();
        continue;
      }
      case api_block::DYLIB_VERSION:
        api.getBinaryInfo().currentVersion = scratch[0];
        api.getBinaryInfo().compatibilityVersion = scratch[1];
        continue;
      case api_block::FLAGS:
        api.getBinaryInfo().isTwoLevelNamespace = (bool)scratch[0];
        api.getBinaryInfo().isAppExtensionSafe = (bool)scratch[1];
        continue;
      case api_block::SWIFT_VERSION:
        api.getBinaryInfo().swiftABIVersion = scratch[0];
        continue;
      case api_block::POTENTIALLY_DEFINED_SELECTOR: {
        unsigned offset = scratch[0];
        unsigned size = scratch[1];
        if (auto sym = readStringFromTable(offset, size))
          api.addPotentiallyDefinedSelector(*sym);
        else
          return sym.takeError();
        continue;
      }
      case api_block::PROJECT_NAME: {
        unsigned offset = scratch[0];
        unsigned size = scratch[1];
        if (auto sym = readStringFromTable(offset, size))
          api.setProjectName(*sym);
        else
          return sym.takeError();
        continue;
      }
      default:
        // Unknown record, possibly for use by a future version of the  format.
        continue;
      }
      continue;
    }
    case BitstreamEntry::SubBlock: {
      if (skipBlock) {
        if (auto err = cursor.SkipBlock())
          return std::move(err);
        continue;
      }
      switch (maybeEntry->ID) {
      case GLOBAL_BLOCK_ID:
        if (auto err = readGlobalBlock(cursor, api))
          return std::move(err);
        continue;
      case OBJC_CLASS_BLOCK_ID:
        if (auto err = readObjCClassBlock(cursor, api))
          return std::move(err);
        continue;
      case OBJC_CATEGORY_BLOCK_ID:
        if (auto err = readObjCCategoryBlock(cursor, api))
          return std::move(err);
        continue;
      case OBJC_PROTOCOL_BLOCK_ID:
        if (auto err = readObjCProtocolBlock(cursor, api))
          return std::move(err);
        continue;
      case ENUM_BLOCK_ID:
        if (auto err = readEnumBlock(cursor, api))
          return std::move(err);
        continue;
      case TYPEDEF_BLOCK_ID:
        if (auto err = readTypedefBlock(cursor, api))
          return std::move(err);
        continue;
      default:
        if (auto err = cursor.SkipBlock())
          return std::move(err);
        continue;
      }
    }
    case BitstreamEntry::EndBlock:
      if (!skipBlock)
        return &sdkdb.recordAPI(std::move(api));
      return nullptr;
    }
  }
}

Expected<APIRecord>
SDKDBBitcodeReader::Implementation::readAPIRecordFromScratch() const {
  if (scratch.size() < 5)
    return make_error<StringError>("scratch entry is too small for APIRecord",
                                   inconvertibleErrorCode());

  // Default values for APIRecord.
  APIRecord record{
      "",
      APILoc(),
      /*Decl=*/nullptr,
      AvailabilityInfo(),
      APILinkage::Unknown,
      SymbolFlags::None,
      APIAccess::Unknown,
  };

  unsigned offset = scratch[0];
  unsigned size = scratch[1];
  if (auto nameStr = readStringFromTable(offset, size))
    record.name = *nameStr;
  else
    return nameStr.takeError();

  record.access = (APIAccess)scratch[2];
  record.linkage = (APILinkage)scratch[3];
  record.flags = (SymbolFlags)scratch[4];
  return record;
}

Error SDKDBBitcodeReader::Implementation::readAvailabilityInfoFromScratch(
    APIRecord &record) const {
  if (scratch.size() < 4)
    return make_error<StringError>(
        "scratch entry is too small for availability",
        inconvertibleErrorCode());
  record.availability = {scratch[0],       0,     scratch[1],
                         (bool)scratch[2], false, (bool)scratch[3]};

  return Error::success();
}

Error SDKDBBitcodeReader::Implementation::readFilenameFromScratch(
    APIRecord &record) const {
  if (scratch.size() < 2)
    return make_error<StringError>("scratch entry is too small for filename",
                                   inconvertibleErrorCode());
  unsigned offset = scratch[0];
  unsigned size = scratch[1];
  if (auto file = readStringFromTable(offset, size))
    record.loc =
        APILoc(file->str(), record.loc.getLine(), record.loc.getColumn());

  return Error::success();
}

Error SDKDBBitcodeReader::Implementation::readSourceLocationFromScratch(
    APIRecord &record) const {
  if (scratch.size() < 2)
    return make_error<StringError>("scratch entry is too small for line & col",
                                   inconvertibleErrorCode());
  record.loc = APILoc(record.loc.getFilename(), scratch[0], scratch[1]);
  return Error::success();
}

Error SDKDBBitcodeReader::Implementation::readGlobalBlock(
    BitstreamCursor &cursor, API &api) const {
  if (auto err = cursor.EnterSubBlock(GLOBAL_BLOCK_ID))
    return err;

  GlobalRecord *record = nullptr;
  while (true) {
    auto maybeEntry = cursor.advance();
    if (!maybeEntry)
      return maybeEntry.takeError();
    auto entry = maybeEntry.get();

    switch (entry.Kind) {
    case BitstreamEntry::Error:
      return make_error<StringError>("error malformed entry",
                                     inconvertibleErrorCode());
    case BitstreamEntry::Record: {
      scratch.clear();
      auto maybeKind = cursor.readRecord(entry.ID, scratch);
      if (!maybeKind)
        return maybeKind.takeError();
      unsigned kind = maybeKind.get();
      if (kind != global_block::INFO && !record)
        return make_error<StringError>("wrong record ordering in global block",
                                       inconvertibleErrorCode());
      switch (kind) {
      case global_block::INFO: {
        auto apiRecord = readAPIRecordFromScratch();
        if (!apiRecord)
          return apiRecord.takeError();
        record = api.addGlobal(
            apiRecord->name, apiRecord->flags, apiRecord->loc,
            apiRecord->availability, apiRecord->access,
            /*Decl=*/nullptr, (GVKind)scratch[5], apiRecord->linkage);
        continue;
      }
      case global_block::AVAILABILITY: {
        if (auto err = readAvailabilityInfoFromScratch(*record))
          return err;
        continue;
      }
      case global_block::FILENAME: {
        if (auto err = readFilenameFromScratch(*record))
          return err;
        continue;
      }
      case global_block::LOCATION: {
        if (auto err = readSourceLocationFromScratch(*record))
          return err;
        continue;
      }
      default:
        // Unknown record, possibly for use by a future version of the format.
        continue;
      }
      continue;
    }
    case BitstreamEntry::SubBlock: {
      if (auto err = cursor.SkipBlock())
        return err;
      continue;
    }
    case BitstreamEntry::EndBlock:
      return Error::success();
    }
  }
}

Error SDKDBBitcodeReader::Implementation::readObjCClassBlock(
    BitstreamCursor &cursor, API &api) const {
  if (auto err = cursor.EnterSubBlock(OBJC_CLASS_BLOCK_ID))
    return err;

  ObjCInterfaceRecord *record = nullptr;
  while (true) {
    auto maybeEntry = cursor.advance();
    if (!maybeEntry)
      return maybeEntry.takeError();
    auto entry = maybeEntry.get();

    switch (entry.Kind) {
    case BitstreamEntry::Error:
      return make_error<StringError>("error malformed entry",
                                     inconvertibleErrorCode());
    case BitstreamEntry::Record: {
      scratch.clear();
      auto maybeKind = cursor.readRecord(entry.ID, scratch);
      if (!maybeKind)
        return maybeKind.takeError();
      unsigned kind = maybeKind.get();
      if (kind != objc_class_block::INFO && !record)
        return make_error<StringError>(
            "wrong record ordering in objc_class block",
            inconvertibleErrorCode());
      switch (kind) {
      case objc_class_block::INFO: {
        auto apiRecord = readAPIRecordFromScratch();
        if (!apiRecord)
          return apiRecord.takeError();
        unsigned offset = scratch[5];
        unsigned size = scratch[6];
        auto superName = readStringFromTable(offset, size);
        if (!superName)
          return superName.takeError();
        ObjCIFSymbolKind symType =
            ObjCIFSymbolKind::Class | ObjCIFSymbolKind::MetaClass;
        // Old format might not have the exception attribute.
        bool hasExceptionAttribute =
            scratch.size() < 8 ? false : (bool)scratch[7];
        if (hasExceptionAttribute)
          symType |= ObjCIFSymbolKind::EHType;
        record = api.addObjCInterface(apiRecord->name, apiRecord->loc,
                                      apiRecord->availability,
                                      apiRecord->access, apiRecord->linkage,
                                      *superName, /*Decl=*/nullptr, symType);
        continue;
      }
      case objc_class_block::AVAILABILITY: {
        if (auto err = readAvailabilityInfoFromScratch(*record))
          return err;
        continue;
      }
      case objc_class_block::FILENAME: {
        if (auto err = readFilenameFromScratch(*record))
          return err;
        continue;
      }
      case objc_class_block::LOCATION: {
        if (auto err = readSourceLocationFromScratch(*record))
          return err;
        continue;
      }
      case objc_class_block::PROTOCOL: {
        if (auto protocol = readStringFromTable(scratch[0], scratch[1])) {
          auto str = api.copyString(*protocol);
          record->protocols.emplace_back(str);
        } else
          return protocol.takeError();
        continue;
      }
      default:
        // Unknown record, possibly for use by a future version of the format.
        continue;
      }
      continue;
    }
    case BitstreamEntry::SubBlock: {
      if (!record)
        return make_error<StringError>("wrong ordering in objc_class block",
                                       inconvertibleErrorCode());
      switch (maybeEntry->ID) {
      case OBJC_METHOD_BLOCK_ID: {
        if (auto err = readObjCMethodBlock(cursor, api, *record))
          return err;
        continue;
      }
      case OBJC_PROPERTY_BLOCK_ID: {
        if (auto err = readObjCPropertyBlock(cursor, api, *record))
          return err;
        continue;
      }
      case OBJC_IVAR_BLOCK_ID: {
        if (auto err = readObjCInstanceVarBlock(cursor, api, *record))
          return err;
        continue;
      }
      default:
        if (auto err = cursor.SkipBlock())
          return err;
        continue;
      }
    }
    case BitstreamEntry::EndBlock:
      return Error::success();
    }
  }
}

Error SDKDBBitcodeReader::Implementation::readObjCCategoryBlock(
    BitstreamCursor &cursor, API &api) const {
  if (auto err = cursor.EnterSubBlock(OBJC_CATEGORY_BLOCK_ID))
    return err;

  ObjCCategoryRecord *record = nullptr;
  while (true) {
    auto maybeEntry = cursor.advance();
    if (!maybeEntry)
      return maybeEntry.takeError();
    auto entry = maybeEntry.get();

    switch (entry.Kind) {
    case BitstreamEntry::Error:
      return make_error<StringError>("error malformed entry",
                                     inconvertibleErrorCode());
    case BitstreamEntry::Record: {
      scratch.clear();
      auto maybeKind = cursor.readRecord(entry.ID, scratch);
      if (!maybeKind)
        return maybeKind.takeError();
      unsigned kind = maybeKind.get();
      if (kind != objc_category_block::INFO && !record)
        return make_error<StringError>(
            "wrong record ordering in objc_category block",
            inconvertibleErrorCode());
      switch (kind) {
      case objc_category_block::INFO: {
        auto apiRecord = readAPIRecordFromScratch();
        if (!apiRecord)
          return apiRecord.takeError();
        unsigned offset = scratch[5];
        unsigned size = scratch[6];
        auto interfaceName = readStringFromTable(offset, size);
        if (!interfaceName)
          return interfaceName.takeError();
        record =
            api.addObjCCategory(*interfaceName, apiRecord->name, apiRecord->loc,
                                apiRecord->availability, apiRecord->access,
                                /*Decl=*/nullptr);
        continue;
      }
      case objc_category_block::AVAILABILITY: {
        if (auto err = readAvailabilityInfoFromScratch(*record))
          return err;
        continue;
      }
      case objc_category_block::FILENAME: {
        if (auto err = readFilenameFromScratch(*record))
          return err;
        continue;
      }
      case objc_category_block::LOCATION: {
        if (auto err = readSourceLocationFromScratch(*record))
          return err;
        continue;
      }
      case objc_category_block::PROTOCOL: {
        if (auto protocol = readStringFromTable(scratch[0], scratch[1])) {
          auto str = api.copyString(*protocol);
          record->protocols.emplace_back(str);
        } else
          return protocol.takeError();
        continue;
      }
      default:
        // Unknown record, possibly for use by a future version of the format.
        continue;
      }
      continue;
    }
    case BitstreamEntry::SubBlock: {
      if (!record)
        return make_error<StringError>("wrong ordering in objc_category block",
                                       inconvertibleErrorCode());
      switch (maybeEntry->ID) {
      case OBJC_METHOD_BLOCK_ID: {
        if (auto err = readObjCMethodBlock(cursor, api, *record))
          return err;
        continue;
      }
      case OBJC_PROPERTY_BLOCK_ID: {
        if (auto err = readObjCPropertyBlock(cursor, api, *record))
          return err;
        continue;
      }
      case OBJC_IVAR_BLOCK_ID: {
        if (auto err = readObjCInstanceVarBlock(cursor, api, *record))
          return err;
        continue;
      }
      default:
        if (auto err = cursor.SkipBlock())
          return err;
        continue;
      }
    }
    case BitstreamEntry::EndBlock:
      return Error::success();
    }
  }
}

Error SDKDBBitcodeReader::Implementation::readObjCProtocolBlock(
    BitstreamCursor &cursor, API &api) const {
  if (auto err = cursor.EnterSubBlock(OBJC_PROTOCOL_BLOCK_ID))
    return err;

  ObjCProtocolRecord *record = nullptr;
  while (true) {
    auto maybeEntry = cursor.advance();
    if (!maybeEntry)
      return maybeEntry.takeError();
    auto entry = maybeEntry.get();

    switch (entry.Kind) {
    case BitstreamEntry::Error:
      return make_error<StringError>("error malformed entry",
                                     inconvertibleErrorCode());
    case BitstreamEntry::Record: {
      scratch.clear();
      auto maybeKind = cursor.readRecord(entry.ID, scratch);
      if (!maybeKind)
        return maybeKind.takeError();
      unsigned kind = maybeKind.get();
      if (kind != objc_protocol_block::INFO && !record)
        return make_error<StringError>(
            "wrong record ordering in objc_protocol block",
            inconvertibleErrorCode());
      switch (kind) {
      case objc_protocol_block::INFO: {
        auto apiRecord = readAPIRecordFromScratch();
        if (!apiRecord)
          return apiRecord.takeError();
        record = api.addObjCProtocol(apiRecord->name, apiRecord->loc,
                                     apiRecord->availability, apiRecord->access,
                                     /*Decl=*/nullptr);
        continue;
      }
      case objc_protocol_block::AVAILABILITY: {
        if (auto err = readAvailabilityInfoFromScratch(*record))
          return err;
        continue;
      }
      case objc_protocol_block::FILENAME: {
        if (auto err = readFilenameFromScratch(*record))
          return err;
        continue;
      }
      case objc_protocol_block::LOCATION: {
        if (auto err = readSourceLocationFromScratch(*record))
          return err;
        continue;
      }
      case objc_protocol_block::PROTOCOL: {
        if (auto protocol = readStringFromTable(scratch[0], scratch[1])) {
          auto str = api.copyString(*protocol);
          record->protocols.emplace_back(str);
        } else
          return protocol.takeError();
        continue;
      }
      default:
        // Unknown record, possibly for use by a future version of the format.
        continue;
      }
      continue;
    }
    case BitstreamEntry::SubBlock: {
      if (!record)
        return make_error<StringError>("wrong ordering in objc_protocol block",
                                       inconvertibleErrorCode());
      switch (maybeEntry->ID) {
      case OBJC_METHOD_BLOCK_ID: {
        if (auto err = readObjCMethodBlock(cursor, api, *record))
          return err;
        continue;
      }
      case OBJC_PROPERTY_BLOCK_ID: {
        if (auto err = readObjCPropertyBlock(cursor, api, *record))
          return err;
        continue;
      }
      case OBJC_IVAR_BLOCK_ID: {
        if (auto err = readObjCInstanceVarBlock(cursor, api, *record))
          return err;
        continue;
      }
      default:
        if (auto err = cursor.SkipBlock())
          return err;
        continue;
      }
    }
    case BitstreamEntry::EndBlock:
      return Error::success();
    }
  }
}

Error SDKDBBitcodeReader::Implementation::readObjCMethodBlock(
    BitstreamCursor &cursor, API &api, ObjCContainerRecord &container) const {
  if (auto err = cursor.EnterSubBlock(OBJC_METHOD_BLOCK_ID))
    return err;

  ObjCMethodRecord *record = nullptr;
  while (true) {
    auto maybeEntry = cursor.advance();
    if (!maybeEntry)
      return maybeEntry.takeError();
    auto entry = maybeEntry.get();

    switch (entry.Kind) {
    case BitstreamEntry::Error:
      return make_error<StringError>("error malformed entry",
                                     inconvertibleErrorCode());
    case BitstreamEntry::Record: {
      scratch.clear();
      auto maybeKind = cursor.readRecord(entry.ID, scratch);
      if (!maybeKind)
        return maybeKind.takeError();
      unsigned kind = maybeKind.get();
      if (kind != objc_method_block::INFO && !record)
        return make_error<StringError>("wrong record ordering in method block",
                                       inconvertibleErrorCode());
      switch (kind) {
      case objc_method_block::INFO: {
        auto apiRecord = readAPIRecordFromScratch();
        if (!apiRecord)
          return apiRecord.takeError();
        record = api.addObjCMethod(&container, apiRecord->name, apiRecord->loc,
                                   apiRecord->availability, apiRecord->access,
                                   (bool)scratch[5], (bool)scratch[6],
                                   (bool)scratch[7],
                                   /*Decl=*/nullptr);
        continue;
      }
      case objc_method_block::AVAILABILITY: {
        if (auto err = readAvailabilityInfoFromScratch(*record))
          return err;
        continue;
      }
      case objc_method_block::FILENAME: {
        if (auto err = readFilenameFromScratch(*record))
          return err;
        continue;
      }
      case objc_method_block::LOCATION: {
        if (auto err = readSourceLocationFromScratch(*record))
          return err;
        continue;
      }
      default:
        // Unknown record, possibly for use by a future version of the format.
        continue;
      }
      continue;
    }
    case BitstreamEntry::SubBlock: {
      if (auto err = cursor.SkipBlock())
        return err;
      continue;
    }
    case BitstreamEntry::EndBlock:
      return Error::success();
    }
  }
}

Error SDKDBBitcodeReader::Implementation::readObjCPropertyBlock(
    BitstreamCursor &cursor, API &api, ObjCContainerRecord &container) const {
  if (auto err = cursor.EnterSubBlock(OBJC_PROPERTY_BLOCK_ID))
    return err;

  ObjCPropertyRecord *record = nullptr;
  while (true) {
    auto maybeEntry = cursor.advance();
    if (!maybeEntry)
      return maybeEntry.takeError();
    auto entry = maybeEntry.get();

    switch (entry.Kind) {
    case BitstreamEntry::Error:
      return make_error<StringError>("error malformed entry",
                                     inconvertibleErrorCode());
    case BitstreamEntry::Record: {
      scratch.clear();
      auto maybeKind = cursor.readRecord(entry.ID, scratch);
      if (!maybeKind)
        return maybeKind.takeError();
      unsigned kind = maybeKind.get();
      if (kind != objc_property_block::INFO && !record)
        return make_error<StringError>(
            "wrong record ordering in property block",
            inconvertibleErrorCode());
      switch (kind) {
      case objc_property_block::INFO: {
        auto apiRecord = readAPIRecordFromScratch();
        if (!apiRecord)
          return apiRecord.takeError();
        auto getter = readStringFromTable(scratch[7], scratch[8]);
        if (!getter)
          return getter.takeError();
        auto setter = readStringFromTable(scratch[9], scratch[10]);
        if (!setter)
          return setter.takeError();
        record = api.addObjCProperty(
            &container, apiRecord->name, *getter, *setter, apiRecord->loc,
            apiRecord->availability, apiRecord->access,
            (ObjCPropertyRecord::AttributeKind)scratch[5], (bool)scratch[6],
            /*Decl=*/nullptr);
        continue;
      }
      case objc_property_block::AVAILABILITY: {
        if (auto err = readAvailabilityInfoFromScratch(*record))
          return err;
        continue;
      }
      case objc_property_block::FILENAME: {
        if (auto err = readFilenameFromScratch(*record))
          return err;
        continue;
      }
      case objc_property_block::LOCATION: {
        if (auto err = readSourceLocationFromScratch(*record))
          return err;
        continue;
      }
      default:
        // Unknown record, possibly for use by a future version of the format.
        continue;
      }
      continue;
    }
    case BitstreamEntry::SubBlock: {
      if (auto err = cursor.SkipBlock())
        return err;
      continue;
    }
    case BitstreamEntry::EndBlock:
      return Error::success();
    }
  }
}

Error SDKDBBitcodeReader::Implementation::readObjCInstanceVarBlock(
    BitstreamCursor &cursor, API &api, ObjCContainerRecord &container) const {
  if (auto err = cursor.EnterSubBlock(OBJC_IVAR_BLOCK_ID))
    return err;

  ObjCInstanceVariableRecord *record = nullptr;
  while (true) {
    auto maybeEntry = cursor.advance();
    if (!maybeEntry)
      return maybeEntry.takeError();
    auto entry = maybeEntry.get();

    switch (entry.Kind) {
    case BitstreamEntry::Error:
      return make_error<StringError>("error malformed entry",
                                     inconvertibleErrorCode());
    case BitstreamEntry::Record: {
      scratch.clear();
      auto maybeKind = cursor.readRecord(entry.ID, scratch);
      if (!maybeKind)
        return maybeKind.takeError();
      unsigned kind = maybeKind.get();
      if (kind != objc_ivar_block::INFO && !record)
        return make_error<StringError>("wrong record ordering in ivar block",
                                       inconvertibleErrorCode());
      switch (kind) {
      case objc_ivar_block::INFO: {
        auto apiRecord = readAPIRecordFromScratch();
        if (!apiRecord)
          return apiRecord.takeError();
        // Do not emit an error, because old SDKDB formats are broken and don't
        // have all required fields. Use an default for AccessControl instead.
        ObjCInstanceVariableRecord::AccessControl control =
            scratch.size() < 6
                ? ObjCInstanceVariableRecord::AccessControl::None
                : (ObjCInstanceVariableRecord::AccessControl)scratch[5];
        record = api.addObjCInstanceVariable(
            &container, apiRecord->name, apiRecord->loc,
            apiRecord->availability, apiRecord->access, control,
            apiRecord->linkage, /*Decl=*/nullptr);
        continue;
      }
      case objc_ivar_block::AVAILABILITY: {
        if (auto err = readAvailabilityInfoFromScratch(*record))
          return err;
        continue;
      }
      case objc_ivar_block::FILENAME: {
        if (auto err = readFilenameFromScratch(*record))
          return err;
        continue;
      }
      case objc_ivar_block::LOCATION: {
        if (auto err = readSourceLocationFromScratch(*record))
          return err;
        continue;
      }
      default:
        // Unknown record, possibly for use by a future version of the format.
        continue;
      }
      continue;
    }
    case BitstreamEntry::SubBlock: {
      if (auto err = cursor.SkipBlock())
        return err;
      continue;
    }
    case BitstreamEntry::EndBlock:
      return Error::success();
    }
  }
}

Error SDKDBBitcodeReader::Implementation::readEnumBlock(BitstreamCursor &cursor,
                                                        API &api) const {
  if (auto err = cursor.EnterSubBlock(ENUM_BLOCK_ID))
    return err;

  EnumRecord *record = nullptr;
  while (true) {
    auto maybeEntry = cursor.advance();
    if (!maybeEntry)
      return maybeEntry.takeError();
    auto entry = maybeEntry.get();

    switch (entry.Kind) {
    case BitstreamEntry::Error:
      return make_error<StringError>("error malformed entry",
                                     inconvertibleErrorCode());
    case BitstreamEntry::Record: {
      scratch.clear();
      auto maybeKind = cursor.readRecord(entry.ID, scratch);
      if (!maybeKind)
        return maybeKind.takeError();
      unsigned kind = maybeKind.get();
      if (kind != enum_block::INFO && !record)
        return make_error<StringError>("wrong record ordering in enum block",
                                       inconvertibleErrorCode());
      switch (kind) {
      case enum_block::INFO: {
        auto apiRecord = readAPIRecordFromScratch();
        if (!apiRecord)
          return apiRecord.takeError();
        unsigned offset = scratch[5];
        unsigned size = scratch[6];
        if (auto usrStr = readStringFromTable(offset, size))
          record = api.addEnum(apiRecord->name, *usrStr, apiRecord->loc,
                               apiRecord->availability, apiRecord->access,
                               /*Decl=*/nullptr);
        else
          return usrStr.takeError();

        continue;
      }
      case enum_block::AVAILABILITY: {
        if (auto err = readAvailabilityInfoFromScratch(*record))
          return err;
        continue;
      }
      case enum_block::FILENAME: {
        if (auto err = readFilenameFromScratch(*record))
          return err;
        continue;
      }
      case enum_block::LOCATION: {
        if (auto err = readSourceLocationFromScratch(*record))
          return err;
        continue;
      }
      default:
        // Unknown record, possibly for use by a future version of the format.
        continue;
      }
      continue;
    }
    case BitstreamEntry::SubBlock: {
      if (!record)
        return make_error<StringError>("wrong ordering in enum block",
                                       inconvertibleErrorCode());
      switch (maybeEntry->ID) {
      case ENUM_CONSTANT_BLOCK_ID: {
        if (auto err = readEnumConstantBlock(cursor, api, record))
          return err;
        continue;
      }
      default:
        if (auto err = cursor.SkipBlock())
          return err;
        continue;
      }
    }
    case BitstreamEntry::EndBlock:
      return Error::success();
    }
  }
}

Error SDKDBBitcodeReader::Implementation::readEnumConstantBlock(
    BitstreamCursor &cursor, API &api, EnumRecord *parent) const {
  if (auto err = cursor.EnterSubBlock(ENUM_CONSTANT_BLOCK_ID))
    return err;

  APIRecord *record = nullptr;
  while (true) {
    auto maybeEntry = cursor.advance();
    if (!maybeEntry)
      return maybeEntry.takeError();
    auto entry = maybeEntry.get();

    switch (entry.Kind) {
    case BitstreamEntry::Error:
      return make_error<StringError>("error malformed entry",
                                     inconvertibleErrorCode());
    case BitstreamEntry::Record: {
      scratch.clear();
      auto maybeKind = cursor.readRecord(entry.ID, scratch);
      if (!maybeKind)
        return maybeKind.takeError();
      unsigned kind = maybeKind.get();
      if (kind != enum_constant_block::INFO && !record)
        return make_error<StringError>(
            "wrong record ordering in enum constant block",
            inconvertibleErrorCode());
      switch (kind) {
      case enum_constant_block::INFO: {
        auto apiRecord = readAPIRecordFromScratch();
        if (!apiRecord)
          return apiRecord.takeError();
        record = api.addEnumConstant(parent, apiRecord->name, apiRecord->loc,
                                     apiRecord->availability, apiRecord->access,
                                     /*Decl=*/nullptr);
        continue;
      }
      case enum_constant_block::AVAILABILITY: {
        if (auto err = readAvailabilityInfoFromScratch(*record))
          return err;
        continue;
      }
      case enum_constant_block::FILENAME: {
        if (auto err = readFilenameFromScratch(*record))
          return err;
        continue;
      }
      case enum_constant_block::LOCATION: {
        if (auto err = readSourceLocationFromScratch(*record))
          return err;
        continue;
      }
      default:
        // Unknown record, possibly for use by a future version of the format.
        continue;
      }
      continue;
    }
    case BitstreamEntry::SubBlock: {
      if (auto err = cursor.SkipBlock())
        return err;
      continue;
    }
    case BitstreamEntry::EndBlock:
      return Error::success();
    }
  }
}

Error SDKDBBitcodeReader::Implementation::readTypedefBlock(
    BitstreamCursor &cursor, API &api) const {
  if (auto err = cursor.EnterSubBlock(TYPEDEF_BLOCK_ID))
    return err;

  APIRecord *record = nullptr;
  while (true) {
    auto maybeEntry = cursor.advance();
    if (!maybeEntry)
      return maybeEntry.takeError();
    auto entry = maybeEntry.get();

    switch (entry.Kind) {
    case BitstreamEntry::Error:
      return make_error<StringError>("error malformed entry",
                                     inconvertibleErrorCode());
    case BitstreamEntry::Record: {
      scratch.clear();
      auto maybeKind = cursor.readRecord(entry.ID, scratch);
      if (!maybeKind)
        return maybeKind.takeError();
      unsigned kind = maybeKind.get();
      if (kind != typedef_block::INFO && !record)
        return make_error<StringError>("wrong record ordering in typedef block",
                                       inconvertibleErrorCode());
      switch (kind) {
      case typedef_block::INFO: {
        auto apiRecord = readAPIRecordFromScratch();
        if (!apiRecord)
          return apiRecord.takeError();
        record = api.addTypeDef(apiRecord->name, apiRecord->loc,
                                apiRecord->availability, apiRecord->access,
                                /*Decl=*/nullptr);
        continue;
      }
      case typedef_block::AVAILABILITY: {
        if (auto err = readAvailabilityInfoFromScratch(*record))
          return err;
        continue;
      }
      case typedef_block::FILENAME: {
        if (auto err = readFilenameFromScratch(*record))
          return err;
        continue;
      }
      case typedef_block::LOCATION: {
        if (auto err = readSourceLocationFromScratch(*record))
          return err;
        continue;
      }
      default:
        // Unknown record, possibly for use by a future version of the format.
        continue;
      }
      continue;
    }
    case BitstreamEntry::SubBlock: {
      if (auto err = cursor.SkipBlock())
        return err;
      continue;
    }
    case BitstreamEntry::EndBlock:
      return Error::success();
    }
  }
}

Error SDKDBBitcodeReader::Implementation::readSDKDBBlock(
    BitstreamCursor &cursor, SDKDBBuilder &builder) const {
  if (auto err = cursor.EnterSubBlock(SDKDB_BLOCK_ID))
    return err;

  bool skipBlock = false;
  SDKDB *db = nullptr;
  while (true) {
    auto maybeEntry = cursor.advance();
    if (!maybeEntry)
      return maybeEntry.takeError();
    auto entry = maybeEntry.get();

    switch (entry.Kind) {
    case BitstreamEntry::Error:
      return make_error<StringError>("error malformed entry",
                                     inconvertibleErrorCode());
    case BitstreamEntry::Record: {
      if (skipBlock) {
        if (auto skipped = cursor.skipRecord(entry.ID))
          continue;
        else
          return skipped.takeError();
      }
      scratch.clear();
      StringRef tripleStr;
      auto maybeKind = cursor.readRecord(entry.ID, scratch, &tripleStr);
      if (!maybeKind)
        return maybeKind.takeError();
      unsigned kind = maybeKind.get();
      switch (kind) {
      case sdkdb_block::TARGET_TRIPLE: {
        Triple triple(tripleStr);
        if (option.targets.size() &&
            llvm::find_if(option.targets, [&](const Triple &target) {
              return SDKDB::areCompatibleTargets(triple, target);
            }) == option.targets.end())
          skipBlock = true;
        else
          db = &builder.getSDKDBForTarget(triple);
        continue;
      }

      default:
        // Unknown metadata record, possibly for use by a future version of the
        // format.
        continue;
      }
    }
    case BitstreamEntry::SubBlock: {
      if (skipBlock || entry.ID != API_BLOCK_ID) {
        if (auto err = cursor.SkipBlock())
          return err;
        continue;
      }
      if (!db)
        return make_error<StringError>("SDKDB is not started with triple",
                                       inconvertibleErrorCode());
      auto api = readAPIBlock(cursor, *db);
      if (!api)
        return api.takeError();

      continue;
    }
    case BitstreamEntry::EndBlock:
      return Error::success();
    }
  } // while
}

Error SDKDBBitcodeReader::Implementation::materialize(
    SDKDBBuilder &builder) const {
  BitstreamCursor cursor(input);
  BitstreamBlockInfo blockInfo;

  // set build version.
  builder.setBuildVersion(buildVersion);
  builder.setBuilderOptions(builderOpts);

  // set project with error.
  for (auto &proj : projectWithError)
    builder.addProjectWithError(proj);

  if (auto err = readSignature(cursor))
    return err;

  while (!cursor.AtEndOfStream()) {
    auto maybeTopLevelEntry = cursor.advance();
    if (!maybeTopLevelEntry)
      return maybeTopLevelEntry.takeError();

    auto topLevelEntry = maybeTopLevelEntry.get();
    if (topLevelEntry.Kind != BitstreamEntry::SubBlock)
      break;

    switch (topLevelEntry.ID) {
    case bitc::BLOCKINFO_BLOCK_ID: {
      if (auto err = readBlockInfoBlock(cursor, blockInfo))
        return err;

      break;
    }

    case SDKDB_BLOCK_ID: {
      if (auto err = readSDKDBBlock(cursor, builder))
        return err;
      break;
    }
    case IDENTIFIER_BLOCK_ID: {
      if (auto err = readIdentificationBlock(cursor))
        return err;
      break;
    }

    default: { // Skip all the other blocks.
      if (auto err = cursor.SkipBlock())
        return err;
      break;
    }
    }
  }

  return Error::success();
}

Expected<bool>
SDKDBBitcodeReader::Implementation::dylibExistsForPath(Triple &target,
                                                       StringRef path) {
  auto result = findOffsetForLibrary(target, path);

  if (!result)
    return result.takeError();

  return *result;
}

Error SDKDBBitcodeReader::Implementation::loadAPIsFromSDKDB(
    SDKDBBuilder &builder, Triple &target, StringRef path) {
  // Create cursor and parse BlockInfo block.
  BitstreamCursor cursor(input);
  BitstreamBlockInfo blockInfo;
  uint64_t sdkdbBlockStart = 0;

  if (auto err = readSignature(cursor))
    return err;

  while (!cursor.AtEndOfStream()) {
    auto maybeTopLevelEntry = cursor.advance();
    if (!maybeTopLevelEntry)
      return maybeTopLevelEntry.takeError();

    auto topLevelEntry = maybeTopLevelEntry.get();
    if (topLevelEntry.Kind != BitstreamEntry::SubBlock)
      break;

    switch (topLevelEntry.ID) {
    case bitc::BLOCKINFO_BLOCK_ID: {
      if (auto err = readBlockInfoBlock(cursor, blockInfo))
        return err;

      break;
    }
    case SDKDB_BLOCK_ID: {
      sdkdbBlockStart = cursor.GetCurrentBitNo();
      if (auto err = cursor.SkipBlock())
        return err;
      break;
    }
    case IDENTIFIER_BLOCK_ID: {
      if (auto err = readIdentificationBlock(cursor))
        return err;
      break;
    }
    default: { // Skip all the other blocks.
      if (auto err = cursor.SkipBlock())
        return err;
      break;
    }
    }
  }

  // reset the cursor to the beginning of SDKDB block so we can enter its
  // context.
  if (auto err = cursor.JumpToBit(sdkdbBlockStart))
    return err;

  if (auto err = cursor.EnterSubBlock(SDKDB_BLOCK_ID))
    return err;

  auto &db = builder.getSDKDBForTarget(target);
  StringSet<> loadedBinaries;
  std::vector<std::string> workSet;
  workSet.emplace_back(path.data(), path.size());

  while (!workSet.empty()) {
    auto current = workSet.back();
    workSet.pop_back();
    // skip if it is already loaded.
    if (loadedBinaries.count(current))
      continue;

    loadedBinaries.insert(current);

    auto offset = findOffsetForLibrary(target, current);
    if (!offset)
      return offset.takeError();

    if (!*offset)
      return make_error<StringError>("Dylib not found in SDKDB",
                                    inconvertibleErrorCode());

    if (auto err = cursor.JumpToBit(*offset))
      return err;

    auto maybeAPIEntry = cursor.advance();
    if (!maybeAPIEntry)
      return maybeAPIEntry.takeError();

    auto apiEntry = maybeAPIEntry.get();

    if (apiEntry.Kind != BitstreamEntry::SubBlock ||
        apiEntry.ID != API_BLOCK_ID)
      return make_error<StringError>("Wrong offset for API block",
                                     inconvertibleErrorCode());

    auto api = readAPIBlock(cursor, db);
    if (!api)
      return api.takeError();

    if (*api && (*api)->hasBinaryInfo()) {
      for (auto reexport : (*api)->getBinaryInfo().reexportedLibraries) {
        if (!loadedBinaries.count(reexport))
          workSet.push_back(reexport.str());
      }
    }
  }

  return Error::success();
}

Error SDKDBBitcodeReader::Implementation::materializeLibraryTable() const {
  // Done if dylibTable is already populated.
  if (!dylibTable.empty())
    return Error::success();

  BitstreamCursor cursor(input);
  BitstreamBlockInfo blockInfo;

  if (auto err = readSignature(cursor))
    return err;

  while (!cursor.AtEndOfStream()) {
    auto maybeTopLevelEntry = cursor.advance();
    if (!maybeTopLevelEntry)
      return maybeTopLevelEntry.takeError();

    auto topLevelEntry = maybeTopLevelEntry.get();
    if (topLevelEntry.Kind != BitstreamEntry::SubBlock)
      break;

    switch (topLevelEntry.ID) {
    case bitc::BLOCKINFO_BLOCK_ID: {
      if (auto err = readBlockInfoBlock(cursor, blockInfo))
        return err;

      break;
    }
    case IDENTIFIER_BLOCK_ID: {
      if (auto err = readIdentificationBlock(cursor))
        return err;
      break;
    }
    case LIBRARY_TABLE_BLOCK_ID: {
      if (auto err = readLibraryTableBlock(cursor))
        return err;
      break;
    }

    default: { // Skip all the other blocks.
      if (auto err = cursor.SkipBlock())
        return err;
      break;
    }
    }
  }

  return Error::success();
}

Error SDKDBBitcodeReader::Implementation::readLibraryTableBlock(
    BitstreamCursor &cursor) const {
  if (auto err = cursor.EnterSubBlock(LIBRARY_TABLE_BLOCK_ID))
    return err;

  Triple target;
  while (true) {
    auto maybeEntry = cursor.advance();
    if (!maybeEntry)
      return maybeEntry.takeError();
    auto entry = maybeEntry.get();

    switch (entry.Kind) {
    case BitstreamEntry::Error:
      return make_error<StringError>("error malformed entry",
                                     inconvertibleErrorCode());
    case BitstreamEntry::Record: {
      scratch.clear();
      StringRef blob;
      auto maybeKind = cursor.readRecord(entry.ID, scratch, &blob);
      if (!maybeKind)
        return maybeKind.takeError();
      unsigned kind = maybeKind.get();
      switch (kind) {
      case library_table_block::TARGET_TRIPLE: {
        target = Triple(blob);
        continue;
      }
      case library_table_block::LOOKUP_TABLE: {
        uint64_t tableOffset = scratch[0];
        auto base = reinterpret_cast<const uint8_t *>(blob.data());
        LibraryTableInfo info(stringTable.data());
        /// The library table.
        std::unique_ptr<SerializedLibraryTable> libraryTable(
            SerializedLibraryTable::Create(
                base + tableOffset, base + sizeof(uint64_t), base, info));

        dylibTable.try_emplace(target.str(), std::move(libraryTable));
        continue;
      }
      default:
        continue;
      }
    }
    case BitstreamEntry::SubBlock:
      return make_error<StringError>("No subblocks in LIBRARY_TABLE",
                                     inconvertibleErrorCode());
    case BitstreamEntry::EndBlock:
      return Error::success();
    }
  } // while
}

Expected<uint64_t>
SDKDBBitcodeReader::Implementation::getOffsetForLibrary(Triple &target,
                                                        StringRef path) {
  if (auto err = materializeLibraryTable())
    return std::move(err);

  for (auto &entry : dylibTable) {
    if (target != Triple(entry.getKey()))
      continue;

    auto dylib = entry.getValue()->find(path);
    if (dylib != entry.getValue()->end())
      return *dylib;

    return 0;
  }

  return 0;
}

Expected<uint64_t>
SDKDBBitcodeReader::Implementation::findOffsetForLibrary(Triple &target,
                                                         StringRef path) {
  auto offset = getOffsetForLibrary(target, path);
  if (!offset)
    return offset.takeError();

  if (*offset)
    return *offset;

  // NOTE: Not all dylibs and frameworks are zippered correctly.
  // This is the workaround to load macOS version of the dylib.
  if (target.isMacCatalystEnvironment()) {
    Triple macTarget(target);
    macTarget.setOS(Triple::MacOSX);
    macTarget.setEnvironmentName("");
    offset = getOffsetForLibrary(macTarget, path);
    if (!offset)
      return offset.takeError();
    if (*offset)
      errs() << "warning: fallback to macOS to find " + path << "\n";
  }

  return *offset;
}

Expected<StringRef>
SDKDBBitcodeReader::Implementation::readStringFromTable(unsigned offset,
                                                        unsigned size) const {
  if (offset + size > stringTable.size())
    return make_error<StringError>("invalid index into string table",
                                   inconvertibleErrorCode());
  return stringTable.substr(offset, size);
}

SDKDBBitcodeReader::Implementation::Implementation(
    MemoryBufferRef input, SDKDBBitcodeMaterializeOption &option, Error &err)
    : input(input), option(option) {
  ErrorAsOutParameter errorAsOutParameter(&err);
  err = validateSDKDB();
}

TAPI_NAMESPACE_INTERNAL_END
