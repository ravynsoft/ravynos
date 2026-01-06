/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <pbxbuild/Tool/OptionsResult.h>
#include <pbxsetting/Environment.h>
#include <pbxsetting/Level.h>
#include <pbxsetting/Setting.h>
#include <plist/Dictionary.h>
#include <plist/Format/ASCII.h>
#include <plist/Format/Encoding.h>

namespace Tool = pbxbuild::Tool;

/*
 * Create an option from an ASCII string representing it.
 */
static pbxspec::PBX::PropertyOption::shared_ptr
Option(std::string const &ascii)
{
    auto contents = std::vector<uint8_t>(ascii.begin(), ascii.end());

    auto format = plist::Format::ASCII::Create(false, plist::Format::Encoding::UTF8);
    auto deserialize = plist::Format::ASCII::Deserialize(contents, format);
    if (deserialize.first == nullptr) {
        return nullptr;
    }

    if (auto dict = plist::CastTo<plist::Dictionary>(deserialize.first.get())) {
        return pbxspec::PBX::PropertyOption::Create(dict);
    } else {
        return nullptr;
    }
}

/*
 * Allows defining an option inline without dealing with escaping
 * embedded quotes or having to quote each line in the string.
 */
#define OPTION(...) Option(#__VA_ARGS__)

/*
 * Helper to create an environment with settings defined inline.
 */
static pbxsetting::Environment
Environment(std::vector<pbxsetting::Setting> const &settings)
{
    pbxsetting::Environment environment;
    environment.insertBack(pbxsetting::Level(settings), false);
    return environment;
}

/* Default working directory. */
static std::string const WorkingDirectory = "";

/* Default file type. */
static pbxspec::PBX::FileType::shared_ptr const FileType = nullptr;



/*
 * Test basic command line flags for boolean types.
 */
TEST(OptionsResult, Boolean)
{
    std::vector<pbxspec::PBX::PropertyOption::shared_ptr> options = {
        OPTION({
            Name = BOOL_TRUE;
            Type = Boolean;
            CommandLineFlag = "true";
        }),
        OPTION({
            Name = BOOL_FALSE;
            Type = Boolean;
            CommandLineFlag = "false";
        }),
        OPTION({
            Name = BOOL_LOWER;
            Type = bool;
            CommandLineFlag = "lower";
        }),
        OPTION({
            Name = BOOL_EXPAND;
            Type = Boolean;
            CommandLineFlag = "expand-$(FLAG)";
        }),
        OPTION({
            Name = BOOL_EMPTY;
            Type = Boolean;
            CommandLineFlag = "empty";
        }),
    };

    auto environment = Environment({
        pbxsetting::Setting::Create("BOOL_TRUE", "YES"),
        pbxsetting::Setting::Create("BOOL_FALSE", "NO"),
        pbxsetting::Setting::Create("BOOL_LOWER", "YES"),
        pbxsetting::Setting::Create("BOOL_EXPAND", "YES"),
        pbxsetting::Setting::Create("BOOL_EMPTY", ""),

        pbxsetting::Setting::Create("FLAG", "flag"),
    });

    auto result = Tool::OptionsResult::Create(environment, WorkingDirectory, options, FileType);
    EXPECT_EQ(result.arguments(), std::vector<std::string>({
        "true",
        "lower",
        "expand-flag",
    }));
}

/*
 * Test basic command line flags for string types.
 */
TEST(OptionsResult, String)
{
    std::vector<pbxspec::PBX::PropertyOption::shared_ptr> options = {
        OPTION({
            Name = STRING_NORMAL;
            Type = String;
            CommandLineFlag = "--string-normal";
        }),
        OPTION({
            Name = STRING_LOWER;
            Type = string;
            CommandLineFlag = "--string-lower";
        }),
        OPTION({
            Name = ENUM_NORMAL;
            Type = Enumeration;
            CommandLineFlag = "--enum-normal";
        }),
        OPTION({
            Name = ENUM_LOWER;
            Type = enum;
            CommandLineFlag = "--enum-lower";
        }),
        OPTION({
            Name = PATH_NORMAL;
            Type = Path;
            CommandLineFlag = "--path-normal";
        }),
        OPTION({
            Name = PATH_LOWER;
            Type = path;
            CommandLineFlag = "--path-lower";
        }),
        OPTION({
            Name = STRING_EXPAND;
            Type = String;
            CommandLineFlag = "--$(FLAG)";
        }),
        OPTION({
            Name = STRING_EMPTY;
            Type = String;
            CommandLineFlag = "--empty";
        }),
    };

    auto environment = Environment({
        pbxsetting::Setting::Create("STRING_NORMAL", "string-normal"),
        pbxsetting::Setting::Create("STRING_LOWER", "string-lower"),
        pbxsetting::Setting::Create("ENUM_NORMAL", "enum-normal"),
        pbxsetting::Setting::Create("ENUM_LOWER", "enum-lower"),
        pbxsetting::Setting::Create("PATH_NORMAL", "path-normal"),
        pbxsetting::Setting::Create("PATH_LOWER", "path-lower"),
        pbxsetting::Setting::Create("STRING_EXPAND", "expand"),
        pbxsetting::Setting::Create("STRING_EMPTY", ""),

        pbxsetting::Setting::Create("FLAG", "flag"),
    });

    auto result = Tool::OptionsResult::Create(environment, WorkingDirectory, options, FileType);
    EXPECT_EQ(result.arguments(), std::vector<std::string>({
        "--string-normal", "string-normal",
        "--string-lower", "string-lower",
        "--enum-normal", "enum-normal",
        "--enum-lower", "enum-lower",
        "--path-normal", "path-normal",
        "--path-lower", "path-lower",
        "--flag", "expand",
    }));
}

/*
 * Test basic command line flags for string list types.
 */
TEST(OptionsResult, StringList)
{
    std::vector<pbxspec::PBX::PropertyOption::shared_ptr> options = {
        OPTION({
            Name = STRINGLIST_NORMAL;
            Type = StringList;
            CommandLineFlag = "--stringlist-normal";
        }),
        OPTION({
            Name = STRINGLIST_LOWER;
            Type = stringlist;
            CommandLineFlag = "--stringlist-lower";
        }),
        OPTION({
            Name = PATHLIST_NORMAL;
            Type = PathList;
            CommandLineFlag = "--pathlist-normal";
        }),
        OPTION({
            Name = PATHLIST_LOWER;
            Type = pathlist;
            CommandLineFlag = "--pathlist-lower";
        }),
        OPTION({
            Name = STRINGLIST_EMPTY;
            Type = StringList;
            CommandLineFlag = "--empty";
        }),
    };

    auto environment = Environment({
        pbxsetting::Setting::Create("STRINGLIST_NORMAL", "stringlist-normal1 stringlist-normal2"),
        pbxsetting::Setting::Create("STRINGLIST_LOWER", "stringlist-lower1 stringlist-lower2"),
        pbxsetting::Setting::Create("PATHLIST_NORMAL", "pathlist-normal1 pathlist-normal2"),
        pbxsetting::Setting::Create("PATHLIST_LOWER", "pathlist-lower1 pathlist-lower2"),
        pbxsetting::Setting::Create("STRINGLIST_EMPTY", ""),
    });

    auto result = Tool::OptionsResult::Create(environment, WorkingDirectory, options, FileType);
    EXPECT_EQ(result.arguments(), std::vector<std::string>({
        "--stringlist-normal", "stringlist-normal1", "--stringlist-normal", "stringlist-normal2",
        "--stringlist-lower", "stringlist-lower1", "--stringlist-lower", "stringlist-lower2",
        "--pathlist-normal", "pathlist-normal1", "--pathlist-normal", "pathlist-normal2",
        "--pathlist-lower", "pathlist-lower1", "--pathlist-lower", "pathlist-lower2",
    }));
}

/*
 * Test the `CommandLineFlagIfFalse` option for booleans.
 */
TEST(OptionsResult, FlagIfFalse)
{
    std::vector<pbxspec::PBX::PropertyOption::shared_ptr> options = {
        OPTION({
            Name = IFFALSE_TRUE;
            Type = Boolean;
            CommandLineFlagIfFalse = "true";
        }),
        OPTION({
            Name = IFFALSE_FALSE;
            Type = Boolean;
            CommandLineFlagIfFalse = "false";
        }),
        OPTION({
            Name = IFFALSE_STRING_VALUE;
            Type = String;
            CommandLineFlagIfFalse = "value-string";
        }),
        OPTION({
            Name = IFFALSE_EXPAND;
            Type = Boolean;
            CommandLineFlagIfFalse = "expand-$(FLAG)";
        }),
        OPTION({
            Name = IFFALSE_STRING_EMPTY;
            Type = String;
            CommandLineFlagIfFalse = "empty-string";
        }),
    };

    auto environment = Environment({
        pbxsetting::Setting::Create("IFFALSE_TRUE", "YES"),
        pbxsetting::Setting::Create("IFFALSE_FALSE", "NO"),
        pbxsetting::Setting::Create("IFFALSE_STRING_VALUE", "string"),
        pbxsetting::Setting::Create("IFFALSE_STRING_EMPTY", ""),
        pbxsetting::Setting::Create("IFFALSE_EXPAND", "NO"),
        pbxsetting::Setting::Create("FLAG", "flag"),
    });

    auto result = Tool::OptionsResult::Create(environment, WorkingDirectory, options, FileType);
    EXPECT_EQ(result.arguments(), std::vector<std::string>({
        "false",
        "expand-flag",
        // TODO(grp): Should this include IFFALSE_STRING_EMPTY?
    }));
}

/*
 * Test the `CommandLineFlagPrefix` option for all option types.
 */
TEST(OptionsResult, PrefixFlag)
{
    std::vector<pbxspec::PBX::PropertyOption::shared_ptr> options = {
        OPTION({
            Name = STRING_PREFIX;
            Type = String;
            CommandLinePrefixFlag = "--prefix=";
        }),
        OPTION({
            Name = STRING_PREFIX_EMPTY;
            Type = String;
            CommandLinePrefixFlag = "--empty=";
        }),
        OPTION({
            Name = STRING_EMPTY_PREFIX;
            Type = String;
            CommandLinePrefixFlag = "";
        }),
        OPTION({
            Name = STRINGLIST_PREFIX;
            Type = StringList;
            CommandLinePrefixFlag = "--list=";
        }),
        OPTION({
            Name = STRING_PREFIX_EXPAND;
            Type = String;
            CommandLinePrefixFlag = "--$(FLAG)=";
        }),
        OPTION({
            Name = BOOL_PREFIX;
            Type = Boolean;
            CommandLinePrefixFlag = "--boolean=";
        }),
    };

    auto environment = Environment({
        pbxsetting::Setting::Create("STRING_PREFIX", "prefix"),
        pbxsetting::Setting::Create("STRING_PREFIX_EMPTY", ""),
        pbxsetting::Setting::Create("STRING_EMPTY_PREFIX", "empty-prefix"),
        pbxsetting::Setting::Create("STRINGLIST_PREFIX", "one two"),
        pbxsetting::Setting::Create("STRING_PREFIX_EXPAND", "expand"),
        pbxsetting::Setting::Create("BOOL_PREFIX", "YES"),

        pbxsetting::Setting::Create("FLAG", "flag"),
    });

    auto result = Tool::OptionsResult::Create(environment, WorkingDirectory, options, FileType);
    EXPECT_EQ(result.arguments(), std::vector<std::string>({
        "--prefix=prefix",
        "empty-prefix",
        "--list=one", "--list=two",
        "--flag=expand",
        "--boolean=YES", // TODO(grp): Is this correct?
    }));
}

/*
 * Test the `CommandLineArgs` option array for all option types.
 */
TEST(OptionsResult, CommandLineArgsArray)
{
    std::vector<pbxspec::PBX::PropertyOption::shared_ptr> options = {
        OPTION({
            Name = ARGS_LIST_TRUE;
            Type = Boolean;
            CommandLineArgs = ( "yes", "yes-$(value)" );
        }),
        OPTION({
            Name = ARGS_LIST_FALSE;
            Type = Boolean;
            CommandLineArgs = ( "no", "no-$(value)" );
        }),
        OPTION({
            Name = ARGS_LIST_STRING;
            Type = String;
            CommandLineArgs = ( "string", "string-$(FLAG)", "string-$(value)" );
        }),
        OPTION({
            Name = ARGS_LIST_EMPTY;
            Type = String;
            CommandLineArgs = ( "empty", "empty-$(FLAG)", "empty-$(value)" );
        }),
        OPTION({
            Name = ARGS_LIST_STRINGLIST;
            Type = StringList;
            CommandLineArgs = ( "stringlist", "stringlist-$(value)" );
        }),
    };

    auto environment = Environment({
        pbxsetting::Setting::Create("ARGS_LIST_TRUE", "YES"),
        pbxsetting::Setting::Create("ARGS_LIST_FALSE", "NO"),
        pbxsetting::Setting::Create("ARGS_LIST_STRING", "value"),
        pbxsetting::Setting::Create("ARGS_LIST_EMPTY", ""),
        pbxsetting::Setting::Create("ARGS_LIST_STRINGLIST", "one two"),

        pbxsetting::Setting::Create("FLAG", "flag"),
    });

    auto result = Tool::OptionsResult::Create(environment, WorkingDirectory, options, FileType);
    EXPECT_EQ(result.arguments(), std::vector<std::string>({
        "yes", "yes-YES",
        "no", "no-NO",
        "string", "string-flag", "string-value",
        "empty", "empty-flag", "empty-", // NOTE: This should still end up here. Use `Condition` to filter if needed.
        "stringlist", "stringlist-one", "stringlist", "stringlist-two",
    }));
}

/*
 * Test the `CommandLineArgs` option dictionary for all option types.
 */
TEST(OptionsResult, CommandLineArgsDictionary)
{
    std::vector<pbxspec::PBX::PropertyOption::shared_ptr> options = {
        OPTION({
            Name = ARGS_DICT_BOOLEAN;
            Type = Boolean;
            CommandLineArgs = {
                YES = ( "yes", "yes-$(FLAG)" );
                NO = ( "no", "no-$(FLAG)" );
            };
        }),
        OPTION({
            Name = ARGS_DICT_VALUE;
            Type = String;
            CommandLineArgs = {
                "value" = ( "value", "value-$(FLAG)" );
                "<<otherwise>>" = ( "otherwise", "otherwise-$(FLAG)" );
            };
        }),
        OPTION({
            Name = ARGS_DICT_INVALID;
            Type = String;
            CommandLineArgs = {
                "invalid" = ( "invalid", "invalid-$(FLAG)" );
                "<<otherwise>>" = ( "invalid-otherwise", "invalid-otherwise-$(FLAG)" );
            };
        }),
        OPTION({
            Name = ARGS_DICT_EMPTY;
            Type = String;
            CommandLineArgs = {
                "empty" = ( "empty", "empty-$(FLAG)" );
                "<<otherwise>>" = ( "empty-otherwise", "empty-otherwise-$(FLAG)" );
            };
        }),
    };

    auto environment = Environment({
        pbxsetting::Setting::Create("ARGS_DICT_BOOLEAN", "YES"),
        pbxsetting::Setting::Create("ARGS_DICT_VALUE", "value"),
        pbxsetting::Setting::Create("ARGS_DICT_INVALID", "other-value"),
        pbxsetting::Setting::Create("ARGS_DICT_EMPTY", ""),

        pbxsetting::Setting::Create("FLAG", "flag"),
    });

    auto result = Tool::OptionsResult::Create(environment, WorkingDirectory, options, FileType);
    EXPECT_EQ(result.arguments(), std::vector<std::string>({
        "yes", "yes-flag",
        "value", "value-flag",
        "invalid-otherwise", "invalid-otherwise-flag",
        "empty-otherwise", "empty-otherwise-flag",
    }));
}

/*
 * Test the `Values` and `AllowedValues` option dictionaries for all option types.
 */
TEST(OptionsResult, ValuesArguments)
{
    std::vector<pbxspec::PBX::PropertyOption::shared_ptr> options = {
        OPTION({
            Name = VALUES_ARRAY;
            Type = String;
            Values = (
                { Value = "array"; CommandLineArgs = ( "array", "array-$(value)" ); },
                { Value = "other"; CommandLineArgs = ( "array-other" ); },
            );
        }),
        OPTION({
            Name = VALUES_FLAG;
            Type = String;
            Values = (
                { Value = "flag"; CommandLineFlag = "flag-$(value)"; },
                { Value = "other"; CommandLineFlag = "flag-other"; },
            );
        }),
        OPTION({
            Name = ALLOWED_VALUES_ARRAY;
            Type = String;
            Values = (
                { Value = "array"; CommandLineArgs = ( "allowed-array", "allowed-array-$(value)" ); },
                { Value = "other"; CommandLineArgs = ( "allowed-array-other" ); },
            );
        }),
        OPTION({
            Name = ALLOWED_VALUES_FLAG;
            Type = String;
            Values = (
                { Value = "flag"; CommandLineFlag = "allowed-flag-$(value)"; },
                { Value = "other"; CommandLineFlag = "allowed-flag-other"; },
            );
        }),
    };

    auto environment = Environment({
        pbxsetting::Setting::Create("VALUES_ARRAY", "array"),
        pbxsetting::Setting::Create("VALUES_FLAG", "flag"),
        pbxsetting::Setting::Create("ALLOWED_VALUES_ARRAY", "array"),
        pbxsetting::Setting::Create("ALLOWED_VALUES_FLAG", "flag"),
    });

    auto result = Tool::OptionsResult::Create(environment, WorkingDirectory, options, FileType);
    EXPECT_EQ(result.arguments(), std::vector<std::string>({
        "array", "array-array",
        "flag-flag",
        "allowed-array", "allowed-array-array",
        "allowed-flag-flag",
    }));
}

/*
 * Test the `AdditionalLinkerArgs` option.
 */
TEST(OptionsResult, AdditionalLinkerArgs)
{
    std::vector<pbxspec::PBX::PropertyOption::shared_ptr> options = {
        OPTION({
            Name = LINKER_ARGS_ARRAY;
            Type = Boolean;
            AdditionalLinkerArgs = ( "array" );
        }),
        OPTION({
            Name = LINKER_ARGS_DICT_VALID;
            Type = String;
            AdditionalLinkerArgs = {
                "value" = ( "value", "value-$(value)" );
                "<<otherwise>>" = ( "otherwise", "otherwise-$(value)" );
            };
        }),
        OPTION({
            Name = ARGS_DICT_DICT_INVALID;
            Type = String;
            AdditionalLinkerArgs = {
                "invalid" = ( "invalid", "invalid-$(value)" );
                "<<otherwise>>" = ( "invalid-otherwise", "invalid-otherwise-$(value)" );
            };
        }),
    };

    auto environment = Environment({
        pbxsetting::Setting::Create("LINKER_ARGS_ARRAY", "YES"),
        pbxsetting::Setting::Create("LINKER_ARGS_DICT_VALID", "value"),
        pbxsetting::Setting::Create("ARGS_DICT_DICT_INVALID", "other"),
    });

    auto result = Tool::OptionsResult::Create(environment, WorkingDirectory, options, FileType);
    EXPECT_EQ(result.linkerArgs(), std::vector<std::string>({
        "array",
        "value", "value-value",
        "invalid-otherwise", "invalid-otherwise-other",
    }));
    EXPECT_EQ(result.arguments(), std::vector<std::string>());
}

/*
 * Test the environment variables options.
 */
TEST(OptionsResult, EnvironmentVariables)
{
    std::vector<pbxspec::PBX::PropertyOption::shared_ptr> options = {
        OPTION({
            Name = ENVIRONMENT_VARIABLE_DIRECT;
            Type = String;
            SetValueInEnvironmentVariable = "DIRECT";
        }),
        OPTION({
            Name = ENVIRONMENT_VARIABLE_INDIRECT;
            Type = String;
            SetValueInEnvironmentVariable = "$(ENVIRONMENT_VARIABLE_INDIRECT_NAME)";
        }),
        OPTION({
            Name = ENVIRONMENT_VARIABLE_EMPTY;
            Type = String;
            SetValueInEnvironmentVariable = "EMPTY";
        }),
    };

    auto environment = Environment({
        pbxsetting::Setting::Create("ENVIRONMENT_VARIABLE_DIRECT", "direct"),
        pbxsetting::Setting::Create("ENVIRONMENT_VARIABLE_INDIRECT", "indirect"),
        pbxsetting::Setting::Create("ENVIRONMENT_VARIABLE_INDIRECT_NAME", "INDIRECT"),
        pbxsetting::Setting::Create("ENVIRONMENT_VARIABLE_EMPTY", ""),
    });

    auto result = Tool::OptionsResult::Create(environment, WorkingDirectory, options, FileType);
    std::unordered_map<std::string, std::string> expectedEnvironmentVariables = {
        { "DIRECT", "direct" },
        { "INDIRECT", "indirect" },
        { "EMPTY", "" },
    };
    EXPECT_EQ(result.environment(), expectedEnvironmentVariables);
    EXPECT_EQ(result.arguments(), std::vector<std::string>());
}

/*
 * Test the `Architectures` option is respected.
 */
TEST(OptionsResult, Architectures)
{
    std::vector<pbxspec::PBX::PropertyOption::shared_ptr> options = {
        OPTION({
            Name = ARCHS_ARM;
            Architectures = ( armv7, arm64 );
            Type = Boolean;
            CommandLineFlag = "arm";
        }),
        OPTION({
            Name = ARCHS_X86;
            Architectures = ( x86, x86_64 );
            Type = Boolean;
            CommandLineFlag = "x86";
        }),
    };

    auto environment = Environment({
        pbxsetting::Setting::Create("ARCHS_ARM", "YES"),
        pbxsetting::Setting::Create("ARCHS_x86", "YES"),

        pbxsetting::Setting::Create("CURRENT_ARCH", "armv7"),
        pbxsetting::Setting::Create("arch", "armv7"),
    });

    auto result = Tool::OptionsResult::Create(environment, WorkingDirectory, options, FileType);
    EXPECT_EQ(result.arguments(), std::vector<std::string>({
        "arm",
    }));
}

/*

To test:
    PropertyOption::defaultValue()
    PropertyOption::fileTypes()
    PropertyOption::flattenRecursiveSearchPathsInValue()

Unsupported:
    PropertyOption::commandLineCondition()
    PropertyOption::condition()
    PropertyOption::conditionFlavors()
    PropertyOption::isCommandInput()
    PropertyOption::isCommandOutput()
    PropertyOption::isInputDependency()
    PropertyOption::inputInclusions()
    PropertyOption::outputDependencies()
    PropertyOption::outputsAreSourceFiles()

*/

