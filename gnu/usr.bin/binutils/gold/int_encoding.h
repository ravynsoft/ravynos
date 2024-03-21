// int_encoding.h -- variable length and unaligned integers -*- C++ -*-

// Copyright (C) 2009-2023 Free Software Foundation, Inc.
// Written by Doug Kwan <dougkwan@google.com> by refactoring scattered
// contents from other files in gold.  Original code written by Ian
// Lance Taylor <iant@google.com> and Caleb Howe  <cshowe@google.com>.

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

#ifndef GOLD_INT_ENCODING_H
#define GOLD_INT_ENCODING_H

#include <vector>
#include "elfcpp.h"
#include "target.h"
#include "parameters.h"

namespace gold
{

//
// LEB 128 encoding support.
//

// Read a ULEB 128 encoded integer from BUFFER.  Return the length of the
// encoded integer at the location PLEN.  The common case of a single-byte
// value is handled inline, and multi-byte values are processed by the _x
// routine, where BYTE is the first byte of the value.

uint64_t
read_unsigned_LEB_128_x(const unsigned char* buffer, size_t* plen,
			unsigned char byte);

inline uint64_t
read_unsigned_LEB_128(const unsigned char* buffer, size_t* plen)
{
  unsigned char byte = *buffer++;

  if ((byte & 0x80) != 0)
    return read_unsigned_LEB_128_x(buffer, plen, byte);

  *plen = 1;
  return static_cast<uint64_t>(byte);
}

// Read an SLEB 128 encoded integer from BUFFER.  Return the length of the
// encoded integer at the location PLEN.  The common case of a single-byte
// value is handled inline, and multi-byte values are processed by the _x
// routine, where BYTE is the first byte of the value.

int64_t
read_signed_LEB_128_x(const unsigned char* buffer, size_t* plen,
		      unsigned char byte);

inline int64_t
read_signed_LEB_128(const unsigned char* buffer, size_t* plen)
{
  unsigned char byte = *buffer++;

  if ((byte & 0x80) != 0)
    return read_signed_LEB_128_x(buffer, plen, byte);

  *plen = 1;
  if (byte & 0x40)
    return -(static_cast<int64_t>(1) << 7) | static_cast<int64_t>(byte);
  return static_cast<int64_t>(byte);
}

// Write a ULEB 128 encoded VALUE to BUFFER.

void
write_unsigned_LEB_128(std::vector<unsigned char>* buffer, uint64_t value);

// Return the ULEB 128 encoded size of VALUE.

size_t
get_length_as_unsigned_LEB_128(uint64_t value);

//
// Unaligned integer encoding support.
//

// Insert VALSIZE-bit integer VALUE into DESTINATION.

template <int valsize>
void insert_into_vector(std::vector<unsigned char>* destination,
                        typename elfcpp::Valtype_base<valsize>::Valtype value)
{
  unsigned char buffer[valsize / 8];
  if (parameters->target().is_big_endian())
    elfcpp::Swap_unaligned<valsize, true>::writeval(buffer, value);
  else
    elfcpp::Swap_unaligned<valsize, false>::writeval(buffer, value);
  destination->insert(destination->end(), buffer, buffer + valsize / 8);
}

// Read a possibly unaligned integer of SIZE from SOURCE.

template <int valsize>
typename elfcpp::Valtype_base<valsize>::Valtype
read_from_pointer(const unsigned char* source)
{
  typename elfcpp::Valtype_base<valsize>::Valtype return_value;
  if (parameters->target().is_big_endian())
    return_value = elfcpp::Swap_unaligned<valsize, true>::readval(source);
  else
    return_value = elfcpp::Swap_unaligned<valsize, false>::readval(source);
  return return_value;
}

// Read a possibly unaligned integer of SIZE.  Update SOURCE after read.

template <int valsize>
typename elfcpp::Valtype_base<valsize>::Valtype
read_from_pointer(unsigned char** source)
{
  typename elfcpp::Valtype_base<valsize>::Valtype return_value;
  if (parameters->target().is_big_endian())
    return_value = elfcpp::Swap_unaligned<valsize, true>::readval(*source);
  else
    return_value = elfcpp::Swap_unaligned<valsize, false>::readval(*source);
  *source += valsize / 8;
  return return_value;
}

// Same as the above except for use with const unsigned char data.

template <int valsize>
typename elfcpp::Valtype_base<valsize>::Valtype
read_from_pointer(const unsigned char** source)
{
  typename elfcpp::Valtype_base<valsize>::Valtype return_value;
  if (parameters->target().is_big_endian())
    return_value = elfcpp::Swap_unaligned<valsize, true>::readval(*source);
  else
    return_value = elfcpp::Swap_unaligned<valsize, false>::readval(*source);
  *source += valsize / 8;
  return return_value;
}

} // End namespace gold.

#endif // !defined(GOLD_INT_ENCODING_H)
