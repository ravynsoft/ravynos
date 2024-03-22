/**************************************************************************
 *
 * Copyright 2012-2021 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 **************************************************************************/

/*
 * State.h --
 *    State declarations.
 */


#include "DriverIncludes.h"
#include "util/u_hash_table.h"
#include "cso_cache/cso_context.h"

#define SUPPORT_MSAA 0
#define SUPPORT_D3D10_1 0
#define SUPPORT_D3D11 0


struct Adapter
{
   struct pipe_screen *screen;
};


static inline Adapter *
CastAdapter(D3D10DDI_HADAPTER hAdapter)
{
   return static_cast<Adapter *>(hAdapter.pDrvPrivate);
}

struct Shader
{
   void *handle;
   uint type;
   struct pipe_shader_state state;
   unsigned output_mapping[PIPE_MAX_SHADER_OUTPUTS];
   bool output_resolved;
};

struct Query;
struct ElementLayout;

struct Device
{
   struct pipe_context *pipe;

   struct cso_context *cso;
   struct pipe_framebuffer_state fb;
   struct pipe_vertex_buffer vertex_buffers[PIPE_MAX_ATTRIBS];
   unsigned vertex_strides[PIPE_MAX_ATTRIBS];
   struct pipe_resource *index_buffer;
   unsigned restart_index;
   unsigned index_size;
   unsigned ib_offset;
   void *samplers[PIPE_SHADER_TYPES][PIPE_MAX_SAMPLERS];
   struct pipe_sampler_view *sampler_views[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_SAMPLER_VIEWS];

   void *empty_fs;
   void *empty_vs;

   enum mesa_prim primitive;

   struct pipe_stream_output_target *so_targets[PIPE_MAX_SO_BUFFERS];
   struct pipe_stream_output_target *draw_so_target;
   Shader *bound_empty_gs;
   Shader *bound_vs;

   unsigned max_dual_source_render_targets;

   D3D10DDI_HRTCORELAYER  hRTCoreLayer;

   HANDLE hDevice;
   HANDLE hContext;

   D3DDDI_DEVICECALLBACKS KTCallbacks;
   D3D10DDI_CORELAYER_DEVICECALLBACKS UMCallbacks;
   DXGI_DDI_BASE_CALLBACKS *pDXGIBaseCallbacks;

   INT LastEmittedQuerySeqNo;
   INT LastFinishedQuerySeqNo;

   Query *pPredicate;
   BOOL PredicateValue;

   ElementLayout *element_layout;
   BOOL velems_changed;
};


static inline Device *
CastDevice(D3D10DDI_HDEVICE hDevice)
{
   return static_cast<Device *>(hDevice.pDrvPrivate);
}


static inline struct pipe_context *
CastPipeContext(D3D10DDI_HDEVICE hDevice)
{
   Device *pDevice = CastDevice(hDevice);
   return pDevice ? pDevice->pipe : NULL;
}


static inline Device *
CastDevice(DXGI_DDI_HDEVICE hDevice)
{
   return reinterpret_cast<Device *>(hDevice);
}


static inline struct pipe_context *
CastPipeDevice(DXGI_DDI_HDEVICE hDevice)
{
   Device *pDevice = CastDevice(hDevice);
   return pDevice ? pDevice->pipe : NULL;
}


static inline void
SetError(D3D10DDI_HDEVICE hDevice, HRESULT hr)
{
   if (FAILED(hr)) {
      Device *pDevice = CastDevice(hDevice);
      pDevice->UMCallbacks.pfnSetErrorCb(pDevice->hRTCoreLayer, hr);
   }
}


struct Resource
{
   DXGI_FORMAT Format;
   UINT MipLevels;
   UINT NumSubResources;
   bool buffer;
   struct pipe_resource *resource;
   struct pipe_transfer **transfers;
   struct pipe_stream_output_target *so_target;
};


static inline Resource *
CastResource(D3D10DDI_HRESOURCE hResource)
{
   return static_cast<Resource *>(hResource.pDrvPrivate);
}


static inline Resource *
CastResource(DXGI_DDI_HRESOURCE hResource)
{
   return reinterpret_cast<Resource *>(hResource);
}


static inline struct pipe_resource *
CastPipeResource(D3D10DDI_HRESOURCE hResource)
{
   Resource *pResource = CastResource(hResource);
   return pResource ? pResource->resource : NULL;
}


static inline struct pipe_resource *
CastPipeResource(DXGI_DDI_HRESOURCE hResource)
{
   Resource *pResource = CastResource(hResource);
   return pResource ? pResource->resource : NULL;
}


static inline struct pipe_resource *
CastPipeBuffer(D3D10DDI_HRESOURCE hResource)
{
   Resource *pResource = CastResource(hResource);
   if (!pResource) {
      return NULL;
   }
   return static_cast<struct pipe_resource *>(pResource->resource);
}


struct RenderTargetView
{
   struct pipe_surface *surface;
   D3D10DDI_HRTRENDERTARGETVIEW hRTRenderTargetView;
};


static inline RenderTargetView *
CastRenderTargetView(D3D10DDI_HRENDERTARGETVIEW hRenderTargetView)
{
   return static_cast<RenderTargetView *>(hRenderTargetView.pDrvPrivate);
}


static inline struct pipe_surface *
CastPipeRenderTargetView(D3D10DDI_HRENDERTARGETVIEW hRenderTargetView)
{
   RenderTargetView *pRenderTargetView = CastRenderTargetView(hRenderTargetView);
   return pRenderTargetView ? pRenderTargetView->surface : NULL;
}


struct DepthStencilView
{
   struct pipe_surface *surface;
   D3D10DDI_HRTDEPTHSTENCILVIEW hRTDepthStencilView;
};


static inline DepthStencilView *
CastDepthStencilView(D3D10DDI_HDEPTHSTENCILVIEW hDepthStencilView)
{
   return static_cast<DepthStencilView *>(hDepthStencilView.pDrvPrivate);
}


static inline struct pipe_surface *
CastPipeDepthStencilView(D3D10DDI_HDEPTHSTENCILVIEW hDepthStencilView)
{
   DepthStencilView *pDepthStencilView = CastDepthStencilView(hDepthStencilView);
   return pDepthStencilView ? pDepthStencilView->surface : NULL;
}


struct BlendState
{
   void *handle;
};


static inline BlendState *
CastBlendState(D3D10DDI_HBLENDSTATE hBlendState)
{
   return static_cast<BlendState *>(hBlendState.pDrvPrivate);
}


static inline void *
CastPipeBlendState(D3D10DDI_HBLENDSTATE hBlendState)
{
   BlendState *pBlendState = CastBlendState(hBlendState);
   return pBlendState ? pBlendState->handle : NULL;
}


struct DepthStencilState
{
   void *handle;
};


static inline DepthStencilState *
CastDepthStencilState(D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState)
{
   return static_cast<DepthStencilState *>(hDepthStencilState.pDrvPrivate);
}


static inline void *
CastPipeDepthStencilState(D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState)
{
   DepthStencilState *pDepthStencilState = CastDepthStencilState(hDepthStencilState);
   return pDepthStencilState ? pDepthStencilState->handle : NULL;
}


struct RasterizerState
{
   void *handle;
};


static inline RasterizerState *
CastRasterizerState(D3D10DDI_HRASTERIZERSTATE hRasterizerState)
{
   return static_cast<RasterizerState *>(hRasterizerState.pDrvPrivate);
}


static inline void *
CastPipeRasterizerState(D3D10DDI_HRASTERIZERSTATE hRasterizerState)
{
   RasterizerState *pRasterizerState = CastRasterizerState(hRasterizerState);
   return pRasterizerState ? pRasterizerState->handle : NULL;
}


static inline Shader *
CastShader(D3D10DDI_HSHADER hShader)
{
   return static_cast<Shader *>(hShader.pDrvPrivate);
}


static inline void *
CastPipeShader(D3D10DDI_HSHADER hShader)
{
   Shader *pShader = static_cast<Shader *>(hShader.pDrvPrivate);
   return pShader ? pShader->handle : NULL;
}


struct ElementLayout
{
   struct cso_velems_state state;
};


static inline ElementLayout *
CastElementLayout(D3D10DDI_HELEMENTLAYOUT hElementLayout)
{
   return static_cast<ElementLayout *>(hElementLayout.pDrvPrivate);
}

struct SamplerState
{
   void *handle;
};


static inline SamplerState *
CastSamplerState(D3D10DDI_HSAMPLER hSampler)
{
   return static_cast<SamplerState *>(hSampler.pDrvPrivate);
}


static inline void *
CastPipeSamplerState(D3D10DDI_HSAMPLER hSampler)
{
   SamplerState *pSamplerState = CastSamplerState(hSampler);
   return pSamplerState ? pSamplerState->handle : NULL;
}


struct ShaderResourceView
{
   struct pipe_sampler_view *handle;
};


static inline ShaderResourceView *
CastShaderResourceView(D3D10DDI_HSHADERRESOURCEVIEW hShaderResourceView)
{
   return static_cast<ShaderResourceView *>(hShaderResourceView.pDrvPrivate);
}


static inline struct pipe_sampler_view *
CastPipeShaderResourceView(D3D10DDI_HSHADERRESOURCEVIEW hShaderResourceView)
{
   ShaderResourceView *pSRView = CastShaderResourceView(hShaderResourceView);
   return pSRView ? pSRView->handle : NULL;
}


struct Query
{
   D3D10DDI_QUERY Type;
   UINT Flags;

   unsigned pipe_type;
   struct pipe_query *handle;
   INT SeqNo;
   UINT GetDataCount;

   D3D10_DDI_QUERY_DATA_PIPELINE_STATISTICS Statistics;
};


static inline Query *
CastQuery(D3D10DDI_HQUERY hQuery)
{
   return static_cast<Query *>(hQuery.pDrvPrivate);
}


static inline struct pipe_query *
CastPipeQuery(D3D10DDI_HQUERY hQuery)
{
   Query *pQuery = CastQuery(hQuery);
   return pQuery ? pQuery->handle : NULL;
}

