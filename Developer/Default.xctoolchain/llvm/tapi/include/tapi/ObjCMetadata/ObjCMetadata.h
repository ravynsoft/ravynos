//===- ObjCMetadata.h - Objective C Metadata Interface ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares ObjCMetadata Interfaces.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_OBJCMETADATA_OBJCMETADATA_H
#define LLVM_OBJCMETADATA_OBJCMETADATA_H

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Object/MachO.h>
#include <llvm/Support/Error.h>
#include <unordered_map>
#include <utility>

namespace llvm {

typedef uint64_t ObjCRef;

class ObjCMetaDataReader;
class ObjCClassRef;
class ObjCProtocolRef;
class ObjCCategoryRef;
class ObjCMethod;
class ObjCProperty;
class ObjCSelectorRef;

typedef SmallVector<ObjCRef, 2> ObjCRefList;
typedef SmallVector<ObjCMethod, 2> ObjCMethodList;
typedef SmallVector<ObjCProperty, 2> ObjCPropertyList;
typedef SmallVector<ObjCClassRef, 2> ObjCClassList;
typedef SmallVector<ObjCCategoryRef, 2> ObjCCategoryList;
typedef SmallVector<ObjCProtocolRef, 2> ObjCProtocolList;
typedef SmallVector<ObjCSelectorRef, 2> ObjCSelectorList;

class ObjCInfoBase {
public:
  ObjCInfoBase(const ObjCMetaDataReader *Reader)
      : Data(), MetadataReader(Reader) {}
  ObjCInfoBase(const ObjCMetaDataReader *Reader, ObjCRef Data)
      : Data(Data), MetadataReader(Reader) {}

  bool operator==(const ObjCInfoBase &Other) const {
    return Data == Other.Data;
  }

  // Returns the VMAddr of the struct.
  ObjCRef &getAddress() { return Data; }

protected:
  ObjCRef Data;
  const ObjCMetaDataReader *MetadataReader;
};

class ObjCProperty : public ObjCInfoBase {
public:
  ObjCProperty(const ObjCMetaDataReader *Reader) : ObjCInfoBase(Reader) {}
  ObjCProperty(const ObjCMetaDataReader *Reader, ObjCRef Data)
      : ObjCInfoBase(Reader, Data) {}

  Expected<StringRef> getName() const;
  Expected<StringRef> getAttribute() const;
  Expected<bool> isDynamic() const;

  // Query for setter and getter.
  // Return empty string if doesn't exists.
  Expected<std::string> getGetter() const;
  Expected<std::string> getSetter() const;
};

class ObjCMethod : public ObjCInfoBase {
public:
  ObjCMethod(const ObjCMetaDataReader *Reader)
      : ObjCInfoBase(Reader), UseRelativeAddress(false) {}
  ObjCMethod(const ObjCMetaDataReader *Reader, ObjCRef Data,
             bool RelativeAddress = false)
      : ObjCInfoBase(Reader, Data), UseRelativeAddress(RelativeAddress) {}

  Expected<StringRef> getName() const;
  Expected<StringRef> getType() const;

  bool isRelativeAddress() const { return UseRelativeAddress; }

private:
  bool UseRelativeAddress;
};

// Interface to read the ObjCClass.
class ObjCClass : public ObjCInfoBase {
public:
  ObjCClass(const ObjCMetaDataReader *Reader) : ObjCInfoBase(Reader) {}
  ObjCClass(const ObjCMetaDataReader *Reader, ObjCRef Data)
      : ObjCInfoBase(Reader, Data) {}

  Expected<StringRef> getName() const;
  Expected<StringRef> getSuperClassName() const;

  Expected<bool> isSwift() const;
  Expected<bool> isMetaClass() const;

  Expected<ObjCPropertyList> properties() const;
  Expected<ObjCPropertyList> classProperties() const;
  Expected<ObjCMethodList> instanceMethods() const;
  Expected<ObjCMethodList> classMethods() const;
};

class ObjCCategory : public ObjCInfoBase {
public:
  ObjCCategory(const ObjCMetaDataReader *Reader) : ObjCInfoBase(Reader) {}
  ObjCCategory(const ObjCMetaDataReader *Reader, ObjCRef Data)
      : ObjCInfoBase(Reader, Data) {}

  Expected<StringRef> getName() const;
  Expected<StringRef> getBaseClassName() const;

  Expected<ObjCPropertyList> properties() const;
  Expected<ObjCMethodList> instanceMethods() const;
  Expected<ObjCMethodList> classMethods() const;
};

class ObjCProtocol : public ObjCInfoBase {
public:
  ObjCProtocol(const ObjCMetaDataReader *Reader) : ObjCInfoBase(Reader) {}
  ObjCProtocol(const ObjCMetaDataReader *Reader, ObjCRef Data)
      : ObjCInfoBase(Reader, Data) {}

  Expected<StringRef> getName() const;

  Expected<ObjCPropertyList> properties() const;
  Expected<ObjCMethodList> instanceMethods() const;
  Expected<ObjCMethodList> classMethods() const;
  Expected<ObjCMethodList> optionalInstanceMethods() const;
  Expected<ObjCMethodList> optionalClassMethods() const;
};

class ObjCClassRef : public ObjCInfoBase {
public:
  ObjCClassRef(const ObjCMetaDataReader *Reader) : ObjCInfoBase(Reader) {}
  ObjCClassRef(const ObjCMetaDataReader *Reader, ObjCRef Data)
      : ObjCInfoBase(Reader, Data) {}

  Expected<StringRef> getClassName() const;
  bool isExternal() const;
  Expected<ObjCClass> getObjCClass() const;
  Expected<ObjCClass> operator*() const { return getObjCClass(); }
};

class ObjCCategoryRef : public ObjCInfoBase {
public:
  ObjCCategoryRef(const ObjCMetaDataReader *Reader) : ObjCInfoBase(Reader) {}
  ObjCCategoryRef(const ObjCMetaDataReader *Reader, ObjCRef Data)
      : ObjCInfoBase(Reader, Data) {}

  Expected<ObjCCategory> getObjCCategory() const;
  Expected<ObjCCategory> operator*() const { return getObjCCategory(); }
};

class ObjCProtocolRef : public ObjCInfoBase {
public:
  ObjCProtocolRef(const ObjCMetaDataReader *Reader) : ObjCInfoBase(Reader) {}
  ObjCProtocolRef(const ObjCMetaDataReader *Reader, ObjCRef Data)
      : ObjCInfoBase(Reader, Data) {}

  Expected<ObjCProtocol> getObjCProtocol() const;
  Expected<ObjCProtocol> operator*() const { return getObjCProtocol(); }
};

class ObjCSelectorRef : public ObjCInfoBase {
public:
  ObjCSelectorRef(const ObjCMetaDataReader *Reader) : ObjCInfoBase(Reader) {}
  ObjCSelectorRef(const ObjCMetaDataReader *Reader, ObjCRef Data)
      : ObjCInfoBase(Reader, Data) {}

  Expected<StringRef> getSelector() const;
};

// ObjC Metadata Reader.
class ObjCMetaDataReader {
public:
  ObjCMetaDataReader(object::MachOObjectFile *Binary, Error &Err);

  object::MachOObjectFile *getMachOObject() {
    return cast<object::MachOObjectFile>(OwningBinary);
  }

  bool isObjC1() const { return ObjCVersion == ObjC1; }
  bool isObjC2() const { return ObjCVersion == ObjC2; }
  Expected<StringRef> getSwiftVersion() const;

  const object::MachOObjectFile *getOwningBinary() const {
    return OwningBinary;
  }

  // Overrides ObjC Class Ref Implementation.
  Expected<ObjCClassList> classes() const;
  Expected<ObjCClass> getObjCClassFromRef(ObjCClassRef Ref) const;
  bool isObjCClassExternal(ObjCClassRef Ref) const;

  // Overrides ObjC Category Ref Implementation.
  Expected<ObjCCategoryList> categories() const;
  Expected<ObjCCategory> getObjCCategoryFromRef(ObjCCategoryRef Ref) const;

  // Overrides ObjC Protocol Ref Implementation.
  Expected<ObjCProtocolList> protocols() const;
  Expected<ObjCProtocol> getObjCProtocolFromRef(ObjCProtocolRef Ref) const;

  // Overrides ObjC Class Implementation.
  Expected<StringRef> getObjCClassName(ObjCClass ClassRef) const;
  Expected<StringRef> getObjCSuperClassName(ObjCClass Data) const;
  Expected<bool> isObjCClassSwift(ObjCClass Data) const;
  Expected<bool> isObjCClassMetaClass(ObjCClass Data) const;
  Expected<ObjCPropertyList> materializePropertyList(ObjCClass Data) const;
  Expected<ObjCPropertyList> materializeClassPropertyList(ObjCClass Data) const;
  Expected<ObjCMethodList> materializeInstanceMethodList(ObjCClass Data) const;
  Expected<ObjCMethodList> materializeClassMethodList(ObjCClass Data) const;

  // Overrides ObjC Category Implementation.
  Expected<StringRef> getObjCCategoryName(ObjCCategory Data) const;
  Expected<StringRef> getObjCCategoryBaseClassName(ObjCCategory Data) const;
  Expected<ObjCPropertyList> materializePropertyList(ObjCCategory Data) const;
  Expected<ObjCMethodList>
  materializeInstanceMethodList(ObjCCategory Data) const;
  Expected<ObjCMethodList> materializeClassMethodList(ObjCCategory Data) const;

  // Overrides ObjC Protocol Implementation.
  Expected<StringRef> getObjCProtocolName(ObjCProtocol Data) const;
  Expected<ObjCPropertyList> materializePropertyList(ObjCProtocol Data) const;
  Expected<ObjCMethodList>
  materializeInstanceMethodList(ObjCProtocol Data) const;
  Expected<ObjCMethodList> materializeClassMethodList(ObjCProtocol Data) const;
  Expected<ObjCMethodList>
  materializeOptionalInstanceMethodList(ObjCProtocol Data) const;
  Expected<ObjCMethodList>
  materializeOptionalClassMethodList(ObjCProtocol Data) const;

  // Override ObjC Property Implementation.
  Expected<StringRef> getPropertyName(ObjCProperty Data) const;
  Expected<StringRef> getPropertyAttribute(ObjCProperty Data) const;

  // Override ObjC Method Implementation.
  Expected<StringRef> getMethodName(ObjCMethod Data) const;
  Expected<StringRef> getMethodType(ObjCMethod Data) const;

  // Override ObjC Selector Ref Implementation.
  Expected<ObjCSelectorList> referencedSelectors() const;
  Expected<StringRef> getObjCSelectorName(ObjCSelectorRef Ref) const;

  // Override Potentially defined selectors Implemetation.
  void getAllPotentiallyDefinedSelectors(StringSet<> &Set) const;

  // Other.
  StringRef getSymbolNameFromRef(ObjCRef Ref) const;
  StringRef guessClassNameBasedOnSymbol(StringRef Sym) const;

  // Helper classes
  Expected<StringRef> convertSwiftVersion(unsigned raw) const;
  const object::SectionRef getSection(const char *segname,
                                      const char *sectname) const;
  Expected<object::SectionRef> getSectionFromAddress(uint64_t Address) const;
  bool isAddressEncrypted(uint64_t Address) const;

  // ObjC2 helper classes
  Expected<ObjCRef> getObjC2ClassRO64(ObjCClass Data) const;
  Expected<ObjCRef> getObjC2ClassRO32(ObjCClass Data) const;
  Expected<ObjCClass> getObjC2MetaClass64(ObjCClass Data) const;
  Expected<ObjCClass> getObjC2MetaClass32(ObjCClass Data) const;
  // ObjC1 helper classes
  Expected<ObjCRefList> getObjC1Modules() const;
  Expected<ObjCRef> getObjC1Symtab(ObjCRef ObjCModule) const;
  Expected<ObjCClassList> getObjC1ClassesFromSymtab(ObjCRef Symtab) const;
  Expected<ObjCCategoryList> getObjC1CategoriesFromSymtab(ObjCRef Symtab) const;
  Expected<ObjCMethodList> getObjC1MethodList(ObjCRef Methods) const;

  // Get symbol from VMAddr. If there is no symbol at vmaddr, return empty
  // StringRef.
  StringRef getSymbol(uint64_t VMAddr) const;
  // Get data from VMAddr. Caller needs to make sure there are no
  // relocations/rebases/binds in range.
  template <typename T> Expected<T> getData(uint64_t VMAddr) const;
  // Get string from VMAddr. Caller needs to make sure there are no
  // relocations/rebases/binds in range.
  Expected<StringRef> getString(uint64_t VMAddr, bool AllowEmpty = false) const;
  // Get pointer value from VMAddr.
  Expected<uint64_t> getPointerValue64(uint64_t VMAddr) const;
  Expected<uint32_t> getPointerValue32(uint64_t VMAddr) const;

private:
  object::MachOObjectFile *OwningBinary;
  enum ObjCRuntimeVersion { Unknown = 0, ObjC1 = 1, ObjC2 = 2 };
  ObjCRuntimeVersion ObjCVersion;

  // Cache all RawPointer has a name associated with it.
  std::unordered_map<uint64_t, StringRef> VMAddrToSymbolMap;
  // Cache all RawPointer that has a VM Addr associcated with it.
  std::unordered_map<uint64_t, uint64_t> VMAddrPointToValueMap;
};

}

#endif
