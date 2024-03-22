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

#ifndef CLOVER_UTIL_ALGORITHM_HPP
#define CLOVER_UTIL_ALGORITHM_HPP

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "util/range.hpp"
#include "util/functional.hpp"

namespace clover {
   namespace detail {
      template<typename R>
      using preferred_reference_type = decltype(*std::declval<R>().begin());
   }

   ///
   /// Return the first element in a range.
   ///
   template<typename R>
   detail::preferred_reference_type<R>
   head(R &&r) {
      assert(!r.empty());
      return r.front();
   }

   ///
   /// Return all elements in a range but the first.
   ///
   template<typename R>
   slice_range<R>
   tail(R &&r) {
      assert(!r.empty());
      return { std::forward<R>(r), 1, r.size() };
   }

   ///
   /// Return the only element in a range.
   ///
   template<typename R>
   detail::preferred_reference_type<R>
   unique(R &&r) {
      if (r.size() != 1)
         throw std::out_of_range("");

      return r.front();
   }

   ///
   /// Combine a variable number of ranges element-wise in a single
   /// range of tuples.
   ///
   template<typename... Rs>
   adaptor_range<zips, Rs...>
   zip(Rs &&... rs) {
      return map(zips(), std::forward<Rs>(rs)...);
   }

   ///
   /// Evaluate the elements of a range.
   ///
   /// Useful because most of the range algorithms evaluate their
   /// result lazily.
   ///
   template<typename R>
   void
   eval(R &&r) {
      for (auto i = r.begin(), e = r.end(); i != e; ++i)
         *i;
   }

   ///
   /// Apply functor \a f element-wise on a variable number of ranges
   /// \a rs.
   ///
   /// The functor \a f should take as many arguments as ranges are
   /// provided.
   ///
   template<typename F, typename... Rs>
   void
   for_each(F &&f, Rs &&... rs) {
      eval(map(std::forward<F>(f), std::forward<Rs>(rs)...));
   }

   ///
   /// Copy all elements from range \a r into an output container
   /// starting from iterator \a i.
   ///
   template<typename R, typename I>
   void
   copy(R &&r, I i) {
      for (detail::preferred_reference_type<R> x : r)
         *(i++) = x;
   }

   ///
   /// Reduce the elements of range \a r by applying functor \a f
   /// element by element.
   ///
   /// \a f should take an accumulator value (which is initialized to
   /// \a a) and an element value as arguments, and return an updated
   /// accumulator value.
   ///
   /// \returns The final value of the accumulator.
   ///
   template<typename F, typename A, typename R>
   A
   fold(F &&f, A a, R &&r) {
      for (detail::preferred_reference_type<R> x : r)
         a = f(a, x);

      return a;
   }

   ///
   /// Return how many elements of range \a r are equal to \a x.
   ///
   template<typename T, typename R>
   typename std::remove_reference<R>::type::size_type
   count(T &&x, R &&r) {
      typename std::remove_reference<R>::type::size_type n = 0;

      for (detail::preferred_reference_type<R> y : r) {
         if (x == y)
            n++;
      }

      return n;
   }

   ///
   /// Return the first element in range \a r for which predicate \a f
   /// evaluates to true.
   ///
   template<typename F, typename R>
   detail::preferred_reference_type<R>
   find(F &&f, R &&r) {
      for (detail::preferred_reference_type<R> x : r) {
         if (f(x))
            return x;
      }

      throw std::out_of_range("");
   }

   ///
   /// Return true if the element-wise application of predicate \a f
   /// on \a rs evaluates to true for all elements.
   ///
   template<typename F, typename... Rs>
   bool
   all_of(F &&f, Rs &&... rs) {
      for (auto b : map(f, rs...)) {
         if (!b)
            return false;
      }

      return true;
   }

   ///
   /// Return true if the element-wise application of predicate \a f
   /// on \a rs evaluates to true for any element.
   ///
   template<typename F, typename... Rs>
   bool
   any_of(F &&f, Rs &&... rs) {
      for (auto b : map(f, rs...)) {
         if (b)
            return true;
      }

      return false;
   }

   ///
   /// Erase elements for which predicate \a f evaluates to true from
   /// container \a r.
   ///
   template<typename F, typename R>
   void
   erase_if(F &&f, R &&r) {
      auto i = r.begin(), e = r.end();

      for (auto j = r.begin(); j != e; ++j) {
         if (!f(*j)) {
            if (j != i)
               *i = std::move(*j);
            ++i;
         }
      }

      r.erase(i, e);
   }

   ///
   /// Build a vector of string from a space separated string
   /// quoted parts content is preserved and unquoted
   ///
   inline std::vector<std::string>
   tokenize(const std::string &s) {
      std::vector<std::string> ss;
      std::ostringstream oss;

      // OpenCL programs can pass a quoted argument, most frequently the
      // include path. This is useful so that path containing spaces is
      // treated as a single argument instead of being split by the spaces.
      // Additionally, the argument should also be unquoted before being
      // passed to the compiler. We avoid using std::string::replace here to
      // remove quotes, as the single and double quote characters can be a
      // part of the file name.
      bool escape_next = false;
      bool in_quote_double = false;
      bool in_quote_single = false;

      for (auto c : s) {
         if (escape_next) {
            oss.put(c);
            escape_next = false;
         } else if (c == '\\') {
            escape_next = true;
         } else if (c == '"' && !in_quote_single) {
            in_quote_double = !in_quote_double;
         } else if (c == '\'' && !in_quote_double) {
            in_quote_single = !in_quote_single;
         } else if (c != ' ' || in_quote_single || in_quote_double) {
            oss.put(c);
         } else if (oss.tellp() > 0) {
            ss.emplace_back(oss.str());
            oss.str("");
         }
      }

      if (oss.tellp() > 0)
         ss.emplace_back(oss.str());

      if (in_quote_double || in_quote_single)
         throw invalid_build_options_error();

      return ss;
   }

   ///
   /// Build a \a sep separated string from a vector of T
   ///
   template<typename T>
   std::string
   detokenize(const std::vector<T> &ss, const std::string &sep) {
      std::string r;

      for (const auto &s : ss)
         r += (r.empty() ? "" : sep) + std::to_string(s);

      return r;
   }

   ///
   /// Build a \a sep separated string from a vector of string
   ///
   template <>
   inline std::string
   detokenize(const std::vector<std::string> &ss, const std::string &sep) {
      std::string r;

      for (const auto &s : ss)
         r += (r.empty() || s.empty() ? "" : sep) + s;

      return r;
   }
}

#endif
