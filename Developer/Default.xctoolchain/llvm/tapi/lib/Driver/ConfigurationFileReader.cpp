//===- ConfigurationFileReader.cpp - Configuration File Reader --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the configuration file reader.
///
//===----------------------------------------------------------------------===//

#include "tapi/Driver/ConfigurationFileReader.h"

#include "tapi/Core/LLVM.h"
#include "tapi/Driver/ConfigurationFile.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/YAMLTraits.h"

using namespace llvm;
using namespace llvm::yaml;
using namespace TAPI_INTERNAL;
using namespace TAPI_INTERNAL::configuration::v1;

LLVM_YAML_IS_FLOW_SEQUENCE_VECTOR(Macro)
LLVM_YAML_IS_SEQUENCE_VECTOR(FrameworkConfiguration)
LLVM_YAML_IS_SEQUENCE_VECTOR(ProjectConfiguration)

namespace llvm {
namespace yaml {

template <> struct ScalarTraits<Macro> {
  static void output(const Macro &macro, void * /*unused*/, raw_ostream &out) {
    out << (macro.second ? "-U" : "-D") << macro.first;
  }

  static StringRef input(StringRef scalar, void * /*unused*/, Macro &value) {
    if (scalar.startswith("-D")) {
      value = std::make_pair(scalar.drop_front(2).str(), false);
      return {};
    }

    if (scalar.startswith("-U")) {
      value = std::make_pair(scalar.drop_front(2).str(), true);
      return {};
    }

    return {"invalid macro"};
  }

  static QuotingType mustQuote(StringRef /*unused*/) {
    return QuotingType::None;
  }
};

using PackedVersion = llvm::MachO::PackedVersion;
template <> struct ScalarTraits<PackedVersion> {
  static void output(const PackedVersion &value, void *, raw_ostream &os) {
    os << value;
  }

  static StringRef input(StringRef scalar, void *, PackedVersion &value) {
    if (!value.parse32(scalar))
      return "invalid packed version string.";
    return {};
  }

  static QuotingType mustQuote(StringRef) { return QuotingType::None; }
};

using llvm::MachO::PlatformType;
template <> struct ScalarEnumerationTraits<PlatformType> {
  static void enumeration(IO &io, PlatformType &platform) {
    using namespace llvm::MachO;
    io.enumCase(platform, "unknown", PLATFORM_UNKNOWN);
    io.enumCase(platform, "macosx", PLATFORM_MACOS);
    io.enumCase(platform, "ios", PLATFORM_IOS);
    io.enumCase(platform, "ios", PLATFORM_IOSSIMULATOR);
    io.enumCase(platform, "watchos", PLATFORM_WATCHOS);
    io.enumCase(platform, "watchos", PLATFORM_WATCHOSSIMULATOR);
    io.enumCase(platform, "tvos", PLATFORM_TVOS);
    io.enumCase(platform, "tvos", PLATFORM_TVOSSIMULATOR);
    io.enumCase(platform, "bridgeos", PLATFORM_BRIDGEOS);
  }
};

template <> struct ScalarEnumerationTraits<clang::Language> {
  static void enumeration(IO &io, clang::Language &Lang) {
    using namespace clang;
    io.enumCase(Lang, "c", Language::C);
    io.enumCase(Lang, "cxx", Language::CXX);
    io.enumCase(Lang, "objective-c", Language::ObjC);
    io.enumCase(Lang, "objective-cxx", Language::ObjCXX);
    io.enumCase(Lang, "unknown", Language::Unknown);
  }
};

template <> struct MappingTraits<HeaderConfiguration> {
  static void mapping(IO &io, HeaderConfiguration &config) {
    io.mapOptional("umbrella", config.umbrellaHeader);
    io.mapOptional("pre-includes", config.preIncludes);
    io.mapOptional("includes", config.includes);
    io.mapOptional("excludes", config.excludes);
  }
};

template <> struct MappingTraits<FrameworkConfiguration> {
  static void mapping(IO &io, FrameworkConfiguration &config) {
    io.mapRequired("name", config.name);
    io.mapRequired("path", config.path);
    io.mapOptional("install-name", config.installName);
    io.mapOptional("language", config.language, defaultLanguage);
    io.mapOptional("include-paths", config.includePaths);
    io.mapOptional("framework-paths", config.frameworkPaths);
    io.mapOptional("macros", config.macros);
    io.mapOptional("public-header", config.publicHeaderConfiguration);
    io.mapOptional("private-header", config.privateHeaderConfiguration);
    io.mapOptional("scan-swift-module", config.scanSwiftModule);
    io.mapOptional("use-overlay", config.useOverlay, true);
    io.mapOptional("clang-extra-args", config.clangExtraArgs);
  }
};

template <> struct MappingTraits<ProjectConfiguration> {
  static void mapping(IO &io, ProjectConfiguration &config) {
    io.mapRequired("name", config.name);
    io.mapOptional("language", config.language, defaultLanguage);
    io.mapOptional("language-std", config.languageStd);
    io.mapOptional("include-paths", config.includePaths);
    io.mapOptional("framework-paths", config.frameworkPaths);
    io.mapOptional("macros", config.macros);
    io.mapOptional("iosmac", config.isiOSMac);
    io.mapOptional("zippered", config.isZippered);
    io.mapOptional("scan-swift-module", config.scanSwiftModule);
    io.mapOptional("use-overlay", config.useOverlay, true);
    io.mapOptional("iosmac-umbrella-only", config.useUmbrellaOnly);
    io.mapOptional("scan-public-headers-in-sdk-content-root", config.scanPublicHeadersInSDKContentRoot);
    io.mapOptional("ignore-existing-partial-sdkdbs",
                   config.ignoreExistingPartialSDKDBs);
    io.mapOptional("root-mask", config.rootMaskPaths);
    io.mapOptional("sdk-mask", config.sdkMaskPaths);
    io.mapOptional("public-header", config.publicHeaderConfiguration);
    io.mapOptional("private-header", config.privateHeaderConfiguration);
    io.mapOptional("use-split-header-dir", config.useSplitHeaderDir);
    io.mapOptional("clang-extra-args", config.clangExtraArgs);
  }
};

template <> struct MappingTraits<ConfigurationFile> {
  static void mapping(IO &io, ConfigurationFile &file) {
    io.mapTag("tapi-configuration-v1", true);
    io.mapOptional("sdk-platform", file.platform, MachO::PLATFORM_UNKNOWN);
    io.mapOptional("sdk-version", file.version);
    io.mapOptional("sdk-root", file.isysroot);
    io.mapOptional("language", file.language, clang::Language::ObjC);
    io.mapOptional("include-paths", file.includePaths);
    io.mapOptional("framework-paths", file.frameworkPaths);
    io.mapOptional("public-dylibs", file.publicDylibs);
    io.mapOptional("macros", file.macros);
    io.mapOptional("frameworks", file.frameworkConfigurations);
    io.mapOptional("projects", file.projectConfigurations);
  }
};

} // end namespace yaml.
} // end namespace llvm.

TAPI_NAMESPACE_INTERNAL_BEGIN

class ConfigurationFileReader::Implementation {
public:
  MemoryBufferRef inputBuffer;
  ConfigurationFile configFile;
  Error parse(StringRef input);
};

Error ConfigurationFileReader::Implementation::parse(StringRef input) {
  auto str = input.trim();
  if (!(str.startswith("---\n") ||
        str.startswith("--- !tapi-configuration-v1\n")) ||
      !str.endswith("..."))
    return make_error<StringError>("invalid input file",
                                   inconvertibleErrorCode());

  yaml::Input yin(input);
  yin >> configFile;

  if (yin.error())
    return make_error<StringError>("malformed file\n", yin.error());
  return Error::success();
}

ConfigurationFileReader::ConfigurationFileReader(MemoryBufferRef inputBuffer,
                                                 Error &error)
    : impl(*new ConfigurationFileReader::Implementation()) {
  ErrorAsOutParameter errorAsOutParam(&error);
  impl.inputBuffer = std::move(inputBuffer);

  error = impl.parse(impl.inputBuffer.getBuffer());
}

Expected<std::unique_ptr<ConfigurationFileReader>>
ConfigurationFileReader::get(MemoryBufferRef inputBuffer) {
  Error error = Error::success();
  std::unique_ptr<ConfigurationFileReader> reader(
      new ConfigurationFileReader(inputBuffer, error));
  if (error)
    return std::move(error);

  return reader;
}

ConfigurationFileReader::~ConfigurationFileReader() { delete &impl; }

ConfigurationFile ConfigurationFileReader::takeConfigurationFile() {
  return std::move(impl.configFile);
}

TAPI_NAMESPACE_INTERNAL_END
