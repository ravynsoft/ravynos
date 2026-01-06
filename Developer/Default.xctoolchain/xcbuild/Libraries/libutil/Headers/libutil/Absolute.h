/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __libutil_Absolute_h
#define __libutil_Absolute_h

#include <libutil/Relative.h>
#include <libutil/Unix.h>
#include <libutil/Windows.h>

#include <string>
#include <ext/optional>

namespace libutil {
namespace Path {

/*
 * A verified absolute path.
 */
template<class Traits>
class BaseAbsolute {
private:
    BaseRelative<Traits> _inner;

private:
    friend class BaseRelative<Traits>;
    explicit BaseAbsolute(BaseRelative<Traits> const &inner);

public:
    bool operator==(BaseAbsolute<Traits> const &rhs) const
    { return _inner == rhs._inner; }
    bool operator!=(BaseAbsolute<Traits> const &rhs) const
    { return _inner != rhs._inner; }

public:
    /*
     * The raw, non-normalized path string.
     */
    std::string const &raw() const
    { return _inner.raw(); }

    /*
     * The inner relative path.
     */
    BaseRelative<Traits> const &relative() const
    { return _inner; }

public:
    /*
     * The normalized path string.
     */
    std::string normalized() const
    { return _inner.normalized(); }

public:
    /*
     * Relative path to this absolute path from another path.
     */
    ext::optional<BaseRelative<Traits>> from(BaseAbsolute<Traits> const &from) const;

public:
    /*
     * The parent directory of this path, or this path if at the root.
     */
    BaseAbsolute<Traits> parent() const
    { return BaseAbsolute(_inner.parent()); }

    /*
     * A child of this path.
     */
    BaseAbsolute<Traits> child(std::string const &name) const
    { return BaseAbsolute(_inner.child(name)); }

    /*
     * The base name of the path.
     */
    std::string base(bool extension = true) const
    { return _inner.base(extension); }

    /*
     * The file extension of the path.
     */
    std::string extension() const
    { return _inner.extension(); }

    /*
     * If the path's file extension matches.
     */
    bool extension(std::string const &extension, bool insensitive = true) const
    { return _inner.extension(extension, insensitive); }

public:
    /*
     * Create an absolute path.
     */
    static ext::optional<BaseAbsolute<Traits>> Create(std::string const &raw)
    { return BaseRelative<Traits>(raw).absolute(); }
};

#if _WIN32
using Absolute = BaseAbsolute<Windows>;
#else
using Absolute = BaseAbsolute<Unix>;
#endif

}
}

#endif // !__libutil_Absolute_h
