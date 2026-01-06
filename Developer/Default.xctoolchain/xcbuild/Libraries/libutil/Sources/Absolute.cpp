/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <libutil/Absolute.h>
#include <libutil/Relative.h>
#include <libutil/Unix.h>
#include <libutil/Windows.h>

#include <algorithm>

namespace Path = libutil::Path;

template<class Traits>
Path::BaseAbsolute<Traits>::
BaseAbsolute(Path::BaseRelative<Traits> const &inner) :
    _inner(inner)
{
}

template<class Traits>
ext::optional<Path::BaseRelative<Traits>> Path::BaseAbsolute<Traits>::
from(Path::BaseAbsolute<Traits> const &from) const
{
    std::string const &pstr = _inner.normalized();
    std::string const &ostr = from.normalized();

    size_t pstart;
    (void)Traits::IsAbsolute(pstr, &pstart);
    size_t ostart;
    (void)Traits::IsAbsolute(ostr, &ostart);

    auto po = pstr.begin() + pstart;
    auto oo = ostr.begin() + ostart;

    if (std::string(pstr.begin(), po) != std::string(ostr.begin(), oo)) {
        /* Paths are from different roots. */
        return ext::nullopt;
    }

    std::string result;

    while (true) {
        auto npo = std::find_if(po, pstr.end(), &Traits::IsSeparator);
        auto noo = std::find_if(oo, ostr.end(), &Traits::IsSeparator);

        std::string spo = std::string(po, npo);
        std::string soo = std::string(oo, noo);

        if (spo == soo) {
            po = (npo == pstr.end() ? pstr.end() : std::next(npo));
            oo = (noo == ostr.end() ? ostr.end() : std::next(noo));
        } else {
            break;
        }

        if (npo == pstr.end() || noo == ostr.end()) {
            break;
        }
    }

    while (oo != ostr.end()) {
        auto woo = std::find_if(std::next(oo), ostr.end(), &Traits::IsSeparator);
        result += "..";
        result += Traits::Separator;
        oo = woo;
    }

    if (po != pstr.end()) {
        result += std::string(po, pstr.end());
    }

    return BaseRelative<Traits>(result);
}

namespace libutil { namespace Path { template class BaseAbsolute<Unix>; } }
namespace libutil { namespace Path { template class BaseAbsolute<Windows>; } }
