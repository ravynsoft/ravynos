/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_UID_h
#define __plist_UID_h

#include <plist/Base.h>
#include <plist/Object.h>

namespace plist {

class UID : public Object {
private:
    uint32_t _value;

public:
    UID(uint32_t value = 0) :
        _value(value)
    {
    }

public:
    inline uint32_t value() const
    {
        return _value;
    }

    inline void setValue(uint32_t value)
    {
        _value = value;
    }

public:
    static std::unique_ptr<UID> New(uint32_t value = 0);

public:
    static std::unique_ptr<UID> Coerce(Object const *obj);

public:
    virtual ObjectType type() const
    {
        return UID::Type();
    }

    static inline ObjectType Type()
    {
        return ObjectType::UID;
    }

protected:
    virtual std::unique_ptr<Object> _copy() const;

public:
    std::unique_ptr<UID> copy() const
    { return plist::static_unique_pointer_cast<UID>(_copy()); }

public:
    virtual bool equals(Object const *obj) const
    {
        if (Object::equals(obj))
            return true;

        UID const *objt = CastTo <UID> (obj);
        return (objt != nullptr && equals(objt));
    }

    virtual bool equals(UID const *obj) const
    {
        return (obj != nullptr && (obj == this || value() == obj->value()));
    }
};

}

#endif  // !__plist_UID_h
