// stringpool.cc -- a string pool for gold

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

#include "gold.h"

#include <cstring>
#include <algorithm>
#include <vector>

#include "output.h"
#include "parameters.h"
#include "stringpool.h"

namespace gold
{

template<typename Stringpool_char>
Stringpool_template<Stringpool_char>::Stringpool_template(uint64_t addralign)
  : string_set_(), key_to_offset_(), strings_(), strtab_size_(0),
    zero_null_(true), optimize_(false), offset_(sizeof(Stringpool_char)),
    addralign_(addralign)
{
  if (parameters->options_valid()
      && parameters->options().optimize() >= 2
      && addralign <= sizeof(Stringpool_char))
    this->optimize_ = true;
}

template<typename Stringpool_char>
void
Stringpool_template<Stringpool_char>::clear()
{
  for (typename std::list<Stringdata*>::iterator p = this->strings_.begin();
       p != this->strings_.end();
       ++p)
    delete[] reinterpret_cast<char*>(*p);
  this->strings_.clear();
  this->key_to_offset_.clear();
  this->string_set_.clear();
}

template<typename Stringpool_char>
Stringpool_template<Stringpool_char>::~Stringpool_template()
{
  this->clear();
}

// Resize the internal hashtable with the expectation we'll get n new
// elements.  Note that the hashtable constructor takes a "number of
// buckets you'd like," rather than "number of elements you'd like,"
// but that's the best we can do.

template<typename Stringpool_char>
void
Stringpool_template<Stringpool_char>::reserve(unsigned int n)
{
  this->key_to_offset_.reserve(n);

#if defined(HAVE_UNORDERED_MAP)
  this->string_set_.rehash(this->string_set_.size() + n);
  return;
#elif defined(HAVE_TR1_UNORDERED_MAP)
  // rehash() implementation is broken in gcc 4.0.3's stl
  //this->string_set_.rehash(this->string_set_.size() + n);
  //return;
#elif defined(HAVE_EXT_HASH_MAP)
  this->string_set_.resize(this->string_set_.size() + n);
  return;
#endif

  // This is the generic "reserve" code, if no #ifdef above triggers.
  String_set_type new_string_set(this->string_set_.size() + n);
  new_string_set.insert(this->string_set_.begin(), this->string_set_.end());
  this->string_set_.swap(new_string_set);
}

// Compare two strings of arbitrary character type for equality.

template<typename Stringpool_char>
bool
Stringpool_template<Stringpool_char>::string_equal(const Stringpool_char* s1,
						   const Stringpool_char* s2)
{
  while (*s1 != 0)
    if (*s1++ != *s2++)
      return false;
  return *s2 == 0;
}

// Specialize string_equal for char.

template<>
inline bool
Stringpool_template<char>::string_equal(const char* s1, const char* s2)
{
  return strcmp(s1, s2) == 0;
}

// Equality comparison function for the hash table.

template<typename Stringpool_char>
inline bool
Stringpool_template<Stringpool_char>::Stringpool_eq::operator()(
    const Hashkey& h1,
    const Hashkey& h2) const
{
  return (h1.hash_code == h2.hash_code
	  && h1.length == h2.length
	  && (h1.string == h2.string
	      || memcmp(h1.string, h2.string,
			h1.length * sizeof(Stringpool_char)) == 0));
}

// Hash function.  The length is in characters, not bytes.

template<typename Stringpool_char>
size_t
Stringpool_template<Stringpool_char>::string_hash(const Stringpool_char* s,
						  size_t length)
{
  return gold::string_hash<Stringpool_char>(s, length);
}

// Add the string S to the list of canonical strings.  Return a
// pointer to the canonical string.  If PKEY is not NULL, set *PKEY to
// the key.  LENGTH is the length of S in characters.  Note that S may
// not be NUL terminated.

template<typename Stringpool_char>
const Stringpool_char*
Stringpool_template<Stringpool_char>::add_string(const Stringpool_char* s,
						 size_t len)
{
  // We are in trouble if we've already computed the string offsets.
  gold_assert(this->strtab_size_ == 0);

  // The size we allocate for a new Stringdata.
  const size_t buffer_size = 1000;
  // The amount we multiply the Stringdata index when calculating the
  // key.
  const size_t key_mult = 1024;
  gold_assert(key_mult >= buffer_size);

  // Convert len to the number of bytes we need to allocate, including
  // the null character.
  len = (len + 1) * sizeof(Stringpool_char);

  size_t alc;
  bool front = true;
  if (len > buffer_size)
    {
      alc = sizeof(Stringdata) + len;
      front = false;
    }
  else if (this->strings_.empty())
    alc = sizeof(Stringdata) + buffer_size;
  else
    {
      Stringdata* psd = this->strings_.front();
      if (len > psd->alc - psd->len)
	alc = sizeof(Stringdata) + buffer_size;
      else
	{
	  char* ret = psd->data + psd->len;
	  memcpy(ret, s, len - sizeof(Stringpool_char));
	  memset(ret + len - sizeof(Stringpool_char), 0,
		 sizeof(Stringpool_char));

	  psd->len += len;

	  return reinterpret_cast<const Stringpool_char*>(ret);
	}
    }

  Stringdata* psd = reinterpret_cast<Stringdata*>(new char[alc]);
  psd->alc = alc - sizeof(Stringdata);
  memcpy(psd->data, s, len - sizeof(Stringpool_char));
  memset(psd->data + len - sizeof(Stringpool_char), 0,
	 sizeof(Stringpool_char));
  psd->len = len;

  if (front)
    this->strings_.push_front(psd);
  else
    this->strings_.push_back(psd);

  return reinterpret_cast<const Stringpool_char*>(psd->data);
}

// Add a string to a string pool.

template<typename Stringpool_char>
const Stringpool_char*
Stringpool_template<Stringpool_char>::add(const Stringpool_char* s, bool copy,
                                          Key* pkey)
{
  return this->add_with_length(s, string_length(s), copy, pkey);
}

// Add a new key offset entry.

template<typename Stringpool_char>
void
Stringpool_template<Stringpool_char>::new_key_offset(size_t length)
{
  section_offset_type offset;
  if (this->zero_null_ && length == 0)
    offset = 0;
  else
    {
      offset = this->offset_;
      // Align strings.
      offset = align_address(offset, this->addralign_);
      this->offset_ = offset + (length + 1) * sizeof(Stringpool_char);
    }
  this->key_to_offset_.push_back(offset);
}

template<typename Stringpool_char>
const Stringpool_char*
Stringpool_template<Stringpool_char>::add_with_length(const Stringpool_char* s,
						      size_t length,
						      bool copy,
						      Key* pkey)
{
  typedef std::pair<typename String_set_type::iterator, bool> Insert_type;

  // We add 1 so that 0 is always invalid.
  const Key k = this->key_to_offset_.size() + 1;

  if (!copy)
    {
      // When we don't need to copy the string, we can call insert
      // directly.

      std::pair<Hashkey, Hashval> element(Hashkey(s, length), k);

      Insert_type ins = this->string_set_.insert(element);

      typename String_set_type::const_iterator p = ins.first;

      if (ins.second)
	{
	  // We just added the string.  The key value has now been
	  // used.
	  this->new_key_offset(length);
	}
      else
	{
	  gold_assert(k != p->second);
	}

      if (pkey != NULL)
	*pkey = p->second;
      return p->first.string;
    }

  // When we have to copy the string, we look it up twice in the hash
  // table.  The problem is that we can't insert S before we
  // canonicalize it by copying it into the canonical list. The hash
  // code will only be computed once.

  Hashkey hk(s, length);
  typename String_set_type::const_iterator p = this->string_set_.find(hk);
  if (p != this->string_set_.end())
    {
      if (pkey != NULL)
	*pkey = p->second;
      return p->first.string;
    }

  this->new_key_offset(length);

  hk.string = this->add_string(s, length);
  // The contents of the string stay the same, so we don't need to
  // adjust hk.hash_code or hk.length.

  std::pair<Hashkey, Hashval> element(hk, k);

  Insert_type ins = this->string_set_.insert(element);
  gold_assert(ins.second);

  if (pkey != NULL)
    *pkey = k;
  return hk.string;
}

template<typename Stringpool_char>
const Stringpool_char*
Stringpool_template<Stringpool_char>::find(const Stringpool_char* s,
					   Key* pkey) const
{
  Hashkey hk(s);
  typename String_set_type::const_iterator p = this->string_set_.find(hk);
  if (p == this->string_set_.end())
    return NULL;

  if (pkey != NULL)
    *pkey = p->second;

  return p->first.string;
}

// Comparison routine used when sorting into an ELF strtab.  We want
// to sort this so that when one string is a suffix of another, we
// always see the shorter string immediately after the longer string.
// For example, we want to see these strings in this order:
//   abcd
//   cd
//   d
// When strings are not suffixes, we don't care what order they are
// in, but we need to ensure that suffixes wind up next to each other.
// So we do a reversed lexicographic sort on the reversed string.

template<typename Stringpool_char>
bool
Stringpool_template<Stringpool_char>::Stringpool_sort_comparison::operator()(
  const Stringpool_sort_info& sort_info1,
  const Stringpool_sort_info& sort_info2) const
{
  const Hashkey& h1(sort_info1->first);
  const Hashkey& h2(sort_info2->first);
  const Stringpool_char* s1 = h1.string;
  const Stringpool_char* s2 = h2.string;
  const size_t len1 = h1.length;
  const size_t len2 = h2.length;
  const size_t minlen = len1 < len2 ? len1 : len2;
  const Stringpool_char* p1 = s1 + len1 - 1;
  const Stringpool_char* p2 = s2 + len2 - 1;
  for (size_t i = minlen; i > 0; --i, --p1, --p2)
    {
      if (*p1 != *p2)
	return *p1 > *p2;
    }
  return len1 > len2;
}

// Return whether s1 is a suffix of s2.

template<typename Stringpool_char>
bool
Stringpool_template<Stringpool_char>::is_suffix(const Stringpool_char* s1,
                                                size_t len1,
						const Stringpool_char* s2,
                                                size_t len2)
{
  if (len1 > len2)
    return false;
  return memcmp(s1, s2 + len2 - len1, len1 * sizeof(Stringpool_char)) == 0;
}

// Turn the stringpool into an ELF strtab: determine the offsets of
// each string in the table.

template<typename Stringpool_char>
void
Stringpool_template<Stringpool_char>::set_string_offsets()
{
  if (this->strtab_size_ != 0)
    {
      // We've already computed the offsets.
      return;
    }

  const size_t charsize = sizeof(Stringpool_char);

  // Offset 0 may be reserved for the empty string.
  section_offset_type offset = this->zero_null_ ? charsize : 0;

  // Sorting to find suffixes can take over 25% of the total CPU time
  // used by the linker.  Since it's merely an optimization to reduce
  // the strtab size, and gives a relatively small benefit (it's
  // typically rare for a symbol to be a suffix of another), we only
  // take the time to sort when the user asks for heavy optimization.
  if (!this->optimize_)
    {
      // If we are not optimizing, the offsets are already assigned.
      offset = this->offset_;
    }
  else
    {
      size_t count = this->string_set_.size();

      std::vector<Stringpool_sort_info> v;
      v.reserve(count);

      for (typename String_set_type::iterator p = this->string_set_.begin();
           p != this->string_set_.end();
           ++p)
        v.push_back(Stringpool_sort_info(p));

      std::sort(v.begin(), v.end(), Stringpool_sort_comparison());

      section_offset_type last_offset = -1;
      for (typename std::vector<Stringpool_sort_info>::iterator last = v.end(),
             curr = v.begin();
           curr != v.end();
           last = curr++)
        {
	  section_offset_type this_offset;
          if (this->zero_null_ && (*curr)->first.string[0] == 0)
            this_offset = 0;
          else if (last != v.end()
                   && ((((*curr)->first.length - (*last)->first.length)
			% this->addralign_) == 0)
                   && is_suffix((*curr)->first.string,
				(*curr)->first.length,
                                (*last)->first.string,
				(*last)->first.length))
            this_offset = (last_offset
			   + (((*last)->first.length - (*curr)->first.length)
			      * charsize));
          else
            {
              this_offset = align_address(offset, this->addralign_);
              offset = this_offset + ((*curr)->first.length + 1) * charsize;
            }
	  this->key_to_offset_[(*curr)->second - 1] = this_offset;
	  last_offset = this_offset;
        }
    }

  this->strtab_size_ = offset;
}

// Get the offset of a string in the ELF strtab.  The string must
// exist.

template<typename Stringpool_char>
section_offset_type
Stringpool_template<Stringpool_char>::get_offset(const Stringpool_char* s)
  const
{
  return this->get_offset_with_length(s, string_length(s));
}

template<typename Stringpool_char>
section_offset_type
Stringpool_template<Stringpool_char>::get_offset_with_length(
    const Stringpool_char* s,
    size_t length) const
{
  gold_assert(this->strtab_size_ != 0);
  Hashkey hk(s, length);
  typename String_set_type::const_iterator p = this->string_set_.find(hk);
  if (p != this->string_set_.end())
    return this->key_to_offset_[p->second - 1];
  gold_unreachable();
}

// Write the ELF strtab into the buffer.

template<typename Stringpool_char>
void
Stringpool_template<Stringpool_char>::write_to_buffer(
    unsigned char* buffer,
    section_size_type bufsize)
{
  gold_assert(this->strtab_size_ != 0);
  gold_assert(bufsize >= this->strtab_size_);
  if (this->zero_null_)
    buffer[0] = '\0';
  for (typename String_set_type::const_iterator p = this->string_set_.begin();
       p != this->string_set_.end();
       ++p)
    {
      const int len = (p->first.length + 1) * sizeof(Stringpool_char);
      const section_offset_type offset = this->key_to_offset_[p->second - 1];
      gold_assert(static_cast<section_size_type>(offset) + len
		  <= this->strtab_size_);
      memcpy(buffer + offset, p->first.string, len);
    }
}

// Write the ELF strtab into the output file at the specified offset.

template<typename Stringpool_char>
void
Stringpool_template<Stringpool_char>::write(Output_file* of, off_t offset)
{
  gold_assert(this->strtab_size_ != 0);
  unsigned char* view = of->get_output_view(offset, this->strtab_size_);
  this->write_to_buffer(view, this->strtab_size_);
  of->write_output_view(offset, this->strtab_size_, view);
}

// Print statistical information to stderr.  This is used for --stats.

template<typename Stringpool_char>
void
Stringpool_template<Stringpool_char>::print_stats(const char* name) const
{
#if defined(HAVE_UNORDERED_MAP) || defined(HAVE_TR1_UNORDERED_MAP) || defined(HAVE_EXT_HASH_MAP)
  fprintf(stderr, _("%s: %s entries: %zu; buckets: %zu\n"),
	  program_name, name, this->string_set_.size(),
	  this->string_set_.bucket_count());
#else
  fprintf(stderr, _("%s: %s entries: %zu\n"),
	  program_name, name, this->table_.size());
#endif
  fprintf(stderr, _("%s: %s Stringdata structures: %zu\n"),
	  program_name, name, this->strings_.size());
}

// Instantiate the templates we need.

template
class Stringpool_template<char>;

template
class Stringpool_template<uint16_t>;

template
class Stringpool_template<uint32_t>;

} // End namespace gold.
