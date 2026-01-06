//===- lib/Driver/SDKDBDriver.cpp - TAPI SDKDB Driver -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the SDKDB driver for the tapi tool.
///
//===----------------------------------------------------------------------===//

#include "tapi/APIVerifier/APIVerifier.h"
#include "tapi/Core/API.h"
#include "tapi/Core/APIJSONSerializer.h"
#include "tapi/Core/Context.h"
#include "tapi/Core/FileSystem.h"
#include "tapi/Core/Framework.h"
#include "tapi/Core/HeaderFile.h"
#include "tapi/Core/MachOReader.h"
#include "tapi/Core/Path.h"
#include "tapi/Core/Registry.h"
#include "tapi/Core/Utils.h"
#include "tapi/Defines.h"
#include "tapi/Diagnostics/Diagnostics.h"
#include "tapi/Driver/Configuration.h"
#include "tapi/Driver/ConfigurationFileReader.h"
#include "tapi/Driver/DirectoryScanner.h"
#include "tapi/Driver/Driver.h"
#include "tapi/Driver/DriverUtils.h"
#include "tapi/Driver/HeaderGlob.h"
#include "tapi/Driver/Options.h"
#include "tapi/Frontend/Frontend.h"
#include "tapi/SDKDB/PartialSDKDB.h"
#include "tapi/SDKDB/SDKDB.h"
#include "clang/Basic/DarwinSDKInfo.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FileUtilities.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/VirtualFileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TextAPI/ArchitectureSet.h"
#include <system_error>

using namespace llvm;
using namespace llvm::MachO;
using namespace clang;

TAPI_NAMESPACE_INTERNAL_BEGIN

// Scan Driver Context.
namespace sdkdb {

class APIPromoter : public APIMutator {
public:
  APIPromoter() = default;

  void visitGlobal(GlobalRecord &record) override {
    // If this a promoted APISet, make all exported symbols to be public.
    if (record.isExported())
      record.access = APIAccess::Public;
  }

  void visitObjCInterface(ObjCInterfaceRecord &record) override {
    // If this a promoted APISet, make all exported objc classes to be public.
    if (record.isExported()) {
      record.access = APIAccess::Public;
      updateObjCContainerRecordToPublic(record);
    }
  }

  void visitObjCCategory(ObjCCategoryRecord &record) override {
    record.access = APIAccess::Public;
    updateObjCContainerRecordToPublic(record);
  }

  void updateObjCContainerRecordToPublic(ObjCContainerRecord &record) {
    for (auto *method : record.methods)
      method->access = APIAccess::Public;
  }
};

class Context : public tapi::internal::Context {
public:
  // Make the context not copyable.
  Context(const Context &) = delete;
  Context &operator=(const Context &) = delete;

  Configuration config;
  Registry registry;
  std::vector<API> publicBinaryResults;
  std::vector<API> internalBinaryResults;
  std::vector<FrontendContext> publicSDKResults;
  std::vector<FrontendContext> internalSDKResults;
  std::vector<API> extraPublicSDKResults;
  std::vector<API> extraInternalSDKResults;
  DirectoryScanner::FileMap publicVFSOverlay;
  DirectoryScanner::FileMap internalVFSOverlay;
  SDKDBAction action;
  std::string outputPath;
  std::string installAPISDKDBPath;
  std::string internalSDKPath;
  std::string publicSDKPath;
  std::string partialSDKDBFilelist;
  std::string diagnosticsFile;
  PathSeq systemIncludePaths;
  PathSeq systemFrameworkPaths;
  PathSeq afterIncludePaths;
  bool verbose;
  PlatformType platform{PLATFORM_UNKNOWN};
  std::string version;
  bool verifyAPI;
  bool verifyAPISkipExternalHeaders;
  std::string verifyAllowlistFileName;
  std::unique_ptr<MemoryBuffer> verifyAllowlist;
  SmallString<PATH_MAX> moduleCachePath;

  std::string projectName;
  bool hasSDKDBError = false;
  bool hasWrittenPartialOutput = false;

  std::optional<DarwinSDKInfo> sdkInfo;

  Context(Options &opt, DiagnosticsEngine &diag)
      : tapi::internal::Context(opt, diag), config(*this) {
    registry.addYAMLReaders();
    outputPath = opt.driverOptions.outputPath;
    installAPISDKDBPath = opt.sdkdbOptions.installAPISDKDBDirectory;
    internalSDKPath = opt.sdkdbOptions.sdkContentRoot;
    publicSDKPath = opt.sdkdbOptions.publicSDKContentRoot;
    partialSDKDBFilelist = opt.sdkdbOptions.partialSDKDBFileList;
    action = opt.sdkdbOptions.action;
    diagnosticsFile = opt.sdkdbOptions.diagnosticsFile;
    verbose = opt.frontendOptions.verbose;
    verifyAPI = opt.tapiOptions.verifyAPI;
    verifyAPISkipExternalHeaders = opt.tapiOptions.verifyAPISkipExternalHeaders;

    if (!opt.tapiOptions.verifyAPIAllowlist.empty()) {
      verifyAllowlistFileName = opt.tapiOptions.verifyAPIAllowlist;
      auto inputBuf =
          MemoryBuffer::getFile(opt.tapiOptions.verifyAPIAllowlist);
      // load the file if it can be opened, otherwise, just ignore.
      if (inputBuf)
        verifyAllowlist = std::move(*inputBuf);
    }

    afterIncludePaths = opt.frontendOptions.afterIncludePaths;
    systemIncludePaths = opt.frontendOptions.systemIncludePaths;
    systemFrameworkPaths =
        getAllPaths(opt.frontendOptions.systemFrameworkPaths);
    // set project name.
    std::optional<std::string> project;
    if (auto srcroot = llvm::sys::Process::GetEnv("SRCROOT"))
        project = llvm::sys::path::stem(*srcroot);
    if (!project || project->empty())
      return;
    projectName = *project;
    config.setProjectName(projectName);
  }

  ~Context() {
    sys::fs::remove_directories(moduleCachePath);
  }

  bool hasSDKInfo() {
    if (sdkInfo)
      return true;
    const auto sysRoot = config.getSysRoot();
    if (!StringRef(sysRoot).endswith(".sdk"))
      return false;
    auto sdkInfoOrErr = parseDarwinSDKInfo(
        getFileManager().getVirtualFileSystem(), std::move(sysRoot));
    if (auto err = sdkInfoOrErr.takeError()) {
      consumeError(std::move(err));
      return false;
    }
    sdkInfo = *sdkInfoOrErr;
    return true;
  }

  Error scanBinaryFile(StringRef path, std::vector<Triple> &triples,
                       bool isPublic) {
    if (!_fm->exists(path))
      return make_error<StringError>(
          "binary file doesn't exist", inconvertibleErrorCode());

    auto bufferOrErr = _fm->getBufferForFile(path);
    if (auto ec = bufferOrErr.getError())
      return errorCodeToError(ec);

    MachOParseOption option;
    option.arches = config.getArchitectures();
    // Do not include undefined (external linkage) symbols in MachO.
    option.parseUndefined = false;
    auto results = readMachOFile(bufferOrErr->get()->getMemBufferRef(), option);
    if (!results)
      return results.takeError();

    for (auto &result : *results) {
      const auto &target = result.second->getTriple();
      if (std::find(triples.begin(), triples.end(), target) ==
          std::end(triples))
        triples.push_back(target);

      // Update all interfaces to public if configuration suggests.
      if (config.isPromotedToPublicDylib(
              result.second->getBinaryInfo().installName)) {
        APIPromoter promoter;
        result.second->visit(promoter);
      }

      if (isPublic)
        publicBinaryResults.emplace_back(std::move(*result.second));
      else
        internalBinaryResults.emplace_back(std::move(*result.second));
    }

    return Error::success();
  }

  Error performOutput(StringRef path,
                      const std::function<Error(raw_ostream &)> &func) const {
    if (outputPath.empty())
      return make_error<StringError>(
          "output path is empty",
          std::make_error_code(std::errc::invalid_argument));

    // if outputPath is "-" redirect all to stdout.
    if (outputPath == "-")
      return func(outs());

    // Otherwise, outputPath is a directory.
    std::error_code err;
    if (!sys::fs::exists(outputPath)) {
      if (auto err = sys::fs::create_directory(outputPath))
        return errorCodeToError(err);
    } else if (!sys::fs::is_directory(outputPath))
      return make_error<StringError>(
          "output path is not a directory",
          std::make_error_code(std::errc::invalid_argument));
    // create output file stream.
    SmallString<PATH_MAX> output(outputPath);
    sys::path::append(output, path);
    raw_fd_ostream os(output, err);
    if (err)
      return errorCodeToError(err);
    return func(os);
  }

  Expected<StringRef> getOrCreateModuleCache() {
    // if pass on commandline, use the one on commandline.
    StringRef path = config.getCommandlineConfig().moduleCachePath;
    if (!path.empty())
      return path;

    // use the one already created.
    if (!moduleCachePath.empty())
      return moduleCachePath;

    // create temporary directory for module cache.
    auto ec = sys::fs::createUniqueDirectory("ModuleCache", moduleCachePath);
    if (ec)
      return errorCodeToError(ec);

    return moduleCachePath;
  }
};

// Helper to update APILoc to remove Root path.
class APILocUpdater : public APIMutator {
public:
  APILocUpdater(StringRef rootPath, API &api)
      : root(rootPath.str()), api(api) {}

  void visitGlobal(GlobalRecord &record) override {
    updateAPILoc(record);
  }

  void visitEnum(EnumRecord &record) override {
    updateAPILoc(record);
    for (auto *constant : record.constants)
      updateAPILoc(*constant);
  }

  void visitObjCInterface(ObjCInterfaceRecord &record) override {
    updateAPILoc(record);
    updateContainer(record);
  }

  void visitObjCCategory(ObjCCategoryRecord &record) override {
    updateAPILoc(record);
    updateContainer(record);
  }

  void visitObjCProtocol(ObjCProtocolRecord &record) override {
    updateAPILoc(record);
    updateContainer(record);
  }

  void visitTypeDef(TypedefRecord &record) override { updateAPILoc(record); }

private:
  void updateAPILoc(APIRecord &record) {
    auto path = record.loc.getFilename();
    if (path.startswith(root)) {
      SmallString<PATH_MAX> newPath(path);
      sys::path::replace_path_prefix(newPath, root, "");
      auto filename = api.copyString(newPath);
      record.loc =
          APILoc(filename, record.loc.getLine(), record.loc.getColumn());
    }
  }

  void updateContainer(ObjCContainerRecord &record) {
    for (auto *method : record.methods)
      updateAPILoc(*method);
    for (auto *prop : record.properties)
      updateAPILoc(*prop);
    for (auto *ivar : record.ivars)
      updateAPILoc(*ivar);
  }
  std::string root;
  API &api;
};

} // namespace sdkdb

namespace {

class SwiftSDKDBError : public llvm::ErrorInfo<SwiftSDKDBError> {
public:
  SwiftSDKDBError(StringRef path, Twine errorMsg)
      : path(path.str()), msg(errorMsg.str()) {}

  void log(llvm::raw_ostream &os) const override {
    os << "failed to read Swift SDKDB " << path << ": " << msg << "\n";
  }

  std::error_code convertToErrorCode() const override {
    return llvm::inconvertibleErrorCode();
  }

  // Used by ErrorInfo::classID.
  static char ID;

private:
  std::string path;
  std::string msg;
};

char SwiftSDKDBError::ID = 0;

static Error makeSwiftSDKDBError(StringRef path, Error err) {
  return make_error<SwiftSDKDBError>(sys::path::filename(path),
                                     toString(std::move(err)));
}

static Error makeSwiftSDKDBError(StringRef path, std::error_code ec) {
  return makeSwiftSDKDBError(path, errorCodeToError(ec));
}

static Error makeSwiftSDKDBError(StringRef path, Twine errorMsg) {
  return make_error<SwiftSDKDBError>(path, errorMsg);
}

} // anonymous namespace

static void inferTriplesFromEnvironment(sdkdb::Context &context,
                                        const Framework &framework,
                                        std::vector<Triple> &triples) {
  if (context.platform == PLATFORM_UNKNOWN)
    return;

  auto archStr = sys::Process::GetEnv("RC_ARCHS");
  if (!archStr)
    return;

  bool isMacCatalyst =
      context.config.isiOSMacProject() || framework.isMacCatalyst();
  bool isDriverKit = context.config.isDriverKitProject() ||
                     framework.isDriverKit() ||
                     sys::Process::GetEnv("DRIVERKIT").value_or("") == "1";

  const bool isZippered = context.config.isZipperedProject();
  std::string iOSMacVersion;
  if (isZippered || isMacCatalyst) {
    VersionTuple envVersion;
    if (context.hasSDKInfo() && !envVersion.tryParse(context.version)) {
      const auto *mapping = context.sdkInfo->getVersionMapping(
          DarwinSDKInfo::OSEnvPair::macOStoMacCatalystPair());
      if (auto version = mapping->map(envVersion, VersionTuple(), std::nullopt))
        iOSMacVersion = version->getAsString();
    } else
      iOSMacVersion = "13.1";
  }

  SmallVector<StringRef, 2> archs;
  StringRef(*archStr).split(archs, ' ');
  for (auto &archName : archs) {
    auto arch = getArchitectureFromName(archName);
    if (!context.config.getArchitectures().contains(arch))
      continue;
    // FIXME: catalyst and driverkit versions are hard coded.
    if (isMacCatalyst)
      triples.emplace_back(getArchitectureName(arch), "apple",
                           getOSAndEnvironmentName(PLATFORM_IOS, iOSMacVersion),
                           "macabi");
    else if (isZippered) {
      triples.emplace_back(getArchitectureName(arch), "apple",
                           getOSAndEnvironmentName(PLATFORM_IOS, iOSMacVersion),
                           "macabi");
      triples.emplace_back(
          getArchitectureName(arch), "apple",
          getOSAndEnvironmentName(context.platform, context.version));

    } else if (isDriverKit)
      triples.emplace_back(getArchitectureName(arch), "apple",
                           getOSAndEnvironmentName(PLATFORM_DRIVERKIT, "19.0"));
    else if (getPlatformName(context.platform).endswith("Simulator"))
      triples.emplace_back(
          getArchitectureName(arch), "apple",
          getOSAndEnvironmentName(context.platform, context.version),
          "simulator");
    else
      triples.emplace_back(
          getArchitectureName(arch), "apple",
          getOSAndEnvironmentName(context.platform, context.version));
  }
}

static bool computeFrontendResultFromFramework(
    sdkdb::Context &context, const Framework &framework,
    std::vector<Triple> &triples,
    // isPublic is used to indicate scanning the public SDK content root.
    // it prefers the public SDK root and sets the public SDK overlay.
    // publicOnly only controls the type of headers for scanning and skips
    // non-public headers.
    bool isPublic, bool publicOnly = false) {
  // bail out of there is no triples.
  if (triples.empty())
    return true;

  auto &fm = context.getFileManager();
  auto &diag = context.getDiag();
  auto job = std::make_unique<FrontendJob>();

  auto rootPath = (isPublic && !context.publicSDKPath.empty())
                      ? context.publicSDKPath
                      : context.internalSDKPath;
  auto frameworkPath = framework.getPath();
  SmallString<PATH_MAX> basePath(rootPath);
  sys::path::append(basePath, frameworkPath);
  job->vfs = &fm.getVirtualFileSystem();
  job->language = context.config.getLanguage(frameworkPath);
  job->language_std = context.config.getLanguageStd();
  job->overwriteRTTI = context.config.getCommandlineConfig().useRTTI;
  job->overwriteNoRTTI = context.config.getCommandlineConfig().useNoRTTI;
  job->visibility = context.config.getCommandlineConfig().visibility;
  job->isysroot = context.config.getSysRoot();
  job->macros = context.config.getMacros(frameworkPath);
  job->includePaths = context.config.getIncludePaths(frameworkPath);
  job->frameworkPaths = context.config.getFrameworkPaths(frameworkPath);
  job->clangExtraArgs = context.config.getClangExtraArgs(frameworkPath);
  job->enableModules = context.config.getCommandlineConfig().enableModules;
  job->moduleCachePath = context.config.getCommandlineConfig().moduleCachePath;
  job->validateSystemHeaders =
      context.config.getCommandlineConfig().validateSystemHeaders;
  job->clangResourcePath =
      context.config.getCommandlineConfig().clangResourcePath;
  job->verbose = context.verbose;

  if (!context.diagnosticsFile.empty()) {
    job->clangExtraArgs.emplace_back("-Xclang");
    job->clangExtraArgs.emplace_back("-diagnostic-log-file");
    job->clangExtraArgs.emplace_back("-Xclang");
    job->clangExtraArgs.emplace_back(context.diagnosticsFile);
  }

  HeaderSeq preIncludes;
  for (auto &header : context.config.getPreIncludedHeaders(
           frameworkPath, HeaderType::Public)) {
    preIncludes.emplace_back(header, HeaderType::Public);
    preIncludes.back().isExtra = true;
    preIncludes.back().isPreInclude = true;
  }

  for (auto &header : context.config.getPreIncludedHeaders(
           frameworkPath, HeaderType::Private)) {
    preIncludes.emplace_back(header, HeaderType::Private);
    preIncludes.back().isExtra = true;
    preIncludes.back().isPreInclude = true;
  }

  // Create a sorted list of framework headers.
  HeaderSeq headerFiles;
  for (const auto &header : framework._headerFiles) {
    if (!fm.exists(header.fullPath))
      diag.report(diag::warn_no_such_header_file)
          << (header.type == HeaderType::Private) << header.fullPath;
    headerFiles.emplace_back(header);
  }

  auto sysroot = context.config.getSysRoot();
  findAndAddHeaderFiles(
      headerFiles, fm, diag,
      context.config.getExtraHeaders(frameworkPath, HeaderType::Public),
      HeaderType::Public, sysroot, basePath);

  findAndAddHeaderFiles(
      headerFiles, fm, diag,
      context.config.getExtraHeaders(frameworkPath, HeaderType::Private),
      HeaderType::Private, sysroot, basePath);

  // Create the excluded headers list.
  std::set<const FileEntry *> excludeHeaderFiles;
  std::vector<std::unique_ptr<HeaderGlob>> excludeHeaderGlobs;
  auto parseGlobs = [&](HeaderType type) {
    for (const auto &str :
         context.config.getExcludedHeaders(frameworkPath, type)) {
      auto glob = HeaderGlob::create(str, type);
      if (glob)
        excludeHeaderGlobs.emplace_back(std::move(glob.get()));
      else {
        consumeError(glob.takeError());
        if (auto file = fm.getFile(str))
          excludeHeaderFiles.emplace(*file);
        else {
          diag.report(diag::warn_no_such_header_file)
              << (type == HeaderType::Private) << str;
        }
      }
    }
    return true;
  };

  if (!parseGlobs(HeaderType::Public))
    return false;

  if (!parseGlobs(HeaderType::Private))
    return false;

  // A builtin list of header globs that we should always exclude and might come
  // from multiple projects. The list is applied to every project so these globs
  // are not checked for if they have matched any header, to suppress excessive
  // warnings for projects that don't install the headers.
  const std::unique_ptr<HeaderGlob> builtinExcludeHeaderGlobs[]{
      // Exclude `**/usr/[local/]include/_modules/**` headers, which are only
      // used to build modules and should not be directly included.
      llvm::cantFail(
          HeaderGlob::create("**/usr/include/_modules/**", HeaderType::Public)),
      llvm::cantFail(HeaderGlob::create("**/usr/local/include/_modules/**",
                                        HeaderType::Private)),
  };

  for (auto &header : headerFiles) {
    for (auto &glob : builtinExcludeHeaderGlobs)
      if (glob->match(header))
        header.isExcluded = true;

    for (auto &glob : excludeHeaderGlobs)
      if (glob->match(header))
        header.isExcluded = true;
  }

  if (!excludeHeaderFiles.empty()) {
    for (auto &header : headerFiles) {
      auto file = fm.getFile(header.fullPath);
      if (!file)
        continue;
      if (excludeHeaderFiles.count(*file))
        header.isExcluded = true;
    }
  }

  for (const auto &glob : excludeHeaderGlobs)
    if (!glob->didMatch())
      diag.report(diag::warn_glob_did_not_match) << glob->str();

  // Exclude all the headers if the framework is in a special path with
  // the name of architectures.
  static const StringSet<> specialDirectoryNames = {
      "arm", "arm64", "i386", "x86_64", "ppc", "machine"};
  if (specialDirectoryNames.find(sys::path::filename(framework.getPath())) !=
      specialDirectoryNames.end()) {
    for (auto &header : headerFiles)
      header.isExcluded = true;
  }

  // Check if the framework has an umbrella header and move that to the
  // beginning.
  auto matchAndMarkUmbrella = [](HeaderSeq &array, Regex &regex,
                                 HeaderType type) -> bool {
    auto it = find_if(array, [&regex, type](const HeaderFile &header) {
      return (header.type == type) && regex.match(header.fullPath);
    });

    if (it == array.end())
      return false;

    it->isUmbrellaHeader = true;
    return true;
  };
  auto publicUmbrellaHeaderPath =
      context.config.getUmbrellaHeader(frameworkPath, HeaderType::Public);
  if (!publicUmbrellaHeaderPath.empty()) {
    auto escapedString = Regex::escape(publicUmbrellaHeaderPath);
    Regex umbrellaRegex(escapedString);

    if (!matchAndMarkUmbrella(headerFiles, umbrellaRegex, HeaderType::Public))
      diag.report(diag::warn_no_such_umbrella_header_file)
          << false /* private */
          << publicUmbrellaHeaderPath;
  } else {
    auto frameworkName = sys::path::stem(framework.getName());
    auto umbrellaName = "/" + Regex::escape(frameworkName) + "\\.h";
    Regex umbrellaRegex(umbrellaName);

    matchAndMarkUmbrella(headerFiles, umbrellaRegex, HeaderType::Public);
  }

  auto privateUmbrellaHeaderPath =
      context.config.getUmbrellaHeader(frameworkPath, HeaderType::Private);
  if (!privateUmbrellaHeaderPath.empty()) {
    auto escapedString = Regex::escape(privateUmbrellaHeaderPath);
    Regex umbrellaRegex(escapedString);

    if (!matchAndMarkUmbrella(headerFiles, umbrellaRegex, HeaderType::Private))
      diag.report(diag::warn_no_such_umbrella_header_file)
          << true /* private */
          << privateUmbrellaHeaderPath;
  } else {
    auto frameworkName = sys::path::stem(framework.getName());
    auto umbrellaName = "/" + Regex::escape(frameworkName) + "[_]?Private\\.h";
    Regex umbrellaRegex(umbrellaName);

    matchAndMarkUmbrella(headerFiles, umbrellaRegex, HeaderType::Private);
  }

  if (diag.hasErrorOccurred())
    return false;

  std::stable_sort(headerFiles.begin(), headerFiles.end());
  job->headerFiles.insert(job->headerFiles.end(), preIncludes.begin(),
                          preIncludes.end());
  job->headerFiles.insert(job->headerFiles.end(), headerFiles.begin(),
                          headerFiles.end());

  // Skip frontend work if there are no headers.
  if (job->headerFiles.empty())
    return true;

  // If use overlay, map the headers onto sysroot.
  if (context.config.useOverlay(frameworkPath)) {
    auto &filemap =
        isPublic ? context.publicVFSOverlay : context.internalVFSOverlay;
    if (!filemap.empty()) {
      for (auto &header : job->headerFiles) {
        auto mappedPath = llvm::find_if(
            filemap, [&](DirectoryScanner::FileMap::const_reference entry) {
              return header.fullPath == entry.second;
            });
        if (mappedPath != filemap.end())
          header.fullPath = mappedPath->first;
      }
      IntrusiveRefCntPtr<vfs::RedirectingFileSystem> vfs(
          vfs::RedirectingFileSystem::create(filemap,
                                             /*UseExternalName=*/true,
                                             *job->vfs));
      job->vfs = vfs;
    }
    job->useRelativePath = true;
  }

  if (framework.isSysRoot) {
    if (!context.config.useOverlay(frameworkPath)) {
      SmallString<PATH_MAX> prefixIncludes(rootPath);
      sys::path::append(prefixIncludes, "usr", "include");
      job->includePaths.insert(job->includePaths.begin(),
                               prefixIncludes.str().str());
      SmallString<PATH_MAX> prefixFrameworks(rootPath);
      sys::path::append(prefixFrameworks, "System", "Library", "Frameworks");
      job->frameworkPaths.insert(job->frameworkPaths.begin(),
                                 prefixFrameworks.str().str());
    }
  } else if (!context.config.useOverlay(frameworkPath)) {
    // Add the current framework directory as a system framework directory.
    // This will prevent it from being droped from the top of the list if
    // there is a matching system framework include path.
    auto currentFrameworkPath = sys::path::parent_path(basePath);
    // for macOS which doesn't use shallow framework layout, need more
    // adjustment.
    if (sys::path::filename(currentFrameworkPath) == "Versions")
      currentFrameworkPath =
          sys::path::parent_path(sys::path::parent_path(currentFrameworkPath));
    job->frameworkPaths.insert(job->frameworkPaths.begin(),
                               currentFrameworkPath.str());
  }

  auto computeAndAddPath = [&](PathSeq &paths, StringRef additionalPath) {
    if (additionalPath.empty())
      return;

    if (!context.config.useOverlay(frameworkPath)) {
      paths.insert(paths.begin(), additionalPath.str());
      return;
    }

    StringRef relativePath = additionalPath;
    relativePath.consume_front(rootPath);
    SmallString<PATH_MAX> mappedPath(sysroot);
    sys::path::append(mappedPath, relativePath);
    paths.insert(paths.begin(), mappedPath.str().str());
  };

  computeAndAddPath(job->includePaths, framework.getAdditionalIncludePath());
  computeAndAddPath(job->frameworkPaths,
                    framework.getAdditionalFrameworkPath());

  // If we are scanning public headers, we use PublicOverlayFileSystem to
  // masking out the private locations from the SDK.
  if (isPublic) {
    IntrusiveRefCntPtr<PublicSDKOverlayFileSystem> overlay(
        new PublicSDKOverlayFileSystem(job->vfs, job->isysroot));
    job->vfs = overlay;
  }

  auto &output =
      isPublic ? context.publicSDKResults : context.internalSDKResults;
  for (auto type : {HeaderType::Public, HeaderType::Private}) {
    if (publicOnly && type == HeaderType::Private)
      continue;

    std::vector<FrontendContext> results;
    for (auto &target : triples) {
      job->target = target;
      job->type = type;
      job->afterIncludePaths = context.afterIncludePaths;
      job->systemIncludePaths = context.systemIncludePaths;
      job->useUmbrellaHeaderOnly = context.config.useUmbrellaOnly();
      if (target.getEnvironment() == Triple::MacABI) {
        job->systemIncludePaths.push_back(
            job->isysroot + MACCATALYST_PREFIX_PATH "/usr/include");
        job->systemIncludePaths.push_back(
            job->isysroot + MACCATALYST_PREFIX_PATH "/usr/local/include");
        job->systemFrameworkPaths.push_back(job->isysroot +
                                            MACCATALYST_PREFIX_PATH
                                            "/System/Library/Frameworks");
        job->systemFrameworkPaths.push_back(
            job->isysroot + MACCATALYST_PREFIX_PATH
            "/System/Library/PrivateFrameworks");
        job->useUmbrellaHeaderOnly = context.config.useUmbrellaOnly();
      }
      llvm::append_range(job->systemFrameworkPaths, context.systemFrameworkPaths);

      if (sys::fs::is_directory(context.outputPath)) {
        std::string filename = framework.getName().empty()
                                   ? "tapi_includes"
                                   : framework.getName().str();
        std::string crashTemplate =
            context.outputPath + "/" + filename + "-%%%%%%";
        job->clangReproducerPath = crashTemplate;
      }
      job->createClangReproducer = true;

      auto contextOrError = runFrontend(*job);
      if (auto err = contextOrError.takeError()) {
        if (canIgnoreFrontendError(err))
          continue;
        return false;
      }

      results.emplace_back(std::move(*contextOrError));
    }
    if (context.verifyAPI && results.size() == 2) {
      auto &api1 = results.front();
      auto &api2 = results.back();
      // Run verifier when the environments are different.
      if (api1.api->getTriple().getEnvironment() !=
          api2.api->getTriple().getEnvironment()) {
        APIVerifier verifier(context.getDiag());
        if (context.verifyAllowlist) {
          auto error = verifier.getConfiguration().readConfig(
              context.verifyAllowlist->getMemBufferRef());
          if (error) {
            diag.report(diag::err_invalid_verifier_allowlist_file)
                << context.verifyAllowlistFileName
                << toString(std::move(error));
            return false;
          }
        }
        // Make sure iosmac is the variant target.
        if (api1.api->getTriple().getEnvironment() ==
            Triple::EnvironmentType::MacABI)
          verifier.verify(api2, api1,
                          /*depth*/ 0, context.verifyAPISkipExternalHeaders);
        else
          verifier.verify(api1, api2,
                          /*depth*/ 0, context.verifyAPISkipExternalHeaders);
      }
    }
    for (auto &r : results)
      output.emplace_back(std::move(r));
  }

  return true;
}

static Expected<API> createAPIsFromSwiftAPIJson(llvm::json::Object &input) {
  auto version = input.getString("version");
  if (*version != "1.0")
    return make_error<APIJSONError>("Unsupported version");

  if (auto result = APIJSONSerializer::parse(&input))
    return std::move(*result);
  else
    return result.takeError();
}

static Error scanFramework(sdkdb::Context &context, Framework &framework,
                           bool isPublic, bool binaryOnly,
                           bool publicOnly = false) {
  if (publicOnly && framework.getPath().contains("PrivateFrameworks"))
    return Error::success();

  //
  // First scan all sub-frameworks, because we most likely will depend on them.
  //
  for (auto &F : framework._subFrameworks) {
    if (auto err = scanFramework(context, F, isPublic, binaryOnly, publicOnly))
      return err;
  }

  //
  // Second scan all versions ...
  //
  for (auto &F : framework._versions) {
    if (auto err = scanFramework(context, F, isPublic, binaryOnly, publicOnly))
      return err;
  }

  //
  // Now scan the framework binary.
  //
  std::vector<Triple> triples;
  for (const auto &path : framework._dynamicLibraryFiles) {
    if (auto err = context.scanBinaryFile(path, triples, isPublic))
      return err;
  }

  if (binaryOnly)
    return Error::success();

  // If there are no binaries, guess the triple from environment.
  if (triples.empty())
    inferTriplesFromEnvironment(context, framework, triples);

  // Now scan the header files.
  if (!computeFrontendResultFromFramework(context, framework, triples,
        isPublic, publicOnly)) {
    if (!context.hasSDKDBError) {
      context.hasSDKDBError = true;
      context.getDiag().report(diag::err_cannot_generate_sdkdb)
          << "Failed to scan header interface";
    }
  }

  return Error::success();
}

static bool interfaceScan(sdkdb::Context &context, Options &opts) {
  auto &config = context.config.getCommandlineConfig();
  auto &diag = context.getDiag();

  // FIXME: Copy the options for now to reduce the amount of change.
  config.language = opts.frontendOptions.language;
  config.std = opts.frontendOptions.language_std;
  config.isysroot = opts.frontendOptions.isysroot;
  config.frameworkPaths = opts.frontendOptions.frameworkPaths;
  config.includePaths = opts.frontendOptions.includePaths;
  config.macros = opts.frontendOptions.macros;
  config.useRTTI = opts.frontendOptions.useRTTI;
  config.useNoRTTI = opts.frontendOptions.useNoRTTI;
  config.visibility = opts.frontendOptions.visibility;
  config.enableModules = opts.frontendOptions.enableModules;
  config.moduleCachePath = opts.frontendOptions.moduleCachePath;
  config.validateSystemHeaders = opts.frontendOptions.validateSystemHeaders;
  config.clangExtraArgs = opts.frontendOptions.clangExtraArgs;
  config.clangResourcePath = opts.frontendOptions.clangResourcePath;

  context.config.setArchitectures(ArchitectureSet::All());

  // Handle extra header directories/files.
  config.extraPublicHeaders = opts.tapiOptions.extraPublicHeaders;
  config.extraPrivateHeaders = opts.tapiOptions.extraPrivateHeaders;

  // Handle excluded header files.
  config.excludePublicHeaders = opts.tapiOptions.excludePublicHeaders;
  config.excludePrivateHeaders = opts.tapiOptions.excludePrivateHeaders;

  // Handle disabling of header scanning.
  config.scanPublicHeaders = opts.sdkdbOptions.scanPublicHeaders;
  config.scanPrivateHeaders = opts.sdkdbOptions.scanPrivateHeaders;

  // Handle public/private umbrella header.
  if (!opts.tapiOptions.publicUmbrellaHeaderPath.empty()) {
    if (!context.getFileManager().exists(
            opts.tapiOptions.publicUmbrellaHeaderPath)) {
      diag.report(clang::diag::err_drv_no_such_file)
          << opts.tapiOptions.publicUmbrellaHeaderPath;
      return false;
    }
    config.publicUmbrellaHeaderPath = opts.tapiOptions.publicUmbrellaHeaderPath;
  }

  if (!opts.tapiOptions.privateUmbrellaHeaderPath.empty()) {
    if (!context.getFileManager().exists(
            opts.tapiOptions.privateUmbrellaHeaderPath)) {
      diag.report(clang::diag::err_drv_no_such_file)
          << opts.tapiOptions.privateUmbrellaHeaderPath;
      return false;
    }
    config.privateUmbrellaHeaderPath =
        opts.tapiOptions.privateUmbrellaHeaderPath;
  }

  // setup overlay file system if needed.
  if (!context.config.getRootMaskPaths().empty() ||
      !context.config.getSDKMaskPaths().empty()) {
    auto &fm = context.getFileManager();
    IntrusiveRefCntPtr<PathMaskingOverlayFileSystem> overlay(
        new PathMaskingOverlayFileSystem(&fm.getVirtualFileSystem()));
    auto addRelativePathFromRoot = [&](StringRef root, StringRef path) {
      if (root.empty())
        return;
      SmallString<PATH_MAX> rootPath(root);
      sys::path::append(rootPath, path);
      overlay->addExtraMaskingDirectory(rootPath);
    };
    for (auto &path : context.config.getRootMaskPaths()) {
      // mask the path from all the roots.
      addRelativePathFromRoot(opts.sdkdbOptions.publicSDKContentRoot, path);
      addRelativePathFromRoot(opts.sdkdbOptions.sdkContentRoot, path);
    }
    for (auto &path : context.config.getSDKMaskPaths())
      addRelativePathFromRoot(config.isysroot, path);
    fm.setVirtualFileSystem(overlay);
  }

  // Scan roots and setup VFS overlays.
  std::vector<Framework> publicFrameworks, internalFrameworks;

  // Scan PublicSDKContentRoot.
  if (config.scanPublicHeaders && !context.config.scanPublicHeadersInSDKContentRoot()) {
    DirectoryScanner scanner(context.getFileManager(), diag,
                             ScannerMode::ScanRuntimeRoot);
    // Scan binary first.
    scanner.setSplitHeaderDir(context.config.useSplitHeaderDir());
    if (!scanner.scan(opts.sdkdbOptions.runtimeRoot))
      return false;

    // Scan headers if it exists.
    auto rootPath = opts.sdkdbOptions.publicSDKContentRoot.empty()
                        ? opts.sdkdbOptions.sdkContentRoot
                        : opts.sdkdbOptions.publicSDKContentRoot;
    scanner.setMode(ScannerMode::ScanPublicSDK);
    // scan returns false if there's an error.
    if (!scanner.scan(rootPath))
      return false;

    // Setup public VFS overlay from the scanning results.
    context.publicVFSOverlay = scanner.getVFSFileMap(
        context.config.getSysRoot(),
        ArrayRef<StringRef>{context.config.getSysRoot(),
                            opts.sdkdbOptions.runtimeRoot, rootPath});

    publicFrameworks = scanner.takeResult();
    assert(publicFrameworks.size() == 1 &&
           "There should be only one top level framework");
  }

  // Scan SDKContentRoot.
  {
    DirectoryScanner scanner(context.getFileManager(), diag,
                             ScannerMode::ScanRuntimeRoot);
    scanner.setSplitHeaderDir(context.config.useSplitHeaderDir());
    if (!scanner.scan(opts.sdkdbOptions.runtimeRoot))
      return false;

    scanner.setMode(ScannerMode::ScanInternalSDK);
    if (!scanner.scan(opts.sdkdbOptions.sdkContentRoot))
      return false;

    // Setup internal VFS overlay from the scanning results.
    context.internalVFSOverlay = scanner.getVFSFileMap(
        context.config.getSysRoot(),
        ArrayRef<StringRef>{context.config.getSysRoot(),
                            opts.sdkdbOptions.runtimeRoot,
                            opts.sdkdbOptions.sdkContentRoot});

    internalFrameworks = scanner.takeResult();
    assert(internalFrameworks.size() == 1 &&
           "There should be only one top level framework");
  }

  // Scan frameworks.
  if (config.scanPublicHeaders && !context.config.scanPublicHeadersInSDKContentRoot()) {
    auto rootPath = opts.sdkdbOptions.publicSDKContentRoot.empty()
                        ? opts.sdkdbOptions.sdkContentRoot
                        : opts.sdkdbOptions.publicSDKContentRoot;
    context.config.setRootPath(rootPath);
    auto &framework = publicFrameworks.front();
    if (auto err = scanFramework(context, framework, /*isPublic*/ true,
                                 /*binary only*/ false)) {
      diag.report(diag::err_cannot_generate_sdkdb) << toString(std::move(err));
      context.hasSDKDBError = true;
    }

    // Update APILoc for the header scan results.
    for (auto &result : context.publicSDKResults) {
      sdkdb::APILocUpdater locUpdater(rootPath, *result.api);
      result.api->visit(locUpdater);
    }
  }

  {
    context.config.setRootPath(opts.sdkdbOptions.sdkContentRoot);
    auto &framework = internalFrameworks.front();
    if (auto err = scanFramework(context, framework, /*isPublic*/ false,
                                 /*binary only*/ !config.scanPrivateHeaders)) {
      diag.report(diag::err_cannot_generate_sdkdb) << toString(std::move(err));
      context.hasSDKDBError = true;
    }

    for (auto &result : context.internalSDKResults) {
      sdkdb::APILocUpdater locUpdater(opts.sdkdbOptions.sdkContentRoot,
                                      *result.api);
      result.api->visit(locUpdater);
    }

    if (context.config.scanPublicHeadersInSDKContentRoot()) {
      if (auto err = scanFramework(context, framework, /*isPublic*/ false,
                                   /*binary only*/ false, /*publicOnly=*/true)) {
        diag.report(diag::err_cannot_generate_sdkdb) << toString(std::move(err));
        context.hasSDKDBError = true;
      }

      for (auto &result : context.internalSDKResults) {
        sdkdb::APILocUpdater locUpdater(opts.sdkdbOptions.sdkContentRoot,
                                        *result.api);
        result.api->visit(locUpdater);
      }
    }
  }

  return true;
}

static bool writePartialSDKDB(sdkdb::Context &context) {
  auto partialSDKOutput = [&](raw_ostream &os) {
    context.hasWrittenPartialOutput = true;
    return PartialSDKDB::serialize(
        os, context.projectName, context.internalBinaryResults,
        context.publicSDKResults, context.extraPublicSDKResults,
        context.internalSDKResults, context.extraInternalSDKResults,
        context.hasSDKDBError);
  };
  if (auto err = context.performOutput("partial.sdkdb", partialSDKOutput)) {
    context.getDiag().report(diag::err_cannot_generate_sdkdb)
        << toString(std::move(err));
    return false;
  }

  return true;
}

static Error parsePartialSDKDB(sdkdb::Context &context, StringRef JSON) {
  auto inputValue = json::parse(JSON);
  if (!inputValue)
    return inputValue.takeError();

  auto *root = inputValue->getAsObject();
  if (!root)
    return make_error<APIJSONError>("API is not a JSON Object");

  if (context.action & SDKDBAction::SDKDBPublicGen) {
    auto partialResult = PartialSDKDB::createPublicAPIsFromJSON(*root);
    if (!partialResult)
      return partialResult.takeError();

    for (auto &result : partialResult->binaryInterfaces)
      context.publicBinaryResults.emplace_back(std::move(result));
    for (auto &result : partialResult->headerInterfaces)
      context.extraPublicSDKResults.emplace_back(std::move(result));
  }

  if (context.action & SDKDBAction::SDKDBPrivateGen) {
    auto partialResult = PartialSDKDB::createPrivateAPIsFromJSON(*root);
    if (!partialResult)
      return partialResult.takeError();

    for (auto &result : partialResult->binaryInterfaces)
      context.internalBinaryResults.emplace_back(std::move(result));
    for (auto &result : partialResult->headerInterfaces)
      context.extraInternalSDKResults.emplace_back(std::move(result));
  }

  return Error::success();
}

static bool readPartialSDKDBFile(sdkdb::Context &context, StringRef path) {
  auto file = context.getFileManager().getFile(path);
  if (!file) {
    context.getDiag().report(diag::err_cannot_open_file)
        << path << file.getError().message();
    return false;
  }

  auto buffer = context.getFileManager().getBufferForFile(*file);
  if (!buffer) {
    context.getDiag().report(diag::err_cannot_read_file) << path;
    return false;
  }

  if (auto err = parsePartialSDKDB(context, (*buffer)->getBuffer())) {
    context.getDiag().report(diag::err_cannot_generate_sdkdb)
        << toString(std::move(err));
    return false;
  }

  return true;
}

static bool readPartialSDKDBInputs(sdkdb::Context &context, Options &opts) {
  for (const auto& input : opts.driverOptions.inputs) {
    if (!readPartialSDKDBFile(context, input))
      return false;
  }

  if (context.partialSDKDBFilelist.empty())
    return true;

  // read partial sdkdb from file list.
  auto file = context.getFileManager().getFile(context.partialSDKDBFilelist);
  if (!file) {
    context.getDiag().report(diag::err_cannot_open_file)
        << context.partialSDKDBFilelist << file.getError().message();
    return false;
  }

  auto bufferOrErr = context.getFileManager().getBufferForFile(*file);
  if (!bufferOrErr) {
    context.getDiag().report(diag::err_cannot_read_file)
        << context.partialSDKDBFilelist;
    return false;
  }

  auto buffer = bufferOrErr.get()->getBuffer();
  SmallVector<StringRef, 16> lines;
  buffer.split(lines, "\n", /*MaxSplit=*/-1, /*KeepEmpty=*/false);
  for (const auto &line : lines) {
    auto l = line.trim();
    if (l.empty())
      continue;

    // Skip comments
    if (l.startswith("#"))
      continue;

    if (!readPartialSDKDBFile(context, l))
      return false;
  }

  return true;
}

static Error readExistingPartialSDKDBFromDirectory(sdkdb::Context &context) {
  // Skip if no output is specified or output is stdout.
  if (context.installAPISDKDBPath.empty())
    return Error::success();

  auto &fm = context.getFileManager();
  // if output path is not a directory, skip.
  if (!fm.isDirectory(context.installAPISDKDBPath))
    return Error::success();

  // look into directory for partial SDKDB files.
  auto &fs = fm.getVirtualFileSystem();
  std::error_code ec;
  std::vector<std::string> inputFiles;
  for (vfs::directory_iterator
           i = fs.dir_begin(context.installAPISDKDBPath, ec),
           ie;
       i != ie; i.increment(ec)) {
    // ignore errors.
    if (ec) {
      ec.clear();
      continue;
    }

    auto path = i->path();
    if (!path.endswith(".sdkdb"))
      continue;
   
    // This is the output name, skip that.
    if (sys::path::filename(path) == "partial.sdkdb")
      continue;

    inputFiles.emplace_back(path.str());
  }

  // sort the path so the output is deterministic and not depending on the
  // traverse order of the file system.
  llvm::sort(inputFiles);

  for (auto &path : inputFiles) {
    const bool isSwiftSDKDB = StringRef(path).ends_with(".swift.sdkdb");

    // Read partial SDKDB output.
    auto file = context.getFileManager().getFile(path);
    if (!file) {
      if (isSwiftSDKDB)
        return makeSwiftSDKDBError(path, file.getError());
      continue;
    }
    auto buffer = context.getFileManager().getBufferForFile(*file);
    if (!buffer) {
      if (isSwiftSDKDB)
        return makeSwiftSDKDBError(path, buffer.getError());
      continue;
    }

    auto inputValue = json::parse((*buffer)->getBuffer());
    if (!inputValue) {
      if (isSwiftSDKDB)
        return makeSwiftSDKDBError(path, inputValue.takeError());
      consumeError(inputValue.takeError());
      continue;
    }

    auto *root = inputValue->getAsObject();
    if (!root) {
      if (isSwiftSDKDB)
        return makeSwiftSDKDBError(path, "Invalid JSON object");
      continue;
    }

    if (isSwiftSDKDB) {
      auto publicResult = createAPIsFromSwiftAPIJson(*root);
      if (!publicResult)
        return makeSwiftSDKDBError(path, publicResult.takeError());

      context.extraPublicSDKResults.emplace_back(std::move(*publicResult));

      auto internalResult = createAPIsFromSwiftAPIJson(*root);
      if (!internalResult)
        return makeSwiftSDKDBError(path, internalResult.takeError());

      context.extraInternalSDKResults.emplace_back(std::move(*internalResult));
    } else {
      {
        auto partialResult = PartialSDKDB::createPublicAPIsFromJSON(*root);
        if (!partialResult) {
          consumeError(partialResult.takeError());
          continue;
        }

        for (auto &result : partialResult->binaryInterfaces)
          context.publicBinaryResults.emplace_back(std::move(result));
        for (auto &result : partialResult->headerInterfaces)
          context.extraPublicSDKResults.emplace_back(std::move(result));
      }

      {
        auto partialResult = PartialSDKDB::createPrivateAPIsFromJSON(*root);
        if (!partialResult) {
          consumeError(partialResult.takeError());
          continue;
        }

        for (auto &result : partialResult->binaryInterfaces)
          context.internalBinaryResults.emplace_back(std::move(result));
        for (auto &result : partialResult->headerInterfaces)
          context.extraInternalSDKResults.emplace_back(std::move(result));
      }
    }
  }

  return Error::success();
}

/// Scan the directory for header and dynamic libraries and generate
/// SDKDB files.
static bool runSDKDBDriver(sdkdb::Context &context, DiagnosticsEngine &diag,
                           Options &opts) {
  // setup diagnostics file.
  if (!opts.sdkdbOptions.diagnosticsFile.empty())
    diag.setupDiagnosticsFile(opts.sdkdbOptions.diagnosticsFile);

  // diagnose incompatible options.
  if (opts.sdkdbOptions.action & SDKDBAction::SDKDBInterfaceScan) {
    if (opts.sdkdbOptions.runtimeRoot.empty()) {
      diag.report(diag::err_expected_option) << "RuntimeRoot"
                                             << "--runtime-root";
      return false;
    }
    if (opts.sdkdbOptions.sdkContentRoot.empty()) {
      diag.report(diag::err_expected_option) << "SDKContentRoot"
                                             << "--sdk-content-root";
      return false;
    }
  }

  if (opts.sdkdbOptions.action == SDKDBAction::SDKDBNone) {
    // infer default action.
    if (!opts.sdkdbOptions.runtimeRoot.empty() &&
        !opts.sdkdbOptions.sdkContentRoot.empty())
      opts.sdkdbOptions.action = SDKDBAction::SDKDBInterfaceScan;
    else {
      // diagnose no action.
      diag.report(diag::err_expected_option) << "SDKDBAction"
                                             << "--action=";
      return false;
    }
  }

  // set configuraion file.
  if (!opts.sdkdbOptions.configurationFile.empty()) {
    auto file =
        context.getFileManager().getFile(opts.sdkdbOptions.configurationFile);
    if (!file) {
      diag.report(clang::diag::err_drv_no_such_file)
          << opts.sdkdbOptions.configurationFile;
      return false;
    }

    auto bufferOrErr = context.getFileManager().getBufferForFile(*file);
    if (auto ec = bufferOrErr.getError()) {
      diag.report(diag::err_cannot_read_file) << (*file)->getName()
                                              << ec.message();
      return false;
    }

    auto configurationFile =
        ConfigurationFileReader::get((*bufferOrErr)->getMemBufferRef());
    if (!configurationFile) {
      diag.report(diag::err_cannot_read_file)
          << (*file)->getName() << toString(configurationFile.takeError());
      return false;
    }

    auto configuration = configurationFile.get()->takeConfigurationFile();
    context.config.setConfiguration(std::move(configuration), context);
  }

  // Run InterfaceScan.
  if (opts.sdkdbOptions.action & SDKDBAction::SDKDBInterfaceScan) {
    if (!context.config.ignoreExistingPartialSDKDBs()) {
      // Scan the output directory to see if there are any partial outputs.
      if (auto err = readExistingPartialSDKDBFromDirectory(context)) {
        diag.report(diag::err_cannot_generate_sdkdb)
            << toString(std::move(err));
        return false;
      }
    }
    // Scan interface from roots.
    if (!interfaceScan(context, opts))
      return false;
  }

  // If all we need is to scan interface, write partialSDKDB and return.
  if (opts.sdkdbOptions.action == SDKDBAction::SDKDBInterfaceScan) {
    if (!writePartialSDKDB(context))
      return false;
    return !diag.hasErrorOccurred();
  }

  // Process extra inputs.
  if (!readPartialSDKDBInputs(context, opts))
    return false;

  // Combine SDKDB.
  if (opts.sdkdbOptions.action & SDKDBAction::SDKDBPublicGen) {
    SDKDBBuilderOptions options = SDKDBBuilderOptions::isPublicOnly;
    SDKDBBuilder builder(diag, options);
    for (auto &api : context.publicBinaryResults) {
      if (auto err = builder.addBinaryAPI(std::move(api)))
        diag.report(diag::err_cannot_generate_sdkdb)
            << toString(std::move(err));
    }

    for (const auto &result : context.publicSDKResults) {
      if (auto err = builder.addHeaderAPI(*result.api))
        diag.report(diag::err_cannot_generate_sdkdb)
            << toString(std::move(err));
    }

    for (const auto &api : context.extraPublicSDKResults) {
      if (auto err = builder.addHeaderAPI(api))
        diag.report(diag::err_cannot_generate_sdkdb)
            << toString(std::move(err));
    }

    if (auto err = builder.finalize()) {
      diag.report(diag::err_cannot_generate_sdkdb) << toString(std::move(err));
      return false;
    }

    auto publicSDKOutput = [&](raw_ostream &os) {
      builder.serialize(os, /*compact*/ false);
      return Error::success();
    };
    if (auto err = context.performOutput("public.sdkdb", publicSDKOutput)) {
      context.getDiag().report(diag::err_cannot_generate_sdkdb)
          << toString(std::move(err));
      return false;
    }
  }

  if (opts.sdkdbOptions.action & SDKDBAction::SDKDBPrivateGen) {
    SDKDBBuilder builder(diag);
    for (auto &api : context.internalBinaryResults) {
      if (auto err = builder.addBinaryAPI(std::move(api)))
        diag.report(diag::err_cannot_generate_sdkdb)
            << toString(std::move(err));
    }

    auto &sdkResult = context.internalSDKResults.empty()
                         ? context.publicSDKResults
                         : context.internalSDKResults;
    for (const auto &result : sdkResult) {
      if (auto err = builder.addHeaderAPI(*result.api))
        diag.report(diag::err_cannot_generate_sdkdb)
            << toString(std::move(err));
    }

    auto &extraSDKResult = context.extraInternalSDKResults.empty()
                         ? context.extraPublicSDKResults
                         : context.extraInternalSDKResults;
    for (const auto &api : extraSDKResult) {
      if (auto err = builder.addHeaderAPI(api))
        diag.report(diag::err_cannot_generate_sdkdb)
            << toString(std::move(err));
    }

    if (auto err = builder.finalize()) {
      diag.report(diag::err_cannot_generate_sdkdb) << toString(std::move(err));
      return false;
    }

    auto internalSDKOutput = [&](raw_ostream &os) {
      builder.serialize(os, /*compact*/ false);
      return Error::success();
    };
    if (auto err = context.performOutput("private.sdkdb", internalSDKOutput)) {
      context.getDiag().report(diag::err_cannot_generate_sdkdb)
          << toString(std::move(err));
      return false;
    }
  }

  return !diag.hasErrorOccurred();
}

bool Driver::SDKDB::run(DiagnosticsEngine &diag, Options &opts) {
  sdkdb::Context context(opts, diag);
  if (!runSDKDBDriver(context, diag, opts)) {
    // If doing interface scan only and the partial output is not written,
    // write an empty context.
    if (opts.sdkdbOptions.action == SDKDBAction::SDKDBInterfaceScan &&
        !context.hasWrittenPartialOutput) {
      sdkdb::Context emptyContext(opts, diag);
      emptyContext.hasSDKDBError = true;
      writePartialSDKDB(emptyContext);
    }
    // Return false to error out if `TAPI_SDKDB_FORCE_ERROR` is set to 1.
    // Otherwise return true to ignore the SDKDB error.
    return sys::Process::GetEnv("TAPI_SDKDB_FORCE_ERROR").value_or("") != "1";
  }

  return true;
}

TAPI_NAMESPACE_INTERNAL_END
