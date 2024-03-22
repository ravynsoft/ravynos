//
// Copyright 2012 Francisco Jerez
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

#ifndef CLOVER_CORE_RESOURCE_HPP
#define CLOVER_CORE_RESOURCE_HPP

#include <list>

#include "core/queue.hpp"
#include "util/algebra.hpp"
#include "pipe/p_state.h"

namespace clover {
   class memory_obj;
   class mapping;

   ///
   /// Class that represents a device-specific instance of some memory
   /// object.
   ///
   class resource {
   public:
      typedef std::array<size_t, 3> vector;

      virtual ~resource();

      resource(const resource &r) = delete;
      resource &
      operator=(const resource &r) = delete;

      void copy(command_queue &q, const vector &origin, const vector &region,
                resource &src_resource, const vector &src_origin);

      void clear(command_queue &q, const vector &origin, const vector &region,
                 const std::string &data);

      mapping *add_map(command_queue &q, cl_map_flags flags, bool blocking,
                       const vector &origin, const vector &region);
      void del_map(void *p);
      unsigned map_count() const;

      const intrusive_ref<clover::device> device;
      memory_obj &obj;

      friend class sub_resource;
      friend class mapping;
      friend class kernel;

   protected:
      resource(clover::device &dev, memory_obj &obj);

      pipe_sampler_view *bind_sampler_view(command_queue &q);
      void unbind_sampler_view(command_queue &q,
                               pipe_sampler_view *st);

      pipe_surface *bind_surface(command_queue &q, bool rw);
      void unbind_surface(command_queue &q, pipe_surface *st);

      pipe_image_view create_image_view(command_queue &q);

      pipe_resource *pipe;
      vector offset;

   private:
      std::list<mapping> maps;
   };

   ///
   /// Resource associated with its own top-level data storage
   /// allocated in some device.
   ///
   class root_resource : public resource {
   public:
      root_resource(clover::device &dev, memory_obj &obj,
                    command_queue &q, const void *data_ptr);
      root_resource(clover::device &dev, memory_obj &obj, root_resource &r);
      virtual ~root_resource();
   };

   ///
   /// Resource that reuses a portion of some other resource as data
   /// storage.
   ///
   class sub_resource : public resource {
   public:
      sub_resource(resource &r, const vector &offset);
   };

   ///
   /// Class that represents a mapping of some resource into the CPU
   /// memory space.
   ///
   class mapping {
   public:
      mapping(command_queue &q, resource &r, cl_map_flags flags,
              bool blocking, const resource::vector &origin,
              const resource::vector &region);
      mapping(mapping &&m);
      ~mapping();

      mapping &
      operator=(mapping m);

      mapping(const mapping &m) = delete;

      template<typename T>
      operator T *() const {
         return (T *)p;
      }

      resource::vector pitch() const;

   private:
      pipe_context *pctx;
      pipe_transfer *pxfer;
      pipe_resource *pres;
      void *p;
   };
}

#endif
