/**************************************************************************
 *
 * Copyright 2015 Advanced Micro Devices, Inc.
 * Copyright 2008 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "dd_pipe.h"

#include "util/u_dump.h"
#include "util/format/u_format.h"
#include "util/u_framebuffer.h"
#include "util/u_helpers.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_process.h"
#include "tgsi/tgsi_parse.h"
#include "tgsi/tgsi_scan.h"
#include "util/os_time.h"
#include <inttypes.h>
#include "util/detect.h"

void
dd_get_debug_filename_and_mkdir(char *buf, size_t buflen, bool verbose)
{
   static unsigned index;
   char dir[256];
   const char *proc_name = util_get_process_name();

   if (!proc_name) {
      fprintf(stderr, "dd: can't get the process name\n");
      proc_name = "unknown";
   }

   snprintf(dir, sizeof(dir), "%s/"DD_DIR, debug_get_option("HOME", "."));

   if (mkdir(dir, 0774) && errno != EEXIST)
      fprintf(stderr, "dd: can't create a directory (%i)\n", errno);

   snprintf(buf, buflen, "%s/%s_%u_%08u", dir, proc_name, (unsigned int)getpid(),
            (unsigned int)p_atomic_inc_return(&index) - 1);

   if (verbose)
      fprintf(stderr, "dd: dumping to file %s\n", buf);
}

FILE *
dd_get_debug_file(bool verbose)
{
   char name[512];
   FILE *f;

   dd_get_debug_filename_and_mkdir(name, sizeof(name), verbose);
   f = fopen(name, "w");
   if (!f) {
      fprintf(stderr, "dd: can't open file %s\n", name);
      return NULL;
   }

   return f;
}

void
dd_parse_apitrace_marker(const char *string, int len, unsigned *call_number)
{
   unsigned num;
   char *s;

   if (len <= 0)
      return;

   /* Make it zero-terminated. */
   s = alloca(len + 1);
   memcpy(s, string, len);
   s[len] = 0;

   /* Parse the number. */
   errno = 0;
   num = strtol(s, NULL, 10);
   if (errno)
      return;

   *call_number = num;
}

void
dd_write_header(FILE *f, struct pipe_screen *screen, unsigned apitrace_call_number)
{
   char cmd_line[4096];
   if (util_get_command_line(cmd_line, sizeof(cmd_line)))
      fprintf(f, "Command: %s\n", cmd_line);
   fprintf(f, "Driver vendor: %s\n", screen->get_vendor(screen));
   fprintf(f, "Device vendor: %s\n", screen->get_device_vendor(screen));
   fprintf(f, "Device name: %s\n\n", screen->get_name(screen));

   if (apitrace_call_number)
      fprintf(f, "Last apitrace call: %u\n\n", apitrace_call_number);
}

FILE *
dd_get_file_stream(struct dd_screen *dscreen, unsigned apitrace_call_number)
{
   struct pipe_screen *screen = dscreen->screen;

   FILE *f = dd_get_debug_file(dscreen->verbose);
   if (!f)
      return NULL;

   dd_write_header(f, screen, apitrace_call_number);
   return f;
}

static void
dd_dump_dmesg(FILE *f)
{
#if DETECT_OS_LINUX
   char line[2000];
   FILE *p = popen("dmesg | tail -n60", "r");

   if (!p)
      return;

   fprintf(f, "\nLast 60 lines of dmesg:\n\n");
   while (fgets(line, sizeof(line), p))
      fputs(line, f);

   pclose(p);
#endif
}

static unsigned
dd_num_active_viewports(struct dd_draw_state *dstate)
{
   struct tgsi_shader_info info;
   const struct tgsi_token *tokens;

   if (dstate->shaders[PIPE_SHADER_GEOMETRY])
      tokens = dstate->shaders[PIPE_SHADER_GEOMETRY]->state.shader.tokens;
   else if (dstate->shaders[PIPE_SHADER_TESS_EVAL])
      tokens = dstate->shaders[PIPE_SHADER_TESS_EVAL]->state.shader.tokens;
   else if (dstate->shaders[PIPE_SHADER_VERTEX])
      tokens = dstate->shaders[PIPE_SHADER_VERTEX]->state.shader.tokens;
   else
      return 1;

   if (tokens) {
      tgsi_scan_shader(tokens, &info);
      if (info.writes_viewport_index)
         return PIPE_MAX_VIEWPORTS;
   }

   return 1;
}

#define COLOR_RESET	"\033[0m"
#define COLOR_SHADER	"\033[1;32m"
#define COLOR_STATE	"\033[1;33m"

#define DUMP(name, var) do { \
   fprintf(f, COLOR_STATE #name ": " COLOR_RESET); \
   util_dump_##name(f, var); \
   fprintf(f, "\n"); \
} while(0)

#define DUMP_I(name, var, i) do { \
   fprintf(f, COLOR_STATE #name " %i: " COLOR_RESET, i); \
   util_dump_##name(f, var); \
   fprintf(f, "\n"); \
} while(0)

#define DUMP_M(name, var, member) do { \
   fprintf(f, "  " #member ": "); \
   util_dump_##name(f, (var)->member); \
   fprintf(f, "\n"); \
} while(0)

#define DUMP_M_ADDR(name, var, member) do { \
   fprintf(f, "  " #member ": "); \
   util_dump_##name(f, &(var)->member); \
   fprintf(f, "\n"); \
} while(0)

#define PRINT_NAMED(type, name, value) \
do { \
   fprintf(f, COLOR_STATE "%s" COLOR_RESET " = ", name); \
   util_dump_##type(f, value); \
   fprintf(f, "\n"); \
} while (0)

static void
util_dump_uint(FILE *f, unsigned i)
{
   fprintf(f, "%u", i);
}

static void
util_dump_int(FILE *f, int i)
{
   fprintf(f, "%d", i);
}

static void
util_dump_hex(FILE *f, unsigned i)
{
   fprintf(f, "0x%x", i);
}

static void
util_dump_double(FILE *f, double d)
{
   fprintf(f, "%f", d);
}

static void
util_dump_format(FILE *f, enum pipe_format format)
{
   fprintf(f, "%s", util_format_name(format));
}

static void
util_dump_color_union(FILE *f, const union pipe_color_union *color)
{
   fprintf(f, "{f = {%f, %f, %f, %f}, ui = {%u, %u, %u, %u}",
           color->f[0], color->f[1], color->f[2], color->f[3],
           color->ui[0], color->ui[1], color->ui[2], color->ui[3]);
}

static void
dd_dump_render_condition(struct dd_draw_state *dstate, FILE *f)
{
   if (dstate->render_cond.query) {
      fprintf(f, "render condition:\n");
      DUMP_M(query_type, &dstate->render_cond, query->type);
      DUMP_M(uint, &dstate->render_cond, condition);
      DUMP_M(uint, &dstate->render_cond, mode);
      fprintf(f, "\n");
   }
}

static void
dd_dump_shader(struct dd_draw_state *dstate, enum pipe_shader_type sh, FILE *f)
{
   int i;
   const char *shader_str[PIPE_SHADER_TYPES];

   shader_str[PIPE_SHADER_VERTEX] = "VERTEX";
   shader_str[PIPE_SHADER_TESS_CTRL] = "TESS_CTRL";
   shader_str[PIPE_SHADER_TESS_EVAL] = "TESS_EVAL";
   shader_str[PIPE_SHADER_GEOMETRY] = "GEOMETRY";
   shader_str[PIPE_SHADER_FRAGMENT] = "FRAGMENT";
   shader_str[PIPE_SHADER_COMPUTE] = "COMPUTE";

   if (sh == PIPE_SHADER_TESS_CTRL &&
       !dstate->shaders[PIPE_SHADER_TESS_CTRL] &&
       dstate->shaders[PIPE_SHADER_TESS_EVAL])
      fprintf(f, "tess_state: {default_outer_level = {%f, %f, %f, %f}, "
              "default_inner_level = {%f, %f}}\n",
              dstate->tess_default_levels[0],
              dstate->tess_default_levels[1],
              dstate->tess_default_levels[2],
              dstate->tess_default_levels[3],
              dstate->tess_default_levels[4],
              dstate->tess_default_levels[5]);

   if (sh == PIPE_SHADER_FRAGMENT)
      if (dstate->rs) {
         unsigned num_viewports = dd_num_active_viewports(dstate);

         if (dstate->rs->state.rs.clip_plane_enable)
            DUMP(clip_state, &dstate->clip_state);

         for (i = 0; i < num_viewports; i++)
            DUMP_I(viewport_state, &dstate->viewports[i], i);

         if (dstate->rs->state.rs.scissor)
            for (i = 0; i < num_viewports; i++)
               DUMP_I(scissor_state, &dstate->scissors[i], i);

         DUMP(rasterizer_state, &dstate->rs->state.rs);

         if (dstate->rs->state.rs.poly_stipple_enable)
            DUMP(poly_stipple, &dstate->polygon_stipple);
         fprintf(f, "\n");
      }

   if (!dstate->shaders[sh])
      return;

   fprintf(f, COLOR_SHADER "begin shader: %s" COLOR_RESET "\n", shader_str[sh]);
   DUMP(shader_state, &dstate->shaders[sh]->state.shader);

   for (i = 0; i < PIPE_MAX_CONSTANT_BUFFERS; i++)
      if (dstate->constant_buffers[sh][i].buffer ||
            dstate->constant_buffers[sh][i].user_buffer) {
         DUMP_I(constant_buffer, &dstate->constant_buffers[sh][i], i);
         if (dstate->constant_buffers[sh][i].buffer)
            DUMP_M(resource, &dstate->constant_buffers[sh][i], buffer);
      }

   for (i = 0; i < PIPE_MAX_SAMPLERS; i++)
      if (dstate->sampler_states[sh][i])
         DUMP_I(sampler_state, &dstate->sampler_states[sh][i]->state.sampler, i);

   for (i = 0; i < PIPE_MAX_SAMPLERS; i++)
      if (dstate->sampler_views[sh][i]) {
         DUMP_I(sampler_view, dstate->sampler_views[sh][i], i);
         DUMP_M(resource, dstate->sampler_views[sh][i], texture);
      }

   for (i = 0; i < PIPE_MAX_SHADER_IMAGES; i++)
      if (dstate->shader_images[sh][i].resource) {
         DUMP_I(image_view, &dstate->shader_images[sh][i], i);
         if (dstate->shader_images[sh][i].resource)
            DUMP_M(resource, &dstate->shader_images[sh][i], resource);
      }

   for (i = 0; i < PIPE_MAX_SHADER_BUFFERS; i++)
      if (dstate->shader_buffers[sh][i].buffer) {
         DUMP_I(shader_buffer, &dstate->shader_buffers[sh][i], i);
         if (dstate->shader_buffers[sh][i].buffer)
            DUMP_M(resource, &dstate->shader_buffers[sh][i], buffer);
      }

   fprintf(f, COLOR_SHADER "end shader: %s" COLOR_RESET "\n\n", shader_str[sh]);
}

static void
dd_dump_flush(struct dd_draw_state *dstate, struct call_flush *info, FILE *f)
{
   fprintf(f, "%s:\n", __func__+8);
   DUMP_M(hex, info, flags);
}

static void
dd_dump_draw_vbo(struct dd_draw_state *dstate, struct pipe_draw_info *info,
                 unsigned drawid_offset,
                 const struct pipe_draw_indirect_info *indirect,
                 const struct pipe_draw_start_count_bias *draw, FILE *f)
{
   int sh, i;

   DUMP(draw_info, info);
   PRINT_NAMED(int, "drawid offset", drawid_offset);
   DUMP(draw_start_count_bias, draw);
   if (indirect) {
      if (indirect->buffer)
         DUMP_M(resource, indirect, buffer);
      if (indirect->indirect_draw_count)
         DUMP_M(resource, indirect, indirect_draw_count);
      if (indirect->count_from_stream_output)
         DUMP_M(stream_output_target, indirect, count_from_stream_output);
   }

   fprintf(f, "\n");

   /* TODO: dump active queries */

   dd_dump_render_condition(dstate, f);

   for (i = 0; i < PIPE_MAX_ATTRIBS; i++)
      if (dstate->vertex_buffers[i].buffer.resource) {
         DUMP_I(vertex_buffer, &dstate->vertex_buffers[i], i);
         if (!dstate->vertex_buffers[i].is_user_buffer)
            DUMP_M(resource, &dstate->vertex_buffers[i], buffer.resource);
      }

   if (dstate->velems) {
      PRINT_NAMED(uint, "num vertex elements",
                        dstate->velems->state.velems.count);
      for (i = 0; i < dstate->velems->state.velems.count; i++) {
         fprintf(f, "  ");
         DUMP_I(vertex_element, &dstate->velems->state.velems.velems[i], i);
      }
   }

   PRINT_NAMED(uint, "num stream output targets", dstate->num_so_targets);
   for (i = 0; i < dstate->num_so_targets; i++)
      if (dstate->so_targets[i]) {
         DUMP_I(stream_output_target, dstate->so_targets[i], i);
         DUMP_M(resource, dstate->so_targets[i], buffer);
         fprintf(f, "  offset = %i\n", dstate->so_offsets[i]);
      }

   fprintf(f, "\n");
   for (sh = 0; sh < PIPE_SHADER_TYPES; sh++) {
      if (sh == PIPE_SHADER_COMPUTE)
         continue;

      dd_dump_shader(dstate, sh, f);
   }

   if (dstate->dsa)
      DUMP(depth_stencil_alpha_state, &dstate->dsa->state.dsa);
   DUMP(stencil_ref, &dstate->stencil_ref);

   if (dstate->blend)
      DUMP(blend_state, &dstate->blend->state.blend);
   DUMP(blend_color, &dstate->blend_color);

   PRINT_NAMED(uint, "min_samples", dstate->min_samples);
   PRINT_NAMED(hex, "sample_mask", dstate->sample_mask);
   fprintf(f, "\n");

   DUMP(framebuffer_state, &dstate->framebuffer_state);
   for (i = 0; i < dstate->framebuffer_state.nr_cbufs; i++)
      if (dstate->framebuffer_state.cbufs[i]) {
         fprintf(f, "  " COLOR_STATE "cbufs[%i]:" COLOR_RESET "\n    ", i);
         DUMP(surface, dstate->framebuffer_state.cbufs[i]);
         fprintf(f, "    ");
         DUMP(resource, dstate->framebuffer_state.cbufs[i]->texture);
      }
   if (dstate->framebuffer_state.zsbuf) {
      fprintf(f, "  " COLOR_STATE "zsbuf:" COLOR_RESET "\n    ");
      DUMP(surface, dstate->framebuffer_state.zsbuf);
      fprintf(f, "    ");
      DUMP(resource, dstate->framebuffer_state.zsbuf->texture);
   }
   fprintf(f, "\n");
}

static void
dd_dump_launch_grid(struct dd_draw_state *dstate, struct pipe_grid_info *info, FILE *f)
{
   fprintf(f, "%s:\n", __func__+8);
   DUMP(grid_info, info);
   fprintf(f, "\n");

   dd_dump_shader(dstate, PIPE_SHADER_COMPUTE, f);
   fprintf(f, "\n");
}

static void
dd_dump_resource_copy_region(struct dd_draw_state *dstate,
                             struct call_resource_copy_region *info,
                             FILE *f)
{
   fprintf(f, "%s:\n", __func__+8);
   DUMP_M(resource, info, dst);
   DUMP_M(uint, info, dst_level);
   DUMP_M(uint, info, dstx);
   DUMP_M(uint, info, dsty);
   DUMP_M(uint, info, dstz);
   DUMP_M(resource, info, src);
   DUMP_M(uint, info, src_level);
   DUMP_M_ADDR(box, info, src_box);
}

static void
dd_dump_blit(struct dd_draw_state *dstate, struct pipe_blit_info *info, FILE *f)
{
   fprintf(f, "%s:\n", __func__+8);
   DUMP_M(resource, info, dst.resource);
   DUMP_M(uint, info, dst.level);
   DUMP_M_ADDR(box, info, dst.box);
   DUMP_M(format, info, dst.format);

   DUMP_M(resource, info, src.resource);
   DUMP_M(uint, info, src.level);
   DUMP_M_ADDR(box, info, src.box);
   DUMP_M(format, info, src.format);

   DUMP_M(hex, info, mask);
   DUMP_M(uint, info, filter);
   DUMP_M(uint, info, scissor_enable);
   DUMP_M_ADDR(scissor_state, info, scissor);
   DUMP_M(uint, info, render_condition_enable);

   if (info->render_condition_enable)
      dd_dump_render_condition(dstate, f);
}

static void
dd_dump_generate_mipmap(struct dd_draw_state *dstate, FILE *f)
{
   fprintf(f, "%s:\n", __func__+8);
   /* TODO */
}

static void
dd_dump_get_query_result_resource(struct call_get_query_result_resource *info, FILE *f)
{
   fprintf(f, "%s:\n", __func__ + 8);
   DUMP_M(query_type, info, query_type);
   DUMP_M(query_flags, info, flags);
   DUMP_M(query_value_type, info, result_type);
   DUMP_M(int, info, index);
   DUMP_M(resource, info, resource);
   DUMP_M(uint, info, offset);
}

static void
dd_dump_flush_resource(struct dd_draw_state *dstate, struct pipe_resource *res,
                       FILE *f)
{
   fprintf(f, "%s:\n", __func__+8);
   DUMP(resource, res);
}

static void
dd_dump_clear(struct dd_draw_state *dstate, struct call_clear *info, FILE *f)
{
   fprintf(f, "%s:\n", __func__+8);
   DUMP_M(uint, info, buffers);
   fprintf(f, "  scissor_state: %d,%d %d,%d\n",
              info->scissor_state.minx, info->scissor_state.miny,
              info->scissor_state.maxx, info->scissor_state.maxy);
   DUMP_M_ADDR(color_union, info, color);
   DUMP_M(double, info, depth);
   DUMP_M(hex, info, stencil);
}

static void
dd_dump_clear_buffer(struct dd_draw_state *dstate, struct call_clear_buffer *info,
                     FILE *f)
{
   int i;
   const char *value = (const char*)info->clear_value;

   fprintf(f, "%s:\n", __func__+8);
   DUMP_M(resource, info, res);
   DUMP_M(uint, info, offset);
   DUMP_M(uint, info, size);
   DUMP_M(uint, info, clear_value_size);

   fprintf(f, "  clear_value:");
   for (i = 0; i < info->clear_value_size; i++)
      fprintf(f, " %02x", value[i]);
   fprintf(f, "\n");
}

static void
dd_dump_transfer_map(struct call_transfer_map *info, FILE *f)
{
   fprintf(f, "%s:\n", __func__+8);
   DUMP_M_ADDR(transfer, info, transfer);
   DUMP_M(ptr, info, transfer_ptr);
   DUMP_M(ptr, info, ptr);
}

static void
dd_dump_transfer_flush_region(struct call_transfer_flush_region *info, FILE *f)
{
   fprintf(f, "%s:\n", __func__+8);
   DUMP_M_ADDR(transfer, info, transfer);
   DUMP_M(ptr, info, transfer_ptr);
   DUMP_M_ADDR(box, info, box);
}

static void
dd_dump_transfer_unmap(struct call_transfer_unmap *info, FILE *f)
{
   fprintf(f, "%s:\n", __func__+8);
   DUMP_M_ADDR(transfer, info, transfer);
   DUMP_M(ptr, info, transfer_ptr);
}

static void
dd_dump_buffer_subdata(struct call_buffer_subdata *info, FILE *f)
{
   fprintf(f, "%s:\n", __func__+8);
   DUMP_M(resource, info, resource);
   DUMP_M(transfer_usage, info, usage);
   DUMP_M(uint, info, offset);
   DUMP_M(uint, info, size);
   DUMP_M(ptr, info, data);
}

static void
dd_dump_texture_subdata(struct call_texture_subdata *info, FILE *f)
{
   fprintf(f, "%s:\n", __func__+8);
   DUMP_M(resource, info, resource);
   DUMP_M(uint, info, level);
   DUMP_M(transfer_usage, info, usage);
   DUMP_M_ADDR(box, info, box);
   DUMP_M(ptr, info, data);
   DUMP_M(uint, info, stride);
   DUMP_M(uint, info, layer_stride);
}

static void
dd_dump_clear_texture(struct dd_draw_state *dstate, FILE *f)
{
   fprintf(f, "%s:\n", __func__+8);
   /* TODO */
}

static void
dd_dump_clear_render_target(struct dd_draw_state *dstate, FILE *f)
{
   fprintf(f, "%s:\n", __func__+8);
   /* TODO */
}

static void
dd_dump_clear_depth_stencil(struct dd_draw_state *dstate, FILE *f)
{
   fprintf(f, "%s:\n", __func__+8);
   /* TODO */
}

static void
dd_dump_driver_state(struct dd_context *dctx, FILE *f, unsigned flags)
{
   if (dctx->pipe->dump_debug_state) {
	   fprintf(f,"\n\n**************************************************"
		     "***************************\n");
	   fprintf(f, "Driver-specific state:\n\n");
	   dctx->pipe->dump_debug_state(dctx->pipe, f, flags);
   }
}

static void
dd_dump_call(FILE *f, struct dd_draw_state *state, struct dd_call *call)
{
   switch (call->type) {
   case CALL_FLUSH:
      dd_dump_flush(state, &call->info.flush, f);
      break;
   case CALL_DRAW_VBO:
      dd_dump_draw_vbo(state, &call->info.draw_vbo.info,
                       call->info.draw_vbo.drawid_offset,
                       &call->info.draw_vbo.indirect,
                       &call->info.draw_vbo.draw, f);
      break;
   case CALL_LAUNCH_GRID:
      dd_dump_launch_grid(state, &call->info.launch_grid, f);
      break;
   case CALL_RESOURCE_COPY_REGION:
      dd_dump_resource_copy_region(state,
                                   &call->info.resource_copy_region, f);
      break;
   case CALL_BLIT:
      dd_dump_blit(state, &call->info.blit, f);
      break;
   case CALL_FLUSH_RESOURCE:
      dd_dump_flush_resource(state, call->info.flush_resource, f);
      break;
   case CALL_CLEAR:
      dd_dump_clear(state, &call->info.clear, f);
      break;
   case CALL_CLEAR_BUFFER:
      dd_dump_clear_buffer(state, &call->info.clear_buffer, f);
      break;
   case CALL_CLEAR_TEXTURE:
      dd_dump_clear_texture(state, f);
      break;
   case CALL_CLEAR_RENDER_TARGET:
      dd_dump_clear_render_target(state, f);
      break;
   case CALL_CLEAR_DEPTH_STENCIL:
      dd_dump_clear_depth_stencil(state, f);
      break;
   case CALL_GENERATE_MIPMAP:
      dd_dump_generate_mipmap(state, f);
      break;
   case CALL_GET_QUERY_RESULT_RESOURCE:
      dd_dump_get_query_result_resource(&call->info.get_query_result_resource, f);
      break;
   case CALL_TRANSFER_MAP:
      dd_dump_transfer_map(&call->info.transfer_map, f);
      break;
   case CALL_TRANSFER_FLUSH_REGION:
      dd_dump_transfer_flush_region(&call->info.transfer_flush_region, f);
      break;
   case CALL_TRANSFER_UNMAP:
      dd_dump_transfer_unmap(&call->info.transfer_unmap, f);
      break;
   case CALL_BUFFER_SUBDATA:
      dd_dump_buffer_subdata(&call->info.buffer_subdata, f);
      break;
   case CALL_TEXTURE_SUBDATA:
      dd_dump_texture_subdata(&call->info.texture_subdata, f);
      break;
   }
}

static void
dd_kill_process(void)
{
#if DETECT_OS_UNIX
   sync();
#endif
   fprintf(stderr, "dd: Aborting the process...\n");
   fflush(stdout);
   fflush(stderr);
   exit(1);
}

static void
dd_unreference_copy_of_call(struct dd_call *dst)
{
   switch (dst->type) {
   case CALL_FLUSH:
      break;
   case CALL_DRAW_VBO:
      pipe_so_target_reference(&dst->info.draw_vbo.indirect.count_from_stream_output, NULL);
      pipe_resource_reference(&dst->info.draw_vbo.indirect.buffer, NULL);
      pipe_resource_reference(&dst->info.draw_vbo.indirect.indirect_draw_count, NULL);
      if (dst->info.draw_vbo.info.index_size &&
          !dst->info.draw_vbo.info.has_user_indices)
         pipe_resource_reference(&dst->info.draw_vbo.info.index.resource, NULL);
      else
         dst->info.draw_vbo.info.index.user = NULL;
      break;
   case CALL_LAUNCH_GRID:
      pipe_resource_reference(&dst->info.launch_grid.indirect, NULL);
      break;
   case CALL_RESOURCE_COPY_REGION:
      pipe_resource_reference(&dst->info.resource_copy_region.dst, NULL);
      pipe_resource_reference(&dst->info.resource_copy_region.src, NULL);
      break;
   case CALL_BLIT:
      pipe_resource_reference(&dst->info.blit.dst.resource, NULL);
      pipe_resource_reference(&dst->info.blit.src.resource, NULL);
      break;
   case CALL_FLUSH_RESOURCE:
      pipe_resource_reference(&dst->info.flush_resource, NULL);
      break;
   case CALL_CLEAR:
      break;
   case CALL_CLEAR_BUFFER:
      pipe_resource_reference(&dst->info.clear_buffer.res, NULL);
      break;
   case CALL_CLEAR_TEXTURE:
      break;
   case CALL_CLEAR_RENDER_TARGET:
      break;
   case CALL_CLEAR_DEPTH_STENCIL:
      break;
   case CALL_GENERATE_MIPMAP:
      pipe_resource_reference(&dst->info.generate_mipmap.res, NULL);
      break;
   case CALL_GET_QUERY_RESULT_RESOURCE:
      pipe_resource_reference(&dst->info.get_query_result_resource.resource, NULL);
      break;
   case CALL_TRANSFER_MAP:
      pipe_resource_reference(&dst->info.transfer_map.transfer.resource, NULL);
      break;
   case CALL_TRANSFER_FLUSH_REGION:
      pipe_resource_reference(&dst->info.transfer_flush_region.transfer.resource, NULL);
      break;
   case CALL_TRANSFER_UNMAP:
      pipe_resource_reference(&dst->info.transfer_unmap.transfer.resource, NULL);
      break;
   case CALL_BUFFER_SUBDATA:
      pipe_resource_reference(&dst->info.buffer_subdata.resource, NULL);
      break;
   case CALL_TEXTURE_SUBDATA:
      pipe_resource_reference(&dst->info.texture_subdata.resource, NULL);
      break;
   }
}

static void
dd_init_copy_of_draw_state(struct dd_draw_state_copy *state)
{
   unsigned i,j;

   /* Just clear pointers to gallium objects. Don't clear the whole structure,
    * because it would kill performance with its size of 130 KB.
    */
   memset(state->base.vertex_buffers, 0,
          sizeof(state->base.vertex_buffers));
   memset(state->base.so_targets, 0,
          sizeof(state->base.so_targets));
   memset(state->base.constant_buffers, 0,
          sizeof(state->base.constant_buffers));
   memset(state->base.sampler_views, 0,
          sizeof(state->base.sampler_views));
   memset(state->base.shader_images, 0,
          sizeof(state->base.shader_images));
   memset(state->base.shader_buffers, 0,
          sizeof(state->base.shader_buffers));
   memset(&state->base.framebuffer_state, 0,
          sizeof(state->base.framebuffer_state));

   memset(state->shaders, 0, sizeof(state->shaders));

   state->base.render_cond.query = &state->render_cond;

   for (i = 0; i < PIPE_SHADER_TYPES; i++) {
      state->base.shaders[i] = &state->shaders[i];
      for (j = 0; j < PIPE_MAX_SAMPLERS; j++)
         state->base.sampler_states[i][j] = &state->sampler_states[i][j];
   }

   state->base.velems = &state->velems;
   state->base.rs = &state->rs;
   state->base.dsa = &state->dsa;
   state->base.blend = &state->blend;
}

static void
dd_unreference_copy_of_draw_state(struct dd_draw_state_copy *state)
{
   struct dd_draw_state *dst = &state->base;
   unsigned i,j;

   for (i = 0; i < ARRAY_SIZE(dst->vertex_buffers); i++)
      pipe_vertex_buffer_unreference(&dst->vertex_buffers[i]);
   for (i = 0; i < ARRAY_SIZE(dst->so_targets); i++)
      pipe_so_target_reference(&dst->so_targets[i], NULL);

   for (i = 0; i < PIPE_SHADER_TYPES; i++) {
      if (dst->shaders[i])
         tgsi_free_tokens(dst->shaders[i]->state.shader.tokens);

      for (j = 0; j < PIPE_MAX_CONSTANT_BUFFERS; j++)
         pipe_resource_reference(&dst->constant_buffers[i][j].buffer, NULL);
      for (j = 0; j < PIPE_MAX_SAMPLERS; j++)
         pipe_sampler_view_reference(&dst->sampler_views[i][j], NULL);
      for (j = 0; j < PIPE_MAX_SHADER_IMAGES; j++)
         pipe_resource_reference(&dst->shader_images[i][j].resource, NULL);
      for (j = 0; j < PIPE_MAX_SHADER_BUFFERS; j++)
         pipe_resource_reference(&dst->shader_buffers[i][j].buffer, NULL);
   }

   util_unreference_framebuffer_state(&dst->framebuffer_state);
}

static void
dd_copy_draw_state(struct dd_draw_state *dst, struct dd_draw_state *src)
{
   unsigned i,j;

   if (src->render_cond.query) {
      *dst->render_cond.query = *src->render_cond.query;
      dst->render_cond.condition = src->render_cond.condition;
      dst->render_cond.mode = src->render_cond.mode;
   } else {
      dst->render_cond.query = NULL;
   }

   for (i = 0; i < ARRAY_SIZE(src->vertex_buffers); i++) {
      pipe_vertex_buffer_reference(&dst->vertex_buffers[i],
                                   &src->vertex_buffers[i]);
   }

   dst->num_so_targets = src->num_so_targets;
   for (i = 0; i < src->num_so_targets; i++)
      pipe_so_target_reference(&dst->so_targets[i], src->so_targets[i]);
   memcpy(dst->so_offsets, src->so_offsets, sizeof(src->so_offsets));

   for (i = 0; i < PIPE_SHADER_TYPES; i++) {
      if (!src->shaders[i]) {
         dst->shaders[i] = NULL;
         continue;
      }

      if (src->shaders[i]) {
         dst->shaders[i]->state.shader = src->shaders[i]->state.shader;
         if (src->shaders[i]->state.shader.tokens) {
            dst->shaders[i]->state.shader.tokens =
               tgsi_dup_tokens(src->shaders[i]->state.shader.tokens);
         } else {
            dst->shaders[i]->state.shader.ir.nir = NULL;
         }
      } else {
         dst->shaders[i] = NULL;
      }

      for (j = 0; j < PIPE_MAX_CONSTANT_BUFFERS; j++) {
         pipe_resource_reference(&dst->constant_buffers[i][j].buffer,
                                 src->constant_buffers[i][j].buffer);
         memcpy(&dst->constant_buffers[i][j], &src->constant_buffers[i][j],
                sizeof(src->constant_buffers[i][j]));
      }

      for (j = 0; j < PIPE_MAX_SAMPLERS; j++) {
         pipe_sampler_view_reference(&dst->sampler_views[i][j],
                                     src->sampler_views[i][j]);
         if (src->sampler_states[i][j])
            dst->sampler_states[i][j]->state.sampler =
               src->sampler_states[i][j]->state.sampler;
         else
            dst->sampler_states[i][j] = NULL;
      }

      for (j = 0; j < PIPE_MAX_SHADER_IMAGES; j++) {
         pipe_resource_reference(&dst->shader_images[i][j].resource,
                                 src->shader_images[i][j].resource);
         memcpy(&dst->shader_images[i][j], &src->shader_images[i][j],
                sizeof(src->shader_images[i][j]));
      }

      for (j = 0; j < PIPE_MAX_SHADER_BUFFERS; j++) {
         pipe_resource_reference(&dst->shader_buffers[i][j].buffer,
                                 src->shader_buffers[i][j].buffer);
         memcpy(&dst->shader_buffers[i][j], &src->shader_buffers[i][j],
                sizeof(src->shader_buffers[i][j]));
      }
   }

   if (src->velems)
      dst->velems->state.velems = src->velems->state.velems;
   else
      dst->velems = NULL;

   if (src->rs)
      dst->rs->state.rs = src->rs->state.rs;
   else
      dst->rs = NULL;

   if (src->dsa)
      dst->dsa->state.dsa = src->dsa->state.dsa;
   else
      dst->dsa = NULL;

   if (src->blend)
      dst->blend->state.blend = src->blend->state.blend;
   else
      dst->blend = NULL;

   dst->blend_color = src->blend_color;
   dst->stencil_ref = src->stencil_ref;
   dst->sample_mask = src->sample_mask;
   dst->min_samples = src->min_samples;
   dst->clip_state = src->clip_state;
   util_copy_framebuffer_state(&dst->framebuffer_state, &src->framebuffer_state);
   memcpy(dst->scissors, src->scissors, sizeof(src->scissors));
   memcpy(dst->viewports, src->viewports, sizeof(src->viewports));
   memcpy(dst->tess_default_levels, src->tess_default_levels,
          sizeof(src->tess_default_levels));
   dst->apitrace_call_number = src->apitrace_call_number;
}

static void
dd_free_record(struct pipe_screen *screen, struct dd_draw_record *record)
{
   u_log_page_destroy(record->log_page);
   dd_unreference_copy_of_call(&record->call);
   dd_unreference_copy_of_draw_state(&record->draw_state);
   screen->fence_reference(screen, &record->prev_bottom_of_pipe, NULL);
   screen->fence_reference(screen, &record->top_of_pipe, NULL);
   screen->fence_reference(screen, &record->bottom_of_pipe, NULL);
   util_queue_fence_destroy(&record->driver_finished);
   FREE(record);
}

static void
dd_write_record(FILE *f, struct dd_draw_record *record)
{
   PRINT_NAMED(ptr, "pipe", record->dctx->pipe);
   PRINT_NAMED(ns, "time before (API call)", record->time_before);
   PRINT_NAMED(ns, "time after (driver done)", record->time_after);
   fprintf(f, "\n");

   dd_dump_call(f, &record->draw_state.base, &record->call);

   if (record->log_page) {
      fprintf(f,"\n\n**************************************************"
                "***************************\n");
      fprintf(f, "Context Log:\n\n");
      u_log_page_print(record->log_page, f);
   }
}

static void
dd_maybe_dump_record(struct dd_screen *dscreen, struct dd_draw_record *record)
{
   if (dscreen->dump_mode == DD_DUMP_ONLY_HANGS ||
       (dscreen->dump_mode == DD_DUMP_APITRACE_CALL &&
        dscreen->apitrace_dump_call != record->draw_state.base.apitrace_call_number))
      return;

   char name[512];
   dd_get_debug_filename_and_mkdir(name, sizeof(name), dscreen->verbose);
   FILE *f = fopen(name, "w");
   if (!f) {
      fprintf(stderr, "dd: failed to open %s\n", name);
      return;
   }

   dd_write_header(f, dscreen->screen, record->draw_state.base.apitrace_call_number);
   dd_write_record(f, record);

   fclose(f);
}

static const char *
dd_fence_state(struct pipe_screen *screen, struct pipe_fence_handle *fence,
               bool *not_reached)
{
   if (!fence)
      return "---";

   bool ok = screen->fence_finish(screen, NULL, fence, 0);

   if (not_reached && !ok)
      *not_reached = true;

   return ok ? "YES" : "NO ";
}

static void
dd_report_hang(struct dd_context *dctx)
{
   struct dd_screen *dscreen = dd_screen(dctx->base.screen);
   struct pipe_screen *screen = dscreen->screen;
   bool encountered_hang = false;
   bool stop_output = false;
   unsigned num_later = 0;

   fprintf(stderr, "GPU hang detected, collecting information...\n\n");

   fprintf(stderr, "Draw #   driver  prev BOP  TOP  BOP  dump file\n"
                   "-------------------------------------------------------------\n");

   list_for_each_entry(struct dd_draw_record, record, &dctx->records, list) {
      if (!encountered_hang &&
          screen->fence_finish(screen, NULL, record->bottom_of_pipe, 0)) {
         dd_maybe_dump_record(dscreen, record);
         continue;
      }

      if (stop_output) {
         dd_maybe_dump_record(dscreen, record);
         num_later++;
         continue;
      }

      bool driver = util_queue_fence_is_signalled(&record->driver_finished);
      bool top_not_reached = false;
      const char *prev_bop = dd_fence_state(screen, record->prev_bottom_of_pipe, NULL);
      const char *top = dd_fence_state(screen, record->top_of_pipe, &top_not_reached);
      const char *bop = dd_fence_state(screen, record->bottom_of_pipe, NULL);

      fprintf(stderr, "%-9u %s      %s     %s  %s  ",
              record->draw_call, driver ? "YES" : "NO ", prev_bop, top, bop);

      char name[512];
      dd_get_debug_filename_and_mkdir(name, sizeof(name), false);

      FILE *f = fopen(name, "w");
      if (!f) {
         fprintf(stderr, "fopen failed\n");
      } else {
         fprintf(stderr, "%s\n", name);

         dd_write_header(f, dscreen->screen, record->draw_state.base.apitrace_call_number);
         dd_write_record(f, record);

         fclose(f);
      }

      if (top_not_reached)
         stop_output = true;
      encountered_hang = true;
   }

   if (num_later)
      fprintf(stderr, "... and %u additional draws.\n", num_later);

   char name[512];
   dd_get_debug_filename_and_mkdir(name, sizeof(name), false);
   FILE *f = fopen(name, "w");
   if (!f) {
      fprintf(stderr, "fopen failed\n");
   } else {
      dd_write_header(f, dscreen->screen, 0);
      dd_dump_driver_state(dctx, f, PIPE_DUMP_DEVICE_STATUS_REGISTERS);
      dd_dump_dmesg(f);
      fclose(f);
   }

   fprintf(stderr, "\nDone.\n");
   dd_kill_process();
}

int
dd_thread_main(void *input)
{
   struct dd_context *dctx = (struct dd_context *)input;
   struct dd_screen *dscreen = dd_screen(dctx->base.screen);
   struct pipe_screen *screen = dscreen->screen;

   const char *process_name = util_get_process_name();
   if (process_name) {
      char threadname[16];
      snprintf(threadname, sizeof(threadname), "%.*s:ddbg",
               (int)MIN2(strlen(process_name), sizeof(threadname) - 6),
               process_name);
      u_thread_setname(threadname);
   }

   mtx_lock(&dctx->mutex);

   for (;;) {
      struct list_head records;
      list_replace(&dctx->records, &records);
      list_inithead(&dctx->records);
      dctx->num_records = 0;

      if (dctx->api_stalled)
         cnd_signal(&dctx->cond);

      if (list_is_empty(&records)) {
         if (dctx->kill_thread)
            break;

         cnd_wait(&dctx->cond, &dctx->mutex);
         continue;
      }

      mtx_unlock(&dctx->mutex);

      /* Wait for the youngest draw. This means hangs can take a bit longer
       * to detect, but it's more efficient this way.  */
      struct dd_draw_record *youngest =
         list_last_entry(&records, struct dd_draw_record, list);

      if (dscreen->timeout_ms > 0) {
         uint64_t abs_timeout = os_time_get_absolute_timeout(
                                 (uint64_t)dscreen->timeout_ms * 1000*1000);

         if (!util_queue_fence_wait_timeout(&youngest->driver_finished, abs_timeout) ||
             !screen->fence_finish(screen, NULL, youngest->bottom_of_pipe,
                                   (uint64_t)dscreen->timeout_ms * 1000*1000)) {
            mtx_lock(&dctx->mutex);
            list_splice(&records, &dctx->records);
            dd_report_hang(dctx);
            /* we won't actually get here */
            mtx_unlock(&dctx->mutex);
         }
      } else {
         util_queue_fence_wait(&youngest->driver_finished);
      }

      list_for_each_entry_safe(struct dd_draw_record, record, &records, list) {
         dd_maybe_dump_record(dscreen, record);
         list_del(&record->list);
         dd_free_record(screen, record);
      }

      mtx_lock(&dctx->mutex);
   }
   mtx_unlock(&dctx->mutex);
   return 0;
}

static struct dd_draw_record *
dd_create_record(struct dd_context *dctx)
{
   struct dd_draw_record *record;

   record = MALLOC_STRUCT(dd_draw_record);
   if (!record)
      return NULL;

   record->dctx = dctx;
   record->draw_call = dctx->num_draw_calls;

   record->prev_bottom_of_pipe = NULL;
   record->top_of_pipe = NULL;
   record->bottom_of_pipe = NULL;
   record->log_page = NULL;
   util_queue_fence_init(&record->driver_finished);
   util_queue_fence_reset(&record->driver_finished);

   dd_init_copy_of_draw_state(&record->draw_state);
   dd_copy_draw_state(&record->draw_state.base, &dctx->draw_state);

   return record;
}

static void
dd_add_record(struct dd_context *dctx, struct dd_draw_record *record)
{
   mtx_lock(&dctx->mutex);
   if (unlikely(dctx->num_records > 10000)) {
      dctx->api_stalled = true;
      /* Since this is only a heuristic to prevent the API thread from getting
       * too far ahead, we don't need a loop here. */
      cnd_wait(&dctx->cond, &dctx->mutex);
      dctx->api_stalled = false;
   }

   if (list_is_empty(&dctx->records))
      cnd_signal(&dctx->cond);

   list_addtail(&record->list, &dctx->records);
   dctx->num_records++;
   mtx_unlock(&dctx->mutex);
}

static void
dd_before_draw(struct dd_context *dctx, struct dd_draw_record *record)
{
   struct dd_screen *dscreen = dd_screen(dctx->base.screen);
   struct pipe_context *pipe = dctx->pipe;
   struct pipe_screen *screen = dscreen->screen;

   record->time_before = os_time_get_nano();

   if (dscreen->timeout_ms > 0) {
      if (dscreen->flush_always && dctx->num_draw_calls >= dscreen->skip_count) {
         pipe->flush(pipe, &record->prev_bottom_of_pipe, 0);
         screen->fence_reference(screen, &record->top_of_pipe, record->prev_bottom_of_pipe);
      } else {
         pipe->flush(pipe, &record->prev_bottom_of_pipe,
                     PIPE_FLUSH_DEFERRED | PIPE_FLUSH_BOTTOM_OF_PIPE);
         pipe->flush(pipe, &record->top_of_pipe,
                     PIPE_FLUSH_DEFERRED | PIPE_FLUSH_TOP_OF_PIPE);
      }
   } else if (dscreen->flush_always && dctx->num_draw_calls >= dscreen->skip_count) {
      pipe->flush(pipe, NULL, 0);
   }

   dd_add_record(dctx, record);
}

static void
dd_after_draw_async(void *data)
{
   struct dd_draw_record *record = (struct dd_draw_record *)data;
   struct dd_context *dctx = record->dctx;
   struct dd_screen *dscreen = dd_screen(dctx->base.screen);

   record->log_page = u_log_new_page(&dctx->log);
   record->time_after = os_time_get_nano();

   util_queue_fence_signal(&record->driver_finished);

   if (dscreen->dump_mode == DD_DUMP_APITRACE_CALL &&
       dscreen->apitrace_dump_call > dctx->draw_state.apitrace_call_number) {
      dd_thread_join(dctx);
      /* No need to continue. */
      exit(0);
   }
}

static void
dd_after_draw(struct dd_context *dctx, struct dd_draw_record *record)
{
   struct dd_screen *dscreen = dd_screen(dctx->base.screen);
   struct pipe_context *pipe = dctx->pipe;

   if (dscreen->timeout_ms > 0) {
      unsigned flush_flags;
      if (dscreen->flush_always && dctx->num_draw_calls >= dscreen->skip_count)
         flush_flags = 0;
      else
         flush_flags = PIPE_FLUSH_DEFERRED | PIPE_FLUSH_BOTTOM_OF_PIPE;
      pipe->flush(pipe, &record->bottom_of_pipe, flush_flags);
   }

   if (pipe->callback) {
      pipe->callback(pipe, dd_after_draw_async, record, true);
   } else {
      dd_after_draw_async(record);
   }

   ++dctx->num_draw_calls;
   if (dscreen->skip_count && dctx->num_draw_calls % 10000 == 0)
      fprintf(stderr, "Gallium debugger reached %u draw calls.\n",
              dctx->num_draw_calls);
}

static void
dd_context_flush(struct pipe_context *_pipe,
                 struct pipe_fence_handle **fence, unsigned flags)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct pipe_screen *screen = pipe->screen;
   struct dd_draw_record *record = dd_create_record(dctx);

   record->call.type = CALL_FLUSH;
   record->call.info.flush.flags = flags;

   record->time_before = os_time_get_nano();

   dd_add_record(dctx, record);

   pipe->flush(pipe, &record->bottom_of_pipe, flags);
   if (fence)
      screen->fence_reference(screen, fence, record->bottom_of_pipe);

   if (pipe->callback) {
      pipe->callback(pipe, dd_after_draw_async, record, true);
   } else {
      dd_after_draw_async(record);
   }
}

static void
dd_context_draw_vbo(struct pipe_context *_pipe,
                    const struct pipe_draw_info *info,
                    unsigned drawid_offset,
                    const struct pipe_draw_indirect_info *indirect,
                    const struct pipe_draw_start_count_bias *draws,
                    unsigned num_draws)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record = dd_create_record(dctx);

   record->call.type = CALL_DRAW_VBO;
   record->call.info.draw_vbo.info = *info;
   record->call.info.draw_vbo.drawid_offset = drawid_offset;
   record->call.info.draw_vbo.draw = draws[0];
   if (info->index_size && !info->has_user_indices) {
      record->call.info.draw_vbo.info.index.resource = NULL;
      pipe_resource_reference(&record->call.info.draw_vbo.info.index.resource,
                              info->index.resource);
   }

   if (indirect) {
      record->call.info.draw_vbo.indirect = *indirect;
      record->call.info.draw_vbo.indirect.buffer = NULL;
      pipe_resource_reference(&record->call.info.draw_vbo.indirect.buffer,
                              indirect->buffer);
      record->call.info.draw_vbo.indirect.indirect_draw_count = NULL;
      pipe_resource_reference(&record->call.info.draw_vbo.indirect.indirect_draw_count,
                              indirect->indirect_draw_count);
      record->call.info.draw_vbo.indirect.count_from_stream_output = NULL;
      pipe_so_target_reference(&record->call.info.draw_vbo.indirect.count_from_stream_output,
                               indirect->count_from_stream_output);
   } else {
      memset(&record->call.info.draw_vbo.indirect, 0, sizeof(*indirect));
   }

   dd_before_draw(dctx, record);
   pipe->draw_vbo(pipe, info, drawid_offset, indirect, draws, num_draws);
   dd_after_draw(dctx, record);
}

static void
dd_context_draw_vertex_state(struct pipe_context *_pipe,
                             struct pipe_vertex_state *state,
                             uint32_t partial_velem_mask,
                             struct pipe_draw_vertex_state_info info,
                             const struct pipe_draw_start_count_bias *draws,
                             unsigned num_draws)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record = dd_create_record(dctx);

   record->call.type = CALL_DRAW_VBO;
   memset(&record->call.info.draw_vbo.info, 0,
          sizeof(record->call.info.draw_vbo.info));
   record->call.info.draw_vbo.info.mode = info.mode;
   record->call.info.draw_vbo.info.index_size = 4;
   record->call.info.draw_vbo.info.instance_count = 1;
   record->call.info.draw_vbo.drawid_offset = 0;
   record->call.info.draw_vbo.draw = draws[0];
   record->call.info.draw_vbo.info.index.resource = NULL;
   pipe_resource_reference(&record->call.info.draw_vbo.info.index.resource,
                           state->input.indexbuf);
   memset(&record->call.info.draw_vbo.indirect, 0,
          sizeof(record->call.info.draw_vbo.indirect));

   dd_before_draw(dctx, record);
   pipe->draw_vertex_state(pipe, state, partial_velem_mask, info, draws, num_draws);
   dd_after_draw(dctx, record);
}

static void
dd_context_launch_grid(struct pipe_context *_pipe,
                       const struct pipe_grid_info *info)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record = dd_create_record(dctx);

   record->call.type = CALL_LAUNCH_GRID;
   record->call.info.launch_grid = *info;
   record->call.info.launch_grid.indirect = NULL;
   pipe_resource_reference(&record->call.info.launch_grid.indirect, info->indirect);

   dd_before_draw(dctx, record);
   pipe->launch_grid(pipe, info);
   dd_after_draw(dctx, record);
}

static void
dd_context_resource_copy_region(struct pipe_context *_pipe,
                                struct pipe_resource *dst, unsigned dst_level,
                                unsigned dstx, unsigned dsty, unsigned dstz,
                                struct pipe_resource *src, unsigned src_level,
                                const struct pipe_box *src_box)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record = dd_create_record(dctx);

   record->call.type = CALL_RESOURCE_COPY_REGION;
   record->call.info.resource_copy_region.dst = NULL;
   pipe_resource_reference(&record->call.info.resource_copy_region.dst, dst);
   record->call.info.resource_copy_region.dst_level = dst_level;
   record->call.info.resource_copy_region.dstx = dstx;
   record->call.info.resource_copy_region.dsty = dsty;
   record->call.info.resource_copy_region.dstz = dstz;
   record->call.info.resource_copy_region.src = NULL;
   pipe_resource_reference(&record->call.info.resource_copy_region.src, src);
   record->call.info.resource_copy_region.src_level = src_level;
   record->call.info.resource_copy_region.src_box = *src_box;

   dd_before_draw(dctx, record);
   pipe->resource_copy_region(pipe,
                              dst, dst_level, dstx, dsty, dstz,
                              src, src_level, src_box);
   dd_after_draw(dctx, record);
}

static void
dd_context_blit(struct pipe_context *_pipe, const struct pipe_blit_info *info)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record = dd_create_record(dctx);

   record->call.type = CALL_BLIT;
   record->call.info.blit = *info;
   record->call.info.blit.dst.resource = NULL;
   pipe_resource_reference(&record->call.info.blit.dst.resource, info->dst.resource);
   record->call.info.blit.src.resource = NULL;
   pipe_resource_reference(&record->call.info.blit.src.resource, info->src.resource);

   dd_before_draw(dctx, record);
   pipe->blit(pipe, info);
   dd_after_draw(dctx, record);
}

static bool
dd_context_generate_mipmap(struct pipe_context *_pipe,
                           struct pipe_resource *res,
                           enum pipe_format format,
                           unsigned base_level,
                           unsigned last_level,
                           unsigned first_layer,
                           unsigned last_layer)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record = dd_create_record(dctx);
   bool result;

   record->call.type = CALL_GENERATE_MIPMAP;
   record->call.info.generate_mipmap.res = NULL;
   pipe_resource_reference(&record->call.info.generate_mipmap.res, res);
   record->call.info.generate_mipmap.format = format;
   record->call.info.generate_mipmap.base_level = base_level;
   record->call.info.generate_mipmap.last_level = last_level;
   record->call.info.generate_mipmap.first_layer = first_layer;
   record->call.info.generate_mipmap.last_layer = last_layer;

   dd_before_draw(dctx, record);
   result = pipe->generate_mipmap(pipe, res, format, base_level, last_level,
                                  first_layer, last_layer);
   dd_after_draw(dctx, record);
   return result;
}

static void
dd_context_get_query_result_resource(struct pipe_context *_pipe,
                                     struct pipe_query *query,
                                     enum pipe_query_flags flags,
                                     enum pipe_query_value_type result_type,
                                     int index,
                                     struct pipe_resource *resource,
                                     unsigned offset)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct dd_query *dquery = dd_query(query);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record = dd_create_record(dctx);

   record->call.type = CALL_GET_QUERY_RESULT_RESOURCE;
   record->call.info.get_query_result_resource.query = query;
   record->call.info.get_query_result_resource.flags = flags;
   record->call.info.get_query_result_resource.result_type = result_type;
   record->call.info.get_query_result_resource.index = index;
   record->call.info.get_query_result_resource.resource = NULL;
   pipe_resource_reference(&record->call.info.get_query_result_resource.resource,
                           resource);
   record->call.info.get_query_result_resource.offset = offset;

   /* The query may be deleted by the time we need to print it. */
   record->call.info.get_query_result_resource.query_type = dquery->type;

   dd_before_draw(dctx, record);
   pipe->get_query_result_resource(pipe, dquery->query, flags,
                                   result_type, index, resource, offset);
   dd_after_draw(dctx, record);
}

static void
dd_context_flush_resource(struct pipe_context *_pipe,
                          struct pipe_resource *resource)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record = dd_create_record(dctx);

   record->call.type = CALL_FLUSH_RESOURCE;
   record->call.info.flush_resource = NULL;
   pipe_resource_reference(&record->call.info.flush_resource, resource);

   dd_before_draw(dctx, record);
   pipe->flush_resource(pipe, resource);
   dd_after_draw(dctx, record);
}

static void
dd_context_clear(struct pipe_context *_pipe, unsigned buffers, const struct pipe_scissor_state *scissor_state,
                 const union pipe_color_union *color, double depth,
                 unsigned stencil)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record = dd_create_record(dctx);

   record->call.type = CALL_CLEAR;
   record->call.info.clear.buffers = buffers;
   if (scissor_state)
      record->call.info.clear.scissor_state = *scissor_state;
   record->call.info.clear.color = *color;
   record->call.info.clear.depth = depth;
   record->call.info.clear.stencil = stencil;

   dd_before_draw(dctx, record);
   pipe->clear(pipe, buffers, scissor_state, color, depth, stencil);
   dd_after_draw(dctx, record);
}

static void
dd_context_clear_render_target(struct pipe_context *_pipe,
                               struct pipe_surface *dst,
                               const union pipe_color_union *color,
                               unsigned dstx, unsigned dsty,
                               unsigned width, unsigned height,
                               bool render_condition_enabled)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record = dd_create_record(dctx);

   record->call.type = CALL_CLEAR_RENDER_TARGET;

   dd_before_draw(dctx, record);
   pipe->clear_render_target(pipe, dst, color, dstx, dsty, width, height,
                             render_condition_enabled);
   dd_after_draw(dctx, record);
}

static void
dd_context_clear_depth_stencil(struct pipe_context *_pipe,
                               struct pipe_surface *dst, unsigned clear_flags,
                               double depth, unsigned stencil, unsigned dstx,
                               unsigned dsty, unsigned width, unsigned height,
                               bool render_condition_enabled)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record = dd_create_record(dctx);

   record->call.type = CALL_CLEAR_DEPTH_STENCIL;

   dd_before_draw(dctx, record);
   pipe->clear_depth_stencil(pipe, dst, clear_flags, depth, stencil,
                             dstx, dsty, width, height,
                             render_condition_enabled);
   dd_after_draw(dctx, record);
}

static void
dd_context_clear_buffer(struct pipe_context *_pipe, struct pipe_resource *res,
                        unsigned offset, unsigned size,
                        const void *clear_value, int clear_value_size)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record = dd_create_record(dctx);

   record->call.type = CALL_CLEAR_BUFFER;
   record->call.info.clear_buffer.res = NULL;
   pipe_resource_reference(&record->call.info.clear_buffer.res, res);
   record->call.info.clear_buffer.offset = offset;
   record->call.info.clear_buffer.size = size;
   record->call.info.clear_buffer.clear_value = clear_value;
   record->call.info.clear_buffer.clear_value_size = clear_value_size;

   dd_before_draw(dctx, record);
   pipe->clear_buffer(pipe, res, offset, size, clear_value, clear_value_size);
   dd_after_draw(dctx, record);
}

static void
dd_context_clear_texture(struct pipe_context *_pipe,
                         struct pipe_resource *res,
                         unsigned level,
                         const struct pipe_box *box,
                         const void *data)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record = dd_create_record(dctx);

   record->call.type = CALL_CLEAR_TEXTURE;

   dd_before_draw(dctx, record);
   pipe->clear_texture(pipe, res, level, box, data);
   dd_after_draw(dctx, record);
}

/********************************************************************
 * transfer
 */

static void *
dd_context_buffer_map(struct pipe_context *_pipe,
                      struct pipe_resource *resource, unsigned level,
                      unsigned usage, const struct pipe_box *box,
                      struct pipe_transfer **transfer)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record =
      dd_screen(dctx->base.screen)->transfers ? dd_create_record(dctx) : NULL;

   if (record) {
      record->call.type = CALL_TRANSFER_MAP;

      dd_before_draw(dctx, record);
   }
   void *ptr = pipe->buffer_map(pipe, resource, level, usage, box, transfer);
   if (record) {
      record->call.info.transfer_map.transfer_ptr = *transfer;
      record->call.info.transfer_map.ptr = ptr;
      if (*transfer) {
         record->call.info.transfer_map.transfer = **transfer;
         record->call.info.transfer_map.transfer.resource = NULL;
         pipe_resource_reference(&record->call.info.transfer_map.transfer.resource,
                                 (*transfer)->resource);
      } else {
         memset(&record->call.info.transfer_map.transfer, 0, sizeof(struct pipe_transfer));
      }

      dd_after_draw(dctx, record);
   }
   return ptr;
}

static void *
dd_context_texture_map(struct pipe_context *_pipe,
                       struct pipe_resource *resource, unsigned level,
                       unsigned usage, const struct pipe_box *box,
                       struct pipe_transfer **transfer)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record =
      dd_screen(dctx->base.screen)->transfers ? dd_create_record(dctx) : NULL;

   if (record) {
      record->call.type = CALL_TRANSFER_MAP;

      dd_before_draw(dctx, record);
   }
   void *ptr = pipe->texture_map(pipe, resource, level, usage, box, transfer);
   if (record) {
      record->call.info.transfer_map.transfer_ptr = *transfer;
      record->call.info.transfer_map.ptr = ptr;
      if (*transfer) {
         record->call.info.transfer_map.transfer = **transfer;
         record->call.info.transfer_map.transfer.resource = NULL;
         pipe_resource_reference(&record->call.info.transfer_map.transfer.resource,
                                 (*transfer)->resource);
      } else {
         memset(&record->call.info.transfer_map.transfer, 0, sizeof(struct pipe_transfer));
      }

      dd_after_draw(dctx, record);
   }
   return ptr;
}

static void
dd_context_transfer_flush_region(struct pipe_context *_pipe,
                                 struct pipe_transfer *transfer,
                                 const struct pipe_box *box)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record =
      dd_screen(dctx->base.screen)->transfers ? dd_create_record(dctx) : NULL;

   if (record) {
      record->call.type = CALL_TRANSFER_FLUSH_REGION;
      record->call.info.transfer_flush_region.transfer_ptr = transfer;
      record->call.info.transfer_flush_region.box = *box;
      record->call.info.transfer_flush_region.transfer = *transfer;
      record->call.info.transfer_flush_region.transfer.resource = NULL;
      pipe_resource_reference(
            &record->call.info.transfer_flush_region.transfer.resource,
            transfer->resource);

      dd_before_draw(dctx, record);
   }
   pipe->transfer_flush_region(pipe, transfer, box);
   if (record)
      dd_after_draw(dctx, record);
}

static void
dd_context_buffer_unmap(struct pipe_context *_pipe,
                          struct pipe_transfer *transfer)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record =
      dd_screen(dctx->base.screen)->transfers ? dd_create_record(dctx) : NULL;

   if (record) {
      record->call.type = CALL_TRANSFER_UNMAP;
      record->call.info.transfer_unmap.transfer_ptr = transfer;
      record->call.info.transfer_unmap.transfer = *transfer;
      record->call.info.transfer_unmap.transfer.resource = NULL;
      pipe_resource_reference(
            &record->call.info.transfer_unmap.transfer.resource,
            transfer->resource);

      dd_before_draw(dctx, record);
   }
   pipe->buffer_unmap(pipe, transfer);
   if (record)
      dd_after_draw(dctx, record);
}

static void
dd_context_texture_unmap(struct pipe_context *_pipe,
                          struct pipe_transfer *transfer)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record =
      dd_screen(dctx->base.screen)->transfers ? dd_create_record(dctx) : NULL;

   if (record) {
      record->call.type = CALL_TRANSFER_UNMAP;
      record->call.info.transfer_unmap.transfer_ptr = transfer;
      record->call.info.transfer_unmap.transfer = *transfer;
      record->call.info.transfer_unmap.transfer.resource = NULL;
      pipe_resource_reference(
            &record->call.info.transfer_unmap.transfer.resource,
            transfer->resource);

      dd_before_draw(dctx, record);
   }
   pipe->texture_unmap(pipe, transfer);
   if (record)
      dd_after_draw(dctx, record);
}

static void
dd_context_buffer_subdata(struct pipe_context *_pipe,
                          struct pipe_resource *resource,
                          unsigned usage, unsigned offset,
                          unsigned size, const void *data)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record =
      dd_screen(dctx->base.screen)->transfers ? dd_create_record(dctx) : NULL;

   if (record) {
      record->call.type = CALL_BUFFER_SUBDATA;
      record->call.info.buffer_subdata.resource = NULL;
      pipe_resource_reference(&record->call.info.buffer_subdata.resource, resource);
      record->call.info.buffer_subdata.usage = usage;
      record->call.info.buffer_subdata.offset = offset;
      record->call.info.buffer_subdata.size = size;
      record->call.info.buffer_subdata.data = data;

      dd_before_draw(dctx, record);
   }
   pipe->buffer_subdata(pipe, resource, usage, offset, size, data);
   if (record)
      dd_after_draw(dctx, record);
}

static void
dd_context_texture_subdata(struct pipe_context *_pipe,
                           struct pipe_resource *resource,
                           unsigned level, unsigned usage,
                           const struct pipe_box *box,
                           const void *data, unsigned stride,
                           uintptr_t layer_stride)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_record *record =
      dd_screen(dctx->base.screen)->transfers ? dd_create_record(dctx) : NULL;

   if (record) {
      record->call.type = CALL_TEXTURE_SUBDATA;
      record->call.info.texture_subdata.resource = NULL;
      pipe_resource_reference(&record->call.info.texture_subdata.resource, resource);
      record->call.info.texture_subdata.level = level;
      record->call.info.texture_subdata.usage = usage;
      record->call.info.texture_subdata.box = *box;
      record->call.info.texture_subdata.data = data;
      record->call.info.texture_subdata.stride = stride;
      record->call.info.texture_subdata.layer_stride = layer_stride;

      dd_before_draw(dctx, record);
   }
   pipe->texture_subdata(pipe, resource, level, usage, box, data,
                         stride, layer_stride);
   if (record)
      dd_after_draw(dctx, record);
}

void
dd_init_draw_functions(struct dd_context *dctx)
{
   CTX_INIT(flush);
   CTX_INIT(draw_vbo);
   CTX_INIT(launch_grid);
   CTX_INIT(resource_copy_region);
   CTX_INIT(blit);
   CTX_INIT(clear);
   CTX_INIT(clear_render_target);
   CTX_INIT(clear_depth_stencil);
   CTX_INIT(clear_buffer);
   CTX_INIT(clear_texture);
   CTX_INIT(flush_resource);
   CTX_INIT(generate_mipmap);
   CTX_INIT(get_query_result_resource);
   CTX_INIT(buffer_map);
   CTX_INIT(texture_map);
   CTX_INIT(transfer_flush_region);
   CTX_INIT(buffer_unmap);
   CTX_INIT(texture_unmap);
   CTX_INIT(buffer_subdata);
   CTX_INIT(texture_subdata);
   CTX_INIT(draw_vertex_state);
}
