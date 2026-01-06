/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_PBX_FileReference_h
#define __pbxproj_PBX_FileReference_h

#ifdef __linux__
#include <cstdint>
#endif

#include <pbxproj/PBX/GroupItem.h>

namespace pbxproj { namespace PBX {

class FileReference : public GroupItem {
public:
    enum class FileEncoding : uint32_t {
        Default = 0,
        UTF8 = 4,
        UTF16 = 10,
        UTF16BE = 2415919360,
        UTF16LE = 2483028224,
        Western = 30,
        Japanese = 2147483649,
        TraditionalChinese = 2147483650,
        Korean = 2147483651,
        Arabic = 2147483652,
        Hebrew = 2147483653,
        Greek = 2147483654,
        Cyrillic = 2147483655,
        SimplifiedChinese = 2147483673,
        CentralEuropean = 2147483677,
        Turkish = 2147483683,
        Icelandic = 2147483685,
    };

public:
    enum class LineEnding : uint32_t {
        Unix = 0,
        MacOS = 1,
        Windows = 2,
    };

public:
    typedef std::shared_ptr <FileReference> shared_ptr;
    typedef std::vector <shared_ptr> vector;

private:
    std::string  _lastKnownFileType;
    std::string  _explicitFileType;
    std::string  _xcLanguageSpecificationIdentifier;
    bool         _includeInIndex;
    FileEncoding _fileEncoding;
    LineEnding   _lineEnding;

public:
    FileReference();

public:
    inline std::string const &lastKnownFileType() const
    { return _lastKnownFileType; }
    inline std::string const &explicitFileType() const
    { return _explicitFileType; }

public:
    inline std::string const &xcLanguageSpecificationIdentifier() const
    { return _xcLanguageSpecificationIdentifier; }

public:
    inline bool includeInIndex() const
    { return _includeInIndex; }

public:
    inline FileEncoding fileEncoding() const
    { return _fileEncoding; }
    inline LineEnding lineEnding() const
    { return _lineEnding; }

protected:
    bool parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

public:
    static inline char const *Isa()
    { return ISA::PBXFileReference; }
};

} }

#endif  // !__pbxproj_PBX_FileReference_h
