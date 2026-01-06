/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Array_h
#define __plist_Array_h

#include <plist/Base.h>
#include <plist/Object.h>

#include <vector>

namespace plist {

class Array : public Object {
public:
    typedef std::vector<std::unique_ptr<Object>> Vector;

private:
    Vector _array;

public:
    Array()
    {
    }

    virtual ~Array()
    {
        clear();
    }

public:
    static std::unique_ptr<Array> New();

public:
    inline bool empty() const
    {
        return _array.empty();
    }

    inline size_t count() const
    {
        return _array.size();
    }

    inline Object const *value(size_t index) const
    {
        return _array[index].get();
    }

    inline Object *value(size_t index)
    {
        return _array[index].get();
    }

    template <typename T>
    inline T *value(size_t index)
    {
        return CastTo <T> (value(index));
    }

    template <typename T>
    inline T const *value(size_t index) const
    {
        return CastTo <T> (value(index));
    }

public:
    inline void remove(size_t index)
    {
        _array.erase(_array.begin() + index);
    }

public:
    inline void clear()
    {
        _array.clear();
    }

public:
    inline void insert(size_t index, std::unique_ptr<Object> obj)
    {
        _array.insert(_array.begin() + index, std::move(obj));
    }

    inline void set(size_t index, std::unique_ptr<Object> obj)
    {
        _array[index] = std::move(obj);
    }

public:
    inline void append(std::unique_ptr<Object> obj)
    {
        _array.push_back(std::move(obj));
    }

public:
    inline Vector::const_iterator begin() const
    {
        return _array.begin();
    }

    inline Vector::const_iterator end() const
    {
        return _array.end();
    }

public:
    static std::unique_ptr<Array> Coerce(Object const *obj);

public:
    virtual ObjectType type() const
    {
        return Array::Type();
    }

    static inline ObjectType Type()
    {
        return ObjectType::Array;
    }

protected:
    virtual std::unique_ptr<Object> _copy() const;

public:
    std::unique_ptr<Array> copy() const
    { return plist::static_unique_pointer_cast<Array>(_copy()); }

public:
    virtual bool equals(Object const *obj) const
    {
        if (Object::equals(obj))
            return true;

        Array const *objt = CastTo <Array> (obj);
        return (objt != nullptr && equals(objt));
    }

    virtual bool equals(Array const *obj) const
    {
        if (obj == nullptr)
            return false;

        if (count() != obj->count())
            return false;

        for (size_t n = 0; n < count(); n++) {
            if (!value(n)->equals(obj->value(n)))
                return false;
        }

        return true;
    }

public:
    void merge(Array const *array);
};

}

#endif  // !__plist_Array_h
