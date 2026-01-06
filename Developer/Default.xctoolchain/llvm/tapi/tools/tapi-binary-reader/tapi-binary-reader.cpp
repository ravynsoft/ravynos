//===- tapi-binary-reader/tapi-binary-reader.cpp ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
///
/// A tool to read API from binary
///
//===----------------------------------------------------------------------===//
#include "tapi/Core/API.h"
#include "tapi/Core/APIJSONSerializer.h"
#include "tapi/Core/APIPrinter.h"
#include "tapi/Core/MachOReader.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::MachO;
using namespace TAPI_INTERNAL;

static cl::OptionCategory tapiCategory("tapi-binary-reader options");

static cl::list<std::string> arches("arch", cl::desc("arch to parse"),
                                    cl::cat(tapiCategory));

static cl::opt<bool> noColors("no-colors", cl::desc("don't use color output"),
                              cl::cat(tapiCategory));

static cl::opt<std::string> jsonOutput("json", cl::desc("output json file"),
                                       cl::cat(tapiCategory));
static cl::opt<bool> noUUID("no-uuid", cl::desc("don't include uuid in json"),
                              cl::cat(tapiCategory));
static cl::opt<bool> noPrint("no-print", cl::desc("don't print API"),
                              cl::cat(tapiCategory));

static cl::opt<std::string> inputFilename(cl::Positional,
                                          cl::desc("<input file>"),
                                          cl::cat(tapiCategory));

int main(int argc, const char *argv[]) {
  // Standard set up, so program fails gracefully.
  sys::PrintStackTraceOnErrorSignal(argv[0]);
  PrettyStackTraceProgram stackPrinter(argc, argv);
  llvm_shutdown_obj shutdown;

  if (sys::Process::FixupStandardFileDescriptors())
    return 1;

  cl::HideUnrelatedOptions(tapiCategory);
  cl::ParseCommandLineOptions(argc, argv, "TAPI Binary Reader\n");

  if (inputFilename.empty()) {
    cl::PrintHelpMessage();
    return 0;
  }

  auto buffer = MemoryBuffer::getFile(inputFilename);
  if (!buffer) {
    errs() << "Cannot read input file: " << inputFilename << "\n";
    return -1;
  }

  if (auto fileType= getMachOFileType(*buffer->get())) {
    if (*fileType == FileType::Invalid) {
      errs() << "Input file is not valid macho interface file: "
             << inputFilename << "\n";
      return -1;
    }
  } else {
    errs() << "Cannot identify the file type of the input: " << inputFilename
           << ": " << toString(fileType.takeError()) << "\n";
    return -1;
  }

  ArchitectureSet archToParse;
  if (arches.empty()) {
    archToParse = ArchitectureSet::All();
  } else {
    for (const auto &arch : arches) {
      archToParse.set(getArchitectureFromName(arch));
    }
  }

  MachOParseOption option;
  option.arches = archToParse;
  auto results = readMachOFile((*buffer)->getMemBufferRef(), option);

  if (!results) {
    errs() << "Cannot read API from the input: " << inputFilename << ": "
           << toString(results.takeError()) << "\n";
    return -1;
  }

  // If json output is given, write to json file, otherwise, just print it.
  if (!jsonOutput.empty()) {
    std::error_code err;
    raw_fd_ostream jsonOut(jsonOutput, err);
    if (err) {
      errs() << "Cannot open \'" << jsonOutput
             << "\' for json output: " << err.message() << "\n";
      return 1;
    }

    APIJSONOption options = {};
    options.noUUID = noUUID;
    for (auto &r : *results) {
      APIJSONSerializer serializer(*r.second, options);
      serializer.serialize(jsonOut);
    }
  }

  if (noPrint)
    return 0;

  for (auto &r : *results) {
    if (!noColors)
      errs().changeColor(raw_ostream::MAGENTA);
    errs() << "Architecture: ";
    if (!noColors)
      errs().resetColor();
    errs() << r.first << "\n";

    SortedAPI sortedResult(*r.second);
    APIPrinter printer(errs(), !noColors);
    sortedResult.visit(printer);
  }

  return 0;
}
