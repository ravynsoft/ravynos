/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxspec/PBX/FileType.h>
#include <pbxspec/PBX/BuildPhaseInjection.h>
#include <pbxspec/Context.h>
#include <pbxspec/Inherit.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Data.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxspec::PBX::FileType;

FileType::
FileType() :
    Specification()
{
}

FileType::
~FileType()
{
}

bool FileType::
inherit(Specification::shared_ptr const &base)
{
    if (base->type() != FileType::Type())
        return false;

    return inherit(std::static_pointer_cast<FileType>(base));
}

bool FileType::
inherit(FileType::shared_ptr const &b)
{
    if (!Specification::inherit(b))
        return false;

    auto base = this->base();

    _uti                                     = Inherit::Override(_uti, base->_uti);
    _extensions                              = _extensions; /* Explicitly not inherited. */
    _mimeTypes                               = Inherit::Combine(_mimeTypes, base->_mimeTypes);
    _typeCodes                               = Inherit::Combine(_typeCodes, base->_typeCodes);
    _filenamePatterns                        = Inherit::Combine(_filenamePatterns, base->_filenamePatterns);
    _magicWords                              = Inherit::Combine(_magicWords, base->_magicWords);
    _language                                = Inherit::Override(_language, base->_language);
    _computerLanguage                        = Inherit::Override(_computerLanguage, base->_computerLanguage);
    _gccDialectName                          = Inherit::Override(_gccDialectName, base->_gccDialectName);
    _prefix                                  = Inherit::Combine(_prefix, base->_prefix);
    _permissions                             = Inherit::Override(_permissions, base->_permissions);
    _buildPhaseInjectionsWhenEmbedding       = Inherit::Combine(_buildPhaseInjectionsWhenEmbedding, base->_buildPhaseInjectionsWhenEmbedding);
    _appliesToBuildRules                     = Inherit::Override(_appliesToBuildRules, base->_appliesToBuildRules);
    _isTextFile                              = Inherit::Override(_isTextFile, base->_isTextFile);
    _isBuildPropertiesFile                   = Inherit::Override(_isBuildPropertiesFile, base->_isBuildPropertiesFile);
    _isSourceCode                            = Inherit::Override(_isSourceCode, base->_isSourceCode);
    _isSwiftSourceCode                       = Inherit::Override(_isSwiftSourceCode, base->_isSwiftSourceCode);
    _isDocumentation                         = Inherit::Override(_isDocumentation, base->_isDocumentation);
    _isPreprocessed                          = Inherit::Override(_isPreprocessed, base->_isPreprocessed);
    _isTransparent                           = Inherit::Override(_isTransparent, base->_isTransparent);
    _isEmbeddable                            = Inherit::Override(_isEmbeddable, base->_isEmbeddable);
    _isExecutable                            = Inherit::Override(_isExecutable, base->_isExecutable);
    _isExecutableWithGUI                     = Inherit::Override(_isExecutableWithGUI, base->_isExecutableWithGUI);
    _isApplication                           = Inherit::Override(_isApplication, base->_isApplication);
    _isBundle                                = Inherit::Override(_isBundle, base->_isBundle);
    _isLibrary                               = Inherit::Override(_isLibrary, base->_isLibrary);
    _isDynamicLibrary                        = Inherit::Override(_isDynamicLibrary, base->_isDynamicLibrary);
    _isStaticLibrary                         = Inherit::Override(_isStaticLibrary, base->_isStaticLibrary);
    _isFolder                                = Inherit::Override(_isFolder, base->_isFolder);
    _isWrappedFolder                         = Inherit::Override(_isWrappedFolder, base->_isWrappedFolder);
    _isScannedForIncludes                    = Inherit::Override(_isScannedForIncludes, base->_isScannedForIncludes);
    _isFrameworkWrapper                      = Inherit::Override(_isFrameworkWrapper, base->_isFrameworkWrapper);
    _isStaticFrameworkWrapper                = Inherit::Override(_isStaticFrameworkWrapper, base->_isStaticFrameworkWrapper);
    _isProjectWrapper                        = Inherit::Override(_isProjectWrapper, base->_isProjectWrapper);
    _isTargetWrapper                         = Inherit::Override(_isTargetWrapper, base->_isTargetWrapper);
    _componentParts                          = Inherit::Combine(_componentParts, base->_componentParts);
    _extraPropertyNames                      = Inherit::Combine(_extraPropertyNames, base->_extraPropertyNames);
    _includeInIndex                          = Inherit::Override(_includeInIndex, base->_includeInIndex);
    _canSetIncludeInIndex                    = Inherit::Override(_canSetIncludeInIndex, base->_canSetIncludeInIndex);
    _requiresHardTabs                        = Inherit::Override(_requiresHardTabs, base->_requiresHardTabs);
    _containsNativeCode                      = Inherit::Override(_containsNativeCode, base->_containsNativeCode);
    _plistStructureDefinition                = Inherit::Override(_plistStructureDefinition, base->_plistStructureDefinition);
    _changesCauseDependencyGraphInvalidation = Inherit::Override(_changesCauseDependencyGraphInvalidation, base->_changesCauseDependencyGraphInvalidation);
    _fallbackAutoroutingBuildPhase           = Inherit::Override(_fallbackAutoroutingBuildPhase, base->_fallbackAutoroutingBuildPhase);
    _codeSignOnCopy                          = Inherit::Override(_codeSignOnCopy, base->_codeSignOnCopy);
    _removeHeadersOnCopy                     = Inherit::Override(_removeHeadersOnCopy, base->_removeHeadersOnCopy);
    _validateOnCopy                          = Inherit::Override(_validateOnCopy, base->_validateOnCopy);

    return true;
}

FileType::shared_ptr FileType::
Parse(Context *context, plist::Dictionary const *dict)
{
    if (!ParseType(context, dict, Type())) {
        return nullptr;
    }

    FileType::shared_ptr result;
    result.reset(new FileType());

    std::unordered_set<std::string> seen;
    if (!result->parse(context, dict, &seen, true))
        return nullptr;

    return result;
}

bool FileType::
parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Specification::parse(context, dict, seen, false))
        return false;

    auto unpack = plist::Keys::Unpack("FileType", dict, seen);

    auto MTs   = unpack.cast <plist::Array> ("MIMETypes");
    auto U     = unpack.cast <plist::String> ("UTI");
    auto Es    = unpack.cast <plist::Array> ("Extensions");
    auto TCs   = unpack.cast <plist::Array> ("TypeCodes");
    auto FPs   = unpack.cast <plist::Array> ("FilenamePatterns");
    auto MWs   = unpack.cast <plist::Array> ("MagicWord");
    auto L     = unpack.cast <plist::String> ("Language");
    auto CL    = unpack.cast <plist::String> ("ComputerLanguage");
    auto GDN   = unpack.cast <plist::String> ("GccDialectName");
    auto Ps    = unpack.cast <plist::Array> ("Prefix");
    auto P     = unpack.cast <plist::String> ("Permissions");
    auto BPIWE = unpack.cast <plist::Array> ("BuildPhaseInjectionsWhenEmbedding");
    auto ATBR  = unpack.coerce <plist::Boolean> ("AppliesToBuildRules");
    auto ITF   = unpack.coerce <plist::Boolean> ("IsTextFile");
    auto IBPF  = unpack.coerce <plist::Boolean> ("IsBuildPropertiesFile");
    auto ISC   = unpack.coerce <plist::Boolean> ("IsSourceCode");
    auto ISSC  = unpack.coerce <plist::Boolean> ("IsSwiftSourceCode");
    auto IP    = unpack.coerce <plist::Boolean> ("IsPreprocessed");
    auto IT    = unpack.coerce <plist::Boolean> ("IsTransparent");
    auto ID    = unpack.coerce <plist::Boolean> ("IsDocumentation");
    auto IEM   = unpack.coerce <plist::Boolean> ("IsEmbeddable");
    auto IE    = unpack.coerce <plist::Boolean> ("IsExecutable");
    auto IEWG  = unpack.coerce <plist::Boolean> ("IsExecutableWithGUI");
    auto IA    = unpack.coerce <plist::Boolean> ("IsApplication");
    auto IB    = unpack.coerce <plist::Boolean> ("IsBundle");
    auto IL    = unpack.coerce <plist::Boolean> ("IsLibrary");
    auto IDL   = unpack.coerce <plist::Boolean> ("IsDynamicLibrary");
    auto ISL   = unpack.coerce <plist::Boolean> ("IsStaticLibrary");
    auto IF    = unpack.coerce <plist::Boolean> ("IsFolder");
    auto IWF   = unpack.coerce <plist::Boolean> ("IsWrapperFolder");
    auto ISFI  = unpack.coerce <plist::Boolean> ("IsScannedForIncludes");
    auto IFW   = unpack.coerce <plist::Boolean> ("IsFrameworkWrapper");
    auto ISFW  = unpack.coerce <plist::Boolean> ("IsStaticFrameworkWrapper");
    auto IPW   = unpack.coerce <plist::Boolean> ("IsProjectWrapper");
    auto ITW   = unpack.coerce <plist::Boolean> ("IsTargetWrapper");
    auto EPNs  = unpack.cast <plist::Array> ("ExtraPropertyNames");
    auto CPs   = unpack.cast <plist::Dictionary> ("ComponentParts");
    auto III   = unpack.coerce <plist::Boolean> ("IncludeInIndex");
    auto CSIII = unpack.coerce <plist::Boolean> ("CanSetIncludeInIndex");
    auto RHT   = unpack.coerce <plist::Boolean> ("RequiresHardTabs");
    auto CNC   = unpack.coerce <plist::Boolean> ("ContainsNativeCode");
    auto PDS   = unpack.cast <plist::String> ("PlistStructureDefinition");
    auto CCDGI = unpack.coerce <plist::Boolean> ("ChangesCauseDependencyGraphInvalidation");
    auto FABP  = unpack.cast <plist::String> ("FallbackAutoroutingBuildPhase");
    auto CSOC  = unpack.coerce <plist::Boolean> ("CodeSignOnCopy");
    auto RHOC  = unpack.coerce <plist::Boolean> ("RemoveHeadersOnCopy");
    auto VOC   = unpack.coerce <plist::Boolean> ("ValidateOnCopy");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (U != nullptr) {
        _uti = U->value();
    }

    if (Es != nullptr) {
        _extensions = std::vector<std::string>();
        for (size_t n = 0; n < Es->count(); n++) {
            auto E = Es->value <plist::String> (n);
            if (E != nullptr) {
                _extensions->push_back(E->value());
            }
        }
    }

    if (MTs != nullptr) {
        _mimeTypes = std::vector<std::string>();
        for (size_t n = 0; n < MTs->count(); n++) {
            auto MT = MTs->value <plist::String> (n);
            if (MT != nullptr) {
                _mimeTypes->push_back(MT->value());
            }
        }
    }

    if (TCs != nullptr) {
        _typeCodes = std::vector<std::string>();
        for (size_t n = 0; n < TCs->count(); n++) {
            auto TC = TCs->value <plist::String> (n);
            if (TC != nullptr) {
                _typeCodes->push_back(TC->value());
            }
        }
    }

    if (FPs != nullptr) {
        _filenamePatterns = std::vector<std::string>();
        for (size_t n = 0; n < FPs->count(); n++) {
            auto FP = FPs->value <plist::String> (n);
            if (FP != nullptr) {
                _filenamePatterns->push_back(FP->value());
            }
        }
    }

    if (MWs != nullptr) {
        _magicWords = std::vector<std::vector<uint8_t>>();
        for (size_t n = 0; n < MWs->count(); n++) {
            if (auto MW = MWs->value <plist::String> (n)) {
                std::vector<uint8_t> MWV = std::vector<uint8_t>(MW->value().begin(), MW->value().end());
                _magicWords->push_back(MWV);
            } else if (auto MW = MWs->value <plist::Data> (n)) {
                _magicWords->push_back(MW->value());
            }
        }
    }

    if (L != nullptr) {
        _language = L->value();
    }

    if (CL != nullptr) {
        _computerLanguage = CL->value();
    }

    if (GDN != nullptr) {
        _gccDialectName = GDN->value();
    }

    if (Ps != nullptr) {
        _prefix = std::vector<std::string>();
        for (size_t n = 0; n < Ps->count(); n++) {
            auto P = Ps->value <plist::String> (n);
            if (P != nullptr) {
                _prefix->push_back(P->value());
            }
        }
    }

    if (P != nullptr) {
        _permissions = P->value();
    }

    if (BPIWE != nullptr) {
        _buildPhaseInjectionsWhenEmbedding = std::vector<BuildPhaseInjection>();
        for (size_t n = 0; n < BPIWE->count(); n++) {
            auto BPI = BPIWE->value <plist::Dictionary> (n);
            if (BPI != nullptr) {
                BuildPhaseInjection injection;
                if (injection.parse(BPI)) {
                    _buildPhaseInjectionsWhenEmbedding->push_back(injection);
                }
            }
        }
    }

    if (ATBR != nullptr) {
        _appliesToBuildRules = ATBR->value();
    }

    if (IT != nullptr) {
        _isTransparent = IT->value();
    }

    if (ID != nullptr) {
        _isDocumentation = ID->value();
    }

    if (IBPF != nullptr) {
        _isBuildPropertiesFile = IBPF->value();
    }

    if (ISC != nullptr) {
        _isSourceCode = ISC->value();
    }

    if (ISSC != nullptr) {
        _isSwiftSourceCode = ISSC->value();
    }

    if (IP != nullptr) {
        _isPreprocessed = IP->value();
    }

    if (IT != nullptr) {
        _isTransparent = IT->value();
    }

    if (IEM != nullptr) {
        _isEmbeddable = IEM->value();
    }

    if (IE != nullptr) {
        _isExecutable = IE->value();
    }

    if (IEWG != nullptr) {
        _isExecutableWithGUI = IEWG->value();
    }

    if (IA != nullptr) {
        _isApplication = IA->value();
    }

    if (IB != nullptr) {
        _isBundle = IB->value();
    }

    if (IL != nullptr) {
        _isLibrary = IL->value();
    }

    if (IDL != nullptr) {
        _isDynamicLibrary = IDL->value();
    }

    if (ISL != nullptr) {
        _isStaticLibrary = ISL->value();
    }

    if (IF != nullptr) {
        _isFolder = IF->value();
    }

    if (IWF != nullptr) {
        _isWrappedFolder = IWF->value();
    }

    if (IFW != nullptr) {
        _isFrameworkWrapper = IFW->value();
    }

    if (ISFW != nullptr) {
        _isStaticFrameworkWrapper = ISFW->value();
    }

    if (IPW != nullptr) {
        _isProjectWrapper = IPW->value();
    }

    if (ITW != nullptr) {
        _isTargetWrapper = ITW->value();
    }

    if (EPNs != nullptr) {
        _extraPropertyNames = std::vector<std::string>();
        for (size_t n = 0; n < EPNs->count(); n++) {
            auto EPN = EPNs->value <plist::String> (n);
            if (EPN != nullptr) {
                _extraPropertyNames->push_back(EPN->value());
            }
        }
    }

    if (CPs != nullptr) {
        _componentParts = std::vector<ComponentPart>();
        for (size_t n = 0; n < CPs->count(); n++) {
            auto K  = CPs->key(n);
            auto CP = CPs->value <plist::Array> (n);
            if (CP != nullptr) {
                ComponentPart cp;
                if (cp.parse(K, CP)) {
                    _componentParts->push_back(cp);
                }
            }
        }
    }

    if (ISFI != nullptr) {
        _isScannedForIncludes = ISFI->value();
    }

    if (III != nullptr) {
        _includeInIndex = III->value();
    }

    if (CSIII != nullptr) {
        _canSetIncludeInIndex = CSIII->value();
    }

    if (RHT != nullptr) {
        _requiresHardTabs = RHT->value();
    }

    if (CNC != nullptr) {
        _containsNativeCode = CNC->value();
    }

    if (PDS != nullptr) {
        _plistStructureDefinition = PDS->value();
    }

    if (CCDGI != nullptr) {
        _changesCauseDependencyGraphInvalidation = CCDGI->value();
    }

    if (FABP != nullptr) {
        _fallbackAutoroutingBuildPhase = FABP->value();
    }

    if (CSOC != nullptr) {
        _codeSignOnCopy = CSOC->value();
    }

    if (RHOC != nullptr) {
        _removeHeadersOnCopy = RHOC->value();
    }

    if (VOC != nullptr) {
        _validateOnCopy = VOC->value();
    }

    return true;
}

FileType::ComponentPart::ComponentPart()
{
}

bool FileType::ComponentPart::
parse(std::string const &identifier, plist::Array const *array)
{
    _identifier = identifier;

    auto count = array->count();

    if (count >= 1) {
        if (auto T = array->value <plist::String> (0)) {
            _type = T->value();
        }
    }

    if (count >= 2) {
        if (auto L = array->value <plist::String> (1)) {
            _location = L->value();
        }
    }

    if (count >= 3) {
        if (auto IDs = array->value <plist::Array> (2)) {
            _identifiers = std::vector<std::string>();
            for (size_t m = 0; m < IDs->count(); m++) {
                if (auto ID = IDs->value <plist::String> (m)) {
                    _identifiers->push_back(ID->value());
                }
            }
        } else if (auto R = array->value <plist::String> (2)) {
            _reference = R->value();
        }
    }

    return true;
}

