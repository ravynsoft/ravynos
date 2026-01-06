/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxspec_PBX_Compiler_h
#define __pbxspec_PBX_Compiler_h

#include <pbxspec/PBX/Tool.h>
#include <pbxsetting/Level.h>
#include <pbxsetting/Value.h>

#include <ext/optional>

namespace pbxspec { namespace PBX {

class Compiler : public Tool {
public:
    typedef std::shared_ptr <Compiler> shared_ptr;
    typedef std::vector <shared_ptr> vector;

protected:
    ext::optional<std::string>                    _execCPlusPlusLinkerPath;
    ext::optional<std::string>                    _executionDescription;
    ext::optional<std::string>                    _sourceFileOption;
    ext::optional<pbxsetting::Value>              _outputDir;
    ext::optional<std::string>                    _outputFileExtension;
    ext::optional<std::string>                    _commandResultsPostprocessor;
    ext::optional<std::string>                    _genericCommandFailedErrorString;
    ext::optional<pbxsetting::Value>              _generatedInfoPlistContentFilePath;
    ext::optional<pbxsetting::Value>              _dependencyInfoFile;
    ext::optional<std::vector<pbxsetting::Value>> _dependencyInfoArgs;
    ext::optional<std::vector<std::string>>       _languages;
    ext::optional<std::vector<std::string>>       _optionConditionFlavors;
    ext::optional<std::vector<std::string>>       _patternsOfFlagsNotAffectingPrecomps;
    ext::optional<std::vector<std::string>>       _messageCategoryInfoOptions;
    ext::optional<std::vector<std::string>>       _synthesizeBuildRuleForBuildPhases;
    ext::optional<std::vector<std::string>>       _inputFileGroupings;
    ext::optional<std::vector<std::string>>       _fallbackTools;
    ext::optional<std::vector<pbxsetting::Value>> _additionalDirectoriesToCreate;
    ext::optional<pbxsetting::Level>              _overridingProperties;
    ext::optional<bool>                           _useCPlusPlusCompilerDriverWhenBundlizing;
    ext::optional<bool>                           _dashIFlagAcceptHeadermaps;
    ext::optional<bool>                           _supportsHeadermaps;
    ext::optional<bool>                           _supportsIsysroot;
    ext::optional<bool>                           _supportsSeparateUserHeaderPaths;
    ext::optional<bool>                           _supportsGeneratePreprocessedFile;
    ext::optional<bool>                           _supportsGenerateAssemblyFile;
    ext::optional<bool>                           _supportsAnalyzeFile;
    ext::optional<bool>                           _supportsSerializedDiagnostics;
    ext::optional<bool>                           _supportsPredictiveCompilation;
    ext::optional<bool>                           _supportsMacOSXDeploymentTarget;
    ext::optional<bool>                           _supportsMacOSXMinVersionFlag;
    ext::optional<bool>                           _prunePrecompiledHeaderCache;
    ext::optional<bool>                           _outputAreProducts;
    ext::optional<bool>                           _outputAreSourceFiles;
    ext::optional<bool>                           _softError;
    ext::optional<bool>                           _deeplyStatInputDirectories;
    ext::optional<bool>                           _dontProcessOutputs;
    ext::optional<bool>                           _showInCompilerSelectionPopup;
    ext::optional<bool>                           _showOnlySelfDefinedProperties;
    ext::optional<bool>                           _mightNotEmitAllOutputs;
    ext::optional<bool>                           _includeInUnionedToolDefaults;

protected:
    Compiler();

public:
    virtual ~Compiler();

public:
    inline SpecificationType type() const override
    { return Compiler::Type(); }

public:
    inline Compiler::shared_ptr base() const
    { return std::static_pointer_cast<Compiler>(Tool::base()); }

public:
    inline ext::optional<std::string> const &execCPlusPlusLinkerPath() const
    { return _execCPlusPlusLinkerPath; }

public:
    inline ext::optional<std::string> const &executionDescription() const
    { return _executionDescription; }

public:
    inline ext::optional<std::string> const &sourceFileOption() const
    { return _sourceFileOption; }

public:
    inline ext::optional<pbxsetting::Value> const &outputDir() const
    { return _outputDir; }
    inline ext::optional<std::string> const &outputFileExtension() const
    { return _outputFileExtension; }

public:
    inline ext::optional<std::string> const &commandResultsPostprocessor() const
    { return _commandResultsPostprocessor; }

public:
    inline ext::optional<std::string> const &genericCommandFailedErrorString() const
    { return _genericCommandFailedErrorString; }

public:
    inline ext::optional<pbxsetting::Value> const &generatedInfoPlistContentFilePath() const
    { return _generatedInfoPlistContentFilePath; }

public:
    inline ext::optional<pbxsetting::Value> const &dependencyInfoFile() const
    { return _dependencyInfoFile; }
    inline ext::optional<std::vector<pbxsetting::Value>> const &dependencyInfoArgs() const
    { return _dependencyInfoArgs; }

public:
    inline ext::optional<std::vector<std::string>> const &languages() const
    { return _languages; }

public:
    inline ext::optional<std::vector<std::string>> const &optionConditionFlavors() const
    { return _optionConditionFlavors; }

public:
    inline ext::optional<std::vector<std::string>> const &patternsOfFlagsNotAffectingPrecomps() const
    { return _patternsOfFlagsNotAffectingPrecomps; }

public:
    inline ext::optional<std::vector<std::string>> const &messageCategoryInfoOptions() const
    { return _messageCategoryInfoOptions; }

public:
    inline ext::optional<std::vector<std::string>> const &synthesizeBuildRuleForBuildPhases() const
    { return _synthesizeBuildRuleForBuildPhases; }

public:
    inline ext::optional<std::vector<std::string>> const &inputFileGroupings() const
    { return _inputFileGroupings; }

public:
    inline ext::optional<std::vector<std::string>> const &fallbackTools() const
    { return _fallbackTools; }

public:
    inline ext::optional<std::vector<pbxsetting::Value>> const &additionalDirectoriesToCreate() const
    { return _additionalDirectoriesToCreate; }

public:
    inline ext::optional<pbxsetting::Level> const &overridingProperties() const
    { return _overridingProperties; }

public:
    inline bool useCPlusPlusCompilerDriverWhenBundlizing() const
    { return _useCPlusPlusCompilerDriverWhenBundlizing.value_or(false); }
    inline ext::optional<bool> useCPlusPlusCompilerDriverWhenBundlizingOptional() const
    { return _useCPlusPlusCompilerDriverWhenBundlizing; }

public:
    inline bool dashIFlagAcceptHeadermaps() const
    { return _dashIFlagAcceptHeadermaps.value_or(false); }
    inline ext::optional<bool> dashIFlagAcceptHeadermapsOptional() const
    { return _dashIFlagAcceptHeadermaps; }
    inline bool supportsHeadermaps() const
    { return _supportsHeadermaps.value_or(false); }
    inline ext::optional<bool> supportsHeadermapsOptional() const
    { return _supportsHeadermaps; }
    inline bool supportsIsysroot() const
    { return _supportsIsysroot.value_or(false); }
    inline ext::optional<bool> supportsIsysrootOptional() const
    { return _supportsIsysroot; }
    inline bool supportsSeparateUserHeaderPaths() const
    { return _supportsSeparateUserHeaderPaths.value_or(false); }
    inline ext::optional<bool> supportsSeparateUserHeaderPathsOptional() const
    { return _supportsSeparateUserHeaderPaths; }
    inline bool supportsGeneratePreprocessedFile() const
    { return _supportsGeneratePreprocessedFile.value_or(false); }
    inline ext::optional<bool> supportsGeneratePreprocessedFileOptional() const
    { return _supportsGeneratePreprocessedFile; }
    inline bool supportsGenerateAssemblyFile() const
    { return _supportsGenerateAssemblyFile.value_or(false); }
    inline ext::optional<bool> supportsGenerateAssemblyFileOptional() const
    { return _supportsGenerateAssemblyFile; }
    inline bool supportsAnalyzeFile() const
    { return _supportsAnalyzeFile.value_or(false); }
    inline ext::optional<bool> supportsAnalyzeFileOptional() const
    { return _supportsAnalyzeFile; }
    inline bool supportsSerializedDiagnostics() const
    { return _supportsSerializedDiagnostics.value_or(false); }
    inline ext::optional<bool> supportsSerializedDiagnosticsOptional() const
    { return _supportsSerializedDiagnostics; }
    inline bool supportsPredictiveCompilation() const
    { return _supportsPredictiveCompilation.value_or(false); }
    inline ext::optional<bool> supportsPredictiveCompilationOptional() const
    { return _supportsPredictiveCompilation; }
    inline bool supportsMacOSXDeploymentTarget() const
    { return _supportsMacOSXDeploymentTarget.value_or(false); }
    inline ext::optional<bool> supportsMacOSXDeploymentTargetOptional() const
    { return _supportsMacOSXDeploymentTarget; }
    inline bool supportsMacOSXMinVersionFlag() const
    { return _supportsMacOSXMinVersionFlag.value_or(false); }
    inline ext::optional<bool> supportsMacOSXMinVersionFlagOptional() const
    { return _supportsMacOSXMinVersionFlag; }

public:
    inline bool prunePrecompiledHeaderCache() const
    { return _prunePrecompiledHeaderCache.value_or(false); }
    inline ext::optional<bool> prunePrecompiledHeaderCacheOptional() const
    { return _prunePrecompiledHeaderCache; }

public:
    inline bool outputAreProducts() const
    { return _outputAreProducts.value_or(false); }
    inline ext::optional<bool> outputAreProductsOptional() const
    { return _outputAreProducts; }
    inline bool outputAreSourceFiles() const
    { return _outputAreSourceFiles.value_or(false); }
    inline ext::optional<bool> outputAreSourceFilesOptional() const
    { return _outputAreSourceFiles; }

public:
    inline bool softError() const
    { return _softError.value_or(false); }
    inline ext::optional<bool> softErrorOptional() const
    { return _softError; }

public:
    inline bool deeplyStatInputDirectories() const
    { return _deeplyStatInputDirectories.value_or(false); }
    inline ext::optional<bool> deeplyStatInputDirectoriesOptional() const
    { return _deeplyStatInputDirectories; }

public:
    inline bool dontProcessOutputs() const
    { return _dontProcessOutputs.value_or(false); }
    inline ext::optional<bool> dontProcessOutputsOptional() const
    { return _dontProcessOutputs; }

public:
    inline bool showInCompilerSelectionPopup() const
    { return _showInCompilerSelectionPopup.value_or(false); }
    inline ext::optional<bool> showInCompilerSelectionPopupOptional() const
    { return _showInCompilerSelectionPopup; }
    inline bool showOnlySelfDefinedProperties() const
    { return _showOnlySelfDefinedProperties.value_or(false); }
    inline ext::optional<bool> showOnlySelfDefinedPropertiesOptional() const
    { return _showOnlySelfDefinedProperties; }

public:
    inline bool mightNotEmitAllOutputs() const
    { return _mightNotEmitAllOutputs.value_or(false); }
    inline ext::optional<bool> mightNotEmitAllOutputsOptional() const
    { return _mightNotEmitAllOutputs; }

public:
    inline bool includeInUnionedToolDefaults() const
    { return _includeInUnionedToolDefaults.value_or(false); }
    inline ext::optional<bool> includeInUnionedToolDefaultsOptional() const
    { return _includeInUnionedToolDefaults; }

protected:
    friend class Specification;
    bool parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

protected:
    bool inherit(Specification::shared_ptr const &base) override;
    bool inherit(Tool::shared_ptr const &base) override;
    virtual bool inherit(Compiler::shared_ptr const &base);

protected:
    static Compiler::shared_ptr Parse(Context *context, plist::Dictionary const *dict);

public:
    static inline SpecificationType Type()
    { return SpecificationType::Compiler; }
};

} }

#endif  // !__pbxspec_PBX_Compiler_h
