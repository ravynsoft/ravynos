/**************************************************************************
 * 
 * Copyright 2008 VMware, Inc.
 * Copyright 2010 VMware, Inc.
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
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

/**
 * Polygon stipple helper module.  Drivers/GPUs which don't support polygon
 * stipple natively can use this module to simulate it.
 *
 * Basically, modify fragment shader to sample the 32x32 stipple pattern
 * texture and do a fragment kill for the 'off' bits.
 *
 * This was originally a 'draw' module stage, but since we don't need
 * vertex window coords or anything, it can be a stand-alone utility module.
 *
 * Authors:  Brian Paul
 */


#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_shader_tokens.h"
#include "util/u_inlines.h"

#include "util/format/u_format.h"
#include "util/u_memory.h"
#include "util/u_pstipple.h"
#include "util/u_sampler.h"

#include "tgsi/tgsi_transform.h"
#include "tgsi/tgsi_dump.h"
#include "tgsi/tgsi_scan.h"

/** Approx number of new tokens for instructions in pstip_transform_inst() */
#define NUM_NEW_TOKENS 53


void
util_pstipple_update_stipple_texture(struct pipe_context *pipe,
                                     struct pipe_resource *tex,
                                     const uint32_t pattern[32])
{
   static const unsigned bit31 = 1u << 31;
   struct pipe_transfer *transfer;
   uint8_t *data;
   int i, j;

   /* map texture memory */
   data = pipe_texture_map(pipe, tex, 0, 0,
                            PIPE_MAP_WRITE, 0, 0, 32, 32, &transfer);

   /*
    * Load alpha texture.
    * Note: 0 means keep the fragment, 255 means kill it.
    * We'll negate the texel value and use KILL_IF which kills if value
    * is negative.
    */
   for (i = 0; i < 32; i++) {
      for (j = 0; j < 32; j++) {
         if (pattern[i] & (bit31 >> j)) {
            /* fragment "on" */
            data[i * transfer->stride + j] = 0;
         }
         else {
            /* fragment "off" */
            data[i * transfer->stride + j] = 255;
         }
      }
   }

   /* unmap */
   pipe->texture_unmap(pipe, transfer);
}


/**
 * Create a 32x32 alpha8 texture that encodes the given stipple pattern.
 */
struct pipe_resource *
util_pstipple_create_stipple_texture(struct pipe_context *pipe,
                                     const uint32_t pattern[32])
{
   struct pipe_screen *screen = pipe->screen;
   struct pipe_resource templat, *tex;

   memset(&templat, 0, sizeof(templat));
   templat.target = PIPE_TEXTURE_2D;
   templat.format = PIPE_FORMAT_A8_UNORM;
   templat.last_level = 0;
   templat.width0 = 32;
   templat.height0 = 32;
   templat.depth0 = 1;
   templat.array_size = 1;
   templat.bind = PIPE_BIND_SAMPLER_VIEW;

   tex = screen->resource_create(screen, &templat);

   if (tex && pattern)
      util_pstipple_update_stipple_texture(pipe, tex, pattern);

   return tex;
}


/**
 * Create sampler view to sample the stipple texture.
 */
struct pipe_sampler_view *
util_pstipple_create_sampler_view(struct pipe_context *pipe,
                                  struct pipe_resource *tex)
{
   struct pipe_sampler_view templat, *sv;

   u_sampler_view_default_template(&templat, tex, tex->format);
   sv = pipe->create_sampler_view(pipe, tex, &templat);

   return sv;
}


/**
 * Create the sampler CSO that'll be used for stippling.
 */
void *
util_pstipple_create_sampler(struct pipe_context *pipe)
{
   struct pipe_sampler_state templat;
   void *s;

   memset(&templat, 0, sizeof(templat));
   templat.wrap_s = PIPE_TEX_WRAP_REPEAT;
   templat.wrap_t = PIPE_TEX_WRAP_REPEAT;
   templat.wrap_r = PIPE_TEX_WRAP_REPEAT;
   templat.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
   templat.min_img_filter = PIPE_TEX_FILTER_NEAREST;
   templat.mag_img_filter = PIPE_TEX_FILTER_NEAREST;
   templat.min_lod = 0.0f;
   templat.max_lod = 0.0f;

   s = pipe->create_sampler_state(pipe, &templat);
   return s;
}



/**
 * Subclass of tgsi_transform_context, used for transforming the
 * user's fragment shader to add the extra texture sample and fragment kill
 * instructions.
 */
struct pstip_transform_context {
   struct tgsi_transform_context base;
   struct tgsi_shader_info info;
   unsigned tempsUsed;  /**< bitmask */
   int wincoordInput;
   unsigned wincoordFile;
   int maxInput;
   unsigned samplersUsed;  /**< bitfield of samplers used */
   int freeSampler;  /** an available sampler for the pstipple */
   int numImmed;
   unsigned coordOrigin;
   unsigned fixedUnit;
   bool hasFixedUnit;
};


/**
 * TGSI declaration transform callback.
 * Track samplers used, temps used, inputs used.
 */
static void
pstip_transform_decl(struct tgsi_transform_context *ctx,
                     struct tgsi_full_declaration *decl)
{
   struct pstip_transform_context *pctx =
      (struct pstip_transform_context *) ctx;

   /* XXX we can use tgsi_shader_info instead of some of this */

   if (decl->Declaration.File == TGSI_FILE_SAMPLER) {
      unsigned i;
      for (i = decl->Range.First; i <= decl->Range.Last; i++) {
         pctx->samplersUsed |= 1u << i;
      }
   }
   else if (decl->Declaration.File == pctx->wincoordFile) {
      pctx->maxInput = MAX2(pctx->maxInput, (int) decl->Range.Last);
      if (decl->Semantic.Name == TGSI_SEMANTIC_POSITION)
         pctx->wincoordInput = (int) decl->Range.First;
   }
   else if (decl->Declaration.File == TGSI_FILE_TEMPORARY) {
      unsigned i;
      for (i = decl->Range.First; i <= decl->Range.Last; i++) {
         pctx->tempsUsed |= (1 << i);
      }
   }

   ctx->emit_declaration(ctx, decl);
}


static void
pstip_transform_immed(struct tgsi_transform_context *ctx,
                      struct tgsi_full_immediate *immed)
{
   struct pstip_transform_context *pctx =
      (struct pstip_transform_context *) ctx;
   pctx->numImmed++;
   ctx->emit_immediate(ctx, immed);
}


/**
 * Find the lowest zero bit in the given word, or -1 if bitfield is all ones.
 */
static int
free_bit(unsigned bitfield)
{
   return ffs(~bitfield) - 1;
}


/**
 * TGSI transform prolog
 * Before the first instruction, insert our new code to sample the
 * stipple texture (using the fragment coord register) then kill the
 * fragment if the stipple texture bit is off.
 *
 * Insert:
 *   declare new registers
 *   MUL texTemp, INPUT[wincoord], 1/32;
 *   TEX texTemp, texTemp, sampler;
 *   KILL_IF -texTemp;   # if -texTemp < 0, kill fragment
 *   [...original code...]
 */
static void
pstip_transform_prolog(struct tgsi_transform_context *ctx)
{
   struct pstip_transform_context *pctx =
      (struct pstip_transform_context *) ctx;
   int wincoordInput;
   int texTemp;
   int sampIdx;

   STATIC_ASSERT(sizeof(pctx->samplersUsed) * 8 >= PIPE_MAX_SAMPLERS);

   /* find free texture sampler */
   pctx->freeSampler = free_bit(pctx->samplersUsed);
   if (pctx->freeSampler < 0 || pctx->freeSampler >= PIPE_MAX_SAMPLERS)
      pctx->freeSampler = PIPE_MAX_SAMPLERS - 1;

   if (pctx->wincoordInput < 0)
      wincoordInput = pctx->maxInput + 1;
   else
      wincoordInput = pctx->wincoordInput;

   if (pctx->wincoordInput < 0) {
      struct tgsi_full_declaration decl;

      decl = tgsi_default_full_declaration();
      /* declare new position input reg */
      decl.Declaration.File = pctx->wincoordFile;
      decl.Declaration.Semantic = 1;
      decl.Semantic.Name = TGSI_SEMANTIC_POSITION;
      decl.Range.First =
      decl.Range.Last = wincoordInput;

      if (pctx->wincoordFile == TGSI_FILE_INPUT) {
         decl.Declaration.Interpolate = 1;
         decl.Interp.Interpolate = TGSI_INTERPOLATE_LINEAR;
      }

      ctx->emit_declaration(ctx, &decl);
   }

   sampIdx = pctx->hasFixedUnit ? (int)pctx->fixedUnit : pctx->freeSampler;

   /* declare new sampler */
   tgsi_transform_sampler_decl(ctx, sampIdx);

   /* if the src shader has SVIEW decl's for each SAMP decl, we
    * need to continue the trend and ensure there is a matching
    * SVIEW for the new SAMP we just created
    */
   if (pctx->info.file_max[TGSI_FILE_SAMPLER_VIEW] != -1) {
      tgsi_transform_sampler_view_decl(ctx,
                                       sampIdx,
                                       TGSI_TEXTURE_2D,
                                       TGSI_RETURN_TYPE_FLOAT);
   }

   /* Declare temp[0] reg if not already declared.
    * We can always use temp[0] since this code is before
    * the rest of the shader.
    */
   texTemp = 0;
   if ((pctx->tempsUsed & (1 << texTemp)) == 0) {
      tgsi_transform_temp_decl(ctx, texTemp);
   }

   /* emit immediate = {1/32, 1/32, 1, 1}
    * The index/position of this immediate will be pctx->numImmed
    */
   tgsi_transform_immediate_decl(ctx, 1.0/32.0, 1.0/32.0, 1.0, 1.0);

   /* 
    * Insert new MUL/TEX/KILL_IF instructions at start of program
    * Take gl_FragCoord, divide by 32 (stipple size), sample the
    * texture and kill fragment if needed.
    *
    * We'd like to use non-normalized texcoords to index into a RECT
    * texture, but we can only use REPEAT wrap mode with normalized
    * texcoords.  Darn.
    */

   /* XXX invert wincoord if origin isn't lower-left... */

   /* MUL texTemp, INPUT[wincoord], 1/32; */
   tgsi_transform_op2_inst(ctx, TGSI_OPCODE_MUL,
                           TGSI_FILE_TEMPORARY, texTemp,
                           TGSI_WRITEMASK_XYZW,
                           pctx->wincoordFile, wincoordInput,
                           TGSI_FILE_IMMEDIATE, pctx->numImmed, false);

   /* TEX texTemp, texTemp, sampler, 2D; */
   tgsi_transform_tex_inst(ctx,
                           TGSI_FILE_TEMPORARY, texTemp,
                           TGSI_FILE_TEMPORARY, texTemp,
                           TGSI_TEXTURE_2D, sampIdx);

   /* KILL_IF -texTemp;   # if -texTemp < 0, kill fragment */
   tgsi_transform_kill_inst(ctx,
                            TGSI_FILE_TEMPORARY, texTemp,
                            TGSI_SWIZZLE_W, true);
}


/**
 * Given a fragment shader, return a new fragment shader which
 * samples a stipple texture and executes KILL.
 *
 * \param samplerUnitOut  returns the index of the sampler unit which
 *                        will be used to sample the stipple texture;
 *                        if NULL, the fixed unit is used
 * \param fixedUnit       fixed texture unit used for the stipple texture
 * \param wincoordFile    TGSI_FILE_INPUT or TGSI_FILE_SYSTEM_VALUE,
 *                        depending on which one is supported by the driver
 *                        for TGSI_SEMANTIC_POSITION in the fragment shader
 */
struct tgsi_token *
util_pstipple_create_fragment_shader(const struct tgsi_token *tokens,
                                     unsigned *samplerUnitOut,
                                     unsigned fixedUnit,
                                     unsigned wincoordFile)
{
   struct pstip_transform_context transform;
   const unsigned newLen = tgsi_num_tokens(tokens) + NUM_NEW_TOKENS;
   struct tgsi_token *new_tokens;

   /* Setup shader transformation info/context.
    */
   memset(&transform, 0, sizeof(transform));
   transform.wincoordInput = -1;
   transform.wincoordFile = wincoordFile;
   transform.maxInput = -1;
   transform.coordOrigin = TGSI_FS_COORD_ORIGIN_UPPER_LEFT;
   transform.hasFixedUnit = !samplerUnitOut;
   transform.fixedUnit = fixedUnit;
   transform.base.prolog = pstip_transform_prolog;
   transform.base.transform_declaration = pstip_transform_decl;
   transform.base.transform_immediate = pstip_transform_immed;

   tgsi_scan_shader(tokens, &transform.info);

   transform.coordOrigin =
      transform.info.properties[TGSI_PROPERTY_FS_COORD_ORIGIN];

   new_tokens = tgsi_transform_shader(tokens, newLen, &transform.base);
   if (!new_tokens)
      return NULL;

#if 0 /* DEBUG */
   tgsi_dump(fs->tokens, 0);
   tgsi_dump(new_fs->tokens, 0);
#endif

   if (samplerUnitOut) {
      assert(transform.freeSampler < PIPE_MAX_SAMPLERS);
      *samplerUnitOut = transform.freeSampler;
   }

   return new_tokens;
}
