/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxspec_PBX_Specification_h
#define __pbxspec_PBX_Specification_h

#include <pbxspec/SpecificationType.h>

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <ext/optional>

namespace libutil { class Filesystem; }
namespace plist { class Dictionary; }
namespace pbxspec { class Manager; }
namespace pbxspec { class Context; }

namespace pbxspec { namespace PBX {

class Specification {
public:
    typedef std::shared_ptr <Specification> shared_ptr;
    typedef std::vector <shared_ptr> vector;

protected:
    Specification::shared_ptr  _base;
    ext::optional<std::string> _basedOnIdentifier;
    ext::optional<std::string> _basedOnDomain;
    std::string                _identifier;
    std::string                _domain;
    ext::optional<bool>        _isGlobalDomainInUI;
    ext::optional<std::string> _clazz;
    ext::optional<std::string> _name;
    ext::optional<std::string> _description;
    ext::optional<std::string> _vendor;
    ext::optional<std::string> _version;

protected:
    Specification();

public:
    bool operator==(Specification const &rhs) const;
    bool operator!=(Specification const &rhs) const;

public:
    virtual SpecificationType type() const = 0;

public:
    inline Specification::shared_ptr base() const
    { return _base; }

public:
    inline ext::optional<std::string> const &basedOnIdentifier() const
    { return _basedOnIdentifier; }
    inline ext::optional<std::string> const &basedOnDomain() const
    { return _basedOnDomain; }

public:
    inline std::string const &identifier() const
    { return _identifier; }

public:
    inline std::string const &domain() const
    { return _domain; }
    inline bool isGlobalDomainInUI() const
    { return _isGlobalDomainInUI.value_or(false); }
    inline ext::optional<bool> isGlobalDomainInUIOptional() const
    { return _isGlobalDomainInUI; }

public:
    inline ext::optional<std::string> const &clazz() const
    { return _clazz; }
    inline ext::optional<std::string> const &name() const
    { return _name; }
    inline ext::optional<std::string> const &description() const
    { return _description; }
    inline ext::optional<std::string> const &vendor() const
    { return _vendor; }
    inline ext::optional<std::string> const &version() const
    { return _version; }

protected:
    virtual bool parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check);

protected:
    friend class pbxspec::Manager;
    virtual bool inherit(Specification::shared_ptr const &base);

protected:
    static bool ParseType(Context *context, plist::Dictionary const *dict, SpecificationType expectedType);

public:
    static ext::optional<Specification::vector> Open(
        libutil::Filesystem const *filesystem,
        Context *context,
        std::string const &filename,
        ext::optional<SpecificationType> defaultType = ext::nullopt);

private:
    static Specification::shared_ptr Parse(Context *context, plist::Dictionary const *dict, ext::optional<SpecificationType> defaultType);
};

} }

#endif  // !__pbxspec_PBX_Specification_h
