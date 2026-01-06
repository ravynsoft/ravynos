/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxspec_PBX_PackageType_h
#define __pbxspec_PBX_PackageType_h

#include <pbxspec/PBX/Specification.h>
#include <pbxsetting/Level.h>

#include <memory>
#include <string>
#include <vector>
#include <ext/optional>

namespace pbxspec { namespace PBX {

class PackageType : public Specification {
public:
    typedef std::shared_ptr <PackageType> shared_ptr;
    typedef std::vector <shared_ptr> vector;

public:
    class ProductReference {
    protected:
        ext::optional<std::string> _name;
        ext::optional<std::string> _fileType;
        ext::optional<bool>        _isLaunchable;

    protected:
        friend class PackageType;
        ProductReference();

    public:
        inline ext::optional<std::string> const &name() const
        { return _name; }

    public:
        inline ext::optional<std::string> const &fileType() const
        { return _fileType; }

    public:
        inline ext::optional<bool> isLaunchable() const
        { return _isLaunchable; }

    protected:
        bool parse(plist::Dictionary const *dict);
    };

protected:
    ext::optional<pbxsetting::Level> _defaultBuildSettings;
    ext::optional<ProductReference>  _productReference;

protected:
    PackageType();

public:
    virtual ~PackageType();

public:
    inline SpecificationType type() const override
    { return PackageType::Type(); }

public:
    inline PackageType::shared_ptr base() const
    { return std::static_pointer_cast<PackageType>(Specification::base()); }

public:
    inline ext::optional<ProductReference> const &productReference() const
    { return _productReference; }

public:
    inline ext::optional<pbxsetting::Level> const &defaultBuildSettings() const
    { return _defaultBuildSettings; }

protected:
    friend class Specification;
    bool parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

protected:
    bool inherit(Specification::shared_ptr const &base) override;
    virtual bool inherit(PackageType::shared_ptr const &base);

protected:
    static PackageType::shared_ptr Parse(Context *context, plist::Dictionary const *dict);

public:
    static inline SpecificationType Type()
    { return SpecificationType::PackageType; }
};

} }

#endif  // !__pbxspec_PBX_PackageType_h
