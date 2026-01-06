/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Null_h
#define __plist_Null_h

#include <plist/Base.h>
#include <plist/Object.h>

namespace plist {

class Null : public Object {
private:
    Null()
    {
    }

public:
    static std::unique_ptr<Null> New();

public:
    static std::unique_ptr<Null> Coerce(Object const *obj);

public:
    virtual ObjectType type() const
    {
        return Null::Type();
    }

    static inline ObjectType Type()
    {
        return ObjectType::Null;
    }

protected:
    virtual std::unique_ptr<Object> _copy() const;

public:
    std::unique_ptr<Null> copy() const
    { return plist::static_unique_pointer_cast<Null>(_copy()); }

public:
    virtual bool equals(Object const *obj) const
    {
        if (Object::equals(obj))
            return true;

        Null const *objt = CastTo <Null> (obj);
        return (objt != nullptr && equals(objt));
    }

    virtual bool equals(Null const *obj) const
    {
        return (obj != nullptr && obj == this);
    }
};

}

#endif  // !__plist_Null_h
