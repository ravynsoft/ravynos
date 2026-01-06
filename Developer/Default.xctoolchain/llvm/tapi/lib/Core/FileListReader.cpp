//===- lib/Core/FileListReader.cpp - File List Reader -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the JSON parser for the file list.
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/FileListReader.h"
#include "clang/Basic/LangStandard.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/MemoryBuffer.h"

using namespace llvm;
using namespace json;
using namespace tapi::internal;

TAPI_NAMESPACE_INTERNAL_BEGIN

class FileListReader::Implementation {
private:
  Expected<StringRef> parseString(const Object *obj, StringRef key,
                                  StringRef error);
  Expected<StringRef> parsePath(const Object *obj);
  Expected<HeaderType> parseType(const Object *obj);
  std::optional<clang::Language> parseLanguage(const Object *obj);
  bool parseSwiftCompatibilityHeaderIndicator(const Object *obj);
  Error parseHeaders(Array &headers);

public:
  std::unique_ptr<MemoryBuffer> inputBuffer;
  unsigned version;
  std::vector<HeaderInfo> headerList;

  Error parse(StringRef input);
};

Expected<StringRef>
FileListReader::Implementation::parseString(const Object *obj, StringRef key,
                                            StringRef error) {
  auto str = obj->getString(key);
  if (!str)
    return make_error<StringError>(error, inconvertibleErrorCode());
  return *str;
}

Expected<HeaderType>
FileListReader::Implementation::parseType(const Object *obj) {
  auto type = parseString(obj, "type", "required field 'type' not specified");
  if (!type)
    return type.takeError();

  if (*type == "public")
    return HeaderType::Public;
  else if (*type == "private")
    return HeaderType::Private;
  else if (*type == "project" && version >= 2)
    return HeaderType::Project;

  return make_error<StringError>("unsupported header type",
                                 inconvertibleErrorCode());
}

Expected<StringRef>
FileListReader::Implementation::parsePath(const Object *obj) {
  auto path = parseString(obj, "path", "required field 'path' not specified");
  if (!path)
    return path.takeError();

  return *path;
}

std::optional<clang::Language>
FileListReader::Implementation::parseLanguage(const Object *obj) {
  auto language = obj->getString("language");
  if (!language)
    return std::nullopt;

  return StringSwitch<clang::Language>(*language)
      .Case("c", clang::Language::C)
      .Case("c++", clang::Language::CXX)
      .Case("objective-c", clang::Language::ObjC)
      .Case("objective-c++", clang::Language::ObjCXX)
      .Default(clang::Language::Unknown);
}

bool FileListReader::Implementation::parseSwiftCompatibilityHeaderIndicator(
    const Object *obj) {
  auto isSwiftCompatibilityHeader = obj->getBoolean("swiftCompatibilityHeader");
  if (!isSwiftCompatibilityHeader)
    return false;

  return *isSwiftCompatibilityHeader;
}

Error FileListReader::Implementation::parseHeaders(Array &headers) {
  for (const auto &header : headers) {
    auto *obj = header.getAsObject();
    if (!obj)
      return make_error<StringError>("expect a JSON object",
                                     inconvertibleErrorCode());
    auto type = parseType(obj);
    if (!type)
      return type.takeError();
    auto path = parsePath(obj);
    if (!path)
      return path.takeError();
    bool isSwiftCompatibilityHeader =
        parseSwiftCompatibilityHeaderIndicator(obj);
    auto language = parseLanguage(obj);

    headerList.emplace_back(HeaderInfo{*type, std::string(*path), language,
                                       isSwiftCompatibilityHeader});
  }

  return Error::success();
}

Error FileListReader::Implementation::parse(StringRef input) {
  auto value = json::parse(input);
  if (!value)
    return value.takeError();

  auto *root = value->getAsObject();
  if (!root)
    return make_error<StringError>("not a JSON object",
                                   inconvertibleErrorCode());

  auto ver = root->getString("version");
  if (!ver)
    return make_error<StringError>("required field 'version' not specified",
                                   inconvertibleErrorCode());
  if (ver->getAsInteger(10, version))
    return make_error<StringError>("invalid version number",
                                   inconvertibleErrorCode());

  if (version < 1 || version > 3)
    return make_error<StringError>("unsupported version",
                                   inconvertibleErrorCode());

  // Not specifying any header files is odd, but valid.
  auto headers = root->getArray("headers");
  if (!headers)
    return Error::success();

  auto error = parseHeaders(*headers);
  if (error)
    return error;

  return Error::success();
}

FileListReader::FileListReader(std::unique_ptr<MemoryBuffer> inputBuffer,
                               Error &error)
    : impl(*new FileListReader::Implementation()) {
  ErrorAsOutParameter errorAsOutParam(&error);
  impl.inputBuffer = std::move(inputBuffer);

  error = impl.parse(impl.inputBuffer->getBuffer());
}

Expected<std::unique_ptr<FileListReader>>
FileListReader::get(std::unique_ptr<MemoryBuffer> inputBuffer) {
  Error error = Error::success();
  std::unique_ptr<FileListReader> reader(
      new FileListReader(std::move(inputBuffer), error));
  if (error)
    return std::move(error);

  return reader;
}

FileListReader::~FileListReader() { delete &impl; }

int FileListReader::getVersion() const { return impl.version; }

void FileListReader::visit(Visitor &visitor) {
  for (auto &file : impl.headerList)
    visitor.visitHeaderFile(file);
}

FileListReader::Visitor::~Visitor() {}

void FileListReader::Visitor::visitHeaderFile(HeaderInfo &header) {}

TAPI_NAMESPACE_INTERNAL_END

