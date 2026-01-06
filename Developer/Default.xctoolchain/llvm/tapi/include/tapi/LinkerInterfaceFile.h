//===-- tapi/LinkerInterfaceFile.h - TAPI File Interface --------*- C++ -*-===*\
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief API for reading TAPI files.
/// \since 1.0
///
//===----------------------------------------------------------------------===//
#ifndef TAPI_LINKER_INTERFACE_FILE_H
#define TAPI_LINKER_INTERFACE_FILE_H

#include <memory>
#include <string>
#include <tapi/Defines.h>
#include <vector>

///
/// \defgroup TAPI_LINKER_INTERFACE_FILE TAPI File APIs
/// \ingroup TAPI_CPP_API
///
/// @{
///

using cpu_type_t = int;
using cpu_subtype_t = int;

TAPI_NAMESPACE_V1_BEGIN

class PackedVersion32;
class Symbol;

///
/// \brief Defines flags that control the parsing of text-based stub files.
/// \since 1.1
///
enum ParsingFlags : unsigned {
  /// \brief Default flags.
  /// \since 1.1
  None = 0,

  /// \brief Only accept a slice if the sub type matches. ABI fall-back mode is
  ///        the default.
  /// \since 1.1
  ExactCpuSubType = 1U << 0,

  /// \brief Disallow weak imported symbols. This adds weak imported symbols to
  ///        the ignore exports list.
  /// \since 1.1
  DisallowWeakImports = 1U << 1,
};

inline ParsingFlags operator|(ParsingFlags lhs, ParsingFlags rhs) noexcept {
  return static_cast<ParsingFlags>(static_cast<unsigned>(lhs) |
                                   static_cast<unsigned>(rhs));
}

inline ParsingFlags operator|=(ParsingFlags &lhs, ParsingFlags rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

///
/// \brief TAPI File APIs
/// \since 1.0
///
class TAPI_PUBLIC LinkerInterfaceFile {
public:
  ///
  /// \brief Returns a list of supported file extensions.
  ///
  /// \returns a list of supported file extensions.
  /// \since 1.0
  ///
  static std::vector<std::string> getSupportedFileExtensions() noexcept;

  ///
  /// \brief Indicate if the provided buffer is a supported Text-based Dynamic
  ///        Library Stub file.
  ///
  /// Checks if the buffer is a supported format. This doesn't check for
  /// malformed buffer content.
  ///
  /// \param[in] path full path to the file.
  /// \param[in] data raw pointer to start of buffer.
  /// \param[in] size size of the buffer in bytes.
  /// \returns true if the format is supported.
  /// \since 1.0
  ///
  static bool isSupported(const std::string &path, const uint8_t *data,
                          size_t size) noexcept;

  ///
  /// \brief Check if we should prefer the text-based stub file.
  ///
  /// \param[in] path full path to the text-based stub file.
  /// \returns true if the text-based stub file should be prefered over any
  ///          dynamic library.
  /// \since 1.0
  /// \deprecated 2.1
  ///
  static bool shouldPreferTextBasedStubFile(const std::string &path) noexcept
      __attribute__((deprecated));
  ///
  /// \brief Check if the text-based stub file and the MachO dynamic library
  ///        file are in sync.
  ///
  /// This validates both files against each other and checks if both files are
  /// still in sync.
  ///
  /// \param[in] tbdPath full path to the text-based stub file.
  /// \param[in] dylibPath full path to the MachO dynamic library file.
  /// \returns true if both files are in sync.
  /// \since 1.0
  /// \deprecated 2.1
  ///
  static bool areEquivalent(const std::string &tbdPath,
                            const std::string &dylibPath) noexcept
      __attribute__((deprecated));

  ///
  /// \brief Create a LinkerInterfaceFile from a file.
  ///
  /// Parses the content of the file with the given constrains for cpu type,
  /// cpu sub-type, flags, and minimum deployment version.
  ///
  /// \param[in] path path to the file.
  /// \param[in] cpuType The cpu type / architecture to check the file for.
  /// \param[in] cpuSubType The cpu sub type / sub architecture to check the
  ///            file for.
  /// \param[in] flags Flags that control the parsing behavior.
  /// \param[in] minOSVersion The minimum OS version / deployment target.
  /// \param[out] errorMessage holds an error message when the return value is a
  ///             nullptr.
  /// \return nullptr on error
  /// \since 1.3
  ///
  static LinkerInterfaceFile *
  create(const std::string &path, cpu_type_t cpuType, cpu_subtype_t cpuSubType,
         ParsingFlags flags, PackedVersion32 minOSVersion,
         std::string &errorMessage) noexcept;

  ///
  /// \brief Query the supported platforms
  /// \return Returns the set of platforms supported by the TAPI file as
  ///         defined by the MachO load command LC_BUILD_VERSION.
  /// \since 1.6
  /// \deprecated 2.2
  ///
  const std::vector<uint32_t> &getPlatformSet() const noexcept
      __attribute__((deprecated));

  ///
  /// \brief Query the minimum deployment version for matching platforms.
  /// \return Returns the set of platforms supported by the TAPI file as
  ///         defined by the MachO load command LC_BUILD_VERSION and
  ///         matching minimum deployment version.
  /// \since 2.2
  ///
  const std::vector<std::pair<uint32_t, PackedVersion32>> &
  getPlatformsAndMinDeployment() const noexcept;

  ///
  /// \brief Query the install name.
  /// \return Returns the install name of the TAPI file.
  /// \since 1.0
  ///
  const std::string &getInstallName() const noexcept;

  ///
  /// \brief Query the install name is version specifc.
  /// \return True if the install name has been adjusted for the provided
  ///         minimum OS version.
  /// \since 1.0
  ///
  bool isInstallNameVersionSpecific() const noexcept;

  ///
  /// \brief Query the current library version.
  /// \return Returns the current library version as 32bit packed version.
  /// \since 1.0
  ///
  PackedVersion32 getCurrentVersion() const noexcept;

  ///
  /// \brief Query the compatibility library version.
  /// \return Returns the compatibility library version as 32bit packed version.
  /// \since 1.0
  ///
  PackedVersion32 getCompatibilityVersion() const noexcept;

  ///
  /// \brief Query the Swift ABI version.
  /// \return Returns the Swift ABI version as unsigned integer.
  /// \since 1.0
  ///
  unsigned getSwiftVersion() const noexcept;

  ///
  /// \brief Query if the library has two level namespace.
  /// \return Returns true if the library has two level namespace.
  /// \since 1.0
  ///
  bool hasTwoLevelNamespace() const noexcept;

  ///
  /// \brief Query if the library is Applicatiuon Extension Safe.
  /// \return Returns true if the library is Application Extension Safe.
  /// \since 1.0
  ///
  bool isApplicationExtensionSafe() const noexcept;

  ///
  /// \brief Query if the library has set not_for_dyld_shared_cache.
  /// Which is only applicable to system libraries.
  /// \return Returns true if the library is shared cache ineligible.
  /// \since 2.3
  ///
  bool isNotForDyldSharedCache() const noexcept;

  ///
  /// \brief Query if the library has any allowable clients.
  /// \return Return true if there are any allowable clients.
  /// \since 1.0
  ///
  bool hasAllowableClients() const noexcept;

  ///
  /// \brief Query if the library has any re-exported libraries.
  /// \return Return true if there are any re-exported libraries.
  /// \since 1.0
  ///
  bool hasReexportedLibraries() const noexcept;

  ///
  /// \brief Query if the library has any weak defined exports.
  /// \return Return true if there are any weak defined exports.
  /// \since 1.0
  ///
  bool hasWeakDefinedExports() const noexcept;

  ///
  /// \brief Obtain the name of the parent framework (umbrella framework).
  /// \return Returns the name of the parent framework (if it exists), otherwise
  ///         an empty string.
  /// \since 1.0
  ///
  const std::string &getParentFrameworkName() const noexcept;

  ///
  /// \brief Obtain the list of allowable clients.
  /// \return Returns a list of allowable clients.
  /// \since 1.0
  ///
  const std::vector<std::string> &allowableClients() const noexcept;

  ///
  /// \brief Obtain the list of re-exported libraries.
  /// \return Returns a list of re-exported libraries.
  /// \since 1.0
  ///
  const std::vector<std::string> &reexportedLibraries() const noexcept;

  ///
  /// \brief Obtain the list of run path search paths.
  /// \return Returns a list of run path search paths.
  /// \since 2.2
  ///
  const std::vector<std::string> &rPaths() const noexcept;

  ///
  /// \brief Obtain the list of relinked libraries.
  /// \return Returns a list of relinked libraries.
  /// \since 2.2
  ///
  const std::vector<std::string> &relinkedLibraries() const noexcept;

  ///
  /// \brief Obtain a list of all symbols to be ignored.
  /// \return Returns a list of all symbols that should be ignored.
  /// \since 1.0
  ///
  const std::vector<std::string> &ignoreExports() const noexcept;

  ///
  /// \brief Obtain a list of all exported symbols.
  /// \return Returns a list of all exported symbols.
  /// \since 1.0
  ///
  const std::vector<Symbol> &exports() const noexcept;

  ///
  /// \brief Obtain a list of all undefined symbols.
  /// \return Returns a list of all undefined symbols.
  /// \since 1.0
  ///
  const std::vector<Symbol> &undefineds() const noexcept;

  ///
  /// \brief Obtain a list of all inlined frameworks.
  /// \return Returns a list of install names of all inlined frameworks.
  /// \since 1.3
  ///
  const std::vector<std::string> &inlinedFrameworkNames() const noexcept;

  ///
  /// \brief Create a LinkerInterfaceFile from the specified inlined framework.
  ///
  /// Creates a LinkerInterfaceFile with the given constrains for cpu type,
  /// cpu sub-type, flags, and minimum deployment version.
  ///
  /// \param[in] installName install name of the inlined framework.
  /// \param[in] cpuType The cpu type / architecture to check the file for.
  /// \param[in] cpuSubType The cpu sub type / sub architecture to check the
  ///            file for.
  /// \param[in] flags Flags that control the parsing behavior.
  /// \param[in] minOSVersion The minimum OS version / deployment target.
  /// \param[out] errorMessage holds an error message when the return value is a
  ///             nullptr.
  /// \return nullptr on error
  /// \since 1.3
  ///
  LinkerInterfaceFile *
  getInlinedFramework(const std::string &installName, cpu_type_t cpuType,
                      cpu_subtype_t cpuSubType, ParsingFlags flags,
                      PackedVersion32 minOSVersion,
                      std::string &errorMessage) const noexcept;

  ///
  /// \brief Destructor.
  /// \since 1.0
  ///
  ~LinkerInterfaceFile() noexcept;

  ///
  /// \brief Copy constructor (deleted).
  /// \since 1.0
  ///
  LinkerInterfaceFile(const LinkerInterfaceFile &) noexcept = delete;
  LinkerInterfaceFile &operator=(const LinkerInterfaceFile &) noexcept = delete;

  ///
  /// \brief Move constructor.
  /// \since 1.0
  ///
  LinkerInterfaceFile(LinkerInterfaceFile &&) noexcept;
  LinkerInterfaceFile &operator=(LinkerInterfaceFile &&) noexcept;

private:
  LinkerInterfaceFile() noexcept;

  class Impl;
  std::unique_ptr<Impl> _pImpl;
};

TAPI_NAMESPACE_V1_END

#endif // TAPI_LINKER_INTERFACE_FILE_H
