/*
 * Copyright 2010 Christoph Bumiller
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "pipe/p_defines.h"

#include "compiler/nir/nir.h"

#include "nv50/nv50_context.h"
#include "nv50/nv50_program.h"

#include "nv50_ir_driver.h"

static inline unsigned
bitcount4(const uint32_t val)
{
   static const uint8_t cnt[16]
   = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
   return cnt[val & 0xf];
}

static int
nv50_vertprog_assign_slots(struct nv50_ir_prog_info_out *info)
{
   struct nv50_program *prog = (struct nv50_program *)info->driverPriv;
   unsigned i, n, c;

   n = 0;
   for (i = 0; i < info->numInputs; ++i) {
      prog->in[i].id = i;
      prog->in[i].sn = info->in[i].sn;
      prog->in[i].si = info->in[i].si;
      prog->in[i].hw = n;
      prog->in[i].mask = info->in[i].mask;

      prog->vp.attrs[(4 * i) / 32] |= info->in[i].mask << ((4 * i) % 32);

      for (c = 0; c < 4; ++c)
         if (info->in[i].mask & (1 << c))
            info->in[i].slot[c] = n++;

      if (info->in[i].sn == TGSI_SEMANTIC_PRIMID)
         prog->vp.attrs[2] |= NV50_3D_VP_GP_BUILTIN_ATTR_EN_PRIMITIVE_ID;
   }
   prog->in_nr = info->numInputs;

   for (i = 0; i < info->numSysVals; ++i) {
      switch (info->sv[i].sn) {
      case SYSTEM_VALUE_INSTANCE_ID:
         prog->vp.attrs[2] |= NV50_3D_VP_GP_BUILTIN_ATTR_EN_INSTANCE_ID;
         continue;
      case SYSTEM_VALUE_VERTEX_ID:
         prog->vp.attrs[2] |= NV50_3D_VP_GP_BUILTIN_ATTR_EN_VERTEX_ID;
         prog->vp.attrs[2] |= NV50_3D_VP_GP_BUILTIN_ATTR_EN_VERTEX_ID_DRAW_ARRAYS_ADD_START;
         continue;
      case SYSTEM_VALUE_PRIMITIVE_ID:
         prog->vp.attrs[2] |= NV50_3D_VP_GP_BUILTIN_ATTR_EN_PRIMITIVE_ID;
         break;
      default:
         break;
      }
   }

   /*
    * Corner case: VP has no inputs, but we will still need to submit data to
    * draw it. HW will shout at us and won't draw anything if we don't enable
    * any input, so let's just pretend it's the first one.
    */
   if (prog->vp.attrs[0] == 0 &&
       prog->vp.attrs[1] == 0 &&
       prog->vp.attrs[2] == 0)
      prog->vp.attrs[0] |= 0xf;

   /* VertexID before InstanceID */
   if (info->io.vertexId < info->numSysVals)
      info->sv[info->io.vertexId].slot[0] = n++;
   if (info->io.instanceId < info->numSysVals)
      info->sv[info->io.instanceId].slot[0] = n++;

   n = 0;
   for (i = 0; i < info->numOutputs; ++i) {
      switch (info->out[i].sn) {
      case TGSI_SEMANTIC_PSIZE:
         prog->vp.psiz = i;
         break;
      case TGSI_SEMANTIC_CLIPDIST:
         prog->vp.clpd[info->out[i].si] = n;
         break;
      case TGSI_SEMANTIC_EDGEFLAG:
         prog->vp.edgeflag = i;
         break;
      case TGSI_SEMANTIC_BCOLOR:
         prog->vp.bfc[info->out[i].si] = i;
         break;
      case TGSI_SEMANTIC_LAYER:
         prog->gp.has_layer = true;
         prog->gp.layerid = n;
         break;
      case TGSI_SEMANTIC_VIEWPORT_INDEX:
         prog->gp.has_viewport = true;
         prog->gp.viewportid = n;
         break;
      default:
         break;
      }
      prog->out[i].id = i;
      prog->out[i].sn = info->out[i].sn;
      prog->out[i].si = info->out[i].si;
      prog->out[i].hw = n;
      prog->out[i].mask = info->out[i].mask;

      for (c = 0; c < 4; ++c)
         if (info->out[i].mask & (1 << c))
            info->out[i].slot[c] = n++;
   }
   prog->out_nr = info->numOutputs;
   prog->max_out = n;
   if (!prog->max_out)
      prog->max_out = 1;

   if (prog->vp.psiz < info->numOutputs)
      prog->vp.psiz = prog->out[prog->vp.psiz].hw;

   return 0;
}

static int
nv50_fragprog_assign_slots(struct nv50_ir_prog_info_out *info)
{
   struct nv50_program *prog = (struct nv50_program *)info->driverPriv;
   unsigned i, n, m, c;
   unsigned nvary;
   unsigned nflat;
   unsigned nintp = 0;

   /* count recorded non-flat inputs */
   for (m = 0, i = 0; i < info->numInputs; ++i) {
      switch (info->in[i].sn) {
      case TGSI_SEMANTIC_POSITION:
         continue;
      default:
         m += info->in[i].flat ? 0 : 1;
         break;
      }
   }
   /* careful: id may be != i in info->in[prog->in[i].id] */

   /* Fill prog->in[] so that non-flat inputs are first and
    * kick out special inputs that don't use the RESULT_MAP.
    */
   for (n = 0, i = 0; i < info->numInputs; ++i) {
      if (info->in[i].sn == TGSI_SEMANTIC_POSITION) {
         prog->fp.interp |= info->in[i].mask << 24;
         for (c = 0; c < 4; ++c)
            if (info->in[i].mask & (1 << c))
               info->in[i].slot[c] = nintp++;
      } else {
         unsigned j = info->in[i].flat ? m++ : n++;

         if (info->in[i].sn == TGSI_SEMANTIC_COLOR)
            prog->vp.bfc[info->in[i].si] = j;
         else if (info->in[i].sn == TGSI_SEMANTIC_PRIMID)
            prog->vp.attrs[2] |= NV50_3D_VP_GP_BUILTIN_ATTR_EN_PRIMITIVE_ID;

         prog->in[j].id = i;
         prog->in[j].mask = info->in[i].mask;
         prog->in[j].sn = info->in[i].sn;
         prog->in[j].si = info->in[i].si;
         prog->in[j].linear = info->in[i].linear;

         prog->in_nr++;
      }
   }
   if (!(prog->fp.interp & (8 << 24))) {
      ++nintp;
      prog->fp.interp |= 8 << 24;
   }

   for (i = 0; i < prog->in_nr; ++i) {
      int j = prog->in[i].id;

      prog->in[i].hw = nintp;
      for (c = 0; c < 4; ++c)
         if (prog->in[i].mask & (1 << c))
            info->in[j].slot[c] = nintp++;
   }
   /* (n == m) if m never increased, i.e. no flat inputs */
   nflat = (n < m) ? (nintp - prog->in[n].hw) : 0;
   nintp -= bitcount4(prog->fp.interp >> 24); /* subtract position inputs */
   nvary = nintp - nflat;

   prog->fp.interp |= nvary << NV50_3D_FP_INTERPOLANT_CTRL_COUNT_NONFLAT__SHIFT;
   prog->fp.interp |= nintp << NV50_3D_FP_INTERPOLANT_CTRL_COUNT__SHIFT;

   /* put front/back colors right after HPOS */
   prog->fp.colors = 4 << NV50_3D_SEMANTIC_COLOR_FFC0_ID__SHIFT;
   for (i = 0; i < 2; ++i)
      if (prog->vp.bfc[i] < 0xff)
         prog->fp.colors += bitcount4(prog->in[prog->vp.bfc[i]].mask) << 16;

   /* FP outputs */

   if (info->prop.fp.numColourResults > 1)
      prog->fp.flags[0] |= NV50_3D_FP_CONTROL_MULTIPLE_RESULTS;

   for (i = 0; i < info->numOutputs; ++i) {
      prog->out[i].id = i;
      prog->out[i].sn = info->out[i].sn;
      prog->out[i].si = info->out[i].si;
      prog->out[i].mask = info->out[i].mask;

      if (i == info->io.fragDepth || i == info->io.sampleMask)
         continue;
      prog->out[i].hw = info->out[i].si * 4;

      for (c = 0; c < 4; ++c)
         info->out[i].slot[c] = prog->out[i].hw + c;

      prog->max_out = MAX2(prog->max_out, prog->out[i].hw + 4);
   }

   if (info->io.sampleMask < PIPE_MAX_SHADER_OUTPUTS) {
      info->out[info->io.sampleMask].slot[0] = prog->max_out++;
      prog->fp.has_samplemask = 1;
   }

   if (info->io.fragDepth < PIPE_MAX_SHADER_OUTPUTS)
      info->out[info->io.fragDepth].slot[2] = prog->max_out++;

   if (!prog->max_out)
      prog->max_out = 4;

   return 0;
}

static int
nv50_program_assign_varying_slots(struct nv50_ir_prog_info_out *info)
{
   switch (info->type) {
   case PIPE_SHADER_VERTEX:
      return nv50_vertprog_assign_slots(info);
   case PIPE_SHADER_GEOMETRY:
      return nv50_vertprog_assign_slots(info);
   case PIPE_SHADER_FRAGMENT:
      return nv50_fragprog_assign_slots(info);
   case PIPE_SHADER_COMPUTE:
      return 0;
   default:
      return -1;
   }
}

static struct nv50_stream_output_state *
nv50_program_create_strmout_state(const struct nv50_ir_prog_info_out *info,
                                  const struct pipe_stream_output_info *pso)
{
   struct nv50_stream_output_state *so;
   unsigned b, i, c;
   unsigned base[4];

   so = MALLOC_STRUCT(nv50_stream_output_state);
   if (!so)
      return NULL;
   memset(so->map, 0xff, sizeof(so->map));

   for (b = 0; b < 4; ++b)
      so->num_attribs[b] = 0;
   for (i = 0; i < pso->num_outputs; ++i) {
      unsigned end =  pso->output[i].dst_offset + pso->output[i].num_components;
      b = pso->output[i].output_buffer;
      assert(b < 4);
      so->num_attribs[b] = MAX2(so->num_attribs[b], end);
   }

   so->ctrl = NV50_3D_STRMOUT_BUFFERS_CTRL_INTERLEAVED;

   so->stride[0] = pso->stride[0] * 4;
   base[0] = 0;
   for (b = 1; b < 4; ++b) {
      assert(!so->num_attribs[b] || so->num_attribs[b] == pso->stride[b]);
      so->stride[b] = so->num_attribs[b] * 4;
      if (so->num_attribs[b])
         so->ctrl = (b + 1) << NV50_3D_STRMOUT_BUFFERS_CTRL_SEPARATE__SHIFT;
      base[b] = align(base[b - 1] + so->num_attribs[b - 1], 4);
   }
   if (so->ctrl & NV50_3D_STRMOUT_BUFFERS_CTRL_INTERLEAVED) {
      assert(so->stride[0] < NV50_3D_STRMOUT_BUFFERS_CTRL_STRIDE__MAX);
      so->ctrl |= so->stride[0] << NV50_3D_STRMOUT_BUFFERS_CTRL_STRIDE__SHIFT;
   }

   so->map_size = base[3] + so->num_attribs[3];

   for (i = 0; i < pso->num_outputs; ++i) {
      const unsigned s = pso->output[i].start_component;
      const unsigned p = pso->output[i].dst_offset;
      const unsigned r = pso->output[i].register_index;
      b = pso->output[i].output_buffer;

      if (r >= info->numOutputs)
         continue;

      for (c = 0; c < pso->output[i].num_components; ++c)
         so->map[base[b] + p + c] = info->out[r].slot[s + c];
   }

   return so;
}

bool
nv50_program_translate(struct nv50_program *prog, uint16_t chipset,
                       struct util_debug_callback *debug)
{
   struct nv50_ir_prog_info *info;
   struct nv50_ir_prog_info_out info_out = {};
   int i, ret;
   const uint8_t map_undef = (prog->type == PIPE_SHADER_VERTEX) ? 0x40 : 0x80;

   info = CALLOC_STRUCT(nv50_ir_prog_info);
   if (!info)
      return false;

   info->type = prog->type;
   info->target = chipset;

   info->bin.nir = nir_shader_clone(NULL, prog->nir);

   info->bin.smemSize = prog->cp.smem_size;
   info->io.auxCBSlot = 15;
   info->io.ucpBase = NV50_CB_AUX_UCP_OFFSET;
   info->io.genUserClip = prog->vp.clpd_nr;
   if (prog->fp.alphatest)
      info->io.alphaRefBase = NV50_CB_AUX_ALPHATEST_OFFSET;

   info->io.suInfoBase = NV50_CB_AUX_TEX_MS_OFFSET;
   info->io.bufInfoBase = NV50_CB_AUX_BUF_INFO(0);
   info->io.sampleInfoBase = NV50_CB_AUX_SAMPLE_OFFSET;
   info->io.msInfoCBSlot = 15;
   info->io.msInfoBase = NV50_CB_AUX_MS_OFFSET;

   info->io.membarOffset = NV50_CB_AUX_MEMBAR_OFFSET;
   info->io.gmemMembar = 15;

   info->assignSlots = nv50_program_assign_varying_slots;

   prog->vp.bfc[0] = 0xff;
   prog->vp.bfc[1] = 0xff;
   prog->vp.edgeflag = 0xff;
   prog->vp.clpd[0] = map_undef;
   prog->vp.clpd[1] = map_undef;
   prog->vp.psiz = map_undef;
   prog->gp.has_layer = 0;
   prog->gp.has_viewport = 0;

   if (prog->type == PIPE_SHADER_COMPUTE)
      info->prop.cp.inputOffset = 0x14;

   info_out.driverPriv = prog;

#ifndef NDEBUG
   info->optLevel = debug_get_num_option("NV50_PROG_OPTIMIZE", 4);
   info->dbgFlags = debug_get_num_option("NV50_PROG_DEBUG", 0);
   info->omitLineNum = debug_get_num_option("NV50_PROG_DEBUG_OMIT_LINENUM", 0);
#else
   info->optLevel = 4;
#endif

   ret = nv50_ir_generate_code(info, &info_out);
   if (ret) {
      NOUVEAU_ERR("shader translation failed: %i\n", ret);
      goto out;
   }

   prog->code = info_out.bin.code;
   prog->code_size = info_out.bin.codeSize;
   prog->relocs = info_out.bin.relocData;
   prog->fixups = info_out.bin.fixupData;
   prog->max_gpr = MAX2(4, (info_out.bin.maxGPR >> 1) + 1);
   prog->tls_space = info_out.bin.tlsSpace;
   prog->cp.smem_size = info_out.bin.smemSize;
   prog->mul_zero_wins = info->io.mul_zero_wins;
   prog->vp.need_vertex_id = info_out.io.vertexId < PIPE_MAX_SHADER_INPUTS;

   prog->vp.clip_enable = (1 << info_out.io.clipDistances) - 1;
   prog->vp.cull_enable =
      ((1 << info_out.io.cullDistances) - 1) << info_out.io.clipDistances;
   prog->vp.clip_mode = 0;
   for (i = 0; i < info_out.io.cullDistances; ++i)
      prog->vp.clip_mode |= 1 << ((info_out.io.clipDistances + i) * 4);

   if (prog->type == PIPE_SHADER_FRAGMENT) {
      if (info_out.prop.fp.writesDepth) {
         prog->fp.flags[0] |= NV50_3D_FP_CONTROL_EXPORTS_Z;
         prog->fp.flags[1] = 0x11;
      }
      if (info_out.prop.fp.usesDiscard)
         prog->fp.flags[0] |= NV50_3D_FP_CONTROL_USES_KIL;
   } else
   if (prog->type == PIPE_SHADER_GEOMETRY) {
      switch (info_out.prop.gp.outputPrim) {
      case MESA_PRIM_LINE_STRIP:
         prog->gp.prim_type = NV50_3D_GP_OUTPUT_PRIMITIVE_TYPE_LINE_STRIP;
         break;
      case MESA_PRIM_TRIANGLE_STRIP:
         prog->gp.prim_type = NV50_3D_GP_OUTPUT_PRIMITIVE_TYPE_TRIANGLE_STRIP;
         break;
      case MESA_PRIM_POINTS:
      default:
         assert(info_out.prop.gp.outputPrim == MESA_PRIM_POINTS);
         prog->gp.prim_type = NV50_3D_GP_OUTPUT_PRIMITIVE_TYPE_POINTS;
         break;
      }
      prog->gp.vert_count = CLAMP(info_out.prop.gp.maxVertices, 1, 1024);
   } else
   if (prog->type == PIPE_SHADER_COMPUTE) {
      for (i = 0; i < NV50_MAX_GLOBALS; i++) {
         prog->cp.gmem[i] = (struct nv50_gmem_state){
            .valid = info_out.prop.cp.gmem[i].valid,
            .image = info_out.prop.cp.gmem[i].image,
            .slot  = info_out.prop.cp.gmem[i].slot
         };
      }
   }

   if (prog->stream_output.num_outputs)
      prog->so = nv50_program_create_strmout_state(&info_out,
                                                   &prog->stream_output);

   util_debug_message(debug, SHADER_INFO,
                      "type: %d, local: %d, shared: %d, gpr: %d, inst: %d, loops: %d, bytes: %d",
                      prog->type, info_out.bin.tlsSpace, info_out.bin.smemSize,
                      prog->max_gpr, info_out.bin.instructions, info_out.loops,
                      info_out.bin.codeSize);

out:
   ralloc_free(info->bin.nir);
   FREE(info);
   return !ret;
}

bool
nv50_program_upload_code(struct nv50_context *nv50, struct nv50_program *prog)
{
   struct nouveau_heap *heap;
   int ret;
   uint32_t size = align(prog->code_size, 0x40);
   uint8_t prog_type;

   switch (prog->type) {
   case PIPE_SHADER_VERTEX:   heap = nv50->screen->vp_code_heap; break;
   case PIPE_SHADER_GEOMETRY: heap = nv50->screen->gp_code_heap; break;
   case PIPE_SHADER_FRAGMENT: heap = nv50->screen->fp_code_heap; break;
   case PIPE_SHADER_COMPUTE:  heap = nv50->screen->fp_code_heap; break;
   default:
      assert(!"invalid program type");
      return false;
   }

   simple_mtx_assert_locked(&nv50->screen->state_lock);
   ret = nouveau_heap_alloc(heap, size, prog, &prog->mem);
   if (ret) {
      /* Out of space: evict everything to compactify the code segment, hoping
       * the working set is much smaller and drifts slowly. Improve me !
       */
      while (heap->next) {
         struct nv50_program *evict = heap->next->priv;
         if (evict)
            nouveau_heap_free(&evict->mem);
      }
      debug_printf("WARNING: out of code space, evicting all shaders.\n");
      ret = nouveau_heap_alloc(heap, size, prog, &prog->mem);
      if (ret) {
         NOUVEAU_ERR("shader too large (0x%x) to fit in code space ?\n", size);
         return false;
      }
   }

   if (prog->type == PIPE_SHADER_COMPUTE) {
      /* CP code must be uploaded in FP code segment. */
      prog_type = NV50_SHADER_STAGE_FRAGMENT;
   } else {
      prog->code_base = prog->mem->start;
      prog_type = nv50_context_shader_stage(prog->type);
   }

   ret = nv50_tls_realloc(nv50->screen, prog->tls_space);
   if (ret < 0) {
      nouveau_heap_free(&prog->mem);
      return false;
   }
   if (ret > 0)
      nv50->state.new_tls_space = true;

   if (prog->relocs)
      nv50_ir_relocate_code(prog->relocs, prog->code, prog->code_base, 0, 0);
   if (prog->fixups)
      nv50_ir_apply_fixups(prog->fixups, prog->code,
                           prog->fp.force_persample_interp,
                           false /* flatshade */,
                           prog->fp.alphatest - 1,
                           false /* msaa */);

   nv50_sifc_linear_u8(&nv50->base, nv50->screen->code,
                       (prog_type << NV50_CODE_BO_SIZE_LOG2) + prog->code_base,
                       NOUVEAU_BO_VRAM, prog->code_size, prog->code);

   BEGIN_NV04(nv50->base.pushbuf, NV50_3D(CODE_CB_FLUSH), 1);
   PUSH_DATA (nv50->base.pushbuf, 0);

   return true;
}

void
nv50_program_destroy(struct nv50_context *nv50, struct nv50_program *p)
{
   struct nir_shader *nir = p->nir;
   const uint8_t type = p->type;

   if (p->mem) {
      if (nv50)
         simple_mtx_assert_locked(&nv50->screen->state_lock);
      nouveau_heap_free(&p->mem);
   }

   FREE(p->code);

   FREE(p->relocs);
   FREE(p->fixups);
   FREE(p->so);

   memset(p, 0, sizeof(*p));

   p->nir = nir;
   p->type = type;
}
