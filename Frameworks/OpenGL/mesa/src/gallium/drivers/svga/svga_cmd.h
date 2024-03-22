/**********************************************************
 * Copyright 2008-2009 VMware, Inc.  All rights reserved.
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
 **********************************************************/

/*
 * svga_cmd.h --
 *
 *      Command construction utility for the SVGA3D protocol used by
 *      the VMware SVGA device, based on the svgautil library.
 */

#ifndef __SVGA3D_H__
#define __SVGA3D_H__


#include "svga_types.h"
#include "svga_winsys.h"
#include "svga_reg.h"
#include "svga3d_reg.h"

#include "pipe/p_defines.h"


struct pipe_surface;
struct svga_transfer;
struct svga_winsys_context;
struct svga_winsys_buffer;
struct svga_winsys_surface;
struct svga_winsys_gb_shader;
struct svga_winsys_gb_query;


/*
 * SVGA Device Interoperability
 */

void *
SVGA3D_FIFOReserve(struct svga_winsys_context *swc, uint32 cmd, uint32 cmdSize, uint32 nr_relocs);

void
SVGA_FIFOCommitAll(struct svga_winsys_context *swc);

/**
 * Return the last command id put in the command buffer.
 */
static inline SVGAFifo3dCmdId
SVGA3D_GetLastCommand(const struct svga_winsys_context *swc)
{
   return swc->last_command;
}

/**
 * Reset/clear the last command put in the command buffer.
 * To be called when buffer is flushed.
 */
static inline void
SVGA3D_ResetLastCommand(struct svga_winsys_context *swc)
{
   swc->last_command = 0;
}


/*
 * Context Management
 */

enum pipe_error
SVGA3D_DefineContext(struct svga_winsys_context *swc);

enum pipe_error
SVGA3D_DestroyContext(struct svga_winsys_context *swc);


/*
 * Surface Management
 */

enum pipe_error
SVGA3D_BeginDefineSurface(struct svga_winsys_context *swc,
                          struct svga_winsys_surface *sid,
                          SVGA3dSurface1Flags flags,
                          SVGA3dSurfaceFormat format,
                          SVGA3dSurfaceFace **faces,
                          SVGA3dSize **mipSizes,
                          uint32 numMipSizes);
enum pipe_error
SVGA3D_DefineSurface2D(struct svga_winsys_context *swc,
                       struct svga_winsys_surface *sid,
                       uint32 width,
                       uint32 height,
                       SVGA3dSurfaceFormat format);
enum pipe_error
SVGA3D_DestroySurface(struct svga_winsys_context *swc,
                      struct svga_winsys_surface *sid);


/*
 * Surface Operations
 */

enum pipe_error
SVGA3D_SurfaceDMA(struct svga_winsys_context *swc,
                  struct svga_transfer *st,
                  SVGA3dTransferType transfer,
                  const SVGA3dCopyBox *boxes,
                  uint32 numBoxes,
                  SVGA3dSurfaceDMAFlags flags);

enum pipe_error
SVGA3D_BufferDMA(struct svga_winsys_context *swc,
                 struct svga_winsys_buffer *guest,
                 struct svga_winsys_surface *host,
                 SVGA3dTransferType transfer,
                 uint32 size,
                 uint32 guest_offset,
                 uint32 host_offset,
                 SVGA3dSurfaceDMAFlags flags);

/*
 * Drawing Operations
 */


enum pipe_error
SVGA3D_BeginClear(struct svga_winsys_context *swc,
                  SVGA3dClearFlag flags,
                  uint32 color, float depth, uint32 stencil,
                  SVGA3dRect **rects, uint32 numRects);

enum pipe_error
SVGA3D_ClearRect(struct svga_winsys_context *swc,
                 SVGA3dClearFlag flags, uint32 color, float depth,
                 uint32 stencil, uint32 x, uint32 y, uint32 w, uint32 h);

enum pipe_error
SVGA3D_BeginDrawPrimitives(struct svga_winsys_context *swc,
                           SVGA3dVertexDecl **decls,
                           uint32 numVertexDecls,
                           SVGA3dPrimitiveRange **ranges,
                           uint32 numRanges);

/*
 * Blits
 */

enum pipe_error
SVGA3D_BeginSurfaceCopy(struct svga_winsys_context *swc,
                        struct pipe_surface *src,
                        struct pipe_surface *dest,
                        SVGA3dCopyBox **boxes, uint32 numBoxes);


enum pipe_error
SVGA3D_SurfaceStretchBlt(struct svga_winsys_context *swc,
                         struct pipe_surface *src,
                         struct pipe_surface *dest,
                         SVGA3dBox *boxSrc, SVGA3dBox *boxDest,
                         SVGA3dStretchBltMode mode);

/*
 * Shared FFP/Shader Render State
 */

enum pipe_error
SVGA3D_SetRenderTarget(struct svga_winsys_context *swc,
                       SVGA3dRenderTargetType type,
                       struct pipe_surface *surface);

enum pipe_error
SVGA3D_SetZRange(struct svga_winsys_context *swc,
                 float zMin, float zMax);

enum pipe_error
SVGA3D_SetViewport(struct svga_winsys_context *swc,
                   SVGA3dRect *rect);

enum pipe_error
SVGA3D_SetScissorRect(struct svga_winsys_context *swc,
                      SVGA3dRect *rect);

enum pipe_error
SVGA3D_SetClipPlane(struct svga_winsys_context *swc,
                    uint32 index, const float *plane);

enum pipe_error
SVGA3D_BeginSetTextureState(struct svga_winsys_context *swc,
                            SVGA3dTextureState **states,
                            uint32 numStates);

enum pipe_error
SVGA3D_BeginSetRenderState(struct svga_winsys_context *swc,
                           SVGA3dRenderState **states,
                           uint32 numStates);


/*
 * Shaders
 */

enum pipe_error
SVGA3D_DefineShader(struct svga_winsys_context *swc,
                    uint32 shid, SVGA3dShaderType type,
                    const uint32 *bytecode, uint32 bytecodeLen);

enum pipe_error
SVGA3D_DestroyShader(struct svga_winsys_context *swc,
                     uint32 shid, SVGA3dShaderType type);

enum pipe_error
SVGA3D_SetShaderConst(struct svga_winsys_context *swc,
                      uint32 reg, SVGA3dShaderType type,
                      SVGA3dShaderConstType ctype, const void *value);

enum pipe_error
SVGA3D_SetShaderConsts(struct svga_winsys_context *swc,
                       uint32 reg,
                       uint32 numRegs,
                       SVGA3dShaderType type,
                       SVGA3dShaderConstType ctype,
                       const void *values);

enum pipe_error
SVGA3D_SetShader(struct svga_winsys_context *swc,
                 SVGA3dShaderType type, uint32 shid);


/*
 * Guest-backed surface functions
 */

enum pipe_error
SVGA3D_BindGBShader(struct svga_winsys_context *swc,
                    struct svga_winsys_gb_shader *gbshader);

enum pipe_error
SVGA3D_SetGBShader(struct svga_winsys_context *swc,
                   SVGA3dShaderType type,
                   struct svga_winsys_gb_shader *gbshader);

enum pipe_error
SVGA3D_BindGBSurface(struct svga_winsys_context *swc,
                     struct svga_winsys_surface *surface);

enum pipe_error
SVGA3D_UpdateGBImage(struct svga_winsys_context *swc,
                     struct svga_winsys_surface *surface,
                     const SVGA3dBox *box,
                     unsigned face, unsigned mipLevel);

enum pipe_error
SVGA3D_UpdateGBSurface(struct svga_winsys_context *swc,
                       struct svga_winsys_surface *surface);


enum pipe_error
SVGA3D_ReadbackGBImage(struct svga_winsys_context *swc,
                       struct svga_winsys_surface *surface,
                       unsigned face, unsigned mipLevel);


enum pipe_error
SVGA3D_ReadbackGBSurface(struct svga_winsys_context *swc,
                         struct svga_winsys_surface *surface);


enum pipe_error
SVGA3D_ReadbackGBImagePartial(struct svga_winsys_context *swc,
                              struct svga_winsys_surface *surface,
                              unsigned face, unsigned mipLevel,
                              const SVGA3dBox *box,
                              bool invertBox);


enum pipe_error
SVGA3D_InvalidateGBImagePartial(struct svga_winsys_context *swc,
                                struct svga_winsys_surface *surface,
                                unsigned face, unsigned mipLevel,
                                const SVGA3dBox *box,
                                bool invertBox);

enum pipe_error
SVGA3D_InvalidateGBSurface(struct svga_winsys_context *swc,
                           struct svga_winsys_surface *surface);


enum pipe_error
SVGA3D_SetGBShaderConstsInline(struct svga_winsys_context *swc,
                               unsigned regStart,
                               unsigned numRegs,
                               SVGA3dShaderType shaderType,
                               SVGA3dShaderConstType constType,
                               const void *values);

/*
 * Queries
 */

enum pipe_error
SVGA3D_BeginQuery(struct svga_winsys_context *swc,
                  SVGA3dQueryType type);

enum pipe_error
SVGA3D_EndQuery(struct svga_winsys_context *swc,
                SVGA3dQueryType type,
                struct svga_winsys_buffer *buffer);

enum pipe_error
SVGA3D_WaitForQuery(struct svga_winsys_context *swc,
                    SVGA3dQueryType type,
                    struct svga_winsys_buffer *buffer);



/*
 * VGPU10 commands
 */

enum pipe_error
SVGA3D_vgpu10_PredCopyRegion(struct svga_winsys_context *swc,
                             struct svga_winsys_surface *dstSurf,
                             uint32 dstSubResource,
                             struct svga_winsys_surface *srcSurf,
                             uint32 srcSubResource,
                             const SVGA3dCopyBox *box);

enum pipe_error
SVGA3D_vgpu10_PredCopy(struct svga_winsys_context *swc,
                       struct svga_winsys_surface *dstSurf,
                       struct svga_winsys_surface *srcSurf);

enum pipe_error
SVGA3D_vgpu10_SetViewports(struct svga_winsys_context *swc,
                           unsigned count, const SVGA3dViewport *viewports);

enum pipe_error
SVGA3D_vgpu10_SetShader(struct svga_winsys_context *swc,
                        SVGA3dShaderType type,
                        struct svga_winsys_gb_shader *gbshader,
                        SVGA3dShaderId shaderId);

enum pipe_error
SVGA3D_vgpu10_SetShaderResources(struct svga_winsys_context *swc,
                                 SVGA3dShaderType type,
                                 uint32 startView,
                                 unsigned count,
                                 const SVGA3dShaderResourceViewId ids[],
                                 struct svga_winsys_surface **views);

enum pipe_error
SVGA3D_vgpu10_SetSamplers(struct svga_winsys_context *swc,
                          unsigned count,
                          uint32 startSampler,
                          SVGA3dShaderType type,
                          const SVGA3dSamplerId *samplerIds);

enum pipe_error
SVGA3D_vgpu10_SetRenderTargets(struct svga_winsys_context *swc,
                               unsigned color_count,
                               struct pipe_surface **color_surfs,
                               struct pipe_surface *depth_stencil_surf);

enum pipe_error
SVGA3D_vgpu10_SetBlendState(struct svga_winsys_context *swc,
                            SVGA3dBlendStateId blendId,
                            const float *blendFactor, uint32 sampleMask);

enum pipe_error
SVGA3D_vgpu10_SetDepthStencilState(struct svga_winsys_context *swc,
                                   SVGA3dDepthStencilStateId depthStencilId,
                                   uint32 stencilRef);

enum pipe_error
SVGA3D_vgpu10_SetRasterizerState(struct svga_winsys_context *swc,
                                 SVGA3dRasterizerStateId rasterizerId);

enum pipe_error
SVGA3D_vgpu10_SetPredication(struct svga_winsys_context *swc,
                             SVGA3dQueryId queryId,
                             uint32 predicateValue);

enum pipe_error
SVGA3D_vgpu10_SetSOTargets(struct svga_winsys_context *swc,
                           unsigned count, const SVGA3dSoTarget *targets,
                           struct svga_winsys_surface **surfaces);

enum pipe_error
SVGA3D_vgpu10_SetScissorRects(struct svga_winsys_context *swc,
                              unsigned count,
                              const SVGASignedRect *rects);

enum pipe_error
SVGA3D_vgpu10_SetStreamOutput(struct svga_winsys_context *swc,
                              SVGA3dStreamOutputId soid);

enum pipe_error
SVGA3D_vgpu10_Draw(struct svga_winsys_context *swc,
                   uint32 vertexCount, uint32 startVertexLocation);

enum pipe_error
SVGA3D_vgpu10_DrawIndexed(struct svga_winsys_context *swc,
                          uint32 indexCount, uint32 startIndexLocation,
                          int32 baseVertexLocation);

enum pipe_error
SVGA3D_vgpu10_DrawInstanced(struct svga_winsys_context *swc,
                            uint32 vertexCountPerInstance,
                            uint32 instanceCount,
                            uint32 startVertexLocation,
                            uint32 startInstanceLocation);

enum pipe_error
SVGA3D_vgpu10_DrawIndexedInstanced(struct svga_winsys_context *swc,
                                   uint32 indexCountPerInstance,
                                   uint32 instanceCount,
                                   uint32 startIndexLocation,
                                   int32  baseVertexLocation,
                                   uint32 startInstanceLocation);

enum pipe_error
SVGA3D_vgpu10_DrawAuto(struct svga_winsys_context *swc);

enum pipe_error
SVGA3D_vgpu10_DefineQuery(struct svga_winsys_context *swc,
                          SVGA3dQueryId queryId,
                          SVGA3dQueryType type,
                          SVGA3dDXQueryFlags flags);

enum pipe_error
SVGA3D_vgpu10_DestroyQuery(struct svga_winsys_context *swc,
                           SVGA3dQueryId queryId);

enum pipe_error
SVGA3D_vgpu10_BindQuery(struct svga_winsys_context *swc,
                        struct svga_winsys_gb_query *gbQuery,
                        SVGA3dQueryId queryId);

enum pipe_error
SVGA3D_vgpu10_SetQueryOffset(struct svga_winsys_context *swc,
                             SVGA3dQueryId queryId,
                             uint32 mobOffset);

enum pipe_error
SVGA3D_vgpu10_BeginQuery(struct svga_winsys_context *swc,
                         SVGA3dQueryId queryId);

enum pipe_error
SVGA3D_vgpu10_EndQuery(struct svga_winsys_context *swc,
                       SVGA3dQueryId queryId);

enum pipe_error
SVGA3D_vgpu10_ClearRenderTargetView(struct svga_winsys_context *swc,
                                    struct pipe_surface *color_surf,
                                    const float *rgba);

enum pipe_error
SVGA3D_vgpu10_ClearDepthStencilView(struct svga_winsys_context *swc,
                                    struct pipe_surface *ds_surf,
                                    uint16 flags, uint16 stencil, float depth);

enum pipe_error
SVGA3D_vgpu10_DefineShaderResourceView(struct svga_winsys_context *swc,
                             SVGA3dShaderResourceViewId shaderResourceViewId,
                             struct svga_winsys_surface *surf,
                             SVGA3dSurfaceFormat format,
                             SVGA3dResourceType resourceDimension,
                             const SVGA3dShaderResourceViewDesc *desc);

enum pipe_error
SVGA3D_vgpu10_DestroyShaderResourceView(struct svga_winsys_context *swc,
                            SVGA3dShaderResourceViewId shaderResourceViewId);

enum pipe_error
SVGA3D_vgpu10_DefineRenderTargetView(struct svga_winsys_context *swc,
                                  SVGA3dRenderTargetViewId renderTargetViewId,
                                  struct svga_winsys_surface *surface,
                                  SVGA3dSurfaceFormat format,
                                  SVGA3dResourceType resourceDimension,
                                  const SVGA3dRenderTargetViewDesc *desc);

enum pipe_error
SVGA3D_vgpu10_DestroyRenderTargetView(struct svga_winsys_context *swc,
                                SVGA3dRenderTargetViewId renderTargetViewId);

enum pipe_error
SVGA3D_vgpu10_DefineDepthStencilView(struct svga_winsys_context *swc,
                                  SVGA3dDepthStencilViewId depthStencilViewId,
                                  struct svga_winsys_surface *surface,
                                  SVGA3dSurfaceFormat format,
                                  SVGA3dResourceType resourceDimension,
                                  const SVGA3dRenderTargetViewDesc *desc);


enum pipe_error
SVGA3D_vgpu10_DestroyDepthStencilView(struct svga_winsys_context *swc,
                                SVGA3dDepthStencilViewId depthStencilViewId);

enum pipe_error
SVGA3D_vgpu10_DefineElementLayout(struct svga_winsys_context *swc,
                               unsigned count,
                               SVGA3dElementLayoutId elementLayoutId,
                               const SVGA3dInputElementDesc *elements);

enum pipe_error
SVGA3D_vgpu10_DestroyElementLayout(struct svga_winsys_context *swc,
                                   SVGA3dElementLayoutId elementLayoutId);

enum pipe_error
SVGA3D_vgpu10_DefineBlendState(struct svga_winsys_context *swc,
                               SVGA3dBlendStateId blendId,
                               uint8 alphaToCoverageEnable,
                               uint8 independentBlendEnable,
                               const SVGA3dDXBlendStatePerRT *perRT);

enum pipe_error
SVGA3D_vgpu10_DestroyBlendState(struct svga_winsys_context *swc,
                                SVGA3dBlendStateId blendId);

enum pipe_error
SVGA3D_vgpu10_DefineDepthStencilState(struct svga_winsys_context *swc,
                                      SVGA3dDepthStencilStateId depthStencilId,
                                      uint8 depthEnable,
                                      SVGA3dDepthWriteMask depthWriteMask,
                                      SVGA3dComparisonFunc depthFunc,
                                      uint8 stencilEnable,
                                      uint8 frontEnable,
                                      uint8 backEnable,
                                      uint8 stencilReadMask,
                                      uint8 stencilWriteMask,
                                      uint8 frontStencilFailOp,
                                      uint8 frontStencilDepthFailOp,
                                      uint8 frontStencilPassOp,
                                      SVGA3dComparisonFunc frontStencilFunc,
                                      uint8 backStencilFailOp,
                                      uint8 backStencilDepthFailOp,
                                      uint8 backStencilPassOp,
                                      SVGA3dComparisonFunc backStencilFunc);

enum pipe_error
SVGA3D_vgpu10_DestroyDepthStencilState(struct svga_winsys_context *swc,
                                       SVGA3dDepthStencilStateId depthStencilId);

enum pipe_error
SVGA3D_vgpu10_DefineRasterizerState(struct svga_winsys_context *swc,
                                    SVGA3dRasterizerStateId rasterizerId,
                                    uint8 fillMode,
                                    SVGA3dCullMode cullMode,
                                    uint8 frontCounterClockwise,
                                    int32 depthBias,
                                    float depthBiasClamp,
                                    float slopeScaledDepthBias,
                                    uint8 depthClipEnable,
                                    uint8 scissorEnable,
                                    uint8 multisampleEnable,
                                    uint8 antialiasedLineEnable,
                                    float lineWidth,
                                    uint8 lineStippleEnable,
                                    uint8 lineStippleFactor,
                                    uint16 lineStipplePattern,
                                    uint8 provokingVertexLast);

enum pipe_error
SVGA3D_vgpu10_DestroyRasterizerState(struct svga_winsys_context *swc,
                                     SVGA3dRasterizerStateId rasterizerId);

enum pipe_error
SVGA3D_vgpu10_DefineSamplerState(struct svga_winsys_context *swc,
                                 SVGA3dSamplerId samplerId,
                                 SVGA3dFilter filter,
                                 uint8 addressU,
                                 uint8 addressV,
                                 uint8 addressW,
                                 float mipLODBias,
                                 uint8 maxAnisotropy,
                                 uint8 comparisonFunc,
                                 SVGA3dRGBAFloat borderColor,
                                 float minLOD,
                                 float maxLOD);

enum pipe_error
SVGA3D_vgpu10_DestroySamplerState(struct svga_winsys_context *swc,
                                  SVGA3dSamplerId samplerId);

enum pipe_error
SVGA3D_vgpu10_DestroyShader(struct svga_winsys_context *swc,
                            SVGA3dShaderId shaderId);

enum pipe_error
SVGA3D_vgpu10_DefineAndBindShader(struct svga_winsys_context *swc,
                                  struct svga_winsys_gb_shader *gbshader,
                                  SVGA3dShaderId shaderId,
                                  SVGA3dShaderType type,
                                  uint32 sizeInBytes);

enum pipe_error
SVGA3D_vgpu10_DefineStreamOutput(struct svga_winsys_context *swc,
      SVGA3dStreamOutputId soid,
      uint32 numOutputStreamEntries,
      uint32 streamOutputStrideInBytes[SVGA3D_DX_MAX_SOTARGETS],
      const SVGA3dStreamOutputDeclarationEntry decl[SVGA3D_MAX_STREAMOUT_DECLS]);

enum pipe_error
SVGA3D_vgpu10_DestroyStreamOutput(struct svga_winsys_context *swc,
                                  SVGA3dStreamOutputId soid);

enum pipe_error
SVGA3D_vgpu10_ReadbackSubResource(struct svga_winsys_context *swc,
                                  struct svga_winsys_surface *surface,
                                  unsigned subResource);

enum pipe_error
SVGA3D_vgpu10_SetInputLayout(struct svga_winsys_context *swc,
                             SVGA3dElementLayoutId elementLayoutId);

enum pipe_error
SVGA3D_vgpu10_SetVertexBuffers(struct svga_winsys_context *swc,
                               unsigned count,
                               uint32 startBuffer,
                               const SVGA3dVertexBuffer_v2 *bufferInfo,
                               struct svga_winsys_surface **surfaces);

enum pipe_error
SVGA3D_vgpu10_SetVertexBuffers_v2(struct svga_winsys_context *swc,
                               unsigned count,
                               uint32 startBuffer,
                               const SVGA3dVertexBuffer_v2 *bufferInfo,
                               struct svga_winsys_surface **surfaces);

enum pipe_error
SVGA3D_vgpu10_SetVertexBuffersOffsetAndSize(struct svga_winsys_context *swc,
                               unsigned count,
                               uint32 startBuffer,
                               const SVGA3dVertexBuffer_v2 *bufferInfo);

enum pipe_error
SVGA3D_vgpu10_SetTopology(struct svga_winsys_context *swc,
                          SVGA3dPrimitiveType topology);

enum pipe_error
SVGA3D_vgpu10_SetIndexBuffer(struct svga_winsys_context *swc,
                             struct svga_winsys_surface *indexes,
                             SVGA3dSurfaceFormat format, uint32 offset);

enum pipe_error
SVGA3D_vgpu10_SetIndexBuffer_v2(struct svga_winsys_context *swc,
                                struct svga_winsys_surface *indexes,
                                SVGA3dSurfaceFormat format, uint32 offset,
                                uint32 sizeInBytes);

enum pipe_error
SVGA3D_vgpu10_SetIndexBufferOffsetAndSize(struct svga_winsys_context *swc,
                             SVGA3dSurfaceFormat format, uint32 offset,
                             uint32 sizeInBytes);

enum pipe_error
SVGA3D_vgpu10_SetSingleConstantBuffer(struct svga_winsys_context *swc,
                                      unsigned slot,
                                      SVGA3dShaderType type,
                                      struct svga_winsys_surface *surface,
                                      uint32 offsetInBytes,
                                      uint32 sizeInBytes);

enum pipe_error
SVGA3D_vgpu10_SetConstantBufferOffset(struct svga_winsys_context *swc,
                                      unsigned command,
                                      unsigned slot,
                                      uint32 offsetInBytes);

enum pipe_error
SVGA3D_vgpu10_UpdateSubResource(struct svga_winsys_context *swc,
                                struct svga_winsys_surface *surface,
                                const SVGA3dBox *box,
                                unsigned subResource);

enum pipe_error
SVGA3D_vgpu10_GenMips(struct svga_winsys_context *swc,
                      SVGA3dShaderResourceViewId shaderResourceViewId,
                      struct svga_winsys_surface *view);

enum pipe_error
SVGA3D_vgpu10_BufferCopy(struct svga_winsys_context *swc,
                         struct svga_winsys_surface *src,
                         struct svga_winsys_surface *dst,
                         unsigned srcx, unsigned dstx, unsigned width);

enum pipe_error
SVGA3D_vgpu10_TransferFromBuffer(struct svga_winsys_context *swc,
                                 struct svga_winsys_surface *src,
                                 unsigned srcOffset, unsigned srcPitch,
                                 unsigned srcSlicePitch,
                                 struct svga_winsys_surface *dst,
                                 unsigned dstSubResource,
                                 SVGA3dBox *dstBox);

/*Cap2 commands*/
enum pipe_error
SVGA3D_vgpu10_IntraSurfaceCopy(struct svga_winsys_context *swc,
                               struct svga_winsys_surface *src,
                               unsigned level, unsigned face,
                               const SVGA3dCopyBox *box);

enum pipe_error
SVGA3D_vgpu10_ResolveCopy(struct svga_winsys_context *swc,
                          unsigned dstSubResource,
                          struct svga_winsys_surface *dst,
                          unsigned srcSubResource,
                          struct svga_winsys_surface *src,
                          const SVGA3dSurfaceFormat copyFormat);

enum pipe_error
SVGA3D_sm5_DrawIndexedInstancedIndirect(struct svga_winsys_context *swc,
                                        struct svga_winsys_surface *argBuffer,
                                        unsigned argOffset);

enum pipe_error
SVGA3D_sm5_DrawInstancedIndirect(struct svga_winsys_context *swc,
                                 struct svga_winsys_surface *argBuffer,
                                 unsigned argOffset);

enum pipe_error
SVGA3D_sm5_DefineUAView(struct svga_winsys_context *swc,
                        SVGA3dUAViewId uaViewId,
                        struct svga_winsys_surface *surface,
                        SVGA3dSurfaceFormat format,
                        SVGA3dResourceType resourceDimension,
                        const SVGA3dUAViewDesc *desc);

enum pipe_error
SVGA3D_sm5_DestroyUAView(struct svga_winsys_context *swc,
                         SVGA3dUAViewId uaViewId);

enum pipe_error
SVGA3D_sm5_SetUAViews(struct svga_winsys_context *swc,
                      uint32 uavSpliceIndex,
                      unsigned count,
                      const SVGA3dUAViewId ids[],
                      struct svga_winsys_surface **uaViews);

enum pipe_error
SVGA3D_sm5_SetCSUAViews(struct svga_winsys_context *swc,
                        unsigned count,
                        const SVGA3dUAViewId ids[],
                        struct svga_winsys_surface **uaViews);

enum pipe_error
SVGA3D_sm5_Dispatch(struct svga_winsys_context *swc,
                    const uint32 threadGroupCount[3]);

enum pipe_error
SVGA3D_sm5_DispatchIndirect(struct svga_winsys_context *swc,
                            struct svga_winsys_surface *argBuffer,
                            uint32 argOffset);

enum pipe_error
SVGA3D_sm5_DefineAndBindStreamOutput(struct svga_winsys_context *swc,
       SVGA3dStreamOutputId soid,
       uint32 numOutputStreamEntries,
       uint32 numOutputStreamStrides,
       uint32 streamOutputStrideInBytes[SVGA3D_DX_MAX_SOTARGETS],
       struct svga_winsys_buffer *declBuf,
       uint32 rasterizedStream,
       uint32 sizeInBytes);

enum pipe_error
SVGA3D_sm5_DefineRasterizerState_v2(struct svga_winsys_context *swc,
                                    SVGA3dRasterizerStateId rasterizerId,
                                    uint8 fillMode,
                                    SVGA3dCullMode cullMode,
                                    uint8 frontCounterClockwise,
                                    int32 depthBias,
                                    float depthBiasClamp,
                                    float slopeScaledDepthBias,
                                    uint8 depthClipEnable,
                                    uint8 scissorEnable,
                                    uint8 multisampleEnable,
                                    uint8 antialiasedLineEnable,
                                    float lineWidth,
                                    uint8 lineStippleEnable,
                                    uint8 lineStippleFactor,
                                    uint16 lineStipplePattern,
                                    uint8 provokingVertexLast,
                                    uint32 forcedSampleCount);
#endif /* __SVGA3D_H__ */
