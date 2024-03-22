/*
 * Copyright Michael Schellenberger Costa
 * Copyright © 2020 Valve Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#ifndef ACO_UTIL_H
#define ACO_UTIL_H

#include "util/bitscan.h"
#include "util/macros.h"
#include "util/u_math.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iterator>
#include <map>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace aco {

/*! \brief      Definition of a span object
 *
 *   \details    A "span" is an "array view" type for holding a view of contiguous
 *               data. The "span" object does not own the data itself.
 */
template <typename T> class span {
public:
   using value_type = T;
   using pointer = value_type*;
   using const_pointer = const value_type*;
   using reference = value_type&;
   using const_reference = const value_type&;
   using iterator = pointer;
   using const_iterator = const_pointer;
   using reverse_iterator = std::reverse_iterator<iterator>;
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;
   using size_type = uint16_t;
   using difference_type = std::ptrdiff_t;

   /*! \brief                  Compiler generated default constructor
    */
   constexpr span() = default;

   /*! \brief                 Constructor taking a pointer and the length of the span
    *  \param[in]   data      Pointer to the underlying data array
    *  \param[in]   length    The size of the span
    */
   constexpr span(uint16_t offset_, const size_type length_) : offset{offset_}, length{length_} {}

   /*! \brief                 Returns an iterator to the begin of the span
    *  \return                data
    */
   constexpr iterator begin() noexcept { return (pointer)((uintptr_t)this + offset); }

   /*! \brief                 Returns a const_iterator to the begin of the span
    *  \return                data
    */
   constexpr const_iterator begin() const noexcept
   {
      return (const_pointer)((uintptr_t)this + offset);
   }

   /*! \brief                 Returns an iterator to the end of the span
    *  \return                data + length
    */
   constexpr iterator end() noexcept { return std::next(begin(), length); }

   /*! \brief                 Returns a const_iterator to the end of the span
    *  \return                data + length
    */
   constexpr const_iterator end() const noexcept { return std::next(begin(), length); }

   /*! \brief                 Returns a const_iterator to the begin of the span
    *  \return                data
    */
   constexpr const_iterator cbegin() const noexcept { return begin(); }

   /*! \brief                 Returns a const_iterator to the end of the span
    *  \return                data + length
    */
   constexpr const_iterator cend() const noexcept { return std::next(begin(), length); }

   /*! \brief                 Returns a reverse_iterator to the end of the span
    *  \return                reverse_iterator(end())
    */
   constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

   /*! \brief                 Returns a const_reverse_iterator to the end of the span
    *  \return                reverse_iterator(end())
    */
   constexpr const_reverse_iterator rbegin() const noexcept
   {
      return const_reverse_iterator(end());
   }

   /*! \brief                 Returns a reverse_iterator to the begin of the span
    *   \return                reverse_iterator(begin())
    */
   constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

   /*! \brief                 Returns a const_reverse_iterator to the begin of the span
    *  \return                reverse_iterator(begin())
    */
   constexpr const_reverse_iterator rend() const noexcept
   {
      return const_reverse_iterator(begin());
   }

   /*! \brief                 Returns a const_reverse_iterator to the end of the span
    *  \return                rbegin()
    */
   constexpr const_reverse_iterator crbegin() const noexcept
   {
      return const_reverse_iterator(cend());
   }

   /*! \brief                 Returns a const_reverse_iterator to the begin of the span
    *  \return                rend()
    */
   constexpr const_reverse_iterator crend() const noexcept
   {
      return const_reverse_iterator(cbegin());
   }

   /*! \brief                 Unchecked access operator
    *  \param[in] index       Index of the element we want to access
    *  \return                *(std::next(data, index))
    */
   constexpr reference operator[](const size_type index) noexcept
   {
      assert(length > index);
      return *(std::next(begin(), index));
   }

   /*! \brief                 Unchecked const access operator
    *  \param[in] index       Index of the element we want to access
    *  \return                *(std::next(data, index))
    */
   constexpr const_reference operator[](const size_type index) const noexcept
   {
      assert(length > index);
      return *(std::next(begin(), index));
   }

   /*! \brief                 Returns a reference to the last element of the span
    *  \return                *(std::next(data, length - 1))
    */
   constexpr reference back() noexcept
   {
      assert(length > 0);
      return *(std::next(begin(), length - 1));
   }

   /*! \brief                 Returns a const_reference to the last element of the span
    *  \return                *(std::next(data, length - 1))
    */
   constexpr const_reference back() const noexcept
   {
      assert(length > 0);
      return *(std::next(begin(), length - 1));
   }

   /*! \brief                 Returns a reference to the first element of the span
    *  \return                *begin()
    */
   constexpr reference front() noexcept
   {
      assert(length > 0);
      return *begin();
   }

   /*! \brief                 Returns a const_reference to the first element of the span
    *  \return                *cbegin()
    */
   constexpr const_reference front() const noexcept
   {
      assert(length > 0);
      return *cbegin();
   }

   /*! \brief                 Returns true if the span is empty
    *  \return                length == 0
    */
   constexpr bool empty() const noexcept { return length == 0; }

   /*! \brief                 Returns the size of the span
    *  \return                length == 0
    */
   constexpr size_type size() const noexcept { return length; }

   /*! \brief                 Decreases the size of the span by 1
    */
   constexpr void pop_back() noexcept
   {
      assert(length > 0);
      --length;
   }

   /*! \brief                 Adds an element to the end of the span
    */
   constexpr void push_back(const_reference val) noexcept { *std::next(begin(), length++) = val; }

   /*! \brief                 Clears the span
    */
   constexpr void clear() noexcept
   {
      offset = 0;
      length = 0;
   }

private:
   uint16_t offset{0};  //!> Byte offset from span to data
   size_type length{0}; //!> Size of the span
};

/*
 * Cache-friendly set of 32-bit IDs with fast insert/erase/lookup and
 * the ability to efficiently iterate over contained elements.
 *
 * Internally implemented as a map of fixed-size bit vectors: If the set contains an ID, the
 * corresponding bit in the appropriate bit vector is set. It doesn't use std::vector<bool> since
 * we then couldn't efficiently iterate over the elements.
 *
 * The interface resembles a subset of std::set/std::unordered_set.
 */
struct IDSet {
   static const uint32_t block_size = 1024u;
   using block_t = std::array<uint64_t, block_size / 64>;

   struct Iterator {
      const IDSet* set;
      std::map<uint32_t, block_t>::const_iterator block;
      uint32_t id;

      Iterator& operator++();

      bool operator!=(const Iterator& other) const;

      uint32_t operator*() const;
   };

   size_t count(uint32_t id) const { return find(id) != end(); }

   Iterator find(uint32_t id) const
   {
      uint32_t block_index = id / block_size;
      auto it = words.find(block_index);
      if (it == words.end())
         return end();

      const block_t& block = it->second;
      uint32_t sub_id = id % block_size;

      if (block[sub_id / 64u] & (1ull << (sub_id % 64u)))
         return Iterator{this, it, id};
      else
         return end();
   }

   std::pair<Iterator, bool> insert(uint32_t id)
   {
      uint32_t block_index = id / block_size;
      auto it = words.try_emplace(block_index).first;
      block_t& block = it->second;
      uint32_t sub_id = id % block_size;

      uint64_t* word = &block[sub_id / 64u];
      uint64_t mask = 1ull << (sub_id % 64u);
      if (*word & mask)
         return std::make_pair(Iterator{this, it, id}, false);

      *word |= mask;
      return std::make_pair(Iterator{this, it, id}, true);
   }

   bool insert(const IDSet other)
   {
      bool inserted = false;

      for (auto it = other.words.begin(); it != other.words.end(); ++it) {
         block_t& dst = words[it->first];
         const block_t& src = it->second;

         for (unsigned j = 0; j < src.size(); j++) {
            uint64_t new_bits = src[j] & ~dst[j];
            if (new_bits) {
               inserted = true;
               dst[j] |= new_bits;
            }
         }
      }
      return inserted;
   }

   size_t erase(uint32_t id)
   {
      uint32_t block_index = id / block_size;
      auto it = words.find(block_index);
      if (it == words.end())
         return 0;

      block_t& block = it->second;
      uint32_t sub_id = id % block_size;

      uint64_t* word = &block[sub_id / 64u];
      uint64_t mask = 1ull << (sub_id % 64u);
      if (!(*word & mask))
         return 0;

      *word ^= mask;
      return 1;
   }

   Iterator cbegin() const
   {
      Iterator res;
      res.set = this;

      for (auto it = words.begin(); it != words.end(); ++it) {
         uint32_t first = get_first_set(it->second);
         if (first != UINT32_MAX) {
            res.block = it;
            res.id = it->first * block_size + first;
            return res;
         }
      }

      return cend();
   }

   Iterator cend() const { return Iterator{this, words.end(), UINT32_MAX}; }

   Iterator begin() const { return cbegin(); }

   Iterator end() const { return cend(); }

   size_t size() const
   {
      size_t bits_set = 0;
      for (auto block : words) {
         for (uint64_t word : block.second)
            bits_set += util_bitcount64(word);
      }
      return bits_set;
   }

   bool empty() const { return !size(); }

private:
   static uint32_t get_first_set(const block_t& words)
   {
      for (size_t i = 0; i < words.size(); i++) {
         if (words[i])
            return i * 64u + (ffsll(words[i]) - 1);
      }
      return UINT32_MAX;
   }

   std::map<uint32_t, block_t> words;
};

inline IDSet::Iterator&
IDSet::Iterator::operator++()
{
   uint32_t block_index = id / block_size;
   const block_t& block_words = block->second;
   uint32_t sub_id = id % block_size;

   uint64_t m = block_words[sub_id / 64u];
   uint32_t bit = sub_id % 64u;
   m = (m >> bit) >> 1;
   if (m) {
      id += ffsll(m);
      return *this;
   }

   for (uint32_t i = sub_id / 64u + 1; i < block_words.size(); i++) {
      if (block_words[i]) {
         id = block_index * block_size + i * 64u + ffsll(block_words[i]) - 1;
         return *this;
      }
   }

   for (++block; block != set->words.end(); ++block) {
      uint32_t first = get_first_set(block->second);
      if (first != UINT32_MAX) {
         id = block->first * block_size + first;
         return *this;
      }
   }

   id = UINT32_MAX;
   return *this;
}

inline bool
IDSet::Iterator::operator!=(const IDSet::Iterator& other) const
{
   assert(set == other.set);
   assert(id != other.id || block == other.block);
   return id != other.id;
}

inline uint32_t
IDSet::Iterator::operator*() const
{
   return id;
}

/*
 * Light-weight memory resource which allows to sequentially allocate from
 * a buffer. Both, the release() method and the destructor release all managed
 * memory.
 *
 * The memory resource is not thread-safe.
 * This class mimics std::pmr::monotonic_buffer_resource
 */
class monotonic_buffer_resource final {
public:
   explicit monotonic_buffer_resource(size_t size = initial_size)
   {
      /* The size parameter refers to the total size of the buffer.
       * The usable data_size is size - sizeof(Buffer).
       */
      size = MAX2(size, minimum_size);
      buffer = (Buffer*)malloc(size);
      buffer->next = nullptr;
      buffer->data_size = size - sizeof(Buffer);
      buffer->current_idx = 0;
   }

   ~monotonic_buffer_resource()
   {
      release();
      free(buffer);
   }

   /* Delete copy-constructor and -assignment to avoid double free() */
   monotonic_buffer_resource(const monotonic_buffer_resource&) = delete;
   monotonic_buffer_resource& operator=(const monotonic_buffer_resource&) = delete;

   void* allocate(size_t size, size_t alignment)
   {
      buffer->current_idx = align(buffer->current_idx, alignment);
      if (buffer->current_idx + size <= buffer->data_size) {
         uint8_t* ptr = &buffer->data[buffer->current_idx];
         buffer->current_idx += size;
         return ptr;
      }

      /* create new larger buffer */
      uint32_t total_size = buffer->data_size + sizeof(Buffer);
      do {
         total_size *= 2;
      } while (total_size - sizeof(Buffer) < size);
      Buffer* next = buffer;
      buffer = (Buffer*)malloc(total_size);
      buffer->next = next;
      buffer->data_size = total_size - sizeof(Buffer);
      buffer->current_idx = 0;

      return allocate(size, alignment);
   }

   void release()
   {
      while (buffer->next) {
         Buffer* next = buffer->next;
         free(buffer);
         buffer = next;
      }
      buffer->current_idx = 0;
   }

   bool operator==(const monotonic_buffer_resource& other) { return buffer == other.buffer; }

private:
   struct Buffer {
      Buffer* next;
      uint32_t current_idx;
      uint32_t data_size;
      uint8_t data[];
   };

   Buffer* buffer;
   static constexpr size_t initial_size = 4096;
   static constexpr size_t minimum_size = 128;
   static_assert(minimum_size > sizeof(Buffer));
};

/*
 * Small memory allocator which wraps monotonic_buffer_resource
 * in order to implement <allocator_traits>.
 *
 * This class mimics std::pmr::polymorphic_allocator with monotonic_buffer_resource
 * as memory resource. The advantage of this specialization is the absence of
 * virtual function calls and the propagation on swap, copy- and move assignment.
 */
template <typename T> class monotonic_allocator {
public:
   monotonic_allocator() = delete;
   monotonic_allocator(monotonic_buffer_resource& m) : memory_resource(m) {}
   template <typename U>
   explicit monotonic_allocator(const monotonic_allocator<U>& rhs)
       : memory_resource(rhs.memory_resource)
   {}

   /* Memory Allocation */
   T* allocate(size_t size)
   {
      uint32_t bytes = sizeof(T) * size;
      return (T*)memory_resource.get().allocate(bytes, alignof(T));
   }

   /* Memory will be freed on destruction of memory_resource */
   void deallocate(T* ptr, size_t size) {}

   /* Implement <allocator_traits> */
   using value_type = T;
   template <class U> struct rebind {
      using other = monotonic_allocator<U>;
   };

   typedef std::true_type propagate_on_container_copy_assignment;
   typedef std::true_type propagate_on_container_move_assignment;
   typedef std::true_type propagate_on_container_swap;

   template <typename> friend class monotonic_allocator;
   template <typename X, typename Y>
   friend bool operator==(monotonic_allocator<X> const& a, monotonic_allocator<Y> const& b);
   template <typename X, typename Y>
   friend bool operator!=(monotonic_allocator<X> const& a, monotonic_allocator<Y> const& b);

private:
   std::reference_wrapper<monotonic_buffer_resource> memory_resource;
};

/* Necessary for <allocator_traits>. */
template <typename X, typename Y>
inline bool
operator==(monotonic_allocator<X> const& a, monotonic_allocator<Y> const& b)
{
   return a.memory_resource.get() == b.memory_resource.get();
}
template <typename X, typename Y>
inline bool
operator!=(monotonic_allocator<X> const& a, monotonic_allocator<Y> const& b)
{
   return !(a == b);
}

/*
 * aco::map - alias for std::map with monotonic_allocator
 *
 * This template specialization mimics std::pmr::map.
 */
template <class Key, class T, class Compare = std::less<Key>>
using map = std::map<Key, T, Compare, aco::monotonic_allocator<std::pair<const Key, T>>>;

/*
 * aco::unordered_map - alias for std::unordered_map with monotonic_allocator
 *
 * This template specialization mimics std::pmr::unordered_map.
 */
template <class Key, class T, class Hash = std::hash<Key>, class Pred = std::equal_to<Key>>
using unordered_map =
   std::unordered_map<Key, T, Hash, Pred, aco::monotonic_allocator<std::pair<const Key, T>>>;

/*
 * Helper class for a integer/bool (access_type) packed into
 * a bigger integer (data_type) with an offset and size.
 * It can be implicitly converted to access_type and supports
 * all arithmetic assignment operators.
 *
 * When used together with a union, this allows storing
 * multiple fields packed into a single integer.
 *
 * Example usage:
 * union {
 *    bitfield_uint<uint32_t, 0,  5,  uint8_t> int5;
 *    bitfield_uint<uint32_t, 5,  26, uint32_t> int26;
 *    bitfield_uint<uint32_t, 31, 1,  bool> bool1;
 * };
 *
 */
template <typename data_type, unsigned offset, unsigned size, typename access_type>
class bitfield_uint {
public:
   static_assert(sizeof(data_type) >= sizeof(access_type), "");
   static_assert(std::is_unsigned<access_type>::value, "");
   static_assert(std::is_unsigned<data_type>::value, "");
   static_assert(sizeof(data_type) * 8 >= offset + size, "");
   static_assert(sizeof(access_type) * 8 >= size, "");
   static_assert(size > 0, "");
   static_assert(!std::is_same_v<access_type, bool> || size == 1, "");

   bitfield_uint() = default;

   constexpr bitfield_uint(const access_type& value) { *this = value; }

   constexpr operator access_type() const { return (storage >> offset) & mask; }

   constexpr bitfield_uint& operator=(const access_type& value)
   {
      storage &= ~(mask << offset);
      storage |= data_type(value & mask) << offset;
      return *this;
   }

   constexpr bitfield_uint& operator=(const bitfield_uint& value)
   {
      return *this = access_type(value);
   }

   constexpr bitfield_uint& operator|=(const access_type& value)
   {
      storage |= data_type(value & mask) << offset;
      return *this;
   }

   constexpr bitfield_uint& operator^=(const access_type& value)
   {
      storage ^= data_type(value & mask) << offset;
      return *this;
   }

   constexpr bitfield_uint& operator&=(const access_type& value)
   {
      storage &= (data_type(value & mask) << offset) | ~(mask << offset);
      return *this;
   }

   constexpr bitfield_uint& operator<<=(const access_type& shift)
   {
      static_assert(!std::is_same_v<access_type, bool>, "");
      assert(shift < size);
      return *this = access_type(*this) << shift;
   }

   constexpr bitfield_uint& operator>>=(const access_type& shift)
   {
      static_assert(!std::is_same_v<access_type, bool>, "");
      assert(shift < size);
      return *this = access_type(*this) >> shift;
   }

   constexpr bitfield_uint& operator*=(const access_type& op)
   {
      static_assert(!std::is_same_v<access_type, bool>, "");
      return *this = access_type(*this) * op;
   }

   constexpr bitfield_uint& operator/=(const access_type& op)
   {
      static_assert(!std::is_same_v<access_type, bool>, "");
      return *this = access_type(*this) / op;
   }

   constexpr bitfield_uint& operator%=(const access_type& op)
   {
      static_assert(!std::is_same_v<access_type, bool>, "");
      return *this = access_type(*this) % op;
   }

   constexpr bitfield_uint& operator+=(const access_type& op)
   {
      static_assert(!std::is_same_v<access_type, bool>, "");
      return *this = access_type(*this) + op;
   }

   constexpr bitfield_uint& operator-=(const access_type& op)
   {
      static_assert(!std::is_same_v<access_type, bool>, "");
      return *this = access_type(*this) - op;
   }

   constexpr bitfield_uint& operator++()
   {
      static_assert(!std::is_same_v<access_type, bool>, "");
      return *this += 1;
   }

   constexpr access_type operator++(int)
   {
      static_assert(!std::is_same_v<access_type, bool>, "");
      access_type temp = *this;
      ++*this;
      return temp;
   }

   constexpr bitfield_uint& operator--()
   {
      static_assert(!std::is_same_v<access_type, bool>, "");
      return *this -= 1;
   }

   constexpr access_type operator--(int)
   {
      static_assert(!std::is_same_v<access_type, bool>, "");
      access_type temp = *this;
      --*this;
      return temp;
   }

   constexpr void swap(access_type& other)
   {
      access_type tmp = *this;
      *this = other;
      other = tmp;
   }

   template <typename other_dt, unsigned other_off, unsigned other_s>
   constexpr void swap(bitfield_uint<other_dt, other_off, other_s, access_type>& other)
   {
      access_type tmp = *this;
      *this = other;
      other = tmp;
   }

protected:
   static const data_type mask = BITFIELD64_MASK(size);

   data_type storage;
};

/*
 * Reference to a single bit in an integer that can be converted to a bool
 * and supports bool (bitwise) assignment operators.
 */
template <typename T> struct bit_reference {
   constexpr bit_reference(T& s, unsigned b) : storage(s), bit(b) {}

   constexpr bit_reference& operator=(const bit_reference& other) { return *this = (bool)other; }

   constexpr bit_reference& operator=(bool val)
   {
      storage &= ~(T(0x1) << bit);
      storage |= T(val) << bit;
      return *this;
   }

   constexpr bit_reference& operator^=(bool val)
   {
      storage ^= T(val) << bit;
      return *this;
   }

   constexpr bit_reference& operator|=(bool val)
   {
      storage |= T(val) << bit;
      return *this;
   }

   constexpr bit_reference& operator&=(bool val)
   {
      storage &= T(val) << bit;
      return *this;
   }

   constexpr operator bool() const { return (storage >> bit) & 0x1; }

   constexpr void swap(bool& other)
   {
      bool tmp = (bool)*this;
      *this = other;
      other = tmp;
   }

   template <typename other_T> constexpr void swap(bit_reference<other_T> other)
   {
      bool tmp = (bool)*this;
      *this = (bool)other;
      other = tmp;
   }

   T& storage;
   unsigned bit;
};

/*
 * Base template for (const) bit iterators over an integer.
 * Only intended to be used with the two specializations
 * bitfield_array::iterator and bitfield_array::const_iterator.
 */
template <typename T, typename refT, typename ptrT> struct bitfield_iterator {
   using difference_type = int;
   using value_type = bool;
   using iterator_category = std::random_access_iterator_tag;
   using reference = refT;
   using const_reference = bool;
   using pointer = ptrT;
   using iterator = bitfield_iterator<T, refT, ptrT>;
   using ncT = std::remove_const_t<T>;

   constexpr bitfield_iterator() : bf(nullptr), index(0) {}
   constexpr bitfield_iterator(T* p, unsigned i) : bf(p), index(i) {}

   /* const iterator must be constructable from iterator */
   constexpr bitfield_iterator(
      const bitfield_iterator<ncT, bit_reference<ncT>, bit_reference<ncT>*>& x)
       : bf(x.bf), index(x.index)
   {}

   constexpr bool operator==(const bitfield_iterator& other) const
   {
      return bf == other.bf && index == other.index;
   }

   constexpr bool operator<(const bitfield_iterator& other) const { return index < other.index; }

   constexpr bool operator!=(const bitfield_iterator& other) const { return !(*this == other); }

   constexpr bool operator>(const bitfield_iterator& other) const { return other < *this; }

   constexpr bool operator<=(const bitfield_iterator& other) const { return !(other < *this); }

   constexpr bool operator>=(const bitfield_iterator& other) const { return !(*this < other); }

   constexpr reference operator*() const { return bit_reference<T>(*bf, index); }

   constexpr iterator& operator++()
   {
      index++;
      return *this;
   }

   constexpr iterator operator++(int)
   {
      iterator tmp = *this;
      index++;
      return tmp;
   }

   constexpr iterator& operator--()
   {
      index--;
      return *this;
   }

   constexpr iterator operator--(int)
   {
      iterator tmp = *this;
      index--;
      return tmp;
   }

   constexpr iterator& operator+=(difference_type value)
   {
      index += value;
      return *this;
   }

   constexpr iterator& operator-=(difference_type value)
   {
      *this += -value;
      return *this;
   }

   constexpr iterator operator+(difference_type value) const
   {
      iterator tmp = *this;
      return tmp += value;
   }

   constexpr iterator operator-(difference_type value) const
   {
      iterator tmp = *this;
      return tmp -= value;
   }

   constexpr reference operator[](difference_type value) const { return *(*this + value); }

   T* bf;
   unsigned index;
};

template <typename T, typename refT, typename ptrT>
constexpr inline bitfield_iterator<T, refT, ptrT>
operator+(int n, const bitfield_iterator<T, refT, ptrT>& x)
{
   return x + n;
}

template <typename T, typename refT, typename ptrT>
constexpr inline int
operator-(const bitfield_iterator<T, refT, ptrT> x, const bitfield_iterator<T, refT, ptrT>& y)
{
   return x.index - y.index;
}

/*
 * Extends bitfield_uint with operator[] and iterators that
 * allow accessing single bits within the uint. Can be used
 * as a more compact version of bool arrays that also still
 * allows accessing the whole array as an integer.
 */
template <typename data_type, unsigned offset, unsigned size, typename access_type>
class bitfield_array : public bitfield_uint<data_type, offset, size, access_type> {
public:
   using value_type = bool;
   using size_type = unsigned;
   using difference_type = int;
   using reference = bit_reference<data_type>;
   using const_reference = bool;
   using pointer = bit_reference<data_type>*;
   using const_pointer = const bool*;
   using iterator =
      bitfield_iterator<data_type, bit_reference<data_type>, bit_reference<data_type>*>;
   using const_iterator = bitfield_iterator<const data_type, bool, const bool*>;
   using reverse_iterator = std::reverse_iterator<iterator>;
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   bitfield_array() = default;

   constexpr bitfield_array(const access_type& value) { *this = value; }

   constexpr bitfield_array& operator=(const access_type& value)
   {
      storage &= ~(mask << offset);
      storage |= data_type(value & mask) << offset;
      return *this;
   }

   constexpr bitfield_array& operator=(const bitfield_array& value)
   {
      return *this = access_type(value);
   }

   constexpr reference operator[](unsigned index)
   {
      assert(index < size);
      return reference(storage, offset + index);
   }

   constexpr bool operator[](unsigned index) const
   {
      assert(index < size);
      return (storage >> (offset + index)) & 0x1;
   }

   constexpr iterator begin() noexcept { return iterator(&storage, offset); }

   constexpr iterator end() noexcept { return std::next(begin(), size); }

   constexpr const_iterator begin() const noexcept { return const_iterator(&storage, offset); }

   constexpr const_iterator end() const noexcept { return std::next(begin(), size); }

   constexpr const_iterator cbegin() const noexcept { return begin(); }

   constexpr const_iterator cend() const noexcept { return std::next(begin(), size); }

   constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

   constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

   constexpr const_reverse_iterator rbegin() const noexcept
   {
      return const_reverse_iterator(end());
   }

   constexpr const_reverse_iterator rend() const noexcept
   {
      return const_reverse_iterator(begin());
   }

   constexpr const_reverse_iterator crbegin() const noexcept
   {
      return const_reverse_iterator(cend());
   }

   constexpr const_reverse_iterator crend() const noexcept
   {
      return const_reverse_iterator(cbegin());
   }

private:
   using bitfield_uint<data_type, offset, size, access_type>::storage;
   using bitfield_uint<data_type, offset, size, access_type>::mask;
};

template <typename T, unsigned offset> using bitfield_bool = bitfield_uint<T, offset, 1, bool>;

template <typename T, unsigned offset, unsigned size>
using bitfield_uint8 = bitfield_uint<T, offset, size, uint8_t>;

template <typename T, unsigned offset, unsigned size>
using bitfield_uint16 = bitfield_uint<T, offset, size, uint16_t>;

template <typename T, unsigned offset, unsigned size>
using bitfield_uint32 = bitfield_uint<T, offset, size, uint32_t>;

template <typename T, unsigned offset, unsigned size>
using bitfield_uint64 = bitfield_uint<T, offset, size, uint64_t>;

template <typename T, unsigned offset, unsigned size>
using bitfield_array8 = bitfield_array<T, offset, size, uint8_t>;

template <typename T, unsigned offset, unsigned size>
using bitfield_array16 = bitfield_array<T, offset, size, uint16_t>;

template <typename T, unsigned offset, unsigned size>
using bitfield_array32 = bitfield_array<T, offset, size, uint32_t>;

template <typename T, unsigned offset, unsigned size>
using bitfield_array64 = bitfield_array<T, offset, size, uint64_t>;

using bitarray8 = bitfield_array<uint8_t, 0, 8, uint8_t>;

} // namespace aco

#endif // ACO_UTIL_H
