//===--- TAPIOptions.td - Options for tapi --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the options accepted by tapi.
//
//===----------------------------------------------------------------------===//

// Include the common option parsing interfaces.
include "llvm/Option/OptParser.td"

/////////
// Flags

// DriverOption - The option is a "standard" option and shared by all tools.
def DriverOption : OptionFlag;

// ArchiveOption - The option is used by the archive driver.
def ArchiveOption : OptionFlag;

// StubOption - The option is used by the dynamic library stubifier.
def StubOption : OptionFlag;

// InstallAPIOption - The option is used by the installapi driver.
def InstallAPIOption : OptionFlag;

// ReexportOption - The option is used by the reexport driver.
def ReexportOption : OptionFlag;

// SDKDBOption - The options is used by the SDKDB driver.
def SDKDBOption : OptionFlag;

// SDKDBVerifyOption - The options is used by the SDKDB driver.
def SDKDBVerifyOption : OptionFlag;

// APIVerifyOption - The option is used by the API verification driver.
def APIVerifyOption : OptionFlag;

/////////
// Options

//
// Driver options
//
def version : Flag<["-", "--"], "version">, Flags<[DriverOption]>,
  HelpText<"Display the version">;
def help : Flag<["-", "--"], "help">, Flags<[DriverOption]>,
  HelpText<"Display this help message">;
def help_hidden : Flag<["-", "--"], "help-hidden">, Flags<[DriverOption, InstallAPIOption]>;
def snapshot : Flag<["--"], "snapshot">, Flags<[DriverOption]>,
  HelpText<"Force creation of a snapshot">;


//
// Archive options
//
def info : Flag<["--"], "info">, Flags<[ArchiveOption]>,
  HelpText<"List the architectures in the input file">;
def extract : Separate<["--"], "extract">, Flags<[ArchiveOption]>,
  HelpText<"Extract the specified <architecture>.">,
  MetaVarName<"<architecture>">;
def remove : Separate<["--"], "remove">, Flags<[ArchiveOption]>,
  HelpText<"Remove the specified <architecture>.">,
  MetaVarName<"<architecture>">;
def merge : Flag<["--"], "merge">, Flags<[ArchiveOption]>,
  HelpText<"Merge the input files.">;
def verify_arch : Separate<["--"], "verify-arch">, Flags<[ArchiveOption]>,
  HelpText<"Verify that the specified <architecture> is present in the file.">,
  MetaVarName<"<architecture>">;
def listSymbols : Flag<["--"], "list-symbols">, Flags<[ArchiveOption]>,
  HelpText<"List the exported symbols.">;
def allow_arch_merges : Flag<["--"], "allow-arch-merges">,
  Flags<[ArchiveOption]>, HelpText<"Allow merges of the same architecture.">;


//
// Shared Options
//
def target : Joined<["--"], "target=">,
  Flags<[InstallAPIOption, ReexportOption, SDKDBOption]>,
  HelpText<"Generate code for the given target">;
def target_legacy_spelling : Separate<["-"], "target">,
  Flags<[InstallAPIOption, ReexportOption, SDKDBOption]>,
  Alias<target>;
def arch : Separate<["-"], "arch">, Flags<[InstallAPIOption, ReexportOption, ArchiveOption, SDKDBOption]>,
  HelpText<"Specify the architecture to build for">,
  MetaVarName<"<architecture>">;

def mtargetos_EQ : Joined<["-"], "mtargetos=">,
  Flags<[InstallAPIOption, ReexportOption, SDKDBOption]>,
  HelpText<"Set the deployment target to be the specified platform and OS version">;

def mmacos_version_min_EQ : Joined<["-"], "mmacos-version-min=">,
  Flags<[InstallAPIOption, ReexportOption]>;
def mmacosx_version_min_EQ : Joined<["-"], "mmacosx-version-min=">,
  Flags<[InstallAPIOption, ReexportOption]>,
  Alias<mmacos_version_min_EQ>;
def macosx_version_min : Separate<["-"], "macosx_version_min">,
  Flags<[InstallAPIOption, ReexportOption]>,
  Alias<mmacos_version_min_EQ>;

def mios_version_min_EQ : Joined<["-"], "mios-version-min=">,
  Flags<[InstallAPIOption, ReexportOption]>;
def miphoneos_version_min_EQ : Joined<["-"], "miphoneos-version-min=">,
  Flags<[InstallAPIOption, ReexportOption]>,
  Alias<mios_version_min_EQ>;
def ios_version_min : Separate<["-"], "ios_version_min">,
  Flags<[InstallAPIOption, ReexportOption]>,
  Alias<mios_version_min_EQ>;

def mwatchos_version_min_EQ : Joined<["-"], "mwatchos-version-min=">,
  Flags<[InstallAPIOption, ReexportOption]>;
def watchos_version_min : Separate<["-"], "watchos_version_min">,
  Flags<[InstallAPIOption, ReexportOption]>,
  Alias<mwatchos_version_min_EQ>;

def mtvos_version_min_EQ : Joined<["-"], "mtvos-version-min=">,
  Flags<[InstallAPIOption, ReexportOption]>;
def tvos_version_min : Separate<["-"], "tvos_version_min">,
  Flags<[InstallAPIOption, ReexportOption]>,
  Alias<mtvos_version_min_EQ>;

def mbridgeos_version_min_EQ : Joined<["-"], "mbridgeos-version-min=">,
  Flags<[InstallAPIOption, ReexportOption]>;
def bridgeos_version_min : Separate<["-"], "bridgeos_version_min">,
  Flags<[InstallAPIOption, ReexportOption]>,
  Alias<mbridgeos_version_min_EQ>;

def mios_simulator_version_min_EQ : Joined<["-"], "mios-simulator-version-min=">,
  Flags<[InstallAPIOption, ReexportOption]>;
def ios_simulator_version_min : Separate<["-"], "ios_simulator_version_min">,
  Flags<[InstallAPIOption, ReexportOption]>,
  Alias<mios_simulator_version_min_EQ>;

def mwatchos_simulator_version_min_EQ : Joined<["-"], "mwatchos-simulator-version-min=">,
  Flags<[InstallAPIOption, ReexportOption]>;
def watchos_simulator_version_min : Separate<["-"], "watchos_simulator_version_min">,
  Flags<[InstallAPIOption, ReexportOption]>,
  Alias<mwatchos_simulator_version_min_EQ>;

def mtvos_simulator_version_min_EQ : Joined<["-"], "mtvos-simulator-version-min=">,
  Flags<[InstallAPIOption, ReexportOption]>;
def tvos_simulator_version_min : Separate<["-"], "tvos_simulator_version_min">,
  Flags<[InstallAPIOption, ReexportOption]>,
  Alias<mtvos_simulator_version_min_EQ>;


def x : JoinedOrSeparate<["-"], "x">, Flags<[SDKDBOption, InstallAPIOption, ReexportOption, APIVerifyOption]>,
  HelpText<"Treat subsequent input files as having type <language>">,
  MetaVarName<"<language>">;
def std_EQ : Joined<["-", "--"], "std=">, Flags<[SDKDBOption, InstallAPIOption, ReexportOption, APIVerifyOption]>,
  HelpText<"Language standard to compile for">;
def ObjC : Flag<["-"], "ObjC">, Flags<[SDKDBOption, InstallAPIOption, ReexportOption]>,
  HelpText<"Treat source input files as Objective-C inputs">;
def ObjCXX : Flag<["-"], "ObjC++">, Flags<[SDKDBOption, InstallAPIOption, ReexportOption]>,
  HelpText<"Treat source input files as Objective-C++ inputs">;
def isysroot : JoinedOrSeparate<["-"], "isysroot">,
  Flags<[StubOption, SDKDBOption, InstallAPIOption, ReexportOption]>,
  HelpText<"Set the system root directory (usually /)">, MetaVarName<"<dir>">;
def iframework : JoinedOrSeparate<["-"], "iframework">,
  Flags<[StubOption, SDKDBOption, InstallAPIOption, ReexportOption]>,
  HelpText<"Add directory to SYSTEM framework search path">;
def isystem : JoinedOrSeparate<["-"], "isystem">,
  Flags<[SDKDBOption, InstallAPIOption, ReexportOption]>,
  HelpText<"Add directory to SYSTEM include search path">, MetaVarName<"<directory>">;
def idirafter: JoinedOrSeparate<["-"], "idirafter">,
  Flags<[SDKDBOption, InstallAPIOption, ReexportOption]>,
  HelpText<"Add directory to AFTER include search path">, MetaVarName<"<directory>">;
def D : JoinedOrSeparate<["-"], "D">, Flags<[SDKDBOption, InstallAPIOption, ReexportOption]>,
  HelpText<"Define macro">;
def U : JoinedOrSeparate<["-"], "U">, Flags<[SDKDBOption, InstallAPIOption, ReexportOption]>,
  HelpText<"Undefine macro">;
def F : JoinedOrSeparate<["-"], "F">, Flags<[StubOption, SDKDBOption, InstallAPIOption, ReexportOption, APIVerifyOption]>,
  HelpText<"Add directory to framework include search path">;
def L : JoinedOrSeparate<["-"], "L">, Flags<[StubOption, InstallAPIOption]>,
  HelpText<"Add directory to the library search path">;
def I : JoinedOrSeparate<["-"], "I">, Flags<[SDKDBOption, InstallAPIOption, ReexportOption]>,
  HelpText<"Add directory to include search path">;
def X : Joined<["-"], "X">, Flags<[InstallAPIOption]>,
  HelpText<"Pass <arg> to run unique clang invocation identified as <label>">, 
  MetaVarName<"<label> <arg>">;
def iquote: JoinedOrSeparate<["-"], "iquote">, Flags<[InstallAPIOption,ReexportOption]>,
  HelpText<"Add directory to QUOTE include search path">, MetaVarName<"<directory>">;
def Xparser : Separate<["-"], "Xparser">, Flags<[SDKDBOption, InstallAPIOption, ReexportOption, APIVerifyOption]>,
  HelpText<"Pass <arg> to the clang parser">, MetaVarName<"<arg>">;
def output : Separate<["-"], "o">,
  Flags<[ArchiveOption, StubOption, InstallAPIOption, ReexportOption, SDKDBOption]>,
  MetaVarName<"<file>">, HelpText<"Write output to <file>">;
def extra_public_header : Separate<["-"], "extra-public-header">,
  Flags<[SDKDBOption,InstallAPIOption]>, MetaVarName<"<path>">,
  HelpText<"Add additional public header location for parsing">;
def extra_public_header_EQ : Joined<["--"], "extra-public-header=">,
  Flags<[SDKDBOption,InstallAPIOption]>, Alias<extra_public_header>;
def extra_private_header : Separate<["-"], "extra-private-header">,
  Flags<[SDKDBOption,InstallAPIOption]>, MetaVarName<"<path>">,
  HelpText<"Add additional private header location for parsing">;
def extra_private_header_EQ : Joined<["--"], "extra-private-header=">,
  Flags<[SDKDBOption,InstallAPIOption]>, Alias<extra_private_header>;
def exclude_public_header : Separate<["-"], "exclude-public-header">,
  Flags<[SDKDBOption,InstallAPIOption]>, MetaVarName<"<glob>">,
  HelpText<"Exclude public header from parsing">;
def exclude_public_header_EQ : Joined<["--"], "exclude-public-header=">,
  Flags<[SDKDBOption,InstallAPIOption]>, Alias<exclude_public_header>;
def exclude_private_header : Separate<["-"], "exclude-private-header">,
  Flags<[SDKDBOption,InstallAPIOption]>, MetaVarName<"<glob>">,
  HelpText<"Exclude private header from parsing">;
def exclude_private_header_EQ : Joined<["--"], "exclude-private-header=">,
  Flags<[SDKDBOption,InstallAPIOption]>, Alias<exclude_private_header>;
def public_umbrella_header : Separate<["-"], "public-umbrella-header">,
  Flags<[SDKDBOption,InstallAPIOption]>, MetaVarName<"<path>">,
  HelpText<"Specify the public umbrella header location">;
def public_umbrella_header_EQ : Joined<["--"], "public-umbrella-header=">,
  Flags<[SDKDBOption,InstallAPIOption]>, Alias<public_umbrella_header>;
def private_umbrella_header : Separate<["-"], "private-umbrella-header">,
  Flags<[SDKDBOption,InstallAPIOption]>, MetaVarName<"<path>">,
  HelpText<"Specify the private umbrella header location">;
def private_umbrella_header_EQ : Joined<["--"], "private-umbrella-header=">,
  Flags<[SDKDBOption,InstallAPIOption]>, Alias<private_umbrella_header>;
def frtti : Flag<["-"], "frtti">, Flags<[SDKDBOption,InstallAPIOption,ReexportOption]>;
def fno_rtti : Flag<["-"], "fno-rtti">, Flags<[SDKDBOption,InstallAPIOption,ReexportOption]>,
  HelpText<"Disable generation of rtti information">;
def fvisibility_EQ : Joined<["-"], "fvisibility=">,
  Flags<[SDKDBOption,InstallAPIOption,ReexportOption]>,
  HelpText<"Set the default symbol visibility for all global declarations">;
def inlinePrivateFrameworks : Flag<["--"], "inline-private-frameworks">,
  Flags<[StubOption]>, HelpText<"Inline private frameworks">;

def fmodules : Flag <["-"], "fmodules">, Flags<[SDKDBOption,InstallAPIOption]>,
  HelpText<"Enable the 'modules' language feature">;
def fmodules_cache_path : Joined<["-"], "fmodules-cache-path=">,
  Flags<[SDKDBOption,InstallAPIOption]>, MetaVarName<"<directory>">,
  HelpText<"Specify the module cache path">;
def fmodules_validate_system_headers : Flag<["-"], "fmodules-validate-system-headers">,
  Flags<[SDKDBOption,InstallAPIOption]>,
  HelpText<"Validate the system headers that a module depends on when loading the module">;

def fobjc_arc : Flag<["-"], "fobjc-arc">,
  Flags<[SDKDBOption,InstallAPIOption,ReexportOption]>,
  HelpText<"Synthesize retain and release calls for Objective-C pointers">;
def fobjc_weak : Flag<["-"], "fobjc-weak">,
  Flags<[SDKDBOption,InstallAPIOption,ReexportOption]>,
  HelpText<"Enable ARC-style weak references in Objective-C">;

def noUUIDs : Flag<["--"], "no-uuids">, Flags<[StubOption,InstallAPIOption]>,
  HelpText<"Don't record the UUIDs from the library in the text-based stub file">;

def filetype : Joined<["--"], "filetype=">, Flags<[StubOption,InstallAPIOption]>,
  HelpText<"Specify the output file type (tbd-v1, tbd-v2, tbd-v3, or tbd-v4)">;

def umbrella : Separate<["-"], "umbrella">,
  Flags<[InstallAPIOption]>, MetaVarName<"<framework_name>">,
  HelpText<"Specifies that the dylib being linked is re-exported through an umbrella framework of the specified name.">;

def Xarch__ : Joined<["-"], "Xarch_">, Flags<[InstallAPIOption]>;
def Xplatform__ : Joined<["-"], "Xplatform_">, Flags<[InstallAPIOption]>;
def Xproject : Joined<["-"], "Xproject">, Flags<[InstallAPIOption]>;

def inferIncludePaths : Flag<["--"], "infer-include-paths">, Flags<[InstallAPIOption]>,
  HelpText<"Infer include paths based on the provided/found header files">;
def noInferIncludePaths : Flag<["--"], "no-infer-include-paths">, Flags<[InstallAPIOption]>,
  HelpText<"Don't infer include paths based on the provided/found header files">;

def verbose : Flag<["-"], "v">, Flags<[SDKDBOption, InstallAPIOption, APIVerifyOption, ReexportOption]>,
  HelpText<"Verbose output, show scan content and driver options">;

//
// Stubifier options
//
def deleteInputFile : Flag<["--"], "delete-input-file">, Flags<[StubOption]>,
  HelpText<"Delete input file on success">;
def deletePrivateFrameworks : Flag<["--"], "delete-private-frameworks">,
  Flags<[StubOption]>, HelpText<"Delete private frameworks from the SDK">;
def setInstallAPI : Flag<["--"], "set-installapi-flag">, Flags<[StubOption]>,
  HelpText<"Set the installapi flag in the text-based stub file">;
def removeSharedCacheFlag: Flag<["--"], "remove-shared-cache-flag">, Flags<[StubOption]>,
  HelpText<"Remove flag for shared cache attribute">;

//
// InstallAPI options
//
def target_variant : Joined<["--"], "target-variant=">,
  Flags<[InstallAPIOption]>,
  HelpText<"Generate code for the given target variant">;
def target_variant_legacy_spelling : Separate<["-"], "target-variant">,
  Flags<[InstallAPIOption]>, Alias<target_variant>;
def install_name : Separate<["-"], "install_name">, Flags<[InstallAPIOption]>,
  HelpText<"Sets an internal \"install path\" (LC_ID_DYLIB) in a dynamic library">,
  MetaVarName<"<path>">;
def current_version : Separate<["-"], "current_version">,
  Flags<[InstallAPIOption]>,
  HelpText<"Specifies the current version number of the library">;
def compatibility_version : Separate<["-"], "compatibility_version">,
  Flags<[InstallAPIOption]>,
  HelpText<"Specifies the compatibility version number of the library">;
def allowable_client : Separate<["-"], "allowable_client">,
  Flags<[InstallAPIOption]>,
  HelpText<"Restricts what can link against the dynamic library being created">;
def rpath: Separate<["-"], "rpath">,
  Flags<[InstallAPIOption]>,
  HelpText<"Add path to the runpath search path list for the dynamic library being created.">;
def relinked_library: Separate<["-"], "relinked_library">,
  Flags<[InstallAPIOption]>,
  HelpText<"Add relinked library.">;
def reexport_install_name : Separate<["-"], "reexport_install_name">,
  MetaVarName<"<name>">, Flags<[InstallAPIOption]>,
  HelpText<"Re-export the specified internal \"install path\" (LC_ID_DYLIB)">;
def _serialize_diags : Separate<["-", "--"], "serialize-diagnostics">,
  Flags<[InstallAPIOption]>,
  HelpText<"Serialize compiler diagnostics to a file">;
def verify_against : Separate<["-"], "verify-against">,
  Flags<[InstallAPIOption]>,
  HelpText<"Verify the specified dynamic library/framework against the headers">;
def verify_against_EQ : Joined<["--"], "verify-against=">,
  Flags<[InstallAPIOption]>, Alias<verify_against>;
def verify_mode_EQ : Joined<["--"], "verify-mode=">,
  Flags<[InstallAPIOption]>,
  HelpText<"Specify the severity and extend of the validation. Valid modes are ErrorsOnly, ErrorsAndWarnings, and Pedantic.">;
def ferror_limit : Separate<["-"], "ferror-limit">, Flags<[InstallAPIOption]>,
  MetaVarName<"<N>">,
  HelpText<"Set the maximum number of errors to emit before stopping (0 = no limit).">;
def fapplication_extension : Flag<["-"], "fapplication-extension">,
  Flags<[InstallAPIOption]>,
  HelpText<"Restrict code to those available for App Extensions">;
def fno_application_extension : Flag<["-"], "fno-application-extension">,
  Flags<[InstallAPIOption]>;
def demangle : Flag<["--", "-"], "demangle">,
  Flags<[InstallAPIOption]>,
  HelpText<"Demangle C++ symbols when printing warnings and errors">;
def dynamiclib : Flag<["-"], "dynamiclib">, Flags<[InstallAPIOption]>;
def fprofile_instr_generate : Flag<["-"], "fprofile-instr-generate">,
  Flags<[InstallAPIOption]>,
  HelpText<"Add extra symbols for InstallAPI that are created by code coverage.">;
def print_after_EQ : Joined<["--"], "print-after=">,
    Flags<[InstallAPIOption]>;

def reexport_l : Joined<["-"], "reexport-l">,
  Flags<[InstallAPIOption]>, HelpText<"Re-export the specified library">;
def reexport_library : Separate<["-"], "reexport_library">, MetaVarName<"<path>">,
  Flags<[InstallAPIOption]>, HelpText<"Re-export the specified library">;
def reexport_framework : Separate<["-"], "reexport_framework">,
  Flags<[InstallAPIOption]>, HelpText<"Re-export the specified framework">;

def verify_api : Flag<["--"], "verify-api">, Flags<[InstallAPIOption, SDKDBOption]>,
  HelpText<"Verify the API of zippered frameworks">;
def no_verify_api : Flag<["--"], "no-verify-api">, Flags<[InstallAPIOption, SDKDBOption]>;
def verify_api_skip_external_headers : Flag<["--"], "verify-api-skip-external-headers">,
  Flags<[InstallAPIOption]>,
  HelpText<"Skip external headers for API verification of zippered frameworks">;
def no_verify_api_skip_external_headers : Flag<["--"], "no-verify-api-skip-external-headers">,
  Flags<[InstallAPIOption, SDKDBOption]>;
def verify_api_error_as_warning : Flag<["--"], "verify-api-error-as-warning">,
  Flags<[InstallAPIOption]>, HelpText<"Emit API Verification errors as warnings">;

def alias_list : Separate<["-"], "alias_list">,
  Flags<[InstallAPIOption]>, MetaVarName<"<path>">,
  HelpText<"Specify the path to a file containing a list of symbol aliases">;

def file_list_EQ : Joined<["--"], "filelist=">,
  Flags<[InstallAPIOption]>, MetaVarName<"<path>">,
  HelpText<"Specifies the <path> to a file that contains a list of headers to parse.">;
def file_list : Separate<["-"], "filelist">,
  Flags<[InstallAPIOption]>, Alias<file_list_EQ>;

def option_list : Separate<["-"], "optionlist">,
  Flags<[InstallAPIOption]>, MetaVarName<"<path>">, 
  HelpText<"Specifies the <path> to a file that contains X<label> arguments to parse.">;

def extra_project_header : Separate<["-"], "extra-project-header">,
  Flags<[InstallAPIOption]>, MetaVarName<"<path>">,
  HelpText<"Add additional project header location for parsing">;
def extra_project_header_EQ : Joined<["--"], "extra-project-header=">,
  Flags<[InstallAPIOption]>, Alias<extra_project_header>;
def exclude_project_header : Separate<["-"], "exclude-project-header">,
  Flags<[InstallAPIOption]>, MetaVarName<"<glob>">,
  HelpText<"Exclude project header from parsing">;
def exclude_project_header_EQ : Joined<["--"], "exclude-project-header=">,
  Flags<[InstallAPIOption]>, Alias<exclude_project_header>;

def ivfsoverlay : JoinedOrSeparate<["-"], "ivfsoverlay">,
  Flags<[InstallAPIOption]>,
  HelpText<"Overlay the virtual filesystem described by file over the real file system">;

def sdkdb_output_dir: Separate<["-"], "sdkdb-output-dir">,
  Flags<[InstallAPIOption]>,
  MetaVarName<"<path>">, HelpText<"Write SDKDB output to path">;

def swift_installapi_interface: JoinedOrSeparate<["-"], "swift-installapi-interface">,
  Flags<[InstallAPIOption]>, MetaVarName<"<path>">,
  HelpText<"Ignore symbols in verification against headers from specified file path for swift generated file.">;

def verify_swift: Flag<["-"], "verify-swift">,
  Flags<[InstallAPIOption]>,
  HelpText<"Verify all symbols from swift interface exist in dynamic library.">;

def dSYM : Joined<["--"], "dSYM=">, Flags<[InstallAPIOption]>,
  MetaVarName<"<path>">, HelpText<"Specify dSYM path.">;
def dsym_alternate_spelling : Joined<["--"], "dsym=">,
  Flags<[InstallAPIOption]>, Alias<dSYM>;

def product_name : Joined<["--"], "product-name=">, Flags<[InstallAPIOption]>,
  MetaVarName<"<name>">, HelpText<"Specify the product name">;

def t: Flag<["-"], "t">, Flags<[InstallAPIOption, StubOption]>,
  HelpText<"Logs each dylib tapi loads. Useful for debugging problems with search paths where the wrong library is loaded.">;

def include_pch : JoinedOrSeparate<["-", "--"], "include-pch">, Flags<[InstallAPIOption]>;
def include_ : JoinedOrSeparate<["-", "--"], "include">, Flags<[InstallAPIOption]>,
  MetaVarName<"<file>">, HelpText<"Include file before parsing, can only be used with -Xproject">;

def not_for_dyld_shared_cache : Joined<["-"], "not_for_dyld_shared_cache">, Flags<[InstallAPIOption]>,
  HelpText<"Mark library as shared cache ineligible">;

//
/// InstallAPI Options that are invalid to workaround project errors. 
/// rdar://116354828 ( tapi invalid drivers options: project failures)
//
def log : Joined<["-"], "log">, Flags<[InstallAPIOption]>;
def fvisibility_inlines: Joined<["-"], "fvisibility-inlines-hidden">, Flags<[InstallAPIOption]>;
def l: Joined<["-"], "l">, Flags<[InstallAPIOption]>;
def framework: Joined<["-"], "framework">, Flags<[InstallAPIOption]>;
def fnortt: Joined<["-"], "fno-rtt">, Flags<[InstallAPIOption]>;
def fno_exceptions: Joined<["-"], "fno-exceptions">, Flags<[InstallAPIOption]>;


//
// SDKDB options
//
def dylibs_only : Flag<["-"], "dylibs-only">,
  Flags<[SDKDBOption]>, HelpText<"Scan only Frameworks and UNIX libs">;
def sdkdb_action :  Joined<["-", "--"], "action=">, Flags<[SDKDBOption]>,
  HelpText<"Set SDKDB generation option: 'scan-interface', "
    "'gen-public', 'gen-private' or 'all'">,
  Values<"scan-interface,gen-public,gen-private,all">;
def public_headers : Flag<["-", "--"], "public-headers">,
  Flags<[SDKDBOption]>, HelpText<"Enable public header scanning">;
def no_public_headers : Flag<["-", "--"], "no-public-headers">,
  Flags<[SDKDBOption]>, HelpText<"Disable public header scanning">;
def private_headers : Flag<["-", "--"], "private-headers">,
  Flags<[SDKDBOption]>, HelpText<"Enable private header scanning">;
def no_private_headers : Flag<["-", "--"], "no-private-headers">,
  Flags<[SDKDBOption]>, HelpText<"Disable private header scanning">;
def config_file : Joined<["--"], "config-file=">, Flags<[SDKDBOption]>,
  MetaVarName<"<file>">, HelpText<"Read configuration from <file>">;
def sdk_content_root : Separate<["--"], "sdk-content-root">, Flags<[SDKDBOption]>,
  MetaVarName<"<SDKContentRoot>">, HelpText<"Path to SDKContentRoot">;
def public_sdk_content_root : Separate<["--"], "public-sdk-content-root">,
  Flags<[SDKDBOption]>, MetaVarName<"<PublicSDKContentRoot>">,
  HelpText<"Path to PublicSDKContentRoot">;
def runtime_root : Separate<["--"], "runtime-root">, Flags<[SDKDBOption]>,
  MetaVarName<"<RuntimeRoot>">, HelpText<"Path to RuntimeRoot">;
def dash_output: Separate<["--"], "output">, Flags<[SDKDBOption]>,
  Alias<output>;
def sdk: Separate<["--"], "sdk">, Flags<[SDKDBOption]>,
  Alias<isysroot>;
def diagnostics_file : Joined<["--"], "diagnostics-file=">,
  Flags<[SDKDBOption]>, MetaVarName<"<file>">,
  HelpText<"Write diagnostics to <file>">;
def partial_sdkdb_list : Separate<["--"], "partial-sdkdb-list">,
  Flags<[SDKDBOption]>, MetaVarName<"<filelist>">,
  HelpText<"Partial SDKDB filelist path">;
def installapi_sdkdb_path : Separate<["--"], "installapi-sdkdb-path">,
  Flags<[SDKDBOption]>, MetaVarName<"<directory>">,
  HelpText<"installapi SDKDB input directory (default to output directory)">;
