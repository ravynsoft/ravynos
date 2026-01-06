//===- ObjCMetadata.cpp - ObjC Metadata Reader ------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the classes needed to parse ObjCMetadata.
//
//===----------------------------------------------------------------------===//
#include "tapi/ObjCMetadata/ObjCMetadata.h"
#include "macho-obj.h"

using namespace llvm;
using namespace object;

Expected<ObjCClass> ObjCClassRef::getObjCClass() const {
  return MetadataReader->getObjCClassFromRef(*this);
}

Expected<ObjCCategory> ObjCCategoryRef::getObjCCategory() const {
  return MetadataReader->getObjCCategoryFromRef(*this);
}

Expected<ObjCProtocol> ObjCProtocolRef::getObjCProtocol() const {
  return MetadataReader->getObjCProtocolFromRef(*this);
}

Expected<StringRef> ObjCClass::getName() const {
  return MetadataReader->getObjCClassName(*this);
}

Expected<StringRef> ObjCClassRef::getClassName() const {
  if (MetadataReader->isObjCClassExternal(*this)) {
    StringRef symName = MetadataReader->getSymbolNameFromRef(Data);
    return MetadataReader->guessClassNameBasedOnSymbol(symName);
  } else if (auto class_t = getObjCClass())
    return class_t->getName();
  else
    return class_t.takeError();
}

bool ObjCClassRef::isExternal() const {
  return MetadataReader->isObjCClassExternal(*this);
}

Expected<StringRef> ObjCClass::getSuperClassName() const {
  return MetadataReader->getObjCSuperClassName(*this);
}

Expected<bool> ObjCClass::isSwift() const {
  return MetadataReader->isObjCClassSwift(*this);
}

Expected<bool> ObjCClass::isMetaClass() const {
  return MetadataReader->isObjCClassMetaClass(*this);
}

Expected<ObjCPropertyList> ObjCClass::properties() const {
  return MetadataReader->materializePropertyList(*this);
}

Expected<ObjCPropertyList> ObjCClass::classProperties() const {
  return MetadataReader->materializeClassPropertyList(*this);
}

Expected<ObjCMethodList> ObjCClass::instanceMethods() const {
  return MetadataReader->materializeInstanceMethodList(*this);
}

Expected<ObjCMethodList> ObjCClass::classMethods() const {
  return MetadataReader->materializeClassMethodList(*this);
}

Expected<StringRef> ObjCCategory::getName() const {
  return MetadataReader->getObjCCategoryName(*this);
}

Expected<StringRef> ObjCCategory::getBaseClassName() const {
  return MetadataReader->getObjCCategoryBaseClassName(*this);
}

Expected<ObjCPropertyList> ObjCCategory::properties() const {
  return MetadataReader->materializePropertyList(*this);
}

Expected<ObjCMethodList> ObjCCategory::instanceMethods() const {
  return MetadataReader->materializeInstanceMethodList(*this);
}

Expected<ObjCMethodList> ObjCCategory::classMethods() const {
  return MetadataReader->materializeClassMethodList(*this);
}

Expected<StringRef> ObjCProtocol::getName() const {
  return MetadataReader->getObjCProtocolName(*this);
}

Expected<ObjCPropertyList> ObjCProtocol::properties() const {
  return MetadataReader->materializePropertyList(*this);
}

Expected<ObjCMethodList> ObjCProtocol::instanceMethods() const {
  return MetadataReader->materializeInstanceMethodList(*this);
}

Expected<ObjCMethodList> ObjCProtocol::classMethods() const {
  return MetadataReader->materializeClassMethodList(*this);
}

Expected<ObjCMethodList> ObjCProtocol::optionalInstanceMethods() const {
  return MetadataReader->materializeOptionalInstanceMethodList(*this);
}

Expected<ObjCMethodList> ObjCProtocol::optionalClassMethods() const {
  return MetadataReader->materializeOptionalClassMethodList(*this);
}

Expected<StringRef> ObjCProperty::getName() const {
  return MetadataReader->getPropertyName(*this);
}

Expected<StringRef> ObjCProperty::getAttribute() const {
  return MetadataReader->getPropertyAttribute(*this);
}

Expected<bool> ObjCProperty::isDynamic() const {
  auto Attr = getAttribute();
  if (!Attr)
    return Attr.takeError();
  // Find @dynamic attribute.
  SmallVector<StringRef, 4> Attrs;
  Attr->split(Attrs, ',');
  for (auto a : Attrs) {
    if (a == "D")
      return true;
  }
  return false;
}


Expected<std::string> ObjCProperty::getGetter() const {
  auto Name = getName();
  if (!Name)
    return Name.takeError();
  if (*Name == "#EncryptedString#")
    return Name->str();
  auto Attr = getAttribute();
  if (!Attr)
    return Attr.takeError();
  // Find getter attribute.
  SmallVector<StringRef, 4> Attrs;
  Attr->split(Attrs, ',');
  for (auto a : Attrs) {
    if (a.startswith("G"))
      return a.drop_front(1).str();
  }
  // Otherwise returns property name.
  return Name->str();
}

Expected<std::string> ObjCProperty::getSetter() const {
  auto Name = getName();
  if (!Name)
    return Name.takeError();
  if (*Name == "#EncryptedString#")
    return Name->str();
  auto Attr = getAttribute();
  if (!Attr)
    return Attr.takeError();
  // Find setter attribute.
  SmallVector<StringRef, 4> Attrs;
  Attr->split(Attrs, ',');
  for (auto a : Attrs) {
    if (a.startswith("S"))
      return a.drop_front(1).str();
    else if (a.startswith("R"))
      return std::string(); // Read-only property
  }
  // Otherwise returns property name.
  std::string setter =
      "set" + Name->substr(0, 1).upper() + Name->substr(1).str() + ":";
  return setter;
}

Expected<StringRef> ObjCMethod::getName() const {
  return MetadataReader->getMethodName(*this);
}

Expected<StringRef> ObjCMethod::getType() const {
  return MetadataReader->getMethodType(*this);
}

Expected<StringRef> ObjCSelectorRef::getSelector() const {
  return MetadataReader->getObjCSelectorName(*this);
}

Expected<StringRef>
ObjCMetaDataReader::convertSwiftVersion(unsigned raw) const {
  switch (raw) {
  case 0:
    return "";
  case 1:
    return "1.0";
  case 2:
    return "1.1";
  case 3:
    return "2.0";
  case 4:
    return "3.0";
  case 5:
    return "4.0";
  case 6:
    return "4.1/4.2";
  case 7:
    return "5.0 or later";
  default:
    return "Unknown future swift version";
  }
}

static Error unsupportedObjCRuntimeError() {
  return make_error<StringError>("Unsupported ObjC Runtime",
                                 object_error::parse_failed);
}

ObjCMetaDataReader::ObjCMetaDataReader(object::MachOObjectFile *Binary,
                                       Error &Err)
    : OwningBinary(Binary) {
  // Identify ObjC Version.
  if (getSection("__OBJC", "__module_info") != SectionRef() &&
      !OwningBinary->is64Bit())
    ObjCVersion = ObjC1;
  else if ((getSection("__OBJC2", "__module_info") != SectionRef()) ||
           (getSection("__DATA_CONST", "__objc_imageinfo") != SectionRef()) ||
           (getSection("__DATA", "__objc_imageinfo") != SectionRef()))
    ObjCVersion = ObjC2;
  else
    ObjCVersion = Unknown;

  // Build Address to symbol name lookup.
  struct SectionInfo {
    uint64_t Address;
    uint64_t Size;
    uint64_t OffsetInSegment;
    uint64_t SegmentStartAddress;
    uint32_t SegmentIndex;
  };
  SmallVector<SectionInfo, 32> Sections;
  uint32_t CurSegIndex = Binary->hasPageZeroSegment() ? 1 : 0;
  StringRef CurSegName;
  uint64_t CurSegAddress;
  for (const SectionRef &Section : Binary->sections()) {
    SectionInfo Info;
    Info.Address = Section.getAddress();
    Info.Size = Section.getSize();
    StringRef SegmentName =
        Binary->getSectionFinalSegmentName(Section.getRawDataRefImpl());
    if (!SegmentName.equals(CurSegName)) {
      ++CurSegIndex;
      CurSegName = SegmentName;
      CurSegAddress = Info.Address;
    }
    Info.SegmentIndex = CurSegIndex - 1;
    Info.OffsetInSegment = Info.Address - CurSegAddress;
    Info.SegmentStartAddress = CurSegAddress;
    Sections.push_back(Info);
  }

  // Cache export entries for lookups.
  StringMap<uint64_t> ExportsToAddress;
  for (const auto &Export : OwningBinary->exports(Err))
    ExportsToAddress[Export.name()] = Export.address();

  if (Err)
    return;

  // Cache the address of binds from the binary. Since we are walking ObjC
  // Metadata, the external symbols cannot be weak binds.

    // FIXME: Threaded rebase logic should move to libObject.
    uint64_t TextAddress = 0;
    for (const auto &Command : Binary->load_commands()) {
      if (Command.C.cmd == MachO::LC_SEGMENT) {
        MachO::segment_command SLC = Binary->getSegmentLoadCommand(Command);
        if (StringRef(SLC.segname) == StringRef("__TEXT")) {
          TextAddress = SLC.vmaddr;
          break;
        }
      } else if (Command.C.cmd == MachO::LC_SEGMENT_64) {
        MachO::segment_command_64 SLC_64 =
            Binary->getSegment64LoadCommand(Command);
        if (StringRef(SLC_64.segname) == StringRef("__TEXT")) {
          TextAddress = SLC_64.vmaddr;
          break;
        }
      }
    }
    // For none chained fixup, walk bind table and rebase table.
    for (const auto &Entry : OwningBinary->bindTable(Err)) {
      if (Err)
        return;
      if (Entry.ordinal() == MachO::BIND_SPECIAL_DYLIB_SELF) {
        // This is a bind to local symbol. Resolve it to actual address.
        auto Addr = ExportsToAddress.find(Entry.symbolName());
        if (Addr != ExportsToAddress.end()) {
          VMAddrPointToValueMap[Entry.address()] = Addr->second;
          continue;
        }
      }
      VMAddrToSymbolMap[Entry.address()] = Entry.symbolName();
    }
    if (Err) {
      consumeError(std::move(Err));
      Err = Error::success();
    }

  // cache all external relocations.
  for (const auto &S : Binary->sections()) {
    for (const auto &Reloc : S.relocations()) {
      uint64_t Address = S.getAddress() + Reloc.getOffset();
      auto Rel = Reloc.getRawDataRefImpl();
      auto RE = Binary->getRelocation(Rel);
      if (Binary->isRelocationScattered(RE) ||
          !Binary->getPlainRelocationExternal(RE))
        continue;

      uint64_t PtrValue = 0;
      if (Binary->is64Bit()) {
        auto Addend = getData<uint64_t>(Address);
        if (!Addend) {
          consumeError(Addend.takeError());
          continue;
        }
        PtrValue = *Addend;
      } else {
        auto Addend = getData<uint32_t>(Address);
        if (!Addend) {
          consumeError(Addend.takeError());
          continue;
        }
        PtrValue = *Addend;
      }
#ifdef notyet // ravynOS LLVM does not have this atm.
      if (OwningBinary->getAnyRelocationType(RE) ==
          MachO::ARM64_RELOC_AUTHENTICATED_POINTER) {
        PtrValue = 0xffffffffULL & PtrValue;
        if (PtrValue & 0x80000000ULL) {
          PtrValue |= 0xffffffff00000000ULL;
        }
      }
#endif
      // Cache the external relocation names as well.
      auto Symbol = Reloc.getSymbol();
      auto NameOrError = Symbol->getName();
      if (!NameOrError) {
        consumeError(NameOrError.takeError());
        continue;
      }
      VMAddrToSymbolMap[Address] = *NameOrError;
      Expected<uint64_t> SymbolVal = Symbol->getValue();
      if (!SymbolVal)
        Err = SymbolVal.takeError();
      PtrValue += *SymbolVal;
      VMAddrPointToValueMap[Address] = PtrValue;
    }
  }
}

Expected<ObjCClassList> ObjCMetaDataReader::classes() const {
  ObjCClassList ClassList;
  if (isObjC2()) {
    SectionRef ClassSect = getSection("__DATA_CONST", "__objc_classlist");
    if (ClassSect == SectionRef())
      ClassSect = getSection("__DATA", "__objc_classlist");
    if (ClassSect == SectionRef())
      ClassSect = getSection("__OBJC2", "__class_list");
    // Return empty if the section doesn't exists.
    if (ClassSect == SectionRef())
      return ClassList;
    if (ClassSect.getSize() % OwningBinary->getBytesInAddress() != 0)
      return make_error<StringError>(
          "objc class section size is not a multiple of pointer size",
          object_error::parse_failed);

    for (unsigned i = 0; i < ClassSect.getSize();
         i += OwningBinary->getBytesInAddress())
      ClassList.push_back(ObjCClassRef(this, ClassSect.getAddress() + i));
  } else if (isObjC1()) {
    auto ObjCModules = getObjC1Modules();
    if (!ObjCModules)
      return ObjCModules.takeError();
    for (auto &M : *ObjCModules) {
      auto sym = getObjC1Symtab(M);
      if (!sym)
        return sym.takeError();
      // Handle empty symtab.
      if (*sym == 0)
        continue;
      if (auto cls = getObjC1ClassesFromSymtab(*sym))
        ClassList.append(cls->begin(), cls->end());
      else
        return cls.takeError();
    }
  }

  return ClassList;
}

Expected<ObjCClass>
ObjCMetaDataReader::getObjCClassFromRef(ObjCClassRef Ref) const {
  if (OwningBinary->is64Bit()) {
    if (auto class_ptr = getPointerValue64(Ref.getAddress()))
      return ObjCClass(this, *class_ptr);
    else
      return class_ptr.takeError();
  } else {
    if (auto class_ptr = getPointerValue32(Ref.getAddress()))
      return ObjCClass(this, *class_ptr);
    else
      return class_ptr.takeError();
  }
}

bool ObjCMetaDataReader::isObjCClassExternal(ObjCClassRef Ref) const {
  // If there are external relocations or external symbols at the address,
  // return true;
  if (VMAddrToSymbolMap.count(Ref.getAddress()))
    return true;

  // If the pointer is nullptr, just return true.
  if (OwningBinary->is64Bit()) {
    if (auto Value = getPointerValue64(Ref.getAddress()))
      return !*Value;
    else {
      consumeError(Value.takeError());
      return true;
    }
  } else {
    if (auto Value = getPointerValue32(Ref.getAddress()))
      return !*Value;
    else {
      consumeError(Value.takeError());
      return true;
    }
  }

  return false;
}

StringRef ObjCMetaDataReader::getSymbolNameFromRef(ObjCRef Ref) const {
  return getSymbol(Ref);
}

StringRef ObjCMetaDataReader::guessClassNameBasedOnSymbol(StringRef Sym) const {
  if (Sym.startswith("_OBJC_CLASS_$_"))
    return Sym.drop_front(strlen("_OBJC_CLASS_$_"));
  else if (Sym.startswith(".objc_class_name_"))
    return Sym.drop_front(strlen(".objc_class_name_"));
  else
    return Sym;
}

Expected<ObjCCategoryList> ObjCMetaDataReader::categories() const {
  ObjCCategoryList CategoryList;
  if (isObjC2()) {
    SectionRef CatSect = getSection("__DATA_CONST", "__objc_catlist");
    if (CatSect == SectionRef())
      CatSect = getSection("__DATA", "__objc_catlist");
    if (CatSect == SectionRef())
      CatSect = getSection("__OBJC2", "__category_list");
    // Return empty if the section doesn't exists.
    if (CatSect == SectionRef())
      return CategoryList;
    if (CatSect.getSize() % OwningBinary->getBytesInAddress() != 0)
      return make_error<StringError>(
          "objc category section size is not a multiple of pointer size",
          object_error::parse_failed);

    for (unsigned i = 0; i < CatSect.getSize();
         i += OwningBinary->getBytesInAddress())
      CategoryList.push_back(ObjCCategoryRef(this, CatSect.getAddress() + i));
  } else if (isObjC1()) {
    auto ObjCModules = getObjC1Modules();
    if (!ObjCModules)
      return ObjCModules.takeError();
    for (auto &M : *ObjCModules) {
      auto sym = getObjC1Symtab(M);
      if (!sym)
        return sym.takeError();
      // Handle empty symtab.
      if (*sym == 0)
        continue;
      if (auto cat = getObjC1CategoriesFromSymtab(*sym))
        CategoryList.append(cat->begin(), cat->end());
      else
        return cat.takeError();
    }
  }

  return CategoryList;
}

Expected<ObjCCategory>
ObjCMetaDataReader::getObjCCategoryFromRef(ObjCCategoryRef Ref) const {
  if (OwningBinary->is64Bit()) {
    if (auto cat_ptr = getPointerValue64(Ref.getAddress()))
      return ObjCCategory(this, *cat_ptr);
    else
      return cat_ptr.takeError();
  } else {
    if (auto cat_ptr = getPointerValue32(Ref.getAddress()))
      return ObjCCategory(this, *cat_ptr);
    else
      return cat_ptr.takeError();
  }
}

Expected<ObjCProtocolList> ObjCMetaDataReader::protocols() const {
  ObjCProtocolList ProtocolList;
  if (isObjC2()) {
    SectionRef ProtoSect = getSection("__DATA_CONST", "__objc_protolist");
    if (ProtoSect == SectionRef())
      ProtoSect = getSection("__DATA", "__objc_protolist");
    if (ProtoSect == SectionRef())
      ProtoSect = getSection("__OBJC2", "__protocol_list");
    // Return empty if the section doesn't exists.
    if (ProtoSect == SectionRef())
      return ProtocolList;
    if (ProtoSect.getSize() % OwningBinary->getBytesInAddress() != 0)
      return make_error<StringError>(
          "objc protocol section size is not a multiple of pointer size",
          object_error::parse_failed);

    for (unsigned i = 0; i < ProtoSect.getSize();
         i += OwningBinary->getBytesInAddress())
      ProtocolList.push_back(ObjCProtocolRef(this, ProtoSect.getAddress() + i));
  }
  // TODO: Add support for ObjC1, return empty list for now.
  return ProtocolList;
}

Expected<ObjCProtocol>
ObjCMetaDataReader::getObjCProtocolFromRef(ObjCProtocolRef Ref) const {
  if (OwningBinary->is64Bit()) {
    if (auto proto_ptr = getPointerValue64(Ref.getAddress()))
      return ObjCProtocol(this, *proto_ptr);
    else
      return proto_ptr.takeError();
  } else {
    if (auto proto_ptr = getPointerValue32(Ref.getAddress()))
      return ObjCProtocol(this, *proto_ptr);
    else
      return proto_ptr.takeError();
  }
}

Expected<StringRef>
ObjCMetaDataReader::getObjCClassName(ObjCClass ClassRef) const {
  if (isObjC2()) {
    if (OwningBinary->is64Bit()) {
      if (auto ro = getObjC2ClassRO64(ClassRef)) {
        if (auto name = getPointerValue64(*ro + offsetof(class_ro64_t, name)))
          return getString(*name);
        else
          return name.takeError();
      } else
        return ro.takeError();
    } else {
      if (auto ro = getObjC2ClassRO32(ClassRef)) {
        if (auto name = getPointerValue32(*ro + offsetof(class_ro32_t, name)))
          return getString(*name);
        else
          return name.takeError();
      } else
        return ro.takeError();
    }
  } else if (isObjC1()) {
    if (auto name = getPointerValue32(ClassRef.getAddress() +
                                      offsetof(objc_class_t, name)))
      return getString(*name);
    else
      return name.takeError();
  }
  return unsupportedObjCRuntimeError();
}

Expected<StringRef>
ObjCMetaDataReader::getObjCSuperClassName(ObjCClass Data) const {
  if (isObjC2()) {
    if (OwningBinary->is64Bit()) {
      uint64_t superAddr = Data.getAddress() + offsetof(class64_t, superclass);
      ObjCClassRef super(this, superAddr);
      return super.getClassName();
    } else {
      uint64_t superAddr = Data.getAddress() + offsetof(class32_t, superclass);
      ObjCClassRef super(this, superAddr);
      return super.getClassName();
    }
  } else if (isObjC1()) {
    if (auto super = getPointerValue32(Data.getAddress() +
                                       offsetof(objc_class_t, super_class))) {
      if (*super)
        return getString(*super);
      else
        return StringRef();
    } else
      return super.takeError();
  }
  return unsupportedObjCRuntimeError();
}

Expected<bool> ObjCMetaDataReader::isObjCClassSwift(ObjCClass Data) const {
  if (isObjC1())
    return false;
  if (OwningBinary->is64Bit()) {
    if (auto class_ro =
            getPointerValue64(Data.getAddress() + offsetof(class64_t, data))) {
      return ((*class_ro & ~0x7) != 0);
    } else
      return class_ro.takeError();
  } else {
    if (auto class_ro =
            getPointerValue32(Data.getAddress() + offsetof(class32_t, data))) {
      return ((*class_ro & ~0x3) != 0);
    } else
      return class_ro.takeError();
  }
}

Expected<bool> ObjCMetaDataReader::isObjCClassMetaClass(ObjCClass Data) const {
  if (isObjC2()) {
    if (OwningBinary->is64Bit()) {
      if (auto class_ro = getObjC2ClassRO64(Data)) {
        if (auto flags =
                getData<uint32_t>(*class_ro + offsetof(class_ro64_t, flags))) {
          return (*flags & RO_META);
        } else
          return flags.takeError();
      } else
        return class_ro.takeError();
    } else {
      if (auto class_ro = getObjC2ClassRO32(Data)) {
        if (auto flags =
                getData<uint32_t>(*class_ro + offsetof(class_ro32_t, flags))) {
          return (*flags & RO_META);
        } else
          return flags.takeError();
      } else
        return class_ro.takeError();
    }
  } else if (isObjC1()) {
    if (auto ClassInfo = getData<int32_t>(Data.getAddress() +
                                          offsetof(objc_class_t, info))) {
      return (*ClassInfo & CLS_META);
    } else
      return ClassInfo.takeError();
  }
  return unsupportedObjCRuntimeError();
}

Expected<ObjCPropertyList>
ObjCMetaDataReader::materializePropertyList(ObjCClass ClassRef) const {
  ObjCPropertyList PropertyList;
  if (isObjC2()) {
    if (OwningBinary->is64Bit()) {
      auto RO = getObjC2ClassRO64(ClassRef);
      if (!RO)
        return RO.takeError();

      auto BaseProperties =
          getPointerValue64(*RO + offsetof(class_ro64_t, baseProperties));
      if (!BaseProperties)
        return BaseProperties.takeError();
      if (!*BaseProperties)
        return PropertyList;

      auto PropList = getData<objc_property_list64>(*BaseProperties);
      if (!PropList)
        return PropList.takeError();
      for (unsigned Idx = 0; Idx < PropList->count; ++Idx)
        PropertyList.emplace_back(this, *BaseProperties +
                                            sizeof(objc_property_list64) +
                                            Idx * sizeof(objc_property64));

      return PropertyList;
    } else {
      auto RO = getObjC2ClassRO32(ClassRef);
      if (!RO)
        return RO.takeError();

      auto BaseProperties =
          getPointerValue32(*RO + offsetof(class_ro32_t, baseProperties));
      if (!BaseProperties)
        return BaseProperties.takeError();
      if (!*BaseProperties)
        return PropertyList;

      auto PropList = getData<objc_property_list32>(*BaseProperties);
      if (!PropList)
        return PropList.takeError();
      for (unsigned Idx = 0; Idx < PropList->count; ++Idx)
        PropertyList.emplace_back(this, *BaseProperties +
                                            sizeof(objc_property_list32) +
                                            Idx * sizeof(objc_property32));
      return PropertyList;
    }
  } else if (isObjC1()) {
    // Just return empty list.
    return PropertyList;
  }
  return unsupportedObjCRuntimeError();
}

Expected<ObjCPropertyList>
ObjCMetaDataReader::materializeClassPropertyList(ObjCClass ClassRef) const {
  if (isObjC2()) {
    if (OwningBinary->is64Bit()) {
      if (auto meta = getObjC2MetaClass64(ClassRef)) {
        if (meta->getAddress() == 0)
          return ObjCPropertyList();
        else
          return materializePropertyList(*meta);
      } else
        return meta.takeError();
    } else {
      if (auto meta = getObjC2MetaClass32(ClassRef)) {
        if (meta->getAddress() == 0)
          return ObjCPropertyList();
        else
          return materializePropertyList(*meta);
      } else
        return meta.takeError();
    }
  } else if (isObjC1())
    return ObjCPropertyList();

  return unsupportedObjCRuntimeError();
}

Expected<StringRef>
ObjCMetaDataReader::getPropertyName(ObjCProperty Data) const {
  if (!isObjC2())
    return unsupportedObjCRuntimeError();
  if (OwningBinary->is64Bit()) {
    auto NamePtr =
        getPointerValue64(Data.getAddress() + offsetof(objc_property64, name));
    if (!NamePtr)
      return NamePtr.takeError();
    return getString(*NamePtr);
  } else {
    auto NamePtr =
        getPointerValue32(Data.getAddress() + offsetof(objc_property32, name));
    if (!NamePtr)
      return NamePtr.takeError();
    return getString(*NamePtr);
  }
}

Expected<StringRef>
ObjCMetaDataReader::getPropertyAttribute(ObjCProperty Data) const {
  if (!isObjC2())
    return unsupportedObjCRuntimeError();
  if (OwningBinary->is64Bit()) {
    auto NamePtr = getPointerValue64(Data.getAddress() +
                                     offsetof(objc_property64, attributes));
    if (!NamePtr)
      return NamePtr.takeError();
    return getString(*NamePtr);
  } else {
    auto NamePtr = getPointerValue32(Data.getAddress() +
                                     offsetof(objc_property32, attributes));
    if (!NamePtr)
      return NamePtr.takeError();
    return getString(*NamePtr);
  }
}

Expected<ObjCMethodList>
ObjCMetaDataReader::materializeInstanceMethodList(ObjCClass ClassRef) const {
  if (isObjC2()) {
    ObjCMethodList MethodList;
    if (OwningBinary->is64Bit()) {
      auto RO = getObjC2ClassRO64(ClassRef);
      if (!RO)
        return RO.takeError();
      auto BaseMethods =
          getPointerValue64(*RO + offsetof(class_ro64_t, baseMethods));
      if (!BaseMethods)
        return BaseMethods.takeError();
      if (!*BaseMethods)
        return MethodList;

      auto MList = getData<method_list64_t>(*BaseMethods);
      if (!MList)
        return MList.takeError();
      bool RA = MList->entsize & METHOD_LIST_ENTSIZE_FLAG_RELATIVE;
      auto EntrySize = RA ? sizeof(method_rel_t) : sizeof(method64_t);
      for (unsigned Idx = 0; Idx < MList->count; ++Idx)
        MethodList.push_back(ObjCMethod(
            this, *BaseMethods + sizeof(method_list64_t) + Idx * EntrySize,
            RA));
      return MethodList;
    } else {
      auto RO = getObjC2ClassRO32(ClassRef);
      if (!RO)
        return RO.takeError();
      auto BaseMethods =
          getPointerValue32(*RO + offsetof(class_ro32_t, baseMethods));
      if (!BaseMethods)
        return BaseMethods.takeError();
      if (!*BaseMethods)
        return MethodList;

      auto MList = getData<method_list32_t>(*BaseMethods);
      if (!MList)
        return MList.takeError();
      bool RA = MList->entsize & METHOD_LIST_ENTSIZE_FLAG_RELATIVE;
      auto EntrySize = RA ? sizeof(method_rel_t) : sizeof(method32_t);
      for (unsigned Idx = 0; Idx < MList->count; ++Idx)
        MethodList.push_back(ObjCMethod(
            this, *BaseMethods + sizeof(method_list32_t) + Idx * EntrySize,
            RA));
      return MethodList;
    }
  } else if (isObjC1()) {
    uint64_t Methods =
        ClassRef.getAddress() + offsetof(objc_class_t, methodLists);
    return getObjC1MethodList(ObjCRef(Methods));
  }
  return unsupportedObjCRuntimeError();
}

Expected<ObjCMethodList>
ObjCMetaDataReader::materializeClassMethodList(ObjCClass ClassRef) const {
  if (isObjC2()) {
    if (OwningBinary->is64Bit()) {
      if (auto meta = getObjC2MetaClass64(ClassRef)) {
        if (meta->getAddress() == 0)
          return ObjCMethodList();
        else
          return materializeInstanceMethodList(*meta);
      } else
        return meta.takeError();
    } else {
      if (auto meta = getObjC2MetaClass32(ClassRef)) {
        if (meta->getAddress() == 0)
          return ObjCMethodList();
        else
          return materializeInstanceMethodList(*meta);
      } else
        return meta.takeError();
    }
  } else if (isObjC1()) {
    ObjCClassRef isa(this, ClassRef.getAddress() + offsetof(objc_class_t, isa));
    auto C = getObjCClassFromRef(isa);
    if (!C)
      return C.takeError();
    return materializeInstanceMethodList(*C);
  }
  return unsupportedObjCRuntimeError();
}

Expected<StringRef>
ObjCMetaDataReader::getObjCCategoryName(ObjCCategory Cat) const {
  if (isObjC2()) {
    if (OwningBinary->is64Bit()) {
      auto CatPtr =
          getPointerValue64(Cat.getAddress() + offsetof(category64_t, name));
      if (!CatPtr)
        return CatPtr.takeError();
      return getString(*CatPtr);
    } else {
      auto CatPtr =
          getPointerValue32(Cat.getAddress() + offsetof(category32_t, name));
      if (!CatPtr)
        return CatPtr.takeError();
      return getString(*CatPtr);
    }
  } else if (isObjC1()) {
    auto NamePtr = getPointerValue32(Cat.getAddress() +
                                     offsetof(objc_category_t, category_name));
    if (!NamePtr)
      return NamePtr.takeError();
    return getString(*NamePtr);
  }

  return unsupportedObjCRuntimeError();
}

Expected<StringRef>
ObjCMetaDataReader::getObjCCategoryBaseClassName(ObjCCategory Cat) const {
  if (isObjC2()) {
    if (OwningBinary->is64Bit()) {
      uint64_t baseClassAddr = Cat.getAddress() + offsetof(category64_t, cls);
      ObjCClassRef cls(this, baseClassAddr);
      if (auto clsName = cls.getClassName())
        return *clsName;
      else
        return clsName.takeError();
    } else {
      uint64_t baseClassAddr = Cat.getAddress() + offsetof(category32_t, cls);
      ObjCClassRef cls(this, baseClassAddr);
      if (auto clsName = cls.getClassName())
        return *clsName;
      else
        return clsName.takeError();
    }
  } else if (isObjC1()) {
    auto NamePtr = getPointerValue32(Cat.getAddress() +
                                     offsetof(objc_category_t, class_name));
    if (!NamePtr)
      return NamePtr.takeError();
    return getString(*NamePtr);
  }

  return unsupportedObjCRuntimeError();
}

Expected<ObjCPropertyList>
ObjCMetaDataReader::materializePropertyList(ObjCCategory Data) const {
  if (isObjC2()) {
    ObjCPropertyList PropertyList;
    if (OwningBinary->is64Bit()) {
      auto BaseProperties = getPointerValue64(
          Data.getAddress() + offsetof(category64_t, instanceProperties));
      if (!BaseProperties)
        return BaseProperties.takeError();
      if (!*BaseProperties)
        return PropertyList;
      auto PropList = getData<objc_property_list64>(*BaseProperties);
      if (!PropList)
        return PropList.takeError();

      for (unsigned Idx = 0; Idx < PropList->count; ++Idx)
        PropertyList.push_back(
            ObjCProperty(this, *BaseProperties + sizeof(objc_property_list64) +
                                   Idx * sizeof(objc_property64)));
      return PropertyList;
    } else {
      auto BaseProperties = getPointerValue32(
          Data.getAddress() + offsetof(category32_t, instanceProperties));
      if (!BaseProperties)
        return BaseProperties.takeError();
      if (!*BaseProperties)
        return PropertyList;
      auto PropList = getData<objc_property_list32>(*BaseProperties);
      if (!PropList)
        return PropList.takeError();

      for (unsigned Idx = 0; Idx < PropList->count; ++Idx)
        PropertyList.push_back(
            ObjCProperty(this, *BaseProperties + sizeof(objc_property_list32) +
                                   Idx * sizeof(objc_property32)));
      return PropertyList;
    }
  } else if (isObjC1())
    return ObjCPropertyList();

  return unsupportedObjCRuntimeError();
}

Expected<ObjCMethodList>
ObjCMetaDataReader::materializeInstanceMethodList(ObjCCategory Data) const {
  if (isObjC2()) {
    ObjCMethodList MethodList;
    if (OwningBinary->is64Bit()) {
      auto BaseMethods = getPointerValue64(
          Data.getAddress() + offsetof(category64_t, instanceMethods));
      if (!BaseMethods)
        return BaseMethods.takeError();
      if (!*BaseMethods)
        return MethodList;
      auto MList = getData<method_list64_t>(*BaseMethods);
      if (!MList)
        return MList.takeError();

      bool RA = MList->entsize & METHOD_LIST_ENTSIZE_FLAG_RELATIVE;
      auto EntrySize = RA ? sizeof(method_rel_t) : sizeof(method64_t);
      for (unsigned Idx = 0; Idx < MList->count; ++Idx)
        MethodList.push_back(ObjCMethod(
            this, *BaseMethods + sizeof(method_list64_t) + Idx * EntrySize,
            RA));
      return MethodList;
    } else {
      auto BaseMethods = getPointerValue32(
          Data.getAddress() + offsetof(category32_t, instanceMethods));
      if (!BaseMethods)
        return BaseMethods.takeError();
      if (!*BaseMethods)
        return MethodList;
      auto MList = getData<method_list32_t>(*BaseMethods);
      if (!MList)
        return MList.takeError();

      bool RA = MList->entsize & METHOD_LIST_ENTSIZE_FLAG_RELATIVE;
      auto EntrySize = RA ? sizeof(method_rel_t) : sizeof(method32_t);
      for (unsigned Idx = 0; Idx < MList->count; ++Idx)
        MethodList.push_back(ObjCMethod(
            this, *BaseMethods + sizeof(method_list32_t) + Idx * EntrySize,
            RA));
      return MethodList;
    }
  } else if (isObjC1()) {
    uint64_t Methods =
        Data.getAddress() + offsetof(objc_category_t, instance_methods);
    return getObjC1MethodList(ObjCRef(Methods));
  }
  return unsupportedObjCRuntimeError();
}

Expected<ObjCMethodList>
ObjCMetaDataReader::materializeClassMethodList(ObjCCategory Data) const {
  if (isObjC2()) {
    ObjCMethodList MethodList;
    if (OwningBinary->is64Bit()) {
      auto BaseMethods = getPointerValue64(
          Data.getAddress() + offsetof(category64_t, classMethods));
      if (!BaseMethods)
        return BaseMethods.takeError();
      if (!*BaseMethods)
        return MethodList;
      auto MList = getData<method_list64_t>(*BaseMethods);
      if (!MList)
        return MList.takeError();

      bool RA = MList->entsize & METHOD_LIST_ENTSIZE_FLAG_RELATIVE;
      auto EntrySize = RA ? sizeof(method_rel_t) : sizeof(method64_t);
      for (unsigned Idx = 0; Idx < MList->count; ++Idx)
        MethodList.push_back(ObjCMethod(
            this, *BaseMethods + sizeof(method_list64_t) + Idx * EntrySize,
            RA));
      return MethodList;
    } else {
      auto BaseMethods = getPointerValue32(
          Data.getAddress() + offsetof(category32_t, classMethods));
      if (!BaseMethods)
        return BaseMethods.takeError();
      if (!*BaseMethods)
        return MethodList;
      auto MList = getData<method_list32_t>(*BaseMethods);
      if (!MList)
        return MList.takeError();

      bool RA = MList->entsize & METHOD_LIST_ENTSIZE_FLAG_RELATIVE;
      auto EntrySize = RA ? sizeof(method_rel_t) : sizeof(method32_t);
      for (unsigned Idx = 0; Idx < MList->count; ++Idx)
        MethodList.push_back(ObjCMethod(
            this, *BaseMethods + sizeof(method_list32_t) + Idx * EntrySize,
            RA));
      return MethodList;
    }
  } else if (isObjC1()) {
    uint64_t Methods =
        Data.getAddress() + offsetof(objc_category_t, class_methods);
    return getObjC1MethodList(ObjCRef(Methods));
  }
  return unsupportedObjCRuntimeError();
}

Expected<StringRef>
ObjCMetaDataReader::getObjCProtocolName(ObjCProtocol Data) const {
  if (isObjC2()) {
    if (OwningBinary->is64Bit()) {
      auto NamePtr =
          getPointerValue64(Data.getAddress() + offsetof(protocol64_t, name));
      if (!NamePtr)
        return NamePtr.takeError();
      return getString(*NamePtr);
    } else {
      auto NamePtr =
          getPointerValue32(Data.getAddress() + offsetof(protocol32_t, name));
      if (!NamePtr)
        return NamePtr.takeError();
      return getString(*NamePtr);
    }
  }
  return unsupportedObjCRuntimeError();
}

Expected<ObjCPropertyList>
ObjCMetaDataReader::materializePropertyList(ObjCProtocol Data) const {
  if (isObjC2()) {
    ObjCPropertyList PropertyList;
    if (OwningBinary->is64Bit()) {
      auto BaseProperties = getPointerValue64(
          Data.getAddress() + offsetof(protocol64_t, instanceProperties));
      if (!BaseProperties)
        return BaseProperties.takeError();
      if (!*BaseProperties)
        return PropertyList;
      auto PropList = getData<objc_property_list64>(*BaseProperties);
      if (!PropList)
        return PropList.takeError();

      for (unsigned Idx = 0; Idx < PropList->count; ++Idx)
        PropertyList.push_back(
            ObjCProperty(this, *BaseProperties + sizeof(objc_property_list64) +
                                   Idx * sizeof(objc_property64)));
      return PropertyList;
    } else {
      auto BaseProperties = getPointerValue32(
          Data.getAddress() + offsetof(protocol32_t, instanceProperties));
      if (!BaseProperties)
        return BaseProperties.takeError();
      if (!*BaseProperties)
        return PropertyList;
      auto PropList = getData<objc_property_list32>(*BaseProperties);
      if (!PropList)
        return PropList.takeError();

      for (unsigned Idx = 0; Idx < PropList->count; ++Idx)
        PropertyList.push_back(
            ObjCProperty(this, *BaseProperties + sizeof(objc_property_list32) +
                                   Idx * sizeof(objc_property32)));
      return PropertyList;
    }
  }
  return unsupportedObjCRuntimeError();
}

Expected<ObjCMethodList>
ObjCMetaDataReader::materializeInstanceMethodList(ObjCProtocol Data) const {
  if (isObjC2()) {
    ObjCMethodList MethodList;
    if (OwningBinary->is64Bit()) {
      auto BaseMethods = getPointerValue64(
          Data.getAddress() + offsetof(protocol64_t, instanceMethods));
      if (!BaseMethods)
        return BaseMethods.takeError();
      if (!*BaseMethods)
        return MethodList;
      auto MList = getData<method_list64_t>(*BaseMethods);
      if (!MList)
        return MList.takeError();
      bool RA = MList->entsize & METHOD_LIST_ENTSIZE_FLAG_RELATIVE;
      auto EntrySize = RA ? sizeof(method_rel_t) : sizeof(method64_t);
      for (unsigned Idx = 0; Idx < MList->count; ++Idx)
        MethodList.push_back(ObjCMethod(
            this, *BaseMethods + sizeof(method_list64_t) + Idx * EntrySize,
            RA));
      return MethodList;
    } else {
      auto BaseMethods = getPointerValue32(
          Data.getAddress() + offsetof(protocol32_t, instanceMethods));
      if (!BaseMethods)
        return BaseMethods.takeError();
      if (!*BaseMethods)
        return MethodList;
      auto MList = getData<method_list32_t>(*BaseMethods);
      if (!MList)
        return MList.takeError();
      bool RA = MList->entsize & METHOD_LIST_ENTSIZE_FLAG_RELATIVE;
      auto EntrySize = RA ? sizeof(method_rel_t) : sizeof(method32_t);
      for (unsigned Idx = 0; Idx < MList->count; ++Idx)
        MethodList.push_back(ObjCMethod(
            this, *BaseMethods + sizeof(method_list32_t) + Idx * EntrySize,
            RA));
      return MethodList;
    }
  }
  return unsupportedObjCRuntimeError();
}

Expected<ObjCMethodList>
ObjCMetaDataReader::materializeClassMethodList(ObjCProtocol Data) const {
  if (isObjC2()) {
    ObjCMethodList MethodList;
    if (OwningBinary->is64Bit()) {
      auto BaseMethods = getPointerValue64(
          Data.getAddress() + offsetof(protocol64_t, classMethods));
      if (!BaseMethods)
        return BaseMethods.takeError();
      if (!*BaseMethods)
        return MethodList;
      auto MList = getData<method_list64_t>(*BaseMethods);
      if (!MList)
        return MList.takeError();
      bool RA = MList->entsize & METHOD_LIST_ENTSIZE_FLAG_RELATIVE;
      auto EntrySize = RA ? sizeof(method_rel_t) : sizeof(method64_t);
      for (unsigned Idx = 0; Idx < MList->count; ++Idx)
        MethodList.push_back(ObjCMethod(
            this, *BaseMethods + sizeof(method_list64_t) + Idx * EntrySize,
            RA));
      return MethodList;
    } else {
      auto BaseMethods = getPointerValue32(
          Data.getAddress() + offsetof(protocol32_t, classMethods));
      if (!BaseMethods)
        return BaseMethods.takeError();
      if (!*BaseMethods)
        return MethodList;
      auto MList = getData<method_list32_t>(*BaseMethods);
      if (!MList)
        return MList.takeError();
      bool RA = MList->entsize & METHOD_LIST_ENTSIZE_FLAG_RELATIVE;
      auto EntrySize = RA ? sizeof(method_rel_t) : sizeof(method32_t);
      for (unsigned Idx = 0; Idx < MList->count; ++Idx)
        MethodList.push_back(ObjCMethod(
            this, *BaseMethods + sizeof(method_list32_t) + Idx * EntrySize,
            RA));
      return MethodList;
    }
  }
  return unsupportedObjCRuntimeError();
}

Expected<ObjCMethodList>
ObjCMetaDataReader::materializeOptionalInstanceMethodList(
    ObjCProtocol Data) const {
  if (isObjC2()) {
    ObjCMethodList MethodList;
    if (OwningBinary->is64Bit()) {
      auto BaseMethods = getPointerValue64(
          Data.getAddress() + offsetof(protocol64_t, optionalInstanceMethods));
      if (!BaseMethods)
        return BaseMethods.takeError();
      if (!*BaseMethods)
        return MethodList;
      auto MList = getData<method_list64_t>(*BaseMethods);
      if (!MList)
        return MList.takeError();
      bool RA = MList->entsize & METHOD_LIST_ENTSIZE_FLAG_RELATIVE;
      auto EntrySize = RA ? sizeof(method_rel_t) : sizeof(method64_t);
      for (unsigned Idx = 0; Idx < MList->count; ++Idx)
        MethodList.push_back(ObjCMethod(
            this, *BaseMethods + sizeof(method_list64_t) + Idx * EntrySize,
            RA));
      return MethodList;
    } else {
      auto BaseMethods = getPointerValue32(
          Data.getAddress() + offsetof(protocol32_t, optionalInstanceMethods));
      if (!BaseMethods)
        return BaseMethods.takeError();
      if (!*BaseMethods)
        return MethodList;
      auto MList = getData<method_list32_t>(*BaseMethods);
      if (!MList)
        return MList.takeError();
      bool RA = MList->entsize & METHOD_LIST_ENTSIZE_FLAG_RELATIVE;
      auto EntrySize = RA ? sizeof(method_rel_t) : sizeof(method32_t);
      for (unsigned Idx = 0; Idx < MList->count; ++Idx)
        MethodList.push_back(ObjCMethod(
            this, *BaseMethods + sizeof(method_list32_t) + Idx * EntrySize,
            RA));
      return MethodList;
    }
  }
  return unsupportedObjCRuntimeError();
}

Expected<ObjCMethodList> ObjCMetaDataReader::materializeOptionalClassMethodList(
    ObjCProtocol Data) const {
  if (isObjC2()) {
    ObjCMethodList MethodList;
    if (OwningBinary->is64Bit()) {
      auto BaseMethods = getPointerValue64(
          Data.getAddress() + offsetof(protocol64_t, optionalClassMethods));
      if (!BaseMethods)
        return BaseMethods.takeError();
      if (!*BaseMethods)
        return MethodList;
      auto MList = getData<method_list64_t>(*BaseMethods);
      if (!MList)
        return MList.takeError();
      bool RA = MList->entsize & METHOD_LIST_ENTSIZE_FLAG_RELATIVE;
      auto EntrySize = RA ? sizeof(method_rel_t) : sizeof(method64_t);
      for (unsigned Idx = 0; Idx < MList->count; ++Idx)
        MethodList.push_back(ObjCMethod(
            this, *BaseMethods + sizeof(method_list64_t) + Idx * EntrySize,
            RA));
      return MethodList;
    } else {
      auto BaseMethods = getPointerValue32(
          Data.getAddress() + offsetof(protocol32_t, optionalClassMethods));
      if (!BaseMethods)
        return BaseMethods.takeError();
      if (!*BaseMethods)
        return MethodList;
      auto MList = getData<method_list32_t>(*BaseMethods);
      if (!MList)
        return MList.takeError();
      bool RA = MList->entsize & METHOD_LIST_ENTSIZE_FLAG_RELATIVE;
      auto EntrySize = RA ? sizeof(method_rel_t) : sizeof(method32_t);
      for (unsigned Idx = 0; Idx < MList->count; ++Idx)
        MethodList.push_back(ObjCMethod(
            this, *BaseMethods + sizeof(method_list32_t) + Idx * EntrySize,
            RA));
      return MethodList;
    }
  }
  return unsupportedObjCRuntimeError();
}

Expected<StringRef> ObjCMetaDataReader::getMethodName(ObjCMethod Data) const {
  if (isObjC1() || isObjC2()) {
    if (OwningBinary->is64Bit()) {
      if (Data.isRelativeAddress()) {
        uint64_t MethodAddress =
            Data.getAddress() + offsetof(method_rel_t, name);
        auto MethodOffset = getData<int32_t>(MethodAddress);
        if (!MethodOffset)
          return MethodOffset.takeError();
        auto MethodName = getPointerValue64(MethodAddress + *MethodOffset);
        if (!MethodName)
          return MethodName.takeError();
        return getString(*MethodName, /*allow empty*/ true);
      } else {
        auto MethodName =
            getPointerValue64(Data.getAddress() + offsetof(method64_t, name));
        if (!MethodName)
          return MethodName.takeError();
        return getString(*MethodName);
      }
    } else {
      if (Data.isRelativeAddress()) {
        uint64_t MethodAddress =
            Data.getAddress() + offsetof(method_rel_t, name);
        auto MethodOffset = getData<int32_t>(MethodAddress);
        if (!MethodOffset)
          return MethodOffset.takeError();
        auto MethodName = getPointerValue32(MethodAddress + *MethodOffset);
        if (!MethodName)
          return MethodName.takeError();
        return getString(*MethodName, /*allow empty*/ true);
      } else {
        auto MethodName =
            getPointerValue32(Data.getAddress() + offsetof(method32_t, name));
        if (!MethodName)
          return MethodName.takeError();
        return getString(*MethodName);
      }
    }
  }
  return unsupportedObjCRuntimeError();
}

Expected<StringRef> ObjCMetaDataReader::getMethodType(ObjCMethod Data) const {
  if (isObjC1() || isObjC2()) {
    if (OwningBinary->is64Bit()) {
      if (Data.isRelativeAddress()) {
        uint64_t TypeAddress =
            Data.getAddress() + offsetof(method_rel_t, types);
        auto TypeOffset = getData<int32_t>(TypeAddress);
        if (!TypeOffset)
          return TypeOffset.takeError();
        // zero offset represents a nullptr.
        if (!*TypeOffset)
          return StringRef();
        return getString(TypeAddress + *TypeOffset);
      } else {
        auto MethodTypes =
            getPointerValue64(Data.getAddress() + offsetof(method64_t, types));
        if (!MethodTypes)
          return MethodTypes.takeError();
        return getString(*MethodTypes);
      }
    } else {
      if (Data.isRelativeAddress()) {
        uint64_t TypeAddress =
            Data.getAddress() + offsetof(method_rel_t, types);
        auto TypeOffset = getData<int32_t>(TypeAddress);
        if (!TypeOffset)
          return TypeOffset.takeError();
        // zero offset represents a nullptr.
        if (!*TypeOffset)
          return StringRef();
        return getString(TypeAddress + *TypeOffset);
      } else {
        auto MethodTypes =
            getPointerValue32(Data.getAddress() + offsetof(method32_t, types));
        if (!MethodTypes)
          return MethodTypes.takeError();
        return getString(*MethodTypes);
      }
    }
  }
  return unsupportedObjCRuntimeError();
}

Expected<ObjCSelectorList> ObjCMetaDataReader::referencedSelectors() const {
  ObjCSelectorList SelectorList;
  SectionRef SelSect = getSection("__DATA", "__objc_selrefs");
  // Return empty if the section doesn't exists.
  if (SelSect == SectionRef())
    return SelectorList;

  if (SelSect.getSize() % OwningBinary->getBytesInAddress() != 0)
    return make_error<StringError>(
        "objc class section size is not a multiple of pointer size",
        object_error::parse_failed);

  for (unsigned i = 0; i < SelSect.getSize();
       i += OwningBinary->getBytesInAddress())
    SelectorList.push_back(ObjCSelectorRef(this, SelSect.getAddress() + i));

  return SelectorList;
}

Expected<StringRef>
ObjCMetaDataReader::getObjCSelectorName(ObjCSelectorRef Ref) const {
  if (isObjC2()) {
    if (OwningBinary->is64Bit()) {
      auto Sel = getPointerValue64(Ref.getAddress());
      if (!Sel)
        return Sel.takeError();
      return getString(*Sel);
    } else {
      auto Sel = getPointerValue32(Ref.getAddress());
      if (!Sel)
        return Sel.takeError();
      return getString(*Sel);
    }
  }
  return unsupportedObjCRuntimeError();
}

void ObjCMetaDataReader::getAllPotentiallyDefinedSelectors(
    StringSet<> &Set) const {
  // Clear the set.
  Set.clear();
  SectionRef ObjCConstSect = getSection("__DATA_CONST", "__objc_const");
  if (ObjCConstSect == SectionRef())
    ObjCConstSect = getSection("__DATA", "__objc_const");
  SectionRef MethodNameSect = getSection("__TEXT", "__objc_methname");
  if (ObjCConstSect == SectionRef() || MethodNameSect == SectionRef())
    return;

  auto ignoreError = [](Error &&E) {
    handleAllErrors(std::move(E), [&](const ErrorInfoBase &EI) {
      return; // ignore.
    });
  };

  if (auto sels = referencedSelectors()) {
    StringSet<> refSels;
    for (auto sel : *sels) {
      if (auto selName = sel.getSelector())
        refSels.insert(*selName);
      else
        ignoreError(selName.takeError());
    }
    if (OwningBinary->is64Bit()) {
      for (uint64_t A = ObjCConstSect.getAddress(),
                    E = ObjCConstSect.getAddress() + ObjCConstSect.getSize();
           A < E; A += OwningBinary->getBytesInAddress()) {
        auto methodName = getPointerValue64(A);
        if (!methodName) {
          ignoreError(methodName.takeError());
          continue;
        }
        if (*methodName >= MethodNameSect.getAddress() &&
            *methodName <
                MethodNameSect.getAddress() + MethodNameSect.getSize()) {
          if (auto name = getString(*methodName)) {
            if (refSels.find(*name) != refSels.end())
              Set.insert(*name);
          } else
            ignoreError(name.takeError());
        }
      }
    } else {
      for (uint64_t A = ObjCConstSect.getAddress(),
                    E = ObjCConstSect.getAddress() + ObjCConstSect.getSize();
           A < E; A += OwningBinary->getBytesInAddress()) {
        auto methodName = getPointerValue32(A);
        if (!methodName) {
          ignoreError(methodName.takeError());
          continue;
        }
        if (*methodName >= MethodNameSect.getAddress() &&
            *methodName <
                MethodNameSect.getAddress() + MethodNameSect.getSize()) {
          if (auto name = getString(*methodName)) {
            if (refSels.find(*name) != refSels.end())
              Set.insert(*name);
          } else
            ignoreError(name.takeError());
        }
      }
    }
  } else
    ignoreError(sels.takeError());
}

Expected<StringRef> ObjCMetaDataReader::getSwiftVersion() const {
  SectionRef Sect = getSection("__OBJC2", "__module_info");
  if (Sect == SectionRef())
    Sect = getSection("__DATA_CONST", "__objc_imageinfo");
  if (Sect == SectionRef())
    Sect = getSection("__DATA", "__objc_imageinfo");
  if (Sect == SectionRef())
    return "";
  auto SectContents = Sect.getContents();
  if (!SectContents)
    return SectContents.takeError();
  imageInfo_t info;
  if (SectContents->size() < sizeof(info))
    return make_error<StringError>("ObjC Image Info section is too small",
                                   object_error::parse_failed);
  memcpy(&info, SectContents->data(), sizeof(info));
  if (OwningBinary->isLittleEndian() != sys::IsLittleEndianHost)
    swapStruct(info);

  return convertSwiftVersion((info.flags >> 8) & 0xff);
}

const SectionRef ObjCMetaDataReader::getSection(const char *segname,
                                                const char *sectname) const {
  for (const SectionRef &Section : OwningBinary->sections()) {
    auto MaybeName = Section.getName();
    if (!MaybeName)
      continue;
    StringRef SectName = *MaybeName;
    DataRefImpl Ref = Section.getRawDataRefImpl();
    StringRef SegName = OwningBinary->getSectionFinalSegmentName(Ref);
    if (SegName == segname && SectName == sectname)
      return Section;
  }
  return SectionRef();
}

Expected<object::SectionRef>
ObjCMetaDataReader::getSectionFromAddress(uint64_t Address) const {
  for (const SectionRef &Section : OwningBinary->sections()) {
    uint64_t SectAddress = Section.getAddress();
    uint64_t SectSize = Section.getSize();
    if (SectSize == 0)
      continue;
    if (Address >= SectAddress && Address < SectAddress + SectSize)
      return Section;
  }
  return make_error<StringError>("requested address not in section",
                                 object_error::parse_failed);
}

bool ObjCMetaDataReader::isAddressEncrypted(uint64_t Address) const {
  // Find encrypted range.
  if (OwningBinary->is64Bit()) {
    uint64_t fileOffset = 0;
    for (const auto &LC : OwningBinary->load_commands()) {
      if (LC.C.cmd == MachO::LC_SEGMENT_64) {
        MachO::segment_command_64 cmd =
            OwningBinary->getSegment64LoadCommand(LC);
        if (Address > cmd.vmaddr && Address < cmd.vmaddr + cmd.vmsize)
          fileOffset = Address - cmd.vmaddr + cmd.fileoff;
      }
    }
    for (const auto &LC : OwningBinary->load_commands()) {
      if (LC.C.cmd == MachO::LC_ENCRYPTION_INFO_64) {
        MachO::encryption_info_command_64 cmd =
            OwningBinary->getEncryptionInfoCommand64(LC);
        if (cmd.cryptid == 0)
          continue;
        if (fileOffset > cmd.cryptoff &&
            fileOffset < cmd.cryptoff + cmd.cryptsize)
          return true;
      }
    }
  } else {
    uint64_t fileOffset = 0;
    for (const auto &LC : OwningBinary->load_commands()) {
      if (LC.C.cmd == MachO::LC_SEGMENT) {
        MachO::segment_command cmd = OwningBinary->getSegmentLoadCommand(LC);
        if (Address > cmd.vmaddr && Address < cmd.vmaddr + cmd.vmsize)
          fileOffset = Address - cmd.vmaddr + cmd.fileoff;
      }
    }
    for (const auto &LC : OwningBinary->load_commands()) {
      if (LC.C.cmd == MachO::LC_ENCRYPTION_INFO) {
        MachO::encryption_info_command cmd =
            OwningBinary->getEncryptionInfoCommand(LC);
        if (cmd.cryptid == 0)
          continue;
        if (fileOffset > cmd.cryptoff &&
            fileOffset < cmd.cryptoff + cmd.cryptsize)
          return true;
      }
    }
  }
  return false;
}

StringRef ObjCMetaDataReader::getSymbol(uint64_t VMAddr) const {
  auto E = VMAddrToSymbolMap.find(VMAddr);
  if (E == VMAddrToSymbolMap.end())
    return StringRef();

  return E->second;
}

template <typename T>
Expected<T> ObjCMetaDataReader::getData(uint64_t VMAddr) const {
  if (isAddressEncrypted(VMAddr))
    return make_error<StringError>("vmaddr is encrypted",
                                   object_error::parse_failed);
  for (const SectionRef &Section : OwningBinary->sections()) {
    uint64_t SectAddress = Section.getAddress();
    uint64_t SectSize = Section.getSize();
    if (SectSize == 0)
      continue;
    if (VMAddr >= SectAddress && VMAddr < SectAddress + SectSize) {
      uint64_t Offset = VMAddr - SectAddress;
      auto SectContents = Section.getContents();
      if (!SectContents)
        return SectContents.takeError();
      if (Offset + sizeof(T) > SectSize)
        return make_error<StringError>(
            "Data extends pass the end of the section",
            object_error::parse_failed);
      T Data;
      memcpy(&Data, SectContents->data() + Offset, sizeof(T));
      if (OwningBinary->isLittleEndian() != sys::IsLittleEndianHost)
        swapStruct(Data);
      return Data;
    }
  }
  return make_error<StringError>("requested address out of bound",
                                 object_error::parse_failed);
}

Expected<StringRef> ObjCMetaDataReader::getString(uint64_t VMAddr,
                                                  bool AllowEmpty) const {
  if (isAddressEncrypted(VMAddr))
    return "#EncryptedString#";

  for (const SectionRef &Section : OwningBinary->sections()) {
    uint64_t SectAddress = Section.getAddress();
    uint64_t SectSize = Section.getSize();
    if (SectSize == 0)
      continue;
    if (VMAddr >= SectAddress && VMAddr < SectAddress + SectSize) {
      uint64_t Offset = VMAddr - SectAddress;
      auto SectContents = Section.getContents();
      if (!SectContents)
        return SectContents.takeError();
      StringRef Target = StringRef(SectContents->data() + Offset);
      if (Target.size() + Offset >= SectSize)
        return make_error<StringError>(
            "Data extends pass the end of the section",
            object_error::parse_failed);
      if (Target.empty() && !AllowEmpty)
        return make_error<StringError>(
            "Expect to read a none zero length string",
            object_error::parse_failed);

      return Target;
    }
  }
  return make_error<StringError>("string out of bound",
                                 object_error::parse_failed);
}

Expected<uint64_t>
ObjCMetaDataReader::getPointerValue64(uint64_t VMAddr) const {
  // If there are rebase at the vmaddr, return cached pointer value.
  auto E = VMAddrPointToValueMap.find(VMAddr);
  if (E != VMAddrPointToValueMap.end())
    return E->second;

  // Otherwise, just load the content from the address.
  return getData<uint64_t>(VMAddr);
}

Expected<uint32_t>
ObjCMetaDataReader::getPointerValue32(uint64_t VMAddr) const {
  auto E = VMAddrPointToValueMap.find(VMAddr);
  if (E != VMAddrPointToValueMap.end())
    return E->second;

  return getData<uint32_t>(VMAddr);
}

Expected<ObjCRef> ObjCMetaDataReader::getObjC2ClassRO64(ObjCClass Data) const {
  assert(isObjC2() && OwningBinary->is64Bit() &&
         "Only support 64 bit Obj2 runtime");
  if (auto class_ro =
          getPointerValue64(Data.getAddress() + offsetof(class64_t, data))) {
    return *class_ro & ~0x7;
  } else
    return class_ro.takeError();
}

Expected<ObjCRef> ObjCMetaDataReader::getObjC2ClassRO32(ObjCClass Data) const {
  assert(isObjC2() && !OwningBinary->is64Bit() &&
         "Only support 32 bit Obj2 runtime");
  if (auto class_ro =
          getPointerValue32(Data.getAddress() + offsetof(class32_t, data))) {
    return *class_ro & ~0x3;
  } else
    return class_ro.takeError();
}

Expected<ObjCClass>
ObjCMetaDataReader::getObjC2MetaClass64(ObjCClass Data) const {
  assert(isObjC2() && OwningBinary->is64Bit() &&
         "Only support 64 bit Obj2 runtime");
  if (auto isa =
          getPointerValue64(Data.getAddress() + offsetof(class64_t, isa))) {
    if (*isa) {
      return ObjCClass(this, *isa);
    } else
      return ObjCClass(this);
  } else
    return isa.takeError();
}

Expected<ObjCClass>
ObjCMetaDataReader::getObjC2MetaClass32(ObjCClass Data) const {
  assert(isObjC2() && !OwningBinary->is64Bit() &&
         "Only support 32 bit Obj2 runtime");
  if (auto isa =
          getPointerValue32(Data.getAddress() + offsetof(class32_t, isa))) {
    if (*isa) {
      return ObjCClass(this, *isa);
    } else
      return ObjCClass(this);
  } else
    return isa.takeError();
}

Expected<ObjCRefList> ObjCMetaDataReader::getObjC1Modules() const {
  ObjCRefList Modules;
  auto ModuleInfo = getSection("__OBJC", "__module_info");
  if (ModuleInfo == SectionRef())
    return Modules;

  if (ModuleInfo.getSize() % sizeof(objc_module_t) != 0)
    return make_error<StringError>(
        "__module_info is not a multiple of objc_module_t size",
        object_error::parse_failed);

  for (unsigned i = 0; i < ModuleInfo.getSize(); i += sizeof(objc_module_t))
    Modules.push_back(ModuleInfo.getAddress() + i);

  return Modules;
}

Expected<ObjCRef> ObjCMetaDataReader::getObjC1Symtab(ObjCRef ObjCModule) const {
  if (auto sym =
          getPointerValue32(ObjCModule + offsetof(objc_module_t, symtab)))
    return ObjCRef(*sym);
  else
    return sym.takeError();
}

Expected<ObjCClassList>
ObjCMetaDataReader::getObjC1ClassesFromSymtab(ObjCRef Symtab) const {
  ObjCClassList Classes;
  auto def_start = Symtab + sizeof(objc_symtab_t);
  auto cls_count =
      getData<uint16_t>(Symtab + offsetof(objc_symtab_t, cls_def_cnt));
  if (!cls_count)
    return cls_count.takeError();

  auto section = getSectionFromAddress(def_start);
  if (section) {
    if ((def_start + *cls_count) > (section->getAddress() + section->getSize()))
      return make_error<StringError>("Symtab extends out of the range",
                                     object_error::parse_failed);
  } else
    return section.takeError();

  for (unsigned i = 0; i < *cls_count; ++i)
    Classes.push_back(ObjCClassRef(this, def_start + i * sizeof(uint32_t)));

  return Classes;
}

Expected<ObjCCategoryList>
ObjCMetaDataReader::getObjC1CategoriesFromSymtab(ObjCRef Symtab) const {
  ObjCCategoryList Categories;
  auto cls_count =
      getData<uint16_t>(Symtab + offsetof(objc_symtab_t, cls_def_cnt));
  if (!cls_count)
    return cls_count.takeError();

  auto def_start =
      Symtab + sizeof(objc_symtab_t) + *cls_count * sizeof(uint32_t);

  auto cat_count =
      getData<uint16_t>(Symtab + offsetof(objc_symtab_t, cat_def_cnt));
  if (!cat_count)
    return cat_count.takeError();

  auto section = getSectionFromAddress(def_start);
  if (section) {
    if ((def_start + *cat_count) > (section->getAddress() + section->getSize()))
      return make_error<StringError>("Symtab extends out of the range",
                                     object_error::parse_failed);
  } else
    return section.takeError();

  for (unsigned i = 0; i < *cat_count; ++i)
    Categories.push_back(
        ObjCCategoryRef(this, def_start + i * sizeof(uint32_t)));

  return Categories;
}

Expected<ObjCMethodList>
ObjCMetaDataReader::getObjC1MethodList(ObjCRef Methods) const {
  auto ML = getPointerValue32(Methods);
  if (!ML)
    return ML.takeError();
  if (!*ML)
    return ObjCMethodList();

  auto L = getData<objc_method_list_t>(*ML);
  if (!L)
    return L.takeError();

  ObjCMethodList MethodList;
  uint64_t start = *ML + sizeof(objc_method_list_t);

  auto section = getSectionFromAddress(start);
  if (section) {
    if ((start + L->method_count) >
        (section->getAddress() + section->getSize()))
      return make_error<StringError>("Method list extends out of the range",
                                     object_error::parse_failed);
  } else
    return section.takeError();

  for (int32_t i = 0; i < L->method_count; ++i)
    MethodList.push_back(ObjCMethod(this, start + i * sizeof(objc_method_t),
                                    /*relative address=*/false));

  return MethodList;
}
