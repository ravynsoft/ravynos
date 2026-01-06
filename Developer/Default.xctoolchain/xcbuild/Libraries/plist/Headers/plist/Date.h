/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Date_h
#define __plist_Date_h

#include <plist/Base.h>
#include <plist/Object.h>
#ifdef __linux__
#include <cstdint>
#endif

#include <ctime>

namespace plist {

class Date : public Object {
private:
    struct tm _value;

public:
    Date(struct tm const &tm) :
        _value(tm)
    {
    }

    Date(std::string const &value);
    Date(uint64_t value = 0);

public:
    inline struct tm const &value() const
    {
        return _value;
    }

    inline void setValue(struct tm const &value)
    {
        _value = value;
    }

public:
    void setStringValue(std::string const &value);
    std::string stringValue() const;

public:
    void setUnixTimeValue(uint64_t value);
    uint64_t unixTimeValue() const;

public:
    static std::unique_ptr<Date> New(struct tm const &value = tm());
    static std::unique_ptr<Date> New(std::string const &value);
    static std::unique_ptr<Date> New(uint64_t value);

public:
    static std::unique_ptr<Date> Coerce(Object const *obj);

public:
    virtual ObjectType type() const
    {
        return Date::Type();
    }

    static inline ObjectType Type()
    {
        return ObjectType::Date;
    }

protected:
    virtual std::unique_ptr<Object> _copy() const;

public:
    std::unique_ptr<Date> copy() const
    { return plist::static_unique_pointer_cast<Date>(_copy()); }

public:
    virtual bool equals(Object const *obj) const
    {
        if (Object::equals(obj))
            return true;

        Date const *objt = CastTo <Date> (obj);
        return (objt != nullptr && equals(objt));
    }

    virtual bool equals(Date const *obj) const
    {
        return (obj != nullptr && (obj == this || unixTimeValue() == obj->unixTimeValue()));
    }
};

}

#endif  // !__plist_Date_h
