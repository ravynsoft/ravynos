/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Object_h
#define __plist_Object_h

#include <plist/Base.h>
#include <plist/ObjectType.h>

#include <memory>
#include <string>

namespace plist {

class Object {
protected:
    Object()
    {
    }

public:
    virtual ~Object()
    {
    }

public:
    virtual ObjectType type() const = 0;

public:
    virtual void release() const
    {
        delete this;
    }

protected:
    virtual std::unique_ptr<Object> _copy() const = 0;

public:
    std::unique_ptr<Object> copy() const
    { return _copy(); }

public:
    virtual bool equals(Object const *obj) const
    {
        return (obj == this);
    }

public:
    static std::unique_ptr<Object> Coerce(Object const *obj);

public:
    static inline ObjectType Type()
    {
        return ObjectType::None;
    }
};

template <typename T>
static inline T *CastTo(Object *obj)
{
    return (obj != nullptr && (obj->type() == T::Type() || T::Type() == ObjectType())) ? static_cast <T *> (obj) : nullptr;
}

template <typename T>
static inline T const *CastTo(Object const *obj)
{
    return (obj != nullptr && (obj->type() == T::Type() || T::Type() == ObjectType())) ? static_cast <T const *> (obj) : nullptr;
}

}

#endif  // !__plist_Object_h
