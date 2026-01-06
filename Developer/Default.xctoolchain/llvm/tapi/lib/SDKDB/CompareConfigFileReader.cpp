//===- lib/SDKDB/CompareConfigFileReader.cpp - Config Reader ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the JSON parser for the compare configuration file.
///
//===----------------------------------------------------------------------===//

#include "tapi/SDKDB/CompareConfigFileReader.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/StringSaver.h"
#include "llvm/TargetParser/Triple.h"
#include <set>

using namespace llvm;
using namespace json;

TAPI_NAMESPACE_INTERNAL_BEGIN

class CompareConfigFileReader::Implementation {
private:
  Expected<StringRef> parseString(const Object *obj, StringRef key,
                                  StringRef error);
  Expected<StringRef> parseName(const Object *obj);
  Expected<EntryType> parseType(const Object *obj);
  Expected<ChangeType> parseChangeType(const Object *obj);
  Expected<std::vector<llvm::Triple>> parseTargets(const Object *obj);
  std::optional<StringRef> parseContainer(const Object *obj);
  std::optional<StringRef> parseInstallName(const Object *obj);
  Error parseExpectedChanges(const Array *changes);

  BumpPtrAllocator allocator;
  UniqueStringSaver strings;

public:
  Implementation() : strings(allocator) {}
  std::unique_ptr<MemoryBuffer> inputBuffer;
  unsigned version;
  StringMap<std::set<Change>> expectedChanges;
  std::set<StringRef> ignoredLibraries;

  Error parse(StringRef input);
};

Expected<StringRef> CompareConfigFileReader::Implementation::parseString(
    const Object *obj, StringRef key, StringRef error) {
  auto str = obj->getString(key);
  if (!str)
    return make_error<StringError>(error, inconvertibleErrorCode());
  return *str;
}

Expected<StringRef>
CompareConfigFileReader::Implementation::parseName(const Object *obj) {
  auto name = parseString(obj, "name", "required field 'name' not specified");
  if (!name)
    return name.takeError();

  return *name;
}

Expected<EntryType>
CompareConfigFileReader::Implementation::parseType(const Object *obj) {
  auto typeStr =
      parseString(obj, "type", "required field 'type' not specified");
  if (!typeStr)
    return typeStr.takeError();

  auto type = StringSwitch<std::optional<EntryType>>(typeStr.get())
                  .Case("library", EntryType::Library)
                  .Case("global", EntryType::Global)
                  .Case("interface", EntryType::Interface)
                  .Case("protocol", EntryType::Protocol)
                  .Case("category", EntryType::Category)
                  .Case("selector", EntryType::Selector)
                  .Case("instance method", EntryType::InstanceMethod)
                  .Case("class method", EntryType::ClassMethod)
                  .Default(std::nullopt);

  if (!type)
    return make_error<StringError>("unsupported entry type",
                                   inconvertibleErrorCode());

  return *type;
}

Expected<ChangeType>
CompareConfigFileReader::Implementation::parseChangeType(const Object *obj) {
  auto changeStr =
      parseString(obj, "change", "required field 'change' not specified");
  if (!changeStr)
    return changeStr.takeError();

  auto change = StringSwitch<std::optional<ChangeType>>(changeStr.get())
                    .Case("remove", ChangeType::Remove)
                    .Case("updateAccess", ChangeType::UpdateAccess)
                    .Case("add", ChangeType::Add)
                    .Default(std::nullopt);

  if (!change)
    return make_error<StringError>("unsupported change type",
                                   inconvertibleErrorCode());
  return *change;
}

Expected<std::vector<llvm::Triple>>
CompareConfigFileReader::Implementation::parseTargets(const Object *obj) {
  const auto *targetsArray = obj->getArray("targets");
  if (!targetsArray)
    return make_error<StringError>("required array 'targets' not specified",
                                   inconvertibleErrorCode());

  std::vector<llvm::Triple> targets;
  for (const auto &target : *targetsArray) {
    auto str = target.getAsString();
    if (!str)
      return make_error<StringError>("expecting string in 'targets'",
                                     inconvertibleErrorCode());
    targets.emplace_back(*str);
  }

  return targets;
}

std::optional<StringRef>
CompareConfigFileReader::Implementation::parseContainer(const Object *obj) {
  return obj->getString("container");
}

std::optional<StringRef>
CompareConfigFileReader::Implementation::parseInstallName(const Object *obj) {
  return obj->getString("installName");
}

Error CompareConfigFileReader::Implementation::parseExpectedChanges(
    const Array *changes) {
  for (const auto &change : *changes) {
    auto *obj = change.getAsObject();
    if (!obj)
      return make_error<StringError>("expect a JSON object",
                                     inconvertibleErrorCode());

    auto name = parseName(obj);
    if (!name)
      return name.takeError();
    auto type = parseType(obj);
    if (!type)
      return type.takeError();
    auto changeType = parseChangeType(obj);
    if (!changeType)
      return changeType.takeError();
    auto targets = parseTargets(obj);
    if (!targets)
      return targets.takeError();

    auto container = parseContainer(obj);
    if (isSelectorType(*type) && !container)
      return make_error<StringError>(
          "method entry must have a 'container' field",
          inconvertibleErrorCode());

    auto installName = parseInstallName(obj);
    if (*type != EntryType::Library && !installName)
      return make_error<StringError>(
          "symbol entry must have an 'installName' field",
          inconvertibleErrorCode());

    for (const auto &triple : *targets)
      expectedChanges[triple.str()].emplace(
          Change{*changeType, *type, strings.save(*name),
                 strings.save(installName.value_or("")),
                 strings.save(container.value_or(""))});
  }

  return Error::success();
}

Error CompareConfigFileReader::Implementation::parse(StringRef input) {
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

  if (version < 1 || version > 1)
    return make_error<StringError>("unsupported version",
                                   inconvertibleErrorCode());

  // Not specifying any change is odd, but valid.
  const auto *changes = root->getArray("expectedChanges");
  if (changes) {
    auto error = parseExpectedChanges(changes);
    if (error)
      return error;
  }

  const auto *libraries = root->getArray("ignoredLibraries");
  if (libraries) {
    for (const auto &library : *libraries) {
      auto installName = library.getAsString();
      if (!installName)
        return make_error<StringError>(
            "invalid install name in 'ignoredLibraries'",
            inconvertibleErrorCode());
      ignoredLibraries.emplace(strings.save(*installName));
    }
  }

  return Error::success();
}

CompareConfigFileReader::CompareConfigFileReader(
    std::unique_ptr<MemoryBuffer> inputBuffer, Error &error)
    : impl(*new CompareConfigFileReader::Implementation()) {
  ErrorAsOutParameter errorAsOutParam(&error);
  impl.inputBuffer = std::move(inputBuffer);

  error = impl.parse(impl.inputBuffer->getBuffer());
}

Expected<std::unique_ptr<CompareConfigFileReader>>
CompareConfigFileReader::get(std::unique_ptr<MemoryBuffer> inputBuffer) {
  Error error = Error::success();
  std::unique_ptr<CompareConfigFileReader> reader(
      new CompareConfigFileReader(std::move(inputBuffer), error));
  if (error)
    return std::move(error);

  return reader;
}

CompareConfigFileReader::~CompareConfigFileReader() { delete &impl; }

int CompareConfigFileReader::getVersion() const { return impl.version; }

const std::set<CompareConfigFileReader::Change> &
CompareConfigFileReader::expectedChanges(const llvm::Triple triple) const {
  return impl.expectedChanges[triple.str()];
}

const std::set<StringRef> &
CompareConfigFileReader::ignoredLibraries() const {
  return impl.ignoredLibraries;
}

TAPI_NAMESPACE_INTERNAL_END
