//
// Copyright 2013 Francisco Jerez
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//

#ifndef CLOVER_UTIL_RANGE_HPP
#define CLOVER_UTIL_RANGE_HPP

#include <array>
#include <vector>

#include "util/adaptor.hpp"

namespace clover {
   ///
   /// Class that identifies container types where the elements of a
   /// range can be stored by the type conversion operator.
   ///
   /// \a T identifies the range element type.
   ///
   template<typename T, typename V>
   struct range_store_traits;

   template<typename T, typename S>
   struct range_store_traits<T, std::vector<S>> {
      typedef void enable;

      template<typename R>
      static std::vector<S>
      create(const R &r) {
         return { r.begin(), r.end() };
      }
   };

   template<typename T, typename S, std::size_t N>
   struct range_store_traits<T, std::array<S, N>> {
      typedef void enable;

      template<typename R>
      static std::array<S, N>
      create(const R &r) {
         std::array<S, N> v;
         assert(r.size() == v.size());
         copy(r, v.begin());
         return v;
      }
   };

   namespace detail {
      ///
      /// Common functionality that is shared by other implementations
      /// of the container concept.
      ///
      template<typename R, typename I, typename CI>
      class basic_range {
      public:
         typedef I iterator;
         typedef CI const_iterator;
         typedef typename std::iterator_traits<iterator>::value_type value_type;
         typedef typename std::iterator_traits<iterator>::reference
            reference;
         typedef typename std::iterator_traits<const_iterator>::reference
            const_reference;
         typedef typename std::iterator_traits<iterator>::difference_type
            difference_type;
         typedef std::size_t size_type;

         bool
         operator==(const basic_range &r) const {
            return *static_cast<const R *>(this) == r;
         }

         bool
         operator!=(const basic_range &r) const {
            return !(*this == r);
         }

         iterator
         begin() {
            return static_cast<R *>(this)->begin();
         }

         iterator
         end() {
            return static_cast<R *>(this)->end();
         }

         const_iterator
         begin() const {
            return static_cast<const R *>(this)->begin();
         }

         const_iterator
         end() const {
            return static_cast<const R *>(this)->end();
         }

         std::reverse_iterator<iterator>
         rbegin() {
            return { begin() };
         }

         std::reverse_iterator<iterator>
         rend() {
            return { end() };
         }

         reference
         front() {
            return *begin();
         }

         reference
         back() {
            return *(end() - 1);
         }

         bool
         empty() const {
            return begin() == end();
         }

         reference
         at(size_type i) {
            if (i >= static_cast<const R *>(this)->size())
               throw std::out_of_range("");

            return begin()[i];
         }

         const_reference
         at(size_type i) const {
            if (i >= static_cast<const R *>(this)->size())
               throw std::out_of_range("");

            return begin()[i];
         }

         reference
         operator[](size_type i) {
            return begin()[i];
         }

         const_reference
         operator[](size_type i) const {
            return begin()[i];
         }

         template<typename V>
         using store_traits = range_store_traits<
               typename std::remove_cv<value_type>::type, V
            >;

         template<typename V,
                  typename = typename store_traits<V>::enable>
         operator V() const {
            return store_traits<V>::create(*static_cast<const R *>(this));
         }
      };
   }

   ///
   /// Range that contains all elements delimited by an iterator pair
   /// (\a i, \a j).  Use range() as convenience constructor.
   ///
   template<typename I>
   class iterator_range : public detail::basic_range<iterator_range<I>, I, I> {
   public:
      typedef detail::basic_range<iterator_range<I>, I, I> super;

      iterator_range() : i(), j() {
      }

      iterator_range(I i, I j) : i(i), j(j) {
      }

      bool
      operator==(const iterator_range &r) const {
         return i == r.i && j == r.j;
      }

      I
      begin() const {
         return i;
      }

      I
      end() const {
         return j;
      }

      typename super::size_type
      size() const {
         return end() - begin();
      }

   private:
      I i, j;
   };

   namespace detail {
      template<typename T>
      using preferred_iterator_type = decltype(std::declval<T>().begin());
   }

   ///
   /// Range that transforms the contents of a number of source ranges
   /// \a os element-wise by using the provided functor \a f.  Use
   /// map() as convenience constructor.
   ///
   template<typename F, typename... Os>
   class adaptor_range :
      public detail::basic_range<adaptor_range<F, Os...>,
                                 detail::iterator_adaptor<
                                    F, detail::preferred_iterator_type<Os>...>,
                                 detail::iterator_adaptor<
                                    F, detail::preferred_iterator_type<const Os>...>
                                 > {
   public:
      typedef detail::basic_range<adaptor_range<F, Os...>,
                                  detail::iterator_adaptor<
                                     F, detail::preferred_iterator_type<Os>...>,
                                  detail::iterator_adaptor<
                                     F, detail::preferred_iterator_type<const Os>...>
                                  > super;

      template<typename G, typename... Rs>
      adaptor_range(G &&f, Rs &&... os) :
         f(std::forward<G>(f)), os(std::forward<Rs>(os)...) {
      }

      bool
      operator==(const adaptor_range &r) const {
         return f == r.f && os == r.os;
      }

      typename super::iterator
      begin() {
         return { f, tuple::map(begins(), os) };
      }

      typename super::iterator
      end() {
         return { f, tuple::map(advances_by(size()),
                                tuple::map(begins(), os)) };
      }

      typename super::const_iterator
      begin() const {
         return { f, tuple::map(begins(), os) };
      }

      typename super::const_iterator
      end() const {
         return { f, tuple::map(advances_by(size()),
                                tuple::map(begins(), os)) };
      }

      typename super::size_type
      size() const {
         return tuple::apply(minimum(), tuple::map(sizes(), os));
      }

   private:
      F f;
      std::tuple<Os...> os;
   };

   ///
   /// Range that contains all elements delimited by the index pair
   /// (\a i, \a j) in the source range \a r.  Use slice() as
   /// convenience constructor.
   ///
   template<typename O>
   class slice_range :
      public detail::basic_range<slice_range<O>,
                                 detail::preferred_iterator_type<O>,
                                 detail::preferred_iterator_type<const O>> {
   public:
      typedef detail::basic_range<slice_range<O>,
                                 detail::preferred_iterator_type<O>,
                                 detail::preferred_iterator_type<const O>
                                  > super;

      template<typename R>
      slice_range(R &&r, typename super::size_type i,
                  typename super::size_type j) :
         o(std::forward<R>(r)), i(i), j(j) {
      }

      bool
      operator==(const slice_range &r) const {
         return o == r.o && i == r.i && j == r.j;
      }

      typename super::iterator
      begin() {
         return std::next(o.begin(), i);
      }

      typename super::iterator
      end() {
         return std::next(o.begin(), j);
      }

      typename super::const_iterator
      begin() const {
         return std::next(o.begin(), i);
      }

      typename super::const_iterator
      end() const {
         return std::next(o.begin(), j);
      }

      typename super::size_type
      size() const {
         return j - i;
      }

   private:
      O o;
      typename super::size_type i, j;
   };

   ///
   /// Create a range from an iterator pair (\a i, \a j).
   ///
   /// \sa iterator_range.
   ///
   template<typename T>
   iterator_range<T>
   range(T i, T j) {
      return { i, j };
   }

   ///
   /// Create a range of \a n elements starting from iterator \a i.
   ///
   /// \sa iterator_range.
   ///
   template<typename T>
   iterator_range<T>
   range(T i, typename std::iterator_traits<T>::difference_type n) {
      return { i, i + n };
   }

   ///
   /// Create a range by transforming the contents of a number of
   /// source ranges \a rs element-wise using a provided functor \a f.
   ///
   /// \sa adaptor_range.
   ///
   template<typename F, typename... Rs>
   adaptor_range<F, Rs...>
   map(F &&f, Rs &&... rs) {
      return { std::forward<F>(f), std::forward<Rs>(rs)... };
   }

   ///
   /// Create a range identical to another range \a r.
   ///
   template<typename R>
   adaptor_range<identity, R>
   range(R &&r) {
      return { identity(), std::forward<R>(r) };
   }

   ///
   /// Create a range by taking the elements delimited by the index
   /// pair (\a i, \a j) in a source range \a r.
   ///
   /// \sa slice_range.
   ///
   template<typename R>
   slice_range<R>
   slice(R &&r, typename slice_range<R>::size_type i,
         typename slice_range<R>::size_type j) {
      return { std::forward<R>(r), i, j };
   }

   ///
   /// Range that behaves as a vector of references of type \a T.
   ///
   /// Useful because STL containers cannot contain references to
   /// objects as elements.
   ///
   template<typename T>
   class ref_vector : public adaptor_range<derefs, std::vector<T *>> {
   public:
      ref_vector(std::initializer_list<std::reference_wrapper<T>> il) :
         adaptor_range<derefs, std::vector<T *>>(derefs(), map(addresses(), il)) {
      }

      template<typename R>
      ref_vector(R &&r) : adaptor_range<derefs, std::vector<T *>>(
         derefs(), map(addresses(), std::forward<R>(r))) {
      }
   };
}

#endif
