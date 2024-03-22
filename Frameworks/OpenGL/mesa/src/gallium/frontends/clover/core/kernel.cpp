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

#include "core/kernel.hpp"
#include "core/resource.hpp"
#include "util/factor.hpp"
#include "util/u_math.h"
#include "pipe/p_context.h"

using namespace clover;

kernel::kernel(clover::program &prog, const std::string &name,
               const std::vector<binary::argument> &bargs) :
   program(prog), _name(name), exec(*this),
   program_ref(prog._kernel_ref_counter) {
   for (auto &barg : bargs) {
      if (barg.semantic == binary::argument::general)
         _args.emplace_back(argument::create(barg));
   }
   for (auto &dev : prog.devices()) {
      auto &b = prog.build(dev).bin;
      auto bsym = find(name_equals(name), b.syms);
      const auto f = id_type_equals(bsym.section, binary::section::data_constant);
      if (!any_of(f, b.secs))
         continue;

      auto mconst = find(f, b.secs);
      auto rb = std::make_unique<root_buffer>(prog.context(), std::vector<cl_mem_properties>(),
                                              CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
                                              mconst.size, mconst.data.data());
      _constant_buffers.emplace(&dev, std::move(rb));
   }
}

template<typename V>
static inline std::vector<uint>
pad_vector(command_queue &q, const V &v, uint x) {
   std::vector<uint> w { v.begin(), v.end() };
   w.resize(q.device().max_block_size().size(), x);
   return w;
}

void
kernel::launch(command_queue &q,
               const std::vector<size_t> &grid_offset,
               const std::vector<size_t> &grid_size,
               const std::vector<size_t> &block_size) {
   const auto b = program().build(q.device()).bin;
   const auto reduced_grid_size =
      map(divides(), grid_size, block_size);

   if (any_of(is_zero(), grid_size))
      return;

   void *st = exec.bind(&q, grid_offset);
   struct pipe_grid_info info = {};

   // The handles are created during exec_context::bind(), so we need make
   // sure to call exec_context::bind() before retrieving them.
   std::vector<uint32_t *> g_handles = map([&](size_t h) {
         return (uint32_t *)&exec.input[h];
      }, exec.g_handles);

   q.pipe->bind_compute_state(q.pipe, st);
   q.pipe->bind_sampler_states(q.pipe, PIPE_SHADER_COMPUTE,
                               0, exec.samplers.size(),
                               exec.samplers.data());

   q.pipe->set_sampler_views(q.pipe, PIPE_SHADER_COMPUTE, 0,
                             exec.sviews.size(), 0, false, exec.sviews.data());
   q.pipe->set_shader_images(q.pipe, PIPE_SHADER_COMPUTE, 0,
                             exec.iviews.size(), 0, exec.iviews.data());
   q.pipe->set_compute_resources(q.pipe, 0, exec.resources.size(),
                                 exec.resources.data());
   q.pipe->set_global_binding(q.pipe, 0, exec.g_buffers.size(),
                              exec.g_buffers.data(), g_handles.data());

   // Fill information for the launch_grid() call.
   info.work_dim = grid_size.size();
   copy(pad_vector(q, block_size, 1), info.block);
   copy(pad_vector(q, reduced_grid_size, 1), info.grid);
   info.pc = find(name_equals(_name), b.syms).offset;
   info.input = exec.input.data();
   info.variable_shared_mem = exec.mem_local;

   q.pipe->launch_grid(q.pipe, &info);

   q.pipe->set_global_binding(q.pipe, 0, exec.g_buffers.size(), NULL, NULL);
   q.pipe->set_compute_resources(q.pipe, 0, exec.resources.size(), NULL);
   q.pipe->set_shader_images(q.pipe, PIPE_SHADER_COMPUTE, 0,
                             0, exec.iviews.size(), NULL);
   q.pipe->set_sampler_views(q.pipe, PIPE_SHADER_COMPUTE, 0,
                             0, exec.sviews.size(), false, NULL);
   q.pipe->bind_sampler_states(q.pipe, PIPE_SHADER_COMPUTE, 0,
                               exec.samplers.size(), NULL);

   q.pipe->memory_barrier(q.pipe, PIPE_BARRIER_GLOBAL_BUFFER);
   exec.unbind();
}

size_t
kernel::mem_local() const {
   size_t sz = 0;

   for (auto &arg : args()) {
      if (dynamic_cast<local_argument *>(&arg))
         sz += arg.storage();
   }

   return sz;
}

size_t
kernel::mem_private() const {
   return 0;
}

const std::string &
kernel::name() const {
   return _name;
}

std::vector<size_t>
kernel::optimal_block_size(const command_queue &q,
                           const std::vector<size_t> &grid_size) const {
   if (any_of(is_zero(), grid_size))
      return grid_size;

   return factor::find_grid_optimal_factor<size_t>(
      q.device().max_threads_per_block(), q.device().max_block_size(),
      grid_size);
}

std::vector<size_t>
kernel::required_block_size() const {
   return find(name_equals(_name), program().symbols()).reqd_work_group_size;
}

kernel::argument_range
kernel::args() {
   return map(derefs(), _args);
}

kernel::const_argument_range
kernel::args() const {
   return map(derefs(), _args);
}

std::vector<clover::binary::arg_info>
kernel::args_infos() {
   std::vector<clover::binary::arg_info> infos;
   for (auto &barg: find(name_equals(_name), program().symbols()).args)
      if (barg.semantic == clover::binary::argument::general)
         infos.emplace_back(barg.info);

   return infos;
}

const binary &
kernel::binary(const command_queue &q) const {
   return program().build(q.device()).bin;
}

kernel::exec_context::exec_context(kernel &kern) :
   kern(kern), q(NULL), print_handler(), mem_local(0), st(NULL), cs() {
}

kernel::exec_context::~exec_context() {
   if (st)
      q->pipe->delete_compute_state(q->pipe, st);
}

void *
kernel::exec_context::bind(intrusive_ptr<command_queue> _q,
                           const std::vector<size_t> &grid_offset) {
   std::swap(q, _q);

   // Bind kernel arguments.
   auto &b = kern.program().build(q->device()).bin;
   auto bsym = find(name_equals(kern.name()), b.syms);
   auto bargs = bsym.args;
   auto msec = find(id_type_equals(bsym.section, binary::section::text_executable), b.secs);
   auto explicit_arg = kern._args.begin();

   for (auto &barg : bargs) {
      switch (barg.semantic) {
      case binary::argument::general:
         (*(explicit_arg++))->bind(*this, barg);
         break;

      case binary::argument::grid_dimension: {
         const cl_uint dimension = grid_offset.size();
         auto arg = argument::create(barg);

         arg->set(sizeof(dimension), &dimension);
         arg->bind(*this, barg);
         break;
      }
      case binary::argument::grid_offset: {
         for (cl_uint x : pad_vector(*q, grid_offset, 0)) {
            auto arg = argument::create(barg);

            arg->set(sizeof(x), &x);
            arg->bind(*this, barg);
         }
         break;
      }
      case binary::argument::image_size: {
         auto img = dynamic_cast<image_argument &>(**(explicit_arg - 1)).get();
         std::vector<cl_uint> image_size{
               static_cast<cl_uint>(img->width()),
               static_cast<cl_uint>(img->height()),
               static_cast<cl_uint>(img->depth())};
         for (auto x : image_size) {
            auto arg = argument::create(barg);

            arg->set(sizeof(x), &x);
            arg->bind(*this, barg);
         }
         break;
      }
      case binary::argument::image_format: {
         auto img = dynamic_cast<image_argument &>(**(explicit_arg - 1)).get();
         cl_image_format fmt = img->format();
         std::vector<cl_uint> image_format{
               static_cast<cl_uint>(fmt.image_channel_data_type),
               static_cast<cl_uint>(fmt.image_channel_order)};
         for (auto x : image_format) {
            auto arg = argument::create(barg);

            arg->set(sizeof(x), &x);
            arg->bind(*this, barg);
         }
         break;
      }
      case binary::argument::constant_buffer: {
         auto arg = argument::create(barg);
         cl_mem buf = kern._constant_buffers.at(&q->device()).get();
         arg->set(sizeof(buf), &buf);
         arg->bind(*this, barg);
         break;
      }
      case binary::argument::printf_buffer: {
         print_handler = printf_handler::create(q, b.printf_infos,
                                                b.printf_strings_in_buffer,
                                                q->device().max_printf_buffer_size());
         cl_mem print_mem = print_handler->get_mem();

         auto arg = argument::create(barg);
         arg->set(sizeof(cl_mem), &print_mem);
         arg->bind(*this, barg);
         break;
      }
      }
   }

   // Create a new compute state if anything changed.
   if (!st || q != _q ||
       cs.req_input_mem != input.size()) {
      if (st)
         _q->pipe->delete_compute_state(_q->pipe, st);

      cs.ir_type = q->device().ir_format();
      cs.prog = &(msec.data[0]);
      // we only pass in NIRs or LLVMs and both IRs decode the size
      cs.static_shared_mem = 0;
      cs.req_input_mem = input.size();
      st = q->pipe->create_compute_state(q->pipe, &cs);
      if (!st) {
         unbind(); // Cleanup
         throw error(CL_OUT_OF_RESOURCES);
      }
   }

   return st;
}

void
kernel::exec_context::unbind() {
   if (print_handler)
      print_handler->print();

   for (auto &arg : kern.args())
      arg.unbind(*this);

   input.clear();
   samplers.clear();
   sviews.clear();
   iviews.clear();
   resources.clear();
   g_buffers.clear();
   g_handles.clear();
   mem_local = 0;
}

namespace {
   template<typename T>
   std::vector<uint8_t>
   bytes(const T& x) {
      return { (uint8_t *)&x, (uint8_t *)&x + sizeof(x) };
   }

   ///
   /// Transform buffer \a v from the native byte order into the byte
   /// order specified by \a e.
   ///
   template<typename T>
   void
   byteswap(T &v, pipe_endian e) {
      if (PIPE_ENDIAN_NATIVE != e)
         std::reverse(v.begin(), v.end());
   }

   ///
   /// Pad buffer \a v to the next multiple of \a n.
   ///
   template<typename T>
   void
   align_vector(T &v, size_t n) {
      v.resize(util_align_npot(v.size(), n));
   }

   bool
   msb(const std::vector<uint8_t> &s) {
      if (PIPE_ENDIAN_NATIVE == PIPE_ENDIAN_LITTLE)
         return s.back() & 0x80;
      else
         return s.front() & 0x80;
   }

   ///
   /// Resize buffer \a v to size \a n using sign or zero extension
   /// according to \a ext.
   ///
   template<typename T>
   void
   extend(T &v, enum binary::argument::ext_type ext, size_t n) {
      const size_t m = std::min(v.size(), n);
      const bool sign_ext = (ext == binary::argument::sign_ext);
      const uint8_t fill = (sign_ext && msb(v) ? ~0 : 0);
      T w(n, fill);

      if (PIPE_ENDIAN_NATIVE == PIPE_ENDIAN_LITTLE)
         std::copy_n(v.begin(), m, w.begin());
      else
         std::copy_n(v.end() - m, m, w.end() - m);

      std::swap(v, w);
   }

   ///
   /// Append buffer \a w to \a v.
   ///
   template<typename T>
   void
   insert(T &v, const T &w) {
      v.insert(v.end(), w.begin(), w.end());
   }

   ///
   /// Append \a n elements to the end of buffer \a v.
   ///
   template<typename T>
   size_t
   allocate(T &v, size_t n) {
      size_t pos = v.size();
      v.resize(pos + n);
      return pos;
   }
}

std::unique_ptr<kernel::argument>
kernel::argument::create(const binary::argument &barg) {
   switch (barg.type) {
   case binary::argument::scalar:
      return std::unique_ptr<kernel::argument>(new scalar_argument(barg.size));

   case binary::argument::global:
      return std::unique_ptr<kernel::argument>(new global_argument);

   case binary::argument::local:
      return std::unique_ptr<kernel::argument>(new local_argument);

   case binary::argument::constant:
      return std::unique_ptr<kernel::argument>(new constant_argument);

   case binary::argument::image_rd:
      return std::unique_ptr<kernel::argument>(new image_rd_argument);

   case binary::argument::image_wr:
      return std::unique_ptr<kernel::argument>(new image_wr_argument);

   case binary::argument::sampler:
      return std::unique_ptr<kernel::argument>(new sampler_argument);

   }
   throw error(CL_INVALID_KERNEL_DEFINITION);
}

kernel::argument::argument() : _set(false) {
}

bool
kernel::argument::set() const {
   return _set;
}

size_t
kernel::argument::storage() const {
   return 0;
}

kernel::scalar_argument::scalar_argument(size_t size) : size(size) {
}

void
kernel::scalar_argument::set(size_t size, const void *value) {
   if (!value)
      throw error(CL_INVALID_ARG_VALUE);

   if (size != this->size)
      throw error(CL_INVALID_ARG_SIZE);

   v = { (uint8_t *)value, (uint8_t *)value + size };
   _set = true;
}

void
kernel::scalar_argument::bind(exec_context &ctx,
                              const binary::argument &barg) {
   auto w = v;

   extend(w, barg.ext_type, barg.target_size);
   byteswap(w, ctx.q->device().endianness());
   align_vector(ctx.input, barg.target_align);
   insert(ctx.input, w);
}

void
kernel::scalar_argument::unbind(exec_context &ctx) {
}

kernel::global_argument::global_argument() : buf(nullptr), svm(nullptr) {
}

void
kernel::global_argument::set(size_t size, const void *value) {
   if (size != sizeof(cl_mem))
      throw error(CL_INVALID_ARG_SIZE);

   buf = pobj<buffer>(value ? *(cl_mem *)value : NULL);
   svm = nullptr;
   _set = true;
}

void
kernel::global_argument::set_svm(const void *value) {
   svm = value;
   buf = nullptr;
   _set = true;
}

void
kernel::global_argument::bind(exec_context &ctx,
                              const binary::argument &barg) {
   align_vector(ctx.input, barg.target_align);

   if (buf) {
      const resource &r = buf->resource_in(*ctx.q);
      ctx.g_handles.push_back(ctx.input.size());
      ctx.g_buffers.push_back(r.pipe);

      // How to handle multi-demensional offsets?
      // We don't need to.  Buffer offsets are always
      // one-dimensional.
      auto v = bytes(r.offset[0]);
      extend(v, barg.ext_type, barg.target_size);
      byteswap(v, ctx.q->device().endianness());
      insert(ctx.input, v);
   } else if (svm) {
      auto v = bytes(svm);
      extend(v, barg.ext_type, barg.target_size);
      byteswap(v, ctx.q->device().endianness());
      insert(ctx.input, v);
   } else {
      // Null pointer.
      allocate(ctx.input, barg.target_size);
   }
}

void
kernel::global_argument::unbind(exec_context &ctx) {
}

size_t
kernel::local_argument::storage() const {
   return _storage;
}

void
kernel::local_argument::set(size_t size, const void *value) {
   if (value)
      throw error(CL_INVALID_ARG_VALUE);

   if (!size)
      throw error(CL_INVALID_ARG_SIZE);

   _storage = size;
   _set = true;
}

void
kernel::local_argument::bind(exec_context &ctx,
                             const binary::argument &barg) {
   ctx.mem_local = ::align(ctx.mem_local, barg.target_align);
   auto v = bytes(ctx.mem_local);

   extend(v, binary::argument::zero_ext, barg.target_size);
   byteswap(v, ctx.q->device().endianness());
   align_vector(ctx.input, ctx.q->device().address_bits() / 8);
   insert(ctx.input, v);

   ctx.mem_local += _storage;
}

void
kernel::local_argument::unbind(exec_context &ctx) {
}

kernel::constant_argument::constant_argument() : buf(nullptr), st(nullptr) {
}

void
kernel::constant_argument::set(size_t size, const void *value) {
   if (size != sizeof(cl_mem))
      throw error(CL_INVALID_ARG_SIZE);

   buf = pobj<buffer>(value ? *(cl_mem *)value : NULL);
   _set = true;
}

void
kernel::constant_argument::bind(exec_context &ctx,
                                const binary::argument &barg) {
   align_vector(ctx.input, barg.target_align);

   if (buf) {
      resource &r = buf->resource_in(*ctx.q);
      auto v = bytes(ctx.resources.size() << 24 | r.offset[0]);

      extend(v, binary::argument::zero_ext, barg.target_size);
      byteswap(v, ctx.q->device().endianness());
      insert(ctx.input, v);

      st = r.bind_surface(*ctx.q, false);
      ctx.resources.push_back(st);
   } else {
      // Null pointer.
      allocate(ctx.input, barg.target_size);
   }
}

void
kernel::constant_argument::unbind(exec_context &ctx) {
   if (buf)
      buf->resource_in(*ctx.q).unbind_surface(*ctx.q, st);
}

kernel::image_rd_argument::image_rd_argument() : st(nullptr) {
}

void
kernel::image_rd_argument::set(size_t size, const void *value) {
   if (!value)
      throw error(CL_INVALID_ARG_VALUE);

   if (size != sizeof(cl_mem))
      throw error(CL_INVALID_ARG_SIZE);

   img = &obj<image>(*(cl_mem *)value);
   _set = true;
}

void
kernel::image_rd_argument::bind(exec_context &ctx,
                                const binary::argument &barg) {
   auto v = bytes(ctx.sviews.size());

   extend(v, binary::argument::zero_ext, barg.target_size);
   byteswap(v, ctx.q->device().endianness());
   align_vector(ctx.input, barg.target_align);
   insert(ctx.input, v);

   st = img->resource_in(*ctx.q).bind_sampler_view(*ctx.q);
   ctx.sviews.push_back(st);
}

void
kernel::image_rd_argument::unbind(exec_context &ctx) {
   img->resource_in(*ctx.q).unbind_sampler_view(*ctx.q, st);
}

void
kernel::image_wr_argument::set(size_t size, const void *value) {
   if (!value)
      throw error(CL_INVALID_ARG_VALUE);

   if (size != sizeof(cl_mem))
      throw error(CL_INVALID_ARG_SIZE);

   img = &obj<image>(*(cl_mem *)value);
   _set = true;
}

void
kernel::image_wr_argument::bind(exec_context &ctx,
                                const binary::argument &barg) {
   auto v = bytes(ctx.iviews.size());

   extend(v, binary::argument::zero_ext, barg.target_size);
   byteswap(v, ctx.q->device().endianness());
   align_vector(ctx.input, barg.target_align);
   insert(ctx.input, v);
   ctx.iviews.push_back(img->resource_in(*ctx.q).create_image_view(*ctx.q));
}

void
kernel::image_wr_argument::unbind(exec_context &ctx) {
}

kernel::sampler_argument::sampler_argument() : s(nullptr), st(nullptr) {
}

void
kernel::sampler_argument::set(size_t size, const void *value) {
   if (!value)
      throw error(CL_INVALID_SAMPLER);

   if (size != sizeof(cl_sampler))
      throw error(CL_INVALID_ARG_SIZE);

   s = &obj(*(cl_sampler *)value);
   _set = true;
}

void
kernel::sampler_argument::bind(exec_context &ctx,
                               const binary::argument &barg) {
   st = s->bind(*ctx.q);
   ctx.samplers.push_back(st);
}

void
kernel::sampler_argument::unbind(exec_context &ctx) {
   s->unbind(*ctx.q, st);
}
