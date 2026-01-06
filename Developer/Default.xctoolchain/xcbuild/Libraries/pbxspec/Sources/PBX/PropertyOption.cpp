/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxspec/PBX/PropertyOption.h>
#include <pbxsetting/Setting.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Object.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxspec::PBX::PropertyOption;

PropertyOption::
PropertyOption() :
    _displayValues       (nullptr),
    _commandLineArgs     (nullptr),
    _additionalLinkerArgs(nullptr),
    _allowedValues       (nullptr),
    _values              (nullptr)
{
}

PropertyOption::
~PropertyOption()
{
    if (_additionalLinkerArgs != nullptr) {
        _additionalLinkerArgs->release();
    }

    if (_values != nullptr) {
        _values->release();
    }

    if (_displayValues != nullptr) {
        _displayValues->release();
    }

    if (_allowedValues != nullptr) {
        _allowedValues->release();
    }

    if (_commandLineArgs != nullptr) {
        _commandLineArgs->release();
    }
}

ext::optional<pbxsetting::Setting> PropertyOption::
defaultSetting(void) const
{
    if (_defaultValue) {
        return pbxsetting::Setting::Create(_name, *_defaultValue);
    } else {
        return ext::nullopt;
    }
}

bool PropertyOption::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("Property/Option", dict, &seen);

    auto N      = unpack.cast <plist::String> ("Name");
    auto DN     = unpack.cast <plist::String> ("DisplayName");
    auto DVs    = unpack.cast <plist::Object> ("DisplayValues");
    auto T      = unpack.cast <plist::String> ("Type");
    auto UT     = unpack.cast <plist::String> ("UIType");
    auto C      = unpack.cast <plist::String> ("Category");
    auto D      = unpack.cast <plist::String> ("Description");
    auto COND   = unpack.cast <plist::String> ("Condition");
    auto AA     = unpack.cast <plist::String> ("AppearsAfter");
    auto B      = unpack.coerce <plist::Boolean> ("Basic");
    auto CO     = unpack.coerce <plist::Boolean> ("CommonOption");
    auto AEV    = unpack.coerce <plist::Boolean> ("AvoidEmptyValue");
    auto CLC    = unpack.cast <plist::String> ("CommandLineCondition");
    auto CLA    = unpack.cast <plist::Object> ("CommandLineArgs");
    auto CLF    = unpack.cast <plist::String> ("CommandLineFlag");
    auto CLFIF  = unpack.cast <plist::String> ("CommandLineFlagIfFalse");
    auto CLPF   = unpack.cast <plist::String> ("CommandLinePrefixFlag");
    auto DV     = unpack.cast <plist::String> ("DefaultValue");
    auto AV     = unpack.cast <plist::Object> ("AllowedValues");
    auto V      = unpack.cast <plist::Object> ("Values");
    auto FTs    = unpack.cast <plist::Array> ("FileTypes");
    auto CFs    = unpack.cast <plist::Array> ("ConditionFlavors");
    auto SVRs   = unpack.cast <plist::Array> ("SupportedVersionRanges");
    auto IID    = unpack.coerce <plist::Boolean> ("IsInputDependency");
    auto ICI    = unpack.coerce <plist::Boolean> ("IsCommandInput");
    auto ICO    = unpack.coerce <plist::Boolean> ("IsCommandOutput");
    auto AMD    = unpack.coerce <plist::Boolean> ("AvoidMacroDefinition");
    auto FRSPIV = unpack.coerce <plist::Boolean> ("FlattenRecursiveSearchPathsInValue");
    auto SVIEV  = unpack.cast <plist::String> ("SetValueInEnvironmentVariable");
    auto II     = unpack.cast <plist::String> ("InputInclusions");
    auto OD     = unpack.cast <plist::String> ("OutputDependencies");
    auto OASF   = unpack.coerce <plist::Boolean> ("OutputsAreSourceFiles");
    auto ALA    = unpack.cast <plist::Object> ("AdditionalLinkerArgs");
    auto As     = unpack.cast <plist::Array> ("Architectures");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (N != nullptr) {
        _name = N->value();
    } else {
        /* Name is required. */
        return false;
    }

    if (DN != nullptr) {
        _displayName = DN->value();
    }

    if (DVs != nullptr) {
        _displayValues = DVs->copy().release();
    }

    if (T != nullptr) {
        _type = T->value();
    } else {
        /* Type is required. */
        return false;
    }

    if (UT != nullptr) {
        _uiType = UT->value();
    }

    if (C != nullptr) {
        _category = C->value();
    }

    if (D != nullptr) {
        _description = D->value();
    }

    if (COND != nullptr) {
        _condition = COND->value();
    }

    if (AA != nullptr) {
        _appearsAfter = AA->value();
    }

    if (B != nullptr) {
        _basic = B->value();
    }

    if (CO != nullptr) {
        _commonOption = CO->value();
    }

    if (AEV != nullptr) {
        _avoidEmptyValues = AEV->value();
    }

    if (CLC != nullptr) {
        _commandLineCondition = CLC->value();
    }

    if (CLA != nullptr) {
        _commandLineArgs = CLA->copy().release();
    }

    if (CLF != nullptr) {
        _commandLineFlag = pbxsetting::Value::Parse(CLF->value());
    }

    if (CLFIF != nullptr) {
        _commandLineFlagIfFalse = pbxsetting::Value::Parse(CLFIF->value());
    }

    if (CLPF != nullptr) {
        _commandLinePrefixFlag = pbxsetting::Value::Parse(CLPF->value());
    }

    if (DV != nullptr) {
        _defaultValue = pbxsetting::Value::Parse(DV->value());
    }

    if (AV != nullptr) {
        _allowedValues = AV->copy().release();
    }

    if (V != nullptr) {
        _values = V->copy().release();
    }

    if (FTs != nullptr) {
        _fileTypes = std::vector<std::string>();
        for (size_t n = 0; n < FTs->count(); n++) {
            if (auto FT = FTs->value <plist::String> (n)) {
                _fileTypes->push_back(FT->value());
            }
        }
    }

    if (CFs != nullptr) {
        _conditionFlavors = std::vector<std::string>();
        for (size_t n = 0; n < CFs->count(); n++) {
            if (auto CF = CFs->value <plist::String> (n)) {
                _conditionFlavors->push_back(CF->value());
            }
        }
    }

    if (SVRs != nullptr) {
        _supportedVersionRanges = std::vector<std::string>();
        for (size_t n = 0; n < SVRs->count(); n++) {
            if (auto SVR = SVRs->value <plist::String> (n)) {
                _supportedVersionRanges->push_back(SVR->value());
            }
        }
    }

    if (IID != nullptr) {
        _isInputDependency = IID->value();
    }

    if (ICI != nullptr) {
        _isCommandInput = ICI->value();
    }

    if (ICO != nullptr) {
        _isCommandOutput = ICO->value();
    }

    if (AMD != nullptr) {
        _avoidMacroDefinition = AMD->value();
    }

    if (FRSPIV != nullptr) {
        _flattenRecursiveSearchPathsInValue = FRSPIV->value();
    }

    if (SVIEV != nullptr) {
        _setValueInEnvironmentVariable = pbxsetting::Value::Parse(SVIEV->value());
    }

    if (II != nullptr) {
        _inputInclusions = II->value();
    }

    if (OD != nullptr) {
        _outputDependencies = OD->value();
    }

    if (OASF != nullptr) {
        _outputsAreSourceFiles = OASF->value();
    }

    if (ALA != nullptr) {
        _additionalLinkerArgs = ALA->copy().release();
    }

    if (As != nullptr) {
        _architectures = std::vector<std::string>();
        for (size_t n = 0; n < As->count(); n++) {
            if (auto A = As->value <plist::String> (n)) {
                _architectures->push_back(A->value());
            }
        }
    }

    return true;
}

PropertyOption::shared_ptr PropertyOption::
Create(plist::Dictionary const *dict)
{
    auto option = std::shared_ptr<PropertyOption>(new PropertyOption());
    if (!option->parse(dict)) {
        return nullptr;
    }

    return option;
}
