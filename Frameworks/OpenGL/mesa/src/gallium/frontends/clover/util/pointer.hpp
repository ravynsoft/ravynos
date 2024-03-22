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

#ifndef CLOVER_UTIL_POINTER_HPP
#define CLOVER_UTIL_POINTER_HPP

#include <atomic>

namespace clover {
   ///
   /// Some helper functions for raw pointer operations
   ///
   template <class T>
   static bool
   ptr_is_aligned(const T *ptr, uintptr_t a) noexcept {
      assert(a == (a & -a));
      uintptr_t ptr_value = reinterpret_cast<uintptr_t>(ptr);
      return (ptr_value & (a - 1)) == 0;
   }

   ///
   /// Base class for objects that support reference counting.
   ///
   class ref_counter {
   public:
      ref_counter(unsigned value = 1) : _ref_count(value) {}

      unsigned
      ref_count() const {
         return _ref_count;
      }

      void
      retain() {
         _ref_count++;
      }

      bool
      release() {
         return (--_ref_count) == 0;
      }

   private:
      std::atomic<unsigned> _ref_count;
   };

   ///
   /// Simple reference to a clover::ref_counter object.  Unlike
   /// clover::intrusive_ptr and clover::intrusive_ref, it does nothing
   /// special when the reference count drops to zero.
   ///
   class ref_holder {
   public:
      ref_holder(ref_counter &o) : p(&o) {
         p->retain();
      }

      ref_holder(const ref_holder &ref) :
         ref_holder(*ref.p) {
      }

      ref_holder(ref_holder &&ref) :
         p(ref.p) {
         ref.p = NULL;
      }

      ~ref_holder() {
         if (p)
            p->release();
      }

      ref_holder &
      operator=(ref_holder ref) {
         std::swap(ref.p, p);
         return *this;
      }

      bool
      operator==(const ref_holder &ref) const {
         return p == ref.p;
      }

      bool
      operator!=(const ref_holder &ref) const {
         return p != ref.p;
      }

   private:
      ref_counter *p;
   };

   ///
   /// Intrusive smart pointer for objects that implement the
   /// clover::ref_counter interface.
   ///
   template<typename T>
   class intrusive_ptr {
   public:
      intrusive_ptr(T *q = NULL) : p(q) {
         if (p)
            p->retain();
      }

      intrusive_ptr(const intrusive_ptr &ptr) :
         intrusive_ptr(ptr.p) {
      }

      intrusive_ptr(intrusive_ptr &&ptr) :
         p(ptr.p) {
         ptr.p = NULL;
      }

      ~intrusive_ptr() {
         if (p && p->release())
            delete p;
      }

      intrusive_ptr &
      operator=(intrusive_ptr ptr) {
         std::swap(ptr.p, p);
         return *this;
      }

      bool
      operator==(const intrusive_ptr &ref) const {
         return p == ref.p;
      }

      bool
      operator!=(const intrusive_ptr &ref) const {
         return p != ref.p;
      }

      T &
      operator*() const {
         return *p;
      }

      T *
      operator->() const {
         return p;
      }

      T *
      operator()() const {
         return p;
      }

      explicit operator bool() const {
         return p;
      }

      explicit operator T *() const {
         return p;
      }

   private:
      T *p;
   };

   ///
   /// Intrusive smart reference for objects that implement the
   /// clover::ref_counter interface.
   ///
   template<typename T>
   class intrusive_ref {
   public:
      intrusive_ref(T &o) : p(&o) {
         p->retain();
      }

      intrusive_ref(const intrusive_ref &ref) :
         intrusive_ref(*ref.p) {
      }

      intrusive_ref(intrusive_ref &&ref) :
         p(ref.p) {
         ref.p = NULL;
      }

      ~intrusive_ref() {
         if (p && p->release())
            delete p;
      }

      intrusive_ref &
      operator=(intrusive_ref ref) {
         std::swap(ref.p, p);
         return *this;
      }

      bool
      operator==(const intrusive_ref &ref) const {
         return p == ref.p;
      }

      bool
      operator!=(const intrusive_ref &ref) const {
         return p != ref.p;
      }

      T &
      operator()() const {
         return *p;
      }

      operator T &() const {
         return *p;
      }

   private:
      T *p;
   };

   ///
   /// Initialize a clover::intrusive_ref from a newly created object
   /// using the specified constructor arguments.
   ///
   template<typename T, typename... As>
   intrusive_ref<T>
   create(As &&... as) {
      intrusive_ref<T> ref { *new T(std::forward<As>(as)...) };
      ref().release();
      return ref;
   }

   ///
   /// Class that implements the usual pointer interface but in fact
   /// contains the object it seems to be pointing to.
   ///
   template<typename T>
   class pseudo_ptr {
   public:
      pseudo_ptr(T x) : x(x) {
      }

      pseudo_ptr(const pseudo_ptr &p) : x(p.x) {
      }

      pseudo_ptr &
      operator=(const pseudo_ptr &p) {
         x = p.x;
         return *this;
      }

      T &
      operator*() {
         return x;
      }

      T *
      operator->() {
         return &x;
      }

      explicit operator bool() const {
         return true;
      }

   private:
      T x;
   };
}

#endif
