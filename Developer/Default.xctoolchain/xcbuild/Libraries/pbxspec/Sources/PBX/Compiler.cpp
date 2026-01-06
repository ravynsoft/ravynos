/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxspec/PBX/Compiler.h>
#include <pbxspec/Context.h>
#include <pbxspec/Inherit.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxspec::PBX::Compiler;

Compiler::
Compiler() :
    Tool()
{
}

Compiler::
~Compiler()
{
}

bool Compiler::
inherit(Specification::shared_ptr const &base)
{
    if (base->type() != Compiler::Type())
        return false;

    return inherit(std::static_pointer_cast<Compiler>(base));
}

bool Compiler::
inherit(Tool::shared_ptr const &base)
{
    if (base->type() != Compiler::Type())
        return false;

    return inherit(std::static_pointer_cast<Compiler>(base));
}

bool Compiler::
inherit(Compiler::shared_ptr const &b)
{
    if (!Tool::inherit(std::static_pointer_cast<Tool>(b)))
        return false;

    auto base = this->base();

    _execCPlusPlusLinkerPath                  = Inherit::Override(_execCPlusPlusLinkerPath, base->_execCPlusPlusLinkerPath);
    _executionDescription                     = Inherit::Override(_executionDescription, base->_executionDescription);
    _sourceFileOption                         = Inherit::Override(_sourceFileOption, base->_sourceFileOption);
    _outputDir                                = Inherit::Override(_outputDir, base->_outputDir);
    _outputFileExtension                      = Inherit::Override(_outputFileExtension, base->_outputFileExtension);
    _commandResultsPostprocessor              = Inherit::Override(_commandResultsPostprocessor, base->_commandResultsPostprocessor);
    _genericCommandFailedErrorString          = Inherit::Override(_genericCommandFailedErrorString, base->_genericCommandFailedErrorString);
    _generatedInfoPlistContentFilePath        = Inherit::Override(_generatedInfoPlistContentFilePath, base->_generatedInfoPlistContentFilePath);
    _dependencyInfoFile                       = Inherit::Override(_dependencyInfoFile, base->_dependencyInfoFile);
    _dependencyInfoArgs                       = Inherit::Combine(_dependencyInfoArgs, base->_dependencyInfoArgs);
    _languages                                = Inherit::Combine(_languages, base->_languages);
    _optionConditionFlavors                   = Inherit::Combine(_optionConditionFlavors, base->_optionConditionFlavors);
    _patternsOfFlagsNotAffectingPrecomps      = Inherit::Combine(_patternsOfFlagsNotAffectingPrecomps, base->_patternsOfFlagsNotAffectingPrecomps);
    _messageCategoryInfoOptions               = Inherit::Combine(_messageCategoryInfoOptions, base->_messageCategoryInfoOptions);
    _synthesizeBuildRuleForBuildPhases        = Inherit::Combine(_synthesizeBuildRuleForBuildPhases, base->_synthesizeBuildRuleForBuildPhases);
    _inputFileGroupings                       = Inherit::Combine(_inputFileGroupings, base->_inputFileGroupings);
    _fallbackTools                            = Inherit::Combine(_fallbackTools, base->_fallbackTools);
    _additionalDirectoriesToCreate            = Inherit::Combine(_additionalDirectoriesToCreate, base->_additionalDirectoriesToCreate);
    _overridingProperties                     = Inherit::Combine(_overridingProperties, base->_overridingProperties);
    _useCPlusPlusCompilerDriverWhenBundlizing = Inherit::Override(_useCPlusPlusCompilerDriverWhenBundlizing, base->_useCPlusPlusCompilerDriverWhenBundlizing);
    _supportsHeadermaps                       = Inherit::Override(_supportsHeadermaps, base->_supportsHeadermaps);
    _supportsIsysroot                         = Inherit::Override(_supportsIsysroot, base->_supportsIsysroot);
    _supportsSeparateUserHeaderPaths          = Inherit::Override(_supportsSeparateUserHeaderPaths, base->_supportsSeparateUserHeaderPaths);
    _supportsGeneratePreprocessedFile         = Inherit::Override(_supportsGeneratePreprocessedFile, base->_supportsGeneratePreprocessedFile);
    _supportsGenerateAssemblyFile             = Inherit::Override(_supportsGenerateAssemblyFile, base->_supportsGenerateAssemblyFile);
    _supportsAnalyzeFile                      = Inherit::Override(_supportsAnalyzeFile, base->_supportsAnalyzeFile);
    _supportsSerializedDiagnostics            = Inherit::Override(_supportsSerializedDiagnostics, base->_supportsSerializedDiagnostics);
    _supportsPredictiveCompilation            = Inherit::Override(_supportsPredictiveCompilation, base->_supportsPredictiveCompilation);
    _supportsMacOSXDeploymentTarget           = Inherit::Override(_supportsMacOSXDeploymentTarget, base->_supportsMacOSXDeploymentTarget);
    _supportsMacOSXMinVersionFlag             = Inherit::Override(_supportsMacOSXMinVersionFlag, base->_supportsMacOSXMinVersionFlag);
    _prunePrecompiledHeaderCache              = Inherit::Override(_prunePrecompiledHeaderCache, base->_prunePrecompiledHeaderCache);
    _outputAreProducts                        = Inherit::Override(_outputAreProducts, base->_outputAreProducts);
    _outputAreSourceFiles                     = Inherit::Override(_outputAreSourceFiles, base->_outputAreSourceFiles);
    _softError                                = Inherit::Override(_softError, base->_softError);
    _deeplyStatInputDirectories               = Inherit::Override(_deeplyStatInputDirectories, base->_deeplyStatInputDirectories);
    _dontProcessOutputs                       = Inherit::Override(_dontProcessOutputs, base->_dontProcessOutputs);
    _showInCompilerSelectionPopup             = Inherit::Override(_showInCompilerSelectionPopup, base->_showInCompilerSelectionPopup);
    _showOnlySelfDefinedProperties            = Inherit::Override(_showOnlySelfDefinedProperties, base->_showOnlySelfDefinedProperties);
    _mightNotEmitAllOutputs                   = Inherit::Override(_mightNotEmitAllOutputs, base->_mightNotEmitAllOutputs);
    _includeInUnionedToolDefaults             = Inherit::Override(_includeInUnionedToolDefaults, base->_includeInUnionedToolDefaults);

    return true;
}

Compiler::shared_ptr Compiler::
Parse(Context *context, plist::Dictionary const *dict)
{
    if (!ParseType(context, dict, Type())) {
        return nullptr;
    }

    Compiler::shared_ptr result;
    result.reset(new Compiler());

    std::unordered_set<std::string> seen;
    if (!result->parse(context, dict, &seen, true))
        return nullptr;

    return result;
}

bool Compiler::
parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Tool::parse(context, dict, seen, false))
        return false;

    auto unpack = plist::Keys::Unpack("Compiler", dict, seen);

    auto ED       = unpack.cast <plist::String> ("ExecutionDescription");
    auto ECPPLP   = unpack.cast <plist::String> ("ExecCPlusPlusLinkerPath");
    auto SFO      = unpack.cast <plist::String> ("SourceFileOption");
    auto OD       = unpack.cast <plist::String> ("OutputDir");
    auto OFE      = unpack.cast <plist::String> ("OutputFileExtension");
    auto CRP      = unpack.cast <plist::String> ("CommandResultsPostprocessor");
    auto GCFES    = unpack.cast <plist::String> ("GenericCommandFailedErrorString");
    auto GIPCFP   = unpack.cast <plist::String> ("GeneratedInfoPlistContentFilePath");
    auto DIF      = unpack.cast <plist::String> ("DependencyInfoFile");
    auto DIAs     = unpack.cast <plist::Array> ("DependencyInfoArgs");
    auto Ls       = unpack.cast <plist::Array> ("Languages");
    auto OCFs     = unpack.cast <plist::Array> ("OptionConditionFlavors");
    auto POFNAPs  = unpack.cast <plist::Array> ("PatternsOfFlagsNotAffectingPrecomps");
    auto MCIOs    = unpack.cast <plist::Array> ("MessageCategoryInfoOptions");
    auto SBRFBPs  = unpack.cast <plist::Array> ("SynthesizeBuildRuleForBuildPhases");
    auto IFGs     = unpack.cast <plist::Array> ("InputFileGroupings");
    auto FTs      = unpack.cast <plist::Array> ("FallbackTools");
    auto ADTCs    = unpack.cast <plist::Array> ("AdditionalDirectoriesToCreate");
    auto OP       = unpack.cast <plist::Dictionary> ("OverridingProperties");
    auto UCPPCDWB = unpack.coerce <plist::Boolean> ("UseCPlusPlusCompilerDriverWhenBundlizing");
    auto DIFAH    = unpack.coerce <plist::Boolean> ("DashIFlagAcceptsHeadermaps");
    auto SH       = unpack.coerce <plist::Boolean> ("SupportsHeadermaps");
    auto SI       = unpack.coerce <plist::Boolean> ("SupportsIsysroot");
    auto SSUHP    = unpack.coerce <plist::Boolean> ("SupportsSeparateUserHeaderPaths");
    auto SGPF     = unpack.coerce <plist::Boolean> ("SupportsGeneratePreprocessedFile");
    auto SGAF     = unpack.coerce <plist::Boolean> ("SupportsGenerateAssemblyFile");
    auto SAF      = unpack.coerce <plist::Boolean> ("SupportsAnalyzeFile");
    auto SSD      = unpack.coerce <plist::Boolean> ("SupportsSerializedDiagnostics");
    auto SPC      = unpack.coerce <plist::Boolean> ("SupportsPredictiveCompilation");
    auto SMOSXDT  = unpack.coerce <plist::Boolean> ("SupportsMacOSXDeploymentTarget");
    auto SMOSXMVF = unpack.coerce <plist::Boolean> ("SupportsMacOSXMinVersionFlag");
    auto PPHC     = unpack.coerce <plist::Boolean> ("PrunePrecompiledHeaderCache");
    auto OAP      = unpack.coerce <plist::Boolean> ("OutputsAreProducts");
    auto OASF     = unpack.coerce <plist::Boolean> ("OutputsAreSourceFiles");
    auto SE       = unpack.coerce <plist::Boolean> ("SoftError");
    auto DSID     = unpack.coerce <plist::Boolean> ("DeeplyStatInputDirectories");
    auto DPO      = unpack.coerce <plist::Boolean> ("DontProcessOutputs");
    auto SICSP    = unpack.coerce <plist::Boolean> ("ShowInCompilerSelectionPopup");
    auto SOSDP    = unpack.coerce <plist::Boolean> ("ShowOnlySelfDefinedProperties");
    auto MNEAO    = unpack.coerce <plist::Boolean> ("MightNotEmitAllOutputs");
    auto IIUTD    = unpack.coerce <plist::Boolean> ("IncludeInUnionedToolDefaults");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (ECPPLP != nullptr) {
        _execCPlusPlusLinkerPath = ECPPLP->value();
    }

    if (ED != nullptr) {
        _executionDescription = ED->value();
    }

    if (SFO != nullptr) {
        _sourceFileOption = SFO->value();
    }

    if (OD != nullptr) {
        _outputDir = pbxsetting::Value::Parse(OD->value());
    }

    if (OFE != nullptr) {
        _outputFileExtension = OFE->value();
    }

    if (CRP != nullptr) {
        _commandResultsPostprocessor = CRP->value();
    }

    if (GCFES != nullptr) {
        _genericCommandFailedErrorString = GCFES->value();
    }

    if (GIPCFP != nullptr) {
        _generatedInfoPlistContentFilePath = pbxsetting::Value::Parse(GIPCFP->value());
    }

    if (DIF != nullptr) {
        _dependencyInfoFile = pbxsetting::Value::Parse(DIF->value());
    }

    if (DIAs != nullptr) {
        _dependencyInfoArgs = std::vector<pbxsetting::Value>();
        for (size_t n = 0; n < DIAs->count(); n++) {
            if (auto DIA = DIAs->value <plist::String> (n)) {
                _dependencyInfoArgs->push_back(pbxsetting::Value::Parse(DIA->value()));
            }
        }
    }

    if (Ls != nullptr) {
        _languages = std::vector<std::string>();
        for (size_t n = 0; n < Ls->count(); n++) {
            if (auto L = Ls->value <plist::String> (n)) {
                _languages->push_back(L->value());
            }
        }
    }

    if (OCFs != nullptr) {
        _optionConditionFlavors = std::vector<std::string>();
        for (size_t n = 0; n < OCFs->count(); n++) {
            if (auto OCF = OCFs->value <plist::String> (n)) {
                _optionConditionFlavors->push_back(OCF->value());
            }
        }
    }

    if (POFNAPs != nullptr) {
        _patternsOfFlagsNotAffectingPrecomps = std::vector<std::string>();
        for (size_t n = 0; n < POFNAPs->count(); n++) {
            if (auto POFNAP = POFNAPs->value <plist::String> (n)) {
                _patternsOfFlagsNotAffectingPrecomps->push_back(POFNAP->value());
            }
        }
    }

    if (MCIOs != nullptr) {
        _messageCategoryInfoOptions = std::vector<std::string>();
        for (size_t n = 0; n < MCIOs->count(); n++) {
            if (auto MCIO = MCIOs->value <plist::String> (n)) {
                _messageCategoryInfoOptions->push_back(MCIO->value());
            }
        }
    }

    if (SBRFBPs != nullptr) {
        _synthesizeBuildRuleForBuildPhases = std::vector<std::string>();
        for (size_t n = 0; n < SBRFBPs->count(); n++) {
            if (auto SBRFBP = SBRFBPs->value <plist::String> (n)) {
                _synthesizeBuildRuleForBuildPhases->push_back(SBRFBP->value());
            }
        }
    }

    if (IFGs != nullptr) {
        _inputFileGroupings = std::vector<std::string>();
        for (size_t n = 0; n < IFGs->count(); n++) {
            if (auto IFG = IFGs->value <plist::String> (n)) {
                _inputFileGroupings->push_back(IFG->value());
            }
        }
    }

    if (FTs != nullptr) {
        _fallbackTools = std::vector<std::string>();
        for (size_t n = 0; n < FTs->count(); n++) {
            if (auto FT = FTs->value <plist::String> (n)) {
                _fallbackTools->push_back(FT->value());
            }
        }
    }

    if (ADTCs != nullptr) {
        _additionalDirectoriesToCreate = std::vector<pbxsetting::Value>();
        for (size_t n = 0; n < ADTCs->count(); n++) {
            if (auto ADTC = ADTCs->value <plist::String> (n)) {
                _additionalDirectoriesToCreate->push_back(pbxsetting::Value::Parse(ADTC->value()));
            }
        }
    }

    if (OP != nullptr) {
        std::vector<pbxsetting::Setting> settings;
        for (size_t n = 0; n < OP->count(); n++) {
            auto OPK = OP->key(n);
            auto OPV = OP->value <plist::String> (OPK);

            if (OPV != nullptr) {
                pbxsetting::Setting setting = pbxsetting::Setting::Parse(OPK, OPV->value());
                settings.push_back(setting);
            }
        }
        _overridingProperties = pbxsetting::Level(settings);
    }

    if (UCPPCDWB != nullptr) {
        _useCPlusPlusCompilerDriverWhenBundlizing = UCPPCDWB->value();
    }

    if (SH != nullptr) {
        _supportsHeadermaps = SH->value();
    }

    if (SI != nullptr) {
        _supportsIsysroot = SI->value();
    }

    if (SSUHP != nullptr) {
        _supportsSeparateUserHeaderPaths = SSUHP->value();
    }

    if (SGPF != nullptr) {
        _supportsGeneratePreprocessedFile = SGPF->value();
    }

    if (SGAF != nullptr) {
        _supportsGenerateAssemblyFile = SGAF->value();
    }

    if (SAF != nullptr) {
        _supportsAnalyzeFile = SAF->value();
    }

    if (SSD != nullptr) {
        _supportsSerializedDiagnostics = SSD->value();
    }

    if (SPC != nullptr) {
        _supportsPredictiveCompilation = SPC->value();
    }

    if (SMOSXDT != nullptr) {
        _supportsMacOSXDeploymentTarget = SMOSXDT->value();
    }

    if (SMOSXMVF != nullptr) {
        _supportsMacOSXMinVersionFlag = SMOSXMVF->value();
    }

    if (PPHC != nullptr) {
        _prunePrecompiledHeaderCache = PPHC->value();
    }

    if (OAP != nullptr) {
        _outputAreProducts = OAP->value();
    }

    if (OASF != nullptr) {
        _outputAreSourceFiles = OASF->value();
    }

    if (SE != nullptr) {
        _softError = SE->value();
    }

    if (DSID != nullptr) {
        _deeplyStatInputDirectories = DSID->value();
    }

    if (DPO != nullptr) {
        _dontProcessOutputs = DPO->value();
    }

    if (SICSP != nullptr) {
        _showInCompilerSelectionPopup = SICSP->value();
    }

    if (SOSDP != nullptr) {
        _showOnlySelfDefinedProperties = SOSDP->value();
    }

    if (MNEAO != nullptr) {
        _mightNotEmitAllOutputs = MNEAO->value();
    }

    if (IIUTD != nullptr) {
        _includeInUnionedToolDefaults = IIUTD->value();
    }

    return true;
}
