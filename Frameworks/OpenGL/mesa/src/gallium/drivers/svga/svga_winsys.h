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

/**
 * @file
 * VMware SVGA specific winsys interface.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 *
 * Documentation taken from the VMware SVGA DDK.
 */

#ifndef SVGA_WINSYS_H_
#define SVGA_WINSYS_H_

#include "svga_types.h"
#include "svga3d_types.h"
#include "svga_reg.h"
#include "svga3d_reg.h"

#include "util/compiler.h"
#include "pipe/p_defines.h"

#include "svga_mksstats.h"

struct svga_winsys_screen;
struct svga_winsys_buffer;
struct pipe_screen;
struct pipe_context;
struct util_debug_callback;
struct pipe_fence_handle;
struct pipe_resource;
struct svga_region;
struct winsys_handle;


#define SVGA_BUFFER_USAGE_PINNED  (1 << 0)
#define SVGA_BUFFER_USAGE_WRAPPED (1 << 1)
#define SVGA_BUFFER_USAGE_SHADER  (1 << 2)

/**
 * Relocation flags to help with dirty tracking
 * SVGA_RELOC_WRITE -   The command will cause a GPU write to this
 *                      resource.
 * SVGA_RELOC_READ -    The command will cause a GPU read from this
 *                      resource.
 * SVGA_RELOC_INTERNAL  The command will only transfer data internally
 *                      within the resource, and optionally clear
 *                      dirty bits
 * SVGA_RELOC_DMA -     Only set for resource buffer DMA uploads for winsys
 *                      implementations that want to track the amount
 *                      of such data referenced in the command stream.
 */
#define SVGA_RELOC_WRITE          (1 << 0)
#define SVGA_RELOC_READ           (1 << 1)
#define SVGA_RELOC_INTERNAL       (1 << 2)
#define SVGA_RELOC_DMA            (1 << 3)

#define SVGA_FENCE_FLAG_EXEC      (1 << 0)
#define SVGA_FENCE_FLAG_QUERY     (1 << 1)

#define SVGA_SURFACE_USAGE_SHARED   (1 << 0)
#define SVGA_SURFACE_USAGE_SCANOUT  (1 << 1)
#define SVGA_SURFACE_USAGE_COHERENT (1 << 2)

#define SVGA_QUERY_FLAG_SET        (1 << 0)
#define SVGA_QUERY_FLAG_REF        (1 << 1)

#define SVGA_HINT_FLAG_CAN_PRE_FLUSH   (1 << 0)  /* Can preemptively flush */
#define SVGA_HINT_FLAG_EXPORT_FENCE_FD (1 << 1)  /* Export a Fence FD */

/**
 * SVGA mks statistics info
 */
struct svga_winsys_stats_timeframe {
   void *counterTime;
   uint64 startTime;
   uint64 adjustedStartTime;
   struct svga_winsys_stats_timeframe *enclosing;

   struct svga_winsys_screen *sws;
   int32 slot;
};

enum svga_stats_count {
   SVGA_STATS_COUNT_BLENDSTATE,
   SVGA_STATS_COUNT_BLITBLITTERCOPY,
   SVGA_STATS_COUNT_DEPTHSTENCILSTATE,
   SVGA_STATS_COUNT_RASTERIZERSTATE,
   SVGA_STATS_COUNT_RAWBUFFERSRVIEW,
   SVGA_STATS_COUNT_SAMPLER,
   SVGA_STATS_COUNT_SAMPLERVIEW,
   SVGA_STATS_COUNT_SURFACEWRITEFLUSH,
   SVGA_STATS_COUNT_TEXREADBACK,
   SVGA_STATS_COUNT_VERTEXELEMENT,
   SVGA_STATS_COUNT_MAX
};

enum svga_stats_time {
   SVGA_STATS_TIME_BLIT,
   SVGA_STATS_TIME_BLITBLITTER,
   SVGA_STATS_TIME_BLITFALLBACK,
   SVGA_STATS_TIME_BUFFERSFLUSH,
   SVGA_STATS_TIME_BUFFERTRANSFERMAP,
   SVGA_STATS_TIME_BUFFERTRANSFERUNMAP,
   SVGA_STATS_TIME_CONTEXTFINISH,
   SVGA_STATS_TIME_CONTEXTFLUSH,
   SVGA_STATS_TIME_COPYREGION,
   SVGA_STATS_TIME_COPYREGIONFALLBACK,
   SVGA_STATS_TIME_CREATEBACKEDSURFACEVIEW,
   SVGA_STATS_TIME_CREATEBUFFER,
   SVGA_STATS_TIME_CREATECONTEXT,
   SVGA_STATS_TIME_CREATECS,
   SVGA_STATS_TIME_CREATEFS,
   SVGA_STATS_TIME_CREATEGS,
   SVGA_STATS_TIME_CREATESURFACE,
   SVGA_STATS_TIME_CREATESURFACEVIEW,
   SVGA_STATS_TIME_CREATETCS,
   SVGA_STATS_TIME_CREATETES,
   SVGA_STATS_TIME_CREATETEXTURE,
   SVGA_STATS_TIME_CREATEVS,
   SVGA_STATS_TIME_DEFINESHADER,
   SVGA_STATS_TIME_DESTROYSURFACE,
   SVGA_STATS_TIME_DRAWVBO,
   SVGA_STATS_TIME_DRAWARRAYS,
   SVGA_STATS_TIME_DRAWELEMENTS,
   SVGA_STATS_TIME_EMITCS,
   SVGA_STATS_TIME_EMITFS,
   SVGA_STATS_TIME_EMITGS,
   SVGA_STATS_TIME_EMITRAWBUFFER,
   SVGA_STATS_TIME_EMITTCS,
   SVGA_STATS_TIME_EMITTES,
   SVGA_STATS_TIME_EMITVS,
   SVGA_STATS_TIME_EMULATESURFACEVIEW,
   SVGA_STATS_TIME_FENCEFINISH,
   SVGA_STATS_TIME_GENERATEINDICES,
   SVGA_STATS_TIME_HWTNLDRAWARRAYS,
   SVGA_STATS_TIME_HWTNLDRAWELEMENTS,
   SVGA_STATS_TIME_HWTNLFLUSH,
   SVGA_STATS_TIME_HWTNLPRIM,
   SVGA_STATS_TIME_LAUNCHGRID,
   SVGA_STATS_TIME_PROPAGATESURFACE,
   SVGA_STATS_TIME_SETSAMPLERVIEWS,
   SVGA_STATS_TIME_SURFACEFLUSH,
   SVGA_STATS_TIME_SWTNLDRAWVBO,
   SVGA_STATS_TIME_SWTNLUPDATEDRAW,
   SVGA_STATS_TIME_SWTNLUPDATEVDECL,
   SVGA_STATS_TIME_TEXTRANSFERMAP,
   SVGA_STATS_TIME_TEXTRANSFERUNMAP,
   SVGA_STATS_TIME_TGSIVGPU10TRANSLATE,
   SVGA_STATS_TIME_TGSIVGPU9TRANSLATE,
   SVGA_STATS_TIME_UPDATECSUAV,
   SVGA_STATS_TIME_UPDATESTATE,
   SVGA_STATS_TIME_UPDATEUAV,
   SVGA_STATS_TIME_VALIDATESURFACEVIEW,
   SVGA_STATS_TIME_VBUFDRAWARRAYS,
   SVGA_STATS_TIME_VBUFDRAWELEMENTS,
   SVGA_STATS_TIME_VBUFRENDERALLOCVERT,
   SVGA_STATS_TIME_VBUFRENDERMAPVERT,
   SVGA_STATS_TIME_VBUFRENDERUNMAPVERT,
   SVGA_STATS_TIME_VBUFSUBMITSTATE,
   SVGA_STATS_TIME_MAX
};

#define SVGA_STATS_PREFIX "GuestGL_"

#define SVGA_STATS_COUNT_NAMES                \
   SVGA_STATS_PREFIX "BlendState",            \
   SVGA_STATS_PREFIX "BlitBlitterCopy",       \
   SVGA_STATS_PREFIX "DepthStencilState",     \
   SVGA_STATS_PREFIX "RasterizerState",       \
   SVGA_STATS_PREFIX "RawBufferSRView",       \
   SVGA_STATS_PREFIX "Sampler",               \
   SVGA_STATS_PREFIX "SamplerView",           \
   SVGA_STATS_PREFIX "SurfaceWriteFlush",     \
   SVGA_STATS_PREFIX "TextureReadback",       \
   SVGA_STATS_PREFIX "VertexElement"          \

#define SVGA_STATS_TIME_NAMES                       \
   SVGA_STATS_PREFIX "Blit",                        \
   SVGA_STATS_PREFIX "BlitBlitter",                 \
   SVGA_STATS_PREFIX "BlitFallback",                \
   SVGA_STATS_PREFIX "BuffersFlush",                \
   SVGA_STATS_PREFIX "BufferTransferMap",           \
   SVGA_STATS_PREFIX "BufferTransferUnmap",         \
   SVGA_STATS_PREFIX "ContextFinish",               \
   SVGA_STATS_PREFIX "ContextFlush",                \
   SVGA_STATS_PREFIX "CopyRegion",                  \
   SVGA_STATS_PREFIX "CopyRegionFallback",          \
   SVGA_STATS_PREFIX "CreateBackedSurfaceView",     \
   SVGA_STATS_PREFIX "CreateBuffer",                \
   SVGA_STATS_PREFIX "CreateContext",               \
   SVGA_STATS_PREFIX "CreateCS",                    \
   SVGA_STATS_PREFIX "CreateFS",                    \
   SVGA_STATS_PREFIX "CreateGS",                    \
   SVGA_STATS_PREFIX "CreateSurface",               \
   SVGA_STATS_PREFIX "CreateSurfaceView",           \
   SVGA_STATS_PREFIX "CreateTCS",                   \
   SVGA_STATS_PREFIX "CreateTES",                   \
   SVGA_STATS_PREFIX "CreateTexture",               \
   SVGA_STATS_PREFIX "CreateVS",                    \
   SVGA_STATS_PREFIX "DefineShader",                \
   SVGA_STATS_PREFIX "DestroySurface",              \
   SVGA_STATS_PREFIX "DrawVBO",                     \
   SVGA_STATS_PREFIX "DrawArrays",                  \
   SVGA_STATS_PREFIX "DrawElements",                \
   SVGA_STATS_PREFIX "EmitCS",                      \
   SVGA_STATS_PREFIX "EmitFS",                      \
   SVGA_STATS_PREFIX "EmitGS",                      \
   SVGA_STATS_PREFIX "EmitRawBuffer",               \
   SVGA_STATS_PREFIX "EmitTCS",                     \
   SVGA_STATS_PREFIX "EmitTES",                     \
   SVGA_STATS_PREFIX "EmitVS",                      \
   SVGA_STATS_PREFIX "EmulateSurfaceView",          \
   SVGA_STATS_PREFIX "FenceFinish",                 \
   SVGA_STATS_PREFIX "GenerateIndices",             \
   SVGA_STATS_PREFIX "HWtnlDrawArrays",             \
   SVGA_STATS_PREFIX "HWtnlDrawElements",           \
   SVGA_STATS_PREFIX "HWtnlFlush",                  \
   SVGA_STATS_PREFIX "HWtnlPrim",                   \
   SVGA_STATS_PREFIX "LaunchGrid",                  \
   SVGA_STATS_PREFIX "PropagateSurface",            \
   SVGA_STATS_PREFIX "SetSamplerViews",             \
   SVGA_STATS_PREFIX "SurfaceFlush",                \
   SVGA_STATS_PREFIX "SwtnlDrawVBO",                \
   SVGA_STATS_PREFIX "SwtnlUpdateDraw",             \
   SVGA_STATS_PREFIX "SwtnlUpdateVDecl",            \
   SVGA_STATS_PREFIX "TextureTransferMap",          \
   SVGA_STATS_PREFIX "TextureTransferUnmap",        \
   SVGA_STATS_PREFIX "TGSIVGPU10Translate",         \
   SVGA_STATS_PREFIX "TGSIVGPU9Translate",          \
   SVGA_STATS_PREFIX "UpdateCSUAV",                 \
   SVGA_STATS_PREFIX "UpdateState",                 \
   SVGA_STATS_PREFIX "UpdateUAV",                   \
   SVGA_STATS_PREFIX "ValidateSurfaceView",         \
   SVGA_STATS_PREFIX "VbufDrawArrays",              \
   SVGA_STATS_PREFIX "VbufDrawElements",            \
   SVGA_STATS_PREFIX "VbufRenderAllocVertices",     \
   SVGA_STATS_PREFIX "VbufRenderMapVertices",       \
   SVGA_STATS_PREFIX "VbufRenderUnmapVertices",     \
   SVGA_STATS_PREFIX "VbufSubmitState"


/** Opaque surface handle */
struct svga_winsys_surface;

/** Opaque guest-backed objects */
struct svga_winsys_gb_shader;
struct svga_winsys_gb_query;


/**
 * SVGA per-context winsys interface.
 */
struct svga_winsys_context
{
   void
   (*destroy)(struct svga_winsys_context *swc);

   void *
   (*reserve)(struct svga_winsys_context *swc,
              uint32_t nr_bytes, uint32_t nr_relocs );

   /**
    * Returns current size of command buffer, in bytes.
    */
   unsigned
   (*get_command_buffer_size)(struct svga_winsys_context *swc);

   /**
    * Emit a relocation for a host surface.
    *
    * @param flags bitmask of SVGA_RELOC_* flags
    *
    * NOTE: Order of this call does matter. It should be the same order
    * as relocations appear in the command buffer.
    */
   void
   (*surface_relocation)(struct svga_winsys_context *swc,
                         uint32 *sid,
                         uint32 *mobid,
                         struct svga_winsys_surface *surface,
                         unsigned flags);

   /**
    * Emit a relocation for a guest memory region.
    *
    * @param flags bitmask of SVGA_RELOC_* flags
    *
    * NOTE: Order of this call does matter. It should be the same order
    * as relocations appear in the command buffer.
    */
   void
   (*region_relocation)(struct svga_winsys_context *swc,
                        struct SVGAGuestPtr *ptr,
                        struct svga_winsys_buffer *buffer,
                        uint32 offset,
                        unsigned flags);

   /**
    * Emit a relocation for a guest-backed shader object.
    *
    * NOTE: Order of this call does matter. It should be the same order
    * as relocations appear in the command buffer.
    */
   void
   (*shader_relocation)(struct svga_winsys_context *swc,
                        uint32 *shid,
                        uint32 *mobid,
                        uint32 *offset,
                        struct svga_winsys_gb_shader *shader,
                        unsigned flags);

   /**
    * Emit a relocation for a guest-backed context.
    *
    * NOTE: Order of this call does matter. It should be the same order
    * as relocations appear in the command buffer.
    */
   void
   (*context_relocation)(struct svga_winsys_context *swc, uint32 *cid);

   /**
    * Emit a relocation for a guest Memory OBject.
    *
    * @param flags bitmask of SVGA_RELOC_* flags
    * @param offset_into_mob Buffer starts at this offset into the MOB.
    *
    * Note that not all commands accept an offset into the MOB and
    * those commands can't use suballocated buffer pools. To trap
    * errors from improper buffer pool usage, set the offset_into_mob
    * pointer to NULL.
    */
   void
   (*mob_relocation)(struct svga_winsys_context *swc,
                     SVGAMobId *id,
                     uint32 *offset_into_mob,
                     struct svga_winsys_buffer *buffer,
                     uint32 offset,
                     unsigned flags);

   /**
    * Emit a relocation for a guest-backed query object.
    *
    * NOTE: Order of this call does matter. It should be the same order
    * as relocations appear in the command buffer.
    */
   void
   (*query_relocation)(struct svga_winsys_context *swc,
                       SVGAMobId *id,
                       struct svga_winsys_gb_query *query);

   /**
    * Bind queries to context.
    * \param flags  exactly one of SVGA_QUERY_FLAG_SET/REF
    */
   enum pipe_error
   (*query_bind)(struct svga_winsys_context *sws,
                 struct svga_winsys_gb_query *query,
                 unsigned flags);

   void
   (*commit)(struct svga_winsys_context *swc);

   enum pipe_error
   (*flush)(struct svga_winsys_context *swc,
            struct pipe_fence_handle **pfence);

   /**
    * Context ID used to fill in the commands
    *
    * Context IDs are arbitrary small non-negative integers,
    * global to the entire SVGA device.
    */
   uint32 cid;

   /**
    * Flags to hint the current context state
    */
   uint32 hints;

   /**
    * File descriptor for imported fence
    */
   int32 imported_fence_fd;

   /**
    ** BEGIN new functions for guest-backed surfaces.
    **/

   bool have_gb_objects;
   bool force_coherent;

   /**
    * Map a guest-backed surface.
    * \param swc The winsys context
    * \param surface The surface to map
    * \param flags  bitmask of PIPE_MAP_x flags
    * \param retry Whether to flush and retry the map
    * \param rebind Whether to issue an immediate rebind and flush.
    *
    * The surface_map() member is allowed to fail due to a
    * shortage of command buffer space, if the
    * PIPE_MAP_DISCARD_WHOLE_RESOURCE bit is set in flags.
    * In that case, the caller must flush the current command
    * buffer and reissue the map.
    */
   void *
   (*surface_map)(struct svga_winsys_context *swc,
                  struct svga_winsys_surface *surface,
                  unsigned flags, bool *retry,
                  bool *rebind);

   /**
    * Unmap a guest-backed surface.
    * \param rebind  returns a flag indicating whether the caller should
    *                issue a SVGA3D_BindGBSurface() call.
    */
   void
   (*surface_unmap)(struct svga_winsys_context *swc,
                    struct svga_winsys_surface *surface,
                    bool *rebind);

   /**
    * Create and define a DX GB shader that resides in the device COTable.
    * Caller of this function will issue the DXDefineShader command.
    */
   struct svga_winsys_gb_shader *
   (*shader_create)(struct svga_winsys_context *swc,
                    uint32 shaderId,
                    SVGA3dShaderType shaderType,
                    const uint32 *bytecode,
                    uint32 bytecodeLen,
                    const SVGA3dDXShaderSignatureHeader *sgnInfo,
                    uint32 sgnLen);

   /**
    * Destroy a DX GB shader.
    * This function will issue the DXDestroyShader command.
    */
   void
   (*shader_destroy)(struct svga_winsys_context *swc,
                     struct svga_winsys_gb_shader *shader);

   /**
    * Rebind a DX GB resource to a context.
    * This is called to reference a DX GB resource in the command stream in
    * order to page in the associated resource in case the memory has been
    * paged out, and to fence it if necessary after command submission.
    */
   enum pipe_error
   (*resource_rebind)(struct svga_winsys_context *swc,
                      struct svga_winsys_surface *surface,
                      struct svga_winsys_gb_shader *shader,
                      unsigned flags);

   /** To report perf/conformance/etc issues to the gallium frontend */
   struct util_debug_callback *debug_callback;

   /** The more recent command issued to command buffer */
   SVGAFifo3dCmdId last_command;

   /** For HUD queries */
   uint64_t num_commands;
   uint64_t num_command_buffers;
   uint64_t num_draw_commands;
   uint64_t num_shader_reloc;
   uint64_t num_surf_reloc;

   /* Whether we are in retry processing */
   unsigned int in_retry;
};


/**
 * SVGA per-screen winsys interface.
 */
struct svga_winsys_screen
{
   void
   (*destroy)(struct svga_winsys_screen *sws);

   SVGA3dHardwareVersion
   (*get_hw_version)(struct svga_winsys_screen *sws);

   int
   (*get_fd)(struct svga_winsys_screen *sws);

   bool
   (*get_cap)(struct svga_winsys_screen *sws,
              SVGA3dDevCapIndex index,
              SVGA3dDevCapResult *result);

   /**
    * Create a new context.
    *
    * Context objects encapsulate all render state, and shader
    * objects are per-context.
    *
    * Surfaces are not per-context. The same surface can be shared
    * between multiple contexts, and surface operations can occur
    * without a context.
    */
   struct svga_winsys_context *
   (*context_create)(struct svga_winsys_screen *sws);

   /**
    * This creates a "surface" object in the SVGA3D device.
    *
    * \param sws Pointer to an svga_winsys_context
    * \param flags Device surface create flags
    * \param format Format Device surface format
    * \param usage Winsys usage: bitmask of SVGA_SURFACE_USAGE_x flags
    * \param size Surface size given in device format
    * \param numLayers Number of layers of the surface (or cube faces)
    * \param numMipLevels Number of mipmap levels for each face
    *
    * Returns the surface ID (sid). Surfaces are generic
    * containers for host VRAM objects like textures, vertex
    * buffers, and depth/stencil buffers.
    *
    * Surfaces are hierarchial:
    *
    * - Surface may have multiple faces (for cube maps)
    *
    * - Each face has a list of mipmap levels
    *
    * - Each mipmap image may have multiple volume
    *   slices for 3D image, or multiple 2D slices for texture array.
    *
    * - Each slice is a 2D array of 'blocks'
    *
    * - Each block may be one or more pixels.
    *   (Usually 1, more for DXT or YUV formats.)
    *
    * Surfaces are generic host VRAM objects. The SVGA3D device
    * may optimize surfaces according to the format they were
    * created with, but this format does not limit the ways in
    * which the surface may be used. For example, a depth surface
    * can be used as a texture, or a floating point image may
    * be used as a vertex buffer. Some surface usages may be
    * lower performance, due to software emulation, but any
    * usage should work with any surface.
    */
   struct svga_winsys_surface *
   (*surface_create)(struct svga_winsys_screen *sws,
                     SVGA3dSurfaceAllFlags flags,
                     SVGA3dSurfaceFormat format,
                     unsigned usage,
                     SVGA3dSize size,
                     uint32 numLayers,
                     uint32 numMipLevels,
                     unsigned sampleCount);

   /**
    * Creates a surface from a winsys handle.
    * Used to implement pipe_screen::resource_from_handle.
    */
   struct svga_winsys_surface *
   (*surface_from_handle)(struct svga_winsys_screen *sws,
                          struct winsys_handle *whandle,
                          SVGA3dSurfaceFormat *format);

   /**
    * Get a winsys_handle from a surface.
    * Used to implement pipe_screen::resource_get_handle.
    */
   bool
   (*surface_get_handle)(struct svga_winsys_screen *sws,
                         struct svga_winsys_surface *surface,
                         unsigned stride,
                         struct winsys_handle *whandle);

   /**
    * Whether this surface is sitting in a validate list
    */
   bool
   (*surface_is_flushed)(struct svga_winsys_screen *sws,
                         struct svga_winsys_surface *surface);

   /**
    * Reference a SVGA3D surface object. This allows sharing of a
    * surface between different objects.
    */
   void
   (*surface_reference)(struct svga_winsys_screen *sws,
                        struct svga_winsys_surface **pdst,
                        struct svga_winsys_surface *src);

   /**
    * Check if a resource (texture, buffer) of the given size
    * and format can be created.
    * \Return TRUE if OK, FALSE if too large.
    */
   bool
   (*surface_can_create)(struct svga_winsys_screen *sws,
                         SVGA3dSurfaceFormat format,
                         SVGA3dSize size,
                         uint32 numLayers,
                         uint32 numMipLevels,
                         uint32 numSamples);

   void
   (*surface_init)(struct svga_winsys_screen *sws,
                   struct svga_winsys_surface *surface,
                   unsigned surf_size, SVGA3dSurfaceAllFlags flags);

   /**
    * Buffer management. Buffer attributes are mostly fixed over its lifetime.
    *
    * @param usage bitmask of SVGA_BUFFER_USAGE_* flags.
    *
    * alignment indicates the client's alignment requirements, eg for
    * SSE instructions.
    */
   struct svga_winsys_buffer *
   (*buffer_create)( struct svga_winsys_screen *sws,
                     unsigned alignment,
                     unsigned usage,
                     unsigned size );

   /**
    * Map the entire data store of a buffer object into the client's address.
    * usage is a bitmask of PIPE_MAP_*
    */
   void *
   (*buffer_map)( struct svga_winsys_screen *sws,
                  struct svga_winsys_buffer *buf,
                  unsigned usage );

   void
   (*buffer_unmap)( struct svga_winsys_screen *sws,
                    struct svga_winsys_buffer *buf );

   void
   (*buffer_destroy)( struct svga_winsys_screen *sws,
                      struct svga_winsys_buffer *buf );


   /**
    * Reference a fence object.
    */
   void
   (*fence_reference)( struct svga_winsys_screen *sws,
                       struct pipe_fence_handle **pdst,
                       struct pipe_fence_handle *src );

   /**
    * Checks whether the fence has been signalled.
    * \param flags  driver-specific meaning
    * \return zero on success.
    */
   int (*fence_signalled)( struct svga_winsys_screen *sws,
                           struct pipe_fence_handle *fence,
                           unsigned flag );

   /**
    * Wait for the fence to finish.
    * \param timeout in nanoseconds (may be OS_TIMEOUT_INFINITE).
    *                0 to return immediately, if the API suports it.
    * \param flags  driver-specific meaning
    * \return zero on success.
    */
   int (*fence_finish)( struct svga_winsys_screen *sws,
                        struct pipe_fence_handle *fence,
                        uint64_t timeout,
                        unsigned flag );

   /**
    * Get the file descriptor associated with the fence
    * \param duplicate duplicate the fd before returning it
    * \return zero on success.
    */
   int (*fence_get_fd)( struct svga_winsys_screen *sws,
                        struct pipe_fence_handle *fence,
                        bool duplicate );

   /**
    * Create a fence using the given file descriptor
    * \return zero on success.
    */
   void (*fence_create_fd)( struct svga_winsys_screen *sws,
                            struct pipe_fence_handle **fence,
                            int32_t fd );

   /**
    * Accumulates fence FD from other devices into the current context
    * \param context_fd FD the context will be waiting on
    * \return zero on success
    */
   int (*fence_server_sync)( struct svga_winsys_screen *sws,
                             int32_t *context_fd,
                             struct pipe_fence_handle *fence );

   /**
    ** BEGIN new functions for guest-backed surfaces.
    **/

   /** Are guest-backed objects enabled? */
   bool have_gb_objects;

   /** Can we do DMA with guest-backed objects enabled? */
   bool have_gb_dma;

   /** Do we support coherent surface memory? */
   bool have_coherent;
   /**
    * Create and define a GB shader.
    */
   struct svga_winsys_gb_shader *
   (*shader_create)(struct svga_winsys_screen *sws,
                    SVGA3dShaderType shaderType,
                    const uint32 *bytecode,
                    uint32 bytecodeLen);

   /**
    * Destroy a GB shader. It's safe to call this function even
    * if the shader is referenced in a context's command stream.
    */
   void
   (*shader_destroy)(struct svga_winsys_screen *sws,
                     struct svga_winsys_gb_shader *shader);

   /**
    * Create and define a GB query.
    */
   struct svga_winsys_gb_query *
   (*query_create)(struct svga_winsys_screen *sws, uint32 len);

   /**
    * Destroy a GB query.
    */
   void
   (*query_destroy)(struct svga_winsys_screen *sws,
                    struct svga_winsys_gb_query *query);

   /**
    * Initialize the query state of the query that resides in the slot
    * specified in offset
    * \return zero on success.
    */
   int
   (*query_init)(struct svga_winsys_screen *sws,
                       struct svga_winsys_gb_query *query,
                       unsigned offset,
                       SVGA3dQueryState queryState);

   /**
    * Inquire for the query state and result of the query that resides
    * in the slot specified in offset
    */
   void
   (*query_get_result)(struct svga_winsys_screen *sws,
                       struct svga_winsys_gb_query *query,
                       unsigned offset,
                       SVGA3dQueryState *queryState,
                       void *result, uint32 resultLen);

   /**
    * Increment a statistic counter
    */
   void
   (*stats_inc)(struct svga_winsys_screen *, enum svga_stats_count);

   /**
    * Push a time frame onto the stack
    */
   void
   (*stats_time_push)(struct svga_winsys_screen *, enum svga_stats_time, struct svga_winsys_stats_timeframe *);

   /**
    * Pop a time frame.
    */
   void
   (*stats_time_pop)(struct svga_winsys_screen *);

   /**
    * Send a host log message
    */
   void
   (*host_log)(struct svga_winsys_screen *sws, const char *message);

   /** Have VGPU v10 hardware? */
   bool have_vgpu10;

   /** Have SM4_1 hardware? */
   bool have_sm4_1;

   /** Have SM5 hardware? */
   bool have_sm5;

   /** To rebind resources at the beginning of a new command buffer */
   bool need_to_rebind_resources;

   bool have_generate_mipmap_cmd;
   bool have_set_predication_cmd;
   bool have_transfer_from_buffer_cmd;
   bool have_fence_fd;
   bool have_intra_surface_copy;
   bool have_constant_buffer_offset_cmd;
   bool have_index_vertex_buffer_offset_cmd;

   /* Have rasterizer state v2 command support */
   bool have_rasterizer_state_v2_cmd;

   /** Have GL43 capable device */
   bool have_gl43;

   /** SVGA device_id version we're running on */
   uint16_t device_id;
};


struct svga_winsys_screen *
svga_winsys_screen(struct pipe_screen *screen);

struct svga_winsys_context *
svga_winsys_context(struct pipe_context *context);

struct pipe_resource *
svga_screen_buffer_wrap_surface(struct pipe_screen *screen,
                                enum SVGA3dSurfaceFormat format,
                                struct svga_winsys_surface *srf);

struct svga_winsys_surface *
svga_screen_buffer_get_winsys_surface(struct pipe_resource *buffer);

#endif /* SVGA_WINSYS_H_ */
