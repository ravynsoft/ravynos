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

#ifndef CLOVER_CORE_MEMORY_HPP
#define CLOVER_CORE_MEMORY_HPP

#include <functional>
#include <map>
#include <memory>
#include <stack>

#include "core/object.hpp"
#include "core/queue.hpp"
#include "core/resource.hpp"

namespace clover {
   class memory_obj : public ref_counter, public _cl_mem {
   protected:
      memory_obj(clover::context &ctx,
                 std::vector<cl_mem_properties> properties,
                 cl_mem_flags flags,
                 size_t size, void *host_ptr);

      memory_obj(const memory_obj &obj) = delete;
      memory_obj &
      operator=(const memory_obj &obj) = delete;

   public:
      virtual ~memory_obj();

      bool
      operator==(const memory_obj &obj) const;

      virtual cl_mem_object_type type() const = 0;
      virtual clover::resource &
      resource_in(command_queue &q) = 0;
      virtual clover::resource &
      resource_undef(command_queue &q) = 0;
      virtual void resource_out(command_queue &q) = 0;

      void destroy_notify(std::function<void ()> f);
      std::vector<cl_mem_properties> properties() const;
      cl_mem_flags flags() const;
      size_t size() const;
      void *host_ptr() const;

      const intrusive_ref<clover::context> context;

   private:
      std::vector<cl_mem_properties> _properties;
      cl_mem_flags _flags;
      size_t _size;
      void *_host_ptr;
      std::stack<std::function<void ()>> _destroy_notify;

   protected:
      std::string data;
   };

   class buffer : public memory_obj {
   protected:
      buffer(clover::context &ctx,
             std::vector<cl_mem_properties> properties,
             cl_mem_flags flags,
             size_t size, void *host_ptr);

   public:
      virtual cl_mem_object_type type() const;
   };

   class root_buffer : public buffer {
   public:
      root_buffer(clover::context &ctx,
                  std::vector<cl_mem_properties> properties,
                  cl_mem_flags flags,
                  size_t size, void *host_ptr);

      virtual clover::resource &
      resource_in(command_queue &q);
      virtual clover::resource &
      resource_undef(command_queue &q);
      virtual void
      resource_out(command_queue &q);

   private:
      clover::resource &
         resource(command_queue &q, const void *data_ptr);

      std::map<device *,
               std::unique_ptr<root_resource>> resources;
      std::mutex resources_mtx;
   };

   class sub_buffer : public buffer {
   public:
      sub_buffer(root_buffer &parent, cl_mem_flags flags,
                 size_t offset, size_t size);

      virtual clover::resource &
      resource_in(command_queue &q);
      virtual clover::resource &
      resource_undef(command_queue &q);
      virtual void
      resource_out(command_queue &q);
      size_t offset() const;

      const intrusive_ref<root_buffer> parent;

   private:
      size_t _offset;
      std::map<device *,
               std::unique_ptr<sub_resource>> resources;
      std::mutex resources_mtx;
   };

   class image : public memory_obj {
   protected:
      image(clover::context &ctx,
            std::vector<cl_mem_properties> properties,
            cl_mem_flags flags,
            const cl_image_format *format,
            size_t width, size_t height, size_t depth, size_t array_size,
            size_t row_pitch, size_t slice_pitch, size_t size,
            void *host_ptr, cl_mem buffer);

   public:
      cl_image_format format() const;
      virtual cl_uint dimensions() const = 0;
      size_t width() const;
      size_t height() const;
      size_t depth() const;
      size_t pixel_size() const;
      size_t row_pitch() const;
      size_t slice_pitch() const;
      size_t array_size() const;
      cl_mem buffer() const;
      virtual clover::resource &
      resource_in(command_queue &q);
      virtual clover::resource &
      resource_undef(command_queue &q);
      virtual void
      resource_out(command_queue &q);

   private:
      clover::resource &
         resource(command_queue &q, const void *data_ptr);

      cl_image_format _format;
      size_t _width;
      size_t _height;
      size_t _depth;
      size_t _row_pitch;
      size_t _slice_pitch;
      size_t _array_size;
      cl_mem _buffer;
      std::map<device *,
               std::unique_ptr<root_resource>> resources;
      std::mutex resources_mtx;
   };

   template<cl_mem_object_type Type, cl_uint Dim>
   class basic_image : public image {
   public:
      using image::image;
      virtual cl_mem_object_type type() const {
         return Type;
      }
      virtual cl_uint dimensions() const {
         return Dim;
      }
   };

   class image1d : public basic_image<CL_MEM_OBJECT_IMAGE1D, 1> {
   public:
      image1d(clover::context &ctx,
              std::vector<cl_mem_properties> properties,
              cl_mem_flags flags,
              const cl_image_format *format,
              size_t width, size_t row_pitch,
              void *host_ptr);
   };

   class image1d_buffer : public basic_image<CL_MEM_OBJECT_IMAGE1D_BUFFER, 1> {
   public:
      image1d_buffer(clover::context &ctx,
                     std::vector<cl_mem_properties> properties,
                     cl_mem_flags flags,
                     const cl_image_format *format,
                     size_t width, size_t row_pitch,
                     void *host_ptr, cl_mem buffer);
   };

   class image1d_array : public basic_image<CL_MEM_OBJECT_IMAGE1D_ARRAY, 1> {
   public:
      image1d_array(clover::context &ctx,
                    std::vector<cl_mem_properties> properties,
                    cl_mem_flags flags,
                    const cl_image_format *format,
                    size_t width,
                    size_t array_size, size_t slice_pitch,
                    void *host_ptr);
   };

   class image2d : public basic_image<CL_MEM_OBJECT_IMAGE2D, 2> {
   public:
      image2d(clover::context &ctx,
              std::vector<cl_mem_properties> properties,
              cl_mem_flags flags,
              const cl_image_format *format, size_t width,
              size_t height, size_t row_pitch,
              void *host_ptr);
   };

   class image2d_array : public basic_image<CL_MEM_OBJECT_IMAGE2D_ARRAY, 2> {
   public:
      image2d_array(clover::context &ctx,
                    std::vector<cl_mem_properties> properties,
                    cl_mem_flags flags,
                    const cl_image_format *format,
                    size_t width, size_t height, size_t array_size,
                    size_t row_pitch, size_t slice_pitch,
                    void *host_ptr);
   };

   class image3d : public basic_image<CL_MEM_OBJECT_IMAGE3D, 3>{
   public:
      image3d(clover::context &ctx,
              std::vector<cl_mem_properties> properties,
              cl_mem_flags flags,
              const cl_image_format *format,
              size_t width, size_t height, size_t depth,
              size_t row_pitch, size_t slice_pitch,
              void *host_ptr);
   };
}

#endif
