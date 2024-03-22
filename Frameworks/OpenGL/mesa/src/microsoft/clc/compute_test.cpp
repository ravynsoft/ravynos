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

#include <unknwn.h>
#include <directx/d3d12.h>
#include <dxgi1_4.h>
#include <gtest/gtest.h>
#include <wrl.h>
#include <dxguids/dxguids.h>

#include "util/u_debug.h"
#include "clc_compiler.h"
#include "compute_test.h"
#include "dxil_validator.h"

#include <spirv-tools/libspirv.hpp>

#if (defined(_WIN32) && defined(_MSC_VER))
inline D3D12_CPU_DESCRIPTOR_HANDLE
GetCPUDescriptorHandleForHeapStart(ID3D12DescriptorHeap *heap)
{
   return heap->GetCPUDescriptorHandleForHeapStart();
}
inline D3D12_GPU_DESCRIPTOR_HANDLE
GetGPUDescriptorHandleForHeapStart(ID3D12DescriptorHeap *heap)
{
   return heap->GetGPUDescriptorHandleForHeapStart();
}
inline D3D12_HEAP_PROPERTIES
GetCustomHeapProperties(ID3D12Device *dev, D3D12_HEAP_TYPE type)
{
   return dev->GetCustomHeapProperties(0, type);
}
#else
inline D3D12_CPU_DESCRIPTOR_HANDLE
GetCPUDescriptorHandleForHeapStart(ID3D12DescriptorHeap *heap)
{
   D3D12_CPU_DESCRIPTOR_HANDLE ret;
   heap->GetCPUDescriptorHandleForHeapStart(&ret);
   return ret;
}
inline D3D12_GPU_DESCRIPTOR_HANDLE
GetGPUDescriptorHandleForHeapStart(ID3D12DescriptorHeap *heap)
{
   D3D12_GPU_DESCRIPTOR_HANDLE ret;
   heap->GetGPUDescriptorHandleForHeapStart(&ret);
   return ret;
}
inline D3D12_HEAP_PROPERTIES
GetCustomHeapProperties(ID3D12Device *dev, D3D12_HEAP_TYPE type)
{
   D3D12_HEAP_PROPERTIES ret;
   dev->GetCustomHeapProperties(&ret, 0, type);
   return ret;
}
#endif

using std::runtime_error;
using Microsoft::WRL::ComPtr;

enum compute_test_debug_flags {
   COMPUTE_DEBUG_EXPERIMENTAL_SHADERS = 1 << 0,
   COMPUTE_DEBUG_USE_HW_D3D           = 1 << 1,
   COMPUTE_DEBUG_OPTIMIZE_LIBCLC      = 1 << 2,
   COMPUTE_DEBUG_SERIALIZE_LIBCLC     = 1 << 3,
};

static const struct debug_named_value compute_debug_options[] = {
   { "experimental_shaders",  COMPUTE_DEBUG_EXPERIMENTAL_SHADERS, "Enable experimental shaders" },
   { "use_hw_d3d",            COMPUTE_DEBUG_USE_HW_D3D,           "Use a hardware D3D device"   },
   { "optimize_libclc",       COMPUTE_DEBUG_OPTIMIZE_LIBCLC,      "Optimize the clc_libclc before using it" },
   { "serialize_libclc",      COMPUTE_DEBUG_SERIALIZE_LIBCLC,     "Serialize and deserialize the clc_libclc" },
   DEBUG_NAMED_VALUE_END
};

DEBUG_GET_ONCE_FLAGS_OPTION(debug_compute, "COMPUTE_TEST_DEBUG", compute_debug_options, 0)

static void warning_callback(void *priv, const char *msg)
{
   fprintf(stderr, "WARNING: %s\n", msg);
}

static void error_callback(void *priv, const char *msg)
{
   fprintf(stderr, "ERROR: %s\n", msg);
}

static const struct clc_logger logger = {
   NULL,
   error_callback,
   warning_callback,
};

void
ComputeTest::enable_d3d12_debug_layer()
{
   HMODULE hD3D12Mod = LoadLibrary("D3D12.DLL");
   if (!hD3D12Mod) {
      fprintf(stderr, "D3D12: failed to load D3D12.DLL\n");
      return;
   }

   typedef HRESULT(WINAPI * PFN_D3D12_GET_DEBUG_INTERFACE)(REFIID riid,
                                                           void **ppFactory);
   PFN_D3D12_GET_DEBUG_INTERFACE D3D12GetDebugInterface = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(hD3D12Mod, "D3D12GetDebugInterface");
   if (!D3D12GetDebugInterface) {
      fprintf(stderr, "D3D12: failed to load D3D12GetDebugInterface from D3D12.DLL\n");
      return;
   }

   ID3D12Debug *debug;
   if (FAILED(D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void **)& debug))) {
      fprintf(stderr, "D3D12: D3D12GetDebugInterface failed\n");
      return;
   }

   debug->EnableDebugLayer();
}

IDXGIFactory4 *
ComputeTest::get_dxgi_factory()
{
   static const GUID IID_IDXGIFactory4 = {
      0x1bc6ea02, 0xef36, 0x464f,
      { 0xbf, 0x0c, 0x21, 0xca, 0x39, 0xe5, 0x16, 0x8a }
   };

   typedef HRESULT(WINAPI * PFN_CREATE_DXGI_FACTORY)(REFIID riid,
                                                     void **ppFactory);
   PFN_CREATE_DXGI_FACTORY CreateDXGIFactory;

   HMODULE hDXGIMod = LoadLibrary("DXGI.DLL");
   if (!hDXGIMod)
      throw runtime_error("Failed to load DXGI.DLL");

   CreateDXGIFactory = (PFN_CREATE_DXGI_FACTORY)GetProcAddress(hDXGIMod, "CreateDXGIFactory");
   if (!CreateDXGIFactory)
      throw runtime_error("Failed to load CreateDXGIFactory from DXGI.DLL");

   IDXGIFactory4 *factory = NULL;
   HRESULT hr = CreateDXGIFactory(IID_IDXGIFactory4, (void **)&factory);
   if (FAILED(hr))
      throw runtime_error("CreateDXGIFactory failed");

   return factory;
}

IDXGIAdapter1 *
ComputeTest::choose_adapter(IDXGIFactory4 *factory)
{
   IDXGIAdapter1 *ret;

   if (debug_get_option_debug_compute() & COMPUTE_DEBUG_USE_HW_D3D) {
      for (unsigned i = 0; SUCCEEDED(factory->EnumAdapters1(i, &ret)); i++) {
         DXGI_ADAPTER_DESC1 desc;
         ret->GetDesc1(&desc);
         if (!(desc.Flags & D3D_DRIVER_TYPE_SOFTWARE))
            return ret;
      }
      throw runtime_error("Failed to enum hardware adapter");
   } else {
      if (FAILED(factory->EnumWarpAdapter(__uuidof(IDXGIAdapter1),
         (void **)& ret)))
         throw runtime_error("Failed to enum warp adapter");
      return ret;
   }
}

ID3D12Device *
ComputeTest::create_device(IDXGIAdapter1 *adapter)
{
   typedef HRESULT(WINAPI *PFN_D3D12CREATEDEVICE)(IUnknown *, D3D_FEATURE_LEVEL, REFIID, void **);
   PFN_D3D12CREATEDEVICE D3D12CreateDevice;

   HMODULE hD3D12Mod = LoadLibrary("D3D12.DLL");
   if (!hD3D12Mod)
      throw runtime_error("failed to load D3D12.DLL");

   if (debug_get_option_debug_compute() & COMPUTE_DEBUG_EXPERIMENTAL_SHADERS) {
      typedef HRESULT(WINAPI *PFN_D3D12ENABLEEXPERIMENTALFEATURES)(UINT, const IID *, void *, UINT *);
      PFN_D3D12ENABLEEXPERIMENTALFEATURES D3D12EnableExperimentalFeatures;
      D3D12EnableExperimentalFeatures = (PFN_D3D12ENABLEEXPERIMENTALFEATURES)
         GetProcAddress(hD3D12Mod, "D3D12EnableExperimentalFeatures");
      if (FAILED(D3D12EnableExperimentalFeatures(1, &D3D12ExperimentalShaderModels, NULL, NULL)))
         throw runtime_error("failed to enable experimental shader models");
   }

   D3D12CreateDevice = (PFN_D3D12CREATEDEVICE)GetProcAddress(hD3D12Mod, "D3D12CreateDevice");
   if (!D3D12CreateDevice)
      throw runtime_error("failed to load D3D12CreateDevice from D3D12.DLL");

   ID3D12Device *dev;
   if (FAILED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0,
       __uuidof(ID3D12Device), (void **)& dev)))
      throw runtime_error("D3D12CreateDevice failed");

   return dev;
}

ComPtr<ID3D12RootSignature>
ComputeTest::create_root_signature(const ComputeTest::Resources &resources)
{
   D3D12_ROOT_PARAMETER1 root_param;
   root_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
   root_param.DescriptorTable.NumDescriptorRanges = resources.ranges.size();
   root_param.DescriptorTable.pDescriptorRanges = resources.ranges.data();
   root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

   D3D12_ROOT_SIGNATURE_DESC1 root_sig_desc;
   root_sig_desc.NumParameters = 1;
   root_sig_desc.pParameters = &root_param;
   root_sig_desc.NumStaticSamplers = 0;
   root_sig_desc.pStaticSamplers = NULL;
   root_sig_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

   D3D12_VERSIONED_ROOT_SIGNATURE_DESC versioned_desc;
   versioned_desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
   versioned_desc.Desc_1_1 = root_sig_desc;

   ID3DBlob *sig, *error;
   if (FAILED(D3D12SerializeVersionedRootSignature(&versioned_desc,
       &sig, &error)))
      throw runtime_error("D3D12SerializeVersionedRootSignature failed");

   ComPtr<ID3D12RootSignature> ret;
   if (FAILED(dev->CreateRootSignature(0,
       sig->GetBufferPointer(),
       sig->GetBufferSize(),
       __uuidof(ID3D12RootSignature),
       (void **)& ret)))
      throw runtime_error("CreateRootSignature failed");

   return ret;
}

ComPtr<ID3D12PipelineState>
ComputeTest::create_pipeline_state(ComPtr<ID3D12RootSignature> &root_sig,
                                   const struct clc_dxil_object &dxil)
{
   D3D12_COMPUTE_PIPELINE_STATE_DESC pipeline_desc = { root_sig.Get() };
   pipeline_desc.CS.pShaderBytecode = dxil.binary.data;
   pipeline_desc.CS.BytecodeLength = dxil.binary.size;

   ComPtr<ID3D12PipelineState> pipeline_state;
   if (FAILED(dev->CreateComputePipelineState(&pipeline_desc,
                                              __uuidof(ID3D12PipelineState),
                                              (void **)& pipeline_state)))
      throw runtime_error("Failed to create pipeline state");
   return pipeline_state;
}

ComPtr<ID3D12Resource>
ComputeTest::create_buffer(int size, D3D12_HEAP_TYPE heap_type)
{
   D3D12_RESOURCE_DESC desc;
   desc.Format = DXGI_FORMAT_UNKNOWN;
   desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
   desc.Width = size;
   desc.Height = 1;
   desc.DepthOrArraySize = 1;
   desc.MipLevels = 1;
   desc.SampleDesc.Count = 1;
   desc.SampleDesc.Quality = 0;
   desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
   desc.Flags = heap_type == D3D12_HEAP_TYPE_DEFAULT ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
   desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

   D3D12_HEAP_PROPERTIES heap_pris = GetCustomHeapProperties(dev, heap_type);

   ComPtr<ID3D12Resource> res;
   if (FAILED(dev->CreateCommittedResource(&heap_pris,
       D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON,
       NULL, __uuidof(ID3D12Resource), (void **)&res)))
      throw runtime_error("CreateCommittedResource failed");

   return res;
}

ComPtr<ID3D12Resource>
ComputeTest::create_upload_buffer_with_data(const void *data, size_t size)
{
   auto upload_res = create_buffer(size, D3D12_HEAP_TYPE_UPLOAD);

   void *ptr = NULL;
   D3D12_RANGE res_range = { 0, (SIZE_T)size };
   if (FAILED(upload_res->Map(0, &res_range, (void **)&ptr)))
      throw runtime_error("Failed to map upload-buffer");
   assert(ptr);
   memcpy(ptr, data, size);
   upload_res->Unmap(0, &res_range);
   return upload_res;
}

ComPtr<ID3D12Resource>
ComputeTest::create_sized_buffer_with_data(size_t buffer_size,
                                           const void *data,
                                           size_t data_size)
{
   auto upload_res = create_upload_buffer_with_data(data, data_size);

   auto res = create_buffer(buffer_size, D3D12_HEAP_TYPE_DEFAULT);
   resource_barrier(res, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
   cmdlist->CopyBufferRegion(res.Get(), 0, upload_res.Get(), 0, data_size);
   resource_barrier(res, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
   execute_cmdlist();

   return res;
}

void
ComputeTest::get_buffer_data(ComPtr<ID3D12Resource> res,
                             void *buf, size_t size)
{
   auto readback_res = create_buffer(align(size, 4), D3D12_HEAP_TYPE_READBACK);
   resource_barrier(res, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE);
   cmdlist->CopyResource(readback_res.Get(), res.Get());
   resource_barrier(res, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
   execute_cmdlist();

   void *ptr = NULL;
   D3D12_RANGE res_range = { 0, size };
   if (FAILED(readback_res->Map(0, &res_range, &ptr)))
      throw runtime_error("Failed to map readback-buffer");

   memcpy(buf, ptr, size);

   D3D12_RANGE empty_range = { 0, 0 };
   readback_res->Unmap(0, &empty_range);
}

void
ComputeTest::resource_barrier(ComPtr<ID3D12Resource> &res,
                              D3D12_RESOURCE_STATES state_before,
                              D3D12_RESOURCE_STATES state_after)
{
   D3D12_RESOURCE_BARRIER barrier;
   barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
   barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
   barrier.Transition.pResource = res.Get();
   barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
   barrier.Transition.StateBefore = state_before;
   barrier.Transition.StateAfter = state_after;
   cmdlist->ResourceBarrier(1, &barrier);
}

void
ComputeTest::execute_cmdlist()
{
   if (FAILED(cmdlist->Close()))
      throw runtime_error("Closing ID3D12GraphicsCommandList failed");

   ID3D12CommandList *cmdlists[] = { cmdlist };
   cmdqueue->ExecuteCommandLists(1, cmdlists);
   cmdqueue_fence->SetEventOnCompletion(fence_value, event);
   cmdqueue->Signal(cmdqueue_fence, fence_value);
   fence_value++;
   WaitForSingleObject(event, INFINITE);

   if (FAILED(cmdalloc->Reset()))
      throw runtime_error("resetting ID3D12CommandAllocator failed");

   if (FAILED(cmdlist->Reset(cmdalloc, NULL)))
      throw runtime_error("resetting ID3D12GraphicsCommandList failed");
}

void
ComputeTest::create_uav_buffer(ComPtr<ID3D12Resource> res,
                               size_t width, size_t byte_stride,
                               D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle)
{
   D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc;
   uav_desc.Format = DXGI_FORMAT_R32_TYPELESS;
   uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
   uav_desc.Buffer.FirstElement = 0;
   uav_desc.Buffer.NumElements = DIV_ROUND_UP(width * byte_stride, 4);
   uav_desc.Buffer.StructureByteStride = 0;
   uav_desc.Buffer.CounterOffsetInBytes = 0;
   uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

   dev->CreateUnorderedAccessView(res.Get(), NULL, &uav_desc, cpu_handle);
}

void
ComputeTest::create_cbv(ComPtr<ID3D12Resource> res, size_t size,
                        D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle)
{
   D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc;
   cbv_desc.BufferLocation = res ? res->GetGPUVirtualAddress() : 0;
   cbv_desc.SizeInBytes = size;

   dev->CreateConstantBufferView(&cbv_desc, cpu_handle);
}

ComPtr<ID3D12Resource>
ComputeTest::add_uav_resource(ComputeTest::Resources &resources,
                              unsigned spaceid, unsigned resid,
                              const void *data, size_t num_elems,
                              size_t elem_size)
{
   size_t size = align(elem_size * num_elems, 4);
   D3D12_CPU_DESCRIPTOR_HANDLE handle;
   ComPtr<ID3D12Resource> res;
   handle = GetCPUDescriptorHandleForHeapStart(uav_heap);
   handle = offset_cpu_handle(handle, resources.descs.size() * uav_heap_incr);

   if (size) {
      if (data)
         res = create_buffer_with_data(data, size);
      else
         res = create_buffer(size, D3D12_HEAP_TYPE_DEFAULT);

      resource_barrier(res, D3D12_RESOURCE_STATE_COMMON,
                       D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
   }
   create_uav_buffer(res, num_elems, elem_size, handle);
   resources.add(res, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, spaceid, resid);
   return res;
}

ComPtr<ID3D12Resource>
ComputeTest::add_cbv_resource(ComputeTest::Resources &resources,
                              unsigned spaceid, unsigned resid,
                              const void *data, size_t size)
{
   unsigned aligned_size = align(size, 256);
   D3D12_CPU_DESCRIPTOR_HANDLE handle;
   ComPtr<ID3D12Resource> res;
   handle = GetCPUDescriptorHandleForHeapStart(uav_heap);
   handle = offset_cpu_handle(handle, resources.descs.size() * uav_heap_incr);

   if (size) {
     assert(data);
     res = create_sized_buffer_with_data(aligned_size, data, size);
   }
   create_cbv(res, aligned_size, handle);
   resources.add(res, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, spaceid, resid);
   return res;
}

void
ComputeTest::run_shader_with_raw_args(Shader shader,
                                      const CompileArgs &compile_args,
                                      const std::vector<RawShaderArg *> &args)
{
   if (args.size() < 1)
      throw runtime_error("no inputs");

   static HMODULE hD3D12Mod = LoadLibrary("D3D12.DLL");
   if (!hD3D12Mod)
      throw runtime_error("Failed to load D3D12.DLL");

   D3D12SerializeVersionedRootSignature = (PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE)GetProcAddress(hD3D12Mod, "D3D12SerializeVersionedRootSignature");

   if (args.size() != shader.dxil->kernel->num_args)
      throw runtime_error("incorrect number of inputs");

   struct clc_runtime_kernel_conf conf = { 0 };

   // Older WARP and some hardware doesn't support int64, so for these tests, unconditionally lower away int64
   // A more complex runtime can be smarter about detecting when this needs to be done
   conf.lower_bit_size = 64;
   conf.max_shader_model = SHADER_MODEL_6_2;
   conf.validator_version = DXIL_VALIDATOR_1_4;

   if (!shader.dxil->metadata.local_size[0])
      conf.local_size[0] = compile_args.x;
   else
      conf.local_size[0] = shader.dxil->metadata.local_size[0];

   if (!shader.dxil->metadata.local_size[1])
      conf.local_size[1] = compile_args.y;
   else
      conf.local_size[1] = shader.dxil->metadata.local_size[1];

   if (!shader.dxil->metadata.local_size[2])
      conf.local_size[2] = compile_args.z;
   else
      conf.local_size[2] = shader.dxil->metadata.local_size[2];

   if (compile_args.x % conf.local_size[0] ||
       compile_args.y % conf.local_size[1] ||
       compile_args.z % conf.local_size[2])
      throw runtime_error("invalid global size must be a multiple of local size");

   std::vector<struct clc_runtime_arg_info> argsinfo(args.size());

   conf.args = argsinfo.data();
   conf.support_global_work_id_offsets =
      compile_args.work_props.global_offset_x != 0 ||
      compile_args.work_props.global_offset_y != 0 ||
      compile_args.work_props.global_offset_z != 0;
   conf.support_workgroup_id_offsets =
      compile_args.work_props.group_id_offset_x != 0 ||
      compile_args.work_props.group_id_offset_y != 0 ||
      compile_args.work_props.group_id_offset_z != 0;

   for (unsigned i = 0; i < shader.dxil->kernel->num_args; ++i) {
      RawShaderArg *arg = args[i];
      size_t size = arg->get_elem_size() * arg->get_num_elems();

      switch (shader.dxil->kernel->args[i].address_qualifier) {
      case CLC_KERNEL_ARG_ADDRESS_LOCAL:
         argsinfo[i].localptr.size = size;
         break;
      default:
         break;
      }
   }

   configure(shader, &conf);
   validate(shader);

   std::shared_ptr<struct clc_dxil_object> &dxil = shader.dxil;

   std::vector<uint8_t> argsbuf(dxil->metadata.kernel_inputs_buf_size);
   std::vector<ComPtr<ID3D12Resource>> argres(shader.dxil->kernel->num_args);
   clc_work_properties_data work_props = compile_args.work_props;
   if (!conf.support_workgroup_id_offsets) {
      work_props.group_count_total_x = compile_args.x / conf.local_size[0];
      work_props.group_count_total_y = compile_args.y / conf.local_size[1];
      work_props.group_count_total_z = compile_args.z / conf.local_size[2];
   }
   if (work_props.work_dim == 0)
      work_props.work_dim = 3;
   Resources resources;

   for (unsigned i = 0; i < dxil->kernel->num_args; ++i) {
      RawShaderArg *arg = args[i];
      size_t size = arg->get_elem_size() * arg->get_num_elems();
      void *slot = argsbuf.data() + dxil->metadata.args[i].offset;

      switch (dxil->kernel->args[i].address_qualifier) {
      case CLC_KERNEL_ARG_ADDRESS_CONSTANT:
      case CLC_KERNEL_ARG_ADDRESS_GLOBAL: {
         assert(dxil->metadata.args[i].size == sizeof(uint64_t));
         uint64_t *ptr_slot = (uint64_t *)slot;
         if (arg->get_data())
            *ptr_slot = (uint64_t)dxil->metadata.args[i].globconstptr.buf_id << 32;
         else
            *ptr_slot = ~0ull;
         break;
      }
      case CLC_KERNEL_ARG_ADDRESS_LOCAL: {
         assert(dxil->metadata.args[i].size == sizeof(uint64_t));
         uint64_t *ptr_slot = (uint64_t *)slot;
         *ptr_slot = dxil->metadata.args[i].localptr.sharedmem_offset;
         break;
      }
      case CLC_KERNEL_ARG_ADDRESS_PRIVATE: {
         assert(size == dxil->metadata.args[i].size);
         memcpy(slot, arg->get_data(), size);
         break;
      }
      default:
         assert(0);
      }
   }

   for (unsigned i = 0; i < dxil->kernel->num_args; ++i) {
      RawShaderArg *arg = args[i];

      if (dxil->kernel->args[i].address_qualifier == CLC_KERNEL_ARG_ADDRESS_GLOBAL ||
          dxil->kernel->args[i].address_qualifier == CLC_KERNEL_ARG_ADDRESS_CONSTANT) {
         argres[i] = add_uav_resource(resources, 0,
                                      dxil->metadata.args[i].globconstptr.buf_id,
                                      arg->get_data(), arg->get_num_elems(),
                                      arg->get_elem_size());
      }
   }

   if (dxil->metadata.printf.uav_id > 0)
      add_uav_resource(resources, 0, dxil->metadata.printf.uav_id, NULL, 1024 * 1024 / 4, 4);

   for (unsigned i = 0; i < dxil->metadata.num_consts; ++i)
      add_uav_resource(resources, 0, dxil->metadata.consts[i].uav_id,
                       dxil->metadata.consts[i].data,
                       dxil->metadata.consts[i].size / 4, 4);

   if (argsbuf.size())
      add_cbv_resource(resources, 0, dxil->metadata.kernel_inputs_cbv_id,
                       argsbuf.data(), argsbuf.size());

   add_cbv_resource(resources, 0, dxil->metadata.work_properties_cbv_id,
                    &work_props, sizeof(work_props));

   auto root_sig = create_root_signature(resources);
   auto pipeline_state = create_pipeline_state(root_sig, *dxil);

   cmdlist->SetDescriptorHeaps(1, &uav_heap);
   cmdlist->SetComputeRootSignature(root_sig.Get());
   cmdlist->SetComputeRootDescriptorTable(0, GetGPUDescriptorHandleForHeapStart(uav_heap));
   cmdlist->SetPipelineState(pipeline_state.Get());

   cmdlist->Dispatch(compile_args.x / conf.local_size[0],
                     compile_args.y / conf.local_size[1],
                     compile_args.z / conf.local_size[2]);

   for (auto &range : resources.ranges) {
      if (range.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_UAV) {
         for (unsigned i = range.OffsetInDescriptorsFromTableStart;
              i < range.NumDescriptors; i++) {
            if (!resources.descs[i].Get())
               continue;

            resource_barrier(resources.descs[i],
                             D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                             D3D12_RESOURCE_STATE_COMMON);
         }
      }
   }

   execute_cmdlist();

   for (unsigned i = 0; i < args.size(); i++) {
      if (!(args[i]->get_direction() & SHADER_ARG_OUTPUT))
         continue;

      assert(dxil->kernel->args[i].address_qualifier == CLC_KERNEL_ARG_ADDRESS_GLOBAL);
      get_buffer_data(argres[i], args[i]->get_data(),
                      args[i]->get_elem_size() * args[i]->get_num_elems());
   }

   ComPtr<ID3D12InfoQueue> info_queue;
   dev->QueryInterface(info_queue.ReleaseAndGetAddressOf());
   if (info_queue)
   {
      EXPECT_EQ(0, info_queue->GetNumStoredMessages());
      for (unsigned i = 0; i < info_queue->GetNumStoredMessages(); ++i) {
         SIZE_T message_size = 0;
         info_queue->GetMessageA(i, nullptr, &message_size);
         D3D12_MESSAGE* message = (D3D12_MESSAGE*)malloc(message_size);
         info_queue->GetMessageA(i, message, &message_size);
         FAIL() << message->pDescription;
         free(message);
      }
   }
}

void
ComputeTest::SetUp()
{
   static struct clc_libclc *compiler_ctx_g = nullptr;

   if (!compiler_ctx_g) {
      clc_libclc_dxil_options options = { };
      options.optimize = (debug_get_option_debug_compute() & COMPUTE_DEBUG_OPTIMIZE_LIBCLC) != 0;

      compiler_ctx_g = clc_libclc_new_dxil(&logger, &options);
      if (!compiler_ctx_g)
         throw runtime_error("failed to create CLC compiler context");

      if (debug_get_option_debug_compute() & COMPUTE_DEBUG_SERIALIZE_LIBCLC) {
         void *serialized = nullptr;
         size_t serialized_size = 0;
         clc_libclc_serialize(compiler_ctx_g, &serialized, &serialized_size);
         if (!serialized)
            throw runtime_error("failed to serialize CLC compiler context");

         clc_free_libclc(compiler_ctx_g);
         compiler_ctx_g = nullptr;

         compiler_ctx_g = clc_libclc_deserialize(serialized, serialized_size);
         if (!compiler_ctx_g)
            throw runtime_error("failed to deserialize CLC compiler context");

         clc_libclc_free_serialized(serialized);
      }
   }
   compiler_ctx = compiler_ctx_g;

   enable_d3d12_debug_layer();

   factory = get_dxgi_factory();
   if (!factory)
      throw runtime_error("failed to create DXGI factory");

   adapter = choose_adapter(factory);
   if (!adapter)
      throw runtime_error("failed to choose adapter");

   dev = create_device(adapter);
   if (!dev)
      throw runtime_error("failed to create device");

   if (FAILED(dev->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                               __uuidof(cmdqueue_fence),
                               (void **)&cmdqueue_fence)))
      throw runtime_error("failed to create fence\n");

   D3D12_COMMAND_QUEUE_DESC queue_desc;
   queue_desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
   queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
   queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
   queue_desc.NodeMask = 0;
   if (FAILED(dev->CreateCommandQueue(&queue_desc,
                                      __uuidof(cmdqueue),
                                      (void **)&cmdqueue)))
      throw runtime_error("failed to create command queue");

   if (FAILED(dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE,
             __uuidof(cmdalloc), (void **)&cmdalloc)))
      throw runtime_error("failed to create command allocator");

   if (FAILED(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE,
             cmdalloc, NULL, __uuidof(cmdlist), (void **)&cmdlist)))
      throw runtime_error("failed to create command list");

   D3D12_DESCRIPTOR_HEAP_DESC heap_desc;
   heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
   heap_desc.NumDescriptors = 1000;
   heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
   heap_desc.NodeMask = 0;
   if (FAILED(dev->CreateDescriptorHeap(&heap_desc,
       __uuidof(uav_heap), (void **)&uav_heap)))
      throw runtime_error("failed to create descriptor heap");

   uav_heap_incr = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

   event = CreateEvent(NULL, false, false, NULL);
   if (!event)
      throw runtime_error("Failed to create event");
   fence_value = 1;
}

void
ComputeTest::TearDown()
{
   CloseHandle(event);

   uav_heap->Release();
   cmdlist->Release();
   cmdalloc->Release();
   cmdqueue->Release();
   cmdqueue_fence->Release();
   dev->Release();
   adapter->Release();
   factory->Release();
}

PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE ComputeTest::D3D12SerializeVersionedRootSignature;

bool
validate_module(const struct clc_dxil_object &dxil)
{
   struct dxil_validator *val = dxil_create_validator(NULL);
   char *err;
   bool res = dxil_validate_module(val, dxil.binary.data,
                                   dxil.binary.size, &err);
   if (!res && err)
      fprintf(stderr, "D3D12: validation failed: %s", err);

   dxil_destroy_validator(val);
   return res;
}

static void
dump_blob(const char *path, const struct clc_dxil_object &dxil)
{
   FILE *fp = fopen(path, "wb");
   if (fp) {
      fwrite(dxil.binary.data, 1, dxil.binary.size, fp);
      fclose(fp);
      printf("D3D12: wrote '%s'...\n", path);
   }
}

ComputeTest::Shader
ComputeTest::compile(const std::vector<const char *> &sources,
                     const std::vector<const char *> &compile_args,
                     bool create_library)
{
   struct clc_compile_args args = {
   };
   args.args = compile_args.data();
   args.num_args = (unsigned)compile_args.size();
   args.features.images = true;
   args.features.images_read_write = true;
   args.features.int64 = true;
   ComputeTest::Shader shader;

   std::vector<Shader> shaders;

   args.source.name = "obj.cl";

   for (unsigned i = 0; i < sources.size(); i++) {
      args.source.value = sources[i];

      clc_binary spirv{};
      if (!clc_compile_c_to_spirv(&args, &logger, &spirv))
         throw runtime_error("failed to compile object!");

      Shader shader;
      shader.obj = std::shared_ptr<clc_binary>(new clc_binary(spirv), [](clc_binary *spirv)
         {
            clc_free_spirv(spirv);
            delete spirv;
         });
      shaders.push_back(shader);
   }

   if (shaders.size() == 1 && create_library)
      return shaders[0];

   return link(shaders, create_library);
}

ComputeTest::Shader
ComputeTest::link(const std::vector<Shader> &sources,
                  bool create_library)
{
   std::vector<const clc_binary*> objs;
   for (auto& source : sources)
      objs.push_back(&*source.obj);

   struct clc_linker_args link_args = {};
   link_args.in_objs = objs.data();
   link_args.num_in_objs = (unsigned)objs.size();
   link_args.create_library = create_library;
   clc_binary spirv{};
   if (!clc_link_spirv(&link_args, &logger, &spirv))
      throw runtime_error("failed to link objects!");

   ComputeTest::Shader shader;
   shader.obj = std::shared_ptr<clc_binary>(new clc_binary(spirv), [](clc_binary *spirv)
      {
         clc_free_spirv(spirv);
         delete spirv;
      });
   if (!link_args.create_library)
      configure(shader, NULL);

   return shader;
}

ComputeTest::Shader
ComputeTest::assemble(const char *source)
{
   spvtools::SpirvTools tools(SPV_ENV_UNIVERSAL_1_0);
   std::vector<uint32_t> binary;
   if (!tools.Assemble(source, strlen(source), &binary))
      throw runtime_error("failed to assemble");

   ComputeTest::Shader shader;
   shader.obj = std::shared_ptr<clc_binary>(new clc_binary{}, [](clc_binary *spirv)
      {
         free(spirv->data);
         delete spirv;
      });
   shader.obj->size = binary.size() * 4;
   shader.obj->data = malloc(shader.obj->size);
   memcpy(shader.obj->data, binary.data(), shader.obj->size);

   configure(shader, NULL);

   return shader;
}

void
ComputeTest::configure(Shader &shader,
                       const struct clc_runtime_kernel_conf *conf)
{
   if (!shader.metadata) {
      shader.metadata = std::shared_ptr<clc_parsed_spirv>(new clc_parsed_spirv{}, [](clc_parsed_spirv *metadata)
         {
            clc_free_parsed_spirv(metadata);
            delete metadata;
         });
      if (!clc_parse_spirv(shader.obj.get(), NULL, shader.metadata.get()))
         throw runtime_error("failed to parse spirv!");
   }

   std::unique_ptr<clc_dxil_object> dxil(new clc_dxil_object{});
   if (!clc_spirv_to_dxil(compiler_ctx, shader.obj.get(), shader.metadata.get(), "main_test", conf, nullptr, &logger, dxil.get()))
      throw runtime_error("failed to compile kernel!");
   shader.dxil = std::shared_ptr<clc_dxil_object>(dxil.release(), [](clc_dxil_object *dxil)
      {
         clc_free_dxil_object(dxil);
         delete dxil;
      });
}

void
ComputeTest::validate(ComputeTest::Shader &shader)
{
   dump_blob("unsigned.cso", *shader.dxil);
   if (!validate_module(*shader.dxil))
      throw runtime_error("failed to validate module!");

   dump_blob("signed.cso", *shader.dxil);
}
