//===- tapi-sdkdb/tapi-sdkdb.cpp --------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
///
/// A tool to read binary SDKDB
///
//===----------------------------------------------------------------------===//
#include "tapi/Core/API.h"
#include "tapi/Core/APIJSONSerializer.h"
#include "tapi/Core/APIPrinter.h"
#include "tapi/Core/MachOReader.h"
#include "tapi/Diagnostics/Diagnostics.h"
#include "tapi/SDKDB/BitcodeReader.h"
#include "tapi/SDKDB/BitcodeWriter.h"
#include "tapi/SDKDB/CompareConfigFileReader.h"
#include "tapi/SDKDB/SDKDB.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace TAPI_INTERNAL;

static cl::OptionCategory tapiCategory("tapi-sdkdb generic options");
static cl::OptionCategory extractCategory("tapi-sdkdb --extract options");
static cl::OptionCategory compareCategory("tapi-sdkdb --compare options");

enum OutputKind {
  Metadata,
  AvailableTargets,
  Interfaces,
  PathCheck,
  ProjectWithError,
  ExtractTargets,
  APILoad,
  Compare,
};

static cl::opt<OutputKind> outputKind(
    cl::desc("choose command action:"),
    cl::values(clEnumValN(Metadata, "metadata", "print bitcode metadata"),
               clEnumValN(AvailableTargets, "available-targets",
                          "print all available targets"),
               clEnumValN(PathCheck, "check-path",
                          "perform fast check path exists for target"),
               clEnumValN(APILoad, "load-api",
                          "load all the APIs (include re-exports)"),
               clEnumValN(ProjectWithError, "error-projects",
                          "print all projects with errors"),
               clEnumValN(ExtractTargets, "extract",
                          "extract targets from SDKDB"),
               clEnumValN(Interfaces, "api", "print all interfaces"),
               clEnumValN(Compare, "compare",
                          "compare SDKDB against baseline for regressions")),
    cl::init(OutputKind::Metadata), cl::cat(tapiCategory));

static cl::list<std::string>
    sdkdbTargets("target", cl::desc("select target to print (all if not set)"),
                 cl::cat(tapiCategory));
static cl::list<std::string>
    apiNames("name",
             cl::desc("select the installName to print (all if not set)"),
             cl::cat(tapiCategory));

static cl::opt<std::string> outputFile("o", cl::desc("<output SDKDB>"),
                                       cl::cat(tapiCategory));

static cl::opt<std::string> baselineFile("baseline",
                                         cl::desc("<baseline SDKDB>"),
                                         cl::cat(compareCategory));

static cl::opt<bool> newAPIAsError("error-new-api",
                                   cl::desc("Treat new APIs in SDKDB as error"),
                                   cl::cat(compareCategory));

static cl::opt<bool> noNewAPI("no-new-api", cl::desc("Do not report new APIs"),
                              cl::cat(compareCategory));

static cl::opt<std::string> diagOut("diagnostic-file",
                                    cl::desc("Output file for diagnostics"),
                                    cl::cat(compareCategory));

// TODO: generalize the option to control whether to include frontend APIs for
// all kinds of actions.
static cl::opt<bool>
    compareFrontendAPI("compare-frontend-api",
                       cl::desc("Compare frontend APIs (enums and typedefs)"),
                       cl::Hidden, cl::cat(compareCategory));

static cl::opt<std::string>
    compareConfigFile("config-file",
                      cl::desc("Configuration file for comparing SDKDB"),
                      cl::cat(compareCategory));

static cl::opt<std::string>
    comparePlatform("platform",
                    cl::desc("base platform (macos, ios, tvos, watchos, xros) "
                             "for comparing SDKDB\n"),
                    cl::cat(compareCategory));

static cl::opt<std::string> sdkdbFile(cl::Positional, cl::desc("<SDKDB>"),
                                      cl::Required, cl::cat(tapiCategory));

static cl::opt<bool>
    removeObjCMetadata("remove-objc-metadata",
                       cl::desc("Remove ObjCMetadata from SDKDB output"),
                       cl::cat(extractCategory));

static cl::opt<bool> removeBundles("remove-bundles",
                                   cl::desc("Remove Bundles from SDKDB output"),
                                   cl::cat(extractCategory));

static cl::opt<bool> simplifiedSDKDB("simplified",
                                     cl::desc("Output simplified SDKDB"),
                                     cl::cat(extractCategory));

int main(int argc, const char *argv[]) {
  // Standard set up, so program fails gracefully.
  sys::PrintStackTraceOnErrorSignal(argv[0]);
  PrettyStackTraceProgram stackPrinter(argc, argv);
  llvm_shutdown_obj shutdown;

  if (sys::Process::FixupStandardFileDescriptors())
    return 1;

  cl::ParseCommandLineOptions(argc, argv, "TAPI SDKDB Reader\n\n"
      "  Read SDKDB from bitcode format\n");

  auto file = MemoryBuffer::getFile(sdkdbFile);
  if (!file) {
    errs() << "cannot open input file: " << sdkdbFile << "\n";
    return 1;
  }

  SDKDBBitcodeMaterializeOption option;

  auto readerOrError =
      SDKDBBitcodeReader::get((*file)->getMemBufferRef(), option);
  if (!readerOrError) {
    errs() << "cannot read SDKDB: " << toString(readerOrError.takeError())
           << "\n";
    return 1;
  }

  auto *reader = readerOrError->get();

  switch (outputKind) {
  case Metadata:
    outs() << "SDKDB Format Version: " << reader->getSDKDBVersion() << "\n";
    outs() << "SDKDB Build Version: " << reader->getBuildVersion() << "\n";
    if (reader->isPublicOnly())
      outs() << "SDKDB contains only public APIs\n";
    if (reader->noObjCMetadata())
      outs() << "SDKDB has no objc metadata\n";
    break;
  case AvailableTargets:
    outs() << "Available Targets:\n";
    for (auto &triple : reader->getAvailableTriples())
      outs() << "  " << triple.str() << "\n";
    break;
  case ProjectWithError:
    for (auto &proj : reader->getProjectsWithError())
      outs() << proj << "\n";
    break;
  case PathCheck: {
    if (sdkdbTargets.size() != 1 && apiNames.size() != 1) {
      errs() << "check-path option requires one -target and one -name option\n";
      return 1;
    }
    Triple target(sdkdbTargets.front());
    auto result = reader->dylibExistsForPath(target, apiNames.front());
    if (!result) {
      errs() << "Cannot read dylib table: " << toString(result.takeError())
             << "\n";
      return 1; 
    }
    if (*result)
      outs() << "Dylib exists for: " << apiNames.front() << " (" << target.str()
             << ")\n";
    else {
      outs() << "Dylib doesn't exist for: " << apiNames.front() << " ("
             << target.str() << ")\n";
      return 1;
    }
    break;
  }
  case APILoad: {
    if (sdkdbTargets.size() != 1 || apiNames.size() != 1) {
      errs() << "load-api option requires one -target and one -name option\n";
      return 1;
    }
    DiagnosticsEngine diag(errs());
    SDKDBBuilder builder(diag);
    Triple target(sdkdbTargets.front());
    if (auto err =
            reader->loadAPIsFromSDKDB(builder, target, apiNames.front())) {
      errs() << "cannot read SDKDB: " << toString(std::move(err)) << "\n";
      return 1;
    }
    builder.serialize(outs(), /*compact*/ false);
    break;
  }
  case ExtractTargets: {
    if (sdkdbTargets.size() < 1) {
      errs() << "extract option requires one or more -target option\n";
      return 1;
    }
    if (outputFile.empty()) {
      errs() << "extract option requires output to be set\n";
      return 1;
    }
    for (auto &tt : sdkdbTargets)
      option.targets.push_back(Triple(tt));
    DiagnosticsEngine diag(errs());
    SDKDBBuilder builder(diag);
    if (auto err = reader->materialize(builder)) {
      errs() << "cannot read SDKDB: " << toString(std::move(err)) << "\n";
      return 1;
    }
    if (simplifiedSDKDB) {
      builder.setRemoveObjCMetadata();
      builder.setRemoveBundles();
      builder.setRemoveEnumTypes();
    }
    if (removeObjCMetadata)
      builder.setRemoveObjCMetadata();
    if (removeBundles)
      builder.setRemoveBundles();
    std::error_code ec;
    raw_fd_ostream fs(outputFile, ec);
    if (ec) {
      errs() << "cannot option output file " << outputFile << ": "
             << ec.message() << "\n";
      return 1;
    }
    SDKDBBitcodeWriter writer;
    writer.writeSDKDBToStream(builder, fs);
    break;
  }
  case Interfaces: {
    for (auto &tt : sdkdbTargets)
      option.targets.push_back(Triple(tt));
    for (auto &name : apiNames)
      option.installNames.insert(name);
    DiagnosticsEngine diag(errs());
    SDKDBBuilder builder(diag);
    if (auto err = reader->materialize(builder)) {
      errs() << "cannot read SDKDB: " << toString(std::move(err)) << "\n";
      return 1;
    }
    builder.serialize(outs(), /*compact*/ false);
    break;
  }
  case Compare: {
    if (baselineFile.empty()) {
      errs() << "-baseline is required for compare\n";
      return 1;
    }
    for (auto &tt : sdkdbTargets)
      option.targets.push_back(Triple(tt));
    for (auto &name : apiNames)
      option.installNames.insert(name);

    // read baseline file.
    auto baseFile = MemoryBuffer::getFile(baselineFile);
    if (!baseFile) {
      errs() << "cannot open baseline file: " << baselineFile << "\n";
      return 1;
    }
    auto baseOrError =
        SDKDBBitcodeReader::get((*baseFile)->getMemBufferRef(), option);
    if (!baseOrError) {
      errs() << "cannot read baseline SDKDB: "
             << toString(baseOrError.takeError()) << "\n";
      return 1;
    }
    auto *base = baseOrError->get();

    // materialize SDKDBs.
    DiagnosticsEngine diag(errs());
    if (!diagOut.empty())
      diag.setupDiagnosticsFile(diagOut);

    SDKDBBuilder builder(diag);
    if (auto err = reader->materialize(builder)) {
      errs() << "cannot read SDKDB: " << toString(std::move(err)) << "\n";
      return 1;
    }
    SDKDBBuilder baseline(diag);
    if (auto err = base->materialize(baseline)) {
      errs() << "cannot read baseline SDKDB: " << toString(std::move(err))
             << "\n";
      return 1;
    }

    // Read config file if provided
    if (!compareConfigFile.empty()) {
      auto configFile =
          MemoryBuffer::getFile(compareConfigFile, /*IsText=*/true);
      if (!configFile) {
        errs() << "cannot open configuration file '" << compareConfigFile
               << "': " << configFile.getError().message() << "\n";
        return 1;
      }
      auto reader = CompareConfigFileReader::get(std::move(configFile.get()));
      if (!reader) {
        errs() << "cannot read configuration file: "
               << toString(reader.takeError()) << "\n";
        return 1;
      }
      builder.setCompareConfigFileReader(std::move(reader.get()));
    }

    if (!comparePlatform.empty()) {
      if (!builder.setFilteredPlatforms(comparePlatform)) {
        errs() << "invalid platform type: " << comparePlatform << "\n";
        return 1;
      }
    }

    builder.setNoNewAPI(noNewAPI);
    builder.setReportNewAPIasError(newAPIAsError);
    builder.setDiagnoseFrontendAPI(compareFrontendAPI);
    if (!builder.diagnoseDifferences(baseline))
      return 1;

    break;
  }
  }
  return 0;
}
