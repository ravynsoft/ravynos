// stringpool.h -- a string pool for gold    -*- C++ -*-

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

#include <string>
#include <list>
#include <vector>

#ifndef GOLD_STRINGPOOL_H
#define GOLD_STRINGPOOL_H

namespace gold
{

class Output_file;

// Return the length of a string in units of Char_type.

template<typename Char_type>
inline size_t
string_length(const Char_type* p)
{
  size_t len = 0;
  for (; *p != 0; ++p)
    ++len;
  return len;
}

// Specialize string_length for char.  Maybe we could just use
// std::char_traits<>::length?

template<>
inline size_t
string_length(const char* p)
{
  return strlen(p);
}

// A Stringpool is a pool of unique strings.  It provides the
// following features:

// Every string in the pool is unique.  Thus, if you have two strings
// in the Stringpool, you can compare them for equality by using
// pointer comparison rather than string comparison.

// There is a key associated with every string in the pool.  If you
// add strings to the Stringpool in the same order, then the key for
// each string will always be the same for any run of the linker.
// This is not true of the string pointers themselves, as they may
// change due to address space randomization.  Some parts of the
// linker (e.g., the symbol table) use the key value instead of the
// string pointer so that repeated runs of the linker will generate
// precisely the same output.

// When you add a string to a Stringpool, Stringpool will optionally
// make a copy of it.  Thus there is no requirement to keep a copy
// elsewhere.

// A Stringpool can be turned into a string table, a sequential series
// of null terminated strings.  The first string may optionally be a
// single zero byte, as required for SHT_STRTAB sections.  This
// conversion is only permitted after all strings have been added to
// the Stringpool.  After doing this conversion, you can ask for the
// offset of any string (or any key) in the stringpool in the string
// table, and you can write the resulting string table to an output
// file.

// When a Stringpool is turned into a string table, then as an
// optimization it will reuse string suffixes to avoid duplicating
// strings.  That is, given the strings "abc" and "bc", only the
// string "abc" will be stored, and "bc" will be represented by an
// offset into the middle of the string "abc".


// A simple chunked vector class--this is a subset of std::vector
// which stores memory in chunks.  We don't provide iterators, because
// we don't need them.

template<typename Element>
class Chunked_vector
{
 public:
  Chunked_vector()
    : chunks_(), size_(0)
  { }

  // Clear the elements.
  void
  clear()
  {
    this->chunks_.clear();
    this->size_ = 0;
  }

  // Reserve elements.
  void
  reserve(unsigned int n)
  {
    if (n > this->chunks_.size() * chunk_size)
      {
	this->chunks_.resize((n + chunk_size - 1) / chunk_size);
	// We need to call reserve() of all chunks since changing
	// this->chunks_ causes Element_vectors to be copied.  The
	// reserved capacity of an Element_vector may be lost in copying.
	for (size_t i = 0; i < this->chunks_.size(); ++i)
	  this->chunks_[i].reserve(chunk_size);
      }
  }

  // Get the number of elements.
  size_t
  size() const
  { return this->size_; }

  // Push a new element on the back of the vector.
  void
  push_back(const Element& element)
  {
    size_t chunk_index = this->size_ / chunk_size;
    if (chunk_index >= this->chunks_.size())
      {
	this->chunks_.push_back(Element_vector());
	this->chunks_.back().reserve(chunk_size);
	gold_assert(chunk_index < this->chunks_.size());
      }
    this->chunks_[chunk_index].push_back(element);
    this->size_++;
  }

  // Return a reference to an entry in the vector.
  Element&
  operator[](size_t i)
  { return this->chunks_[i / chunk_size][i % chunk_size]; }

  const Element&
  operator[](size_t i) const
  { return this->chunks_[i / chunk_size][i % chunk_size]; }

 private:
  static const unsigned int chunk_size = 8192;

  typedef std::vector<Element> Element_vector;
  typedef std::vector<Element_vector> Chunk_vector;

  Chunk_vector chunks_;
  size_t size_;
};


// Stringpools are implemented in terms of Stringpool_template, which
// is generalized on the type of character used for the strings.  Most
// uses will want the Stringpool type which uses char.  Other cases
// are used for merging wide string constants.

template<typename Stringpool_char>
class Stringpool_template
{
 public:
  // The type of a key into the stringpool.  As described above, a key
  // value will always be the same during any run of the linker.  Zero
  // is never a valid key value.
  typedef size_t Key;

  // Create a Stringpool.
  Stringpool_template(uint64_t addralign = 1);

  ~Stringpool_template();

  // Clear all the data from the stringpool.
  void
  clear();

  // Hint to the stringpool class that you intend to insert n additional
  // elements.  The stringpool class can use this info however it likes;
  // in practice it will resize its internal hashtables to make room.
  void
  reserve(unsigned int n);

  // Indicate that we should not reserve offset 0 to hold the empty
  // string when converting the stringpool to a string table.  This
  // should not be called for a proper ELF SHT_STRTAB section.
  void
  set_no_zero_null()
  {
    gold_assert(this->string_set_.empty()
		&& this->offset_ == sizeof(Stringpool_char));
    this->zero_null_ = false;
    this->offset_ = 0;
  }

  // Indicate that this string pool should be optimized, even if not
  // running with -O2.
  void
  set_optimize()
  { this->optimize_ = true; }

  // Add the string S to the pool.  This returns a canonical permanent
  // pointer to the string in the pool.  If COPY is true, the string
  // is copied into permanent storage.  If PKEY is not NULL, this sets
  // *PKEY to the key for the string.
  const Stringpool_char*
  add(const Stringpool_char* s, bool copy, Key* pkey);

  // Add the string S to the pool.
  const Stringpool_char*
  add(const std::basic_string<Stringpool_char>& s, bool copy, Key* pkey)
  { return this->add_with_length(s.data(), s.size(), copy, pkey); }

  // Add string S of length LEN characters to the pool.  If COPY is
  // true, S need not be null terminated.
  const Stringpool_char*
  add_with_length(const Stringpool_char* s, size_t len, bool copy, Key* pkey);

  // If the string S is present in the pool, return the canonical
  // string pointer.  Otherwise, return NULL.  If PKEY is not NULL,
  // set *PKEY to the key.
  const Stringpool_char*
  find(const Stringpool_char* s, Key* pkey) const;

  // Turn the stringpool into a string table: determine the offsets of
  // all the strings.  After this is called, no more strings may be
  // added to the stringpool.
  void
  set_string_offsets();

  // Get the offset of the string S in the string table.  This returns
  // the offset in bytes, not in units of Stringpool_char.  This may
  // only be called after set_string_offsets has been called.
  section_offset_type
  get_offset(const Stringpool_char* s) const;

  // Get the offset of the string S in the string table.
  section_offset_type
  get_offset(const std::basic_string<Stringpool_char>& s) const
  { return this->get_offset_with_length(s.c_str(), s.size()); }

  // Get the offset of string S, with length LENGTH characters, in the
  // string table.
  section_offset_type
  get_offset_with_length(const Stringpool_char* s, size_t length) const;

  // Get the offset of the string with key K.
  section_offset_type
  get_offset_from_key(Key k) const
  {
    gold_assert(k <= this->key_to_offset_.size());
    return this->key_to_offset_[k - 1];
  }

  // Get the size of the string table.  This returns the number of
  // bytes, not in units of Stringpool_char.
  section_size_type
  get_strtab_size() const
  {
    gold_assert(this->strtab_size_ != 0);
    return this->strtab_size_;
  }

  // Write the string table into the output file at the specified
  // offset.
  void
  write(Output_file*, off_t offset);

  // Write the string table into the specified buffer, of the
  // specified size.  buffer_size should be at least
  // get_strtab_size().
  void
  write_to_buffer(unsigned char* buffer, section_size_type buffer_size);

  // Dump statistical information to stderr.
  void
  print_stats(const char*) const;

 private:
  Stringpool_template(const Stringpool_template&);
  Stringpool_template& operator=(const Stringpool_template&);

  // Return whether two strings are equal.
  static bool
  string_equal(const Stringpool_char*, const Stringpool_char*);

  // Compute a hash code for a string.  LENGTH is the length of the
  // string in characters.
  static size_t
  string_hash(const Stringpool_char*, size_t length);

  // We store the actual data in a list of these buffers.
  struct Stringdata
  {
    // Length of data in buffer.
    size_t len;
    // Allocated size of buffer.
    size_t alc;
    // Buffer.
    char data[1];
  };

  // Add a new key offset entry.
  void
  new_key_offset(size_t);

  // Copy a string into the buffers, returning a canonical string.
  const Stringpool_char*
  add_string(const Stringpool_char*, size_t);

  // Return whether s1 is a suffix of s2.
  static bool
  is_suffix(const Stringpool_char* s1, size_t len1,
            const Stringpool_char* s2, size_t len2);

  // The hash table key includes the string, the length of the string,
  // and the hash code for the string.  We put the hash code
  // explicitly into the key so that we can do a find()/insert()
  // sequence without having to recompute the hash.  Computing the
  // hash code is a significant user of CPU time in the linker.
  struct Hashkey
  {
    const Stringpool_char* string;
    // Length is in characters, not bytes.
    size_t length;
    size_t hash_code;

    // This goes in an STL container, so we need a default
    // constructor.
    Hashkey()
      : string(NULL), length(0), hash_code(0)
    { }

    // Note that these constructors are relatively expensive, because
    // they compute the hash code.
    explicit Hashkey(const Stringpool_char* s)
      : string(s), length(string_length(s)), hash_code(string_hash(s, length))
    { }

    Hashkey(const Stringpool_char* s, size_t len)
      : string(s), length(len), hash_code(string_hash(s, len))
    { }
  };

  // Hash function.  This is trivial, since we have already computed
  // the hash.
  struct Stringpool_hash
  {
    size_t
    operator()(const Hashkey& hk) const
    { return hk.hash_code; }
  };

  // Equality comparison function for hash table.
  struct Stringpool_eq
  {
    bool
    operator()(const Hashkey&, const Hashkey&) const;
  };

  // The hash table is a map from strings to Keys.

  typedef Key Hashval;

  typedef Unordered_map<Hashkey, Hashval, Stringpool_hash,
			Stringpool_eq> String_set_type;

  // Comparison routine used when sorting into a string table.

  typedef typename String_set_type::iterator Stringpool_sort_info;

  struct Stringpool_sort_comparison
  {
    bool
    operator()(const Stringpool_sort_info&, const Stringpool_sort_info&) const;
  };

  // Keys map to offsets via a Chunked_vector.  We only use the
  // offsets if we turn this into an string table section.
  typedef Chunked_vector<section_offset_type> Key_to_offset;

  // List of Stringdata structures.
  typedef std::list<Stringdata*> Stringdata_list;

  // Mapping from const char* to namepool entry.
  String_set_type string_set_;
  // Mapping from Key to string table offset.
  Key_to_offset key_to_offset_;
  // List of buffers.
  Stringdata_list strings_;
  // Size of string table.
  section_size_type strtab_size_;
  // Whether to reserve offset 0 to hold the null string.
  bool zero_null_;
  // Whether to optimize the string table.
  bool optimize_;
  // offset of the next string.
  section_offset_type offset_;
  // The alignment of strings in the stringpool.
  uint64_t addralign_;
};

// The most common type of Stringpool.
typedef Stringpool_template<char> Stringpool;

} // End namespace gold.

#endif // !defined(GOLD_STRINGPOOL_H)
