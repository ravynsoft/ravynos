/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_AuxiliaryFile_h
#define __pbxbuild_Tool_AuxiliaryFile_h

#ifdef __linux__
#include <cstdint>
#endif
#include <string>
#include <vector>
#include <ext/optional>

namespace pbxbuild {
namespace Tool {

class AuxiliaryFile {
public:
    class Chunk {
    public:
        enum class Type {
            Data,
            File,
        };

    private:
        Type                                _type;

    private:
        ext::optional<std::vector<uint8_t>> _data;
        ext::optional<std::string>          _file;

    private:
        Chunk(
            Type type,
            ext::optional<std::vector<uint8_t>> const &data,
            ext::optional<std::string> const &file);

    public:
        Type type() const
        { return _type; }

    public:
        ext::optional<std::vector<uint8_t>> const &data() const
        { return _data; }
        ext::optional<std::string> const &file() const
        { return _file; }

    public:
        static Chunk Data(std::vector<uint8_t> const &data);
        static Chunk File(std::string const &file);
    };

private:
    std::string        _path;
    std::vector<Chunk> _chunks;
    bool               _executable;

public:
    AuxiliaryFile(std::string const &path, std::vector<Chunk> const &chunks, bool executable = false);

public:
    std::string const &path() const
    { return _path; }
    std::vector<Chunk> const &chunks() const
    { return _chunks; }
    bool executable() const
    { return _executable; }

public:
    static AuxiliaryFile Data(std::string const &path, std::vector<uint8_t> const &data, bool executable = false);
    static AuxiliaryFile File(std::string const &path, std::string const &file, bool executable = false);
};

}
}

#endif // !__pbxbuild_Tool_AuxiliaryFile_h
