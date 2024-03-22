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

#ifndef CLOVER_UTIL_ADAPTOR_HPP
#define CLOVER_UTIL_ADAPTOR_HPP

#include <iterator>

#include "util/compat.hpp"
#include "util/tuple.hpp"
#include "util/pointer.hpp"
#include "util/functional.hpp"

namespace clover {
   namespace detail {
      ///
      /// Implementation of the iterator concept that transforms the
      /// value of the source iterators \a Is on dereference by use of
      /// a functor \a F.
      ///
      /// The exact category of the resulting iterator should be the
      /// least common denominator of the source iterator categories.
      ///
      template<typename F, typename... Is>
      class iterator_adaptor {
      public:
         typedef std::forward_iterator_tag iterator_category;
         typedef typename invoke_result<
               F, typename std::iterator_traits<Is>::reference...
            >::type reference;
         typedef typename std::remove_reference<reference>::type value_type;
         typedef pseudo_ptr<value_type> pointer;
         typedef std::ptrdiff_t difference_type;

         iterator_adaptor() {
         }

         iterator_adaptor(F f, std::tuple<Is...> &&its) :
            f(f), its(std::move(its)) {
         }

         reference
         operator*() const {
            return tuple::apply(f, tuple::map(derefs(), its));
         }

         iterator_adaptor &
         operator++() {
            tuple::map(preincs(), its);
            return *this;
         }

         iterator_adaptor
         operator++(int) {
            auto jt = *this;
            ++*this;
            return jt;
         }

         bool
         operator==(const iterator_adaptor &jt) const {
            return its == jt.its;
         }

         bool
         operator!=(const iterator_adaptor &jt) const {
            return its != jt.its;
         }

         pointer
         operator->() const {
            return { **this };
         }

         iterator_adaptor &
         operator--() {
            tuple::map(predecs(), its);
            return *this;
         }

         iterator_adaptor
         operator--(int) {
            auto jt = *this;
            --*this;
            return jt;
         }

         iterator_adaptor &
         operator+=(difference_type n) {
            tuple::map(advances_by(n), its);
            return *this;
         }

         iterator_adaptor &
         operator-=(difference_type n) {
            tuple::map(advances_by(-n), its);
            return *this;
         }

         iterator_adaptor
         operator+(difference_type n) const {
            auto jt = *this;
            jt += n;
            return jt;
         }

         iterator_adaptor
         operator-(difference_type n) const {
            auto jt = *this;
            jt -= n;
            return jt;
         }

         difference_type
         operator-(const iterator_adaptor &jt) const {
            return std::get<0>(its) - std::get<0>(jt.its);
         }

         reference
         operator[](difference_type n) const {
            return *(*this + n);
         }

         bool
         operator<(iterator_adaptor &jt) const {
            return *this - jt < 0;
         }

         bool
         operator>(iterator_adaptor &jt) const {
            return *this - jt > 0;
         }

         bool
         operator>=(iterator_adaptor &jt) const {
            return !(*this < jt);
         }

         bool
         operator<=(iterator_adaptor &jt) const {
            return !(*this > jt);
         }

      protected:
         F f;
         std::tuple<Is...> its;
      };

      template<typename F, typename... Is>
      iterator_adaptor<F, Is...>
      operator+(typename iterator_adaptor<F, Is...>::difference_type n,
                const iterator_adaptor<F, Is...> &jt) {
         return (jt + n);
      }

      template<typename F, typename... Is>
      iterator_adaptor<F, Is...>
      operator-(typename iterator_adaptor<F, Is...>::difference_type n,
                const iterator_adaptor<F, Is...> &jt) {
         return (jt - n);
      }
   }
}

#endif
