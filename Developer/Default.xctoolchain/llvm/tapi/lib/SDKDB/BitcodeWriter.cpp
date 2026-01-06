//===- tapi/SDKDB/BitcodeWriter.cpp - TAPI SDKDB Bitcode Writer -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implementes SDKDB bitcode writer.
///
//===----------------------------------------------------------------------===//

#include "tapi/SDKDB/BitcodeWriter.h"
#include "SDKDBBitcodeFormat.h"

#include "tapi/Core/APIVisitor.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/Bitcode/BitcodeConvenience.h"
#include "llvm/Bitstream/BitCodes.h"
#include "llvm/Bitstream/BitstreamWriter.h"
#include "llvm/MC/StringTableBuilder.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/OnDiskHashTable.h"
#include "llvm/Support/Path.h"

using namespace llvm;

TAPI_NAMESPACE_INTERNAL_BEGIN

namespace {
/// Used to serialize the on-disk library table.
class LibraryTableInfo {
  StringTableBuilder &stringTable;

public:
  LibraryTableInfo(StringTableBuilder &stringTable)
      : stringTable(stringTable) {}

  using key_type = StringRef;
  using key_type_ref = const key_type &;
  using data_type = uint64_t;
  using data_type_ref = const data_type &;
  using hash_value_type = uint64_t;
  using offset_type = unsigned;

  // NOLINTNEXTLINE
  hash_value_type ComputeHash(key_type_ref key) { return hash_value(key); }

  std::pair<offset_type, offset_type> // NOLINTNEXTLINE
  EmitKeyDataLength(raw_ostream &out, key_type_ref key, data_type_ref data) {
    // StringRef: offset=32 length=16.
    offset_type keyLength = sizeof(uint32_t) + sizeof(uint16_t);
    // Offset to library block.
    offset_type dataLength = sizeof(data_type);
    // The size of the key and data is fixed, so we don't emit them to the
    // stream.
    return {keyLength, dataLength};
  }

  // NOLINTNEXTLINE
  void EmitKey(raw_ostream &out, key_type_ref key, offset_type len) {
    unsigned keyOffset = stringTable.getOffset(key);
    unsigned keySize = key.size();
    support::endian::Writer writer(out, support::little);
    writer.write<uint32_t>(keyOffset);
    writer.write<uint16_t>(keySize);
  }

  // NOLINTNEXTLINE
  void EmitData(raw_ostream &out, key_type_ref key, data_type_ref data,
                offset_type len) {
    support::endian::Writer writer(out, support::little);
    writer.write<data_type>(data);
  }
};
} // end anonymous namespace

class SDKDBWriter {
public:
  SDKDBWriter(const SDKDBBuilder &builder);

  void writeToStream(raw_ostream &os);

private:
  void addSDKDB(const SDKDB &sdkdb);

  void writeBlockInfoBlock();
  void writeControlBlock();
  void writeIdentifierBlock();
  void writeSDKDBBlock(const SDKDB &db);
  void writeAPIBlock(const API& api);
  void writeBinaryInfoBlock(const BinaryInfo &info);
  void writeLibraryTable();

  std::optional<StringRef> getShallowFrameworkPath(StringRef installName);

  const SDKDBBuilder &builder;

  /// Output buffer
  SmallVector<char, 0> buffer;
  std::unique_ptr<llvm::BitstreamWriter> writer;
  StringTableBuilder stringBuilder{StringTableBuilder::RAW};

  /// Scratch space for bitstream writing.
  SmallVector<uint64_t, 64> scratchRecord;

  /// Allocator for strings that is not in binary but needs to be in
  /// stringtable.
  BumpPtrAllocator allocator;

  /// Table for building library index. [triple][path] -> index
  Triple currentTriple;
  uint64_t currentAPIStart;
  StringMap<StringMap<uint64_t>> libraryIndex;
};

// This is a list of hard coded install_name path -> symlinked location.
static StringMap<std::string> symlinkMap = {
    {"/System/Library/Frameworks/Foundation.framework/Foundation",
     "/usr/lib/libextension.dylib"},
    {"/System/Library/Frameworks/Foundation.framework/Versions/C/Foundation",
     "/usr/lib/libextension.dylib"},
    {"/System/Library/Frameworks/Network.framework/Network",
     "/System/Library/PrivateFrameworks/Network.framework/Network"},
    {"/System/Library/Frameworks/Network.framework/Versions/A/Network",
     "/System/Library/PrivateFrameworks/Network.framework/Network"},
    {"/System/Library/Frameworks/PencilKit.framework/PencilKit",
     "/usr/lib/swift/libswiftPencilKit.dylib"},
    {"/System/Library/Frameworks/PencilKit.framework/Versions/A/PencilKit",
     "/usr/lib/swift/libswiftPencilKit.dylib"},
    {"/System/iOSSupport/System/Library/Frameworks/PencilKit.framework/"
     "PencilKit",
     "/System/iOSSupport/usr/lib/swift/libswiftPencilKit.dylib"},
    {"/System/iOSSupport/System/Library/Frameworks/PencilKit.framework/"
     "Versions/A/PencilKit",
     "/System/iOSSupport/usr/lib/swift/libswiftPencilKit.dylib"},
};

void SDKDBBitcodeWriter::writeSDKDBToStream(const SDKDBBuilder &builder,
                                            raw_ostream &os) {
  SDKDBWriter writer(builder);
  writer.writeToStream(os);
}

SDKDBWriter::SDKDBWriter(const SDKDBBuilder &builder) : builder(builder) {
  writer.reset(new BitstreamWriter(buffer));
}

static std::optional<StringRef> getPreviousInstallName(StringRef sym) {
  if (sym.consume_front("$ld$install_name$os"))
    return sym.split('$').second;
  return std::nullopt;
}

std::optional<StringRef>
SDKDBWriter::getShallowFrameworkPath(StringRef installName) {
  // For install name like /S/L/F/Foo.framework/Versions/A/Foo, return the
  // shallow path /S/L/F/Foo.framework/Foo
  auto versionDir = sys::path::parent_path(sys::path::parent_path(installName));
  if (sys::path::filename(versionDir) != "Versions")
    return std::nullopt;

  SmallString<PATH_MAX> shallowPath(sys::path::parent_path(versionDir));
  sys::path::append(shallowPath, sys::path::filename(installName));

  // Put the string into allocator so it can be insert into StringTable later.
  void *ptr = allocator.Allocate(shallowPath.size(), 1);
  memcpy(ptr, shallowPath.data(), shallowPath.size());
  return StringRef(reinterpret_cast<const char *>(ptr), shallowPath.size());
}

namespace {

/// These are manifest constants used by the bitcode writer. They do not need to
/// be kept in sync with the reader, but need to be consistent within this file.
enum {
  // SDKDB_BLOCK abbrev id's.
  SDKDB_TARGET_TRIPLE_ABBREV = bitc::FIRST_APPLICATION_ABBREV,

  // API_BLOCK abbrev id's.
  API_INSTALL_NAME_ABBREV = bitc::FIRST_APPLICATION_ABBREV,
  API_FILE_TYPE_ABBREV,
  API_UUID_ABBREV,
  API_REEXPORTED_ABBREV,
  API_ALLOWABLE_CLIENT_ABBREV,
  API_PARENT_UMBRELLA_ABBREV,
  API_DYLIB_VERSION_ABBREV,
  API_FLAGS_ABBREV,
  API_SWIFT_VERSION_ABBREV,
  API_POTENTIALLY_DEFINED_SELECTOR_ABBREV,
  API_PROJECT_NAME_ABBREV,

  // GLOBAL_BLOCK abbrev id's.
  GLOBAL_INFO_ABBREV = bitc::FIRST_APPLICATION_ABBREV,
  GLOBAL_AVAILABILITY_ABBREV,
  GLOBAL_FILENAME_ABBREV,
  GLOBAL_LOCATION_ABBREV,

  // OBJC_CLASS_BLOCK abbrev id's.
  OBJC_CLASS_INFO_ABBREV = bitc::FIRST_APPLICATION_ABBREV,
  OBJC_CLASS_AVAILABILITY_ABBREV,
  OBJC_CLASS_FILENAME_ABBREV,
  OBJC_CLASS_LOCATION_ABBREV,
  OBJC_CLASS_PROTOCOL_ABBREV,

  // OBJC_CATEGORY_BLOCK abbrev id's.
  OBJC_CATEGORY_INFO_ABBREV = bitc::FIRST_APPLICATION_ABBREV,
  OBJC_CATEGORY_AVAILABILITY_ABBREV,
  OBJC_CATEGORY_FILENAME_ABBREV,
  OBJC_CATEGORY_LOCATION_ABBREV,
  OBJC_CATEGORY_PROTOCOL_ABBREV,

  // OBJC_PROTOCOL_BLOCK abbrev id's.
  OBJC_PROTOCOL_INFO_ABBREV = bitc::FIRST_APPLICATION_ABBREV,
  OBJC_PROTOCOL_AVAILABILITY_ABBREV,
  OBJC_PROTOCOL_FILENAME_ABBREV,
  OBJC_PROTOCOL_LOCATION_ABBREV,
  OBJC_PROTOCOL_PROTOCOL_ABBREV,

  // OBJC_METHOD_BLOCK abbrev id's.
  OBJC_METHOD_ABBREV = bitc::FIRST_APPLICATION_ABBREV,
  OBJC_METHOD_AVAILABILITY_ABBREV,
  OBJC_METHOD_FILENAME_ABBREV,
  OBJC_METHOD_LOCATION_ABBREV,

  // OBJC_PROPERTY_BLOCK abbrev id's.
  OBJC_PROPERTY_ABBREV = bitc::FIRST_APPLICATION_ABBREV,
  OBJC_PROPERTY_AVAILABILITY_ABBREV,
  OBJC_PROPERTY_FILENAME_ABBREV,
  OBJC_PROPERTY_LOCATION_ABBREV,

  // OBJC_IVAR_BLOCK abbrev id's.
  OBJC_IVAR_ABBREV = bitc::FIRST_APPLICATION_ABBREV,
  OBJC_IVAR_AVAILABILITY_ABBREV,
  OBJC_IVAR_FILENAME_ABBREV,
  OBJC_IVAR_LOCATION_ABBREV,

  // LIBRARY_TABLE_BLOCK abbrev id's
  LIBRARY_TABLE_TARGET_TRIPLE_ABBREV = bitc::FIRST_APPLICATION_ABBREV,
  LIBRARY_TABLE_LOOKUP_TABLE_ABBREV,

  // ENUM_BLOCK abbrev id's.
  ENUM_INFO_ABBREV = bitc::FIRST_APPLICATION_ABBREV,
  ENUM_AVAILABILITY_ABBREV,
  ENUM_FILENAME_ABBREV,
  ENUM_LOCATION_ABBREV,

  // ENUM_CONSTANT_BLOCK abbrev id's.
  ENUM_CONSTANT_INFO_ABBREV = bitc::FIRST_APPLICATION_ABBREV,
  ENUM_CONSTANT_AVAILABILITY_ABBREV,
  ENUM_CONSTANT_FILENAME_ABBREV,
  ENUM_CONSTANT_LOCATION_ABBREV,

  // TYPEDEF_BLOCK abbrev id's.
  TYPEDEF_INFO_ABBREV = bitc::FIRST_APPLICATION_ABBREV,
  TYPEDEF_AVAILABILITY_ABBREV,
  TYPEDEF_FILENAME_ABBREV,
  TYPEDEF_LOCATION_ABBREV,
};

class APICollector : public APIVisitor {
public:
  APICollector(StringTableBuilder &strTable, const SDKDBBuilder &builder)
      : strTable(strTable), builder(builder) {}

  void visitGlobal(const GlobalRecord &record) override {
    processAPIRecord(record);
    // Add names of previous install name into string table.
    if (auto name = getPreviousInstallName(record.name))
      strTable.add(*name);
  }

  void visitObjCInterface(const ObjCInterfaceRecord &record) override {
    if (builder.noObjCMetadata())
      return;
    processAPIRecord(record);
    processObjCContainer(record);
    strTable.add(record.superClass);
  }

  void visitObjCCategory(const ObjCCategoryRecord &record) override {
    if (builder.noObjCMetadata())
      return;
    processAPIRecord(record);
    processObjCContainer(record);
    strTable.add(record.interface);
  }

  void visitObjCProtocol(const ObjCProtocolRecord &record) override {
    if (builder.noObjCMetadata())
      return;
    processAPIRecord(record);
    processObjCContainer(record);
  }

  void visitEnum(const EnumRecord &record) override {
    if (builder.excludeEnumTypes())
      return;
    processAPIRecord(record);
    strTable.add(record.usr);
    for (auto *constant : record.constants)
      processAPIRecord(*constant);
  }

  void visitTypeDef(const TypedefRecord &record) override {
    if (builder.excludeEnumTypes())
      return;
    processAPIRecord(record);
  }

private:
  void processAPIRecord(const APIRecord &record);
  void processObjCContainer(const ObjCContainerRecord &record);

  StringTableBuilder &strTable;
  const SDKDBBuilder &builder;
};

// TODO: handle newly added fields in BitCode:
//   - USR
//   - docComment
class APISerializer : public APIVisitor {
public:
  APISerializer(BitstreamWriter &writer, StringTableBuilder &table,
                uint64_t currentAPIStart, StringMap<uint64_t> &libraryIndex,
                const SDKDBBuilder &builder)
      : writer(writer), stringBuilder(table), currentAPIStart(currentAPIStart),
        libraryIndex(libraryIndex), builder(builder) {}

  void visitGlobal(const GlobalRecord &record) override;

  void visitObjCInterface(const ObjCInterfaceRecord &record) override;

  void visitObjCCategory(const ObjCCategoryRecord &record) override;

  void visitObjCProtocol(const ObjCProtocolRecord &record) override;

  void visitEnum(const EnumRecord &record) override;

  void visitTypeDef(const TypedefRecord &record) override;

private:
  // Return true if loaded, false if skipped.
  bool loadRecordIntoScratch(unsigned abbrev, const APIRecord &record);

  void writeAvailabilityBlock(const AvailabilityInfo &info, unsigned id,
                              unsigned abbrev);
  void writeLocationBlock(const APILoc &loc, unsigned fileID,
                          unsigned fileAbbrev, unsigned locID,
                          unsigned locAbbrev);
  void writeObjCContainer(const ObjCContainerRecord &record,
                          unsigned protocolID, unsigned protocolAbbrev);
  void writeObjCMethod(const ObjCMethodRecord &record);
  void writeObjCProperty(const ObjCPropertyRecord &record);
  void writeObjCInstanceVariable(const ObjCInstanceVariableRecord &record);

  BitstreamWriter &writer;
  StringTableBuilder &stringBuilder;
  uint64_t currentAPIStart;
  StringMap<uint64_t> &libraryIndex;
  const SDKDBBuilder &builder;
  /// Scratch space.
  SmallVector<uint64_t, 64> scratchRecord;
};

} // anonymous namespace

// Add SDKDB to output writer. Record all the strings and calculate the size
// of the slice.
void SDKDBWriter::addSDKDB(const SDKDB &sdkdb) {
  APICollector collector(stringBuilder, builder);
  for (auto *api : sdkdb.api()) {
    api->visit(collector);

    for (auto &selector : api->getPotentiallyDefinedSelectors())
      stringBuilder.add(selector.first());

    auto project = api->getProjectName();
    if (!project.empty())
      stringBuilder.add(project);

    // add binary info string.
    if (!api->hasBinaryInfo())
      continue;
    auto &binaryInfo = api->getBinaryInfo();
    if (builder.excludeBundles() &&
        binaryInfo.fileType == FileType::MachO_Bundle)
      continue;
    stringBuilder.add(binaryInfo.installName);
    if (auto shallowName = getShallowFrameworkPath(binaryInfo.installName))
      stringBuilder.add(*shallowName);
    auto symlink = symlinkMap.find(binaryInfo.installName);
    if (symlink != symlinkMap.end())
      stringBuilder.add(symlink->second);
    for (auto reexport : binaryInfo.reexportedLibraries)
      stringBuilder.add(reexport);
    stringBuilder.add(binaryInfo.parentUmbrella);
  }
}

// Write SDKDB binary output.
void SDKDBWriter::writeToStream(raw_ostream &os) {
  // Collect all the string first.
  for (auto *sdkdb : builder.getDatabases())
    addSDKDB(*sdkdb);

  // Finalize StringBuilder.
  stringBuilder.finalize();

  // Emit the signature.
  for (unsigned char byte : SDKDB_SIGNATURE)
    writer->Emit(byte, 8);

  // Emit the blocks.
  writeBlockInfoBlock();
  writeControlBlock();
  writeIdentifierBlock();
  for (auto *db : builder.getDatabases())
    writeSDKDBBlock(*db);
  writeLibraryTable();

  // Write the buffer to the stream.
  os.write(buffer.data(), buffer.size());
  os.flush();
}

/// Record the name of a block.
static void emitBlockID(BitstreamWriter &out, unsigned id, StringRef name,
                        SmallVectorImpl<unsigned char> &nameBuffer) {
  SmallVector<unsigned, 1> idBuffer;
  idBuffer.push_back(id);
  out.EmitRecord(bitc::BLOCKINFO_CODE_SETBID, idBuffer);

  // Emit the block name if present.
  if (name.empty())
    return;
  nameBuffer.resize(name.size());
  memcpy(nameBuffer.data(), name.data(), name.size());
  out.EmitRecord(bitc::BLOCKINFO_CODE_BLOCKNAME, nameBuffer);
}

/// Record the name of a record within a block.
static void emitRecordID(BitstreamWriter &out, unsigned id, StringRef name,
                         SmallVectorImpl<unsigned char> &nameBuffer) {
  assert(id < 256 && "can't fit record ID in next to name");
  nameBuffer.resize(name.size() + 1);
  nameBuffer[0] = id;
  memcpy(nameBuffer.data() + 1, name.data(), name.size());
  out.EmitRecord(bitc::BLOCKINFO_CODE_SETRECORDNAME, nameBuffer);
}

/// Emit APIRecord based entries.
static void addAPIRecordAbbrev(BitCodeAbbrev *abbv) {
  // name string offset and size.
  abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8));
  abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));
  // bit fields.
  // Access.
  abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 2));
  // Linkage.
  abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 3));
  // Flags.
  abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 3));
}

/// Emit availability based entries.
static void addAvailabilityAbbrev(BitstreamWriter &writer, unsigned record,
                                  unsigned block, unsigned abbrev) {
  auto abbv = std::make_shared<BitCodeAbbrev>();
  abbv->Add(BitCodeAbbrevOp(record));
  // Introduced.
  abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));
  // Obsoleted.
  abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));
  // Unavailable.
  abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1));
  // isSPIAvailable.
  abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1));
  if (writer.EmitBlockInfoAbbrev(block, abbv) != abbrev)
    llvm_unreachable("Unexpected abbrev ordering!");
}

static void addFileNameAbbrev(BitstreamWriter &writer, unsigned record,
                              unsigned block, unsigned abbrev) {
  auto abbv = std::make_shared<BitCodeAbbrev>();
  abbv->Add(BitCodeAbbrevOp(record));
  // File name offset and size.
  abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8));
  abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));
  if (writer.EmitBlockInfoAbbrev(block, abbv) != abbrev)
    llvm_unreachable("Unexpected abbrev ordering!");
}

static void addLocationAbbrev(BitstreamWriter &writer, unsigned record,
                              unsigned block, unsigned abbrev) {
  auto abbv = std::make_shared<BitCodeAbbrev>();
  abbv->Add(BitCodeAbbrevOp(record));
  // Line and column number.
  abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));
  abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));
  if (writer.EmitBlockInfoAbbrev(block, abbv) != abbrev)
    llvm_unreachable("Unexpected abbrev ordering!");
}

static void addProtocolAbbrev(BitstreamWriter &writer, unsigned record,
                              unsigned block, unsigned abbrev) {
  auto abbv = std::make_shared<BitCodeAbbrev>();
  abbv->Add(BitCodeAbbrevOp(record));
  // Protocol name offset and size.
  abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8));
  abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));
  if (writer.EmitBlockInfoAbbrev(block, abbv) != abbrev)
    llvm_unreachable("Unexpected abbrev ordering!");
}

void SDKDBWriter::writeBlockInfoBlock() {
  BCBlockRAII restoreBlock(*writer, bitc::BLOCKINFO_BLOCK_ID,
                           /*abbrevLen=*/2);

  SmallVector<unsigned char, 64> nameBuffer;
#define BLOCK(X) emitBlockID(*writer, X##_ID, #X, nameBuffer)
#define BLOCK_RECORD(K, X) emitRecordID(*writer, K::X, #X, nameBuffer)

  BLOCK(CONTROL_BLOCK);
  BLOCK_RECORD(control_block, METADATA);
  BLOCK_RECORD(control_block, PROJECT_WITH_ERROR);

  BLOCK(IDENTIFIER_BLOCK);
  BLOCK_RECORD(identifier_block, STRING_TABLE);

  BLOCK(SDKDB_BLOCK);
  BLOCK_RECORD(sdkdb_block, TARGET_TRIPLE);

  BLOCK(API_BLOCK);
  BLOCK_RECORD(api_block, INSTALL_NAME);
  BLOCK_RECORD(api_block, UUID);
  BLOCK_RECORD(api_block, FILE_TYPE);
  BLOCK_RECORD(api_block, REEXPORTED);
  BLOCK_RECORD(api_block, ALLOWABLE_CLIENTS);
  BLOCK_RECORD(api_block, PARENT_UMBRELLA);
  BLOCK_RECORD(api_block, DYLIB_VERSION);
  BLOCK_RECORD(api_block, FLAGS);
  BLOCK_RECORD(api_block, SWIFT_VERSION);
  BLOCK_RECORD(api_block, POTENTIALLY_DEFINED_SELECTOR);
  BLOCK_RECORD(api_block, PROJECT_NAME);

  BLOCK(GLOBAL_BLOCK);
  BLOCK_RECORD(global_block, INFO);
  BLOCK_RECORD(global_block, AVAILABILITY);
  BLOCK_RECORD(global_block, FILENAME);
  BLOCK_RECORD(global_block, LOCATION);

  BLOCK(OBJC_CLASS_BLOCK);
  BLOCK_RECORD(objc_class_block, INFO);
  BLOCK_RECORD(objc_class_block, AVAILABILITY);
  BLOCK_RECORD(objc_class_block, FILENAME);
  BLOCK_RECORD(objc_class_block, LOCATION);
  BLOCK_RECORD(objc_class_block, PROTOCOL);

  BLOCK(OBJC_CATEGORY_BLOCK);
  BLOCK_RECORD(objc_category_block, INFO);
  BLOCK_RECORD(objc_category_block, AVAILABILITY);
  BLOCK_RECORD(objc_category_block, FILENAME);
  BLOCK_RECORD(objc_category_block, LOCATION);
  BLOCK_RECORD(objc_category_block, PROTOCOL);

  BLOCK(OBJC_PROTOCOL_BLOCK);
  BLOCK_RECORD(objc_protocol_block, INFO);
  BLOCK_RECORD(objc_protocol_block, AVAILABILITY);
  BLOCK_RECORD(objc_protocol_block, FILENAME);
  BLOCK_RECORD(objc_protocol_block, LOCATION);
  BLOCK_RECORD(objc_protocol_block, PROTOCOL);

  BLOCK(OBJC_METHOD_BLOCK);
  BLOCK_RECORD(objc_method_block, INFO);
  BLOCK_RECORD(objc_method_block, AVAILABILITY);
  BLOCK_RECORD(objc_method_block, FILENAME);
  BLOCK_RECORD(objc_method_block, LOCATION);

  BLOCK(OBJC_PROPERTY_BLOCK);
  BLOCK_RECORD(objc_property_block, INFO);
  BLOCK_RECORD(objc_property_block, AVAILABILITY);
  BLOCK_RECORD(objc_property_block, FILENAME);
  BLOCK_RECORD(objc_property_block, LOCATION);

  BLOCK(OBJC_IVAR_BLOCK);
  BLOCK_RECORD(objc_ivar_block, INFO);
  BLOCK_RECORD(objc_ivar_block, AVAILABILITY);
  BLOCK_RECORD(objc_ivar_block, FILENAME);
  BLOCK_RECORD(objc_ivar_block, LOCATION);

  BLOCK(LIBRARY_TABLE_BLOCK);
  BLOCK_RECORD(library_table_block, TARGET_TRIPLE);
  BLOCK_RECORD(library_table_block, LOOKUP_TABLE);

  BLOCK(ENUM_BLOCK);
  BLOCK_RECORD(enum_block, INFO);
  BLOCK_RECORD(enum_block, AVAILABILITY);
  BLOCK_RECORD(enum_block, FILENAME);
  BLOCK_RECORD(enum_block, LOCATION);

  BLOCK(ENUM_CONSTANT_BLOCK);
  BLOCK_RECORD(enum_constant_block, INFO);
  BLOCK_RECORD(enum_constant_block, AVAILABILITY);
  BLOCK_RECORD(enum_constant_block, FILENAME);
  BLOCK_RECORD(enum_constant_block, LOCATION);

  BLOCK(TYPEDEF_BLOCK);
  BLOCK_RECORD(typedef_block, INFO);
  BLOCK_RECORD(typedef_block, AVAILABILITY);
  BLOCK_RECORD(typedef_block, FILENAME);
  BLOCK_RECORD(typedef_block, LOCATION);
#undef BLOCK
#undef BLOCK_RECORD

  // Setup Abbrev ID for repeating blocks. Others can be setup in place.
  // SDKDB Entries.
  { // SDKDB Target Triple.
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(sdkdb_block::TARGET_TRIPLE));
    // Target triple.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
    if (writer->EmitBlockInfoAbbrev(SDKDB_BLOCK_ID, abbv) !=
        SDKDB_TARGET_TRIPLE_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
  }
  // API Entries.
  { // InstallName.
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(api_block::INSTALL_NAME));
    // InstallName string offset and size.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8));
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));
    if (writer->EmitBlockInfoAbbrev(API_BLOCK_ID, abbv) !=
        API_INSTALL_NAME_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
  }
  { // File Type.
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(api_block::FILE_TYPE));
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 4));
    if (writer->EmitBlockInfoAbbrev(API_BLOCK_ID, abbv) != API_FILE_TYPE_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
  }
  { // UUID.
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(api_block::UUID));
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
    if (writer->EmitBlockInfoAbbrev(API_BLOCK_ID, abbv) != API_UUID_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
  }
  { // Re-exported.
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(api_block::REEXPORTED));
    // Re-exported library string.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8));
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));
    if (writer->EmitBlockInfoAbbrev(API_BLOCK_ID, abbv) !=
        API_REEXPORTED_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
  }
  { // Allowable clients.
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(api_block::ALLOWABLE_CLIENTS));
    // Library string.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8));
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));
    if (writer->EmitBlockInfoAbbrev(API_BLOCK_ID, abbv) !=
        API_ALLOWABLE_CLIENT_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
  }
  { // Parent umbrella.
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(api_block::PARENT_UMBRELLA));
    // Library string.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8));
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));
    if (writer->EmitBlockInfoAbbrev(API_BLOCK_ID, abbv) !=
        API_PARENT_UMBRELLA_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
  }
  { // Dylib verions.
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(api_block::DYLIB_VERSION));
    // current version.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32));
    // compatibility version.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32));
    if (writer->EmitBlockInfoAbbrev(API_BLOCK_ID, abbv) !=
        API_DYLIB_VERSION_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
  }
  { // Flags.
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(api_block::FLAGS));
    // Two level namespace.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1));
    // Applicate extension safe.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1));
    if (writer->EmitBlockInfoAbbrev(API_BLOCK_ID, abbv) !=
        API_FLAGS_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
  }
  { // Swift version.
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(api_block::SWIFT_VERSION));
    // Swift version.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 8));
    if (writer->EmitBlockInfoAbbrev(API_BLOCK_ID, abbv) !=
        API_SWIFT_VERSION_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
  }
  { // Potentially defined selectors
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(api_block::POTENTIALLY_DEFINED_SELECTOR));
    // Selector name.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8));
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));
    if (writer->EmitBlockInfoAbbrev(API_BLOCK_ID, abbv) !=
        API_POTENTIALLY_DEFINED_SELECTOR_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
  }
  { // Project name.
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(api_block::PROJECT_NAME));
    // Project name.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8));
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));
    if (writer->EmitBlockInfoAbbrev(API_BLOCK_ID, abbv) !=
        API_PROJECT_NAME_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
  }
  // Global Entry.
  {
    // INFO.
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(global_block::INFO));
    addAPIRecordAbbrev(abbv.get());
    // GVKind.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 2));
    if (writer->EmitBlockInfoAbbrev(GLOBAL_BLOCK_ID, abbv) !=
        GLOBAL_INFO_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");

    addAvailabilityAbbrev(*writer, global_block::AVAILABILITY, GLOBAL_BLOCK_ID,
                          GLOBAL_AVAILABILITY_ABBREV);
    addFileNameAbbrev(*writer, global_block::FILENAME, GLOBAL_BLOCK_ID,
                      GLOBAL_FILENAME_ABBREV);
    addLocationAbbrev(*writer, global_block::LOCATION, GLOBAL_BLOCK_ID,
                      GLOBAL_LOCATION_ABBREV);
  }
  // ObjC Class Entry.
  {
    // INFO
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(objc_class_block::INFO));
    addAPIRecordAbbrev(abbv.get());
    // SuperClass Name.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8));
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));
    // Exception attribute.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1));
    if (writer->EmitBlockInfoAbbrev(OBJC_CLASS_BLOCK_ID, abbv) !=
        OBJC_CLASS_INFO_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
    addAvailabilityAbbrev(*writer, objc_class_block::AVAILABILITY,
                          OBJC_CLASS_BLOCK_ID, OBJC_CLASS_AVAILABILITY_ABBREV);
    addFileNameAbbrev(*writer, objc_class_block::FILENAME, OBJC_CLASS_BLOCK_ID,
                      OBJC_CLASS_FILENAME_ABBREV);
    addLocationAbbrev(*writer, objc_class_block::LOCATION, OBJC_CLASS_BLOCK_ID,
                      OBJC_CLASS_LOCATION_ABBREV);
    addProtocolAbbrev(*writer, objc_class_block::PROTOCOL, OBJC_CLASS_BLOCK_ID,
                      OBJC_CLASS_PROTOCOL_ABBREV);
  }
  // ObjC Category Entry.
  {
    // INFO
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(objc_category_block::INFO));
    addAPIRecordAbbrev(abbv.get());
    // Interface Name.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8));
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));
    if (writer->EmitBlockInfoAbbrev(OBJC_CATEGORY_BLOCK_ID, abbv) !=
        OBJC_CATEGORY_INFO_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
    addAvailabilityAbbrev(*writer, objc_category_block::AVAILABILITY,
                          OBJC_CATEGORY_BLOCK_ID,
                          OBJC_CATEGORY_AVAILABILITY_ABBREV);
    addFileNameAbbrev(*writer, objc_category_block::FILENAME,
                      OBJC_CATEGORY_BLOCK_ID, OBJC_CATEGORY_FILENAME_ABBREV);
    addLocationAbbrev(*writer, objc_category_block::LOCATION,
                      OBJC_CATEGORY_BLOCK_ID, OBJC_CATEGORY_LOCATION_ABBREV);
    addProtocolAbbrev(*writer, objc_category_block::PROTOCOL,
                      OBJC_CATEGORY_BLOCK_ID, OBJC_CATEGORY_PROTOCOL_ABBREV);
  }
  // ObjC Protocol Entry.
  {
    // INFO
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(objc_protocol_block::INFO));
    addAPIRecordAbbrev(abbv.get());
    if (writer->EmitBlockInfoAbbrev(OBJC_PROTOCOL_BLOCK_ID, abbv) !=
        OBJC_PROTOCOL_INFO_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
    addAvailabilityAbbrev(*writer, objc_protocol_block::AVAILABILITY,
                          OBJC_PROTOCOL_BLOCK_ID,
                          OBJC_PROTOCOL_AVAILABILITY_ABBREV);
    addFileNameAbbrev(*writer, objc_protocol_block::FILENAME,
                      OBJC_PROTOCOL_BLOCK_ID, OBJC_PROTOCOL_FILENAME_ABBREV);
    addLocationAbbrev(*writer, objc_protocol_block::LOCATION,
                      OBJC_PROTOCOL_BLOCK_ID, OBJC_PROTOCOL_LOCATION_ABBREV);
    addProtocolAbbrev(*writer, objc_protocol_block::PROTOCOL,
                      OBJC_PROTOCOL_BLOCK_ID, OBJC_PROTOCOL_PROTOCOL_ABBREV);
  }
  // ObjC Method Entry.
  {
    // INFO
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(objc_method_block::INFO));
    addAPIRecordAbbrev(abbv.get());
    // instanceMethod.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1));
    // optional.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1));
    // dynamic.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1));
    if (writer->EmitBlockInfoAbbrev(OBJC_METHOD_BLOCK_ID, abbv) !=
        OBJC_METHOD_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
    addAvailabilityAbbrev(*writer, objc_method_block::AVAILABILITY,
                          OBJC_METHOD_BLOCK_ID,
                          OBJC_METHOD_AVAILABILITY_ABBREV);
    addFileNameAbbrev(*writer, objc_method_block::FILENAME,
                      OBJC_METHOD_BLOCK_ID, OBJC_METHOD_FILENAME_ABBREV);
    addLocationAbbrev(*writer, objc_method_block::LOCATION,
                      OBJC_METHOD_BLOCK_ID, OBJC_METHOD_LOCATION_ABBREV);
  }
  // ObjC Property Entry.
  {
    // INFO
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(objc_property_block::INFO));
    addAPIRecordAbbrev(abbv.get());
    // attributes.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 3));
    // optional.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1));
    // getter.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8));
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));
    // setter.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8));
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));
    if (writer->EmitBlockInfoAbbrev(OBJC_PROPERTY_BLOCK_ID, abbv) !=
        OBJC_PROPERTY_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
    addAvailabilityAbbrev(*writer, objc_property_block::AVAILABILITY,
                          OBJC_PROPERTY_BLOCK_ID,
                          OBJC_PROPERTY_AVAILABILITY_ABBREV);
    addFileNameAbbrev(*writer, objc_property_block::FILENAME,
                      OBJC_PROPERTY_BLOCK_ID, OBJC_PROPERTY_FILENAME_ABBREV);
    addLocationAbbrev(*writer, objc_property_block::LOCATION,
                      OBJC_PROPERTY_BLOCK_ID, OBJC_PROPERTY_LOCATION_ABBREV);
  }
  // ObjC ivar Entry.
  {
    // INFO
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(objc_ivar_block::INFO));
    addAPIRecordAbbrev(abbv.get());
    // access control.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 3));
    if (writer->EmitBlockInfoAbbrev(OBJC_IVAR_BLOCK_ID, abbv) !=
        OBJC_IVAR_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
    addAvailabilityAbbrev(*writer, objc_ivar_block::AVAILABILITY,
                          OBJC_IVAR_BLOCK_ID, OBJC_IVAR_AVAILABILITY_ABBREV);
    addFileNameAbbrev(*writer, objc_ivar_block::FILENAME, OBJC_IVAR_BLOCK_ID,
                      OBJC_IVAR_FILENAME_ABBREV);
    addLocationAbbrev(*writer, objc_ivar_block::LOCATION, OBJC_IVAR_BLOCK_ID,
                      OBJC_IVAR_LOCATION_ABBREV);
  }
  // Library table lookup entry.
  { // Target Triple.
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(library_table_block::TARGET_TRIPLE));
    // Target triple.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
    if (writer->EmitBlockInfoAbbrev(LIBRARY_TABLE_BLOCK_ID, abbv) !=
        LIBRARY_TABLE_TARGET_TRIPLE_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
  }
  { // Lookup table
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(library_table_block::LOOKUP_TABLE));
    // Size
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8));
    // Data
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
    if (writer->EmitBlockInfoAbbrev(LIBRARY_TABLE_BLOCK_ID, abbv) !=
        LIBRARY_TABLE_LOOKUP_TABLE_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");
  }
  // Enum Entry.
  {
    // INFO.
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(enum_block::INFO));
    addAPIRecordAbbrev(abbv.get());
    // USR.
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8));
    abbv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));
    if (writer->EmitBlockInfoAbbrev(ENUM_BLOCK_ID, abbv) != ENUM_INFO_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");

    addAvailabilityAbbrev(*writer, enum_block::AVAILABILITY, ENUM_BLOCK_ID,
                          ENUM_AVAILABILITY_ABBREV);
    addFileNameAbbrev(*writer, enum_block::FILENAME, ENUM_BLOCK_ID,
                      ENUM_FILENAME_ABBREV);
    addLocationAbbrev(*writer, enum_block::LOCATION, ENUM_BLOCK_ID,
                      ENUM_LOCATION_ABBREV);
  }
  // Enum Constant Entry.
  {
    // INFO.
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(enum_constant_block::INFO));
    addAPIRecordAbbrev(abbv.get());
    if (writer->EmitBlockInfoAbbrev(ENUM_CONSTANT_BLOCK_ID, abbv) !=
        ENUM_CONSTANT_INFO_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");

    addAvailabilityAbbrev(*writer, enum_constant_block::AVAILABILITY,
                          ENUM_CONSTANT_BLOCK_ID,
                          ENUM_CONSTANT_AVAILABILITY_ABBREV);
    addFileNameAbbrev(*writer, enum_constant_block::FILENAME,
                      ENUM_CONSTANT_BLOCK_ID, ENUM_CONSTANT_FILENAME_ABBREV);
    addLocationAbbrev(*writer, enum_constant_block::LOCATION,
                      ENUM_CONSTANT_BLOCK_ID, ENUM_CONSTANT_LOCATION_ABBREV);
  }
  // typedef Entry.
  {
    // INFO.
    auto abbv = std::make_shared<BitCodeAbbrev>();
    abbv->Add(BitCodeAbbrevOp(typedef_block::INFO));
    addAPIRecordAbbrev(abbv.get());
    if (writer->EmitBlockInfoAbbrev(TYPEDEF_BLOCK_ID, abbv) !=
        TYPEDEF_INFO_ABBREV)
      llvm_unreachable("Unexpected abbrev ordering!");

    addAvailabilityAbbrev(*writer, typedef_block::AVAILABILITY,
                          TYPEDEF_BLOCK_ID, TYPEDEF_AVAILABILITY_ABBREV);
    addFileNameAbbrev(*writer, typedef_block::FILENAME, TYPEDEF_BLOCK_ID,
                      TYPEDEF_FILENAME_ABBREV);
    addLocationAbbrev(*writer, typedef_block::LOCATION, TYPEDEF_BLOCK_ID,
                      TYPEDEF_LOCATION_ABBREV);
  }
}

void SDKDBWriter::writeControlBlock() {
  BCBlockRAII restoreBlock(*writer, CONTROL_BLOCK_ID, /*abbrevLen=*/3);

  // Setup all abbreviation first.
  auto metadataAbbrev = std::make_shared<BitCodeAbbrev>();
  metadataAbbrev->Add(BitCodeAbbrevOp(control_block::METADATA));
  // Binary store major version.
  metadataAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 16));
  // Binary store minor version.
  metadataAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 16));
  // Flags encoded in SDKDBBuilderOptions.
  metadataAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 16));
  // Blob for build version.
  metadataAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
  auto metadataAbbrevCode = writer->EmitAbbrev(std::move(metadataAbbrev));

  // Project with error record.
  auto projectAbbrev = std::make_shared<BitCodeAbbrev>();
  projectAbbrev->Add(BitCodeAbbrevOp(control_block::PROJECT_WITH_ERROR));
  // Project name as array in char6
  projectAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
  auto projectAbbrevCode = writer->EmitAbbrev(std::move(projectAbbrev));

  // METADATA
  scratchRecord = {control_block::METADATA, VERSION_MAJOR, VERSION_MINOR,
                   builder.getRawOptionEncoding()};
  writer->EmitRecordWithBlob(metadataAbbrevCode, scratchRecord,
                             builder.getBuildVersion());

  // Project with error.
  scratchRecord.clear();
  scratchRecord = {control_block::PROJECT_WITH_ERROR};

  for (auto &proj : builder.getProjectWithError())
    writer->EmitRecordWithBlob(projectAbbrevCode, scratchRecord, proj);
}

void SDKDBWriter::writeIdentifierBlock() {
  BCBlockRAII restoreBlock(*writer, IDENTIFIER_BLOCK_ID, /*abbrevLen=*/3);

  // Setup all abbreviation first.
  auto stringTableAbbrev = std::make_shared<BitCodeAbbrev>();
  stringTableAbbrev->Add(BitCodeAbbrevOp(identifier_block::STRING_TABLE));
  stringTableAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8));
  stringTableAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
  unsigned stringTableAbbrevCode =
      writer->EmitAbbrev(std::move(stringTableAbbrev));

  // Finalize and optimize the string table. No more strings can be added now.
  stringBuilder.finalize();
  SmallString<0> stringBuffer;
  buffer.reserve(stringBuilder.getSize());
  {
    raw_svector_ostream os(stringBuffer);
    stringBuilder.write(os);
  }

  scratchRecord = {identifier_block::STRING_TABLE, buffer.size()};
  writer->EmitRecordWithBlob(stringTableAbbrevCode, scratchRecord,
                             stringBuffer);
}

void SDKDBWriter::writeSDKDBBlock(const SDKDB &db) {
  BCBlockRAII restoreBlock(*writer, SDKDB_BLOCK_ID, /*abbrevLen=*/3);
  scratchRecord = {sdkdb_block::TARGET_TRIPLE};
  writer->EmitRecordWithBlob(SDKDB_TARGET_TRIPLE_ABBREV, scratchRecord,
                             db.getTargetTriple().str());
  currentTriple = db.getTargetTriple();
  for (auto *api : db.api()) {
    currentAPIStart = writer->GetCurrentBitNo();
    writeAPIBlock(*api);
  }
}

void SDKDBWriter::writeAPIBlock(const API& api) {
  if (api.isEmpty())
    return;

  if (builder.excludeBundles() && api.hasBinaryInfo() &&
      api.getBinaryInfo().fileType == FileType::MachO_Bundle)
    return;

  BCBlockRAII restoreBlock(*writer, API_BLOCK_ID, /*abbrevLen=*/5);
  if (api.hasBinaryInfo())
    writeBinaryInfoBlock(api.getBinaryInfo());

  APISerializer serializer(*writer, stringBuilder, currentAPIStart,
                           libraryIndex[currentTriple.str()], builder);
  api.visit(serializer);

  // potentially defined selectors.
  for (auto &selector: api.getPotentiallyDefinedSelectors()) {
    unsigned nameOffset = stringBuilder.getOffset(selector.first());
    scratchRecord = {api_block::POTENTIALLY_DEFINED_SELECTOR, nameOffset,
                     selector.first().size()};
    writer->EmitRecordWithAbbrev(API_POTENTIALLY_DEFINED_SELECTOR_ABBREV,
                                 scratchRecord);
  }

  auto project = api.getProjectName();
  if (!project.empty()) {
    unsigned nameOffset = stringBuilder.getOffset(project);
    scratchRecord = {api_block::PROJECT_NAME, nameOffset, project.size()};
    writer->EmitRecordWithAbbrev(API_PROJECT_NAME_ABBREV, scratchRecord);
  }
}

void SDKDBWriter::writeBinaryInfoBlock(const BinaryInfo &info) {
  auto installName = info.installName;
  if (!installName.empty()) {
    // Insert library path into table.
    libraryIndex[currentTriple.str()][installName] = currentAPIStart;
    // If there is a shallow framework path for the framework, add to table.
    if (auto shallowName = getShallowFrameworkPath(installName))
      libraryIndex[currentTriple.str()][*shallowName] = currentAPIStart;
    auto symlink = symlinkMap.find(installName);
    if (symlink != symlinkMap.end())
      libraryIndex[currentTriple.str()][symlink->second] = currentAPIStart;
    unsigned installNameOffset = stringBuilder.getOffset(installName);
    scratchRecord = {api_block::INSTALL_NAME, installNameOffset,
                     installName.size()};
    writer->EmitRecordWithAbbrev(API_INSTALL_NAME_ABBREV, scratchRecord);
  }

  if (info.fileType != FileType::Invalid) {
    scratchRecord = {api_block::FILE_TYPE, (unsigned)info.fileType};
    writer->EmitRecordWithAbbrev(API_FILE_TYPE_ABBREV, scratchRecord);
  }

  if (builder.hasUUID() && !info.uuid.empty()) {
    scratchRecord = {api_block::UUID};
    writer->EmitRecordWithBlob(API_UUID_ABBREV, scratchRecord, info.uuid);
  }

  for (auto reexport: info.reexportedLibraries) {
    unsigned nameOffset = stringBuilder.getOffset(reexport);
    scratchRecord = {api_block::REEXPORTED, nameOffset, reexport.size()};
    writer->EmitRecordWithAbbrev(API_REEXPORTED_ABBREV, scratchRecord);
  }

  if (!info.parentUmbrella.empty()) {
    unsigned nameOffset = stringBuilder.getOffset(info.parentUmbrella);
    scratchRecord = {api_block::PARENT_UMBRELLA, nameOffset,
                     info.parentUmbrella.size()};
    writer->EmitRecordWithAbbrev(API_PARENT_UMBRELLA_ABBREV, scratchRecord);
  }

  if (info.currentVersion.rawValue() || info.compatibilityVersion.rawValue()) {
    scratchRecord = {api_block::DYLIB_VERSION, info.currentVersion.rawValue(),
                     info.compatibilityVersion.rawValue()};
    writer->EmitRecordWithAbbrev(API_DYLIB_VERSION_ABBREV, scratchRecord);
  }

  if (info.isTwoLevelNamespace || info.isAppExtensionSafe) {
    scratchRecord = {api_block::FLAGS, info.isTwoLevelNamespace,
                     info.isAppExtensionSafe};
    writer->EmitRecordWithAbbrev(API_FLAGS_ABBREV, scratchRecord);
  }

  if (info.swiftABIVersion) {
    scratchRecord = {api_block::SWIFT_VERSION, info.swiftABIVersion};
    writer->EmitRecordWithAbbrev(API_SWIFT_VERSION_ABBREV, scratchRecord);
  }
}

void SDKDBWriter::writeLibraryTable() {
  // Write library lookup table, one block each target.
  SmallString<4096> hashTableBlob;
  for (auto &entry : libraryIndex) {
    BCBlockRAII restoreBlock(*writer, LIBRARY_TABLE_BLOCK_ID, /*abbrevLen=*/3);
    // Write target triple.
    scratchRecord = {library_table_block::TARGET_TRIPLE};
    writer->EmitRecordWithBlob(LIBRARY_TABLE_TARGET_TRIPLE_ABBREV,
                               scratchRecord, entry.getKey());

    // Generate onDisk hash table for table.
    OnDiskChainedHashTableGenerator<LibraryTableInfo> generator;
    LibraryTableInfo info(stringBuilder);
    for (auto &lib : entry.getValue())
      generator.insert(lib.getKey(), lib.getValue(), info);

    raw_svector_ostream blobStream(hashTableBlob);
    // Make sure that no bucket is at offset 0
    support::endian::write<uint64_t>(blobStream, 0, support::little);
    auto tableOffset = generator.Emit(blobStream, info);
    scratchRecord = {library_table_block::LOOKUP_TABLE, tableOffset};
    writer->EmitRecordWithBlob(LIBRARY_TABLE_LOOKUP_TABLE_ABBREV, scratchRecord,
                               hashTableBlob);
  }
}

void APICollector::processAPIRecord(const APIRecord &record) {
  if (builder.isPublicOnly() && (record.access < APIAccess::Public))
    return;
  strTable.add(record.name);
  strTable.add(record.loc.getFilename());
}

void APICollector::processObjCContainer(const ObjCContainerRecord &record) {
  if (builder.isPublicOnly() && (record.access < APIAccess::Public))
    return;

  for (auto *method : record.methods)
    processAPIRecord(*method);

  for (auto *property : record.properties) {
    processAPIRecord(*property);
    strTable.add(property->getterName);
    strTable.add(property->setterName);
  }

  for (auto *ivar : record.ivars) 
    processAPIRecord(*ivar);

  for (auto protocol : record.protocols)
    strTable.add(protocol);
}

void APISerializer::visitGlobal(const GlobalRecord &record) {
  if(!loadRecordIntoScratch(global_block::INFO, record))
    return;

  BCBlockRAII restoreBlock(writer, GLOBAL_BLOCK_ID, /*abbrevLen=*/3);
  scratchRecord.push_back((unsigned)record.kind);
  writer.EmitRecordWithAbbrev(GLOBAL_INFO_ABBREV, scratchRecord);
  writeAvailabilityBlock(record.availability, global_block::AVAILABILITY,
                         GLOBAL_AVAILABILITY_ABBREV);
  writeLocationBlock(record.loc, global_block::FILENAME, GLOBAL_FILENAME_ABBREV,
                     global_block::LOCATION, GLOBAL_LOCATION_ABBREV);
  // Add previous installName into lookup table.
  // Using try_emplace here to not overwriting any value if already exists.
  if (auto name = getPreviousInstallName(record.name))
    libraryIndex.try_emplace(*name, currentAPIStart);
}

void APISerializer::visitObjCInterface(const ObjCInterfaceRecord &record) {
  if (builder.noObjCMetadata())
    return;

  if(!loadRecordIntoScratch(objc_class_block::INFO, record))
    return;

  BCBlockRAII restoreBlock(writer, OBJC_CLASS_BLOCK_ID, /*abbrevLen=*/4);
  unsigned superOffset = stringBuilder.getOffset(record.superClass);
  scratchRecord.push_back(superOffset);
  scratchRecord.push_back(record.superClass.size());
  scratchRecord.push_back(record.hasExceptionAttribute());
  writer.EmitRecordWithAbbrev(OBJC_CLASS_INFO_ABBREV, scratchRecord);
  writeAvailabilityBlock(record.availability, objc_class_block::AVAILABILITY,
                         OBJC_CLASS_AVAILABILITY_ABBREV);
  writeLocationBlock(record.loc, objc_class_block::FILENAME,
                     OBJC_CLASS_FILENAME_ABBREV, objc_class_block::LOCATION,
                     OBJC_CLASS_LOCATION_ABBREV);
  writeObjCContainer(record, objc_class_block::PROTOCOL,
                     OBJC_CLASS_PROTOCOL_ABBREV);
}

void APISerializer::visitObjCCategory(const ObjCCategoryRecord &record) {
  if (builder.noObjCMetadata())
    return;

  if(!loadRecordIntoScratch(objc_category_block::INFO, record))
    return;

  BCBlockRAII restoreBlock(writer, OBJC_CATEGORY_BLOCK_ID, /*abbrevLen=*/4);
  unsigned interfaceOffset = stringBuilder.getOffset(record.interface);
  scratchRecord.push_back(interfaceOffset);
  scratchRecord.push_back(record.interface.size());
  writer.EmitRecordWithAbbrev(OBJC_CATEGORY_INFO_ABBREV, scratchRecord);
  writeAvailabilityBlock(record.availability, objc_category_block::AVAILABILITY,
                         OBJC_CATEGORY_AVAILABILITY_ABBREV);
  writeLocationBlock(
      record.loc, objc_category_block::FILENAME, OBJC_CATEGORY_FILENAME_ABBREV,
      objc_category_block::LOCATION, OBJC_CATEGORY_LOCATION_ABBREV);
  writeObjCContainer(record, objc_category_block::PROTOCOL,
                     OBJC_CATEGORY_PROTOCOL_ABBREV);
}

void APISerializer::visitObjCProtocol(const ObjCProtocolRecord &record) {
  if (builder.noObjCMetadata())
    return;

  if(!loadRecordIntoScratch(objc_protocol_block::INFO, record))
    return;

  BCBlockRAII restoreBlock(writer, OBJC_PROTOCOL_BLOCK_ID, /*abbrevLen=*/4);
  writer.EmitRecordWithAbbrev(OBJC_PROTOCOL_INFO_ABBREV, scratchRecord);
  writeAvailabilityBlock(record.availability, objc_protocol_block::AVAILABILITY,
                         OBJC_PROTOCOL_AVAILABILITY_ABBREV);
  writeLocationBlock(
      record.loc, objc_protocol_block::FILENAME, OBJC_PROTOCOL_FILENAME_ABBREV,
      objc_protocol_block::LOCATION, OBJC_PROTOCOL_LOCATION_ABBREV);
  writeObjCContainer(record, objc_protocol_block::PROTOCOL,
                     OBJC_PROTOCOL_PROTOCOL_ABBREV);
}

void APISerializer::visitEnum(const EnumRecord &record) {
  if (builder.excludeEnumTypes())
    return;

  if(!loadRecordIntoScratch(enum_block::INFO, record))
    return;

  BCBlockRAII restoreBlock(writer, ENUM_BLOCK_ID, /*abbrevLen=*/3);
  unsigned usrOffset = stringBuilder.getOffset(record.usr);
  unsigned usrSize = record.usr.size();
  scratchRecord.push_back(usrOffset);
  scratchRecord.push_back(usrSize);
  writer.EmitRecordWithAbbrev(ENUM_INFO_ABBREV, scratchRecord);
  writeAvailabilityBlock(record.availability, enum_block::AVAILABILITY,
                         ENUM_AVAILABILITY_ABBREV);
  writeLocationBlock(record.loc, enum_block::FILENAME, ENUM_FILENAME_ABBREV,
                     enum_block::LOCATION, ENUM_LOCATION_ABBREV);

  for (auto *c : record.constants) {
    if (!loadRecordIntoScratch(enum_constant_block::INFO, *c))
      return;

    BCBlockRAII restoreBlock(writer, ENUM_CONSTANT_BLOCK_ID, /*abbrevLen=*/3);
    writer.EmitRecordWithAbbrev(ENUM_CONSTANT_INFO_ABBREV, scratchRecord);
    writeAvailabilityBlock(c->availability, enum_constant_block::AVAILABILITY,
                           ENUM_CONSTANT_AVAILABILITY_ABBREV);
    writeLocationBlock(
        c->loc, enum_constant_block::FILENAME, ENUM_CONSTANT_FILENAME_ABBREV,
        enum_constant_block::LOCATION, ENUM_CONSTANT_LOCATION_ABBREV);
  }
}

void APISerializer::visitTypeDef(const TypedefRecord &record) {
  if (builder.excludeEnumTypes())
    return;

  if (!loadRecordIntoScratch(typedef_block::INFO, record))
    return;

  BCBlockRAII restoreBlock(writer, TYPEDEF_BLOCK_ID, /*abbrevLen=*/3);
  writer.EmitRecordWithAbbrev(TYPEDEF_INFO_ABBREV, scratchRecord);
  writeAvailabilityBlock(record.availability, typedef_block::AVAILABILITY,
                         TYPEDEF_AVAILABILITY_ABBREV);
  writeLocationBlock(record.loc, typedef_block::FILENAME,
                     TYPEDEF_FILENAME_ABBREV, typedef_block::LOCATION,
                     TYPEDEF_LOCATION_ABBREV);
}

bool APISerializer::loadRecordIntoScratch(unsigned abbrev,
                                          const APIRecord &record) {
  // skipped non public ones if we are in public only mode.
  if (builder.isPublicOnly() && (record.access < APIAccess::Public))
    return false;

  unsigned nameOffset = stringBuilder.getOffset(record.name);
  unsigned nameSize = record.name.size();
  // Maskout the high bits of flags to prevent version mismatch.
  // FIXME: The 3 bit restriction should be removed in the next major
  // update.
  const unsigned flags = (uint8_t)record.flags & 0x1f;
  scratchRecord = {abbrev,
                   nameOffset,
                   nameSize,
                   (unsigned)record.access,
                   (unsigned)record.linkage,
                   flags};
  return true;
}

void APISerializer::writeAvailabilityBlock(const AvailabilityInfo &info,
                                           unsigned id, unsigned abbrev) {
  if (info.isDefault())
    return;

  scratchRecord = {id, info._introduced.rawValue(), info._obsoleted.rawValue(),
                   info._unavailable, info._isSPIAvailable};
  writer.EmitRecordWithAbbrev(abbrev, scratchRecord);
}

void APISerializer::writeLocationBlock(const APILoc &loc, unsigned fileID,
                                       unsigned fileAbbrev, unsigned locID,
                                       unsigned locAbbrev) {
  if (loc.isInvalid())
    return;

  unsigned fileOffset = stringBuilder.getOffset(loc.getFilename());
  unsigned fileSize = loc.getFilename().size();
  scratchRecord = {fileID, fileOffset, fileSize};
  writer.EmitRecordWithAbbrev(fileAbbrev, scratchRecord);

  if (!builder.preserveLocation())
    return;

  scratchRecord = {locID, loc.getLine(), loc.getColumn()};
  writer.EmitRecordWithAbbrev(locAbbrev, scratchRecord);
}

void APISerializer::writeObjCContainer(const ObjCContainerRecord &record,
                                       unsigned protocolID,
                                       unsigned protocolAbbrev) {
  for (auto *method : record.methods)
    writeObjCMethod(*method);

  for (auto *property : record.properties)
    writeObjCProperty(*property);

  for (auto *ivar : record.ivars)
    writeObjCInstanceVariable(*ivar);

  for (auto protocol : record.protocols) {
    unsigned nameOffset = stringBuilder.getOffset(protocol);
    scratchRecord = {protocolID, nameOffset, protocol.size()};
    writer.EmitRecordWithAbbrev(protocolAbbrev, scratchRecord);
  }
}

void APISerializer::writeObjCMethod(const ObjCMethodRecord &record) {
  if(!loadRecordIntoScratch(objc_method_block::INFO, record))
    return;

  BCBlockRAII restoreBlock(writer, OBJC_METHOD_BLOCK_ID, /*abbrevLen=*/3);
  scratchRecord.push_back(record.isInstanceMethod);
  scratchRecord.push_back(record.isOptional);
  scratchRecord.push_back(record.isDynamic);
  writer.EmitRecordWithAbbrev(OBJC_METHOD_ABBREV, scratchRecord);
  writeAvailabilityBlock(record.availability, objc_method_block::AVAILABILITY,
                         OBJC_METHOD_AVAILABILITY_ABBREV);
  writeLocationBlock(record.loc, objc_method_block::FILENAME,
                     OBJC_METHOD_FILENAME_ABBREV, objc_method_block::LOCATION,
                     OBJC_METHOD_LOCATION_ABBREV);
}

void APISerializer::writeObjCProperty(const ObjCPropertyRecord &record) {
  if(!loadRecordIntoScratch(objc_property_block::INFO, record))
    return;

  BCBlockRAII restoreBlock(writer, OBJC_PROPERTY_BLOCK_ID, /*abbrevLen=*/3);
  scratchRecord.push_back((unsigned)record.attributes);
  scratchRecord.push_back(record.isOptional);
  scratchRecord.push_back(stringBuilder.getOffset(record.getterName));
  scratchRecord.push_back(record.getterName.size());
  scratchRecord.push_back(stringBuilder.getOffset(record.setterName));
  scratchRecord.push_back(record.setterName.size());
  writer.EmitRecordWithAbbrev(OBJC_PROPERTY_ABBREV, scratchRecord);
  writeAvailabilityBlock(record.availability, objc_property_block::AVAILABILITY,
                         OBJC_PROPERTY_AVAILABILITY_ABBREV);
  writeLocationBlock(
      record.loc, objc_property_block::FILENAME, OBJC_PROPERTY_FILENAME_ABBREV,
      objc_property_block::LOCATION, OBJC_PROPERTY_LOCATION_ABBREV);
}

void APISerializer::writeObjCInstanceVariable(
    const ObjCInstanceVariableRecord &record) {
  if (!loadRecordIntoScratch(objc_ivar_block::INFO, record))
    return;

  BCBlockRAII restoreBlock(writer, OBJC_IVAR_BLOCK_ID, /*abbrevLen=*/3);
  scratchRecord.push_back((unsigned)record.accessControl);
  writer.EmitRecordWithAbbrev(OBJC_IVAR_ABBREV, scratchRecord);
  writeAvailabilityBlock(record.availability, objc_ivar_block::AVAILABILITY,
                         OBJC_IVAR_AVAILABILITY_ABBREV);
  writeLocationBlock(record.loc, objc_ivar_block::FILENAME,
                     OBJC_IVAR_FILENAME_ABBREV, objc_ivar_block::LOCATION,
                     OBJC_IVAR_LOCATION_ABBREV);
}

TAPI_NAMESPACE_INTERNAL_END
