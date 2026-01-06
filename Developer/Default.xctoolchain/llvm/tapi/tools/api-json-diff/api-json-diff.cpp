//===- api-json-diff/api-json-diff.cpp --------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
///
/// A test tool to load serialized API in JSON of different formats
/// against each other.
///
//===----------------------------------------------------------------------===//
#include "tapi/Core/API.h"
#include "tapi/Core/APIJSONSerializer.h"
#include "tapi/Diagnostics/Diagnostics.h"
#include "tapi/SDKDB/PartialSDKDB.h"
#include "tapi/SDKDB/SDKDB.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/raw_ostream.h"
#include <utility>

using namespace llvm;
using namespace llvm::MachO;
using namespace TAPI_INTERNAL;

static cl::OptionCategory tapiCategory("api-json-diff options");

static cl::opt<bool> isPartialSDKDB("partial-sdkdb",
                                    cl::desc("input is partial sdkdb"),
                                    cl::cat(tapiCategory));
static cl::opt<bool> isFullSDKDB("sdkdb", cl::desc("input is complete sdkdb"),
                                 cl::cat(tapiCategory));

static cl::opt<std::string> lhsInputFilename(cl::Positional,
                                             cl::desc("<input file1>"),
                                             cl::cat(tapiCategory));

static cl::opt<std::string> rhsInputFilename(cl::Positional,
                                             cl::desc("<input file2>"),
                                             cl::cat(tapiCategory));

namespace {

Expected<std::unique_ptr<llvm::MemoryBuffer>>
loadFile(const StringRef filePath) {
  auto buffer = MemoryBuffer::getFile(filePath);
  if (!buffer)
    return createStringError(buffer.getError(), "unable to open file");
  return std::move(*buffer);
}

Expected<std::unique_ptr<json::Object>> loadJSON(const StringRef filePath) {
  auto bufferOrErr = loadFile(filePath);
  if (!bufferOrErr)
    return bufferOrErr.takeError();

  auto jsonOrErr = json::parse((*bufferOrErr)->getBuffer());
  if (!jsonOrErr)
    return jsonOrErr.takeError();
  return std::make_unique<json::Object>(*jsonOrErr->getAsObject());
}

using Partials = std::pair<const PartialSDKDB, const PartialSDKDB>;
Expected<std::unique_ptr<Partials>> loadPartialSDKDB(const StringRef filePath) {
  auto jsonOrErr = loadJSON(filePath);
  if (!jsonOrErr)
    return jsonOrErr.takeError();

  auto publicPartial = PartialSDKDB::createPublicAPIsFromJSON(**jsonOrErr);
  if (!publicPartial)
    return publicPartial.takeError();
  auto privatePartial = PartialSDKDB::createPrivateAPIsFromJSON(**jsonOrErr);
  if (!privatePartial)
    return privatePartial.takeError();

  return std::make_unique<Partials>(
      std::make_pair(std::move(*publicPartial), std::move(*privatePartial)));
}

struct APIPtrComp {
  bool operator()(const API *lhs, const API *rhs) const { return *lhs < *rhs; }
};

using OrderedAPISet = std::set<const API *, APIPtrComp>;

bool checkPartialSDKDB(const PartialSDKDB &lhs, const PartialSDKDB &rhs) {
  if (lhs.project != rhs.project)
    return false;
  if (lhs.hasError != rhs.hasError)
    return false;

  if (lhs.binaryInterfaces.size() != rhs.binaryInterfaces.size())
    return false;
  if (lhs.headerInterfaces.size() != rhs.headerInterfaces.size())
    return false;

  // Add all apis into ordered container then compare them.
  OrderedAPISet lhsAPIs;
  OrderedAPISet rhsAPIs;
  llvm::for_each(lhs.binaryInterfaces,
                 [&lhsAPIs](const auto &api) { lhsAPIs.insert(&api); });
  llvm::for_each(lhs.headerInterfaces,
                 [&lhsAPIs](const auto &api) { lhsAPIs.insert(&api); });
  llvm::for_each(rhs.binaryInterfaces,
                 [&rhsAPIs](const auto &api) { rhsAPIs.insert(&api); });
  llvm::for_each(rhs.headerInterfaces,
                 [&rhsAPIs](const auto &api) { rhsAPIs.insert(&api); });
  for (auto lhsIt = lhsAPIs.begin(), rhsIt = rhsAPIs.begin();
       lhsIt != lhsAPIs.end() && rhsIt != rhsAPIs.end(); ++lhsIt, ++rhsIt) {
    if (!(**lhsIt == **rhsIt))
      return false;
  }
  return true;
}

bool checkPartialsEquality(const Partials &lhs, const Partials &rhs) {
  const auto &[lhsPublic, lhsPrivate] = lhs;
  const auto &[rhsPublic, rhsPrivate] = rhs;
  if (!checkPartialSDKDB(lhsPublic, rhsPublic))
    return false;

  return checkPartialSDKDB(lhsPrivate, rhsPrivate);
}

Expected<std::unique_ptr<const API>> loadFullAPI(const StringRef filePath) {
  auto jsonOrErr = loadJSON(filePath);
  if (!jsonOrErr)
    return jsonOrErr.takeError();
  auto apiOrErr = APIJSONSerializer::parse(jsonOrErr->get());
  if (!apiOrErr)
    return apiOrErr.takeError();
  return std::make_unique<const API>(std::move(*apiOrErr));
}

Expected<bool> checkFullSDKDBEquality(const StringRef lhsPath,
                                      const StringRef rhsPath) {

  auto lhsBuffer = loadFile(lhsPath);
  if (!lhsBuffer)
    return lhsBuffer.takeError();

  auto rhsBuffer = loadFile(rhsPath);
  if (!rhsBuffer)
    return rhsBuffer.takeError();

  DiagnosticsEngine diag(llvm::errs());
  SDKDBBuilder lhsBuilder(diag);
  if (auto err = lhsBuilder.parse((*lhsBuffer)->getBuffer())) {
    errs() << lhsPath << ": ";
    return std::move(err);
  }

  SDKDBBuilder rhsBuilder(diag);
  if (auto err = rhsBuilder.parse((*rhsBuffer)->getBuffer())) {
    errs() << rhsPath << ": ";
    return std::move(err);
  }

  // Capture APIs from SDKDB objects.
  OrderedAPISet lhsAPIs;
  for (const auto *db : lhsBuilder.getDatabases())
    for (const auto *api : db->api())
      lhsAPIs.insert(api);

  OrderedAPISet rhsAPIs;
  for (const auto *db : rhsBuilder.getDatabases())
    for (const auto *api : db->api())
      rhsAPIs.insert(api);

  if (lhsAPIs.size() != rhsAPIs.size())
    return false;

  for (auto lhsIt = lhsAPIs.begin(), rhsIt = rhsAPIs.begin();
       lhsIt != lhsAPIs.end() && rhsIt != rhsAPIs.end(); ++lhsIt, ++rhsIt) {
    if (!(**lhsIt == **rhsIt))
      return false;
  }
  return true;
}

template <typename ApiT>
std::unique_ptr<const ApiT>
handleInput(const StringRef fileName,
            Expected<std::unique_ptr<const ApiT>> apiOrErr) {
  if (auto &&error = apiOrErr.takeError()) {
    errs() << "error: unable to process " << fileName << ": "
           << toString(std::move(error)) << "\n";
    return nullptr;
  }
  return std::move(*apiOrErr);
}

enum Status { InputError = -1, Equal = 0, Inequal = 1 };

Status loadAndComparePartialSDKDB() {
  auto lhsSDKDB = handleInput<Partials>(lhsInputFilename,
                                        loadPartialSDKDB(lhsInputFilename));
  if (!lhsSDKDB)
    return Status::InputError;
  auto rhsSDKDB = handleInput<Partials>(rhsInputFilename,
                                        loadPartialSDKDB(rhsInputFilename));
  if (!rhsSDKDB)
    return Status::InputError;
  if (checkPartialsEquality(*lhsSDKDB, *rhsSDKDB))
    return Status::Equal;

  return Status::Inequal;
}

Status loadAndCompareFullSDKDB() {
  auto result = checkFullSDKDBEquality(lhsInputFilename, rhsInputFilename);
  if (auto error = result.takeError()) {
    errs() << toString(std::move(error)) << "\n";
    return Status::InputError;
  }
  if (*result)
    return Status::Equal;
  return Status::Inequal;
}

Status loadAndCompareAPI() {
  // Handle basic API as JSON input.
  auto lhsAPI =
      handleInput<API>(lhsInputFilename, loadFullAPI(lhsInputFilename));
  if (!lhsAPI)
    return Status::InputError;
  auto rhsAPI =
      handleInput<API>(rhsInputFilename, loadFullAPI(rhsInputFilename));
  if (!rhsAPI)
    return Status::InputError;

  // TODO: add pretty print logic for differences.
  if (*lhsAPI == *rhsAPI)
    return Status::Equal;
  return Status::Inequal;
}

} // namespace

int main(int argc, const char *argv[]) {
  // Standard set up, so program fails gracefully.
  sys::PrintStackTraceOnErrorSignal(argv[0]);
  const PrettyStackTraceProgram stackPrinter(argc, argv);
  const llvm_shutdown_obj shutdown;

  if (sys::Process::FixupStandardFileDescriptors())
    return 1;

  cl::HideUnrelatedOptions(tapiCategory);
  cl::ParseCommandLineOptions(argc, argv, "API JSON Diff\n");

  if (lhsInputFilename.empty() || rhsInputFilename.empty()) {
    cl::PrintHelpMessage();
    return 0;
  }

  if (isPartialSDKDB)
    return loadAndComparePartialSDKDB();

  if (isFullSDKDB)
    return loadAndCompareFullSDKDB();

  return loadAndCompareAPI();
}
