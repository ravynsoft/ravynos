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
 * svga_cmd.c --
 *
 *      Command construction utility for the SVGA3D protocol used by
 *      the VMware SVGA device, based on the svgautil library.
 */

#include "svga_winsys.h"
#include "svga_resource_buffer.h"
#include "svga_resource_texture.h"
#include "svga_surface.h"
#include "svga_cmd.h"

/*
 *----------------------------------------------------------------------
 *
 * surface_to_surfaceid --
 *
 *      Utility function for surface ids.
 *      Can handle null surface. Does a surface_reallocation so you need
 *      to have allocated the fifo space before converting.
 *
 *
 * param flags  mask of SVGA_RELOC_READ / _WRITE
 *
 * Results:
 *      id is filled out.
 *
 * Side effects:
 *      One surface relocation is performed for texture handle.
 *
 *----------------------------------------------------------------------
 */

static inline void
surface_to_surfaceid(struct svga_winsys_context *swc, // IN
                     struct pipe_surface *surface,    // IN
                     SVGA3dSurfaceImageId *id,        // OUT
                     unsigned flags)                  // IN
{
   if (surface) {
      struct svga_surface *s = svga_surface(surface);
      swc->surface_relocation(swc, &id->sid, NULL, s->handle, flags);
      id->face = s->real_layer; /* faces have the same order */
      id->mipmap = s->real_level;
   }
   else {
      swc->surface_relocation(swc, &id->sid, NULL, NULL, flags);
      id->face = 0;
      id->mipmap = 0;
   }
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_FIFOReserve --
 *
 *      Reserve space for an SVGA3D FIFO command.
 *
 *      The 2D SVGA commands have been around for a while, so they
 *      have a rather asymmetric structure. The SVGA3D protocol is
 *      more uniform: each command begins with a header containing the
 *      command number and the full size.
 *
 *      This is a convenience wrapper around SVGA_FIFOReserve. We
 *      reserve space for the whole command, and write the header.
 *
 *      This function must be paired with SVGA_FIFOCommitAll().
 *
 * Results:
 *      Returns a pointer to the space reserved for command-specific
 *      data. It must be 'cmdSize' bytes long.
 *
 * Side effects:
 *      Begins a FIFO reservation.
 *
 *----------------------------------------------------------------------
 */

void *
SVGA3D_FIFOReserve(struct svga_winsys_context *swc,
                   uint32 cmd,       // IN
                   uint32 cmdSize,   // IN
                   uint32 nr_relocs) // IN
{
   SVGA3dCmdHeader *header;

   header = swc->reserve(swc, sizeof *header + cmdSize, nr_relocs);
   if (!header)
      return NULL;

   header->id = cmd;
   header->size = cmdSize;

   swc->last_command = cmd;

   swc->num_commands++;

   return &header[1];
}


void
SVGA_FIFOCommitAll(struct svga_winsys_context *swc)
{
   swc->commit(swc);
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_DefineContext --
 *
 *      Create a new context, to be referred to with the provided ID.
 *
 *      Context objects encapsulate all render state, and shader
 *      objects are per-context.
 *
 *      Surfaces are not per-context. The same surface can be shared
 *      between multiple contexts, and surface operations can occur
 *      without a context.
 *
 *      If the provided context ID already existed, it is redefined.
 *
 *      Context IDs are arbitrary small non-negative integers,
 *      global to the entire SVGA device.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_DefineContext(struct svga_winsys_context *swc)  // IN
{
   SVGA3dCmdDefineContext *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_CONTEXT_DEFINE, sizeof *cmd, 0);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;

   swc->commit(swc);

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_DestroyContext --
 *
 *      Delete a context created with SVGA3D_DefineContext.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_DestroyContext(struct svga_winsys_context *swc)  // IN
{
   SVGA3dCmdDestroyContext *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_CONTEXT_DESTROY, sizeof *cmd, 0);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;

   swc->commit(swc);

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_BeginDefineSurface --
 *
 *      Begin a SURFACE_DEFINE command. This reserves space for it in
 *      the FIFO, and returns pointers to the command's faces and
 *      mipsizes arrays.
 *
 *      This function must be paired with SVGA_FIFOCommitAll().
 *      The faces and mipSizes arrays are initialized to zero.
 *
 *      This creates a "surface" object in the SVGA3D device,
 *      with the provided surface ID (sid). Surfaces are generic
 *      containers for host VRAM objects like textures, vertex
 *      buffers, and depth/stencil buffers.
 *
 *      Surfaces are hierarchical:
 *
 *        - Surface may have multiple faces (for cube maps)
 *
 *          - Each face has a list of mipmap levels
 *
 *             - Each mipmap image may have multiple volume
 *               slices, if the image is three dimensional.
 *
 *                - Each slice is a 2D array of 'blocks'
 *
 *                   - Each block may be one or more pixels.
 *                     (Usually 1, more for DXT or YUV formats.)
 *
 *      Surfaces are generic host VRAM objects. The SVGA3D device
 *      may optimize surfaces according to the format they were
 *      created with, but this format does not limit the ways in
 *      which the surface may be used. For example, a depth surface
 *      can be used as a texture, or a floating point image may
 *      be used as a vertex buffer. Some surface usages may be
 *      lower performance, due to software emulation, but any
 *      usage should work with any surface.
 *
 *      If 'sid' is already defined, the old surface is deleted
 *      and this new surface replaces it.
 *
 *      Surface IDs are arbitrary small non-negative integers,
 *      global to the entire SVGA device.
 *
 * Results:
 *      Returns pointers to arrays allocated in the FIFO for 'faces'
 *      and 'mipSizes'.
 *
 * Side effects:
 *      Begins a FIFO reservation.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_BeginDefineSurface(struct svga_winsys_context *swc,
                          struct svga_winsys_surface *sid, // IN
                          SVGA3dSurface1Flags flags,    // IN
                          SVGA3dSurfaceFormat format,  // IN
                          SVGA3dSurfaceFace **faces,   // OUT
                          SVGA3dSize **mipSizes,       // OUT
                          uint32 numMipSizes)          // IN
{
   SVGA3dCmdDefineSurface *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SURFACE_DEFINE, sizeof *cmd +
                            sizeof **mipSizes * numMipSizes, 1);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   swc->surface_relocation(swc, &cmd->sid, NULL, sid,
                           SVGA_RELOC_WRITE | SVGA_RELOC_INTERNAL);
   cmd->surfaceFlags = flags;
   cmd->format = format;

   *faces = &cmd->face[0];
   *mipSizes = (SVGA3dSize*) &cmd[1];

   memset(*faces, 0, sizeof **faces * SVGA3D_MAX_SURFACE_FACES);
   memset(*mipSizes, 0, sizeof **mipSizes * numMipSizes);

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_DefineSurface2D --
 *
 *      This is a simplified version of SVGA3D_BeginDefineSurface(),
 *      which does not support cube maps, mipmaps, or volume textures.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_DefineSurface2D(struct svga_winsys_context *swc,    // IN
                       struct svga_winsys_surface *sid, // IN
                       uint32 width,                // IN
                       uint32 height,               // IN
                       SVGA3dSurfaceFormat format)  // IN
{
   SVGA3dSize *mipSizes;
   SVGA3dSurfaceFace *faces;
   enum pipe_error ret;

   ret = SVGA3D_BeginDefineSurface(swc,
                                   sid, 0, format, &faces, &mipSizes, 1);
   if (ret != PIPE_OK)
      return ret;

   faces[0].numMipLevels = 1;

   mipSizes[0].width = width;
   mipSizes[0].height = height;
   mipSizes[0].depth = 1;

   swc->commit(swc);

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_DestroySurface --
 *
 *      Release the host VRAM encapsulated by a particular surface ID.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_DestroySurface(struct svga_winsys_context *swc,
                      struct svga_winsys_surface *sid)  // IN
{
   SVGA3dCmdDestroySurface *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SURFACE_DESTROY, sizeof *cmd, 1);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;
   
   swc->surface_relocation(swc, &cmd->sid, NULL, sid,
                           SVGA_RELOC_WRITE | SVGA_RELOC_INTERNAL);
   swc->commit(swc);

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_SurfaceDMA--
 *
 *      Emit a SURFACE_DMA command.
 *
 *      When the SVGA3D device asynchronously processes this FIFO
 *      command, a DMA operation is performed between host VRAM and
 *      a generic SVGAGuestPtr. The guest pointer may refer to guest
 *      VRAM (provided by the SVGA PCI device) or to guest system
 *      memory that has been set up as a Guest Memory Region (GMR)
 *      by the SVGA device.
 *
 *      The guest's DMA buffer must remain valid (not freed, paged out,
 *      or overwritten) until the host has finished processing this
 *      command. The guest can determine that the host has finished
 *      by using the SVGA device's FIFO Fence mechanism.
 *
 *      The guest's image buffer can be an arbitrary size and shape.
 *      Guest image data is interpreted according to the SVGA3D surface
 *      format specified when the surface was defined.
 *
 *      The caller may optionally define the guest image's pitch.
 *      guestImage->pitch can either be zero (assume image is tightly
 *      packed) or it must be the number of bytes between vertically
 *      adjacent image blocks.
 *
 *      The provided copybox list specifies which regions of the source
 *      image are to be copied, and where they appear on the destination.
 *
 *      NOTE: srcx/srcy are always on the guest image and x/y are
 *      always on the host image, regardless of the actual transfer
 *      direction!
 *
 *      For efficiency, the SVGA3D device is free to copy more data
 *      than specified. For example, it may round copy boxes outwards
 *      such that they lie on particular alignment boundaries.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_SurfaceDMA(struct svga_winsys_context *swc,
                  struct svga_transfer *st,         // IN
                  SVGA3dTransferType transfer,      // IN
                  const SVGA3dCopyBox *boxes,       // IN
                  uint32 numBoxes,                  // IN
                  SVGA3dSurfaceDMAFlags flags)      // IN
{
   struct svga_texture *texture = svga_texture(st->base.resource);
   SVGA3dCmdSurfaceDMA *cmd;
   SVGA3dCmdSurfaceDMASuffix *pSuffix;
   uint32 boxesSize = sizeof *boxes * numBoxes;
   unsigned region_flags;
   unsigned surface_flags;

   assert(!swc->have_gb_objects);

   if (transfer == SVGA3D_WRITE_HOST_VRAM) {
      region_flags = SVGA_RELOC_READ;
      surface_flags = SVGA_RELOC_WRITE;
   }
   else if (transfer == SVGA3D_READ_HOST_VRAM) {
      region_flags = SVGA_RELOC_WRITE;
      surface_flags = SVGA_RELOC_READ;
   }
   else {
      assert(0);
      return PIPE_ERROR_BAD_INPUT;
   }

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SURFACE_DMA,
                            sizeof *cmd + boxesSize + sizeof *pSuffix,
                            2);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   swc->region_relocation(swc, &cmd->guest.ptr, st->hwbuf, 0, region_flags);
   cmd->guest.pitch = st->base.stride;

   swc->surface_relocation(swc, &cmd->host.sid, NULL,
                           texture->handle, surface_flags);
   cmd->host.face = st->slice; /* PIPE_TEX_FACE_* and SVGA3D_CUBEFACE_* match */
   cmd->host.mipmap = st->base.level;

   cmd->transfer = transfer;

   memcpy(&cmd[1], boxes, boxesSize);

   pSuffix = (SVGA3dCmdSurfaceDMASuffix *)((uint8_t*)cmd + sizeof *cmd + boxesSize);
   pSuffix->suffixSize = sizeof *pSuffix;
   pSuffix->maximumOffset = st->hw_nblocksy*st->base.stride;
   pSuffix->flags = flags;

   swc->commit(swc);
   swc->hints |= SVGA_HINT_FLAG_CAN_PRE_FLUSH;

   return PIPE_OK;
}


enum pipe_error
SVGA3D_BufferDMA(struct svga_winsys_context *swc,
                 struct svga_winsys_buffer *guest,
                 struct svga_winsys_surface *host,
                 SVGA3dTransferType transfer,      // IN
                 uint32 size,                      // IN
                 uint32 guest_offset,              // IN
                 uint32 host_offset,               // IN
                 SVGA3dSurfaceDMAFlags flags)      // IN
{
   SVGA3dCmdSurfaceDMA *cmd;
   SVGA3dCopyBox *box;
   SVGA3dCmdSurfaceDMASuffix *pSuffix;
   unsigned region_flags;
   unsigned surface_flags;
   
   assert(!swc->have_gb_objects);

   if (transfer == SVGA3D_WRITE_HOST_VRAM) {
      region_flags = SVGA_RELOC_READ;
      surface_flags = SVGA_RELOC_WRITE;
   }
   else if (transfer == SVGA3D_READ_HOST_VRAM) {
      region_flags = SVGA_RELOC_WRITE;
      surface_flags = SVGA_RELOC_READ;
   }
   else {
      assert(0);
      return PIPE_ERROR_BAD_INPUT;
   }

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SURFACE_DMA,
                            sizeof *cmd + sizeof *box + sizeof *pSuffix,
                            2);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   swc->region_relocation(swc, &cmd->guest.ptr, guest, 0, region_flags);
   cmd->guest.pitch = 0;

   swc->surface_relocation(swc, &cmd->host.sid,
                           NULL, host, surface_flags);
   cmd->host.face = 0;
   cmd->host.mipmap = 0;

   cmd->transfer = transfer;

   box = (SVGA3dCopyBox *)&cmd[1];
   box->x = host_offset;
   box->y = 0;
   box->z = 0;
   box->w = size;
   box->h = 1;
   box->d = 1;
   box->srcx = guest_offset;
   box->srcy = 0;
   box->srcz = 0;

   pSuffix = (SVGA3dCmdSurfaceDMASuffix *)((uint8_t*)cmd + sizeof *cmd + sizeof *box);
   pSuffix->suffixSize = sizeof *pSuffix;
   pSuffix->maximumOffset = guest_offset + size;
   pSuffix->flags = flags;

   swc->commit(swc);
   swc->hints |= SVGA_HINT_FLAG_CAN_PRE_FLUSH;

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_SetRenderTarget --
 *
 *      Bind a surface object to a particular render target attachment
 *      point on the current context. Render target attachment points
 *      exist for color buffers, a depth buffer, and a stencil buffer.
 *
 *      The SVGA3D device is quite lenient about the types of surfaces
 *      that may be used as render targets. The color buffers must
 *      all be the same size, but the depth and stencil buffers do not
 *      have to be the same size as the color buffer. All attachments
 *      are optional.
 *
 *      Some combinations of render target formats may require software
 *      emulation, depending on the capabilities of the host graphics
 *      API and graphics hardware.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_SetRenderTarget(struct svga_winsys_context *swc,
                       SVGA3dRenderTargetType type,   // IN
                       struct pipe_surface *surface)  // IN
{
   SVGA3dCmdSetRenderTarget *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SETRENDERTARGET, sizeof *cmd, 1);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->type = type;
   surface_to_surfaceid(swc, surface, &cmd->target, SVGA_RELOC_WRITE);
   swc->commit(swc);

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_DefineShader --
 *
 *      Upload the bytecode for a new shader. The bytecode is "SVGA3D
 *      format", which is theoretically a binary-compatible superset
 *      of Microsoft's DirectX shader bytecode. In practice, the
 *      SVGA3D bytecode doesn't yet have any extensions to DirectX's
 *      bytecode format.
 *
 *      The SVGA3D device supports shader models 1.1 through 2.0.
 *
 *      The caller chooses a shader ID (small positive integer) by
 *      which this shader will be identified in future commands. This
 *      ID is in a namespace which is per-context and per-shader-type.
 *
 *      'bytecodeLen' is specified in bytes. It must be a multiple of 4.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_DefineShader(struct svga_winsys_context *swc,
                    uint32 shid,                  // IN
                    SVGA3dShaderType type,        // IN
                    const uint32 *bytecode,       // IN
                    uint32 bytecodeLen)           // IN
{
   SVGA3dCmdDefineShader *cmd;

   assert(bytecodeLen % 4 == 0);

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SHADER_DEFINE, sizeof *cmd + bytecodeLen,
                            0);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->shid = shid;
   cmd->type = type;
   memcpy(&cmd[1], bytecode, bytecodeLen);
   swc->commit(swc);

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_DestroyShader --
 *
 *      Delete a shader that was created by SVGA3D_DefineShader. If
 *      the shader was the current vertex or pixel shader for its
 *      context, rendering results are undefined until a new shader is
 *      bound.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_DestroyShader(struct svga_winsys_context *swc,
                     uint32 shid,            // IN
                     SVGA3dShaderType type)  // IN
{
   SVGA3dCmdDestroyShader *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SHADER_DESTROY, sizeof *cmd,
                            0);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->shid = shid;
   cmd->type = type;
   swc->commit(swc);

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_SetShaderConst --
 *
 *      Set the value of a shader constant.
 *
 *      Shader constants are analogous to uniform variables in GLSL,
 *      except that they belong to the render context rather than to
 *      an individual shader.
 *
 *      Constants may have one of three types: A 4-vector of floats,
 *      a 4-vector of integers, or a single boolean flag.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_SetShaderConst(struct svga_winsys_context *swc,
                      uint32 reg,                   // IN
                      SVGA3dShaderType type,        // IN
                      SVGA3dShaderConstType ctype,  // IN
                      const void *value)            // IN
{
   SVGA3dCmdSetShaderConst *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SET_SHADER_CONST, sizeof *cmd,
                            0);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->reg = reg;
   cmd->type = type;
   cmd->ctype = ctype;

   switch (ctype) {

   case SVGA3D_CONST_TYPE_FLOAT:
   case SVGA3D_CONST_TYPE_INT:
      memcpy(&cmd->values, value, sizeof cmd->values);
      break;

   case SVGA3D_CONST_TYPE_BOOL:
      memset(&cmd->values, 0, sizeof cmd->values);
      cmd->values[0] = *(uint32*)value;
      break;

   default:
      assert(0);
      break;

   }
   swc->commit(swc);

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_SetShaderConsts --
 *
 *      Set the value of successive shader constants.
 *
 *      Shader constants are analogous to uniform variables in GLSL,
 *      except that they belong to the render context rather than to
 *      an individual shader.
 *
 *      Constants may have one of three types: A 4-vector of floats,
 *      a 4-vector of integers, or a single boolean flag.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_SetShaderConsts(struct svga_winsys_context *swc,
                        uint32 reg,                   // IN
                        uint32 numRegs,               // IN
                        SVGA3dShaderType type,        // IN
                        SVGA3dShaderConstType ctype,  // IN
                        const void *values)           // IN
{
   SVGA3dCmdSetShaderConst *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SET_SHADER_CONST,
                            sizeof *cmd + (numRegs - 1) * sizeof cmd->values,
                            0);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->reg = reg;
   cmd->type = type;
   cmd->ctype = ctype;

   memcpy(&cmd->values, values, numRegs * sizeof cmd->values);

   swc->commit(swc);

   return PIPE_OK;
}





/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_SetShader --
 *
 *      Switch active shaders. This binds a new vertex or pixel shader
 *      to the specified context.
 *
 *      A shader ID of SVGA3D_INVALID_ID unbinds any shader, switching
 *      back to the fixed function vertex or pixel pipeline.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_SetShader(struct svga_winsys_context *swc,
                 SVGA3dShaderType type,  // IN
                 uint32 shid)            // IN
{
   SVGA3dCmdSetShader *cmd;

   assert(type == SVGA3D_SHADERTYPE_VS || type == SVGA3D_SHADERTYPE_PS);

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SET_SHADER, sizeof *cmd,
                            0);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->type = type;
   cmd->shid = shid;
   swc->commit(swc);

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_BeginClear --
 *
 *      Begin a CLEAR command. This reserves space for it in the FIFO,
 *      and returns a pointer to the command's rectangle array.  This
 *      function must be paired with SVGA_FIFOCommitAll().
 *
 *      Clear is a rendering operation which fills a list of
 *      rectangles with constant values on all render target types
 *      indicated by 'flags'.
 *
 *      Clear is not affected by clipping, depth test, or other
 *      render state which affects the fragment pipeline.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      May write to attached render target surfaces.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_BeginClear(struct svga_winsys_context *swc,
                  SVGA3dClearFlag flags,  // IN
                  uint32 color,           // IN
                  float depth,            // IN
                  uint32 stencil,         // IN
                  SVGA3dRect **rects,     // OUT
                  uint32 numRects)        // IN
{
   SVGA3dCmdClear *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_CLEAR,
                            sizeof *cmd + sizeof **rects * numRects,
                            0);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->clearFlag = flags;
   cmd->color = color;
   cmd->depth = depth;
   cmd->stencil = stencil;
   *rects = (SVGA3dRect*) &cmd[1];

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_ClearRect --
 *
 *      This is a simplified version of SVGA3D_BeginClear().
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_ClearRect(struct svga_winsys_context *swc,
                 SVGA3dClearFlag flags,  // IN
                 uint32 color,           // IN
                 float depth,            // IN
                 uint32 stencil,         // IN
                 uint32 x,               // IN
                 uint32 y,               // IN
                 uint32 w,               // IN
                 uint32 h)               // IN
{
   SVGA3dRect *rect;
   enum pipe_error ret;

   ret = SVGA3D_BeginClear(swc, flags, color, depth, stencil, &rect, 1);
   if (ret != PIPE_OK)
      return PIPE_ERROR_OUT_OF_MEMORY;

   memset(rect, 0, sizeof *rect);
   rect->x = x;
   rect->y = y;
   rect->w = w;
   rect->h = h;
   swc->commit(swc);

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_BeginDrawPrimitives --
 *
 *      Begin a DRAW_PRIMITIVES command. This reserves space for it in
 *      the FIFO, and returns a pointer to the command's arrays.
 *      This function must be paired with SVGA_FIFOCommitAll().
 *
 *      Drawing commands consist of two variable-length arrays:
 *      SVGA3dVertexDecl elements declare a set of vertex buffers to
 *      use while rendering, and SVGA3dPrimitiveRange elements specify
 *      groups of primitives each with an optional index buffer.
 *
 *      The decls and ranges arrays are initialized to zero.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      May write to attached render target surfaces.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_BeginDrawPrimitives(struct svga_winsys_context *swc,
                           SVGA3dVertexDecl **decls,      // OUT
                           uint32 numVertexDecls,         // IN
                           SVGA3dPrimitiveRange **ranges, // OUT
                           uint32 numRanges)              // IN
{
   SVGA3dCmdDrawPrimitives *cmd;
   SVGA3dVertexDecl *declArray;
   SVGA3dPrimitiveRange *rangeArray;
   uint32 declSize = sizeof **decls * numVertexDecls;
   uint32 rangeSize = sizeof **ranges * numRanges;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_DRAW_PRIMITIVES,
                            sizeof *cmd + declSize + rangeSize,
                            numVertexDecls + numRanges);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->numVertexDecls = numVertexDecls;
   cmd->numRanges = numRanges;

   declArray = (SVGA3dVertexDecl*) &cmd[1];
   rangeArray = (SVGA3dPrimitiveRange*) &declArray[numVertexDecls];

   memset(declArray, 0, declSize);
   memset(rangeArray, 0, rangeSize);

   *decls = declArray;
   *ranges = rangeArray;

   swc->hints |= SVGA_HINT_FLAG_CAN_PRE_FLUSH;

   swc->num_draw_commands++;

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_BeginSurfaceCopy --
 *
 *      Begin a SURFACE_COPY command. This reserves space for it in
 *      the FIFO, and returns a pointer to the command's arrays.  This
 *      function must be paired with SVGA_FIFOCommitAll().
 *
 *      The box array is initialized with zeroes.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Asynchronously copies a list of boxes from surface to surface.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_BeginSurfaceCopy(struct svga_winsys_context *swc,
                        struct pipe_surface *src,    // IN
                        struct pipe_surface *dest,   // IN
                        SVGA3dCopyBox **boxes,       // OUT
                        uint32 numBoxes)             // IN
{
   SVGA3dCmdSurfaceCopy *cmd;
   uint32 boxesSize = sizeof **boxes * numBoxes;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SURFACE_COPY, sizeof *cmd + boxesSize,
                            2);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   surface_to_surfaceid(swc, src, &cmd->src, SVGA_RELOC_READ);
   surface_to_surfaceid(swc, dest, &cmd->dest, SVGA_RELOC_WRITE);
   *boxes = (SVGA3dCopyBox*) &cmd[1];

   memset(*boxes, 0, boxesSize);

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_SurfaceStretchBlt --
 *
 *      Issue a SURFACE_STRETCHBLT command: an asynchronous
 *      surface-to-surface blit, with scaling.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Asynchronously copies one box from surface to surface.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_SurfaceStretchBlt(struct svga_winsys_context *swc,
                         struct pipe_surface *src,    // IN
                         struct pipe_surface *dest,   // IN
                         SVGA3dBox *boxSrc,           // IN
                         SVGA3dBox *boxDest,          // IN
                         SVGA3dStretchBltMode mode)   // IN
{
   SVGA3dCmdSurfaceStretchBlt *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SURFACE_STRETCHBLT, sizeof *cmd,
                            2);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   surface_to_surfaceid(swc, src, &cmd->src, SVGA_RELOC_READ);
   surface_to_surfaceid(swc, dest, &cmd->dest, SVGA_RELOC_WRITE);
   cmd->boxSrc = *boxSrc;
   cmd->boxDest = *boxDest;
   cmd->mode = mode;
   swc->commit(swc);

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_SetViewport --
 *
 *      Set the current context's viewport rectangle. The viewport
 *      is clipped to the dimensions of the current render target,
 *      then all rendering is clipped to the viewport.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_SetViewport(struct svga_winsys_context *swc,
                   SVGA3dRect *rect)  // IN
{
   SVGA3dCmdSetViewport *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SETVIEWPORT, sizeof *cmd,
                            0);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->rect = *rect;
   swc->commit(swc);

   return PIPE_OK;
}




/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_SetScissorRect --
 *
 *      Set the current context's scissor rectangle. If scissoring
 *      is enabled then all rendering is clipped to the scissor bounds.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_SetScissorRect(struct svga_winsys_context *swc,
                      SVGA3dRect *rect)  // IN
{
   SVGA3dCmdSetScissorRect *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SETSCISSORRECT, sizeof *cmd,
                            0);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->rect = *rect;
   swc->commit(swc);

   return PIPE_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_SetClipPlane --
 *
 *      Set one of the current context's clip planes. If the clip
 *      plane is enabled then all 3d rendering is clipped against
 *      the plane.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_SetClipPlane(struct svga_winsys_context *swc,
                    uint32 index, const float *plane)
{
   SVGA3dCmdSetClipPlane *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SETCLIPPLANE, sizeof *cmd,
                            0);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->index = index;
   cmd->plane[0] = plane[0];
   cmd->plane[1] = plane[1];
   cmd->plane[2] = plane[2];
   cmd->plane[3] = plane[3];
   swc->commit(swc);

   return PIPE_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_SetZRange --
 *
 *      Set the range of the depth buffer to use. 'min' and 'max'
 *      are values between 0.0 and 1.0.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_SetZRange(struct svga_winsys_context *swc,
                 float zMin,  // IN
                 float zMax)  // IN
{
   SVGA3dCmdSetZRange *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SETZRANGE, sizeof *cmd,
                            0);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->zRange.min = zMin;
   cmd->zRange.max = zMax;
   swc->commit(swc);

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_BeginSetTextureState --
 *
 *      Begin a SETTEXTURESTATE command. This reserves space for it in
 *      the FIFO, and returns a pointer to the command's texture state
 *      array.  This function must be paired with SVGA_FIFOCommitAll().
 *
 *      This command sets rendering state which is per-texture-unit.
 *
 *      XXX: Individual texture states need documentation. However,
 *           they are very similar to the texture states defined by
 *           Direct3D. The D3D documentation is a good starting point
 *           for understanding SVGA3D texture states.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_BeginSetTextureState(struct svga_winsys_context *swc,
                            SVGA3dTextureState **states,  // OUT
                            uint32 numStates)             // IN
{
   SVGA3dCmdSetTextureState *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SETTEXTURESTATE,
                            sizeof *cmd + sizeof **states * numStates,
                            numStates);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   *states = (SVGA3dTextureState*) &cmd[1];

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_BeginSetRenderState --
 *
 *      Begin a SETRENDERSTATE command. This reserves space for it in
 *      the FIFO, and returns a pointer to the command's texture state
 *      array.  This function must be paired with SVGA_FIFOCommitAll().
 *
 *      This command sets rendering state which is global to the context.
 *
 *      XXX: Individual render states need documentation. However,
 *           they are very similar to the render states defined by
 *           Direct3D. The D3D documentation is a good starting point
 *           for understanding SVGA3D render states.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_BeginSetRenderState(struct svga_winsys_context *swc,
                           SVGA3dRenderState **states,  // OUT
                           uint32 numStates)            // IN
{
   SVGA3dCmdSetRenderState *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SETRENDERSTATE,
                            sizeof *cmd + sizeof **states * numStates,
                            0);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   *states = (SVGA3dRenderState*) &cmd[1];

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_BeginGBQuery--
 *
 *      GB resource version of SVGA3D_BeginQuery.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Commits space in the FIFO memory.
 *
 *----------------------------------------------------------------------
 */

static enum pipe_error
SVGA3D_BeginGBQuery(struct svga_winsys_context *swc,
		    SVGA3dQueryType type) // IN
{
   SVGA3dCmdBeginGBQuery *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_BEGIN_GB_QUERY,
                            sizeof *cmd,
                            1);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->type = type;

   swc->commit(swc);

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_BeginQuery--
 *
 *      Issues a SVGA_3D_CMD_BEGIN_QUERY command.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Commits space in the FIFO memory.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_BeginQuery(struct svga_winsys_context *swc,
                  SVGA3dQueryType type) // IN
{
   SVGA3dCmdBeginQuery *cmd;

   if (swc->have_gb_objects)
      return SVGA3D_BeginGBQuery(swc, type);

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_BEGIN_QUERY,
                            sizeof *cmd,
                            0);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->type = type;

   swc->commit(swc);

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_EndGBQuery--
 *
 *      GB resource version of SVGA3D_EndQuery.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Commits space in the FIFO memory.
 *
 *----------------------------------------------------------------------
 */

static enum pipe_error
SVGA3D_EndGBQuery(struct svga_winsys_context *swc,
		  SVGA3dQueryType type,              // IN
		  struct svga_winsys_buffer *buffer) // IN/OUT
{
   SVGA3dCmdEndGBQuery *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_END_GB_QUERY,
                            sizeof *cmd,
                            2);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->type = type;

   swc->mob_relocation(swc, &cmd->mobid, &cmd->offset, buffer,
		       0, SVGA_RELOC_READ | SVGA_RELOC_WRITE);

   swc->commit(swc);
   
   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_EndQuery--
 *
 *      Issues a SVGA_3D_CMD_END_QUERY command.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Commits space in the FIFO memory.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_EndQuery(struct svga_winsys_context *swc,
                SVGA3dQueryType type,              // IN
                struct svga_winsys_buffer *buffer) // IN/OUT
{
   SVGA3dCmdEndQuery *cmd;

   if (swc->have_gb_objects)
      return SVGA3D_EndGBQuery(swc, type, buffer);

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_END_QUERY,
                            sizeof *cmd,
                            1);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->type = type;

   swc->region_relocation(swc, &cmd->guestResult, buffer, 0,
                          SVGA_RELOC_READ | SVGA_RELOC_WRITE);

   swc->commit(swc);

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_WaitForGBQuery--
 *
 *      GB resource version of SVGA3D_WaitForQuery.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Commits space in the FIFO memory.
 *
 *----------------------------------------------------------------------
 */

static enum pipe_error
SVGA3D_WaitForGBQuery(struct svga_winsys_context *swc,
		      SVGA3dQueryType type,              // IN
		      struct svga_winsys_buffer *buffer) // IN/OUT
{
   SVGA3dCmdWaitForGBQuery *cmd;

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_WAIT_FOR_GB_QUERY,
                            sizeof *cmd,
                            2);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->type = type;

   swc->mob_relocation(swc, &cmd->mobid, &cmd->offset, buffer,
		       0, SVGA_RELOC_READ | SVGA_RELOC_WRITE);

   swc->commit(swc);

   return PIPE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * SVGA3D_WaitForQuery--
 *
 *      Issues a SVGA_3D_CMD_WAIT_FOR_QUERY command.  This reserves space
 *      for it in the FIFO.  This doesn't actually wait for the query to
 *      finish but instead tells the host to start a wait at the driver
 *      level.  The caller can wait on the status variable in the
 *      guestPtr memory or send an insert fence instruction after this
 *      command and wait on the fence.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Commits space in the FIFO memory.
 *
 *----------------------------------------------------------------------
 */

enum pipe_error
SVGA3D_WaitForQuery(struct svga_winsys_context *swc,
                    SVGA3dQueryType type,              // IN
                    struct svga_winsys_buffer *buffer) // IN/OUT
{
   SVGA3dCmdWaitForQuery *cmd;

   if (swc->have_gb_objects)
      return SVGA3D_WaitForGBQuery(swc, type, buffer);

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_WAIT_FOR_QUERY,
                            sizeof *cmd,
                            1);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->type = type;

   swc->region_relocation(swc, &cmd->guestResult, buffer, 0,
                          SVGA_RELOC_READ | SVGA_RELOC_WRITE);

   swc->commit(swc);

   return PIPE_OK;
}


enum pipe_error
SVGA3D_BindGBShader(struct svga_winsys_context *swc,
                    struct svga_winsys_gb_shader *gbshader)
{
   SVGA3dCmdBindGBShader *cmd = 
      SVGA3D_FIFOReserve(swc,
                         SVGA_3D_CMD_BIND_GB_SHADER,
                         sizeof *cmd,
                         2);  /* two relocations */

   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   swc->shader_relocation(swc, &cmd->shid, &cmd->mobid,
			  &cmd->offsetInBytes, gbshader, 0);

   swc->commit(swc);

   return PIPE_OK;
}


enum pipe_error
SVGA3D_SetGBShader(struct svga_winsys_context *swc,
                   SVGA3dShaderType type,  // IN
                   struct svga_winsys_gb_shader *gbshader)
{
   SVGA3dCmdSetShader *cmd;

   assert(type == SVGA3D_SHADERTYPE_VS || type == SVGA3D_SHADERTYPE_PS);
   
   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SET_SHADER,
                            sizeof *cmd,
                            2);  /* two relocations */
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;
   
   cmd->cid = swc->cid;
   cmd->type = type;
   if (gbshader)
      swc->shader_relocation(swc, &cmd->shid, NULL, NULL, gbshader, 0);
   else
      cmd->shid = SVGA_ID_INVALID;
   swc->commit(swc);

   return PIPE_OK;
}


/**
 * \param flags  mask of SVGA_RELOC_READ / _WRITE
 */
enum pipe_error
SVGA3D_BindGBSurface(struct svga_winsys_context *swc,
                     struct svga_winsys_surface *surface)
{
   SVGA3dCmdBindGBSurface *cmd = 
      SVGA3D_FIFOReserve(swc,
                         SVGA_3D_CMD_BIND_GB_SURFACE,
                         sizeof *cmd,
                         2);  /* two relocations */

   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   swc->surface_relocation(swc, &cmd->sid, &cmd->mobid, surface,
                           SVGA_RELOC_READ);

   swc->commit(swc);

   return PIPE_OK;
}


/**
 * Update an image in a guest-backed surface.
 * (Inform the device that the guest-contents have been updated.)
 */
enum pipe_error
SVGA3D_UpdateGBImage(struct svga_winsys_context *swc,
                     struct svga_winsys_surface *surface,
                     const SVGA3dBox *box,
                     unsigned face, unsigned mipLevel)

{
   SVGA3dCmdUpdateGBImage *cmd =
      SVGA3D_FIFOReserve(swc,
                         SVGA_3D_CMD_UPDATE_GB_IMAGE,
                         sizeof *cmd,
                         1);  /* one relocation */

   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   swc->surface_relocation(swc, &cmd->image.sid, NULL, surface,
                           SVGA_RELOC_WRITE | SVGA_RELOC_INTERNAL);
   cmd->image.face = face;
   cmd->image.mipmap = mipLevel;
   cmd->box = *box;

   swc->commit(swc);
   swc->hints |= SVGA_HINT_FLAG_CAN_PRE_FLUSH;

   return PIPE_OK;
}


/**
 * Update an entire guest-backed surface.
 * (Inform the device that the guest-contents have been updated.)
 */
enum pipe_error
SVGA3D_UpdateGBSurface(struct svga_winsys_context *swc,
                       struct svga_winsys_surface *surface)
{
   SVGA3dCmdUpdateGBSurface *cmd =
      SVGA3D_FIFOReserve(swc,
                         SVGA_3D_CMD_UPDATE_GB_SURFACE,
                         sizeof *cmd,
                         1);  /* one relocation */

   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   swc->surface_relocation(swc, &cmd->sid, NULL, surface,
                           SVGA_RELOC_WRITE | SVGA_RELOC_INTERNAL);

   swc->commit(swc);
   swc->hints |= SVGA_HINT_FLAG_CAN_PRE_FLUSH;

   return PIPE_OK;
}


/**
 * Readback an image in a guest-backed surface.
 * (Request the device to flush the dirty contents into the guest.)
 */
enum pipe_error
SVGA3D_ReadbackGBImage(struct svga_winsys_context *swc,
                       struct svga_winsys_surface *surface,
                       unsigned face, unsigned mipLevel)
{
   SVGA3dCmdReadbackGBImage *cmd =
      SVGA3D_FIFOReserve(swc,
                         SVGA_3D_CMD_READBACK_GB_IMAGE,
                         sizeof *cmd,
                         1);  /* one relocation */

   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   swc->surface_relocation(swc, &cmd->image.sid, NULL, surface,
                           SVGA_RELOC_READ | SVGA_RELOC_INTERNAL);
   cmd->image.face = face;
   cmd->image.mipmap = mipLevel;

   swc->commit(swc);
   swc->hints |= SVGA_HINT_FLAG_CAN_PRE_FLUSH;

   return PIPE_OK;
}


/**
 * Readback an entire guest-backed surface.
 * (Request the device to flush the dirty contents into the guest.)
 */
enum pipe_error
SVGA3D_ReadbackGBSurface(struct svga_winsys_context *swc,
                         struct svga_winsys_surface *surface)
{
   SVGA3dCmdReadbackGBSurface *cmd =
      SVGA3D_FIFOReserve(swc,
                         SVGA_3D_CMD_READBACK_GB_SURFACE,
                         sizeof *cmd,
                         1);  /* one relocation */

   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   swc->surface_relocation(swc, &cmd->sid, NULL, surface,
                           SVGA_RELOC_READ | SVGA_RELOC_INTERNAL);

   swc->commit(swc);
   swc->hints |= SVGA_HINT_FLAG_CAN_PRE_FLUSH;

   return PIPE_OK;
}


enum pipe_error
SVGA3D_ReadbackGBImagePartial(struct svga_winsys_context *swc,
                              struct svga_winsys_surface *surface,
                              unsigned face, unsigned mipLevel,
                              const SVGA3dBox *box,
                              bool invertBox)
{
   SVGA3dCmdReadbackGBImagePartial *cmd =
      SVGA3D_FIFOReserve(swc,
                         SVGA_3D_CMD_READBACK_GB_IMAGE_PARTIAL,
                         sizeof *cmd,
                         1);  /* one relocation */
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   swc->surface_relocation(swc, &cmd->image.sid, NULL, surface,
                           SVGA_RELOC_READ | SVGA_RELOC_INTERNAL);
   cmd->image.face = face;
   cmd->image.mipmap = mipLevel;
   cmd->box = *box;
   cmd->invertBox = invertBox;

   swc->commit(swc);
   swc->hints |= SVGA_HINT_FLAG_CAN_PRE_FLUSH;

   return PIPE_OK;
}


enum pipe_error
SVGA3D_InvalidateGBImagePartial(struct svga_winsys_context *swc,
                                struct svga_winsys_surface *surface,
                                unsigned face, unsigned mipLevel,
                                const SVGA3dBox *box,
                                bool invertBox)
{
   SVGA3dCmdInvalidateGBImagePartial *cmd =
      SVGA3D_FIFOReserve(swc,
                         SVGA_3D_CMD_INVALIDATE_GB_IMAGE_PARTIAL,
                         sizeof *cmd,
                         1);  /* one relocation */
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   swc->surface_relocation(swc, &cmd->image.sid, NULL, surface,
                           SVGA_RELOC_READ | SVGA_RELOC_INTERNAL);
   cmd->image.face = face;
   cmd->image.mipmap = mipLevel;
   cmd->box = *box;
   cmd->invertBox = invertBox;

   swc->commit(swc);

   return PIPE_OK;
}

enum pipe_error
SVGA3D_InvalidateGBSurface(struct svga_winsys_context *swc,
                           struct svga_winsys_surface *surface)
{
   SVGA3dCmdInvalidateGBSurface *cmd =
      SVGA3D_FIFOReserve(swc,
                         SVGA_3D_CMD_INVALIDATE_GB_SURFACE,
                         sizeof *cmd,
                         1);  /* one relocation */
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   swc->surface_relocation(swc, &cmd->sid, NULL, surface,
                           SVGA_RELOC_READ | SVGA_RELOC_INTERNAL);
   swc->commit(swc);

   return PIPE_OK;
}

enum pipe_error
SVGA3D_SetGBShaderConstsInline(struct svga_winsys_context *swc,
                              unsigned regStart,
                              unsigned numRegs,
                              SVGA3dShaderType shaderType,
                              SVGA3dShaderConstType constType,
                              const void *values)
{
   SVGA3dCmdSetGBShaderConstInline *cmd;

   assert(numRegs > 0);

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SET_GB_SHADERCONSTS_INLINE,
                            sizeof *cmd + numRegs * sizeof(float[4]),
                            0); /* no relocations */
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   cmd->cid = swc->cid;
   cmd->regStart = regStart;
   cmd->shaderType = shaderType;
   cmd->constType = constType;

   memcpy(&cmd[1], values, numRegs * sizeof(float[4]));

   swc->commit(swc);

   return PIPE_OK;
}
