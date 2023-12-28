// system.h -- general definitions for gold   -*- C++ -*-

// Copyright (C) 2006-2023 Free Software Foundation, Inc.
// Written by Ian Lance Taylor <iant@google.com>.

// This file is part of gold.

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
// MA 02110-1301, USA.

#ifndef SYSTEM_H
#define SYSTEM_H 1

#ifndef ENABLE_NLS
  // The Solaris version of locale.h always includes libintl.h.  If we
  // have been configured with --disable-nls then ENABLE_NLS will not
  // be defined and the dummy definitions of bindtextdomain (et al)
  // below will conflict with the definitions in libintl.h.  So we
  // define these values to prevent the bogus inclusion of libintl.h.
# define _LIBINTL_H
# define _LIBGETTEXT_H
#endif

#ifdef ENABLE_NLS
// On some systems, things go awry when <libintl.h> comes after <clocale>.
# include <libintl.h>
# include <clocale>
# define _(String) gettext (String)
# ifdef gettext_noop
#  define N_(String) gettext_noop (String)
# else
#  define N_(String) (String)
# endif
#else
// Include <clocale> first to avoid conflicts with these macros.
# include <clocale>
# define gettext(Msgid) (Msgid)
# define dgettext(Domainname, Msgid) (Msgid)
# define dcgettext(Domainname, Msgid, Category) (Msgid)
# define ngettext(Msgid1, Msgid2, n) \
  (n == 1 ? Msgid1 : Msgid2)
# define dngettext(Domainname, Msgid1, Msgid2, n) \
  (n == 1 ? Msgid1 : Msgid2)
# define dcngettext(Domainname, Msgid1, Msgid2, n, Category) \
  (n == 1 ? Msgid1 : Msgid2)
# define textdomain(Domainname) do {} while (0)
# define bindtextdomain(Domainname, Dirname) do {} while (0)
# define _(String) (String)
# define N_(String) (String)
#endif

// Figure out how to get a hash set and a hash map.

#if defined(HAVE_UNORDERED_SET) && defined(HAVE_UNORDERED_MAP)

#include <unordered_set>
#include <unordered_map>

// We need a template typedef here.

#define Unordered_set std::unordered_set
#define Unordered_map std::unordered_map
#define Unordered_multimap std::unordered_multimap

#define reserve_unordered_map(map, n) ((map)->rehash(n))

#elif defined(HAVE_TR1_UNORDERED_SET) && defined(HAVE_TR1_UNORDERED_MAP) \
      && defined(HAVE_TR1_UNORDERED_MAP_REHASH)

#include <tr1/unordered_set>
#include <tr1/unordered_map>

// We need a template typedef here.

#define Unordered_set std::tr1::unordered_set
#define Unordered_map std::tr1::unordered_map
#define Unordered_multimap std::tr1::unordered_multimap

#define reserve_unordered_map(map, n) ((map)->rehash(n))

#ifndef HAVE_TR1_HASH_OFF_T
// The library does not support hashes of off_t values.  Add support
// here.  This is likely to be specific to libstdc++.  This issue
// arises with GCC 4.1.x when compiling in 32-bit mode with a 64-bit
// off_t type.
namespace std { namespace tr1 {
template<>
struct hash<off_t> : public std::unary_function<off_t, std::size_t>
{
  std::size_t
  operator()(off_t val) const
  { return static_cast<std::size_t>(val); }
};
} } // Close namespaces.
#endif // !defined(HAVE_TR1_HASH_OFF_T)

#elif defined(HAVE_EXT_HASH_MAP) && defined(HAVE_EXT_HASH_SET)

#include <ext/hash_map>
#include <ext/hash_set>
#include <string>

#define Unordered_set __gnu_cxx::hash_set
#define Unordered_map __gnu_cxx::hash_map
#define Unordered_multimap __gnu_cxx::hash_multimap

namespace __gnu_cxx
{

template<>
struct hash<std::string>
{
  size_t
  operator()(std::string s) const
  { return __stl_hash_string(s.c_str()); }
};

template<typename T>
struct hash<T*>
{
  size_t
  operator()(T* p) const
  { return reinterpret_cast<size_t>(p); }
};

}

#define reserve_unordered_map(map, n) ((map)->resize(n))

#else

// The fallback is to just use set and map.

#include <set>
#include <map>

#define Unordered_set std::set
#define Unordered_map std::map
#define Unordered_multimap std::multimap

#define reserve_unordered_map(map, n)

#endif

#ifndef HAVE_PREAD
extern "C" ssize_t pread(int, void*, size_t, off_t);
#endif

#ifndef HAVE_FTRUNCATE
extern "C" int ftruncate(int, off_t);
#endif

#ifndef HAVE_FFSLL
extern "C" int ffsll(long long);
#endif

#if !HAVE_DECL_MEMMEM
extern "C" void *memmem(const void *, size_t, const void *, size_t);
#endif

#if !HAVE_DECL_STRNDUP
extern "C" char *strndup(const char *, size_t);
#endif

#endif // !defined(SYSTEM_H)
