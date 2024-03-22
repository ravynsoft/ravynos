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

#ifndef CLOVER_UTIL_FUNCTIONAL_HPP
#define CLOVER_UTIL_FUNCTIONAL_HPP

#include <type_traits>

namespace clover {
   struct identity {
      template<typename T>
      typename std::remove_reference<T>::type
      operator()(T &&x) const {
         return x;
      }
   };

   struct plus {
      template<typename T, typename S>
      typename std::common_type<T, S>::type
      operator()(T x, S y) const {
         return x + y;
      }
   };

   struct minus {
      template<typename T, typename S>
      typename std::common_type<T, S>::type
      operator()(T x, S y) const {
         return x - y;
      }
   };

   struct negate {
      template<typename T>
      T
      operator()(T x) const {
         return -x;
      }
   };

   struct multiplies {
      template<typename T, typename S>
      typename std::common_type<T, S>::type
      operator()(T x, S y) const {
         return x * y;
      }
   };

   struct divides {
      template<typename T, typename S>
      typename std::common_type<T, S>::type
      operator()(T x, S y) const {
         return x / y;
      }
   };

   struct modulus {
      template<typename T, typename S>
      typename std::common_type<T, S>::type
      operator()(T x, S y) const {
         return x % y;
      }
   };

   struct minimum {
      template<typename T>
      T
      operator()(T x) const {
         return x;
      }

      template<typename T, typename... Ts>
      T
      operator()(T x, Ts... xs) const {
         T y = minimum()(xs...);
         return x < y ? x : y;
      }
   };

   struct maximum {
      template<typename T>
      T
      operator()(T x) const {
         return x;
      }

      template<typename T, typename... Ts>
      T
      operator()(T x, Ts... xs) const {
         T y = maximum()(xs...);
         return x < y ? y : x;
      }
   };

   struct preincs {
      template<typename T>
      T &
      operator()(T &x) const {
         return ++x;
      }
   };

   struct predecs {
      template<typename T>
      T &
      operator()(T &x) const {
         return --x;
      }
   };

   template<typename T>
   class multiplies_by_t {
   public:
      multiplies_by_t(T x) : x(x) {
      }

      template<typename S>
      typename std::common_type<T, S>::type
      operator()(S y) const {
         return x * y;
      }

   private:
      T x;
   };

   template<typename T>
   multiplies_by_t<T>
   multiplies_by(T x) {
      return { x };
   }

   template<typename T>
   class preincs_by_t {
   public:
      preincs_by_t(T n) : n(n) {
      }

      template<typename S>
      S &
      operator()(S &x) const {
         return x += n;
      }

   private:
      T n;
   };

   template<typename T>
   preincs_by_t<T>
   preincs_by(T n) {
      return { n };
   }

   template<typename T>
   class predecs_by_t {
   public:
      predecs_by_t(T n) : n(n) {
      }

      template<typename S>
      S &
      operator()(S &x) const {
         return x -= n;
      }

   private:
      T n;
   };

   template<typename T>
   predecs_by_t<T>
   predecs_by(T n) {
      return { n };
   }

   struct greater {
      template<typename T, typename S>
      bool
      operator()(T x, S y) const {
         return x > y;
      }
   };

   struct evals {
      template<typename T>
      auto
      operator()(T &&x) const -> decltype(x()) {
         return x();
      }
   };

   struct derefs {
      template<typename T>
      auto
      operator()(T &&x) const -> decltype(*x) {
         return *x;
      }
   };

   struct addresses {
      template<typename T>
      T *
      operator()(T &x) const {
         return &x;
      }

      template<typename T>
      T *
      operator()(std::reference_wrapper<T> x) const {
         return &x.get();
      }
   };

   struct begins {
      template<typename T>
      auto
      operator()(T &x) const -> decltype(x.begin()) {
         return x.begin();
      }
   };

   struct ends {
      template<typename T>
      auto
      operator()(T &x) const -> decltype(x.end()) {
         return x.end();
      }
   };

   struct sizes {
      template<typename T>
      auto
      operator()(T &x) const -> decltype(x.size()) {
         return x.size();
      }
   };

   template<typename T>
   class advances_by_t {
   public:
      advances_by_t(T n) : n(n) {
      }

      template<typename S>
      S
      operator()(S &&it) const {
         std::advance(it, n);
         return std::forward<S>(it);
      }

   private:
      T n;
   };

   template<typename T>
   advances_by_t<T>
   advances_by(T n) {
      return { n };
   }

   struct zips {
      template<typename... Ts>
      std::tuple<Ts...>
      operator()(Ts &&... xs) const {
         return std::tuple<Ts...>(std::forward<Ts>(xs)...);
      }
   };

   struct is_zero {
      template<typename T>
      bool
      operator()(const T &x) const {
         return x == 0;
      }
   };

   struct keys {
      template<typename P>
      auto
      operator()(P &&p) const -> decltype(std::get<0>(std::forward<P>(p))) {
         return std::get<0>(std::forward<P>(p));
      }
   };

   struct values {
      template<typename P>
      auto
      operator()(P &&p) const -> decltype(std::get<1>(std::forward<P>(p))) {
         return std::get<1>(std::forward<P>(p));
      }
   };

   template<typename T>
   class equals_t {
   public:
      equals_t(T &&x) : x(x) {}

      template<typename S>
      bool
      operator()(S &&y) const {
         return x == y;
      }

   private:
      T x;
   };

   template<typename T>
   equals_t<T>
   equals(T &&x) {
      return { std::forward<T>(x) };
   }

   class name_equals {
   public:
      name_equals(const std::string &name) : name(name) {
      }

      template<typename T>
      bool
      operator()(const T &x) const {
         return std::string(x.name.begin(), x.name.end()) == name;
      }

   private:
      const std::string &name;
   };

   template<typename T>
   class key_equals_t {
   public:
      key_equals_t(T &&x) : x(x) {
      }

      template<typename P>
      bool
      operator()(const P &p) const {
         return p.first == x;
      }

   private:
      T x;
   };

   template<typename T>
   key_equals_t<T>
   key_equals(T &&x) {
      return { std::forward<T>(x) };
   }

   template<typename T>
   class type_equals_t {
   public:
      type_equals_t(T type) : type(type) {
      }

      template<typename S>
      bool
      operator()(const S &x) const {
         return x.type == type;
      }

   private:
      T type;
   };

   template<typename T>
   type_equals_t<T>
   type_equals(T x) {
      return { x };
   }

   template<typename T>
   class id_type_equals_t {
   public:
      id_type_equals_t(const uint32_t id, T t) :
         id(id), type(t) {
      }

      template<typename X>
      bool
      operator()(const X &x) const {
         return id == x.id && type(x);
      }

   private:
      const uint32_t id;
      type_equals_t<T> type;
   };

   template<typename T>
   id_type_equals_t<T>
   id_type_equals(const uint32_t id, T x) {
      return { id, x };
   }

   struct interval_overlaps {
      template<typename T>
      bool
      operator()(T x0, T x1, T y0, T y1) {
         return ((x0 <= y0 && y0 < x1) ||
                 (y0 <= x0 && x0 < y1));
      }
   };
}

#endif
