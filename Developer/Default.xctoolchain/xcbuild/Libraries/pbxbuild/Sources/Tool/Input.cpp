/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/Input.h>

namespace Tool = pbxbuild::Tool;
namespace Target = pbxbuild::Target;

Tool::Input::
Input(
    std::string const &path,
    pbxspec::PBX::FileType::shared_ptr const &fileType,
    Target::BuildRules::BuildRule::shared_ptr const &buildRule,
    ext::optional<std::string> const &fileNameDisambiguator,
    ext::optional<std::string> const &localization,
    ext::optional<std::string> const &localizationGroupIdentifier,
    ext::optional<std::vector<std::string>> const &attributes,
    ext::optional<std::vector<std::string>> const &compilerFlags) :
    _path                       (path),
    _fileType                   (fileType),
    _buildRule                  (buildRule),
    _fileNameDisambiguator      (fileNameDisambiguator),
    _localization               (localization),
    _localizationGroupIdentifier(localizationGroupIdentifier),
    _attributes                 (attributes),
    _compilerFlags              (compilerFlags)
{
}

Tool::Input::
Input(std::string const &path, pbxspec::PBX::FileType::shared_ptr const &fileType) :
    Input(
        path,
        fileType,
        nullptr,
        ext::nullopt,
        ext::nullopt,
        ext::nullopt,
        ext::nullopt,
        ext::nullopt)
{
}

Tool::Input::
~Input()
{
}

