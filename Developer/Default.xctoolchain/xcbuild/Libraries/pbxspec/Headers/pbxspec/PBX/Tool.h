/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxspec_PBX_Tool_h
#define __pbxspec_PBX_Tool_h

#include <pbxspec/PBX/Specification.h>
#include <pbxspec/PBX/PropertyOption.h>
#include <pbxsetting/Value.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <ext/optional>

namespace pbxsetting { class Level; }

namespace pbxspec { namespace PBX {

class Tool : public Specification {
public:
    typedef std::shared_ptr <Tool> shared_ptr;
    typedef std::vector <shared_ptr> vector;

protected:
    ext::optional<pbxsetting::Value>               _execPath;
    ext::optional<pbxsetting::Value>               _execDescription;
    ext::optional<pbxsetting::Value>               _execDescriptionForPrecompile;
    ext::optional<pbxsetting::Value>               _execDescriptionForCompile;
    ext::optional<pbxsetting::Value>               _execDescriptionForCreateBitcode;
    ext::optional<pbxsetting::Value>               _progressDescription;
    ext::optional<pbxsetting::Value>               _progressDescriptionForPrecompile;
    ext::optional<pbxsetting::Value>               _progressDescriptionForCompile;
    ext::optional<pbxsetting::Value>               _progressDescriptionForCreateBitcode;
    ext::optional<pbxsetting::Value>               _commandLine;
    ext::optional<std::string>                     _commandInvocationClass;
    ext::optional<pbxsetting::Value>               _commandIdentifier;
    ext::optional<pbxsetting::Value>               _ruleName;
    ext::optional<pbxsetting::Value>               _ruleFormat;
    ext::optional<pbxsetting::Value>               _additionalInputFiles;
    ext::optional<std::string>                     _builtinJambaseRuleName;
    ext::optional<std::vector<std::string>>        _fileTypes;
    ext::optional<std::vector<std::string>>        _inputFileTypes;
    ext::optional<std::vector<std::string>>        _inputTypes;
    ext::optional<std::vector<std::string>>        _architectures;
    ext::optional<std::vector<pbxsetting::Value>>  _outputs;
    ext::optional<pbxsetting::Value>               _outputPath;
    ext::optional<std::unordered_set<std::string>> _deletedProperties;
    ext::optional<std::unordered_map<std::string, pbxsetting::Value>> _environmentVariables;
    ext::optional<std::vector<int>>                _successExitCodes;
    plist::Object                                 *_commandOutputParser;
    ext::optional<bool>                            _isAbstract;
    ext::optional<bool>                            _isArchitectureNeutral;
    ext::optional<bool>                            _caresAboutInclusionDependencies;
    ext::optional<bool>                            _synthesizeBuildRule;
    ext::optional<bool>                            _shouldRerunOnError;
    ext::optional<bool>                            _deeplyStatInputDirectories;
    ext::optional<bool>                            _isUnsafeToInterrupt;
    ext::optional<int>                             _messageLimit;
    ext::optional<PropertyOption::vector>          _options;
    PropertyOption::used_map                       _optionsUsed;

protected:
    Tool();

public:
    virtual ~Tool();

public:
    inline SpecificationType type() const override
    { return Tool::Type(); }

public:
    inline Tool::shared_ptr base() const
    { return std::static_pointer_cast<Tool>(Specification::base()); }

public:
    inline ext::optional<pbxsetting::Value> const &execPath() const
    { return _execPath; }

public:
    inline ext::optional<pbxsetting::Value> const &execDescription() const
    { return _execDescription; }
    inline ext::optional<pbxsetting::Value> const &execDescriptionForPrecompile() const
    { return _execDescriptionForPrecompile; }
    inline ext::optional<pbxsetting::Value> const &execDescriptionForCompile() const
    { return _execDescriptionForCompile; }
    inline ext::optional<pbxsetting::Value> const &execDescriptionForCreateBitcode() const
    { return _execDescriptionForCreateBitcode; }

public:
    inline ext::optional<pbxsetting::Value> const &progressDescription() const
    { return _progressDescription; }
    inline ext::optional<pbxsetting::Value> const &progressDescriptionForPrecompile() const
    { return _progressDescriptionForPrecompile; }
    inline ext::optional<pbxsetting::Value> const &progressDescriptionForCompile() const
    { return _progressDescriptionForCompile; }
    inline ext::optional<pbxsetting::Value> const &progressDescriptionForCreateBitcode() const
    { return _progressDescriptionForCreateBitcode; }

public:
    inline ext::optional<pbxsetting::Value> const &commandLine() const
    { return _commandLine; }

public:
    inline ext::optional<std::string> const &commandInvocationClass() const
    { return _commandInvocationClass; }
    inline ext::optional<pbxsetting::Value> const &commandIdentifier() const
    { return _commandIdentifier; }

public:
    inline ext::optional<pbxsetting::Value> const &ruleName() const
    { return _ruleName; }
    inline ext::optional<pbxsetting::Value> const &ruleFormat() const
    { return _ruleFormat; }
    inline ext::optional<std::string> const &builtinJambaseRuleName() const
    { return _builtinJambaseRuleName; }

public:
    inline ext::optional<pbxsetting::Value> const &additionalInputFiles() const
    { return _additionalInputFiles; }

public:
    inline ext::optional<std::vector<std::string>> const &fileTypes() const
    { return _fileTypes; }
    inline ext::optional<std::vector<std::string>> const &inputFileTypes() const
    { return _inputFileTypes; }
    inline ext::optional<std::vector<std::string>> const &inputTypes() const
    { return _inputTypes; }

public:
    inline ext::optional<std::vector<std::string>> const &architectures() const
    { return _architectures; }

public:
    inline ext::optional<std::vector<pbxsetting::Value>> const &outputs() const
    { return _outputs; }
    inline ext::optional<pbxsetting::Value> const &outputPath() const
    { return _outputPath; }

public:
    inline ext::optional<std::unordered_map<std::string, pbxsetting::Value>> const &environmentVariables() const
    { return _environmentVariables; }

public:
    inline ext::optional<std::vector<int>> const &successExitCodes() const
    { return _successExitCodes; }

public:
    inline plist::Object const *commandOutputParser() const
    { return _commandOutputParser; }

public:
    inline bool isAbstract() const
    { return _isAbstract.value_or(false); }
    inline ext::optional<bool> isAbstractOptional() const
    { return _isAbstract; }

public:
    inline bool isArchitectureNeutral() const
    { return _isArchitectureNeutral.value_or(false); }
    inline ext::optional<bool> isArchitectureNeutralOptional() const
    { return _isArchitectureNeutral; }

public:
    inline bool caresAboutInclusionDependencies() const
    { return _caresAboutInclusionDependencies.value_or(false); }
    inline ext::optional<bool> caresAboutInclusionDependenciesOptional() const
    { return _caresAboutInclusionDependencies; }

public:
    inline bool synthesizeBuildRule() const
    { return _synthesizeBuildRule.value_or(false); }
    inline ext::optional<bool> synthesizeBuildRuleOptional() const
    { return _synthesizeBuildRule; }

public:
    inline bool shouldRerunOnError() const
    { return _shouldRerunOnError.value_or(false); }
    inline ext::optional<bool> shouldRerunOnErrorOptional() const
    { return _shouldRerunOnError; }

public:
    inline bool deeplyStatInputDirectories() const
    { return _deeplyStatInputDirectories.value_or(false); }
    inline ext::optional<bool> deeplyStatInputDirectoriesOptional() const
    { return _deeplyStatInputDirectories; }

public:
    inline bool isUnsafeToInterrupt() const
    { return _isUnsafeToInterrupt.value_or(false); }
    inline ext::optional<bool> isUnsafeToInterruptOptional() const
    { return _isUnsafeToInterrupt; }

public:
    inline ext::optional<int> messageLimit() const
    { return _messageLimit; }

public:
    inline ext::optional<PropertyOption::vector> const &options() const
    { return _options; }

public:
    inline ext::optional<std::unordered_set<std::string>> const &deletedProperties() const
    { return _deletedProperties; }

public:
    pbxsetting::Level defaultSettings(void) const;

protected:
    friend class Specification;
    bool parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

protected:
    bool inherit(Specification::shared_ptr const &base) override;
    virtual bool inherit(Tool::shared_ptr const &base);

protected:
    static Tool::shared_ptr Parse(Context *context, plist::Dictionary const *dict);

public:
    static inline SpecificationType Type()
    { return SpecificationType::Tool; }
};

} }

#endif  // !__pbxspec_PBX_Tool_h
