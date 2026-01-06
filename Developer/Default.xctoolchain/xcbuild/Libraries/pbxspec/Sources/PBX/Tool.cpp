/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxspec/PBX/Tool.h>
#include <pbxspec/Context.h>
#include <pbxspec/Inherit.h>
#include <pbxsetting/Type.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/Integer.h>
#include <plist/Object.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxspec::PBX::Tool;

Tool::
Tool() :
    Specification       (),
    _commandOutputParser(nullptr)
{
}

Tool::~Tool()
{
    if (_commandOutputParser != nullptr) {
        _commandOutputParser->release();
    }
}

pbxsetting::Level Tool::
defaultSettings(void) const
{
    std::vector<pbxsetting::Setting> settings;
    if (_options) {
        for (PBX::PropertyOption::shared_ptr const &option : *_options) {
            if (ext::optional<pbxsetting::Setting> setting = option->defaultSetting()) {
                settings.push_back(*setting);
            }
        }
    }
    return pbxsetting::Level(settings);
}

Tool::shared_ptr Tool::
Parse(Context *context, plist::Dictionary const *dict)
{
    if (!ParseType(context, dict, Type())) {
        return nullptr;
    }

    Tool::shared_ptr result;
    result.reset(new Tool());

    std::unordered_set<std::string> seen;
    if (!result->parse(context, dict, &seen, true))
        return nullptr;

    return result;
}

bool Tool::
parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Specification::parse(context, dict, seen, false))
        return false;

    auto unpack = plist::Keys::Unpack("Tool", dict, seen);

    auto EP     = unpack.cast <plist::String> ("ExecPath");
    auto ED     = unpack.cast <plist::String> ("ExecDescription");
    auto EDPC   = unpack.cast <plist::String> ("ExecDescriptionForPrecompile");
    auto EDC    = unpack.cast <plist::String> ("ExecDescriptionForCompile");
    auto EDCB   = unpack.cast <plist::String> ("ExecDescriptionForCreateBitcode");
    auto PD     = unpack.cast <plist::String> ("ProgressDescription");
    auto PDPC   = unpack.cast <plist::String> ("ProgressDescriptionForPrecompile");
    auto PDC    = unpack.cast <plist::String> ("ProgressDescriptionForCompile");
    auto PDCB   = unpack.cast <plist::String> ("ProgressDescriptionForCreateBitcode");
    auto CL     = unpack.cast <plist::String> ("CommandLine");
    auto CIC    = unpack.cast <plist::String> ("CommandInvocationClass");
    auto CI     = unpack.cast <plist::String> ("CommandIdentifier");
    auto RN     = unpack.cast <plist::Object> ("RuleName");
    auto RF     = unpack.cast <plist::String> ("RuleFormat");
    auto AIF    = unpack.cast <plist::String> ("AdditionalInputFiles");
    auto BJRN   = unpack.cast <plist::String> ("BuiltinJambaseRuleName");
    auto FTs    = unpack.cast <plist::Array> ("FileTypes");
    auto IFTs   = unpack.cast <plist::Array> ("InputFileTypes");
    auto ITs    = unpack.cast <plist::Array> ("InputTypes");
    auto Os     = unpack.cast <plist::Array> ("Outputs");
    auto OP     = unpack.cast <plist::String> ("OutputPath");
    auto As     = unpack.cast <plist::Object> ("Architectures");
    auto EVs    = unpack.cast <plist::Dictionary> ("EnvironmentVariables");
    auto SECs   = unpack.cast <plist::Array> ("SuccessExitCodes");
    auto COP    = unpack.cast <plist::Object> ("CommandOutputParser");
    auto IA     = unpack.coerce <plist::Boolean> ("IsAbstract");
    auto IAN    = unpack.coerce <plist::Boolean> ("IsArchitectureNeutral");
    auto CAID   = unpack.coerce <plist::Boolean> ("CaresAboutInclusionDependencies");
    auto SBR    = unpack.coerce <plist::Boolean> ("SynthesizeBuildRule");
    auto SROE   = unpack.coerce <plist::Boolean> ("ShouldRerunOnError");
    auto DSID   = unpack.coerce <plist::Boolean> ("DeeplyStatInputDirectories");
    auto IUTI   = unpack.coerce <plist::Boolean> ("IsUnsafeToInterrupt");
    auto ML     = unpack.coerce <plist::Integer> ("MessageLimit");
    auto OPs    = unpack.cast <plist::Array> ("Options");
    auto DPs    = unpack.cast <plist::Array> ("DeletedProperties");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (EP != nullptr) {
        _execPath = pbxsetting::Value::Parse(EP->value());
    }

    if (ED != nullptr) {
        _execDescription = pbxsetting::Value::Parse(ED->value());
    }

    if (EDPC != nullptr) {
        _execDescriptionForPrecompile = pbxsetting::Value::Parse(EDPC->value());
    }

    if (EDC != nullptr) {
        _execDescriptionForCompile = pbxsetting::Value::Parse(EDC->value());
    }

    if (EDCB != nullptr) {
        _execDescriptionForCreateBitcode = pbxsetting::Value::Parse(EDCB->value());
    }

    if (PD != nullptr) {
        _progressDescription = pbxsetting::Value::Parse(PD->value());
    }

    if (PDPC != nullptr) {
        _progressDescriptionForPrecompile = pbxsetting::Value::Parse(PDPC->value());
    }

    if (PDC != nullptr) {
        _progressDescriptionForCompile = pbxsetting::Value::Parse(PDC->value());
    }

    if (PDCB != nullptr) {
        _progressDescriptionForCreateBitcode = pbxsetting::Value::Parse(PDCB->value());
    }

    if (CL != nullptr) {
        _commandLine = pbxsetting::Value::Parse(CL->value());
    }

    if (CIC != nullptr) {
        _commandInvocationClass = CIC->value();
    }

    if (CI != nullptr) {
        _commandIdentifier = pbxsetting::Value::Parse(CI->value());
    }

    if (auto RNS = plist::CastTo<plist::String>(RN)) {
        _ruleName = pbxsetting::Value::Parse(RNS->value());
    } else if (auto RNA = plist::CastTo<plist::Array>(RN)) {
        std::string value;

        for (size_t n = 0; n < RNA->count(); n++) {
            if (n != 0) {
                value += " ";
            }

            if (auto entry = RNA->value<plist::String>(n)) {
                value += entry->value();
            }
        }

        _ruleName = pbxsetting::Value::Parse(value);
    }

    if (RF != nullptr) {
        _ruleFormat = pbxsetting::Value::Parse(RF->value());
    }

    if (AIF != nullptr) {
        _additionalInputFiles = pbxsetting::Value::Parse(AIF->value());
    }

    if (BJRN != nullptr) {
        _builtinJambaseRuleName = BJRN->value();
    }

    if (FTs != nullptr) {
        _fileTypes = std::vector<std::string>();
        for (size_t n = 0; n < FTs->count(); n++) {
            if (auto FT = FTs->value <plist::String> (n)) {
                _fileTypes->push_back(FT->value());
            }
        }
    }

    if (IFTs != nullptr) {
        _inputFileTypes = std::vector<std::string>();
        for (size_t n = 0; n < IFTs->count(); n++) {
            if (auto IFT = IFTs->value <plist::String> (n)) {
                _inputFileTypes->push_back(IFT->value());
            }
        }
    }

    if (ITs != nullptr) {
        _inputTypes = std::vector<std::string>();
        for (size_t n = 0; n < ITs->count(); n++) {
            if (auto IT = ITs->value <plist::String> (n)) {
                _inputTypes->push_back(IT->value());
            }
        }
    }

    if (auto AS = plist::CastTo<plist::Array>(As)) {
        _architectures = std::vector<std::string>();
        for (size_t n = 0; n < AS->count(); n++) {
            if (auto A = AS->value <plist::String> (n)) {
                _architectures->push_back(A->value());
            }
        }
    } else if (auto AS = plist::CastTo<plist::String>(As)) {
        _architectures = pbxsetting::Type::ParseList(AS->value());
    }

    if (Os != nullptr) {
        _outputs = std::vector<pbxsetting::Value>();
        for (size_t n = 0; n < Os->count(); n++) {
            if (auto O = Os->value <plist::String> (n)) {
                _outputs->push_back(pbxsetting::Value::Parse(O->value()));
            }
        }
    }

    if (OP != nullptr) {
        _outputPath = pbxsetting::Value::Parse(OP->value());
    }

    if (EVs != nullptr) {
        _environmentVariables = std::unordered_map<std::string, pbxsetting::Value>();
        for (size_t n = 0; n < EVs->count(); n++) {
            auto EVk = EVs->key(n);
            if (auto EVv = EVs->value <plist::String> (EVk)) {
                _environmentVariables->insert({ EVk, pbxsetting::Value::Parse(EVv->value()) });
            }
        }
    }

    if (SECs != nullptr) {
        _successExitCodes = std::vector<int>();
        for (size_t n = 0; n < SECs->count(); n++) {
            if (auto SEC = SECs->value <plist::String> (n)) {
                _successExitCodes->push_back(pbxsetting::Type::ParseInteger(SEC->value()));
            }
        }
    }

    if (COP != nullptr) {
        if (_commandOutputParser != nullptr) {
            _commandOutputParser->release();
        }

        _commandOutputParser = COP->copy().release();
    }

    if (IA != nullptr) {
        _isAbstract = IA->value();
    }

    if (IAN != nullptr) {
        _isArchitectureNeutral = IAN->value();
    }

    if (CAID != nullptr) {
        _caresAboutInclusionDependencies = CAID->value();
    }

    if (SBR != nullptr) {
        _synthesizeBuildRule = SBR->value();
    }

    if (SROE != nullptr) {
        _shouldRerunOnError = SROE->value();
    }

    if (DSID != nullptr) {
        _deeplyStatInputDirectories = DSID->value();
    }

    if (IUTI != nullptr) {
        _isUnsafeToInterrupt = IUTI->value();
    }

    if (ML != nullptr) {
        _messageLimit = ML->value();
    }

    if (OPs != nullptr) {
        _options = PropertyOption::vector();
        for (size_t n = 0; n < OPs->count(); n++) {
            if (auto OP = OPs->value <plist::Dictionary> (n)) {
                PropertyOption::shared_ptr option;
                option.reset(new PropertyOption);
                if (option->parse(OP)) {
                    PropertyOption::Insert(&*_options, &_optionsUsed, option);
                }
            }
        }
    }

    if (DPs != nullptr) {
        _deletedProperties = std::unordered_set<std::string>();
        for (size_t n = 0; n < DPs->count(); n++) {
            if (auto DP = DPs->value <plist::String> (n)) {
                _deletedProperties->insert(DP->value());
            }
        }
    }

    return true;
}

bool Tool::
inherit(Specification::shared_ptr const &base)
{
    if (base->type() != Tool::Type())
        return false;

    return inherit(std::static_pointer_cast<Tool>(base));
}

bool Tool::
inherit(Tool::shared_ptr const &b)
{
    if (!Specification::inherit(b))
        return false;

    auto base = this->base();

    _execPath                            = Inherit::Override(_execPath, base->_execPath);
    _execDescription                     = Inherit::Override(_execDescription, base->_execDescription);
    _execDescriptionForPrecompile        = Inherit::Override(_execDescriptionForPrecompile, base->_execDescriptionForPrecompile);
    _execDescriptionForCompile           = Inherit::Override(_execDescriptionForCompile, base->_execDescriptionForCompile);
    _execDescriptionForCreateBitcode     = Inherit::Override(_execDescriptionForCreateBitcode, base->_execDescriptionForCreateBitcode);
    _progressDescription                 = Inherit::Override(_progressDescription, base->_progressDescription);
    _progressDescriptionForPrecompile    = Inherit::Override(_progressDescriptionForPrecompile, base->_progressDescriptionForPrecompile);
    _progressDescriptionForCompile       = Inherit::Override(_progressDescriptionForCompile, base->_progressDescriptionForCompile);
    _progressDescriptionForCreateBitcode = Inherit::Override(_progressDescriptionForCreateBitcode, base->_progressDescriptionForCreateBitcode);
    _commandLine                         = Inherit::Override(_commandLine, base->_commandLine);
    _commandInvocationClass              = Inherit::Override(_commandInvocationClass, base->_commandInvocationClass);
    _commandIdentifier                   = Inherit::Override(_commandIdentifier, base->_commandIdentifier);
    _ruleName                            = Inherit::Override(_ruleName, base->_ruleName);
    _ruleFormat                          = Inherit::Override(_ruleFormat, base->_ruleFormat);
    _additionalInputFiles                = Inherit::Override(_additionalInputFiles, base->_additionalInputFiles);
    _builtinJambaseRuleName              = Inherit::Override(_builtinJambaseRuleName, base->_builtinJambaseRuleName);
    _fileTypes                           = Inherit::Combine(_fileTypes, base->_fileTypes);
    _inputFileTypes                      = Inherit::Combine(_inputFileTypes, base->_inputFileTypes);
    _inputTypes                          = Inherit::Combine(_inputTypes, base->_inputTypes);
    _architectures                       = Inherit::Combine(_architectures, base->_architectures);
    _outputs                             = Inherit::Combine(_outputs, base->_outputs);
    _outputPath                          = Inherit::Override(_outputPath, base->_outputPath);
    _deletedProperties                   = Inherit::Combine(_deletedProperties, base->_deletedProperties);
    _environmentVariables                = Inherit::Combine(_environmentVariables, base->_environmentVariables);
    _successExitCodes                    = Inherit::Combine(_successExitCodes, base->_successExitCodes);
    _commandOutputParser                 = _commandOutputParser ? _commandOutputParser : (base->_commandOutputParser ? base->_commandOutputParser->copy().release() : nullptr);
    _isAbstract                          = Inherit::Override(_isAbstract, base->_isAbstract);
    _isArchitectureNeutral               = Inherit::Override(_isArchitectureNeutral, base->_isArchitectureNeutral);
    _caresAboutInclusionDependencies     = Inherit::Override(_caresAboutInclusionDependencies, base->_caresAboutInclusionDependencies);
    _synthesizeBuildRule                 = Inherit::Override(_synthesizeBuildRule, base->_synthesizeBuildRule);
    _shouldRerunOnError                  = Inherit::Override(_shouldRerunOnError, base->_shouldRerunOnError);
    _deeplyStatInputDirectories          = Inherit::Override(_deeplyStatInputDirectories, base->_deeplyStatInputDirectories);
    _isUnsafeToInterrupt                 = Inherit::Override(_isUnsafeToInterrupt, base->_isUnsafeToInterrupt);
    _messageLimit                        = Inherit::Override(_messageLimit, base->_messageLimit);
    _options                             = Inherit::Combine(_options, base->_options, &_optionsUsed, &base->_optionsUsed);

    return true;
}
