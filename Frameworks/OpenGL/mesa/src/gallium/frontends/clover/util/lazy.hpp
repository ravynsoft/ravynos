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

#ifndef CLOVER_UTIL_LAZY_HPP
#define CLOVER_UTIL_LAZY_HPP

#include <type_traits>
#include <stdexcept>
#include <memory>

namespace clover {
   namespace detail {
      template<typename T>
      class basic_lazy {
      public:
         virtual
         ~basic_lazy() {
         }

         virtual basic_lazy *
         clone() const = 0;

         virtual
         operator T() const = 0;
      };

      template<typename T, typename F>
      class deferred_lazy : public basic_lazy<T> {
      public:
         template<typename G>
         deferred_lazy(G &&f) : f(new F(std::forward<G>(f))) {
         }

         virtual basic_lazy<T> *
         clone() const {
            return new deferred_lazy(*this);
         }

         operator T() const {
            if (f) {
               x = (*f)();
               f = {};
            }

            return x;
         }

      private:
         mutable std::shared_ptr<F> f;
         mutable T x;
      };

      template<typename T>
      class strict_lazy : public basic_lazy<T> {
      public:
         template<typename S>
         strict_lazy(S &&x) : x(std::forward<S>(x)) {
         }

         virtual basic_lazy<T> *
         clone() const {
            return new strict_lazy(*this);
         }

         operator T() const {
            return x;
         }

      private:
         T x;
      };
   }

   ///
   /// Object that represents a value of type \a T that is calculated
   /// lazily as soon as it is required.
   ///
   template<typename T>
   class lazy {
   public:
      class undefined_error : std::logic_error {
      public:
         undefined_error() : std::logic_error("") {
         }
      };

      ///
      /// Initialize to some fixed value \a x which isn't calculated
      /// lazily.
      ///
      lazy(T x) : obj(new detail::strict_lazy<T>(x)) {
      }

      ///
      /// Initialize by providing a functor \a f that will calculate
      /// the value on-demand.
      ///
      template<typename F>
      lazy(F &&f) : obj(new detail::deferred_lazy<
                           T, typename std::remove_reference<F>::type
                        >(std::forward<F>(f))) {
      }

      ///
      /// Initialize to undefined.
      ///
      lazy() : lazy([]() {
               throw undefined_error();
               return T();
            }) {
      }

      lazy(const lazy &other) : obj(obj->clone()) {
      }

      lazy(lazy &&other) : obj(NULL) {
         std::swap(obj, other.obj);
      }

      ~lazy() {
         delete obj;
      }

      lazy &
      operator=(lazy other) {
         std::swap(obj, other.obj);
         return *this;
      }

      ///
      /// Evaluate the value.
      ///
      operator T() const {
         return *obj;
      }

   private:
      detail::basic_lazy<T> *obj;
   };
}

#endif
