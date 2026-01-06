/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __libutil_Unix_h
#define __libutil_Unix_h

#include <string>

namespace libutil {
namespace Path {

template<typename Traits>
class BaseAbsolute;

template<typename Traits>
class BaseRelative;

class Unix {
public:
    Unix() = delete;
    ~Unix() = delete;

public:
    friend class BaseAbsolute<Unix>;
    friend class BaseRelative<Unix>;

private:
    static char Separator;
    static bool IsSeparator(char c);

private:
    static bool IsAbsolute(std::string const &path, size_t *start = nullptr);
    static bool Resolve(
        std::string const &path,
        std::string const &against,
        std::string *base,
        std::string *relative);
};

}
}

#endif // !__libutil_Unix_h
