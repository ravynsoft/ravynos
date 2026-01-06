/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <libutil/Relative.h>
#include <libutil/Absolute.h>
#include <libutil/Unix.h>
#include <libutil/Windows.h>

#include <algorithm>
#include <cassert>

#if _WIN32
#include <cstring>
#else
#include <strings.h>
#endif

namespace Path = libutil::Path;

template<class Traits>
Path::BaseRelative<Traits>::
BaseRelative(std::string const &raw) :
    _raw(raw)
{
}

static std::string
NormalizePath(
    std::string const &path,
    std::string::const_iterator start,
    std::string::const_iterator end,
    bool absolute,
    bool collapse,
    char separator,
    bool(*isSeparator)(char c))
{
    std::string output;

    for (auto it = start; it != end; ++it) {
        if (isSeparator(*it)) {
            /* Skip runs of multiple separators. */
            while (std::next(it) != end && isSeparator(*std::next(it))) {
                ++it;
            }

            /* Only after the beginning, end, or after an existing trailing separator. */
            if (!output.empty() && std::next(it) != end && !isSeparator(output.back())) {
                output += separator;
            }
        } else if (collapse && (it == start || isSeparator(*std::prev(it))) && *it == '.') {
            /* Handle relative paths. */
            if (std::next(it) == end || isSeparator(*std::next(it))) {
                /* Path component is current. */
                if (!output.empty() && isSeparator(output.back())) {
                    output.pop_back();
                }
            } else if (*std::next(it) == '.' && (std::next(it, 2) == end || isSeparator(*std::next(it, 2)))) {
                /* Path component is parent. */
                if (output.empty()) {
                    if (!absolute) {
                        /* Initial parent. Copy to output if relative. */
                        output += '.';
                        output += '.';
                    }
                } else {
                    /* Parent, remove segment. */
                    auto separator = std::find_if(std::next(output.rbegin()), output.rend(), isSeparator);
                    if (separator != output.rend()) {
                        separator = std::next(separator);
                    }
                    output.erase(separator.base(), output.rbegin().base());
                }

                /* Skip second dot. */
                ++it;
            } else {
                /* Path component starting with dot, copy. */
                output += '.';
            }
        } else {
            /* Path entry character, just copy. */
            output += *it;
        }
    }

    return output;
}

template<class Traits>
std::string Path::BaseRelative<Traits>::
normalized() const
{
    std::string output;
    output.reserve(_raw.size());

    size_t start;
    bool absolute = Traits::IsAbsolute(_raw, &start);

    for (std::string::size_type n = 0; n < start; ++n) {
        output += (Traits::IsSeparator(_raw[n]) ? Traits::Separator : _raw[n]);
    }

    output += NormalizePath(_raw, _raw.begin() + start, _raw.end(), absolute, true, Traits::Separator, &Traits::IsSeparator);

    return output;
}

template<class Traits>
ext::optional<Path::BaseAbsolute<Traits>> Path::BaseRelative<Traits>::
absolute() const
{
    if (Traits::IsAbsolute(_raw)) {
        return Path::BaseAbsolute<Traits>(*this);
    } else {
        return ext::nullopt;
    }
}

template<class Traits>
Path::BaseAbsolute<Traits> Path::BaseRelative<Traits>::
resolved(Path::BaseAbsolute<Traits> const &against) const
{
    if (ext::optional<Path::BaseAbsolute<Traits>> absolute = this->absolute()) {
        return *absolute;
    } else {
        std::string base;
        std::string relative;
        if (!Traits::Resolve(_raw, against.raw(), &base, &relative)) {
            abort();
        }

        if (auto absolute = Path::BaseAbsolute<Traits>::Create(base)) {
            return absolute->child(relative);
        } else {
            /* Should always be absolute... */
            abort();
        }
    }
}

template<class Traits>
Path::BaseRelative<Traits> Path::BaseRelative<Traits>::
parent() const
{
    size_t start;
    (void)Traits::IsAbsolute(_raw, &start);

    auto begin = (_raw.size() > start && Path::Windows::IsSeparator(_raw.back()) ? std::next(_raw.rbegin()) : _raw.rbegin());
    auto end = std::prev(_raw.rend(), start);
    auto it = std::find_if(begin, end, &Path::Windows::IsSeparator);

    if (it == end) {
        return Path::BaseRelative<Traits>(std::string(_raw.begin(), end.base()));
    } else {
        return Path::BaseRelative<Traits>(std::string(_raw.begin(), std::prev(it.base())));
    }
}

template<class Traits>
Path::BaseRelative<Traits> Path::BaseRelative<Traits>::
child(std::string const &name) const
{
    if (_raw.empty()) {
        return Path::BaseRelative<Traits>(name);
    } else if (name.empty()) {
        return Path::BaseRelative<Traits>(_raw);
    }

    if (Traits::IsSeparator(_raw.back())) {
        /* Already has a trailing separator, don't duplicate. */
        return Path::BaseRelative<Traits>(_raw + name);
    } else {
        return Path::BaseRelative<Traits>(_raw + Traits::Separator + name);
    }
}

template<class Traits>
std::string Path::BaseRelative<Traits>::
base(bool extension) const
{
    size_t start;
    (void)Traits::IsAbsolute(_raw, &start);

    std::string base;
    auto sit = std::prev(_raw.rend(), start);
    auto it = std::find_if(_raw.rbegin(), sit, &Traits::IsSeparator);

    if (it == sit) {
        /* Remove root even if no separators. */
        base = _raw.substr(start);
    } else {
        /* Remove up to the last separator. */
        base = std::string(it.base(), _raw.end());
    }

    if (!extension) {
        size_t pos = base.rfind('.');
        if (pos != std::string::npos) {
            base = base.substr(0, pos);
        }
    }

    return base;
}

template<class Traits>
std::string Path::BaseRelative<Traits>::
extension() const
{
    std::string base = this->base();
    size_t pos = base.rfind('.');
    if (pos == std::string::npos) {
        return std::string();
    }

    return base.substr(pos + 1);
}

template<class Traits>
bool Path::BaseRelative<Traits>::
extension(std::string const &extension, bool insensitive) const
{
    std::string pathExtension = this->extension();

    if (insensitive) {
#if _WIN32
        return ::_stricmp(pathExtension.c_str(), extension.c_str()) == 0;
#else
        return ::strcasecmp(pathExtension.c_str(), extension.c_str()) == 0;
#endif
    } else {
        return pathExtension == extension;
    }
}

namespace libutil { namespace Path { template class BaseRelative<Unix>; } }
namespace libutil { namespace Path { template class BaseRelative<Windows>; } }
