/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/AuxiliaryFile.h>

namespace Tool = pbxbuild::Tool;

Tool::AuxiliaryFile::Chunk::
Chunk(Type type, ext::optional<std::vector<uint8_t>> const &data, ext::optional<std::string> const &file) :
    _type(type),
    _data(data),
    _file(file)
{
}

Tool::AuxiliaryFile::Chunk Tool::AuxiliaryFile::Chunk::
Data(std::vector<uint8_t> const &data)
{
    return Chunk(Type::Data, data, ext::nullopt);
}

Tool::AuxiliaryFile::Chunk Tool::AuxiliaryFile::Chunk::
File(std::string const &file)
{
    return Chunk(Type::File, ext::nullopt, file);
}

Tool::AuxiliaryFile::
AuxiliaryFile(std::string const &path, std::vector<Chunk> const &chunks, bool executable) :
    _path      (path),
    _chunks    (chunks),
    _executable(executable)
{
}

Tool::AuxiliaryFile Tool::AuxiliaryFile::
Data(std::string const &path, std::vector<uint8_t> const &data, bool executable)
{
    return Tool::AuxiliaryFile(path, { Chunk::Data(data) }, executable);
}

Tool::AuxiliaryFile Tool::AuxiliaryFile::
File(std::string const &path, std::string const &file, bool executable)
{
    return Tool::AuxiliaryFile(path, { Chunk::File(file) }, executable);
}


