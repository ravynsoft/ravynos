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

#ifndef CLOVER_UTIL_ALGEBRA_HPP
#define CLOVER_UTIL_ALGEBRA_HPP

#include <type_traits>

#include "util/range.hpp"
#include "util/functional.hpp"

namespace clover {
   ///
   /// Class that identifies vectors (in the linear-algebraic sense).
   ///
   /// There should be a definition of this class for each type that
   /// makes sense as vector arithmetic operand.
   ///
   template<typename V, typename = void>
   struct vector_traits;

   ///
   /// References of vectors are vectors.
   ///
   template<typename T>
   struct vector_traits<T &, typename vector_traits<T>::enable> {
      typedef void enable;
   };

   ///
   /// Constant vectors are vectors.
   ///
   template<typename T>
   struct vector_traits<const T, typename vector_traits<T>::enable> {
      typedef void enable;
   };

   ///
   /// Arrays of arithmetic types are vectors.
   ///
   template<typename T, std::size_t N>
   struct vector_traits<std::array<T, N>,
                        typename std::enable_if<
                           std::is_arithmetic<T>::value>::type> {
      typedef void enable;
   };

   namespace detail {
      template<typename... Ts>
      struct are_defined {
         typedef void enable;
      };
   }

   ///
   /// The result of mapping a vector is a vector.
   ///
   template<typename F, typename... Vs>
   struct vector_traits<adaptor_range<F, Vs...>,
                        typename detail::are_defined<
                           typename vector_traits<Vs>::enable...>::enable> {
      typedef void enable;
   };

   ///
   /// Vector sum.
   ///
   template<typename U, typename V,
            typename = typename vector_traits<U>::enable,
            typename = typename vector_traits<V>::enable>
   adaptor_range<plus, U, V>
   operator+(U &&u, V &&v) {
      return map(plus(), std::forward<U>(u), std::forward<V>(v));
   }

   ///
   /// Vector difference.
   ///
   template<typename U, typename V,
            typename = typename vector_traits<U>::enable,
            typename = typename vector_traits<V>::enable>
   adaptor_range<minus, U, V>
   operator-(U &&u, V &&v) {
      return map(minus(), std::forward<U>(u), std::forward<V>(v));
   }

   ///
   /// Scalar multiplication.
   ///
   template<typename U, typename T,
            typename = typename vector_traits<U>::enable>
   adaptor_range<multiplies_by_t<T>, U>
   operator*(U &&u, T &&a) {
      return map(multiplies_by<T>(std::forward<T>(a)), std::forward<U>(u));
   }

   ///
   /// Scalar multiplication.
   ///
   template<typename U, typename T,
            typename = typename vector_traits<U>::enable>
   adaptor_range<multiplies_by_t<T>, U>
   operator*(T &&a, U &&u) {
      return map(multiplies_by<T>(std::forward<T>(a)), std::forward<U>(u));
   }

   ///
   /// Additive inverse.
   ///
   template<typename U,
            typename = typename vector_traits<U>::enable>
   adaptor_range<negate, U>
   operator-(U &&u) {
      return map(negate(), std::forward<U>(u));
   }

   namespace detail {
      template<typename U, typename V>
      using dot_type = typename std::common_type<
           typename std::remove_reference<U>::type::value_type,
           typename std::remove_reference<V>::type::value_type
         >::type;
   }

   ///
   /// Dot product of two vectors.
   ///
   /// It can also do matrix multiplication if \a u or \a v is a
   /// vector of vectors.
   ///
   template<typename U, typename V,
            typename = typename vector_traits<U>::enable,
            typename = typename vector_traits<V>::enable>
   detail::dot_type<U, V>
   dot(U &&u, V &&v) {
      return fold(plus(), detail::dot_type<U, V>(),
                  map(multiplies(), u, v));
   }
}

#endif
