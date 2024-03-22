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

#ifndef CLOVER_UTIL_TUPLE_HPP
#define CLOVER_UTIL_TUPLE_HPP

#include <tuple>

namespace clover {
   namespace tuple {
      ///
      /// Static sequence of integers.
      ///
      template<int... Is>
      struct integral_sequence;

      ///
      /// Static sequence containing all integers from 0 to N-1.
      ///
      template<int N, int... Is>
      struct enumerate {
         typedef typename enumerate<N-1, N-1, Is...>::type
            type;
      };

      template<int... Is>
      struct enumerate<0, Is...> {
         typedef integral_sequence<Is...> type;
      };

      namespace detail {
         template<typename F, typename T,
                  typename E = typename enumerate<std::tuple_size<
                        typename std::remove_reference<T>::type>::value
                     >::type>
         struct _apply;

         template<typename F, typename T, int... Is>
         struct _apply<F, T, integral_sequence<Is...>> {
            typedef typename std::remove_reference<F>::type func_type;
            typedef decltype(
               std::declval<func_type>()(std::get<Is>(std::declval<T &&>())...)
               ) value_type;

            static value_type
               eval(F &&f, T &&t) {
               return f(std::get<Is>(std::forward<T>(t))...);
            }
         };
      }

      ///
      /// Evaluate function \a f with the elements of tuple \a t
      /// expanded as arguments.
      ///
      template<typename F, typename T>
      typename detail::_apply<F, T>::value_type
      apply(F &&f, T &&t) {
         return detail::_apply<F, T>::eval(std::forward<F>(f),
                                           std::forward<T>(t));
      }

      namespace detail {
         template<typename F, typename T,
                  typename E = typename enumerate<std::tuple_size<
                        typename std::remove_reference<T>::type>::value
                     >::type>
         struct _map;

         template<typename F, typename T, int... Is>
         struct _map<F, T, integral_sequence<Is...>> {
            typedef typename std::remove_reference<F>::type func_type;
            typedef std::tuple<
               decltype(std::declval<func_type>()(
                           std::get<Is>(std::declval<T &&>())))...
               > value_type;

            static value_type
               eval(F &&f, T &&t) {
               return value_type(f(std::get<Is>(std::forward<T>(t)))...);
            }
         };
      }

      ///
      /// Evaluate function \a f on each element of the tuple \a t and
      /// return the resulting values as a new tuple.
      ///
      template<typename F, typename T>
      typename detail::_map<F, T>::value_type
      map(F &&f, T &&t) {
         return detail::_map<F, T>::eval(std::forward<F>(f),
                                         std::forward<T>(t));
      }
   }
}

#endif
