/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_Input_h
#define __pbxbuild_Tool_Input_h

#include <pbxbuild/Target/BuildRules.h>

#include <string>
#include <ext/optional>

namespace libutil { class Filesystem; }
namespace pbxsetting { class Environment; }

namespace pbxbuild {
namespace Tool {

class Environment;

/*
 * Represents a resolved file to build as part of a phase.
 */
class Input {
private:
    std::string                               _path;
    pbxspec::PBX::FileType::shared_ptr        _fileType;

private:
    Target::BuildRules::BuildRule::shared_ptr _buildRule;
    ext::optional<std::string>                _fileNameDisambiguator;

private:
    ext::optional<std::string>                _localization;
    ext::optional<std::string>                _localizationGroupIdentifier;

private:
    ext::optional<std::vector<std::string>>   _attributes;
    ext::optional<std::vector<std::string>>   _compilerFlags;

public:
    Input(
        std::string const &path,
        pbxspec::PBX::FileType::shared_ptr const &fileType,
        Target::BuildRules::BuildRule::shared_ptr const &buildRule,
        ext::optional<std::string> const &fileNameDisambiguator,
        ext::optional<std::string> const &localization,
        ext::optional<std::string> const &localizationGroupIdentifier,
        ext::optional<std::vector<std::string>> const &attributes,
        ext::optional<std::vector<std::string>> const &compilerFlags);
    Input(std::string const &path, pbxspec::PBX::FileType::shared_ptr const &fileType);
    ~Input();

public:
    /*
     * The resolved absolute path to the file.
     */
    std::string const &path() const
    { return _path; }

    /*
     * The type of this file. Optional.
     */
    pbxspec::PBX::FileType::shared_ptr const &fileType() const
    { return _fileType; }

public:
    /*
     * The build rule saying *how* to build this file. Optional.
     */
    Target::BuildRules::BuildRule::shared_ptr const &buildRule() const
    { return _buildRule; }

    /*
     * A disambiguation identifier for different files with the same base name
     * within a target. This should be used for the output path along with the
     * base name in order to avoid overwriting outputs.
     */
    ext::optional<std::string> const &fileNameDisambiguator() const
    { return _fileNameDisambiguator; }

public:
    /*
     * The localization this file is for. This is relevant for variant groups,
     * which contain many versions of the same file for different lproj outputs.
     */
    ext::optional<std::string> const &localization() const
    { return _localization; }

    /*
     * An opaque identifier of a localization group. Individual localizations
     * are found within a group.
     */
    ext::optional<std::string> const &localizationGroupIdentifier() const
    { return _localizationGroupIdentifier; }

public:
    /*
     * Additional attributes of the file, usually header properties.
     */
    ext::optional<std::vector<std::string>> const &attributes() const
    { return _attributes; }

    /*
     * Additional flags to add when compiling the file.
     */
    ext::optional<std::vector<std::string>> const &compilerFlags() const
    { return _compilerFlags; }
};

}
}

#endif // !__pbxbuild_Tool_Input_h
