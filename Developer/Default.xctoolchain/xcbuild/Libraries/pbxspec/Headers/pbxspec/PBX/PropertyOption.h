/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxspec_PBX_PropertyOption_h
#define __pbxspec_PBX_PropertyOption_h

#include <pbxsetting/Value.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <ext/optional>

namespace plist { class Object; }
namespace plist { class Dictionary; }
namespace pbxsetting { class Setting; }

namespace pbxspec { namespace PBX {

class BuildSystem;
class Tool;

class PropertyOption {
public:
    typedef std::shared_ptr <PropertyOption> shared_ptr;
    typedef std::vector <shared_ptr> vector;
    typedef std::unordered_map <std::string, size_t> used_map;

    static void Insert(vector *options, used_map *used, shared_ptr const &option)
    {
        auto it = used->find(option->name());
        if (it != used->end()) {
            (*options)[it->second] = option;
        } else {
            used->insert({ option->name(), options->size() });
            options->push_back(option);
        }
    }

protected:
    std::string                              _name;
    ext::optional<std::string>               _displayName;
    plist::Object                           *_displayValues;
    std::string                              _type;
    ext::optional<std::string>               _uiType;
    ext::optional<std::string>               _category;
    ext::optional<std::string>               _description;
    ext::optional<std::string>               _condition;
    ext::optional<std::string>               _appearsAfter;
    ext::optional<std::string>               _inputInclusions;
    ext::optional<std::string>               _outputDependencies;
    ext::optional<bool>                      _basic;
    ext::optional<bool>                      _commonOption;
    ext::optional<bool>                      _avoidEmptyValues;
    ext::optional<std::string>               _commandLineCondition;
    ext::optional<pbxsetting::Value>         _commandLineFlag;
    ext::optional<pbxsetting::Value>         _commandLineFlagIfFalse;
    ext::optional<pbxsetting::Value>         _commandLinePrefixFlag;
    plist::Object                            *_commandLineArgs;
    plist::Object                            *_additionalLinkerArgs;
    ext::optional<pbxsetting::Value>          _defaultValue;
    plist::Object                            *_allowedValues;
    plist::Object                            *_values;
    ext::optional<std::vector<std::string>>  _architectures;
    ext::optional<std::vector<std::string>>  _fileTypes;
    ext::optional<std::vector<std::string>>  _conditionFlavors;
    ext::optional<std::vector<std::string>>  _supportedVersionRanges;
    ext::optional<bool>                      _isInputDependency;
    ext::optional<bool>                      _isCommandInput;
    ext::optional<bool>                      _isCommandOutput;
    ext::optional<bool>                      _outputsAreSourceFiles;
    ext::optional<bool>                      _avoidMacroDefinition;
    ext::optional<bool>                      _flattenRecursiveSearchPathsInValue;
    ext::optional<pbxsetting::Value>         _setValueInEnvironmentVariable;

protected:
    friend class BuildSystem;
    friend class BuildSettings;
    friend class Tool;
    PropertyOption();

public:
    ~PropertyOption();

public:
    inline std::string const &name() const
    { return _name; }
    inline ext::optional<std::string> const &displayName() const
    { return _displayName; }
    inline ext::optional<std::string> const &category() const
    { return _category; }
    inline ext::optional<std::string> const &description() const
    { return _description; }

public:
    inline std::string const &type() const
    { return _type; }
    inline ext::optional<std::string> const &uiType() const
    { return _uiType; }

public:
    inline bool basic() const
    { return _basic.value_or(false); }
    inline ext::optional<bool> basicOptional() const
    { return _basic; }
    inline bool commonOption() const
    { return _commonOption.value_or(false); }
    inline ext::optional<bool> commonOptionOptional() const
    { return _commonOption; }

public:
    inline ext::optional<std::string> const &condition() const
    { return _condition; }
    inline ext::optional<std::string> const &appearsAfter() const
    { return _appearsAfter; }

public:
    inline ext::optional<std::vector<std::string>> const &architectures() const
    { return _architectures; }

public:
    inline bool avoidEmptyValues() const
    { return _avoidEmptyValues.value_or(false); }
    inline ext::optional<bool> avoidEmptyValuesOptional() const
    { return _avoidEmptyValues; }
    inline ext::optional<pbxsetting::Value> const &defaultValue() const
    { return _defaultValue; }
    inline plist::Object const *allowedValues() const
    { return _allowedValues; }
    inline plist::Object const *values() const
    { return _values; }

public:
    inline ext::optional<std::string> const &commandLineCondition() const
    { return _commandLineCondition; }
    inline plist::Object const *commandLineArgs() const
    { return _commandLineArgs; }
    inline ext::optional<pbxsetting::Value> const &commandLineFlag() const
    { return _commandLineFlag; }
    inline ext::optional<pbxsetting::Value> const &commandLineFlagIfFalse() const
    { return _commandLineFlagIfFalse; }
    inline ext::optional<pbxsetting::Value> const &commandLinePrefixFlag() const
    { return _commandLinePrefixFlag; }

public:
    inline plist::Object const *additionalLinkerArgs() const
    { return _additionalLinkerArgs; }

public:
    inline bool isCommandInput() const
    { return _isCommandInput.value_or(false); }
    inline ext::optional<bool> isCommandInputOptional() const
    { return _isCommandInput; }
    inline bool isCommandOutput() const
    { return _isCommandOutput.value_or(false); }
    inline ext::optional<bool> isCommandOutputOptional() const
    { return _isCommandOutput; }

public:
    inline ext::optional<std::string> const &inputInclusions() const
    { return _inputInclusions; }
    inline ext::optional<std::string> const &outputDependencies() const
    { return _outputDependencies; }

public:
    inline bool outputsAreSourceFiles() const
    { return _outputsAreSourceFiles.value_or(false); }
    inline ext::optional<bool> outputsAreSourceFilesOptional() const
    { return _outputsAreSourceFiles; }

public:
    inline ext::optional<std::vector<std::string>> const &fileTypes() const
    { return _fileTypes; }

public:
    inline ext::optional<std::vector<std::string>> const &conditionFlavors() const
    { return _conditionFlavors; }

public:
    inline ext::optional<std::vector<std::string>> const &supportedVersionRanges() const
    { return _supportedVersionRanges; }

public:
    inline bool isInputDependency() const
    { return _isInputDependency.value_or(false); }
    inline ext::optional<bool> isInputDependencyOptional() const
    { return _isInputDependency; }

public:
    inline bool avoidMacroDefinition() const
    { return _avoidMacroDefinition.value_or(false); }
    inline ext::optional<bool> avoidMacroDefinitionOptional() const
    { return _avoidMacroDefinition; }

public:
    inline bool flattenRecursiveSearchPathsInValue() const
    { return _flattenRecursiveSearchPathsInValue.value_or(false); }
    inline ext::optional<bool> flattenRecursiveSearchPathsInValueOptional() const
    { return _flattenRecursiveSearchPathsInValue; }

public:
    inline ext::optional<pbxsetting::Value> const &setValueInEnvironmentVariable() const
    { return _setValueInEnvironmentVariable; }

public:
    ext::optional<pbxsetting::Setting> defaultSetting(void) const;

protected:
    bool parse(plist::Dictionary const *dict);

public:
    static PropertyOption::shared_ptr Create(plist::Dictionary const *dict);
};

} }

#endif  // !__pbxspec_PBX_PropertyOption_h
