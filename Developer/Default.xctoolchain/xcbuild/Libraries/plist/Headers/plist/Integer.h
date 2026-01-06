/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Integer_h
#define __plist_Integer_h

#include <plist/Base.h>
#include <plist/Object.h>

namespace plist {

class Integer : public Object {
private:
    int64_t _value;

public:
    Integer(int64_t value = 0) :
        _value(value)
    {
    }

public:
    inline int64_t value() const
    {
        return _value;
    }

    inline void setValue(int64_t value)
    {
        _value = value;
    }

public:
    static std::unique_ptr<Integer> New(int64_t value = 0);

public:
    static std::unique_ptr<Integer> Coerce(Object const *obj);

public:
    virtual ObjectType type() const
    {
        return Integer::Type();
    }

    static inline ObjectType Type()
    {
        return ObjectType::Integer;
    }

protected:
    virtual std::unique_ptr<Object> _copy() const;

public:
    std::unique_ptr<Integer> copy() const
    { return plist::static_unique_pointer_cast<Integer>(_copy()); }

public:
    virtual bool equals(Object const *obj) const
    {
        if (Object::equals(obj))
            return true;

        Integer const *objt = CastTo <Integer> (obj);
        return (objt != nullptr && equals(objt));
    }

    virtual bool equals(Integer const *obj) const
    {
        return (obj != nullptr && (obj == this || value() == obj->value()));
    }
};

}

#endif  // !__plist_Integer_h
