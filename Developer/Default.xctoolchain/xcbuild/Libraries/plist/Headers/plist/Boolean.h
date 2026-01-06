/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Boolean_h
#define __plist_Boolean_h

#include <plist/Base.h>
#include <plist/Object.h>

namespace plist {

class Boolean : public Object {
private:
    bool _value;

private:
    Boolean(bool value) :
        _value(value)
    {
    }

public:
    static std::unique_ptr<Boolean> New(bool value);

public:
    inline bool value() const
    {
        return _value;
    }

public:
    static std::unique_ptr<Boolean> Coerce(Object const *obj);

public:
    virtual ObjectType type() const
    {
        return Boolean::Type();
    }

    static inline ObjectType Type()
    {
        return ObjectType::Boolean;
    }

protected:
    virtual std::unique_ptr<Object> _copy() const;

public:
    std::unique_ptr<Boolean> copy() const
    { return plist::static_unique_pointer_cast<Boolean>(_copy()); }

public:
    virtual bool equals(Object const *obj) const;
    virtual bool equals(Boolean const *obj) const;
};

}

#endif  // !__plist_Boolean_h
