//===--- Options.cpp - Options --------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "tapi/Driver/Options.h"
#include "tapi/Config/Version.h"
#include "tapi/Core/Path.h"
#include "tapi/Core/Utils.h"
#include "tapi/Defines.h"
#include "tapi/Diagnostics/Diagnostics.h"
#include "tapi/Driver/DriverOptions.h"
#include "tapi/LinkerInterfaceFile.h"
#include "tapi/Version.h"
#include "clang/Basic/Version.inc"
#include "clang/Config/config.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/OptSpecifier.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/VirtualFileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TextAPI/TextAPIError.h"
#include <string>
#include <utility>

using namespace llvm;
using namespace llvm::opt;
using namespace llvm::MachO;
using namespace clang;

TAPI_NAMESPACE_INTERNAL_BEGIN

constexpr char toolName[] = "Text-based Stubs Tool";

static TAPICommand getTAPICommand(StringRef value) {
  // Accept both command options (with and without "-") to not break existing
  // tools.
  return StringSwitch<TAPICommand>(value.ltrim("-"))
      .Case("archive", TAPICommand::Archive)
      .Case("stubify", TAPICommand::Stubify)
      .Case("installapi", TAPICommand::InstallAPI)
      .Case("reexport", TAPICommand::Reexport)
      .Case("sdkdb", TAPICommand::SDKDB)
      .Case("api-verify", TAPICommand::APIVerify)
      .Default(TAPICommand::Driver);
}

static StringRef getNameFromTAPICommand(TAPICommand command) {
  switch (command) {
  case TAPICommand::Driver:
    return "";
  case TAPICommand::Archive:
    return "archive";
  case TAPICommand::Stubify:
    return "stubify";
  case TAPICommand::InstallAPI:
    return "installapi";
  case TAPICommand::Reexport:
    return "reexport";
  case TAPICommand::SDKDB:
    return "sdkdb";
  case TAPICommand::APIVerify:
    return "api-verify";
  }
}

static InputArgList parseArgString(DiagnosticsEngine &diags,
                                   ArrayRef<const char *> argString,
                                   OptTable *optTable, unsigned includedFlags,
                                   unsigned excludedFlags) {
  unsigned missingArgIndex, missingArgCount;
  auto args = optTable->ParseArgs(argString, missingArgIndex, missingArgCount,
                                  includedFlags, excludedFlags);

  // Print errors and warnings for the parsed arguments.
  if (missingArgCount) {
    diags.report(clang::diag::err_drv_missing_argument)
        << args.getArgString(missingArgIndex) << missingArgCount;
    return args;
  }

  for (auto unknownArg : args.filtered(OPT_UNKNOWN)) {
    diags.report(clang::diag::err_drv_unknown_argument)
        << unknownArg->getAsString(args);
  }

  return args;
}

static unsigned getIncludeOptionFlagMasks(TAPICommand command) {
  unsigned flags = TapiFlags::DriverOption;

  switch (command) {
  case TAPICommand::Driver:
    break;
  case TAPICommand::Archive:
    flags |= TapiFlags::ArchiveOption;
    break;
  case TAPICommand::Stubify:
    flags |= TapiFlags::StubOption;
    break;
  case TAPICommand::InstallAPI:
    flags |= TapiFlags::InstallAPIOption;
    break;
  case TAPICommand::Reexport:
    flags |= TapiFlags::ReexportOption;
    break;
  case TAPICommand::SDKDB:
    flags |= TapiFlags::SDKDBOption;
    break;
  case TAPICommand::APIVerify:
    flags |= TapiFlags::APIVerifyOption;
    break;
  }

  return flags;
}

static std::string getTAPIExecutableDirectory() {
  // Exists solely for the purpose of lookup of the resource path.
  // This just needs to be some symbol in the binary.
  static int staticSymbol;
  auto mainExecutable = sys::fs::getMainExecutable("tapi", &staticSymbol);
  StringRef dir = llvm::sys::path::parent_path(mainExecutable);
  return dir.str();
}

static std::string getClangResourcesPath(FileManager &fm) {
  // The driver detects the builtin header path based on the path of the
  // executable.
  auto tapiDir = getTAPIExecutableDirectory();

  // Compute the path to the resource directory.

  // Try the default tapi path.
  SmallString<PATH_MAX> path(tapiDir);
  llvm::sys::path::append(path, "..", CLANG_INSTALL_LIBDIR_BASENAME, "tapi",
                          TAPI_MAKE_STRING(TAPI_VERSION_MAJOR));
  if (fm.exists(path))
    return path.str().str();

  // Try the default clang path. This is used by check-tapi.
  path = tapiDir;
  llvm::sys::path::append(path, "..", CLANG_INSTALL_LIBDIR_BASENAME, "clang",
                          CLANG_VERSION_MAJOR_STRING);
  if (fm.exists(path))
    return path.str().str();

  return std::string();
}

static std::string getClangExecutablePath() {
  // Try to find clang first in the toolchain. If that fails, then fall-back to
  // the default search PATH.
  auto tapiDir = getTAPIExecutableDirectory();
  auto clangBinary =
      sys::findProgramByName("clang", ArrayRef(StringRef(tapiDir)));
  if (clangBinary.getError())
    clangBinary = sys::findProgramByName("clang");
  if (auto ec = clangBinary.getError())
    return "clang";
  else
    return clangBinary.get();
}

static std::string getTAPIConfigurationFile(FileManager &fm,
                                            StringRef platformName) {
  auto tapiDir = getTAPIExecutableDirectory();
  // Configuration directory is /usr/local/tapi/config.
  SmallString<PATH_MAX> path(tapiDir);
  llvm::sys::path::append(path, "..", "local", "tapi", "config");
  // Configuration file is named platform.conf.
  llvm::sys::path::append(path, platformName + ".conf");

  if (fm.exists(path))
    return path.str().str();

  return std::string();
}

/// Parse JSON input into argument list.
///
/* Expected input format.
 *  { "label" : ["-ClangArg1", "-ClangArg2"] }
 */
///
/// Input is interpreted as "-Xlabel ClangArg1 -XLabel ClangArg2".
static Expected<llvm::opt::InputArgList>
getArgListFromJSON(const StringRef input, llvm::opt::OptTable *optTable,
                   std::vector<std::string> &storage) {
  auto valOrErr = json::parse(input);
  if (!valOrErr)
    return valOrErr.takeError();

  auto *root = valOrErr->getAsObject();
  if (!root)
    return llvm::opt::InputArgList();

  for (const auto &kv : *root) {
    auto *argList = kv.getSecond().getAsArray();
    std::string label = "-X" + kv.getFirst().str();
    if (!argList)
      return make_error<TextAPIError>(TextAPIErrorCode::InvalidInputFormat);
    for (auto arg : *argList) {
      auto argStr = arg.getAsString();
      if (!argStr)
        return make_error<TextAPIError>(TextAPIErrorCode::InvalidInputFormat);
      storage.emplace_back(label);
      storage.emplace_back(*argStr);
    }
  }

  std::vector<const char *> cArgs(storage.size());
  llvm::for_each(
      storage, [&cArgs](std::string &str) { cArgs.emplace_back(str.data()); });

  unsigned missingArgIndex, missingArgCount;
  return optTable->ParseArgs(cArgs, missingArgIndex, missingArgCount);
}


static std::optional<Triple>
parsePlatformAndDeploymentTarget(DiagnosticsEngine &diag, InputArgList &args) {
  // Handle deployment target.
  const std::pair<unsigned, PlatformType> platforms[] = {
      {OPT_mmacos_version_min_EQ, PLATFORM_MACOS},
      {OPT_mios_version_min_EQ, PLATFORM_IOS},
      {OPT_mios_simulator_version_min_EQ, PLATFORM_IOSSIMULATOR},
      {OPT_mtvos_version_min_EQ, PLATFORM_TVOS},
      {OPT_mtvos_simulator_version_min_EQ, PLATFORM_TVOSSIMULATOR},
      {OPT_mwatchos_version_min_EQ, PLATFORM_WATCHOS},
      {OPT_mwatchos_simulator_version_min_EQ, PLATFORM_WATCHOSSIMULATOR},
      {OPT_mbridgeos_version_min_EQ, PLATFORM_BRIDGEOS},
  };

  PlatformType platform = PLATFORM_UNKNOWN;
  std::string osVersion;
  const Arg *first = nullptr;
  for (const auto &target : platforms) {
    auto *arg = args.getLastArg(target.first);
    if (arg == nullptr)
      continue;

    if (first != nullptr) {
      diag.report(clang::diag::err_drv_argument_not_allowed_with)
          << first->getAsString(args) << arg->getAsString(args);
      return std::nullopt;
    }

    first = arg;
    platform = target.second;
    osVersion = arg->getValue();
  }

  if (platform == PLATFORM_UNKNOWN) {
    // If no deployment target was specified on the command line, check for
    // environment defines.

    const std::pair<const char *, PlatformType> platforms[] = {
        {"MACOSX_DEPLOYMENT_TARGET", PLATFORM_MACOS},
        {"IPHONEOS_DEPLOYMENT_TARGET", PLATFORM_IOS},
        {"TVOS_DEPLOYMENT_TARGET", PLATFORM_TVOS},
        {"WATCHOS_DEPLOYMENT_TARGET", PLATFORM_WATCHOS},
        {"BRIDGEOS_DEPLOYMENT_TARGET", PLATFORM_BRIDGEOS},
        {"DRIVERKIT_DEPLOYMENT_TARGET", PLATFORM_DRIVERKIT},
        {"XROS_DEPLOYMENT_TARGET", PLATFORM_XROS},
    };

    const char *first = nullptr;
    for (const auto &target : platforms) {
      auto *env = ::getenv(target.first);
      if (env == nullptr)
        continue;

      if (first != nullptr) {
        diag.report(clang::diag::err_drv_conflicting_deployment_targets)
            << first << target.first;
        return std::nullopt;
      }

      first = target.first;
      platform = target.second;
      osVersion = env;
    }
  }

  if (first)
    diag.report(diag::warn_platform_specific_deployment_target)
        << first->getAsString(args);

  Triple target;
  target.setVendor(Triple::Apple);
  target.setOSName(getOSAndEnvironmentName(platform, osVersion));
  return target;
}

bool Options::processOptionList(DiagnosticsEngine &diag,
                                llvm::opt::InputArgList &args) {
  auto *arg = args.getLastArg(OPT_option_list);
  if (!arg)
    return true;

  const StringRef path = arg->getValue(0);
  auto inputOrErr = fm->getBufferForFile(path);
  if (auto err = inputOrErr.getError()) {
    diag.report(diag::err_cannot_read_file) << path << err.message();
    return false;
  }

  // Backing storage referenced for argument processing.
  std::vector<std::string> storage;

  auto argsOrErr =
      getArgListFromJSON((*inputOrErr)->getBuffer(), table.get(), storage);
  if (auto err = argsOrErr.takeError()) {
    diag.report(diag::err_cannot_read_file) << path << toString(std::move(err));
    return false;
  }

  return processXOptions(diag, *argsOrErr, /*clearOptions=*/false);
}

bool Options::processXOptions(DiagnosticsEngine &diag, InputArgList &args,
                              bool clearOptions) {
  // Preset Xoptions before iterating through arg list.
  if (clearOptions) {
    if (args.hasArgNoClaim(OPT_Xparser))
      frontendOptions.clangExtraArgs.clear();
    if (args.hasArgNoClaim(OPT_Xproject))
      projectLevelOptions = {};
    if (args.hasArgNoClaim(OPT_X))
      frontendOptions.uniqueClangArgs.clear();
  }
  for (auto it = args.begin(), e = args.end(); it != e; ++it) {
    auto *arg = *it;
    // Handle special labels first.
    if (arg->getOption().matches(OPT_Xparser)) {
      processXparserOptions(args, it);
      continue;
    } else if (arg->getOption().matches(OPT_Xarch__)) {
      if (!processXarchOptions(diag, args, it))
        return false;
      continue;
    } else if (arg->getOption().matches(OPT_Xplatform__)) {
      if (!processXplatformOptions(diag, args, it))
        return false;
      continue;
    } else if (arg->getOption().matches(OPT_Xproject)) {
      if (!processXprojectOptions(diag, args, it))
        return false;
      continue;
    } else if (!arg->getOption().matches(OPT_X))
      continue;

    // Handle any user defined labels.
    const std::string label = arg->getValue(0);

    // Ban "public" and "private" labels.
    const auto checkedLabel = StringRef(label).lower();
    if ((checkedLabel == "public") || (checkedLabel == "private")) {
      diag.report(diag::err_invalid_label) << label;
      return false;
    }

    auto nextIt = std::next(it);
    if (nextIt == e) {
      diag.report(clang::diag::err_drv_missing_argument)
          << arg->getAsString(args) << 1;
      return false;
    }

    auto *nextArg = *nextIt;
    switch ((ID)nextArg->getOption().getID()) {
    case OPT_D:
    case OPT_U:
      break;
    default:
      diag.report(clang::diag::err_drv_argument_not_allowed_with)
          << arg->getAsString(args) << nextArg->getAsString(args);
      return false;
    }

    const StringRef argString = nextArg->getSpelling();
    const auto &argValues = nextArg->getValues();
    if (argValues.empty())
      frontendOptions.uniqueClangArgs[label].emplace_back(argString.str());
    else
      for (const auto &value : argValues)
        frontendOptions.uniqueClangArgs[label].emplace_back(
            (argString + value).str());

    arg->claim();
    nextArg->claim();
  }

  return true;
}

void Options::processXparserOptions(llvm::opt::InputArgList &args,
                                    arg_iterator curr) {
  auto *arg = *curr;
  if (arg->isClaimed())
    return;
  frontendOptions.clangExtraArgs.emplace_back(arg->getValue());
  arg->claim();
}

bool Options::processXprojectOptions(DiagnosticsEngine &diag,
                                     llvm::opt::InputArgList &args,
                                     arg_iterator curr) {
  auto *arg = *curr;

  auto nextIt = std::next(curr);
  if (nextIt == args.end()) {
    diag.report(clang::diag::err_drv_missing_argument)
        << arg->getAsString(args) << 1;
    return false;
  }

  auto *nextArg = *nextIt;
  switch ((ID)nextArg->getOption().getID()) {
  case OPT_fobjc_arc:
    projectLevelOptions.useObjectiveCARC = true;
    break;
  case OPT_fmodules:
    projectLevelOptions.enableModules = true;
    break;
  case OPT_fmodules_cache_path:
    projectLevelOptions.moduleCachePath = nextArg->getValue();
    break;
  case OPT_include_:
    projectLevelOptions.prefixHeaders.emplace_back(nextArg->getValue());
    break;
  default:
    diag.report(clang::diag::err_drv_argument_not_allowed_with)
        << arg->getAsString(args) << nextArg->getAsString(args);
    return false;
  }
  arg->claim();
  nextArg->claim();
  return true;
}

bool Options::processXarchOptions(DiagnosticsEngine &diag,
                                  llvm::opt::InputArgList &args,
                                  arg_iterator curr) {
  auto *arg = *curr;
  auto architecture = getArchitectureFromName(arg->getValue(0));
  if (architecture == AK_unknown) {
    diag.report(clang::diag::err_drv_invalid_arch_name)
        << arg->getAsString(args);
    return false;
  }

  auto nextIt = std::next(curr);
  if (nextIt == args.end()) {
    diag.report(clang::diag::err_drv_missing_argument)
        << arg->getAsString(args) << 1;
    return false;
  }
  auto *nextArg = *nextIt;
  switch ((ID)nextArg->getOption().getID()) {
  case OPT_allowable_client:
  case OPT_reexport_install_name:
  case OPT_reexport_l:
  case OPT_reexport_framework:
  case OPT_reexport_library:
  case OPT_rpath:
    break;
  default:
    diag.report(clang::diag::err_drv_argument_not_allowed_with)
        << arg->getAsString(args) << nextArg->getAsString(args);
    return false;
  }

  argToArchMap[nextArg] = architecture;
  arg->claim();

  return true;
}

bool Options::processXplatformOptions(DiagnosticsEngine &diag,
                                      InputArgList &args, arg_iterator curr) {
  auto *arg = *curr;

  auto platform = getPlatformFromName(arg->getValue(0));
  if (platform == PLATFORM_UNKNOWN) {
    diag.report(diag::err_invalid_platform_name) << arg->getAsString(args);
    return false;
  }

  auto nextIt = std::next(curr);
  if (nextIt == args.end()) {
    diag.report(clang::diag::err_drv_missing_argument)
        << arg->getAsString(args) << 1;
    return false;
  }
  auto *nextArg = *nextIt;
  switch ((ID)nextArg->getOption().getID()) {
  case OPT_iframework:
    break;
  default:
    diag.report(clang::diag::err_drv_argument_not_allowed_with)
        << arg->getAsString(args) << nextArg->getAsString(args);
    return false;
  }

  argToPlatformMap[nextArg] = platform;
  arg->claim();

  return true;
}

/// \brief Process driver related options.
bool Options::processDriverOptions(DiagnosticsEngine &diag,
                                   InputArgList &args) {
  // Handle -version.
  if (args.hasArg(OPT_version))
    driverOptions.printVersion = true;

  // Handle help options.
  if (args.hasArg(OPT_help_hidden))
    driverOptions.printHelp = driverOptions.printHelpHidden = true;

  if (args.hasArg(OPT_help))
    driverOptions.printHelp = true;

  // Handle output file.
  SmallString<PATH_MAX> outputPath;
  if (auto *arg = args.getLastArg(OPT_output)) {
    outputPath = arg->getValue();
    if (outputPath != "-")
      fm->makeAbsolutePath(outputPath);
    driverOptions.outputPath = outputPath.str().str();
  }

  // Handle input files.
  if (args.hasArgNoClaim(OPT_INPUT))
    driverOptions.inputs.clear();

  for (const auto &path : args.getAllArgValues(OPT_INPUT)) {
    if (!fm->exists(path)) {
      diag.report(clang::diag::err_drv_no_such_file) << path;
      return false;
    }

    SmallString<PATH_MAX> absolutePath(path);
    fm->makeAbsolutePath(absolutePath);
    driverOptions.inputs.emplace_back(absolutePath.str());
  }

  // Handle vfs overlay paths.
  if (args.hasArgNoClaim(OPT_ivfsoverlay))
    driverOptions.vfsOverlayPaths.clear();

  for (auto *arg : args.filtered(OPT_ivfsoverlay))
    driverOptions.vfsOverlayPaths.emplace_back(arg->getValue());

  if (driverOptions.clangExecutablePath.empty()) {
    driverOptions.clangExecutablePath = getClangExecutablePath();
  }

  return true;
}

/// \brief Process archive related options.
bool Options::processArchiveOptions(DiagnosticsEngine &diag,
                                    InputArgList &args) {
  Arg *lastArg = nullptr;

  // Handle --info.
  if ((lastArg = args.getLastArg(OPT_info)))
    archiveOptions.action = ArchiveAction::ShowInfo;

  // Handle --extract <architecture>
  if (auto *arg = args.getLastArg(OPT_extract)) {
    if (lastArg) {
      diag.report(clang::diag::err_drv_argument_not_allowed_with)
          << lastArg->getAsString(args) << arg->getAsString(args);
      return false;
    }

    auto arch = getArchitectureFromName(arg->getValue());
    if (arch == AK_unknown) {
      diag.report(clang::diag::err_drv_invalid_arch_name) << arg->getValue();
      return false;
    }

    archiveOptions.action = ArchiveAction::ExtractArchitecture;
    archiveOptions.arch = arch;
    lastArg = arg;
  }

  // Handle --remove <architecture>
  if (auto *arg = args.getLastArg(OPT_remove)) {
    if (lastArg) {
      diag.report(clang::diag::err_drv_argument_not_allowed_with)
          << lastArg->getAsString(args) << arg->getAsString(args);
      return false;
    }

    auto arch = getArchitectureFromName(arg->getValue());
    if (arch == AK_unknown) {
      diag.report(clang::diag::err_drv_invalid_arch_name) << arg->getValue();
      return false;
    }

    archiveOptions.action = ArchiveAction::RemoveArchitecture;
    archiveOptions.arch = arch;
    lastArg = arg;
  }

  // Handle --verify-arch <architecture>.
  if (auto *arg = args.getLastArg(OPT_verify_arch)) {
    if (lastArg) {
      diag.report(clang::diag::err_drv_argument_not_allowed_with)
          << lastArg->getAsString(args) << arg->getAsString(args);
      return false;
    }

    auto arch = getArchitectureFromName(arg->getValue());
    if (arch == AK_unknown) {
      diag.report(clang::diag::err_drv_invalid_arch_name) << arg->getValue();
      return false;
    }

    archiveOptions.action = ArchiveAction::VerifyArchitecture;
    archiveOptions.arch = arch;
    lastArg = arg;
  }

  // Handle --merge.
  if (auto *arg = args.getLastArg(OPT_merge)) {
    if (lastArg) {
      diag.report(clang::diag::err_drv_argument_not_allowed_with)
          << lastArg->getAsString(args) << arg->getAsString(args);
      return false;
    }

    archiveOptions.action = ArchiveAction::Merge;
  }

  // Handle --list-symbols.
  if (auto *arg = args.getLastArg(OPT_listSymbols)) {
    if (lastArg) {
      diag.report(clang::diag::err_drv_argument_not_allowed_with)
          << lastArg->getAsString(args) << arg->getAsString(args);
      return false;
    }

    archiveOptions.action = ArchiveAction::ListSymbols;
  }

  // Handle --allow-arch-merges
  if (args.hasArg(OPT_allow_arch_merges))
    archiveOptions.allowArchitectureMerges = true;

  return true;
}

/// \brief Process linker related options.
bool Options::processLinkerOptions(DiagnosticsEngine &diag,
                                   InputArgList &args) {
  // Handle dynamic lib.
  if (args.hasArg(OPT_dynamiclib))
    linkerOptions.isDynamicLibrary = true;

  // Handle install name.
  if (auto *arg = args.getLastArg(OPT_install_name))
    linkerOptions.installName = arg->getValue();

  // Handle current version.
  if (auto *arg = args.getLastArg(OPT_current_version)) {
    auto result = linkerOptions.currentVersion.parse64(arg->getValue());
    if (!result.first) {
      diag.report(diag::err_invalid_current_version) << arg->getValue();
      return false;
    }
    if (result.second)
      diag.report(diag::warn_truncating_current_version) << arg->getValue();
  }

  // Handle compatibility version.
  if (auto *arg = args.getLastArg(OPT_compatibility_version))
    if (!linkerOptions.compatibilityVersion.parse32(arg->getValue())) {
      diag.report(diag::err_invalid_compatibility_version) << arg->getValue();
      return false;
    }

  // Handle allowable clients.
  if (args.hasArgNoClaim(OPT_allowable_client))
    linkerOptions.allowableClients.clear();

  
  ArchitectureSet architectures; 
  for (auto t : frontendOptions.targets)
    architectures.set(mapToArchitecture(t));
  for (auto *arg : args.filtered(OPT_allowable_client)) {
    if (argToArchMap.count(arg))
      linkerOptions.allowableClients.emplace_back(arg->getValue(),
                                                  argToArchMap[arg]);
    else
      linkerOptions.allowableClients.emplace_back(arg->getValue(),
                                                  architectures);
  }

  // Handle reexported libraries.
  if (args.hasArgNoClaim(OPT_reexport_install_name)) {
    linkerOptions.reexportInstallNames.clear();
    diag.report(diag::warn_reexport_install_name);
  }

  for (auto *arg : args.filtered(OPT_reexport_install_name)) {
    if (argToArchMap.count(arg))
      linkerOptions.reexportInstallNames.emplace_back(arg->getValue(),
                                                      argToArchMap[arg]);
    else
      linkerOptions.reexportInstallNames.emplace_back(arg->getValue(),
                                                      architectures);
  }

  if (args.hasArgNoClaim(OPT_reexport_l))
    linkerOptions.reexportedLibraries.clear();

  for (auto *arg : args.filtered(OPT_reexport_l)) {
    if (argToArchMap.count(arg))
      linkerOptions.reexportedLibraries.emplace_back(arg->getValue(),
                                                     argToArchMap[arg]);
    else
      linkerOptions.reexportedLibraries.emplace_back(arg->getValue(),
                                                     architectures);
  }

  if (args.hasArgNoClaim(OPT_reexport_library))
    linkerOptions.reexportedLibraryPaths.clear();

  for (auto *arg : args.filtered(OPT_reexport_library)) {
    if (argToArchMap.count(arg))
      linkerOptions.reexportedLibraryPaths.emplace_back(arg->getValue(),
                                                        argToArchMap[arg]);
    else
      linkerOptions.reexportedLibraryPaths.emplace_back(arg->getValue(),
                                                        architectures);
  }

  if (args.hasArgNoClaim(OPT_reexport_framework))
    linkerOptions.reexportedFrameworks.clear();

  for (auto *arg : args.filtered(OPT_reexport_framework)) {
    if (argToArchMap.count(arg))
      linkerOptions.reexportedFrameworks.emplace_back(arg->getValue(),
                                                      argToArchMap[arg]);
    else
      linkerOptions.reexportedFrameworks.emplace_back(arg->getValue(),
                                                      architectures);
  }

  linkerOptions.isOSLibNotForSharedCache =
      args.hasArg(OPT_not_for_dyld_shared_cache);

  if (args.hasArgNoClaim(OPT_rpath))
    linkerOptions.rpaths.clear();
  for (auto *arg : args.filtered(OPT_rpath)) {
    if (argToArchMap.count(arg))
      linkerOptions.rpaths.emplace_back(arg->getValue(), argToArchMap[arg]);
    else
      linkerOptions.rpaths.emplace_back(arg->getValue(), architectures);
  }

  if (args.hasArgNoClaim(OPT_relinked_library))
    linkerOptions.relinkedLibraries.clear();
  for (auto *arg : args.filtered(OPT_relinked_library))
    linkerOptions.relinkedLibraries.emplace_back(arg->getValue());

  // Handle application extension safe flag.
  if (::getenv("LD_NO_ENCRYPT") != nullptr)
    linkerOptions.isApplicationExtensionSafe = true;

  if (::getenv("LD_APPLICATION_EXTENSION_SAFE") != nullptr)
    linkerOptions.isApplicationExtensionSafe = true;

  linkerOptions.isApplicationExtensionSafe =
      args.hasFlag(OPT_fapplication_extension, OPT_fno_application_extension,
                   /*Default=*/linkerOptions.isApplicationExtensionSafe);
  for (auto *arg : args.filtered(OPT_alias_list))
    linkerOptions.aliasLists.emplace_back(arg->getValue());

  return true;
}

/// \brief Process frontend related options.
bool Options::processFrontendOptions(DiagnosticsEngine &diag,
                                     InputArgList &args) {
  // Handle isysroot.
  if (auto *arg = args.getLastArg(OPT_isysroot)) {
    SmallString<PATH_MAX> path(arg->getValue());
    fm->makeAbsolutePath(path);
    if (!fm->exists(path)) {
      diag.report(diag::err_missing_sysroot) << path;
      return false;
    }
    frontendOptions.isysroot = path.str().str();
  } else if (frontendOptions.isysroot.empty()) {
    // Mirror CLANG and obtain the isysroot from the SDKROOT environment
    // variable, if it wasn't defined by the  command line.
    if (auto *env = ::getenv("SDKROOT")) {
      // Only use the SDKROOT as the default if it is an absolute path, exists,
      // and it is not the root path.
      if (llvm::sys::path::is_absolute(env) && fm->exists(env) &&
          StringRef(env) != "/")
        frontendOptions.isysroot = env;
    }
  }

  // Handle umbrella option.
  if (auto *arg = args.getLastArg(OPT_umbrella))
    frontendOptions.umbrella = arg->getValue();

  // Handle SYSTEM framework paths.
  if (args.hasArgNoClaim((OPT_iframework)))
    frontendOptions.systemFrameworkPaths.clear();

  for (auto *arg : args.filtered(OPT_iframework))
    // Note, when argument should apply for all platforms, no value is
    // passed as this is the common case.
    frontendOptions.systemFrameworkPaths.emplace_back(
        arg->getValue(), argToPlatformMap.count(arg)
                             ? argToPlatformMap[arg]
                             : std::optional<PlatformType>{});

  // Handle framework paths.
  PathSeq frameworkPaths;
  for (auto *arg : args.filtered(OPT_F))
    frameworkPaths.emplace_back(arg->getValue());

  // Handle library paths.
  PathSeq libraryPaths;
  for (auto *arg : args.filtered(OPT_L))
    libraryPaths.emplace_back(arg->getValue());

  /// Construct the search paths for libraries and frameworks.
  // Add default framework/library paths.
  PathSeq defaultLibraryPaths = {"/usr/lib", "/usr/local/lib"};
  PathSeq defaultFrameworkPaths = {"/Library/Frameworks",
                                   "/System/Library/Frameworks"};

  // Add the private framework path, since it is not added by default.
  // Except when invoking SDKDB as it responsible for providing expected paths.
  if ((command != TAPICommand::SDKDB) && (command != TAPICommand::Stubify) &&
      StringRef(frontendOptions.isysroot).contains("Internal"))
    defaultFrameworkPaths.emplace_back("/System/Library/PrivateFrameworks");

  if (!libraryPaths.empty())
    frontendOptions.libraryPaths = libraryPaths;

  for (const auto &libPath : defaultLibraryPaths) {
    SmallString<PATH_MAX> path(frontendOptions.isysroot);
    sys::path::append(path, libPath);
    frontendOptions.libraryPaths.emplace_back(path.str());
  }

  if (!frameworkPaths.empty())
    frontendOptions.frameworkPaths = frameworkPaths;

  for (const auto &frameworkPath : defaultFrameworkPaths) {
    SmallString<PATH_MAX> path(frontendOptions.isysroot);
    sys::path::append(path, frameworkPath);
    frontendOptions.systemFrameworkPaths.emplace_back(
        path.str(), std::optional<PlatformType>{});
  }

  // Do basic error checking first for mixing -target and -arch options.
  auto *argArch = args.getLastArgNoClaim(OPT_arch);
  auto *argTarget = args.getLastArgNoClaim(OPT_target);
  auto *argTargetVariant = args.getLastArgNoClaim(OPT_target_variant);
  if (argArch && (argTarget || argTargetVariant)) {
    diag.report(clang::diag::err_drv_argument_not_allowed_with)
        << argArch->getAsString(args)
        << (argTarget ? argTarget : argTargetVariant)->getAsString(args);
    return false;
  }

  auto *argMinTargetOS = args.getLastArgNoClaim(OPT_mtargetos_EQ);
  if ((argTarget || argTargetVariant) && argMinTargetOS) {
    diag.report(clang::diag::err_drv_cannot_mix_options)
        << argTarget->getAsString(args) << argMinTargetOS->getAsString(args);
    return false;
  }

  // Handle -target first.
  if (argTarget) {
    for (auto *arg : args.filtered(OPT_target)) {
      Triple target(arg->getValue());
      if (target.getVendor() != Triple::Apple) {
        diag.report(diag::err_unsupported_vendor)
            << target.getVendorName() << arg->getAsString(args);
        return false;
      }

      auto archName = target.getArchName();
      if (getArchitectureFromName(archName) == AK_unknown) {
        diag.report(diag::err_unsupported_arch)
            << archName << arg->getAsString(args);
        return false;
      }

      switch (target.getOS()) {
      default:
        diag.report(diag::err_unsupported_os)
            << target.getOSName() << arg->getAsString(args);
        return false;
      case Triple::MacOSX:
      case Triple::IOS:
      case Triple::TvOS:
      case Triple::WatchOS:
      case Triple::DriverKit:
      case Triple::XROS:
        break;
      }

      switch (target.getEnvironment()) {
      default:
        diag.report(diag::err_unsupported_environment)
            << target.getEnvironmentName() << arg->getAsString(args);
        return false;
      case Triple::UnknownEnvironment:
      case Triple::MacABI:
      case Triple::Simulator:
        break;
      }
      frontendOptions.targets.push_back(target);
    }
  } else {

    // Create target without arch to assign later.
    Triple target;
    if (argMinTargetOS) {
      target = Triple(Twine("unknown-apple-") + argMinTargetOS->getValue());
      if (mapToPlatformType(target) == PLATFORM_UNKNOWN) {
        diag.report(clang::diag::err_drv_invalid_os_in_arg)
            << target.getOSName() << argMinTargetOS->getAsString(args);
        return false;
      }
    } else {
      auto result = parsePlatformAndDeploymentTarget(diag, args);
      if (!result.has_value())
        return false;
      target = result.value();
    }

    for (auto *arg : args.filtered(OPT_arch)) {
      auto arch = getArchitectureFromName(arg->getValue());
      if (arch == AK_unknown) {
        diag.report(clang::diag::err_drv_invalid_arch_name) << arg->getValue();
        return false;
      }
      target.setArchName(arg->getValue());
      frontendOptions.targets.push_back(target);
    }
  }

  for (auto *arg : args.filtered(OPT_target_variant)) {
    Triple variant(arg->getValue());
    if (variant.getVendor() != Triple::Apple) {
      diag.report(diag::err_unsupported_vendor)
          << variant.getVendorName() << arg->getAsString(args);
      return false;
    }

    switch (variant.getOS()) {
    default:
      diag.report(diag::err_unsupported_os)
          << variant.getOSName() << arg->getAsString(args);
      return false;
    case Triple::MacOSX:
    case Triple::IOS:
      break;
    }

    switch (variant.getEnvironment()) {
    default:
      diag.report(diag::err_unsupported_environment)
          << variant.getEnvironmentName() << arg->getAsString(args);
      return false;
    case Triple::UnknownEnvironment:
    case Triple::MacABI:
      break;
    }

    // See if there is a matching --target option for this --target-variant
    // option.
    auto it = find_if(frontendOptions.targets, [&](const Triple &target) {
      return (target.getArch() == variant.getArch()) &&
             (target.getVendor() == variant.getVendor());
    });

    if (it == frontendOptions.targets.end()) {
      diag.report(diag::err_no_matching_target) << arg->getAsString(args);
      return false;
    }

    frontendOptions.targetVariants.push_back(variant);
  }


  // Handle language option.
  if (auto *arg = args.getLastArg(OPT_x)) {
    frontendOptions.language =
        StringSwitch<clang::Language>(arg->getValue())
            .Case("c", clang::Language::C)
            .Case("c++", clang::Language::CXX)
            .Case("objective-c", clang::Language::ObjC)
            .Case("objective-c++", clang::Language::ObjCXX)
            .Default(clang::Language::Unknown);

    if (frontendOptions.language == clang::Language::Unknown) {
      diag.report(clang::diag::err_drv_invalid_value)
          << arg->getAsString(args) << arg->getValue();
      return false;
    }
  }

  // Handle ObjC/ObjC++ switch.
  for (auto *arg : args.filtered(OPT_ObjC, OPT_ObjCXX)) {
    if (arg->getOption().matches(OPT_ObjC))
      frontendOptions.language = clang::Language::ObjC;
    else
      frontendOptions.language = clang::Language::ObjCXX;
  }

  // Handle language std.
  if (auto *arg = args.getLastArg(OPT_std_EQ))
    frontendOptions.language_std = arg->getValue();

  // Handle SYSTEM include paths.
  if (args.hasArgNoClaim(OPT_isystem))
    frontendOptions.systemIncludePaths.clear();

  for (auto *arg : args.filtered(OPT_isystem))
    frontendOptions.systemIncludePaths.emplace_back(arg->getValue());

  // Handle AFTER include paths.
  if (args.hasArgNoClaim(OPT_idirafter))
    frontendOptions.afterIncludePaths.clear();

  for (auto *arg : args.filtered(OPT_idirafter))
    frontendOptions.afterIncludePaths.emplace_back(arg->getValue());

  // Handle include paths.
  if (args.hasArgNoClaim(OPT_I))
    frontendOptions.includePaths.clear();

  for (auto *arg : args.filtered(OPT_I))
    frontendOptions.includePaths.emplace_back(arg->getValue());

  // Handle quoted include paths
  if (args.hasArgNoClaim(OPT_iquote))
    frontendOptions.quotedIncludePaths.clear();

  for (auto *arg : args.filtered(OPT_iquote))
    frontendOptions.quotedIncludePaths.emplace_back(arg->getValue());

  // Handle prefix headers.
  if (auto *arg = args.getLastArgNoClaim(OPT_include_))
    if (!arg->isClaimed()) {
      diag.report(diag::err_prefix_header_usage) << arg->getAsString(args);
      return false;
    }

  if (auto *arg = args.getLastArgNoClaim(OPT_include_pch)) {
    diag.report(diag::err_pch) << arg->getAsString(args);
    return false;
  }

  // Add macros from the command line.
  if (args.hasArgNoClaim(OPT_D) || args.hasArgNoClaim(OPT_U))
    frontendOptions.macros.clear();

  for (auto *arg : args.filtered(OPT_D, OPT_U)) {
    if (arg->isClaimed())
      continue;
    if (arg->getOption().matches(OPT_D))
      frontendOptions.macros.emplace_back(arg->getValue(), /*isUndef=*/false);
    else
      frontendOptions.macros.emplace_back(arg->getValue(), /*isUndef=*/true);
  }

  // Handle RTTI generation. Get the last of -fno-rtti and -frtti and set the
  // overwrite.
  if (auto *argRTTI = args.getLastArg(OPT_frtti, OPT_fno_rtti)) {
    if (argRTTI->getOption().getID() == OPT_frtti)
      frontendOptions.useRTTI = true;
    else
      frontendOptions.useNoRTTI = true;
  }

  // Handle visibility.
  if (auto *arg = args.getLastArg(OPT_fvisibility_EQ))
    frontendOptions.visibility = arg->getValue();

  // Handle module related options.
  if (auto *arg = args.getLastArgNoClaim(OPT_fmodules))
    if (!arg->isClaimed())
      frontendOptions.enableModules = true;

  if (auto *arg = args.getLastArgNoClaim(OPT_fmodules_cache_path))
    if (!arg->isClaimed())
      frontendOptions.moduleCachePath = arg->getValue();

  if (args.hasArg(OPT_fmodules_validate_system_headers))
    frontendOptions.validateSystemHeaders = true;

  if (auto *arg = args.getLastArgNoClaim(OPT_product_name))
    frontendOptions.productName = arg->getValue();

  // Handle clang resource path.
  if (frontendOptions.clangResourcePath.empty())
    frontendOptions.clangResourcePath = getClangResourcesPath(*fm);

  // Handle Objective-C ARC
  if (auto *arg = args.getLastArgNoClaim(OPT_fobjc_arc))
    if (!arg->isClaimed())
      frontendOptions.useObjectiveCARC = true;

  if (args.hasArg(OPT_fobjc_weak))
    frontendOptions.useObjectiveCWeakARC = true;

  if (args.hasArg(OPT_verbose))
    frontendOptions.verbose = true;

  return true;
}

/// \brief Process frontend related options.
bool Options::processDiagnosticsOptions(DiagnosticsEngine &diag,
                                        InputArgList &args) {
  // Handle diagnostics file.
  if (auto *arg = args.getLastArg(OPT__serialize_diags))
    diagnosticsOptions.serializeDiagnosticsFile = arg->getValue();

  if (auto *arg = args.getLastArg(OPT_ferror_limit)) {
    if (StringRef(arg->getValue())
            .getAsInteger(10, diagnosticsOptions.errorLimit)) {
      diag.report(clang::diag::err_drv_invalid_int_value)
          << arg->getAsString(args) << arg->getValue();
      return false;
    }
  }

  return true;
}

/// \brief Handle TAPI related options.
bool Options::processTAPIOptions(DiagnosticsEngine &diag, InputArgList &args) {
  // Handle file list.
  if (args.hasArgNoClaim(OPT_file_list_EQ))
    tapiOptions.fileLists.clear();
  for (auto *arg : args.filtered(OPT_file_list_EQ))
    tapiOptions.fileLists.emplace_back(arg->getValue());

  // Handle public/private umbrella header.
  if (auto *arg = args.getLastArg(OPT_public_umbrella_header))
    tapiOptions.publicUmbrellaHeaderPath = arg->getValue();

  if (auto *arg = args.getLastArg(OPT_private_umbrella_header))
    tapiOptions.privateUmbrellaHeaderPath = arg->getValue();

  auto &fm = getFileManager();
  auto addFilePaths = [&fm, &diag, &args](PathSeq &headers,
                                          OptSpecifier optID) {
    for (const auto &path : args.getAllArgValues(optID)) {
      if (fm.isDirectory(path, /*CacheFailure=*/false)) {
        auto result = enumerateHeaderFiles(fm, path);
        if (!result) {
          diag.report(diag::err) << path << toString(result.takeError());
          return false;
        }
        // Sort headers to ensure deterministic behavior.
        sort(*result);
        for (auto &path : *result)
          headers.emplace_back(path);
      } else
        headers.emplace_back(path);
    }

    return true;
  };

  // Handle extra header directories/files.
  if (args.hasArgNoClaim(OPT_extra_public_header))
    tapiOptions.extraPublicHeaders.clear();

  if (!addFilePaths(tapiOptions.extraPublicHeaders, OPT_extra_public_header))
    return false;

  if (args.hasArgNoClaim(OPT_extra_private_header))
    tapiOptions.extraPrivateHeaders.clear();

  if (!addFilePaths(tapiOptions.extraPrivateHeaders, OPT_extra_private_header))
    return false;

  if (args.hasArgNoClaim(OPT_extra_project_header))
    tapiOptions.extraProjectHeaders.clear();

  if (!addFilePaths(tapiOptions.extraProjectHeaders, OPT_extra_project_header))
    return false;

  // Handle excluded header files.
  if (args.hasArgNoClaim(OPT_exclude_public_header))
    tapiOptions.excludePublicHeaders.clear();

  if (!addFilePaths(tapiOptions.excludePublicHeaders,
                    OPT_exclude_public_header))
    return false;

  if (args.hasArgNoClaim(OPT_exclude_private_header))
    tapiOptions.excludePrivateHeaders.clear();

  if (!addFilePaths(tapiOptions.excludePrivateHeaders,
                    OPT_exclude_private_header))
    return false;

  if (args.hasArgNoClaim(OPT_exclude_project_header))
    tapiOptions.excludeProjectHeaders.clear();

  if (!addFilePaths(tapiOptions.excludeProjectHeaders,
                    OPT_exclude_project_header))
    return false;

  // Handle swift symbols files.
  if (args.hasArgNoClaim(OPT_swift_installapi_interface))
    tapiOptions.swiftInstallAPIInterfaces.clear();

  if (!addFilePaths(tapiOptions.swiftInstallAPIInterfaces,
                    OPT_swift_installapi_interface))
    return false;

  // Handle verify swift.
  if (args.hasArg(OPT_verify_swift))
    tapiOptions.verifySwift = true;

  // Handle verify against.
  if (auto *arg = args.getLastArg(OPT_verify_against))
    tapiOptions.verifyAgainst = arg->getValue();

  // Handle verification mode.
  if (auto *arg = args.getLastArg(OPT_verify_mode_EQ)) {
    tapiOptions.verificationMode =
        StringSwitch<VerificationMode>(arg->getValue())
            .Case("ErrorsOnly", VerificationMode::ErrorsOnly)
            .Case("ErrorsAndWarnings", VerificationMode::ErrorsAndWarnings)
            .Case("Pedantic", VerificationMode::Pedantic)
            .Default(VerificationMode::Invalid);

    if (tapiOptions.verificationMode == VerificationMode::Invalid) {
      diag.report(clang::diag::err_drv_invalid_value)
          << arg->getAsString(args) << arg->getValue();
      return false;
    }
  }

  // Check if need to generate extra symbols for code coverage.
  if (args.hasArg(OPT_fprofile_instr_generate))
    tapiOptions.generateCodeCoverageSymbols = true;

  // Handle demangling.
  if (args.hasArg(OPT_demangle))
    tapiOptions.demangle = true;

  if (args.hasArg(OPT_deleteInputFile))
    tapiOptions.deleteInputFile = true;

  if (::getenv("TAPI_DELETE_INPUT_FILE") != nullptr)
    tapiOptions.deleteInputFile = true;

  if (args.hasArg(OPT_inlinePrivateFrameworks))
    tapiOptions.inlinePrivateFrameworks = true;

  if (args.hasArg(OPT_deletePrivateFrameworks))
    tapiOptions.deletePrivateFrameworks = true;
  
  if (args.hasArg(OPT_removeSharedCacheFlag))
    tapiOptions.removeSharedCacheFlag = true;

  if (args.hasArg(OPT_noUUIDs))
    diag.report(diag::warn_no_uuids);

  if (args.hasArg(OPT_setInstallAPI))
    diag.report(diag::warn_installapi_flag);

  if (args.hasArg(OPT_snapshot))
    diag.report(diag::warn_snapshot);

  if (auto *arg = args.getLastArg(OPT_dSYM))
    tapiOptions.dSYM = arg->getValue();

  if (args.hasArg(OPT_t))
    tapiOptions.traceLibraryLocation = true;


  auto parseFileType = [](StringRef fileType) {
    return StringSwitch<FileType>(fileType)
        .Case("tbd-v1", FileType::TBD_V1)
        .Case("tbd-v2", FileType::TBD_V2)
        .Case("tbd-v3", FileType::TBD_V3)
        .Case("tbd-v4", FileType::TBD_V4)
        .Case("tbd-v5", FileType::TBD_V5)
        .Default(FileType::Invalid);
  };

  if (auto *arg = args.getLastArg(OPT_filetype)) {
    tapiOptions.fileType = parseFileType(arg->getValue());
    if (tapiOptions.fileType == FileType::Invalid) {
      diag.report(clang::diag::err_drv_invalid_value)
          << arg->getAsString(args) << arg->getValue();
      return false;
    }
  }

  auto fileTypeEnv = ::getenv("TAPI_OUTPUT_FILE_TYPE");
  if (fileTypeEnv != nullptr) {
    tapiOptions.fileType = parseFileType(fileTypeEnv);
    if (tapiOptions.fileType == FileType::Invalid) {
      diag.report(clang::diag::err_drv_invalid_value)
          << "env TAPI_OUTPUT_FILE_TYPE" << fileTypeEnv;
      return false;
    }
  }

  if (args.hasArg(OPT_dylibs_only))
    tapiOptions.scanAll = false;

  if (args.hasArgNoClaim(OPT_inferIncludePaths) ||
      args.hasArgNoClaim(OPT_noInferIncludePaths))
    tapiOptions.inferIncludePaths =
        args.hasFlag(OPT_inferIncludePaths, OPT_noInferIncludePaths, true);

  if (auto *arg = args.getLastArg(OPT_print_after_EQ))
    tapiOptions.printAfter = arg->getValue();

  tapiOptions.verifyAPI = args.hasFlag(OPT_verify_api, OPT_no_verify_api,
                                       /*Default=*/tapiOptions.verifyAPI);

  tapiOptions.verifyAPISkipExternalHeaders =
      args.hasFlag(OPT_verify_api_skip_external_headers,
                   OPT_no_verify_api_skip_external_headers,
                   /*Default=*/tapiOptions.verifyAPISkipExternalHeaders);

  if (args.hasArg(OPT_verify_api_error_as_warning) ||
      sys::Process::GetEnv("TAPI_API_VERIFY_ERROR_AS_WARNING"))
    tapiOptions.verifyAPIErrorAsWarning = true;

  tapiOptions.verifyAPIAllowlist =
      getTAPIConfigurationFile(getFileManager(), "EquivalentTypes");

  // store all arguments to -isysroot.
  tapiOptions.allSysroots = args.getAllArgValues(OPT_isysroot);

  tapiOptions.isBnI = inBnIEnvironment();

  if (auto *arg = args.getLastArg(OPT_sdkdb_output_dir))
    tapiOptions.sdkdbOutputPath = arg->getValue();
  else if (auto path = sys::Process::GetEnv("XBS_TAPI_SDKDB_OUTPUT_PATH"))
    // If the XBS environment is set, turn on SDKDB output as well.
    tapiOptions.sdkdbOutputPath = *path;
  else if (tapiOptions.isBnI && !tapiOptions.verifyAgainst.empty()) {
    // Directly infer output path in XBS. When in B&I environment and
    // --verify-against is used, infer the output path from LD_TRACE_FILE.
    auto ldTraceFile = sys::Process::GetEnv("LD_TRACE_FILE");
    if (ldTraceFile) {
      SmallString<PATH_MAX> sdkdbOutputPath(
          sys::path::parent_path(*ldTraceFile));
      sys::path::append(sdkdbOutputPath, "SDKDB");
      tapiOptions.sdkdbOutputPath = sdkdbOutputPath.str().str();
    }
  }

  // Handle previously ignored, but invalid options.
  for (auto *arg :
       args.filtered(OPT_log, OPT_fvisibility_inlines, OPT_l, OPT_framework,
                     OPT_fnortt, OPT_fno_exceptions)) {
    if (arg->isClaimed())
      continue;
    diag.report(diag::warn_ignored_option) << arg->getAsString(args);
  }

  return true;
}

/// Handle SDKDB options.
bool Options::processSDKDBOptions(DiagnosticsEngine &diag, InputArgList &args) {
  // Handle disabling of header scanning.
  sdkdbOptions.scanPublicHeaders =
      args.hasFlag(OPT_public_headers, OPT_no_public_headers, true);

  // no private headers by default.
  sdkdbOptions.scanPrivateHeaders =
      args.hasFlag(OPT_private_headers, OPT_no_private_headers, false);

  if (!sdkdbOptions.scanPublicHeaders && !sdkdbOptions.scanPrivateHeaders) {
    diag.report(diag::err_parsing_disabled);
    return false;
  }

  // Handle configuration file.
  if (auto *arg = args.getLastArg(OPT_config_file))
    sdkdbOptions.configurationFile = arg->getValue();

  // Handle diagnostics file.
  if (auto *arg = args.getLastArg(OPT_diagnostics_file))
    sdkdbOptions.diagnosticsFile = arg->getValue();
  else if (tapiOptions.isBnI && driverOptions.outputPath != "-") {
    SmallString<PATH_MAX> diagPath(driverOptions.outputPath);
    sys::path::append(diagPath, "diag.plist");
    sdkdbOptions.diagnosticsFile = diagPath.c_str();
  }

  sdkdbOptions.runtimeRoot = args.getLastArgValue(OPT_runtime_root).str();
  sdkdbOptions.sdkContentRoot =
      args.getLastArgValue(OPT_sdk_content_root).str();
  sdkdbOptions.publicSDKContentRoot =
      args.getLastArgValue(OPT_public_sdk_content_root).str();

  // Get partial sdkdb filelist.
  sdkdbOptions.partialSDKDBFileList =
      args.getLastArgValue(OPT_partial_sdkdb_list).str();

  if (auto *arg = args.getLastArg(OPT_installapi_sdkdb_path))
    sdkdbOptions.installAPISDKDBDirectory = arg->getValue();
  else if (driverOptions.outputPath != "-")
    // if not set, default to output directory.
    sdkdbOptions.installAPISDKDBDirectory = driverOptions.outputPath;

  // Handle SDKDB action, default to full.
  if (auto *arg = args.getLastArg(OPT_sdkdb_action))
    sdkdbOptions.action = StringSwitch<SDKDBAction>(arg->getValue())
                              .Case("all", SDKDBAll)
                              .Case("scan-interface", SDKDBInterfaceScan)
                              .Case("gen-public", SDKDBPublicGen)
                              .Case("gen-private", SDKDBPrivateGen)
                              .Default(SDKDBInterfaceScan);

  return true;
}

static IntrusiveRefCntPtr<llvm::vfs::FileSystem>
createVFSFromYAML(DiagnosticsEngine &diag,
                  IntrusiveRefCntPtr<llvm::vfs::FileSystem> baseFS,
                  PathSeq vfsOverlayPaths) {
  if (vfsOverlayPaths.empty())
    return baseFS;

  IntrusiveRefCntPtr<llvm::vfs::FileSystem> result = baseFS;
  // earlier vfs files are on the bottom
  for (const auto &path : vfsOverlayPaths) {
    auto buffer = result->getBufferForFile(path);
    if (!buffer) {
      diag.report(diag::err_missing_vfs_overlay_file) << path;
      continue;
    }

    IntrusiveRefCntPtr<llvm::vfs::FileSystem> fs = llvm::vfs::getVFSFromYAML(
        std::move(buffer.get()), /*DiagHandler=*/nullptr, path,
        /*DiagContext=*/nullptr, result);
    if (!fs) {
      diag.report(diag::err_invalid_vfs_overlay) << path;
      continue;
    }

    result = fs;
  }

  return result;
}

Options::Options(DiagnosticsEngine &diag, ArrayRef<const char *> argString) {
  // Create the default file manager for all file operations.
  fm = new FileManager(clang::FileSystemOptions());

  table.reset(createDriverOptTable());

  // Program name
  programName = sys::path::stem(argString.front()).str();
  argString = argString.slice(1);

  // Show the umbrella help when no command was specified and no other
  // arguments were passed to tapi.
  if (argString.empty()) {
    driverOptions.printHelp = true;
    return;
  }

  command = getTAPICommand(argString.front());
  if (command != TAPICommand::Driver)
    argString = argString.slice(1);
  auto args = parseArgString(diag, argString, table.get(),
                             getIncludeOptionFlagMasks(command), 0);

  if (diag.hasErrorOccurred())
    return;

  // This has to happen before all other option processing.
  if (!processXOptions(diag, args))
    return;

  if (!processOptionList(diag, args))
    return;

  if (!processDriverOptions(diag, args))
    return;

  {
    auto fs = createVFSFromYAML(diag, &fm->getVirtualFileSystem(),
                                driverOptions.vfsOverlayPaths);
    fm->setVirtualFileSystem(fs);
  }

  if (!processArchiveOptions(diag, args))
    return;

  if (!processFrontendOptions(diag, args))
    return;

  if (!processLinkerOptions(diag, args))
    return;

  if (!processDiagnosticsOptions(diag, args))
    return;

  if (!processTAPIOptions(diag, args))
    return;

  if (!processSDKDBOptions(diag, args))
    return;
}

/// \brief Print umbrella help for tapi.
static void printDriverHelp(bool hidden = false) {
  outs() << "OVERVIEW: " << toolName
         << "\n\n"
            "USAGE: tapi [--version][--help]\n"
            "       tapi <command> [<args>]\n\n"
            "Commands:\n"
            "  archive     Merge or thin text-based stub files\n"
            "  stubify     Create a text-based stub file from a library\n"
            "  installapi  Create a text-based stub file by scanning the "
            "header files\n"
            "  reexport    Create a linker reexport file by scanning the "
            "header files\n"
            "\n";

  if (hidden) {
    outs() << "Experimental Commands:\n"
              "  sdkdb         Generate SDKDB from SDKContent\n"
              "  api-verify    Compare interfaces between frameworks\n"
              "\n";
  }
  outs()
      << "See 'tapi <command> --help' to read more about a specific command.\n";
}

/// \brief Print InstallAPI hidden help.
static void printInstallAPIHiddenHelp() {
  outs()
      << "\n\nRare InstallAPI Options:\n\n"
         "  -X<label> <arg>         Parse input headers with additional "
         "<arg>.\n"
         "The pass is uniquely identified with <label>. "
         "Each unique and user-defined <label> multiples the number of passes "
         "tapi parses over header input. "
         "The <arg> is respected for each <label> pass and is mutually "
         "exclusive to other labels.\n"
         "Only -D<macro> and -U<macro> are respected with user-defined labels."
         "\n\n\n"
         "  -Xproject <arg>         Add <arg> when parsing project "
         "header input.\n"
         "Only -fmodules, -fobjc-arc and -include are respected with -Xproject."
         "\n\n\n"
         "  -Xarch_<arch> <arg>     Apply <arg> for <arch> slice in "
         "output.\n"
         "Only -allowable-client, -reexport-l, -reexport_framework and -rpath "
         "are respected with -Xarch."
         "\n\n\n"
         "  -Xplatform_<platform> <arg>\n"
         "                          Add <arg> when parsing over header "
         "input for <platform>.\n"
         "Only -iframework is respected with -Xplatform."
         "\n\n\n"
         "  -Xparser <arg>          Add <arg> when parsing over header "
         "input.\n"
         "Any clang parsing options can be applied with -Xparser."
         "\n\n\n"
         "  -optionlist <path>      Read <path> that contains X<label> "
         "arguments to parse.\n"
         "The <path> should be to a json file holding any of the above "
         "-X<label> options.\n\n"
         "Expected Format:\n"
         "   {\n"
         "      \"label\" : [\"-clangArg1\", \"-clangArg2\"],\n"
         "   }\n\n"
         "Example Usage:\n"
         "   {\n"
         "      \"DarwinMacros\" : [\"-DApple=1\", \"-DUnix\"],\n"
         "   }\n"
         "This is equivalent to 'tapi installapi -XDarwinMacros -DApple=1 "
         "-XDarwinMacros -DUnix'\n";

  return;
}

void Options::printHelp() const {
  if (command == TAPICommand::Driver) {
    printDriverHelp(driverOptions.printHelpHidden);
    return;
  }

  table->printHelp(
      outs(),
      (programName + " " + getNameFromTAPICommand(command)).str().c_str(),
      toolName, /*FlagsToInclude=*/getIncludeOptionFlagMasks(command),
      /*FlagsToExclude=*/0, /*ShowAllAliases=*/false);
  if (command == TAPICommand::InstallAPI && driverOptions.printHelpHidden)
    printInstallAPIHiddenHelp();
}

TAPI_NAMESPACE_INTERNAL_END
