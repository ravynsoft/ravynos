/*
 * Copyright © Microsoft Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdexcept>

#include <directx/d3d12.h>
#include <dxgi1_4.h>
#include <gtest/gtest.h>
#include <wrl.h>

#include "clc_compiler.h"

using std::runtime_error;
using Microsoft::WRL::ComPtr;

inline D3D12_CPU_DESCRIPTOR_HANDLE
offset_cpu_handle(D3D12_CPU_DESCRIPTOR_HANDLE handle, UINT offset)
{
   handle.ptr += offset;
   return handle;
}

inline size_t
align(size_t value, unsigned alignment)
{
   assert(alignment > 0);
   return ((value + (alignment - 1)) / alignment) * alignment;
}

class ComputeTest : public ::testing::Test {
protected:
   struct Shader {
      std::shared_ptr<struct clc_binary> obj;
      std::shared_ptr<struct clc_parsed_spirv> metadata;
      std::shared_ptr<struct clc_dxil_object> dxil;
   };

   static void
   enable_d3d12_debug_layer();

   static IDXGIFactory4 *
   get_dxgi_factory();

   static IDXGIAdapter1 *
   choose_adapter(IDXGIFactory4 *factory);

   static ID3D12Device *
   create_device(IDXGIAdapter1 *adapter);

   struct Resources {
      void add(ComPtr<ID3D12Resource> res,
               D3D12_DESCRIPTOR_RANGE_TYPE type,
               unsigned spaceid,
               unsigned resid)
      {
         descs.push_back(res);

         if(!ranges.empty() &&
            ranges.back().RangeType == type &&
            ranges.back().RegisterSpace == spaceid &&
            ranges.back().BaseShaderRegister + ranges.back().NumDescriptors == resid) {
            ranges.back().NumDescriptors++;
            return;
         }

         D3D12_DESCRIPTOR_RANGE1 range;

         range.RangeType = type;
         range.NumDescriptors = 1;
         range.BaseShaderRegister = resid;
         range.RegisterSpace = spaceid;
         range.OffsetInDescriptorsFromTableStart = descs.size() - 1;
         range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS;
         ranges.push_back(range);
      }

      std::vector<D3D12_DESCRIPTOR_RANGE1> ranges;
      std::vector<ComPtr<ID3D12Resource>> descs;
   };

   ComPtr<ID3D12RootSignature>
   create_root_signature(const Resources &resources);

   ComPtr<ID3D12PipelineState>
   create_pipeline_state(ComPtr<ID3D12RootSignature> &root_sig,
                         const struct clc_dxil_object &dxil);

   ComPtr<ID3D12Resource>
   create_buffer(int size, D3D12_HEAP_TYPE heap_type);

   ComPtr<ID3D12Resource>
   create_upload_buffer_with_data(const void *data, size_t size);

   ComPtr<ID3D12Resource>
   create_sized_buffer_with_data(size_t buffer_size, const void *data,
                                 size_t data_size);

   ComPtr<ID3D12Resource>
   create_buffer_with_data(const void *data, size_t size)
   {
      return create_sized_buffer_with_data(size, data, size);
   }

   void
   get_buffer_data(ComPtr<ID3D12Resource> res,
                   void *buf, size_t size);

   void
   resource_barrier(ComPtr<ID3D12Resource> &res,
                    D3D12_RESOURCE_STATES state_before,
                    D3D12_RESOURCE_STATES state_after);

   void
   execute_cmdlist();

   void
   create_uav_buffer(ComPtr<ID3D12Resource> res,
                     size_t width, size_t byte_stride,
                     D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle);

   void create_cbv(ComPtr<ID3D12Resource> res, size_t size,
                   D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle);

   ComPtr<ID3D12Resource>
   add_uav_resource(Resources &resources, unsigned spaceid, unsigned resid,
                    const void *data = NULL, size_t num_elems = 0,
                    size_t elem_size = 0);

   ComPtr<ID3D12Resource>
   add_cbv_resource(Resources &resources, unsigned spaceid, unsigned resid,
                    const void *data, size_t size);

   void
   SetUp() override;

   void
   TearDown() override;

   Shader
   compile(const std::vector<const char *> &sources,
           const std::vector<const char *> &compile_args = {},
           bool create_library = false);

   Shader
   link(const std::vector<Shader> &sources,
        bool create_library = false);

   Shader
   assemble(const char *source);

   void
   configure(Shader &shader,
             const struct clc_runtime_kernel_conf *conf);

   void
   validate(Shader &shader);

   template <typename T>
   Shader
   specialize(Shader &shader, uint32_t id, T const& val)
   {
      Shader new_shader;
      new_shader.obj = std::shared_ptr<clc_binary>(new clc_binary{}, [](clc_binary *spirv)
         {
            clc_free_spirv(spirv);
            delete spirv;
         });
      if (!shader.metadata)
         configure(shader, NULL);

      clc_spirv_specialization spec;
      spec.id = id;
      memcpy(&spec.value, &val, sizeof(val));
      clc_spirv_specialization_consts consts;
      consts.specializations = &spec;
      consts.num_specializations = 1;
      if (!clc_specialize_spirv(shader.obj.get(), shader.metadata.get(), &consts, new_shader.obj.get()))
         throw runtime_error("failed to specialize");

      configure(new_shader, NULL);

      return new_shader;
   }

   enum ShaderArgDirection {
      SHADER_ARG_INPUT = 1,
      SHADER_ARG_OUTPUT = 2,
      SHADER_ARG_INOUT = SHADER_ARG_INPUT | SHADER_ARG_OUTPUT,
   };

   class RawShaderArg {
   public:
      RawShaderArg(enum ShaderArgDirection dir) : dir(dir) { }
      virtual size_t get_elem_size() const = 0;
      virtual size_t get_num_elems() const = 0;
      virtual const void *get_data() const = 0;
      virtual void *get_data() = 0;
      enum ShaderArgDirection get_direction() { return dir; }
   private:
      enum ShaderArgDirection dir;
   };

   class NullShaderArg : public RawShaderArg {
   public:
      NullShaderArg() : RawShaderArg(SHADER_ARG_INPUT) { }
      size_t get_elem_size() const override { return 0; }
      size_t get_num_elems() const override { return 0; }
      const void *get_data() const override { return NULL; }
      void *get_data() override { return NULL; }
   };

   template <typename T>
   class ShaderArg : public std::vector<T>, public RawShaderArg
   {
   public:
      ShaderArg(const T &v, enum ShaderArgDirection dir = SHADER_ARG_INOUT) :
         std::vector<T>({ v }), RawShaderArg(dir) { }
      ShaderArg(const std::vector<T> &v, enum ShaderArgDirection dir = SHADER_ARG_INOUT) :
         std::vector<T>(v), RawShaderArg(dir) { }
      ShaderArg(const std::initializer_list<T> v, enum ShaderArgDirection dir = SHADER_ARG_INOUT) :
         std::vector<T>(v), RawShaderArg(dir) { }

      ShaderArg<T>& operator =(const T &v)
      {
         this->clear();
         this->push_back(v);
         return *this;
      }

      operator T&() { return this->at(0); }
      operator const T&() const { return this->at(0); }

      ShaderArg<T>& operator =(const std::vector<T> &v)
      {
         *this = v;
         return *this;
      }

      ShaderArg<T>& operator =(std::initializer_list<T> v)
      {
         *this = v;
         return *this;
      }

      size_t get_elem_size() const override { return sizeof(T); }
      size_t get_num_elems() const override { return this->size(); }
      const void *get_data() const override { return this->data(); }
      void *get_data() override { return this->data(); }
   };

   struct CompileArgs
   {
      unsigned x, y, z;
      std::vector<const char *> compiler_command_line;
      clc_work_properties_data work_props;
   };

private:
   void gather_args(std::vector<RawShaderArg *> &args) { }

   template <typename T, typename... Rest>
   void gather_args(std::vector<RawShaderArg *> &args, T &arg, Rest&... rest)
   {
      args.push_back(&arg);
      gather_args(args, rest...);
   }

   void run_shader_with_raw_args(Shader shader,
                                 const CompileArgs &compile_args,
                                 const std::vector<RawShaderArg *> &args);

protected:
   template <typename... Args>
   void run_shader(Shader shader,
                   const CompileArgs &compile_args,
                   Args&... args)
   {
      std::vector<RawShaderArg *> raw_args;
      gather_args(raw_args, args...);
      run_shader_with_raw_args(shader, compile_args, raw_args);
   }

   template <typename... Args>
   void run_shader(const std::vector<const char *> &sources,
                   unsigned x, unsigned y, unsigned z,
                   Args&... args)
   {
      std::vector<RawShaderArg *> raw_args;
      gather_args(raw_args, args...);
      CompileArgs compile_args = { x, y, z };
      run_shader_with_raw_args(compile(sources), compile_args, raw_args);
   }

   template <typename... Args>
   void run_shader(const std::vector<const char *> &sources,
                   const CompileArgs &compile_args,
                   Args&... args)
   {
      std::vector<RawShaderArg *> raw_args;
      gather_args(raw_args, args...);
      run_shader_with_raw_args(
         compile(sources, compile_args.compiler_command_line),
         compile_args, raw_args);
   }

   template <typename... Args>
   void run_shader(const char *source,
                   unsigned x, unsigned y, unsigned z,
                   Args&... args)
   {
      std::vector<RawShaderArg *> raw_args;
      gather_args(raw_args, args...);
      CompileArgs compile_args = { x, y, z };
      run_shader_with_raw_args(compile({ source }), compile_args, raw_args);
   }

   IDXGIFactory4 *factory;
   IDXGIAdapter1 *adapter;
   ID3D12Device *dev;
   ID3D12Fence *cmdqueue_fence;
   ID3D12CommandQueue *cmdqueue;
   ID3D12CommandAllocator *cmdalloc;
   ID3D12GraphicsCommandList *cmdlist;
   ID3D12DescriptorHeap *uav_heap;

   struct clc_libclc *compiler_ctx;

   UINT uav_heap_incr;
   int fence_value;

   HANDLE event;
   static PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE D3D12SerializeVersionedRootSignature;
};
