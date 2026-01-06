//===- tools/tapi-run/tapi-run.cpp - TAPI Run Tool --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
///
/// A tool to run perfromance tests.
///
//===----------------------------------------------------------------------===//

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include "tapi/Core/FileSystem.h"
#include "tapi/tapi.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/BinaryFormat/MachO.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

static cl::OptionCategory tapiRunCategory("tapi-run options");

static cl::opt<std::string> inputDirectory(cl::Positional,
                                           cl::desc("<directory>"),
                                           cl::cat(tapiRunCategory));

static cl::opt<std::string> outputFilename("o", cl::desc("Output filename"),
                                           cl::value_desc("filename"));

static cl::list<std::string> archs("arch", cl::CommaSeparated,
                                   cl::desc("list of architectures to parse"),
                                   cl::value_desc("armv7,armv7s,..."),
                                   cl::cat(tapiRunCategory));

static cl::opt<std::string>
    deploymentTarget("version_min", cl::desc("minimum deployment target"),
                     cl::value_desc("10.0"), cl::cat(tapiRunCategory));

static cl::opt<unsigned> num("n", cl::desc("number of iterations"),
                             cl::value_desc("1"), cl::init(1),
                             cl::cat(tapiRunCategory));

static std::tuple<cpu_type_t, cpu_subtype_t, StringRef>
parseArchKind(StringRef arch) {
  auto cpuType = StringSwitch<cpu_type_t>(arch)
                     .Case("armv7", MachO::CPU_TYPE_ARM)
                     .Case("armv7s", MachO::CPU_TYPE_ARM)
                     .Case("armv7k", MachO::CPU_TYPE_ARM)
                     .Case("arm64", MachO::CPU_TYPE_ARM64)
                     .Case("i386", MachO::CPU_TYPE_I386)
                     .Case("x86_64", MachO::CPU_TYPE_X86_64)
                     .Case("x86_64h", MachO::CPU_TYPE_X86_64)
                     .Default(MachO::CPU_TYPE_ANY);

  auto cpuSubType = StringSwitch<cpu_subtype_t>(arch)
                        .Case("armv7", MachO::CPU_SUBTYPE_ARM_V7)
                        .Case("armv7s", MachO::CPU_SUBTYPE_ARM_V7S)
                        .Case("armv7k", MachO::CPU_SUBTYPE_ARM_V7K)
                        .Case("arm64", MachO::CPU_SUBTYPE_ARM64_ALL)
                        .Case("i386", MachO::CPU_SUBTYPE_I386_ALL)
                        .Case("x86_64", MachO::CPU_SUBTYPE_X86_64_ALL)
                        .Case("x86_64h", MachO::CPU_SUBTYPE_X86_64_H)
                        .Default(MachO::CPU_TYPE_ANY);

  return std::make_tuple(cpuType, cpuSubType, arch);
}

static tapi::PackedVersion32 parseVersion32(StringRef str) {
  uint32_t version = 0;
  if (str.empty())
    return 0;

  SmallVector<StringRef, 3> parts;
  SplitString(str, parts, ".");

  unsigned long long num = 0;
  if (getAsUnsignedInteger(parts[0], 10, num))
    return 0;

  if (num > UINT16_MAX)
    return 0;

  version = num << 16;

  if (parts.size() > 1) {
    if (getAsUnsignedInteger(parts[1], 10, num))
      return 0;

    if (num > UINT8_MAX)
      return 0;

    version |= (num << 8);
  }

  if (parts.size() > 2) {
    if (getAsUnsignedInteger(parts[2], 10, num))
      return 0;

    if (num > UINT8_MAX)
      return 0;

    version |= num;
  }

  return version;
}

int main(int argc, const char *argv[]) {
  // Standard set up, so program fails gracefully.
  sys::PrintStackTraceOnErrorSignal(argv[0]);
  PrettyStackTraceProgram stackPrinter(argc, argv);
  llvm_shutdown_obj shutdown;

  cl::HideUnrelatedOptions(tapiRunCategory);
  cl::ParseCommandLineOptions(argc, argv, "TAPI Run Tool\n");

  if (inputDirectory.empty()) {
    cl::PrintHelpMessage();
    return 0;
  }

  if (!sys::fs::exists(inputDirectory)) {
    errs() << "error: path does not exist (" << inputDirectory << ").\n";
    return 1;
  }

  if (!sys::fs::is_directory(inputDirectory)) {
    errs() << "error: path is not a directory (" << inputDirectory << ").\n";
    return 1;
  }

  SmallString<PATH_MAX> path(inputDirectory);
  if (auto ec = tapi::internal::realpath(path)) {
    errs() << "error: " << ec.message() << " (" << path << ")\n";
    return 1;
  }

  std::vector<std::tuple<cpu_type_t, cpu_subtype_t, StringRef>> archSet;
  for (auto &arch : archs) {
    auto archKind = parseArchKind(arch);
    if (std::get<0>(archKind) == MachO::CPU_TYPE_ANY) {
      errs() << "error: unsupported architecture " << arch << ".\n";
      return 1;
    }
    archSet.emplace_back(archKind);
  }

  if (archSet.empty()) {
    errs() << "error: no architecture provided.\n";
    return 1;
  }

  if (deploymentTarget.empty()) {
    errs() << "error: no minimum deployment target specified.\n";
    return 1;
  }

  auto packedVersion = parseVersion32(deploymentTarget);
  if (packedVersion == tapi::PackedVersion32(0)) {
    errs() << "error: invalid minimum version " << deploymentTarget << ".\n";
    return 1;
  }

  if (outputFilename.empty())
    outputFilename = "-";

  std::error_code ec2;
  raw_fd_ostream file(outputFilename, ec2, sys::fs::OpenFlags::OF_None);

  auto currentBenchmarkName = sys::path::stem(path);

  auto start = TimeRecord::getCurrentTime(/*start=*/true);
  std::error_code ec;
  for (sys::fs::recursive_directory_iterator i(path, ec), ie; i != ie;
       i.increment(ec)) {

    // Skip files/directories/symlinks we cannot read.
    if (ec) {
      errs() << "error: " << ec.message() << " (" << i->path() << ")\n";
      return 1;
    }

    bool isSymlink;
    if (auto ec = sys::fs::is_symlink_file(i->path(), isSymlink)) {
      errs() << "error: " << ec.message() << " (" << i->path() << ")\n";
      return 1;
    }

    // Don't follow symlinks.
    if (isSymlink) {
      i.no_push();
      continue;
    }

    if (sys::path::extension(i->path()) != ".tbd")
      continue;

    for (auto &arch : archSet) {
      for (unsigned j = 0; j < num; ++j) {
        std::string errorMessage;
        auto file = std::unique_ptr<tapi::LinkerInterfaceFile>(
            tapi::LinkerInterfaceFile::create(
                i->path(), std::get<0>(arch), std::get<1>(arch),
                tapi::ParsingFlags::None, packedVersion, errorMessage));
        if (file == nullptr) {
          errs() << "error: " << errorMessage << "\n";
          return 1;
        }
      }
    }
  }

  auto time = TimeRecord::getCurrentTime(/*start=*/false);
  time -= start;
  file << "nts." << currentBenchmarkName << ".user "
       << format("%0.6f", time.getUserTime()) << "\n";
  file << "nts." << currentBenchmarkName << ".sys "
       << format("%0.6f", time.getSystemTime()) << "\n";
  file << "nts." << currentBenchmarkName << ".wall "
       << format("%0.6f", time.getWallTime()) << "\n";

  file.flush();
  if (ec2) {
    errs() << "error: " << ec2.message() << "\n";
    return 1;
  }

  return 0;
}

#pragma clang diagnostic pop
