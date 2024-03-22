/* SPDX-License-Identifier: GPL-2.0 OR MIT */
/*
 * Copyright 2012-2022 VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

/*
 * svga3d_dx.h --
 *
 *    SVGA 3d hardware definitions for DX10 support.
 */





#ifndef _SVGA3D_DX_H_
#define _SVGA3D_DX_H_






#include "svga_reg.h"
#include "svga3d_limits.h"
#include "svga3d_types.h"


#define SVGA3D_INPUT_MIN               0
#define SVGA3D_INPUT_PER_VERTEX_DATA   0
#define SVGA3D_INPUT_PER_INSTANCE_DATA 1
#define SVGA3D_INPUT_MAX               2
typedef uint32 SVGA3dInputClassification;


#define SVGA3D_RESOURCE_TYPE_MIN      1
#define SVGA3D_RESOURCE_BUFFER        1
#define SVGA3D_RESOURCE_TEXTURE1D     2
#define SVGA3D_RESOURCE_TEXTURE2D     3
#define SVGA3D_RESOURCE_TEXTURE3D     4
#define SVGA3D_RESOURCE_TEXTURECUBE   5
#define SVGA3D_RESOURCE_TYPE_DX10_MAX 6
#define SVGA3D_RESOURCE_BUFFEREX      6
#define SVGA3D_RESOURCE_TYPE_MAX      7
typedef uint32 SVGA3dResourceType;


#define SVGA3D_COLOR_WRITE_ENABLE_RED     (1 << 0)
#define SVGA3D_COLOR_WRITE_ENABLE_GREEN   (1 << 1)
#define SVGA3D_COLOR_WRITE_ENABLE_BLUE    (1 << 2)
#define SVGA3D_COLOR_WRITE_ENABLE_ALPHA   (1 << 3)
#define SVGA3D_COLOR_WRITE_ENABLE_ALL     (SVGA3D_COLOR_WRITE_ENABLE_RED |   \
                                           SVGA3D_COLOR_WRITE_ENABLE_GREEN | \
                                           SVGA3D_COLOR_WRITE_ENABLE_BLUE |  \
                                           SVGA3D_COLOR_WRITE_ENABLE_ALPHA)
typedef uint8 SVGA3dColorWriteEnable;


#define SVGA3D_DEPTH_WRITE_MASK_ZERO   0
#define SVGA3D_DEPTH_WRITE_MASK_ALL    1
typedef uint8 SVGA3dDepthWriteMask;


#define SVGA3D_FILTER_MIP_LINEAR  (1 << 0)
#define SVGA3D_FILTER_MAG_LINEAR  (1 << 2)
#define SVGA3D_FILTER_MIN_LINEAR  (1 << 4)
#define SVGA3D_FILTER_ANISOTROPIC (1 << 6)
#define SVGA3D_FILTER_COMPARE     (1 << 7)
typedef uint32 SVGA3dFilter;


#define SVGA3D_CULL_INVALID 0
#define SVGA3D_CULL_MIN     1
#define SVGA3D_CULL_NONE    1
#define SVGA3D_CULL_FRONT   2
#define SVGA3D_CULL_BACK    3
#define SVGA3D_CULL_MAX     4
typedef uint8 SVGA3dCullMode;


#define SVGA3D_COMPARISON_INVALID         0
#define SVGA3D_COMPARISON_MIN             1
#define SVGA3D_COMPARISON_NEVER           1
#define SVGA3D_COMPARISON_LESS            2
#define SVGA3D_COMPARISON_EQUAL           3
#define SVGA3D_COMPARISON_LESS_EQUAL      4
#define SVGA3D_COMPARISON_GREATER         5
#define SVGA3D_COMPARISON_NOT_EQUAL       6
#define SVGA3D_COMPARISON_GREATER_EQUAL   7
#define SVGA3D_COMPARISON_ALWAYS          8
#define SVGA3D_COMPARISON_MAX             9
typedef uint8 SVGA3dComparisonFunc;


#define SVGA3D_MULTISAMPLE_RAST_DISABLE        0
#define SVGA3D_MULTISAMPLE_RAST_ENABLE         1
#define SVGA3D_MULTISAMPLE_RAST_DX_MAX         1
#define SVGA3D_MULTISAMPLE_RAST_DISABLE_LINE   2
#define SVGA3D_MULTISAMPLE_RAST_MAX            2
typedef uint8 SVGA3dMultisampleRastEnable;

#define SVGA3D_DX_MAX_VERTEXBUFFERS 32
#define SVGA3D_DX_MAX_VERTEXINPUTREGISTERS 16
#define SVGA3D_DX_SM41_MAX_VERTEXINPUTREGISTERS 32
#define SVGA3D_DX_MAX_SOTARGETS 4
#define SVGA3D_DX_MAX_SRVIEWS 128
#define SVGA3D_DX_MAX_CONSTBUFFERS 16
#define SVGA3D_DX_MAX_SAMPLERS 16
#define SVGA3D_DX_MAX_CLASS_INSTANCES 253

#define SVGA3D_DX_MAX_CONSTBUF_BINDING_SIZE (4096 * 4 * (uint32)sizeof(uint32))

typedef uint32 SVGA3dShaderResourceViewId;
typedef uint32 SVGA3dRenderTargetViewId;
typedef uint32 SVGA3dDepthStencilViewId;
typedef uint32 SVGA3dUAViewId;

typedef uint32 SVGA3dShaderId;
typedef uint32 SVGA3dElementLayoutId;
typedef uint32 SVGA3dSamplerId;
typedef uint32 SVGA3dBlendStateId;
typedef uint32 SVGA3dDepthStencilStateId;
typedef uint32 SVGA3dRasterizerStateId;
typedef uint32 SVGA3dQueryId;
typedef uint32 SVGA3dStreamOutputId;

typedef union {
   struct {
      float r;
      float g;
      float b;
      float a;
   };

   float value[4];
} SVGA3dRGBAFloat;

typedef union {
   struct {
      uint32 r;
      uint32 g;
      uint32 b;
      uint32 a;
   };

   uint32 value[4];
} SVGA3dRGBAUint32;

typedef
#include "vmware_pack_begin.h"
struct {
   uint32 cid;
   SVGAMobId mobid;
}
#include "vmware_pack_end.h"
SVGAOTableDXContextEntry;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDefineContext {
   uint32 cid;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDefineContext;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDestroyContext {
   uint32 cid;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDestroyContext;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXBindContext {
   uint32 cid;
   SVGAMobId mobid;
   uint32 validContents;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXBindContext;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXReadbackContext {
   uint32 cid;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXReadbackContext;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXInvalidateContext {
   uint32 cid;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXInvalidateContext;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetSingleConstantBuffer {
   uint32 slot;
   SVGA3dShaderType type;
   SVGA3dSurfaceId sid;
   uint32 offsetInBytes;
   uint32 sizeInBytes;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetSingleConstantBuffer;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetShaderResources {
   uint32 startView;
   SVGA3dShaderType type;


}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetShaderResources;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetShader {
   SVGA3dShaderId shaderId;
   SVGA3dShaderType type;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetShader;


typedef union {
   struct {
      uint32 cbOffset : 12;
      uint32 cbId     : 4;
      uint32 baseSamp : 4;
      uint32 baseTex  : 7;
      uint32 reserved : 5;
   };
   uint32 value;
} SVGA3dIfaceData;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetShaderIface {
   SVGA3dShaderType type;
   uint32 numClassInstances;
   uint32 index;
   uint32 iface;
   SVGA3dIfaceData data;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetShaderIface;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXBindShaderIface {
   uint32 cid;
   SVGAMobId mobid;
   uint32 offsetInBytes;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXBindShaderIface;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetSamplers {
   uint32 startSampler;
   SVGA3dShaderType type;


}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetSamplers;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDraw {
   uint32 vertexCount;
   uint32 startVertexLocation;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDraw;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDrawIndexed {
   uint32 indexCount;
   uint32 startIndexLocation;
   int32  baseVertexLocation;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDrawIndexed;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDrawInstanced {
   uint32 vertexCountPerInstance;
   uint32 instanceCount;
   uint32 startVertexLocation;
   uint32 startInstanceLocation;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDrawInstanced;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDrawIndexedInstanced {
   uint32 indexCountPerInstance;
   uint32 instanceCount;
   uint32 startIndexLocation;
   int32  baseVertexLocation;
   uint32 startInstanceLocation;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDrawIndexedInstanced;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDrawIndexedInstancedIndirect {
   SVGA3dSurfaceId argsBufferSid;
   uint32 byteOffsetForArgs;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDrawIndexedInstancedIndirect;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDrawInstancedIndirect {
   SVGA3dSurfaceId argsBufferSid;
   uint32 byteOffsetForArgs;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDrawInstancedIndirect;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDrawAuto {
   uint32 pad0;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDrawAuto;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDispatch {
   uint32 threadGroupCountX;
   uint32 threadGroupCountY;
   uint32 threadGroupCountZ;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDispatch;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDispatchIndirect {
   SVGA3dSurfaceId argsBufferSid;
   uint32 byteOffsetForArgs;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDispatchIndirect;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetInputLayout {
   SVGA3dElementLayoutId elementLayoutId;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetInputLayout;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dVertexBuffer {
   SVGA3dSurfaceId sid;
   uint32 stride;
   uint32 offset;
}
#include "vmware_pack_end.h"
SVGA3dVertexBuffer;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetVertexBuffers {
   uint32 startBuffer;

}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetVertexBuffers;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dVertexBuffer_v2 {
   SVGA3dSurfaceId sid;
   uint32 stride;
   uint32 offset;
   uint32 sizeInBytes;
}
#include "vmware_pack_end.h"
SVGA3dVertexBuffer_v2;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetVertexBuffers_v2 {
   uint32 startBuffer;

}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetVertexBuffers_v2;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dVertexBufferOffsetAndSize {
   uint32 stride;
   uint32 offset;
   uint32 sizeInBytes;
}
#include "vmware_pack_end.h"
SVGA3dVertexBufferOffsetAndSize;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetVertexBuffersOffsetAndSize {
   uint32 startBuffer;

}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetVertexBuffersOffsetAndSize;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetIndexBuffer {
   SVGA3dSurfaceId sid;
   SVGA3dSurfaceFormat format;
   uint32 offset;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetIndexBuffer;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetIndexBuffer_v2 {
   SVGA3dSurfaceId sid;
   SVGA3dSurfaceFormat format;
   uint32 offset;
   uint32 sizeInBytes;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetIndexBuffer_v2;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetIndexBufferOffsetAndSize {
   SVGA3dSurfaceFormat format;
   uint32 offset;
   uint32 sizeInBytes;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetIndexBufferOffsetAndSize;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetTopology {
   SVGA3dPrimitiveType topology;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetTopology;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetRenderTargets {
   SVGA3dDepthStencilViewId depthStencilViewId;

}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetRenderTargets;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetBlendState {
   SVGA3dBlendStateId blendId;
   float blendFactor[4];
   uint32 sampleMask;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetBlendState;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetDepthStencilState {
   SVGA3dDepthStencilStateId depthStencilId;
   uint32 stencilRef;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetDepthStencilState;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetRasterizerState {
   SVGA3dRasterizerStateId rasterizerId;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetRasterizerState;


#define SVGA3D_DXQUERY_FLAG_PREDICATEHINT (1 << 0)
typedef uint32 SVGA3dDXQueryFlags;


#define SVGADX_QDSTATE_INVALID   ((uint8)-1)
#define SVGADX_QDSTATE_MIN       0
#define SVGADX_QDSTATE_IDLE      0
#define SVGADX_QDSTATE_ACTIVE    1
#define SVGADX_QDSTATE_PENDING   2
#define SVGADX_QDSTATE_FINISHED  3
#define SVGADX_QDSTATE_MAX       4
typedef uint8 SVGADXQueryDeviceState;

typedef
#include "vmware_pack_begin.h"
struct {
   SVGA3dQueryTypeUint8 type;
   uint16 pad0;
   SVGADXQueryDeviceState state;
   SVGA3dDXQueryFlags flags;
   SVGAMobId mobid;
   uint32 offset;
}
#include "vmware_pack_end.h"
SVGACOTableDXQueryEntry;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDefineQuery {
   SVGA3dQueryId queryId;
   SVGA3dQueryType type;
   SVGA3dDXQueryFlags flags;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDefineQuery;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDestroyQuery {
   SVGA3dQueryId queryId;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDestroyQuery;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXBindQuery {
   SVGA3dQueryId queryId;
   SVGAMobId mobid;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXBindQuery;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetQueryOffset {
   SVGA3dQueryId queryId;
   uint32 mobOffset;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetQueryOffset;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXBeginQuery {
   SVGA3dQueryId queryId;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXBeginQuery;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXEndQuery {
   SVGA3dQueryId queryId;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXEndQuery;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXReadbackQuery {
   SVGA3dQueryId queryId;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXReadbackQuery;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXMoveQuery {
   SVGA3dQueryId queryId;
   SVGAMobId mobid;
   uint32 mobOffset;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXMoveQuery;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXBindAllQuery {
   uint32 cid;
   SVGAMobId mobid;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXBindAllQuery;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXReadbackAllQuery {
   uint32 cid;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXReadbackAllQuery;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetPredication {
   SVGA3dQueryId queryId;
   uint32 predicateValue;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetPredication;

typedef
#include "vmware_pack_begin.h"
struct MKS3dDXSOState {
   uint32 offset;
   uint32 intOffset;
   uint32 vertexCount;
   uint32 dead;
}
#include "vmware_pack_end.h"
SVGA3dDXSOState;


#define SVGA3D_DX_SO_OFFSET_APPEND ((uint32) ~0u)

typedef
#include "vmware_pack_begin.h"
struct SVGA3dSoTarget {
   SVGA3dSurfaceId sid;
   uint32 offset;
   uint32 sizeInBytes;
}
#include "vmware_pack_end.h"
SVGA3dSoTarget;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetSOTargets {
   uint32 pad0;

}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetSOTargets;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dViewport
{
   float x;
   float y;
   float width;
   float height;
   float minDepth;
   float maxDepth;
}
#include "vmware_pack_end.h"
SVGA3dViewport;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetViewports {
   uint32 pad0;

}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetViewports;

#define SVGA3D_DX_MAX_VIEWPORTS  16

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetScissorRects {
   uint32 pad0;

}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetScissorRects;

#define SVGA3D_DX_MAX_SCISSORRECTS  16

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXClearRenderTargetView {
   SVGA3dRenderTargetViewId renderTargetViewId;
   SVGA3dRGBAFloat rgba;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXClearRenderTargetView;



typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXClearDepthStencilView {
   uint16 flags;
   uint16 stencil;
   SVGA3dDepthStencilViewId depthStencilViewId;
   float depth;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXClearDepthStencilView;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXPredCopyRegion {
   SVGA3dSurfaceId dstSid;
   uint32 dstSubResource;
   SVGA3dSurfaceId srcSid;
   uint32 srcSubResource;
   SVGA3dCopyBox box;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXPredCopyRegion;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXPredStagingCopyRegion {
   SVGA3dSurfaceId dstSid;
   uint32 dstSubResource;
   SVGA3dSurfaceId srcSid;
   uint32 srcSubResource;
   SVGA3dCopyBox box;
   uint8 readback;
   uint8 unsynchronized;
   uint8 mustBeZero[2];
}
#include "vmware_pack_end.h"
SVGA3dCmdDXPredStagingCopyRegion;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXPredCopy {
   SVGA3dSurfaceId dstSid;
   SVGA3dSurfaceId srcSid;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXPredCopy;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXPredConvertRegion {
   SVGA3dSurfaceId dstSid;
   uint32 dstSubResource;
   SVGA3dBox destBox;
   SVGA3dSurfaceId srcSid;
   uint32 srcSubResource;
   SVGA3dBox srcBox;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXPredConvertRegion;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXPredStagingConvertRegion {
   SVGA3dSurfaceId dstSid;
   uint32 dstSubResource;
   SVGA3dBox destBox;
   SVGA3dSurfaceId srcSid;
   uint32 srcSubResource;
   SVGA3dBox srcBox;
   uint8 readback;
   uint8 unsynchronized;
   uint8 mustBeZero[2];
}
#include "vmware_pack_end.h"
SVGA3dCmdDXPredStagingConvertRegion;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXPredConvert {
   SVGA3dSurfaceId dstSid;
   SVGA3dSurfaceId srcSid;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXPredConvert;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXPredStagingConvert {
   SVGA3dSurfaceId dstSid;
   SVGA3dSurfaceId srcSid;
   uint8 readback;
   uint8 unsynchronized;
   uint8 mustBeZero[2];
}
#include "vmware_pack_end.h"
SVGA3dCmdDXPredStagingConvert;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXBufferCopy {
   SVGA3dSurfaceId dest;
   SVGA3dSurfaceId src;
   uint32 destX;
   uint32 srcX;
   uint32 width;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXBufferCopy;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXStagingBufferCopy {
   SVGA3dSurfaceId dest;
   SVGA3dSurfaceId src;
   uint32 destX;
   uint32 srcX;
   uint32 width;
   uint8 readback;
   uint8 unsynchronized;
   uint8 mustBeZero[2];
}
#include "vmware_pack_end.h"
SVGA3dCmdDXStagingBufferCopy;



typedef
#include "vmware_pack_begin.h"
struct {
   SVGA3dSurfaceId dstSid;
   uint32 dstSubResource;
   SVGA3dSurfaceId srcSid;
   uint32 srcSubResource;
   SVGA3dSurfaceFormat copyFormat;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXResolveCopy;


typedef
#include "vmware_pack_begin.h"
struct {
   SVGA3dSurfaceId dstSid;
   uint32 dstSubResource;
   SVGA3dSurfaceId srcSid;
   uint32 srcSubResource;
   SVGA3dSurfaceFormat copyFormat;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXPredResolveCopy;

typedef uint32 SVGA3dDXPresentBltMode;
#define SVGADX_PRESENTBLT_LINEAR           (1 << 0)
#define SVGADX_PRESENTBLT_FORCE_SRC_SRGB   (1 << 1)
#define SVGADX_PRESENTBLT_FORCE_SRC_XRBIAS (1 << 2)
#define SVGADX_PRESENTBLT_MODE_MAX         (1 << 3)

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXPresentBlt {
   SVGA3dSurfaceId srcSid;
   uint32 srcSubResource;
   SVGA3dSurfaceId dstSid;
   uint32 destSubResource;
   SVGA3dBox boxSrc;
   SVGA3dBox boxDest;
   SVGA3dDXPresentBltMode mode;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXPresentBlt;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXGenMips {
   SVGA3dShaderResourceViewId shaderResourceViewId;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXGenMips;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXUpdateSubResource {
   SVGA3dSurfaceId sid;
   uint32 subResource;
   SVGA3dBox box;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXUpdateSubResource;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXReadbackSubResource {
   SVGA3dSurfaceId sid;
   uint32 subResource;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXReadbackSubResource;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXInvalidateSubResource {
   SVGA3dSurfaceId sid;
   uint32 subResource;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXInvalidateSubResource;



typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXTransferFromBuffer {
   SVGA3dSurfaceId srcSid;
   uint32 srcOffset;
   uint32 srcPitch;
   uint32 srcSlicePitch;
   SVGA3dSurfaceId destSid;
   uint32 destSubResource;
   SVGA3dBox destBox;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXTransferFromBuffer;


#define SVGA3D_TRANSFER_TO_BUFFER_READBACK   (1 << 0)
#define SVGA3D_TRANSFER_TO_BUFFER_FLAGS_MASK (1 << 0)
typedef uint32 SVGA3dTransferToBufferFlags;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXTransferToBuffer {
   SVGA3dSurfaceId srcSid;
   uint32 srcSubResource;
   SVGA3dBox srcBox;
   SVGA3dSurfaceId destSid;
   uint32 destOffset;
   uint32 destPitch;
   uint32 destSlicePitch;
   SVGA3dTransferToBufferFlags flags;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXTransferToBuffer;



typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXPredTransferFromBuffer {
   SVGA3dSurfaceId srcSid;
   uint32 srcOffset;
   uint32 srcPitch;
   uint32 srcSlicePitch;
   SVGA3dSurfaceId destSid;
   uint32 destSubResource;
   SVGA3dBox destBox;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXPredTransferFromBuffer;



typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSurfaceCopyAndReadback {
   SVGA3dSurfaceId srcSid;
   SVGA3dSurfaceId destSid;
   SVGA3dCopyBox box;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSurfaceCopyAndReadback;



typedef uint32 SVGADXHintId;
#define SVGA_DX_HINT_NONE              0
#define SVGA_DX_HINT_PREFETCH_OBJECT   1
#define SVGA_DX_HINT_PREEVICT_OBJECT   2
#define SVGA_DX_HINT_PREFETCH_COBJECT  3
#define SVGA_DX_HINT_PREEVICT_COBJECT  4
#define SVGA_DX_HINT_MAX               5

typedef
#include "vmware_pack_begin.h"
struct SVGAObjectRef {
   SVGAOTableType type;
   uint32 id;
}
#include "vmware_pack_end.h"
SVGAObjectRef;

typedef
#include "vmware_pack_begin.h"
struct SVGACObjectRef {
   SVGACOTableType type;
   uint32 cid;
   uint32 id;
}
#include "vmware_pack_end.h"
SVGACObjectRef;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXHint {
   SVGADXHintId hintId;


}
#include "vmware_pack_end.h"
SVGA3dCmdDXHint;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXBufferUpdate {
   SVGA3dSurfaceId sid;
   uint32 x;
   uint32 width;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXBufferUpdate;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetConstantBufferOffset {
   uint32 slot;
   uint32 offsetInBytes;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetConstantBufferOffset;

typedef SVGA3dCmdDXSetConstantBufferOffset SVGA3dCmdDXSetVSConstantBufferOffset;


typedef SVGA3dCmdDXSetConstantBufferOffset SVGA3dCmdDXSetPSConstantBufferOffset;


typedef SVGA3dCmdDXSetConstantBufferOffset SVGA3dCmdDXSetGSConstantBufferOffset;


typedef SVGA3dCmdDXSetConstantBufferOffset SVGA3dCmdDXSetHSConstantBufferOffset;


typedef SVGA3dCmdDXSetConstantBufferOffset SVGA3dCmdDXSetDSConstantBufferOffset;


typedef SVGA3dCmdDXSetConstantBufferOffset SVGA3dCmdDXSetCSConstantBufferOffset;



#define SVGA3D_BUFFEREX_SRV_RAW        (1 << 0)
#define SVGA3D_BUFFEREX_SRV_FLAGS_MAX  (1 << 1)
#define SVGA3D_BUFFEREX_SRV_FLAGS_MASK (SVGA3D_BUFFEREX_SRV_FLAGS_MAX - 1)
typedef uint32 SVGA3dBufferExFlags;

typedef
#include "vmware_pack_begin.h"
struct {
   union {
      struct {
         uint32 firstElement;
         uint32 numElements;
         uint32 pad0;
         uint32 pad1;
      } buffer;
      struct {
         uint32 mostDetailedMip;
         uint32 firstArraySlice;
         uint32 mipLevels;
         uint32 arraySize;
      } tex;
      struct {
         uint32 firstElement;
         uint32 numElements;
         SVGA3dBufferExFlags flags;
         uint32 pad0;
      } bufferex;
   };
}
#include "vmware_pack_end.h"
SVGA3dShaderResourceViewDesc;

typedef
#include "vmware_pack_begin.h"
struct {
   SVGA3dSurfaceId sid;
   SVGA3dSurfaceFormat format;
   SVGA3dResourceType resourceDimension;
   SVGA3dShaderResourceViewDesc desc;
   uint32 pad;
}
#include "vmware_pack_end.h"
SVGACOTableDXSRViewEntry;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDefineShaderResourceView {
   SVGA3dShaderResourceViewId shaderResourceViewId;

   SVGA3dSurfaceId sid;
   SVGA3dSurfaceFormat format;
   SVGA3dResourceType resourceDimension;

   SVGA3dShaderResourceViewDesc desc;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDefineShaderResourceView;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDestroyShaderResourceView {
   SVGA3dShaderResourceViewId shaderResourceViewId;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDestroyShaderResourceView;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dRenderTargetViewDesc {
   union {
      struct {
         uint32 firstElement;
         uint32 numElements;
         uint32 padding0;
      } buffer;
      struct {
         uint32 mipSlice;
         uint32 firstArraySlice;
         uint32 arraySize;
      } tex;
      struct {
         uint32 mipSlice;
         uint32 firstW;
         uint32 wSize;
      } tex3D;
   };
}
#include "vmware_pack_end.h"
SVGA3dRenderTargetViewDesc;

typedef
#include "vmware_pack_begin.h"
struct {
   SVGA3dSurfaceId sid;
   SVGA3dSurfaceFormat format;
   SVGA3dResourceType resourceDimension;
   SVGA3dRenderTargetViewDesc desc;
   uint32 pad[2];
}
#include "vmware_pack_end.h"
SVGACOTableDXRTViewEntry;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDefineRenderTargetView {
   SVGA3dRenderTargetViewId renderTargetViewId;

   SVGA3dSurfaceId sid;
   SVGA3dSurfaceFormat format;
   SVGA3dResourceType resourceDimension;

   SVGA3dRenderTargetViewDesc desc;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDefineRenderTargetView;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDestroyRenderTargetView {
   SVGA3dRenderTargetViewId renderTargetViewId;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDestroyRenderTargetView;



#define SVGA3D_DXDSVIEW_CREATE_READ_ONLY_DEPTH   0x01
#define SVGA3D_DXDSVIEW_CREATE_READ_ONLY_STENCIL 0x02
#define SVGA3D_DXDSVIEW_CREATE_FLAG_MASK         0x03
typedef uint8 SVGA3DCreateDSViewFlags;

typedef
#include "vmware_pack_begin.h"
struct {
   SVGA3dSurfaceId sid;
   SVGA3dSurfaceFormat format;
   SVGA3dResourceType resourceDimension;
   uint32 mipSlice;
   uint32 firstArraySlice;
   uint32 arraySize;
   SVGA3DCreateDSViewFlags flags;
   uint8 pad0;
   uint16 pad1;
   uint32 pad2;
}
#include "vmware_pack_end.h"
SVGACOTableDXDSViewEntry;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDefineDepthStencilView {
   SVGA3dDepthStencilViewId depthStencilViewId;

   SVGA3dSurfaceId sid;
   SVGA3dSurfaceFormat format;
   SVGA3dResourceType resourceDimension;
   uint32 mipSlice;
   uint32 firstArraySlice;
   uint32 arraySize;
   SVGA3DCreateDSViewFlags flags;
   uint8 pad0;
   uint16 pad1;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDefineDepthStencilView;



typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDefineDepthStencilView_v2 {
   SVGA3dDepthStencilViewId depthStencilViewId;

   SVGA3dSurfaceId sid;
   SVGA3dSurfaceFormat format;
   SVGA3dResourceType resourceDimension;
   uint32 mipSlice;
   uint32 firstArraySlice;
   uint32 arraySize;
   SVGA3DCreateDSViewFlags flags;
   uint8 pad0;
   uint16 pad1;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDefineDepthStencilView_v2;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDestroyDepthStencilView {
   SVGA3dDepthStencilViewId depthStencilViewId;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDestroyDepthStencilView;



#define SVGA3D_UABUFFER_RAW     (1 << 0)
#define SVGA3D_UABUFFER_APPEND  (1 << 1)
#define SVGA3D_UABUFFER_COUNTER (1 << 2)
typedef uint32 SVGA3dUABufferFlags;

typedef
#include "vmware_pack_begin.h"
struct {
   union {
      struct {
         uint32 firstElement;
         uint32 numElements;
         SVGA3dUABufferFlags flags;
         uint32 padding0;
         uint32 padding1;
      } buffer;
      struct {
         uint32 mipSlice;
         uint32 firstArraySlice;
         uint32 arraySize;
         uint32 padding0;
         uint32 padding1;
      } tex;
      struct {
         uint32 mipSlice;
         uint32 firstW;
         uint32 wSize;
         uint32 padding0;
         uint32 padding1;
      } tex3D;
   };
}
#include "vmware_pack_end.h"
SVGA3dUAViewDesc;

typedef
#include "vmware_pack_begin.h"
struct {
   SVGA3dSurfaceId sid;
   SVGA3dSurfaceFormat format;
   SVGA3dResourceType resourceDimension;
   SVGA3dUAViewDesc desc;
   uint32 structureCount;
   uint32 pad[7];
}
#include "vmware_pack_end.h"
SVGACOTableDXUAViewEntry;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDefineUAView {
   SVGA3dUAViewId uaViewId;

   SVGA3dSurfaceId sid;
   SVGA3dSurfaceFormat format;
   SVGA3dResourceType resourceDimension;

   SVGA3dUAViewDesc desc;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDefineUAView;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDestroyUAView {
   SVGA3dUAViewId uaViewId;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDestroyUAView;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXClearUAViewUint {
   SVGA3dUAViewId uaViewId;
   SVGA3dRGBAUint32 value;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXClearUAViewUint;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXClearUAViewFloat {
   SVGA3dUAViewId uaViewId;
   SVGA3dRGBAFloat value;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXClearUAViewFloat;




typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXCopyStructureCount {
   SVGA3dUAViewId srcUAViewId;
   SVGA3dSurfaceId destSid;
   uint32 destByteOffset;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXCopyStructureCount;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetStructureCount {
   SVGA3dUAViewId uaViewId;
   uint32 structureCount;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetStructureCount;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetUAViews {
   uint32 uavSpliceIndex;

}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetUAViews;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetCSUAViews {
   uint32 startIndex;

}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetCSUAViews;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dInputElementDesc {
   uint32 inputSlot;
   uint32 alignedByteOffset;
   SVGA3dSurfaceFormat format;
   SVGA3dInputClassification inputSlotClass;
   uint32 instanceDataStepRate;
   uint32 inputRegister;
}
#include "vmware_pack_end.h"
SVGA3dInputElementDesc;

typedef
#include "vmware_pack_begin.h"
struct {
   uint32 elid;
   uint32 numDescs;
   SVGA3dInputElementDesc descs[32];
   uint32 pad[62];
}
#include "vmware_pack_end.h"
SVGACOTableDXElementLayoutEntry;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDefineElementLayout {
   SVGA3dElementLayoutId elementLayoutId;

}
#include "vmware_pack_end.h"
SVGA3dCmdDXDefineElementLayout;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDestroyElementLayout {
   SVGA3dElementLayoutId elementLayoutId;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDestroyElementLayout;



#define SVGA3D_DX_MAX_RENDER_TARGETS 8

typedef
#include "vmware_pack_begin.h"
struct SVGA3dDXBlendStatePerRT {
      uint8 blendEnable;
      uint8 srcBlend;
      uint8 destBlend;
      uint8 blendOp;
      uint8 srcBlendAlpha;
      uint8 destBlendAlpha;
      uint8 blendOpAlpha;
      SVGA3dColorWriteEnable renderTargetWriteMask;
      uint8 logicOpEnable;
      uint8 logicOp;
      uint16 pad0;
}
#include "vmware_pack_end.h"
SVGA3dDXBlendStatePerRT;

typedef
#include "vmware_pack_begin.h"
struct {
   uint8 alphaToCoverageEnable;
   uint8 independentBlendEnable;
   uint16 pad0;
   SVGA3dDXBlendStatePerRT perRT[SVGA3D_MAX_RENDER_TARGETS];
   uint32 pad1[7];
}
#include "vmware_pack_end.h"
SVGACOTableDXBlendStateEntry;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDefineBlendState {
   SVGA3dBlendStateId blendId;
   uint8 alphaToCoverageEnable;
   uint8 independentBlendEnable;
   uint16 pad0;
   SVGA3dDXBlendStatePerRT perRT[SVGA3D_MAX_RENDER_TARGETS];
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDefineBlendState;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDestroyBlendState {
   SVGA3dBlendStateId blendId;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDestroyBlendState;

typedef
#include "vmware_pack_begin.h"
struct {
   uint8 depthEnable;
   SVGA3dDepthWriteMask depthWriteMask;
   SVGA3dComparisonFunc depthFunc;
   uint8 stencilEnable;
   uint8 frontEnable;
   uint8 backEnable;
   uint8 stencilReadMask;
   uint8 stencilWriteMask;

   uint8 frontStencilFailOp;
   uint8 frontStencilDepthFailOp;
   uint8 frontStencilPassOp;
   SVGA3dComparisonFunc frontStencilFunc;

   uint8 backStencilFailOp;
   uint8 backStencilDepthFailOp;
   uint8 backStencilPassOp;
   SVGA3dComparisonFunc backStencilFunc;
}
#include "vmware_pack_end.h"
SVGACOTableDXDepthStencilEntry;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDefineDepthStencilState {
   SVGA3dDepthStencilStateId depthStencilId;

   uint8 depthEnable;
   SVGA3dDepthWriteMask depthWriteMask;
   SVGA3dComparisonFunc depthFunc;
   uint8 stencilEnable;
   uint8 frontEnable;
   uint8 backEnable;
   uint8 stencilReadMask;
   uint8 stencilWriteMask;

   uint8 frontStencilFailOp;
   uint8 frontStencilDepthFailOp;
   uint8 frontStencilPassOp;
   SVGA3dComparisonFunc frontStencilFunc;

   uint8 backStencilFailOp;
   uint8 backStencilDepthFailOp;
   uint8 backStencilPassOp;
   SVGA3dComparisonFunc backStencilFunc;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDefineDepthStencilState;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDestroyDepthStencilState {
   SVGA3dDepthStencilStateId depthStencilId;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDestroyDepthStencilState;


typedef
#include "vmware_pack_begin.h"
struct {
   uint8 fillMode;
   SVGA3dCullMode cullMode;
   uint8 frontCounterClockwise;
   uint8 provokingVertexLast;
   int32 depthBias;
   float depthBiasClamp;
   float slopeScaledDepthBias;
   uint8 depthClipEnable;
   uint8 scissorEnable;
   SVGA3dMultisampleRastEnable multisampleEnable;
   uint8 antialiasedLineEnable;
   float lineWidth;
   uint8 lineStippleEnable;
   uint8 lineStippleFactor;
   uint16 lineStipplePattern;
   uint8 forcedSampleCount;
   uint8 mustBeZero[3];
}
#include "vmware_pack_end.h"
SVGACOTableDXRasterizerStateEntry;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDefineRasterizerState {
   SVGA3dRasterizerStateId rasterizerId;

   uint8 fillMode;
   SVGA3dCullMode cullMode;
   uint8 frontCounterClockwise;
   uint8 provokingVertexLast;
   int32 depthBias;
   float depthBiasClamp;
   float slopeScaledDepthBias;
   uint8 depthClipEnable;
   uint8 scissorEnable;
   SVGA3dMultisampleRastEnable multisampleEnable;
   uint8 antialiasedLineEnable;
   float lineWidth;
   uint8 lineStippleEnable;
   uint8 lineStippleFactor;
   uint16 lineStipplePattern;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDefineRasterizerState;



typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDefineRasterizerState_v2 {
   SVGA3dRasterizerStateId rasterizerId;

   uint8 fillMode;
   SVGA3dCullMode cullMode;
   uint8 frontCounterClockwise;
   uint8 provokingVertexLast;
   int32 depthBias;
   float depthBiasClamp;
   float slopeScaledDepthBias;
   uint8 depthClipEnable;
   uint8 scissorEnable;
   SVGA3dMultisampleRastEnable multisampleEnable;
   uint8 antialiasedLineEnable;
   float lineWidth;
   uint8 lineStippleEnable;
   uint8 lineStippleFactor;
   uint16 lineStipplePattern;
   uint32 forcedSampleCount;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDefineRasterizerState_v2;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDestroyRasterizerState {
   SVGA3dRasterizerStateId rasterizerId;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDestroyRasterizerState;


typedef
#include "vmware_pack_begin.h"
struct {
   SVGA3dFilter filter;
   uint8 addressU;
   uint8 addressV;
   uint8 addressW;
   uint8 pad0;
   float mipLODBias;
   uint8 maxAnisotropy;
   SVGA3dComparisonFunc comparisonFunc;
   uint16 pad1;
   SVGA3dRGBAFloat borderColor;
   float minLOD;
   float maxLOD;
   uint32 pad2[6];
}
#include "vmware_pack_end.h"
SVGACOTableDXSamplerEntry;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDefineSamplerState {
   SVGA3dSamplerId samplerId;
   SVGA3dFilter filter;
   uint8 addressU;
   uint8 addressV;
   uint8 addressW;
   uint8 pad0;
   float mipLODBias;
   uint8 maxAnisotropy;
   SVGA3dComparisonFunc comparisonFunc;
   uint16 pad1;
   SVGA3dRGBAFloat borderColor;
   float minLOD;
   float maxLOD;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDefineSamplerState;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDestroySamplerState {
   SVGA3dSamplerId samplerId;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDestroySamplerState;


#define SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED                          0
#define SVGADX_SIGNATURE_SEMANTIC_NAME_POSITION                           1
#define SVGADX_SIGNATURE_SEMANTIC_NAME_CLIP_DISTANCE                      2
#define SVGADX_SIGNATURE_SEMANTIC_NAME_CULL_DISTANCE                      3
#define SVGADX_SIGNATURE_SEMANTIC_NAME_RENDER_TARGET_ARRAY_INDEX          4
#define SVGADX_SIGNATURE_SEMANTIC_NAME_VIEWPORT_ARRAY_INDEX               5
#define SVGADX_SIGNATURE_SEMANTIC_NAME_VERTEX_ID                          6
#define SVGADX_SIGNATURE_SEMANTIC_NAME_PRIMITIVE_ID                       7
#define SVGADX_SIGNATURE_SEMANTIC_NAME_INSTANCE_ID                        8
#define SVGADX_SIGNATURE_SEMANTIC_NAME_IS_FRONT_FACE                      9
#define SVGADX_SIGNATURE_SEMANTIC_NAME_SAMPLE_INDEX                       10
#define SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_QUAD_U_EQ_0_EDGE_TESSFACTOR  11
#define SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_QUAD_V_EQ_0_EDGE_TESSFACTOR  12
#define SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_QUAD_U_EQ_1_EDGE_TESSFACTOR  13
#define SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_QUAD_V_EQ_1_EDGE_TESSFACTOR  14
#define SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_QUAD_U_INSIDE_TESSFACTOR     15
#define SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_QUAD_V_INSIDE_TESSFACTOR     16
#define SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_TRI_U_EQ_0_EDGE_TESSFACTOR   17
#define SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_TRI_V_EQ_0_EDGE_TESSFACTOR   18
#define SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_TRI_W_EQ_0_EDGE_TESSFACTOR   19
#define SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_TRI_INSIDE_TESSFACTOR        20
#define SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_LINE_DETAIL_TESSFACTOR       21
#define SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_LINE_DENSITY_TESSFACTOR      22
#define SVGADX_SIGNATURE_SEMANTIC_NAME_MAX                                23
typedef uint32 SVGA3dDXSignatureSemanticName;

#define SVGADX_SIGNATURE_REGISTER_COMPONENT_UNKNOWN 0
typedef uint32 SVGA3dDXSignatureRegisterComponentType;

#define SVGADX_SIGNATURE_MIN_PRECISION_DEFAULT 0
typedef uint32 SVGA3dDXSignatureMinPrecision;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dDXSignatureEntry {
   uint32 registerIndex;
   SVGA3dDXSignatureSemanticName semanticName;
   uint32 mask;
   SVGA3dDXSignatureRegisterComponentType componentType;
   SVGA3dDXSignatureMinPrecision minPrecision;
}
#include "vmware_pack_end.h"
SVGA3dDXShaderSignatureEntry;

#define SVGADX_SIGNATURE_HEADER_VERSION_0 0x08a92d12


typedef
#include "vmware_pack_begin.h"
struct SVGA3dDXSignatureHeader {
   uint32 headerVersion;
   uint32 numInputSignatures;
   uint32 numOutputSignatures;
   uint32 numPatchConstantSignatures;
}
#include "vmware_pack_end.h"
SVGA3dDXShaderSignatureHeader;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDefineShader {
   SVGA3dShaderId shaderId;
   SVGA3dShaderType type;
   uint32 sizeInBytes;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDefineShader;

typedef
#include "vmware_pack_begin.h"
struct SVGACOTableDXShaderEntry {
   SVGA3dShaderType type;
   uint32 sizeInBytes;
   uint32 offsetInBytes;
   SVGAMobId mobid;
   uint32 pad[4];
}
#include "vmware_pack_end.h"
SVGACOTableDXShaderEntry;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDestroyShader {
   SVGA3dShaderId shaderId;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDestroyShader;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXBindShader {
   uint32 cid;
   uint32 shid;
   SVGAMobId mobid;
   uint32 offsetInBytes;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXBindShader;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXBindAllShader {
   uint32 cid;
   SVGAMobId mobid;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXBindAllShader;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXCondBindAllShader {
   uint32 cid;
   SVGAMobId testMobid;
   SVGAMobId mobid;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXCondBindAllShader;


#define SVGA3D_MAX_DX10_STREAMOUT_DECLS 64
#define SVGA3D_MAX_STREAMOUT_DECLS 512

typedef
#include "vmware_pack_begin.h"
struct SVGA3dStreamOutputDeclarationEntry {
   uint32 outputSlot;
   uint32 registerIndex;
   uint8  registerMask;
   uint8  pad0;
   uint16 pad1;
   uint32 stream;
}
#include "vmware_pack_end.h"
SVGA3dStreamOutputDeclarationEntry;

typedef
#include "vmware_pack_begin.h"
struct SVGAOTableStreamOutputEntry {
   uint32 numOutputStreamEntries;
   SVGA3dStreamOutputDeclarationEntry decl[SVGA3D_MAX_DX10_STREAMOUT_DECLS];
   uint32 streamOutputStrideInBytes[SVGA3D_DX_MAX_SOTARGETS];
   uint32 rasterizedStream;
   uint32 numOutputStreamStrides;
   uint32 mobid;
   uint32 offsetInBytes;
   uint8 usesMob;
   uint8 pad0;
   uint16 pad1;
   uint32 pad2[246];
}
#include "vmware_pack_end.h"
SVGACOTableDXStreamOutputEntry;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDefineStreamOutput {
   SVGA3dStreamOutputId soid;
   uint32 numOutputStreamEntries;
   SVGA3dStreamOutputDeclarationEntry decl[SVGA3D_MAX_DX10_STREAMOUT_DECLS];
   uint32 streamOutputStrideInBytes[SVGA3D_DX_MAX_SOTARGETS];
   uint32 rasterizedStream;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDefineStreamOutput;



#define SVGA3D_DX_SO_NO_RASTERIZED_STREAM 0xFFFFFFFF

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDefineStreamOutputWithMob {
   SVGA3dStreamOutputId soid;
   uint32 numOutputStreamEntries;
   uint32 numOutputStreamStrides;
   uint32 streamOutputStrideInBytes[SVGA3D_DX_MAX_SOTARGETS];
   uint32 rasterizedStream;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDefineStreamOutputWithMob;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXBindStreamOutput {
   SVGA3dStreamOutputId soid;
   uint32 mobid;
   uint32 offsetInBytes;
   uint32 sizeInBytes;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXBindStreamOutput;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXDestroyStreamOutput {
   SVGA3dStreamOutputId soid;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXDestroyStreamOutput;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetStreamOutput {
   SVGA3dStreamOutputId soid;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetStreamOutput;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetMinLOD {
   SVGA3dSurfaceId sid;
   float minLOD;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetMinLOD;

typedef
#include "vmware_pack_begin.h"
struct {
   uint64 value;
   uint32 mobId;
   uint32 mobOffset;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXMobFence64;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXSetCOTable {
   uint32 cid;
   uint32 mobid;
   SVGACOTableType type;
   uint32 validSizeInBytes;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXSetCOTable;


typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXGrowCOTable {
   uint32 cid;
   uint32 mobid;
   SVGACOTableType type;
   uint32 validSizeInBytes;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXGrowCOTable;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXReadbackCOTable {
   uint32 cid;
   SVGACOTableType type;
}
#include "vmware_pack_end.h"
SVGA3dCmdDXReadbackCOTable;



typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXPredStagingCopy {
   SVGA3dSurfaceId dstSid;
   SVGA3dSurfaceId srcSid;
   uint8 readback;
   uint8 unsynchronized;
   uint8 mustBeZero[2];

}
#include "vmware_pack_end.h"
SVGA3dCmdDXPredStagingCopy;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCmdDXStagingCopy {
   SVGA3dSurfaceId dstSid;
   SVGA3dSurfaceId srcSid;
   uint8 readback;
   uint8 unsynchronized;
   uint8 mustBeZero[2];

}
#include "vmware_pack_end.h"
SVGA3dCmdDXStagingCopy;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dCOTableData {
   uint32 mobid;
}
#include "vmware_pack_end.h"
SVGA3dCOTableData;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dBufferBinding {
   uint32 bufferId;
   uint32 stride;
   uint32 offset;
}
#include "vmware_pack_end.h"
SVGA3dBufferBinding;

typedef
#include "vmware_pack_begin.h"
struct SVGA3dConstantBufferBinding {
   uint32 sid;
   uint32 offsetInBytes;
   uint32 sizeInBytes;
}
#include "vmware_pack_end.h"
SVGA3dConstantBufferBinding;

typedef
#include "vmware_pack_begin.h"
struct SVGADXInputAssemblyMobFormat {
   uint32 layoutId;
   SVGA3dBufferBinding vertexBuffers[SVGA3D_DX_MAX_VERTEXBUFFERS];
   uint32 indexBufferSid;
   uint32 pad;
   uint32 indexBufferOffset;
   uint32 indexBufferFormat;
   uint32 topology;
}
#include "vmware_pack_end.h"
SVGADXInputAssemblyMobFormat;

typedef
#include "vmware_pack_begin.h"
struct SVGADXContextMobFormat {
   SVGADXInputAssemblyMobFormat inputAssembly;

   struct {
      uint32 blendStateId;
      uint32 blendFactor[4];
      uint32 sampleMask;
      uint32 depthStencilStateId;
      uint32 stencilRef;
      uint32 rasterizerStateId;
      uint32 depthStencilViewId;
      uint32 renderTargetViewIds[SVGA3D_MAX_SIMULTANEOUS_RENDER_TARGETS];
   } renderState;

   uint32 pad0[8];

   struct {
      uint32 targets[SVGA3D_DX_MAX_SOTARGETS];
      uint32 soid;
   } streamOut;

   uint32 pad1[10];

   uint32 uavSpliceIndex;

   uint8 numViewports;
   uint8 numScissorRects;
   uint16 pad2[1];

   uint32 pad3[3];

   SVGA3dViewport viewports[SVGA3D_DX_MAX_VIEWPORTS];
   uint32 pad4[32];

   SVGASignedRect scissorRects[SVGA3D_DX_MAX_SCISSORRECTS];
   uint32 pad5[64];

   struct {
      uint32 queryID;
      uint32 value;
   } predication;

   SVGAMobId shaderIfaceMobid;
   uint32 shaderIfaceOffset;
   struct {
      uint32 shaderId;
      SVGA3dConstantBufferBinding constantBuffers[SVGA3D_DX_MAX_CONSTBUFFERS];
      uint32 shaderResources[SVGA3D_DX_MAX_SRVIEWS];
      uint32 samplers[SVGA3D_DX_MAX_SAMPLERS];
   } shaderState[SVGA3D_NUM_SHADERTYPE];
   uint32 pad6[26];

   SVGA3dQueryId queryID[SVGA3D_MAX_QUERY];

   SVGA3dCOTableData cotables[SVGA_COTABLE_MAX];

   uint32 pad7[64];

   uint32 uaViewIds[SVGA3D_DX11_1_MAX_UAVIEWS];
   uint32 csuaViewIds[SVGA3D_DX11_1_MAX_UAVIEWS];

   uint32 pad8[188];
}
#include "vmware_pack_end.h"
SVGADXContextMobFormat;


#define SVGA3D_DX_MAX_CLASS_INSTANCES_PADDED 256

typedef
#include "vmware_pack_begin.h"
struct SVGADXShaderIfaceMobFormat {
   struct {
      uint32 numClassInstances;
      uint32 iface[SVGA3D_DX_MAX_CLASS_INSTANCES_PADDED];
      SVGA3dIfaceData data[SVGA3D_DX_MAX_CLASS_INSTANCES_PADDED];
   } shaderIfaceState[SVGA3D_NUM_SHADERTYPE];

   uint32 pad0[1018];
}
#include "vmware_pack_end.h"
SVGADXShaderIfaceMobFormat;

#endif
