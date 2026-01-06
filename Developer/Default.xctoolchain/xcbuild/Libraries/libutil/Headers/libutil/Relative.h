/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __libutil_Relative_h
#define __libutil_Relative_h

#include <libutil/Unix.h>
#include <libutil/Windows.h>

#include <string>
#include <ext/optional>

namespace libutil {
namespace Path {

template<typename Traits>
class BaseAbsolute;

/*
 * A file path. May be absolute or relative.
 */
template<typename Traits>
class BaseRelative {
private:
    std::string _raw;

public:
    explicit BaseRelative(std::string const &raw);

public:
    bool operator==(BaseRelative<Traits> const &rhs) const
    { return this->normalized() == rhs.normalized(); }
    bool operator!=(BaseRelative<Traits> const &rhs) const
    { return this->normalized() != rhs.normalized(); }

public:
    /*
     * The raw, non-normalized path string.
     */
    std::string const &raw() const
    { return _raw; }

public:
    /*
     * The normalized path string.
     */
    std::string normalized() const;

public:
    /*
     * Returns the absolute path, if this path is already absolute.
     */
    ext::optional<BaseAbsolute<Traits>> absolute() const;

    /*
     * Resolve against a working directory to create an absolute
     * path. If already absolute, the returned path is unchanged.
     */
    BaseAbsolute<Traits> resolved(BaseAbsolute<Traits> const &workingDirectory) const;

public:
    /*
     * The parent directory of this path.
     */
    BaseRelative<Traits> parent() const;

    /*
     * A child of this path.
     */
    BaseRelative<Traits> child(std::string const &name) const;

    /*
     * The base name of the path.
     */
    std::string base(bool extension = true) const;

    /*
     * The file extension of the path.
     */
    std::string extension() const;

    /*
     * If the path's file extension matches.
     */
    bool extension(std::string const &extension, bool insensitive = true) const;
};

#if _WIN32
using Relative = BaseRelative<Windows>;
#else
using Relative = BaseRelative<Unix>;
#endif

}
}

#endif // !__libutil_Relative_h
