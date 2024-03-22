/*
 * Copyright (C) 2017-2019 Alyssa Rosenzweig
 * Copyright (C) 2017-2019 Connor Abbott
 * Copyright (C) 2019 Collabora, Ltd.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "genxml/gen_macros.h"
#include "util/set.h"
#include "decode.h"

#if PAN_ARCH <= 9

static void
pandecode_primitive(struct pandecode_context *ctx, const void *p)
{
   pan_unpack(p, PRIMITIVE, primitive);
   DUMP_UNPACKED(ctx, PRIMITIVE, primitive, "Primitive:\n");

#if PAN_ARCH <= 7
   /* Validate an index buffer is present if we need one. TODO: verify
    * relationship between invocation_count and index_count */

   if (primitive.indices) {
      /* Grab the size */
      unsigned size = (primitive.index_type == MALI_INDEX_TYPE_UINT32)
                         ? sizeof(uint32_t)
                         : primitive.index_type;

      /* Ensure we got a size, and if so, validate the index buffer
       * is large enough to hold a full set of indices of the given
       * size */

      if (!size)
         pandecode_log(ctx, "// XXX: index size missing\n");
      else
         pandecode_validate_buffer(ctx, primitive.indices,
                                   primitive.index_count * size);
   } else if (primitive.index_type)
      pandecode_log(ctx, "// XXX: unexpected index size\n");
#endif
}

#if PAN_ARCH <= 7
static void
pandecode_attributes(struct pandecode_context *ctx, mali_ptr addr, int count,
                     bool varying, enum mali_job_type job_type)
{
   char *prefix = varying ? "Varying" : "Attribute";
   assert(addr);

   if (!count) {
      pandecode_log(ctx, "// warn: No %s records\n", prefix);
      return;
   }

   MAP_ADDR(ctx, ATTRIBUTE_BUFFER, addr, cl);

   for (int i = 0; i < count; ++i) {
      pan_unpack(cl + i * pan_size(ATTRIBUTE_BUFFER), ATTRIBUTE_BUFFER, temp);
      DUMP_UNPACKED(ctx, ATTRIBUTE_BUFFER, temp, "%s:\n", prefix);

      switch (temp.type) {
      case MALI_ATTRIBUTE_TYPE_1D_NPOT_DIVISOR_WRITE_REDUCTION:
      case MALI_ATTRIBUTE_TYPE_1D_NPOT_DIVISOR: {
         pan_unpack(cl + (i + 1) * pan_size(ATTRIBUTE_BUFFER),
                    ATTRIBUTE_BUFFER_CONTINUATION_NPOT, temp2);
         pan_print(ctx->dump_stream, ATTRIBUTE_BUFFER_CONTINUATION_NPOT, temp2,
                   (ctx->indent + 1) * 2);
         i++;
         break;
      }
      case MALI_ATTRIBUTE_TYPE_3D_LINEAR:
      case MALI_ATTRIBUTE_TYPE_3D_INTERLEAVED: {
         pan_unpack(cl + (i + 1) * pan_size(ATTRIBUTE_BUFFER_CONTINUATION_3D),
                    ATTRIBUTE_BUFFER_CONTINUATION_3D, temp2);
         pan_print(ctx->dump_stream, ATTRIBUTE_BUFFER_CONTINUATION_3D, temp2,
                   (ctx->indent + 1) * 2);
         i++;
         break;
      }
      default:
         break;
      }
   }
   pandecode_log(ctx, "\n");
}

static unsigned
pandecode_attribute_meta(struct pandecode_context *ctx, int count,
                         mali_ptr attribute, bool varying)
{
   unsigned max = 0;

   for (int i = 0; i < count; ++i, attribute += pan_size(ATTRIBUTE)) {
      MAP_ADDR(ctx, ATTRIBUTE, attribute, cl);
      pan_unpack(cl, ATTRIBUTE, a);
      DUMP_UNPACKED(ctx, ATTRIBUTE, a, "%s:\n",
                    varying ? "Varying" : "Attribute");
      max = MAX2(max, a.buffer_index);
   }

   pandecode_log(ctx, "\n");
   return MIN2(max + 1, 256);
}

/* return bits [lo, hi) of word */
static u32
bits(u32 word, u32 lo, u32 hi)
{
   if (hi - lo >= 32)
      return word; // avoid undefined behavior with the shift

   if (lo >= 32)
      return 0;

   return (word >> lo) & ((1 << (hi - lo)) - 1);
}

static void
pandecode_invocation(struct pandecode_context *ctx, const void *i)
{
   /* Decode invocation_count. See the comment before the definition of
    * invocation_count for an explanation.
    */
   pan_unpack(i, INVOCATION, invocation);

   unsigned size_x =
      bits(invocation.invocations, 0, invocation.size_y_shift) + 1;
   unsigned size_y = bits(invocation.invocations, invocation.size_y_shift,
                          invocation.size_z_shift) +
                     1;
   unsigned size_z = bits(invocation.invocations, invocation.size_z_shift,
                          invocation.workgroups_x_shift) +
                     1;

   unsigned groups_x =
      bits(invocation.invocations, invocation.workgroups_x_shift,
           invocation.workgroups_y_shift) +
      1;
   unsigned groups_y =
      bits(invocation.invocations, invocation.workgroups_y_shift,
           invocation.workgroups_z_shift) +
      1;
   unsigned groups_z =
      bits(invocation.invocations, invocation.workgroups_z_shift, 32) + 1;

   pandecode_log(ctx, "Invocation (%d, %d, %d) x (%d, %d, %d)\n", size_x,
                 size_y, size_z, groups_x, groups_y, groups_z);

   DUMP_UNPACKED(ctx, INVOCATION, invocation, "Invocation:\n")
}

static void
pandecode_textures(struct pandecode_context *ctx, mali_ptr textures,
                   unsigned texture_count)
{
   if (!textures)
      return;

   pandecode_log(ctx, "Textures %" PRIx64 ":\n", textures);
   ctx->indent++;

#if PAN_ARCH >= 6
   const void *cl =
      pandecode_fetch_gpu_mem(ctx, textures, pan_size(TEXTURE) * texture_count);

   for (unsigned tex = 0; tex < texture_count; ++tex)
      GENX(pandecode_texture)(ctx, cl + pan_size(TEXTURE) * tex, tex);
#else
   mali_ptr *PANDECODE_PTR_VAR(ctx, u, textures);

   for (int tex = 0; tex < texture_count; ++tex) {
      mali_ptr *PANDECODE_PTR_VAR(ctx, u, textures + tex * sizeof(mali_ptr));
      char *a = pointer_as_memory_reference(ctx, *u);
      pandecode_log(ctx, "%s,\n", a);
      free(a);
   }

   /* Now, finally, descend down into the texture descriptor */
   for (unsigned tex = 0; tex < texture_count; ++tex) {
      mali_ptr *PANDECODE_PTR_VAR(ctx, u, textures + tex * sizeof(mali_ptr));
      GENX(pandecode_texture)(ctx, *u, tex);
   }
#endif
   ctx->indent--;
   pandecode_log(ctx, "\n");
}

static void
pandecode_samplers(struct pandecode_context *ctx, mali_ptr samplers,
                   unsigned sampler_count)
{
   pandecode_log(ctx, "Samplers %" PRIx64 ":\n", samplers);
   ctx->indent++;

   for (int i = 0; i < sampler_count; ++i)
      DUMP_ADDR(ctx, SAMPLER, samplers + (pan_size(SAMPLER) * i),
                "Sampler %d:\n", i);

   ctx->indent--;
   pandecode_log(ctx, "\n");
}

static void
pandecode_uniform_buffers(struct pandecode_context *ctx, mali_ptr pubufs,
                          int ubufs_count)
{
   uint64_t *PANDECODE_PTR_VAR(ctx, ubufs, pubufs);

   for (int i = 0; i < ubufs_count; i++) {
      mali_ptr addr = (ubufs[i] >> 10) << 2;
      unsigned size = addr ? (((ubufs[i] & ((1 << 10) - 1)) + 1) * 16) : 0;

      pandecode_validate_buffer(ctx, addr, size);

      char *ptr = pointer_as_memory_reference(ctx, addr);
      pandecode_log(ctx, "ubuf_%d[%u] = %s;\n", i, size, ptr);
      free(ptr);
   }

   pandecode_log(ctx, "\n");
}

static void
pandecode_uniforms(struct pandecode_context *ctx, mali_ptr uniforms,
                   unsigned uniform_count)
{
   pandecode_validate_buffer(ctx, uniforms, uniform_count * 16);

   char *ptr = pointer_as_memory_reference(ctx, uniforms);
   pandecode_log(ctx, "vec4 uniforms[%u] = %s;\n", uniform_count, ptr);
   free(ptr);
   pandecode_log(ctx, "\n");
}

void
GENX(pandecode_dcd)(struct pandecode_context *ctx, const struct MALI_DRAW *p,
                    enum mali_job_type job_type, unsigned gpu_id)
{
#if PAN_ARCH >= 5
   struct pandecode_fbd fbd_info = {.rt_count = 1};
#endif

   if (PAN_ARCH >= 6 || (PAN_ARCH == 5 && job_type != MALI_JOB_TYPE_TILER)) {
#if PAN_ARCH >= 5
      DUMP_ADDR(ctx, LOCAL_STORAGE, p->thread_storage & ~1, "Local Storage:\n");
#endif
   } else {
#if PAN_ARCH == 5
      /* On v5 only, the actual framebuffer pointer is tagged with extra
       * metadata that we validate but do not print.
       */
      pan_unpack(&p->fbd, FRAMEBUFFER_POINTER, ptr);

      if (!ptr.type || ptr.zs_crc_extension_present ||
          ptr.render_target_count != 1) {

         fprintf(ctx->dump_stream, "Unexpected framebuffer pointer settings");
      }

      GENX(pandecode_fbd)(ctx, ptr.pointer, false, gpu_id);
#elif PAN_ARCH == 4
      GENX(pandecode_fbd)(ctx, p->fbd, false, gpu_id);
#endif
   }

   int varying_count = 0, attribute_count = 0, uniform_count = 0,
       uniform_buffer_count = 0;
   int texture_count = 0, sampler_count = 0;

   if (p->state) {
      uint32_t *cl =
         pandecode_fetch_gpu_mem(ctx, p->state, pan_size(RENDERER_STATE));

      pan_unpack(cl, RENDERER_STATE, state);

      if (state.shader.shader & ~0xF)
         pandecode_shader_disassemble(ctx, state.shader.shader & ~0xF, gpu_id);

#if PAN_ARCH >= 6
      bool idvs = (job_type == MALI_JOB_TYPE_INDEXED_VERTEX);

      if (idvs && state.secondary_shader)
         pandecode_shader_disassemble(ctx, state.secondary_shader, gpu_id);
#endif
      DUMP_UNPACKED(ctx, RENDERER_STATE, state, "State:\n");
      ctx->indent++;

      /* Save for dumps */
      attribute_count = state.shader.attribute_count;
      varying_count = state.shader.varying_count;
      texture_count = state.shader.texture_count;
      sampler_count = state.shader.sampler_count;
      uniform_buffer_count = state.properties.uniform_buffer_count;

#if PAN_ARCH >= 6
      uniform_count = state.preload.uniform_count;
#else
      uniform_count = state.properties.uniform_count;
#endif

#if PAN_ARCH == 4
      mali_ptr shader = state.blend_shader & ~0xF;
      if (state.multisample_misc.blend_shader && shader)
         pandecode_shader_disassemble(ctx, shader, gpu_id);
#endif
      ctx->indent--;
      pandecode_log(ctx, "\n");

      /* MRT blend fields are used on v5+. Technically, they are optional on v5
       * for backwards compatibility but we don't care about that.
       */
#if PAN_ARCH >= 5
      if ((job_type == MALI_JOB_TYPE_TILER ||
           job_type == MALI_JOB_TYPE_FRAGMENT) &&
          PAN_ARCH >= 5) {
         void *blend_base = ((void *)cl) + pan_size(RENDERER_STATE);

         for (unsigned i = 0; i < fbd_info.rt_count; i++) {
            mali_ptr shader =
               GENX(pandecode_blend)(ctx, blend_base, i, state.shader.shader);
            if (shader & ~0xF)
               pandecode_shader_disassemble(ctx, shader, gpu_id);
         }
      }
#endif
   } else
      pandecode_log(ctx, "// XXX: missing shader descriptor\n");

   if (p->viewport) {
      DUMP_ADDR(ctx, VIEWPORT, p->viewport, "Viewport:\n");
      pandecode_log(ctx, "\n");
   }

   unsigned max_attr_index = 0;

   if (p->attributes)
      max_attr_index =
         pandecode_attribute_meta(ctx, attribute_count, p->attributes, false);

   if (p->attribute_buffers)
      pandecode_attributes(ctx, p->attribute_buffers, max_attr_index, false,
                           job_type);

   if (p->varyings) {
      varying_count =
         pandecode_attribute_meta(ctx, varying_count, p->varyings, true);
   }

   if (p->varying_buffers)
      pandecode_attributes(ctx, p->varying_buffers, varying_count, true,
                           job_type);

   if (p->uniform_buffers) {
      if (uniform_buffer_count)
         pandecode_uniform_buffers(ctx, p->uniform_buffers,
                                   uniform_buffer_count);
      else
         pandecode_log(ctx, "// warn: UBOs specified but not referenced\n");
   } else if (uniform_buffer_count)
      pandecode_log(ctx, "// XXX: UBOs referenced but not specified\n");

   /* We don't want to actually dump uniforms, but we do need to validate
    * that the counts we were given are sane */

   if (p->push_uniforms) {
      if (uniform_count)
         pandecode_uniforms(ctx, p->push_uniforms, uniform_count);
      else
         pandecode_log(ctx, "// warn: Uniforms specified but not referenced\n");
   } else if (uniform_count)
      pandecode_log(ctx, "// XXX: Uniforms referenced but not specified\n");

   if (p->textures)
      pandecode_textures(ctx, p->textures, texture_count);

   if (p->samplers)
      pandecode_samplers(ctx, p->samplers, sampler_count);
}

static void
pandecode_vertex_compute_geometry_job(struct pandecode_context *ctx,
                                      const struct MALI_JOB_HEADER *h,
                                      mali_ptr job, unsigned gpu_id)
{
   struct mali_compute_job_packed *PANDECODE_PTR_VAR(ctx, p, job);
   pan_section_unpack(p, COMPUTE_JOB, DRAW, draw);
   GENX(pandecode_dcd)(ctx, &draw, h->type, gpu_id);

   pandecode_log(ctx, "Vertex Job Payload:\n");
   ctx->indent++;
   pandecode_invocation(ctx, pan_section_ptr(p, COMPUTE_JOB, INVOCATION));
   DUMP_SECTION(ctx, COMPUTE_JOB, PARAMETERS, p, "Vertex Job Parameters:\n");
   DUMP_UNPACKED(ctx, DRAW, draw, "Draw:\n");
   ctx->indent--;
   pandecode_log(ctx, "\n");
}
#endif

static void
pandecode_write_value_job(struct pandecode_context *ctx, mali_ptr job)
{
   struct mali_write_value_job_packed *PANDECODE_PTR_VAR(ctx, p, job);
   pan_section_unpack(p, WRITE_VALUE_JOB, PAYLOAD, u);
   DUMP_SECTION(ctx, WRITE_VALUE_JOB, PAYLOAD, p, "Write Value Payload:\n");
   pandecode_log(ctx, "\n");
}

static void
pandecode_cache_flush_job(struct pandecode_context *ctx, mali_ptr job)
{
   struct mali_cache_flush_job_packed *PANDECODE_PTR_VAR(ctx, p, job);
   pan_section_unpack(p, CACHE_FLUSH_JOB, PAYLOAD, u);
   DUMP_SECTION(ctx, CACHE_FLUSH_JOB, PAYLOAD, p, "Cache Flush Payload:\n");
   pandecode_log(ctx, "\n");
}

static void
pandecode_tiler_job(struct pandecode_context *ctx,
                    const struct MALI_JOB_HEADER *h, mali_ptr job,
                    unsigned gpu_id)
{
   struct mali_tiler_job_packed *PANDECODE_PTR_VAR(ctx, p, job);
   pan_section_unpack(p, TILER_JOB, DRAW, draw);
   GENX(pandecode_dcd)(ctx, &draw, h->type, gpu_id);
   pandecode_log(ctx, "Tiler Job Payload:\n");
   ctx->indent++;

#if PAN_ARCH <= 7
   pandecode_invocation(ctx, pan_section_ptr(p, TILER_JOB, INVOCATION));
#endif

   pandecode_primitive(ctx, pan_section_ptr(p, TILER_JOB, PRIMITIVE));
   DUMP_UNPACKED(ctx, DRAW, draw, "Draw:\n");

   DUMP_SECTION(ctx, TILER_JOB, PRIMITIVE_SIZE, p, "Primitive Size:\n");

#if PAN_ARCH >= 6
   pan_section_unpack(p, TILER_JOB, TILER, tiler_ptr);
   GENX(pandecode_tiler)(ctx, tiler_ptr.address, gpu_id);

#if PAN_ARCH >= 9
   DUMP_SECTION(ctx, TILER_JOB, INSTANCE_COUNT, p, "Instance count:\n");
   DUMP_SECTION(ctx, TILER_JOB, VERTEX_COUNT, p, "Vertex count:\n");
   DUMP_SECTION(ctx, TILER_JOB, SCISSOR, p, "Scissor:\n");
   DUMP_SECTION(ctx, TILER_JOB, INDICES, p, "Indices:\n");
#else
   pan_section_unpack(p, TILER_JOB, PADDING, padding);
#endif

#endif
   ctx->indent--;
   pandecode_log(ctx, "\n");
}

static void
pandecode_fragment_job(struct pandecode_context *ctx, mali_ptr job,
                       unsigned gpu_id)
{
   struct mali_fragment_job_packed *PANDECODE_PTR_VAR(ctx, p, job);
   pan_section_unpack(p, FRAGMENT_JOB, PAYLOAD, s);

   uint64_t fbd_pointer;

#if PAN_ARCH >= 5
   /* On v5 and newer, the actual framebuffer pointer is tagged with extra
    * metadata that we need to disregard.
    */
   pan_unpack(&s.framebuffer, FRAMEBUFFER_POINTER, ptr);
   fbd_pointer = ptr.pointer;
#else
   /* On v4, the framebuffer pointer is untagged. */
   fbd_pointer = s.framebuffer;
#endif

   UNUSED struct pandecode_fbd info =
      GENX(pandecode_fbd)(ctx, fbd_pointer, true, gpu_id);

#if PAN_ARCH >= 5
   if (!ptr.type || ptr.zs_crc_extension_present != info.has_extra ||
       ptr.render_target_count != info.rt_count) {
      pandecode_log(ctx, "invalid FBD tag\n");
   }
#endif

   DUMP_UNPACKED(ctx, FRAGMENT_JOB_PAYLOAD, s, "Fragment Job Payload:\n");

   pandecode_log(ctx, "\n");
}

#if PAN_ARCH == 6 || PAN_ARCH == 7
static void
pandecode_indexed_vertex_job(struct pandecode_context *ctx,
                             const struct MALI_JOB_HEADER *h, mali_ptr job,
                             unsigned gpu_id)
{
   struct mali_indexed_vertex_job_packed *PANDECODE_PTR_VAR(ctx, p, job);

   pandecode_log(ctx, "Vertex:\n");
   pan_section_unpack(p, INDEXED_VERTEX_JOB, VERTEX_DRAW, vert_draw);
   GENX(pandecode_dcd)(ctx, &vert_draw, h->type, gpu_id);
   DUMP_UNPACKED(ctx, DRAW, vert_draw, "Vertex Draw:\n");

   pandecode_log(ctx, "Fragment:\n");
   pan_section_unpack(p, INDEXED_VERTEX_JOB, FRAGMENT_DRAW, frag_draw);
   GENX(pandecode_dcd)(ctx, &frag_draw, MALI_JOB_TYPE_FRAGMENT, gpu_id);
   DUMP_UNPACKED(ctx, DRAW, frag_draw, "Fragment Draw:\n");

   pan_section_unpack(p, INDEXED_VERTEX_JOB, TILER, tiler_ptr);
   pandecode_log(ctx, "Tiler Job Payload:\n");
   ctx->indent++;
   GENX(pandecode_tiler)(ctx, tiler_ptr.address, gpu_id);
   ctx->indent--;

   pandecode_invocation(ctx,
                        pan_section_ptr(p, INDEXED_VERTEX_JOB, INVOCATION));
   pandecode_primitive(ctx, pan_section_ptr(p, INDEXED_VERTEX_JOB, PRIMITIVE));

   DUMP_SECTION(ctx, INDEXED_VERTEX_JOB, PRIMITIVE_SIZE, p,
                "Primitive Size:\n");

   pan_section_unpack(p, INDEXED_VERTEX_JOB, PADDING, padding);
}
#endif

#if PAN_ARCH == 9
static void
pandecode_malloc_vertex_job(struct pandecode_context *ctx, mali_ptr job,
                            unsigned gpu_id)
{
   struct mali_malloc_vertex_job_packed *PANDECODE_PTR_VAR(ctx, p, job);

   DUMP_SECTION(ctx, MALLOC_VERTEX_JOB, PRIMITIVE, p, "Primitive:\n");
   DUMP_SECTION(ctx, MALLOC_VERTEX_JOB, INSTANCE_COUNT, p, "Instance count:\n");
   DUMP_SECTION(ctx, MALLOC_VERTEX_JOB, ALLOCATION, p, "Allocation:\n");
   DUMP_SECTION(ctx, MALLOC_VERTEX_JOB, TILER, p, "Tiler:\n");
   DUMP_SECTION(ctx, MALLOC_VERTEX_JOB, SCISSOR, p, "Scissor:\n");
   DUMP_SECTION(ctx, MALLOC_VERTEX_JOB, PRIMITIVE_SIZE, p, "Primitive Size:\n");
   DUMP_SECTION(ctx, MALLOC_VERTEX_JOB, INDICES, p, "Indices:\n");

   pan_section_unpack(p, MALLOC_VERTEX_JOB, DRAW, dcd);

   pan_section_unpack(p, MALLOC_VERTEX_JOB, TILER, tiler_ptr);
   pandecode_log(ctx, "Tiler Job Payload:\n");
   ctx->indent++;
   if (tiler_ptr.address)
      GENX(pandecode_tiler)(ctx, tiler_ptr.address, gpu_id);
   else
      pandecode_log(ctx, "<omitted>\n");
   ctx->indent--;

   GENX(pandecode_dcd)(ctx, &dcd, 0, gpu_id);

   pan_section_unpack(p, MALLOC_VERTEX_JOB, POSITION, position);
   pan_section_unpack(p, MALLOC_VERTEX_JOB, VARYING, varying);
   GENX(pandecode_shader_environment)(ctx, &position, gpu_id);
   GENX(pandecode_shader_environment)(ctx, &varying, gpu_id);
}

static void
pandecode_compute_job(struct pandecode_context *ctx, mali_ptr job,
                      unsigned gpu_id)
{
   struct mali_compute_job_packed *PANDECODE_PTR_VAR(ctx, p, job);
   pan_section_unpack(p, COMPUTE_JOB, PAYLOAD, payload);

   GENX(pandecode_shader_environment)(ctx, &payload.compute, gpu_id);
   DUMP_SECTION(ctx, COMPUTE_JOB, PAYLOAD, p, "Compute");
}
#endif

/*
 * Trace a job chain at a particular GPU address, interpreted for a particular
 * GPU using the job manager.
 */
void
GENX(pandecode_jc)(struct pandecode_context *ctx, mali_ptr jc_gpu_va,
                   unsigned gpu_id)
{
   pandecode_dump_file_open(ctx);

   struct set *va_set = _mesa_pointer_set_create(NULL);
   struct set_entry *entry = NULL;

   mali_ptr next_job = 0;

   do {
      struct mali_job_header_packed *hdr =
         PANDECODE_PTR(ctx, jc_gpu_va, struct mali_job_header_packed);

      entry = _mesa_set_search(va_set, hdr);
      if (entry != NULL) {
         fprintf(stdout, "Job list has a cycle\n");
         break;
      }

      pan_unpack(hdr, JOB_HEADER, h);
      next_job = h.next;

      DUMP_UNPACKED(ctx, JOB_HEADER, h, "Job Header (%" PRIx64 "):\n",
                    jc_gpu_va);
      pandecode_log(ctx, "\n");

      switch (h.type) {
      case MALI_JOB_TYPE_WRITE_VALUE:
         pandecode_write_value_job(ctx, jc_gpu_va);
         break;

      case MALI_JOB_TYPE_CACHE_FLUSH:
         pandecode_cache_flush_job(ctx, jc_gpu_va);
         break;

      case MALI_JOB_TYPE_TILER:
         pandecode_tiler_job(ctx, &h, jc_gpu_va, gpu_id);
         break;

#if PAN_ARCH <= 7
      case MALI_JOB_TYPE_VERTEX:
      case MALI_JOB_TYPE_COMPUTE:
         pandecode_vertex_compute_geometry_job(ctx, &h, jc_gpu_va, gpu_id);
         break;

#if PAN_ARCH >= 6
      case MALI_JOB_TYPE_INDEXED_VERTEX:
         pandecode_indexed_vertex_job(ctx, &h, jc_gpu_va, gpu_id);
         break;
#endif
#else
      case MALI_JOB_TYPE_COMPUTE:
         pandecode_compute_job(ctx, jc_gpu_va, gpu_id);
         break;

      case MALI_JOB_TYPE_MALLOC_VERTEX:
         pandecode_malloc_vertex_job(ctx, jc_gpu_va, gpu_id);
         break;
#endif

      case MALI_JOB_TYPE_FRAGMENT:
         pandecode_fragment_job(ctx, jc_gpu_va, gpu_id);
         break;

      default:
         break;
      }

      /* Track the latest visited job CPU VA to detect cycles */
      _mesa_set_add(va_set, hdr);
   } while ((jc_gpu_va = next_job));

   _mesa_set_destroy(va_set, NULL);

   fflush(ctx->dump_stream);
   pandecode_map_read_write(ctx);
}

void
GENX(pandecode_abort_on_fault)(struct pandecode_context *ctx,
                               mali_ptr jc_gpu_va)
{
   mali_ptr next_job = 0;

   do {
      pan_unpack(PANDECODE_PTR(ctx, jc_gpu_va, struct mali_job_header_packed),
                 JOB_HEADER, h);
      next_job = h.next;

      /* Ensure the job is marked COMPLETE */
      if (h.exception_status != 0x1) {
         fprintf(stderr, "Incomplete job or timeout\n");
         fflush(NULL);
         abort();
      }
   } while ((jc_gpu_va = next_job));

   pandecode_map_read_write(ctx);
}

#endif
