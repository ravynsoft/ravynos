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

#ifndef CLOVER_CORE_KERNEL_HPP
#define CLOVER_CORE_KERNEL_HPP

#include <map>
#include <memory>

#include "core/object.hpp"
#include "core/printf.hpp"
#include "core/program.hpp"
#include "core/memory.hpp"
#include "core/sampler.hpp"
#include "pipe/p_state.h"

namespace clover {
   class kernel : public ref_counter, public _cl_kernel {
   private:
      ///
      /// Class containing all the state required to execute a compute
      /// kernel.
      ///
      struct exec_context {
         exec_context(kernel &kern);
         ~exec_context();

         exec_context(const exec_context &) = delete;
         exec_context &
         operator=(const exec_context &) = delete;

         void *bind(intrusive_ptr<command_queue> _q,
                    const std::vector<size_t> &grid_offset);
         void unbind();

         kernel &kern;
         intrusive_ptr<command_queue> q;
         std::unique_ptr<printf_handler> print_handler;

         std::vector<uint8_t> input;
         std::vector<void *> samplers;
         std::vector<pipe_sampler_view *> sviews;
         std::vector<pipe_image_view> iviews;
         std::vector<pipe_surface *> resources;
         std::vector<pipe_resource *> g_buffers;
         std::vector<size_t> g_handles;
         size_t mem_local;

      private:
         void *st;
         pipe_compute_state cs;
      };

   public:
      class argument {
      public:
         static std::unique_ptr<argument>
         create(const binary::argument &barg);

         argument(const argument &arg) = delete;
         argument &
         operator=(const argument &arg) = delete;

         /// \a true if the argument has been set.
         bool set() const;

         /// Storage space required for the referenced object.
         virtual size_t storage() const;

         /// Set this argument to some object.
         virtual void set(size_t size, const void *value) = 0;

         /// Set this argument to an SVM pointer.
         virtual void set_svm(const void *value) {
            throw error(CL_INVALID_ARG_INDEX);
         };

         /// Allocate the necessary resources to bind the specified
         /// object to this argument, and update \a ctx accordingly.
         virtual void bind(exec_context &ctx,
                           const binary::argument &barg) = 0;

         /// Free any resources that were allocated in bind().
         virtual void unbind(exec_context &ctx) = 0;

         virtual ~argument() {};
      protected:
         argument();

         bool _set;
      };

   private:
      typedef adaptor_range<
            derefs, std::vector<std::unique_ptr<argument>> &
         > argument_range;

      typedef adaptor_range<
            derefs, const std::vector<std::unique_ptr<argument>> &
         > const_argument_range;

   public:
      kernel(clover::program &prog, const std::string &name,
             const std::vector<clover::binary::argument> &bargs);

      kernel(const kernel &kern) = delete;
      kernel &
      operator=(const kernel &kern) = delete;

      void launch(command_queue &q,
                  const std::vector<size_t> &grid_offset,
                  const std::vector<size_t> &grid_size,
                  const std::vector<size_t> &block_size);

      size_t mem_local() const;
      size_t mem_private() const;

      const std::string &name() const;

      std::vector<size_t>
      optimal_block_size(const command_queue &q,
                         const std::vector<size_t> &grid_size) const;
      std::vector<size_t>
      required_block_size() const;

      argument_range args();
      const_argument_range args() const;
      std::vector<clover::binary::arg_info> args_infos();

      const intrusive_ref<clover::program> program;

   private:
      const clover::binary &binary(const command_queue &q) const;

      class scalar_argument : public argument {
      public:
         scalar_argument(size_t size);

         virtual void set(size_t size, const void *value);
         virtual void bind(exec_context &ctx,
                           const binary::argument &barg);
         virtual void unbind(exec_context &ctx);

      private:
         size_t size;
         std::vector<uint8_t> v;
      };

      class global_argument : public argument {
      public:
         global_argument();

         virtual void set(size_t size, const void *value);
         virtual void set_svm(const void *value);
         virtual void bind(exec_context &ctx,
                           const binary::argument &barg);
         virtual void unbind(exec_context &ctx);

      private:
         buffer *buf;
         const void *svm;
      };

      class local_argument : public argument {
      public:
         virtual size_t storage() const;

         virtual void set(size_t size, const void *value);
         virtual void bind(exec_context &ctx,
                           const binary::argument &barg);
         virtual void unbind(exec_context &ctx);

      private:
         size_t _storage = 0;
      };

      class constant_argument : public argument {
      public:
         constant_argument();

         virtual void set(size_t size, const void *value);
         virtual void bind(exec_context &ctx,
                           const binary::argument &barg);
         virtual void unbind(exec_context &ctx);

      private:
         buffer *buf;
         pipe_surface *st;
      };

      class image_argument : public argument {
      public:
         const image *get() const {
            return img;
         }
      protected:
         image *img;
      };

      class image_rd_argument : public image_argument {
      public:
         image_rd_argument();

         virtual void set(size_t size, const void *value);
         virtual void bind(exec_context &ctx,
                           const binary::argument &barg);
         virtual void unbind(exec_context &ctx);

      private:
         pipe_sampler_view *st;
      };

      class image_wr_argument : public image_argument {
      public:
         virtual void set(size_t size, const void *value);
         virtual void bind(exec_context &ctx,
                           const binary::argument &barg);
         virtual void unbind(exec_context &ctx);
      };

      class sampler_argument : public argument {
      public:
         sampler_argument();

         virtual void set(size_t size, const void *value);
         virtual void bind(exec_context &ctx,
                           const binary::argument &barg);
         virtual void unbind(exec_context &ctx);

      private:
         sampler *s;
         void *st;
      };

      std::vector<std::unique_ptr<argument>> _args;
      std::map<device *, std::unique_ptr<root_buffer> > _constant_buffers;
      std::string _name;
      exec_context exec;
      const ref_holder program_ref;
   };
}

#endif
