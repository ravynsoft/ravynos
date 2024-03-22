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

#include "core/resource.hpp"
#include "core/memory.hpp"
#include "pipe/p_screen.h"
#include "util/u_sampler.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_resource.h"
#include "util/u_surface.h"

using namespace clover;

namespace {
   class box {
   public:
      box(const resource::vector &origin, const resource::vector &size) :
        pipe({ (int)origin[0], (int16_t)origin[1],
               (int16_t)origin[2], (int)size[0],
               (int16_t)size[1], (int16_t)size[2] }) {
      }

      operator const pipe_box *() {
         return &pipe;
      }

   protected:
      pipe_box pipe;
   };
}

resource::resource(clover::device &dev, memory_obj &obj) :
   device(dev), obj(obj), pipe(NULL), offset() {
}

resource::~resource() {
}

void
resource::copy(command_queue &q, const vector &origin, const vector &region,
               resource &src_res, const vector &src_origin) {
   auto p = offset + origin;

   q.pipe->resource_copy_region(q.pipe, pipe, 0, p[0], p[1], p[2],
                                src_res.pipe, 0,
                                box(src_res.offset + src_origin, region));
}

void
resource::clear(command_queue &q, const vector &origin, const vector &region,
                const std::string &data) {
   auto from = offset + origin;

   if (pipe->target == PIPE_BUFFER) {
      q.pipe->clear_buffer(q.pipe, pipe, from[0], region[0], data.data(), data.size());
   } else {
      std::string texture_data;
      texture_data.reserve(util_format_get_blocksize(pipe->format));
      util_format_pack_rgba(pipe->format, &texture_data[0], data.data(), 1);
      if (q.pipe->clear_texture) {
         q.pipe->clear_texture(q.pipe, pipe, 0, box(from, region), texture_data.data());
      } else {
         u_default_clear_texture(q.pipe, pipe, 0, box(from, region), texture_data.data());
      }
   }
}

mapping *
resource::add_map(command_queue &q, cl_map_flags flags, bool blocking,
                  const vector &origin, const vector &region) {
   maps.emplace_back(q, *this, flags, blocking, origin, region);
   return &maps.back();
}

void
resource::del_map(void *p) {
   erase_if([&](const mapping &b) {
         return static_cast<void *>(b) == p;
      }, maps);
}

unsigned
resource::map_count() const {
   return maps.size();
}

pipe_sampler_view *
resource::bind_sampler_view(command_queue &q) {
   pipe_sampler_view info;

   u_sampler_view_default_template(&info, pipe, pipe->format);
   return q.pipe->create_sampler_view(q.pipe, pipe, &info);
}

void
resource::unbind_sampler_view(command_queue &q,
                              pipe_sampler_view *st) {
   q.pipe->sampler_view_destroy(q.pipe, st);
}

pipe_image_view
resource::create_image_view(command_queue &q) {
   pipe_image_view view;
   view.resource = pipe;
   view.format = pipe->format;
   view.access = 0;
   view.shader_access = PIPE_IMAGE_ACCESS_WRITE;

   if (pipe->target == PIPE_BUFFER) {
      view.u.buf.offset = 0;
      view.u.buf.size = obj.size();
   } else {
      view.u.tex.first_layer = 0;
      if (util_texture_is_array(pipe->target))
         view.u.tex.last_layer = pipe->array_size - 1;
      else
         view.u.tex.last_layer = 0;
      view.u.tex.level = 0;
   }

   return view;
}

pipe_surface *
resource::bind_surface(command_queue &q, bool rw) {
   pipe_surface info {};

   info.format = pipe->format;
   info.writable = rw;

   if (pipe->target == PIPE_BUFFER)
      info.u.buf.last_element = pipe->width0 - 1;

   return q.pipe->create_surface(q.pipe, pipe, &info);
}

void
resource::unbind_surface(command_queue &q, pipe_surface *st) {
   q.pipe->surface_destroy(q.pipe, st);
}

root_resource::root_resource(clover::device &dev, memory_obj &obj,
                             command_queue &q, const void *data_ptr) :
   resource(dev, obj) {
   pipe_resource info {};

   if (image *img = dynamic_cast<image *>(&obj)) {
      info.format = translate_format(img->format());
      info.width0 = img->width();
      info.height0 = img->height();
      info.depth0 = img->depth();
      info.array_size = MAX2(1, img->array_size());
   } else {
      info.width0 = obj.size();
      info.height0 = 1;
      info.depth0 = 1;
      info.array_size = 1;
   }

   info.target = translate_target(obj.type());
   info.bind = (PIPE_BIND_SAMPLER_VIEW |
                PIPE_BIND_COMPUTE_RESOURCE |
                PIPE_BIND_GLOBAL);

   if (obj.flags() & CL_MEM_USE_HOST_PTR && dev.allows_user_pointers()) {
      // Page alignment is normally required for this, just try, hope for the
      // best and fall back if it fails.
      pipe = dev.pipe->resource_from_user_memory(dev.pipe, &info, obj.host_ptr());
      if (pipe)
         return;
   }

   if (obj.flags() & (CL_MEM_ALLOC_HOST_PTR | CL_MEM_USE_HOST_PTR)) {
      info.usage = PIPE_USAGE_STAGING;
   }

   pipe = dev.pipe->resource_create(dev.pipe, &info);
   if (!pipe)
      throw error(CL_OUT_OF_RESOURCES);

   if (data_ptr) {
      box rect { {{ 0, 0, 0 }}, {{ info.width0, info.height0, info.depth0 }} };
      unsigned cpp = util_format_get_blocksize(info.format);

      if (pipe->target == PIPE_BUFFER)
         q.pipe->buffer_subdata(q.pipe, pipe, PIPE_MAP_WRITE,
                                0, info.width0, data_ptr);
      else
         q.pipe->texture_subdata(q.pipe, pipe, 0, PIPE_MAP_WRITE,
                                 rect, data_ptr, cpp * info.width0,
                                 cpp * info.width0 * info.height0);
   }
}

root_resource::root_resource(clover::device &dev, memory_obj &obj,
                             root_resource &r) :
   resource(dev, obj) {
   assert(0); // XXX -- resource shared among dev and r.dev
}

root_resource::~root_resource() {
   pipe_resource_reference(&this->pipe, NULL);
}

sub_resource::sub_resource(resource &r, const vector &offset) :
   resource(r.device(), r.obj) {
   this->pipe = r.pipe;
   this->offset = r.offset + offset;
}

mapping::mapping(command_queue &q, resource &r,
                 cl_map_flags flags, bool blocking,
                 const resource::vector &origin,
                 const resource::vector &region) :
   pctx(q.pipe), pres(NULL) {
   unsigned usage = ((flags & CL_MAP_WRITE ? PIPE_MAP_WRITE : 0 ) |
                     (flags & CL_MAP_READ ? PIPE_MAP_READ : 0 ) |
                     (flags & CL_MAP_WRITE_INVALIDATE_REGION ?
                      PIPE_MAP_DISCARD_RANGE : 0) |
                     (!blocking ? PIPE_MAP_UNSYNCHRONIZED : 0));

   p = pctx->buffer_map(pctx, r.pipe, 0, usage,
                          box(origin + r.offset, region), &pxfer);
   if (!p) {
      pxfer = NULL;
      throw error(CL_OUT_OF_RESOURCES);
   }
   pipe_resource_reference(&pres, r.pipe);
}

mapping::mapping(mapping &&m) :
   pctx(m.pctx), pxfer(m.pxfer), pres(m.pres), p(m.p) {
   m.pctx = NULL;
   m.pxfer = NULL;
   m.pres = NULL;
   m.p = NULL;
}

mapping::~mapping() {
   if (pxfer) {
      pctx->buffer_unmap(pctx, pxfer);
   }
   pipe_resource_reference(&pres, NULL);
}

mapping &
mapping::operator=(mapping m) {
   std::swap(pctx, m.pctx);
   std::swap(pxfer, m.pxfer);
   std::swap(pres, m.pres);
   std::swap(p, m.p);
   return *this;
}

resource::vector
mapping::pitch() const
{
   return {
      util_format_get_blocksize(pres->format),
      pxfer->stride,
      pxfer->layer_stride,
   };
}
