//===- libtapi/LinkerInterfaceFile.cpp - TAPI File Interface ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the C++ linker interface file API.
///
//===----------------------------------------------------------------------===//
#include "tapi/Core/LLVM.h"
#include "tapi/Core/Registry.h"
#include "tapi/Core/Utils.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Object/MachO.h"
#include "llvm/TextAPI/InterfaceFile.h"
#include <string>
#include <tapi/LinkerInterfaceFile.h>
#include <tapi/PackedVersion32.h>
#include <tapi/Symbol.h>
#include <vector>

using namespace llvm;
using namespace llvm::MachO;

TAPI_NAMESPACE_V1_BEGIN

using namespace tapi::internal;
using InterfaceFile = llvm::MachO::InterfaceFile;
using PackedVersion = llvm::MachO::PackedVersion;

static PackedVersion parseVersion32(StringRef str) {
  uint32_t version = 0;
  if (str.empty())
    return PackedVersion();

  SmallVector<StringRef, 3> parts;
  SplitString(str, parts, ".");

  unsigned long long num = 0;
  if (getAsUnsignedInteger(parts[0], 10, num))
    return PackedVersion();

  if (num > UINT16_MAX)
    return PackedVersion();

  version = num << 16;

  if (parts.size() > 1) {
    if (getAsUnsignedInteger(parts[1], 10, num))
      return PackedVersion();

    if (num > UINT8_MAX)
      return PackedVersion();

    version |= (num << 8);
  }

  if (parts.size() > 2) {
    if (getAsUnsignedInteger(parts[2], 10, num))
      return PackedVersion();

    if (num > UINT8_MAX)
      return PackedVersion();

    version |= num;
  }

  return PackedVersion(version);
}

class LLVM_LIBRARY_VISIBILITY LinkerInterfaceFile::Impl {
public:
  std::vector<std::pair<uint32_t, PackedVersion32>> _platformAndMinOS;
  std::vector<uint32_t> _platforms;
  std::string _installName;
  std::string _parentFrameworkName;

  PackedVersion _currentVersion;
  PackedVersion _compatibilityVersion;
  unsigned _swiftABIVersion;
  bool _hasTwoLevelNamespace{false};
  bool _isAppExtensionSafe{false};
  bool _isOSLibNotForSharedCache{false};
  bool _hasWeakDefExports{false};
  bool _installPathOverride{false};

  std::vector<std::string> _reexportedLibraries;
  std::vector<std::string> _allowableClients;
  std::vector<std::string> _ignoreExports;
  std::vector<std::string> _inlinedFrameworkNames;
  std::vector<std::string> _rPaths;
  std::vector<std::string> _relinkedLibraries;

  // All exports and reexports.
  // TODO: Treat them seperately to match TextFile output.
  // and support all globals option.
  std::vector<Symbol> _exports;
  std::vector<Symbol> _undefineds;
  std::shared_ptr<const InterfaceFile> _interface;
  std::vector<std::shared_ptr<const InterfaceFile>> _inlinedFrameworks;

  Impl() noexcept = default;

  bool init(const std::shared_ptr<const InterfaceFile> &interface,
            cpu_type_t cpuType, cpu_subtype_t cpuSubType, ParsingFlags flags,
            PackedVersion32 minOSVersion, std::string &errorMessage) noexcept;

  template <typename T>
  void addSymbol(T &&name, llvm::MachO::SymbolFlags flags) {
    if (find(_ignoreExports, name) == _ignoreExports.end())
      _exports.emplace_back(std::forward<T>(name),
                            static_cast<SymbolFlags>(flags));
  }

  void processSymbol(StringRef name, PackedVersion minOSVersion,
                     bool disallowWeakImports) {
    // $ld$ <action> $ <condition> $ <symbol-name>
    if (!name.startswith("$ld$"))
      return;

    StringRef action, condition, symbolName;
    std::tie(action, name) = name.drop_front(4).split('$');
    std::tie(condition, symbolName) = name.split('$');
    if (action.empty() || condition.empty() || symbolName.empty())
      return;

    if (!condition.startswith("os"))
      return;

    auto version = parseVersion32(condition.drop_front(2));
    if (version != minOSVersion)
      return;

    if (action == "hide") {
      _ignoreExports.emplace_back(symbolName);
      return;
    }

    if (action == "add") {
      _exports.emplace_back(symbolName);
      return;
    }

    if (action == "weak") {
      if (disallowWeakImports)
        _ignoreExports.emplace_back(symbolName);

      return;
    }

    if (action == "install_name") {
      _installName = symbolName.str();
      _installPathOverride = true;
      if (_installName == "/System/Library/Frameworks/"
                          "ApplicationServices.framework/Versions/A/"
                          "ApplicationServices") {
        _compatibilityVersion = PackedVersion(1, 0, 0);
      }
      return;
    }

    if (action == "compatibility_version") {
      _compatibilityVersion = parseVersion32(symbolName);
      return;
    }
  }
};

static Architecture getArchForCPU(cpu_type_t cpuType, cpu_subtype_t cpuSubType,
                                  bool enforceCpuSubType,
                                  ArchitectureSet archs) {
  // First check the exact cpu type and cpu sub type.
  auto arch = getArchitectureFromCpuType(cpuType, cpuSubType);
  if (archs.has(arch))
    return arch;

  if (enforceCpuSubType)
    return AK_unknown;
  return arch;
}

LinkerInterfaceFile::LinkerInterfaceFile() noexcept
    : _pImpl{new LinkerInterfaceFile::Impl} {}
LinkerInterfaceFile::~LinkerInterfaceFile() noexcept = default;
LinkerInterfaceFile::LinkerInterfaceFile(LinkerInterfaceFile &&) noexcept =
    default;
LinkerInterfaceFile &
LinkerInterfaceFile::operator=(LinkerInterfaceFile &&) noexcept = default;

std::vector<std::string>
LinkerInterfaceFile::getSupportedFileExtensions() noexcept {
  return {".tbd"};
}

/// \brief Load and parse the provided TBD file in the buffer and return on
///        success the interface file.
static Expected<std::unique_ptr<const InterfaceFile>>
loadFile(std::unique_ptr<MemoryBuffer> buffer,
         ReadFlags readFlags = ReadFlags::Symbols) {
  Registry registry;
  registry.addYAMLReaders();
  registry.addJSONReaders();
  registry.addDiagnosticReader();

  auto textFile = registry.readTextFile(std::move(buffer), readFlags);
  if (!textFile)
    return textFile.takeError();

  return std::unique_ptr<const InterfaceFile>(
      cast<const InterfaceFile>(textFile.get().release()));

  return std::make_unique<const InterfaceFile>(InterfaceFile());
}

bool LinkerInterfaceFile::isSupported(const std::string &path,
                                      const uint8_t *data,
                                      size_t size) noexcept {
  Registry registry;
  registry.addYAMLReaders();
  registry.addJSONReaders();
  registry.addDiagnosticReader();
  auto memBuffer = MemoryBufferRef(
      StringRef(reinterpret_cast<const char *>(data), size), path);
  return registry.canRead(memBuffer);
}

bool LinkerInterfaceFile::shouldPreferTextBasedStubFile(
    const std::string &path) noexcept {
  return true;
}

bool LinkerInterfaceFile::areEquivalent(const std::string &tbdPath,
                                        const std::string &dylibPath) noexcept {
  return false;
}

bool LinkerInterfaceFile::Impl::init(
    const std::shared_ptr<const InterfaceFile> &interface, cpu_type_t cpuType,
    cpu_subtype_t cpuSubType, ParsingFlags flags, PackedVersion32 minOSVersion,
    std::string &errorMessage) noexcept {
  _interface = interface;
  bool enforceCpuSubType = flags & ParsingFlags::ExactCpuSubType;
  auto arch = getArchForCPU(cpuType, cpuSubType, enforceCpuSubType,
                            interface->getArchitectures());
  if (arch == AK_unknown) {
    auto arch = getArchitectureFromCpuType(cpuType, cpuSubType);
    auto count = interface->getArchitectures().count();
    if (count > 1)
      errorMessage = "missing required architecture " +
                     getArchitectureName(arch).str() + " in file " +
                     interface->getPath().str() + " (" + std::to_string(count) +
                     " slices)";
    else
      errorMessage = "missing required architecture " +
                     getArchitectureName(arch).str() + " in file " +
                     interface->getPath().str();
    return false;
  }

  // Remove the patch level.

  auto minOSPackedVersion =
      PackedVersion(minOSVersion.getMajor(), minOSVersion.getMinor(), 0);

  for (auto target : interface->targets()) {
    if (target.Arch != arch)
      continue;
    if (target.Platform == PLATFORM_UNKNOWN)
      continue;
    uint32_t platform = static_cast<uint32_t>(target.Platform);
    _platforms.emplace_back(platform);
    PackedVersion32 minDeployment =
        PackedVersion(target.MinDeployment).rawValue();
    _platformAndMinOS.emplace_back(platform, minDeployment);
  }
  llvm::sort(_platforms);
  _installName = std::string(interface->getInstallName());
  _currentVersion = interface->getCurrentVersion();
  _compatibilityVersion = interface->getCompatibilityVersion();
  _hasTwoLevelNamespace = interface->isTwoLevelNamespace();
  _isAppExtensionSafe = interface->isApplicationExtensionSafe();
  _isOSLibNotForSharedCache = interface->isOSLibNotForSharedCache();
  _swiftABIVersion = interface->getSwiftABIVersion();
  for (const auto &it : interface->umbrellas()) {
    if (it.first.Arch != arch)
      continue;
    _parentFrameworkName = it.second;
    break;
  }

  // Pre-scan for special linker symbols.
  for (const auto *symbol : interface->exports()) {
    if (symbol->getKind() != EncodeKind::GlobalSymbol)
      continue;

    if (!symbol->hasArchitecture(arch))
      continue;

    processSymbol(symbol->getName(), minOSPackedVersion,
                  flags & ParsingFlags::DisallowWeakImports);
  }
  sort(_ignoreExports);
  auto last = std::unique(_ignoreExports.begin(), _ignoreExports.end());
  _ignoreExports.erase(last, _ignoreExports.end());

  bool useObjC1ABI =
      interface->getPlatforms().count(PLATFORM_MACOS) && (arch == AK_i386);
  for (const auto *symbol : interface->symbols()) {
    if (symbol->isUndefined())
      continue;
    if (!symbol->hasArchitecture(arch))
      continue;

    switch (symbol->getKind()) {
    case EncodeKind::GlobalSymbol:
      if (symbol->getName().startswith("$ld$") &&
          !symbol->getName().startswith("$ld$previous"))
        continue;
      addSymbol(symbol->getName(), symbol->getFlags());
      break;
    case EncodeKind::ObjectiveCClass:
      if (useObjC1ABI) {
        addSymbol(".objc_class_name_" + symbol->getName().str(),
                  symbol->getFlags());
      } else {
        addSymbol("_OBJC_CLASS_$_" + symbol->getName().str(),
                  symbol->getFlags());
        addSymbol("_OBJC_METACLASS_$_" + symbol->getName().str(),
                  symbol->getFlags());
      }
      break;
    case EncodeKind::ObjectiveCClassEHType:
      addSymbol("_OBJC_EHTYPE_$_" + symbol->getName().str(),
                symbol->getFlags());
      break;
    case EncodeKind::ObjectiveCInstanceVariable:
      addSymbol("_OBJC_IVAR_$_" + symbol->getName().str(), symbol->getFlags());
      break;
    }

    if (symbol->isWeakDefined())
      _hasWeakDefExports = true;
  }

  for (const auto *symbol : interface->undefineds()) {
    if (!symbol->hasArchitecture(arch))
      continue;

    switch (symbol->getKind()) {
    case EncodeKind::GlobalSymbol:
      _undefineds.emplace_back(symbol->getName(),
                               static_cast<SymbolFlags>(symbol->getFlags()));
      break;
    case EncodeKind::ObjectiveCClass:
      if (useObjC1ABI) {
        _undefineds.emplace_back(".objc_class_name_" + symbol->getName().str(),
                                 static_cast<SymbolFlags>(symbol->getFlags()));
      } else {
        _undefineds.emplace_back("_OBJC_CLASS_$_" + symbol->getName().str(),
                                 static_cast<SymbolFlags>(symbol->getFlags()));
        _undefineds.emplace_back("_OBJC_METACLASS_$_" + symbol->getName().str(),
                                 static_cast<SymbolFlags>(symbol->getFlags()));
      }
      break;
    case EncodeKind::ObjectiveCClassEHType:
      _undefineds.emplace_back("_OBJC_EHTYPE_$_" + symbol->getName().str(),
                               static_cast<SymbolFlags>(symbol->getFlags()));
      break;
    case EncodeKind::ObjectiveCInstanceVariable:
      _undefineds.emplace_back("_OBJC_IVAR_$_" + symbol->getName().str(),
                               static_cast<SymbolFlags>(symbol->getFlags()));
      break;
    }
  }

  for (const auto &lib : interface->allowableClients())
    for (const auto &target : lib.targets())
      if (target.Arch == arch)
        _allowableClients.emplace_back(lib.getInstallName());

  for (const auto &lib : interface->reexportedLibraries())
    for (const auto &target : lib.targets())
      if (target.Arch == arch)
        _reexportedLibraries.emplace_back(lib.getInstallName());

  for (const auto &[target, path] : interface->rpaths())
    if (target.Arch == arch)
      _rPaths.emplace_back(path);

  for (auto &file : interface->documents()) {
    auto framework = std::static_pointer_cast<const InterfaceFile>(file);
    _inlinedFrameworkNames.emplace_back(framework->getInstallName());
    _inlinedFrameworks.emplace_back(framework);
  }

  return true;
}

LinkerInterfaceFile *
LinkerInterfaceFile::create(const std::string &path, cpu_type_t cpuType,
                            cpu_subtype_t cpuSubType, ParsingFlags flags,
                            PackedVersion32 minOSVersion,
                            std::string &errorMessage) noexcept {

  auto errorOr = MemoryBuffer::getFile(path, /*IsText=*/true,
                                       /*RequiresNullTerminator=*/true,
                                       /*IsVolatile=*/inBnIEnvironment());
  if (auto ec = errorOr.getError()) {
    errorMessage = ec.message();
    return nullptr;
  }

  auto interfaceOrError = loadFile(std::move(errorOr.get()));
  if (!interfaceOrError) {
    errorMessage = toString(interfaceOrError.takeError());
    return nullptr;
  }

  auto *file = new LinkerInterfaceFile;
  if (file == nullptr) {
    errorMessage = "could not allocate memory";
    return nullptr;
  }

  std::shared_ptr<const InterfaceFile> interface =
      std::move(interfaceOrError.get());

  if (file->_pImpl->init(interface, cpuType, cpuSubType, flags, minOSVersion,
                         errorMessage)) {
    return file;
  }

  delete file;
  return nullptr;
}

const std::vector<std::pair<uint32_t, PackedVersion32>> &
LinkerInterfaceFile::getPlatformsAndMinDeployment() const noexcept {
  return _pImpl->_platformAndMinOS;
}

const std::vector<uint32_t>&
LinkerInterfaceFile::getPlatformSet() const noexcept {
  return _pImpl->_platforms;
}

const std::string &LinkerInterfaceFile::getInstallName() const noexcept {
  return _pImpl->_installName;
}

bool LinkerInterfaceFile::isInstallNameVersionSpecific() const noexcept {
  return _pImpl->_installPathOverride;
}

PackedVersion32 LinkerInterfaceFile::getCurrentVersion() const noexcept {
  return PackedVersion32(_pImpl->_currentVersion.rawValue());
}

PackedVersion32 LinkerInterfaceFile::getCompatibilityVersion() const noexcept {
  return PackedVersion32(_pImpl->_compatibilityVersion.rawValue());
}

unsigned LinkerInterfaceFile::getSwiftVersion() const noexcept {
  return _pImpl->_swiftABIVersion;
}

bool LinkerInterfaceFile::hasTwoLevelNamespace() const noexcept {
  return _pImpl->_hasTwoLevelNamespace;
}

bool LinkerInterfaceFile::isApplicationExtensionSafe() const noexcept {
  return _pImpl->_isAppExtensionSafe;
}

bool LinkerInterfaceFile::isNotForDyldSharedCache() const noexcept {
  return _pImpl->_isOSLibNotForSharedCache;
}

bool LinkerInterfaceFile::hasAllowableClients() const noexcept {
  return !_pImpl->_allowableClients.empty();
}

bool LinkerInterfaceFile::hasReexportedLibraries() const noexcept {
  return !_pImpl->_reexportedLibraries.empty();
}

bool LinkerInterfaceFile::hasWeakDefinedExports() const noexcept {
  return _pImpl->_hasWeakDefExports;
}

const std::string &
LinkerInterfaceFile::getParentFrameworkName() const noexcept {
  return _pImpl->_parentFrameworkName;
}

const std::vector<std::string> &
LinkerInterfaceFile::allowableClients() const noexcept {
  return _pImpl->_allowableClients;
}

const std::vector<std::string> &
LinkerInterfaceFile::reexportedLibraries() const noexcept {
  return _pImpl->_reexportedLibraries;
}

const std::vector<std::string> &LinkerInterfaceFile::rPaths() const noexcept {
  return _pImpl->_rPaths;
}

const std::vector<std::string> &
LinkerInterfaceFile::relinkedLibraries() const noexcept {
  return _pImpl->_relinkedLibraries;
}

const std::vector<std::string> &
LinkerInterfaceFile::ignoreExports() const noexcept {
  return _pImpl->_ignoreExports;
}

const std::vector<Symbol> &LinkerInterfaceFile::exports() const noexcept {
  return _pImpl->_exports;
}

const std::vector<Symbol> &LinkerInterfaceFile::undefineds() const noexcept {
  return _pImpl->_undefineds;
}

const std::vector<std::string> &
LinkerInterfaceFile::inlinedFrameworkNames() const noexcept {
  return _pImpl->_inlinedFrameworkNames;
}

LinkerInterfaceFile *LinkerInterfaceFile::getInlinedFramework(
    const std::string &installName, cpu_type_t cpuType,
    cpu_subtype_t cpuSubType, ParsingFlags flags, PackedVersion32 minOSVersion,
    std::string &errorMessage) const noexcept {

  auto it = std::find_if(_pImpl->_inlinedFrameworks.begin(),
                         _pImpl->_inlinedFrameworks.end(),
                         [&](const std::shared_ptr<const InterfaceFile> &it) {
                           return it->getInstallName() == installName;
                         });

  if (it == _pImpl->_inlinedFrameworks.end()) {
    errorMessage = "no such inlined framework";
    return nullptr;
  }

  auto file = new LinkerInterfaceFile;
  if (file == nullptr) {
    errorMessage = "could not allocate memory";
    return nullptr;
  }

  if (file->_pImpl->init(*it, cpuType, cpuSubType, flags, minOSVersion,
                         errorMessage))
    return file;

  delete file;
  return nullptr;
}

TAPI_NAMESPACE_V1_END
