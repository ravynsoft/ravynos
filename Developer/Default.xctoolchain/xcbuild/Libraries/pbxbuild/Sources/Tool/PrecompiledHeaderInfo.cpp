/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/PrecompiledHeaderInfo.h>
#include <libutil/FSUtil.h>
#include <libutil/Wildcard.h>
#include <libutil/md5.h>

#include <sstream>
#include <iomanip>

namespace Tool = pbxbuild::Tool;
using libutil::FSUtil;
using libutil::Wildcard;

Tool::PrecompiledHeaderInfo::
PrecompiledHeaderInfo(std::string const &prefixHeader, pbxspec::PBX::FileType::shared_ptr const &fileType, std::vector<std::string> const &arguments, std::vector<std::string> const &relevantArguments) :
    _prefixHeader     (prefixHeader),
    _fileType         (fileType),
    _arguments        (arguments),
    _relevantArguments(relevantArguments)
{
}

Tool::PrecompiledHeaderInfo::
~PrecompiledHeaderInfo()
{
}

pbxsetting::Value Tool::PrecompiledHeaderInfo::
logicalOutputPath() const
{
    pbxsetting::Value outputDirectory = pbxsetting::Value::Parse("$(PRECOMP_DESTINATION_DIR)/$(PRODUCT_NAME)-") + pbxsetting::Value::String(hash());
    return outputDirectory + pbxsetting::Value::String("/" + FSUtil::GetBaseName(_prefixHeader));
}

pbxsetting::Value Tool::PrecompiledHeaderInfo::
compileOutputPath() const
{
    return logicalOutputPath() + pbxsetting::Value::String(".pch");
}

pbxsetting::Value Tool::PrecompiledHeaderInfo::
serializedOutputPath() const
{
    return logicalOutputPath() + pbxsetting::Value::String(".hash-criteria");
}

std::string Tool::PrecompiledHeaderInfo::
hash() const
{
    // TODO(grp): Generate this hash properly.
    std::vector<uint8_t> content = serialize();

    md5_state_t state;
    md5_init(&state);
    md5_append(&state, reinterpret_cast<const md5_byte_t *>(content.data()), content.size());
    uint8_t digest[16];
    md5_finish(&state, reinterpret_cast<md5_byte_t *>(&digest));

    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint8_t c : digest) {
        ss << std::setw(2) << static_cast<int>(c);
    }
    return ss.str();
}

std::vector<uint8_t> Tool::PrecompiledHeaderInfo::
serialize() const
{
    std::string result;
    for (std::string const &argument : _relevantArguments) {
        result += argument + "\n";
    }

    // TODO(grp): Populate these properly from the compiler and SDK.
    result += "<FIXME: target>\n";
    result += "<FIXME: version>\n";
    result += "SDK_PRODUCT_BUILD_VERSION=FIXME\n";
    result += "FIXME\n";

    return std::vector<uint8_t>(result.begin(), result.end());
}

Tool::PrecompiledHeaderInfo Tool::PrecompiledHeaderInfo::
Create(pbxspec::PBX::Compiler::shared_ptr const &compiler, std::string const &prefixHeader, pbxspec::PBX::FileType::shared_ptr const &fileType, std::vector<std::string> const &arguments)
{
    std::vector<std::string> relevantArguments;
    for (std::string const &argument : arguments) {
        bool ignore = false;
        if (compiler->patternsOfFlagsNotAffectingPrecomps()) {
            for (std::string const &pattern : *compiler->patternsOfFlagsNotAffectingPrecomps()) {
                if (Wildcard::Match(pattern, argument)) {
                    ignore = true;
                    break;
                }
            }
        }

        if (!ignore) {
            relevantArguments.push_back(argument);
        }
    }

    return Tool::PrecompiledHeaderInfo(prefixHeader, fileType, arguments, relevantArguments);
}
