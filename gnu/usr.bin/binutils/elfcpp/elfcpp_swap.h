// elfcpp_swap.h -- Handle swapping for elfcpp   -*- C++ -*-

// Copyright (C) 2006-2023 Free Software Foundation, Inc.
// Written by Ian Lance Taylor <iant@google.com>.

// This file is part of elfcpp.
   
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// as published by the Free Software Foundation; either version 2, or
// (at your option) any later version.

// In addition to the permissions in the GNU Library General Public
// License, the Free Software Foundation gives you unlimited
// permission to link the compiled version of this file into
// combinations with other programs, and to distribute those
// combinations without any restriction coming from the use of this
// file.  (The Library Public License restrictions do apply in other
// respects; for example, they cover modification of the file, and
/// distribution when not linked into a combined executable.)

// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.

// You should have received a copy of the GNU Library General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
// 02110-1301, USA.

// This header file defines basic template classes to efficiently swap
// numbers between host form and target form.  When the host and
// target have the same endianness, these turn into no-ops.

#ifndef ELFCPP_SWAP_H
#define ELFCPP_SWAP_H

#include <stdint.h>

// We need an autoconf-generated config.h file for endianness and
// swapping.  We check two macros: WORDS_BIGENDIAN and
// HAVE_BYTESWAP_H.

#include "config.h"

#ifdef HAVE_BYTESWAP_H
#include <byteswap.h>
#endif // defined(HAVE_BYTESWAP_H)

// Provide our own versions of the byteswap functions.
#if !HAVE_DECL_BSWAP_16
static inline uint16_t
bswap_16(uint16_t v)
{
  return ((v >> 8) & 0xff) | ((v & 0xff) << 8);
}
#endif // !HAVE_DECL_BSWAP16

#if !HAVE_DECL_BSWAP_32
static inline uint32_t
bswap_32(uint32_t v)
{
  return (  ((v & 0xff000000) >> 24)
	  | ((v & 0x00ff0000) >>  8)
	  | ((v & 0x0000ff00) <<  8)
	  | ((v & 0x000000ff) << 24));
}
#endif // !HAVE_DECL_BSWAP32

#if !HAVE_DECL_BSWAP_64
static inline uint64_t
bswap_64(uint64_t v)
{
  return (  ((v & 0xff00000000000000ULL) >> 56)
	  | ((v & 0x00ff000000000000ULL) >> 40)
	  | ((v & 0x0000ff0000000000ULL) >> 24)
	  | ((v & 0x000000ff00000000ULL) >>  8)
	  | ((v & 0x00000000ff000000ULL) <<  8)
	  | ((v & 0x0000000000ff0000ULL) << 24)
	  | ((v & 0x000000000000ff00ULL) << 40)
	  | ((v & 0x00000000000000ffULL) << 56));
}
#endif // !HAVE_DECL_BSWAP64

// gcc 4.3 and later provides __builtin_bswap32 and __builtin_bswap64.

#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
#undef bswap_32
#define bswap_32 __builtin_bswap32
#undef bswap_64
#define bswap_64 __builtin_bswap64
#endif

namespace elfcpp
{

// Endian simply indicates whether the host is big endian or not.

struct Endian
{
 public:
  // Used for template specializations.
  static const bool host_big_endian = 
#ifdef WORDS_BIGENDIAN
    true
#else
    false
#endif
    ;
};

// Valtype_base is a template based on size (8, 16, 32, 64) which
// defines the type Valtype as the unsigned integer, and
// Signed_valtype as the signed integer, of the specified size.

template<int size>
struct Valtype_base;

template<>
struct Valtype_base<8>
{
  typedef uint8_t Valtype;
  typedef int8_t Signed_valtype;
};

template<>
struct Valtype_base<16>
{
  typedef uint16_t Valtype;
  typedef int16_t Signed_valtype;
};

template<>
struct Valtype_base<32>
{
  typedef uint32_t Valtype;
  typedef int32_t Signed_valtype;
};

template<>
struct Valtype_base<64>
{
  typedef uint64_t Valtype;
  typedef int64_t Signed_valtype;
};

// Convert_endian is a template based on size and on whether the host
// and target have the same endianness.  It defines the type Valtype
// as Valtype_base does, and also defines a function convert_host
// which takes an argument of type Valtype and returns the same value,
// but swapped if the host and target have different endianness.

template<int size, bool same_endian>
struct Convert_endian;

template<int size>
struct Convert_endian<size, true>
{
  typedef typename Valtype_base<size>::Valtype Valtype;

  static inline Valtype
  convert_host(Valtype v)
  { return v; }
};

template<>
struct Convert_endian<8, false>
{
  typedef Valtype_base<8>::Valtype Valtype;

  static inline Valtype
  convert_host(Valtype v)
  { return v; }
};

template<>
struct Convert_endian<16, false>
{
  typedef Valtype_base<16>::Valtype Valtype;

  static inline Valtype
  convert_host(Valtype v)
  { return bswap_16(v); }
};

template<>
struct Convert_endian<32, false>
{
  typedef Valtype_base<32>::Valtype Valtype;

  static inline Valtype
  convert_host(Valtype v)
  { return bswap_32(v); }
};

template<>
struct Convert_endian<64, false>
{
  typedef Valtype_base<64>::Valtype Valtype;

  static inline Valtype
  convert_host(Valtype v)
  { return bswap_64(v); }
};

// Convert is a template based on size and on whether the target is
// big endian.  It defines Valtype and convert_host like
// Convert_endian.  That is, it is just like Convert_endian except in
// the meaning of the second template parameter.

template<int size, bool big_endian>
struct Convert
{
  typedef typename Valtype_base<size>::Valtype Valtype;

  static inline Valtype
  convert_host(Valtype v)
  {
    return Convert_endian<size, big_endian == Endian::host_big_endian>
      ::convert_host(v);
  }
};

// Swap is a template based on size and on whether the target is big
// endian.  It defines the type Valtype and the functions readval and
// writeval.  The functions read and write values of the appropriate
// size out of buffers, swapping them if necessary.  readval and
// writeval are overloaded to take pointers to the appropriate type or
// pointers to unsigned char.

template<int size, bool big_endian>
struct Swap
{
  typedef typename Valtype_base<size>::Valtype Valtype;

  static inline Valtype
  readval(const Valtype* wv)
  { return Convert<size, big_endian>::convert_host(*wv); }

  static inline void
  writeval(Valtype* wv, Valtype v)
  { *wv = Convert<size, big_endian>::convert_host(v); }

  static inline Valtype
  readval(const unsigned char* wv)
  { return readval(reinterpret_cast<const Valtype*>(wv)); }

  static inline void
  writeval(unsigned char* wv, Valtype v)
  { writeval(reinterpret_cast<Valtype*>(wv), v); }
};

// We need to specialize the 8-bit version of Swap to avoid
// conflicting overloads, since both versions of readval and writeval
// will have the same type parameters.

template<bool big_endian>
struct Swap<8, big_endian>
{
  typedef typename Valtype_base<8>::Valtype Valtype;

  static inline Valtype
  readval(const Valtype* wv)
  { return *wv; }

  static inline void
  writeval(Valtype* wv, Valtype v)
  { *wv = v; }
};

// Swap_unaligned is a template based on size and on whether the
// target is big endian.  It defines the type Valtype and the
// functions readval and writeval.  The functions read and write
// values of the appropriate size out of buffers which may be
// misaligned.

template<int size, bool big_endian>
struct Swap_unaligned;

template<bool big_endian>
struct Swap_unaligned<8, big_endian>
{
  typedef typename Valtype_base<8>::Valtype Valtype;

  static inline Valtype
  readval(const unsigned char* wv)
  { return *wv; }

  static inline void
  writeval(unsigned char* wv, Valtype v)
  { *wv = v; }
};

template<>
struct Swap_unaligned<16, false>
{
  typedef Valtype_base<16>::Valtype Valtype;

  static inline Valtype
  readval(const unsigned char* wv)
  {
    return (wv[1] << 8) | wv[0];
  }

  static inline void
  writeval(unsigned char* wv, Valtype v)
  {
    wv[1] = v >> 8;
    wv[0] = v;
  }
};

template<>
struct Swap_unaligned<16, true>
{
  typedef Valtype_base<16>::Valtype Valtype;

  static inline Valtype
  readval(const unsigned char* wv)
  {
    return (wv[0] << 8) | wv[1];
  }

  static inline void
  writeval(unsigned char* wv, Valtype v)
  {
    wv[0] = v >> 8;
    wv[1] = v;
  }
};

template<>
struct Swap_unaligned<32, false>
{
  typedef Valtype_base<32>::Valtype Valtype;

  static inline Valtype
  readval(const unsigned char* wv)
  {
    return (wv[3] << 24) | (wv[2] << 16) | (wv[1] << 8) | wv[0];
  }

  static inline void
  writeval(unsigned char* wv, Valtype v)
  {
    wv[3] = v >> 24;
    wv[2] = v >> 16;
    wv[1] = v >> 8;
    wv[0] = v;
  }
};

template<>
struct Swap_unaligned<32, true>
{
  typedef Valtype_base<32>::Valtype Valtype;

  static inline Valtype
  readval(const unsigned char* wv)
  {
    return (wv[0] << 24) | (wv[1] << 16) | (wv[2] << 8) | wv[3];
  }

  static inline void
  writeval(unsigned char* wv, Valtype v)
  {
    wv[0] = v >> 24;
    wv[1] = v >> 16;
    wv[2] = v >> 8;
    wv[3] = v;
  }
};

template<>
struct Swap_unaligned<64, false>
{
  typedef Valtype_base<64>::Valtype Valtype;

  static inline Valtype
  readval(const unsigned char* wv)
  {
    return ((static_cast<Valtype>(wv[7]) << 56)
	    | (static_cast<Valtype>(wv[6]) << 48)
	    | (static_cast<Valtype>(wv[5]) << 40)
	    | (static_cast<Valtype>(wv[4]) << 32)
	    | (static_cast<Valtype>(wv[3]) << 24)
	    | (static_cast<Valtype>(wv[2]) << 16)
	    | (static_cast<Valtype>(wv[1]) << 8)
	    | static_cast<Valtype>(wv[0]));
  }

  static inline void
  writeval(unsigned char* wv, Valtype v)
  {
    wv[7] = v >> 56;
    wv[6] = v >> 48;
    wv[5] = v >> 40;
    wv[4] = v >> 32;
    wv[3] = v >> 24;
    wv[2] = v >> 16;
    wv[1] = v >> 8;
    wv[0] = v;
  }
};

template<>
struct Swap_unaligned<64, true>
{
  typedef Valtype_base<64>::Valtype Valtype;

  static inline Valtype
  readval(const unsigned char* wv)
  {
    return ((static_cast<Valtype>(wv[0]) << 56)
	    | (static_cast<Valtype>(wv[1]) << 48)
	    | (static_cast<Valtype>(wv[2]) << 40)
	    | (static_cast<Valtype>(wv[3]) << 32)
	    | (static_cast<Valtype>(wv[4]) << 24)
	    | (static_cast<Valtype>(wv[5]) << 16)
	    | (static_cast<Valtype>(wv[6]) << 8)
	    | static_cast<Valtype>(wv[7]));
  }

  static inline void
  writeval(unsigned char* wv, Valtype v)
  {
    wv[0] = v >> 56;
    wv[1] = v >> 48;
    wv[2] = v >> 40;
    wv[3] = v >> 32;
    wv[4] = v >> 24;
    wv[5] = v >> 16;
    wv[6] = v >> 8;
    wv[7] = v;
  }
};

// Swap_aligned32 is a template based on size and on whether the
// target is big endian.  It defines the type Valtype and the
// functions readval and writeval.  The functions read and write
// values of the appropriate size out of buffers which may not be
// 64-bit aligned, but are 32-bit aligned.

template<int size, bool big_endian>
struct Swap_aligned32
{
  typedef typename Valtype_base<size>::Valtype Valtype;

  static inline Valtype
  readval(const unsigned char* wv)
  { return Swap<size, big_endian>::readval(
	reinterpret_cast<const Valtype*>(wv)); }

  static inline void
  writeval(unsigned char* wv, Valtype v)
  { Swap<size, big_endian>::writeval(reinterpret_cast<Valtype*>(wv), v); }
};

template<>
struct Swap_aligned32<64, true>
{
  typedef Valtype_base<64>::Valtype Valtype;

  static inline Valtype
  readval(const unsigned char* wv)
  {
    return ((static_cast<Valtype>(Swap<32, true>::readval(wv)) << 32)
	    | static_cast<Valtype>(Swap<32, true>::readval(wv + 4)));
  }

  static inline void
  writeval(unsigned char* wv, Valtype v)
  {
    typedef Valtype_base<32>::Valtype Valtype32;

    Swap<32, true>::writeval(wv, static_cast<Valtype32>(v >> 32));
    Swap<32, true>::writeval(wv + 4, static_cast<Valtype32>(v));
  }
};

template<>
struct Swap_aligned32<64, false>
{
  typedef Valtype_base<64>::Valtype Valtype;

  static inline Valtype
  readval(const unsigned char* wv)
  {
    return ((static_cast<Valtype>(Swap<32, false>::readval(wv + 4)) << 32)
	    | static_cast<Valtype>(Swap<32, false>::readval(wv)));
  }

  static inline void
  writeval(unsigned char* wv, Valtype v)
  {
    typedef Valtype_base<32>::Valtype Valtype32;

    Swap<32, false>::writeval(wv + 4, static_cast<Valtype32>(v >> 32));
    Swap<32, false>::writeval(wv, static_cast<Valtype32>(v));
  }
};

} // End namespace elfcpp.

#endif // !defined(ELFCPP_SWAP_H)
