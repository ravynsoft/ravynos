set( LLVM_LINK_COMPONENTS
  Object
  BinaryFormat
  TargetParser
  TextAPI
  DebugInfoDWARF
  )

add_tapi_library(tapiCore
  API.cpp
  API2SymbolConverter.cpp
  APIJSONSerializer.cpp
  APIPrinter.cpp
  APIVisitor.cpp
  AvailabilityInfo.cpp
  ClangDiagnostics.cpp
  Demangler.cpp
  FakeSymbols.cpp
  FileListReader.cpp
  FileManager.cpp
  FileSystem.cpp
  Framework.cpp
  HeaderFile.cpp
  InterfaceFileManager.cpp
  JSONReaderWriter.cpp
  MachODylibReader.cpp
  MachOReader.cpp
  Path.cpp
  ReexportFileWriter.cpp
  Registry.cpp
  SymbolVerifier.cpp
  Utils.cpp
  YAMLReaderWriter.cpp

  LINK_LIBS
  clangBasic
  tapiObjCMetadata
  )
