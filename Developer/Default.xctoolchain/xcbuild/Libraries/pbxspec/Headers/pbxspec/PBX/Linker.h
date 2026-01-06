/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxspec_PBX_Linker_h
#define __pbxspec_PBX_Linker_h

#include <pbxspec/PBX/Tool.h>

#include <ext/optional>

namespace pbxspec { namespace PBX {

class Linker : public Tool {
public:
    typedef std::shared_ptr <Linker> shared_ptr;
    typedef std::vector <shared_ptr> vector;

protected:
    ext::optional<std::vector<std::string>> _binaryFormats;
    ext::optional<pbxsetting::Value>        _dependencyInfoFile;
    ext::optional<bool>                     _supportsInputFileList;

protected:
    Linker();

public:
    virtual ~Linker();

public:
    inline SpecificationType type() const override
    { return Linker::Type(); }

public:
    inline Linker::shared_ptr base() const
    { return std::static_pointer_cast<Linker>(Tool::base()); }

public:
    inline ext::optional<std::vector<std::string>> const &binaryFormats() const
    { return _binaryFormats; }

public:
    inline ext::optional<pbxsetting::Value> const &dependencyInfoFile() const
    { return _dependencyInfoFile; }

public:
    inline bool supportsInputFileList() const
    { return _supportsInputFileList.value_or(false); }
    inline ext::optional<bool> supportsInputFileListOptional() const
    { return _supportsInputFileList; }

protected:
    bool inherit(Specification::shared_ptr const &base) override;
    bool inherit(Tool::shared_ptr const &base) override;
    virtual bool inherit(Linker::shared_ptr const &base);

protected:
    friend class Specification;
    bool parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

protected:
    static Linker::shared_ptr Parse(Context *context, plist::Dictionary const *dict);

public:
    static inline SpecificationType Type()
    { return SpecificationType::Linker; }
};

} }

#endif  // !__pbxspec_PBX_Linker_h
