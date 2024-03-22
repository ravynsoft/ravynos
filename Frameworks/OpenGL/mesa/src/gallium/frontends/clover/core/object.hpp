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

#ifndef CLOVER_CORE_OBJECT_HPP
#define CLOVER_CORE_OBJECT_HPP

#include <cassert>
#include <functional>
#include <vector>

#include "CL/cl.h"

#include "core/error.hpp"
#include "core/property.hpp"
#include "api/dispatch.hpp"
#include "util/macros.h"

///
/// Main namespace of the CL gallium frontend.
///
namespace clover {
   ///
   /// Class that represents a CL API object.
   ///
   template<typename T, typename S>
   struct descriptor {
      typedef T object_type;
      typedef S descriptor_type;

      descriptor() : dispatch(&_dispatch) {
         static_assert(std::is_standard_layout<descriptor_type>::value,
                       "ICD requires CL API objects to be standard layout.");
      }

      const cl_icd_dispatch *dispatch;
   };

   struct default_tag;
   struct allow_empty_tag;
   struct wait_list_tag;
   struct property_list_tag;

   namespace detail {
      template<typename T, typename D>
      struct descriptor_traits {
         typedef T object_type;

         static void
         validate(D *d) {
            auto o = static_cast<typename D::object_type *>(d);
            if (!o || o->dispatch != &_dispatch ||
                !dynamic_cast<object_type *>(o))
               throw invalid_object_error<T>();
         }

         static void
         validate_list(D * const *ds, size_t n) {
            if (!ds || !n)
               throw error(CL_INVALID_VALUE);
         }
      };

      template<typename D>
      struct descriptor_traits<default_tag, D> {
         typedef typename D::object_type object_type;

         static void
         validate(D *d) {
            if (!d || d->dispatch != &_dispatch)
               throw invalid_object_error<object_type>();
         }

         static void
         validate_list(D *const *ds, size_t n) {
            if (!ds || !n)
               throw error(CL_INVALID_VALUE);
         }
      };

      template<typename D>
      struct descriptor_traits<allow_empty_tag, D> {
         typedef typename D::object_type object_type;

         static void
         validate(D *d) {
            if (!d || d->dispatch != &_dispatch)
               throw invalid_object_error<object_type>();
         }

         static void
         validate_list(D *const *ds, size_t n) {
            if (bool(ds) != bool(n))
               throw error(CL_INVALID_VALUE);
         }
      };

      template<typename D>
      struct descriptor_traits<wait_list_tag, D> {
         typedef typename D::object_type object_type;

         static void
         validate(D *d) {
            if (!d || d->dispatch != &_dispatch)
               throw invalid_wait_list_error();
         }

         static void
         validate_list(D *const *ds, size_t n) {
            if (bool(ds) != bool(n))
               throw invalid_wait_list_error();
         }
      };
   }

   ///
   /// Get a Clover object from an API object performing object
   /// validation.
   ///
   /// \a T can either be the Clover object type to return or a \a tag
   /// object to select some special validation behavior by means of a
   /// specialization of the detail::descriptor_traits template.  The
   /// default behavior is to infer the most general Clover object
   /// type for the given API object.
   ///
   template<typename T = default_tag, typename D>
   typename detail::descriptor_traits<T, D>::object_type &
   obj(D *d) {
      detail::descriptor_traits<T, D>::validate(d);

      return static_cast<
         typename detail::descriptor_traits<T, D>::object_type &>(*d);
   }

   ///
   /// Get a pointer to a Clover object from an API object performing
   /// object validation.  Returns \c NULL if its argument is \c NULL.
   ///
   /// \sa obj
   ///
   template<typename T = default_tag, typename D>
   typename detail::descriptor_traits<T, D>::object_type *
   pobj(D *d) {
      if (d)
         detail::descriptor_traits<T, D>::validate(d);

      return static_cast<
         typename detail::descriptor_traits<T, D>::object_type *>(d);
   }

   ///
   /// Get an API object from a Clover object.
   ///
   template<typename O>
   typename O::descriptor_type *
   desc(O &o) {
      return static_cast<typename O::descriptor_type *>(&o);
   }

   ///
   /// Get an API object from a pointer to a Clover object.
   ///
   template<typename O>
   typename O::descriptor_type *
   desc(O *o) {
      return static_cast<typename O::descriptor_type *>(o);
   }

   ///
   /// Get a range of Clover objects from a range of API objects
   /// performing object validation.
   ///
   /// \sa obj
   ///
   template<typename T = default_tag, typename D>
   ref_vector<typename detail::descriptor_traits<T, D>::object_type>
   objs(D *const *ds, size_t n) {
      detail::descriptor_traits<T, D>::validate_list(ds, n);
      return map(obj<T, D>, range(ds, n));
   }

   ///
   /// Get a range of API objects from a range of Clover objects.
   ///
   template<typename Os>
   std::vector<typename Os::value_type::descriptor_type *>
   descs(const Os &os) {
      return map([](typename Os::value_type &o) {
            return desc(o);
         }, os);
   }
}

struct _cl_context :
   public clover::descriptor<clover::context, _cl_context> {};

struct _cl_device_id :
   public clover::descriptor<clover::device, _cl_device_id> {};

struct _cl_event :
   public clover::descriptor<clover::event, _cl_event> {};

struct _cl_kernel :
   public clover::descriptor<clover::kernel, _cl_kernel> {};

struct _cl_mem :
   public clover::descriptor<clover::memory_obj, _cl_mem> {};

struct _cl_platform_id :
   public clover::descriptor<clover::platform, _cl_platform_id> {};

struct _cl_program :
   public clover::descriptor<clover::program, _cl_program> {};

struct _cl_command_queue :
   public clover::descriptor<clover::command_queue, _cl_command_queue> {};

struct _cl_sampler :
   public clover::descriptor<clover::sampler, _cl_sampler> {};

#endif
