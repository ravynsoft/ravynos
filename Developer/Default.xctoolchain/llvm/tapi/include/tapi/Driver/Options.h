//===--- tapi/Driver/Options.h - Options ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef TAPI_DRIVER_OPTIONS_H
#define TAPI_DRIVER_OPTIONS_H

#include "tapi/Core/FileManager.h"
#include "tapi/Core/LLVM.h"
#include "tapi/Core/Path.h"
#include "tapi/Core/SymbolVerifier.h"
#include "tapi/Defines.h"
#include "tapi/Diagnostics/Diagnostics.h"
#include "tapi/Driver/DriverOptions.h"
#include "clang/Frontend/FrontendOptions.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/Option.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/TextAPI/Architecture.h"
#include "llvm/TextAPI/InterfaceFile.h"
#include "llvm/TextAPI/PackedVersion.h"
#include "llvm/TextAPI/Platform.h"
#include <set>
#include <string>
#include <vector>

TAPI_NAMESPACE_INTERNAL_BEGIN

using Macro = std::pair<std::string, bool /*isUndef*/>;

/// \brief A list of supported TAPI commands.
enum class TAPICommand : unsigned {
  Driver,
  Archive,
  Stubify,
  InstallAPI,
  Reexport,
  SDKDB,
  APIVerify,
};

/// \brief Archive action.
enum class ArchiveAction {
  Unknown,

  /// \brief Print the architectures in the input file.
  ShowInfo,

  /// \brief Specify the architecture to extract from the input file.
  ExtractArchitecture,

  /// \brief Specify the architecture to remove from the input file.
  RemoveArchitecture,

  /// \brief Verify the architecture exists in the input file.
  VerifyArchitecture,

  /// \brief Merge the input files.
  Merge,

  /// \brief List the exported symbols.
  ListSymbols,
};

struct DriverOptions {
  /// \brief Print version informtion.
  bool printVersion = false;

  /// \brief Print help.
  bool printHelp = false;

  /// \brief Print hidden options too.
  bool printHelpHidden = false;

  /// \brief List of input paths.
  PathSeq inputs;

  /// \brief Output path.
  std::string outputPath;

  /// VFS Overlay paths.
  PathSeq vfsOverlayPaths;

  /// Clang executable path.
  std::string clangExecutablePath;
};

struct ArchiveOptions {
  /// \brief Specifies which archive action to run.
  ArchiveAction action = ArchiveAction::Unknown;

  /// \brief Specifies the archive action architecture to use (if applicable).
  Architecture arch = llvm::MachO::AK_unknown;

  /// \brief This allows merging of TBD files containing the same architecture.
  bool allowArchitectureMerges = false;
};

struct LinkerOptions {
  /// \brief The install name to use for the dynamic library.
  std::string installName;

  /// \brief The current version to use for the dynamic library.
  PackedVersion currentVersion;

  /// \brief The compatibility version to use for the dynamic library.
  PackedVersion compatibilityVersion;

  /// \brief Set if we should scan for a dynamic library and not a framework.
  bool isDynamicLibrary = false;

  /// \brief List of allowable clients to use for the dynamic library.
  std::vector<std::pair<std::string, ArchitectureSet>> allowableClients;

  /// \brief List of reexported libraries to use for the dynamic library.
  std::vector<std::pair<std::string, ArchitectureSet>> reexportInstallNames;

  /// \brief List of reexported libraries to use for the dynamic library.
  std::vector<std::pair<std::string, ArchitectureSet>> reexportedLibraries;

  /// \brief List of reexported libraries to use for the dynamic library.
  std::vector<std::pair<std::string, ArchitectureSet>> reexportedLibraryPaths;

  /// \brief List of reexported frameworks to use for the dynamic library.
  std::vector<std::pair<std::string, ArchitectureSet>> reexportedFrameworks;

  /// \brief Is application extension safe.
  bool isApplicationExtensionSafe = false;

  /// \brief Is OS library that is not for shared cache.
  bool isOSLibNotForSharedCache = false;

  /// \brief Path to the alias list file.
  PathSeq aliasLists;

  /// \brief List of run search paths.
  std::vector<std::pair<std::string, ArchitectureSet>> rpaths;

  /// \brief List of relinked libraries to use for dynamic library.
  std::vector<InterfaceFileRef> relinkedLibraries;
};

struct FrontendOptions {
  /// \brief Targets to build for.
  std::vector<llvm::Triple> targets;

  /// \brief Additonal target variants to build for.
  std::vector<llvm::Triple> targetVariants;

  /// \brief Specify the language to use for parsing.
  clang::Language language = clang::Language::Unknown;

  /// \brief Language standard to use for parsing.
  std::string language_std;

  /// \brief The sysroot to search for SDK headers.
  std::string isysroot;

  /// \brief Name of the umbrella framework.
  std::string umbrella;

  /// \brief Additional SYSTEM framework search paths.
  PathToPlatformSeq systemFrameworkPaths;

  /// \brief Additional framework search paths.
  PathSeq frameworkPaths;

  /// \brief Additional library search paths.
  PathSeq libraryPaths;

  /// \brief Additional SYSTEM include paths.
  PathSeq systemIncludePaths;

  /// \brief Additional AFTER include paths.
  PathSeq afterIncludePaths;

  /// \brief Additional include paths.
  PathSeq includePaths;

  /// \brief Additional include local paths.
  PathSeq quotedIncludePaths;

  /// \brief Macros to use for for parsing.
  std::vector<Macro> macros;

  /// \brief overwrite to use RTTI.
  bool useRTTI = false;

  /// \brief overwrite to use no-RTTI.
  bool useNoRTTI = false;

  /// \brief Set the visibility.
  /// TODO: We should disallow this for header parsing, but we could still use
  /// it for warnings.
  std::string visibility;

  /// \brief Use modules.
  bool enableModules = false;

  /// \brief Module cache path.
  std::string moduleCachePath;

  /// \brief The name of the product being built.
  std::string productName;

  /// \brief Validate system headers when using modules.
  bool validateSystemHeaders = false;

  /// \brief Additional clang flags to be passed to the parser.
  std::vector<std::string> clangExtraArgs;

  /// \brief Clang resource path.
  std::string clangResourcePath;

  /// \brief Use Objective-C ARC (-fobjc-arc).
  bool useObjectiveCARC = false;

  /// \brief Use Objective-C weak ARC (-fobjc-weak).
  bool useObjectiveCWeakARC = false;

  /// \brief Verbose, show scan content and options.
  bool verbose = false;

  /// \brief Unique clang options to pass per key in map.
  std::map<std::string, std::vector<std::string>> uniqueClangArgs;

  /// \brief Prefix headers to include before parsing.
  std::vector<std::string> prefixHeaders;
};

struct DiagnosticsOptions {
  /// \brief Output path for the serialized diagnostics file.
  std::string serializeDiagnosticsFile;

  /// \brief Error limit.
  unsigned errorLimit = 0;
};

struct TAPIOptions {
  /// Path to file lists (JSON).
  std::vector<std::string> fileLists;

  /// \brief Path to public umbrella header.
  std::string publicUmbrellaHeaderPath;

  /// \brief Path to private umbrella header.
  std::string privateUmbrellaHeaderPath;

  /// \brief List of extra public header files.
  PathSeq extraPublicHeaders;

  /// \brief List of extra private header files.
  PathSeq extraPrivateHeaders;

  /// List of extra project header files.
  PathSeq extraProjectHeaders;

  /// \brief List of excluded public header files.
  PathSeq excludePublicHeaders;

  /// \brief List of excluded private header files.
  PathSeq excludePrivateHeaders;

  /// \brief List of excluded project header files.
  PathSeq excludeProjectHeaders;

  /// \brief List of swift interface files.
  PathSeq swiftInstallAPIInterfaces;

  /// \brief All -isysroot options. For multiple sysroot support.
  PathSeq allSysroots;

  /// \brief Path to dynamic library for verification.
  std::string verifyAgainst;

  /// \brief Verification mode.
  VerificationMode verificationMode = VerificationMode::ErrorsOnly;

  /// \brief Generate additional symbols for code coverage.
  bool generateCodeCoverageSymbols = false;

  /// \brief Demangle symbols (C++, Swift) when printing.
  bool demangle = false;

  /// \brief Log each library path that was consumed.
  bool traceLibraryLocation = false;

  /// \brief Specify whether to verify that all symbols from swift interface
  /// are represented in the binary.
  bool verifySwift = false;

  /// \brief Delete input file after stubbing.
  bool deleteInputFile = false;

  /// \brief Inline private frameworks.
  bool inlinePrivateFrameworks = false;

  /// \brief Delete private frameworks.
  bool deletePrivateFrameworks = false;

  /// \brief Remove shared cache flags.
  bool removeSharedCacheFlag = false;


  /// \brief Specify the output file type.
  llvm::MachO::FileType fileType = llvm::MachO::FileType::TBD_V5;

  /// \bried Scan Bundles and Extensions for SDKDB.
  bool scanAll = true;

  /// \brief Infer the include paths based on the provided/found header files.
  bool inferIncludePaths = true;

  // FIXME: re-implement printAfter to work with SymbolVerifier.
  /// \brief Print the API/XPI after a certain phase.
  std::string printAfter;

  /// \brief Verify the API of zippered frameworks.
  bool verifyAPI = true;

  /// \brief Skip external headers when verifying the API of a zippered
  /// framework.
  bool verifyAPISkipExternalHeaders = true;

  /// \brief Emit API verification errors as warning.
  bool verifyAPIErrorAsWarning = false;

  /// \brief Allowlist YAML file for API verification.
  std::string verifyAPIAllowlist; // EquivalentTypes.conf

  /// \brief SDKDB output location.
  std::string sdkdbOutputPath;

  /// \brief Path to dSYM.
  std::string dSYM;

  /// \brief Specify whether tapi is running in B&I environment.
  bool isBnI = false;
};

/// Specify the actions for SDKDB Driver.
enum SDKDBAction : unsigned {
  SDKDBNone = 0,
  SDKDBInterfaceScan = 1,
  SDKDBPublicGen = 1 << 1,
  SDKDBPrivateGen = 1 << 2,
  SDKDBAll = SDKDBInterfaceScan | SDKDBPublicGen | SDKDBPrivateGen,
};

struct SDKDBOptions {
  /// SDKDB Action.
  SDKDBAction action = SDKDBNone;

  /// Scan public headers.
  bool scanPublicHeaders = true;

  /// Scan private headers.
  bool scanPrivateHeaders = true;

  /// Path to configuration file.
  std::string configurationFile;

  /// Path to diagnostics file.
  std::string diagnosticsFile;

  /// Path to all the roots.
  std::string runtimeRoot;
  std::string sdkContentRoot;
  std::string publicSDKContentRoot;

  /// Path to partial SDKDB file list.
  std::string partialSDKDBFileList;
  /// Path to partial SDKDB directory from installAPI.
  std::string installAPISDKDBDirectory;
};

class Options {
private:
  // Handle options passed that must bind with pre-determined condition.
  // e.g. architecture specific options.
  bool processXOptions(DiagnosticsEngine &diag, llvm::opt::InputArgList &args,
                       bool clearOptions = true);

  bool processOptionList(DiagnosticsEngine &diag,
                         llvm::opt::InputArgList &args);

  using arg_iterator = llvm::opt::arg_iterator<llvm::opt::Arg **>;
  bool processXarchOptions(DiagnosticsEngine &diag,
                           llvm::opt::InputArgList &args, arg_iterator curr);

  bool processXplatformOptions(DiagnosticsEngine &diag,
                               llvm::opt::InputArgList &args,
                               arg_iterator curr);

  void processXparserOptions(llvm::opt::InputArgList &args, arg_iterator curr);

  bool processXprojectOptions(DiagnosticsEngine &diag,
                              llvm::opt::InputArgList &args, arg_iterator curr);

  bool processDriverOptions(DiagnosticsEngine &diag,
                            llvm::opt::InputArgList &args);

  bool processArchiveOptions(DiagnosticsEngine &diag,
                             llvm::opt::InputArgList &args);

  bool processLinkerOptions(DiagnosticsEngine &diag,
                            llvm::opt::InputArgList &args);

  bool processFrontendOptions(DiagnosticsEngine &diag,
                              llvm::opt::InputArgList &args);

  bool processDiagnosticsOptions(DiagnosticsEngine &diag,
                                 llvm::opt::InputArgList &args);

  bool processTAPIOptions(DiagnosticsEngine &diag,
                          llvm::opt::InputArgList &args);

  bool processSDKDBOptions(DiagnosticsEngine &diag,
                           llvm::opt::InputArgList &args);

public:
  /// \brief The TAPI command to run.
  TAPICommand command = TAPICommand::Driver;

  /// The various options grouped together.
  DriverOptions driverOptions;
  ArchiveOptions archiveOptions;
  LinkerOptions linkerOptions;
  FrontendOptions frontendOptions;
  DiagnosticsOptions diagnosticsOptions;
  TAPIOptions tapiOptions;
  SDKDBOptions sdkdbOptions;

  Options() = delete;

  /// \brief Constructor for options.
  Options(DiagnosticsEngine &diag, ArrayRef<const char *> argString);

  FileManager &getFileManager() const { return *fm; }

  const FrontendOptions &getProjectHeaderOptions() {
    return projectLevelOptions;
  }

  /// \brief Print the help depending on the recognized coomand.
  void printHelp() const;

private:
  std::string programName;
  std::unique_ptr<llvm::opt::OptTable> table;
  IntrusiveRefCntPtr<FileManager> fm;
  std::map<const llvm::opt::Arg *, Architecture> argToArchMap;
  std::map<const llvm::opt::Arg *, PlatformType> argToPlatformMap;
  FrontendOptions projectLevelOptions;

  friend class Context;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_DRIVER_OPTIONS_H
