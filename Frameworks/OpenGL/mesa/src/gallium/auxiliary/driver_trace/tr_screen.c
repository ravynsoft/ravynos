/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
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

#include "util/format/u_format.h"
#include "util/u_memory.h"
#include "util/hash_table.h"

#include "tr_dump.h"
#include "tr_dump_defines.h"
#include "tr_dump_state.h"
#include "tr_texture.h"
#include "tr_context.h"
#include "tr_screen.h"
#include "tr_public.h"
#include "tr_util.h"


static bool trace = false;
static struct hash_table *trace_screens;

static const char *
trace_screen_get_name(struct pipe_screen *_screen)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   const char *result;

   trace_dump_call_begin("pipe_screen", "get_name");

   trace_dump_arg(ptr, screen);

   result = screen->get_name(screen);

   trace_dump_ret(string, result);

   trace_dump_call_end();

   return result;
}


static const char *
trace_screen_get_vendor(struct pipe_screen *_screen)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   const char *result;

   trace_dump_call_begin("pipe_screen", "get_vendor");

   trace_dump_arg(ptr, screen);

   result = screen->get_vendor(screen);

   trace_dump_ret(string, result);

   trace_dump_call_end();

   return result;
}


static const char *
trace_screen_get_device_vendor(struct pipe_screen *_screen)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   const char *result;

   trace_dump_call_begin("pipe_screen", "get_device_vendor");

   trace_dump_arg(ptr, screen);

   result = screen->get_device_vendor(screen);

   trace_dump_ret(string, result);

   trace_dump_call_end();

   return result;
}


static const void *
trace_screen_get_compiler_options(struct pipe_screen *_screen,
                                  enum pipe_shader_ir ir,
                                  enum pipe_shader_type shader)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   const void *result;

   trace_dump_call_begin("pipe_screen", "get_compiler_options");

   trace_dump_arg(ptr, screen);
   trace_dump_arg_enum(pipe_shader_ir, ir);
   trace_dump_arg_enum(pipe_shader_type, shader);

   result = screen->get_compiler_options(screen, ir, shader);

   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   return result;
}


static struct disk_cache *
trace_screen_get_disk_shader_cache(struct pipe_screen *_screen)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;

   trace_dump_call_begin("pipe_screen", "get_disk_shader_cache");

   trace_dump_arg(ptr, screen);

   struct disk_cache *result = screen->get_disk_shader_cache(screen);

   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   return result;
}


static int
trace_screen_get_param(struct pipe_screen *_screen,
                       enum pipe_cap param)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   int result;

   trace_dump_call_begin("pipe_screen", "get_param");

   trace_dump_arg(ptr, screen);
   trace_dump_arg_enum(pipe_cap, param);

   result = screen->get_param(screen, param);

   trace_dump_ret(int, result);

   trace_dump_call_end();

   return result;
}


static int
trace_screen_get_shader_param(struct pipe_screen *_screen,
                              enum pipe_shader_type shader,
                              enum pipe_shader_cap param)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   int result;

   trace_dump_call_begin("pipe_screen", "get_shader_param");

   trace_dump_arg(ptr, screen);
   trace_dump_arg_enum(pipe_shader_type, shader);
   trace_dump_arg_enum(pipe_shader_cap, param);

   result = screen->get_shader_param(screen, shader, param);

   trace_dump_ret(int, result);

   trace_dump_call_end();

   return result;
}


static float
trace_screen_get_paramf(struct pipe_screen *_screen,
                        enum pipe_capf param)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   float result;

   trace_dump_call_begin("pipe_screen", "get_paramf");

   trace_dump_arg(ptr, screen);
   trace_dump_arg_enum(pipe_capf, param);

   result = screen->get_paramf(screen, param);

   trace_dump_ret(float, result);

   trace_dump_call_end();

   return result;
}


static int
trace_screen_get_compute_param(struct pipe_screen *_screen,
                               enum pipe_shader_ir ir_type,
                               enum pipe_compute_cap param,
                               void *data)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   int result;

   trace_dump_call_begin("pipe_screen", "get_compute_param");

   trace_dump_arg(ptr, screen);
   trace_dump_arg_enum(pipe_shader_ir, ir_type);
   trace_dump_arg_enum(pipe_compute_cap, param);
   trace_dump_arg(ptr, data);

   result = screen->get_compute_param(screen, ir_type, param, data);

   trace_dump_ret(int, result);

   trace_dump_call_end();

   return result;
}

static int
trace_screen_get_video_param(struct pipe_screen *_screen,
                             enum pipe_video_profile profile,
                             enum pipe_video_entrypoint entrypoint,
                             enum pipe_video_cap param)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   int result;

   trace_dump_call_begin("pipe_screen", "get_video_param");

   trace_dump_arg(ptr, screen);
   trace_dump_arg_enum(pipe_video_profile, profile);
   trace_dump_arg_enum(pipe_video_entrypoint, entrypoint);
   trace_dump_arg_enum(pipe_video_cap, param);

   result = screen->get_video_param(screen, profile, entrypoint, param);

   trace_dump_ret(int, result);

   trace_dump_call_end();

   return result;
}

static bool
trace_screen_is_format_supported(struct pipe_screen *_screen,
                                 enum pipe_format format,
                                 enum pipe_texture_target target,
                                 unsigned sample_count,
                                 unsigned storage_sample_count,
                                 unsigned tex_usage)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   bool result;

   trace_dump_call_begin("pipe_screen", "is_format_supported");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(format, format);
   trace_dump_arg_enum(pipe_texture_target, target);
   trace_dump_arg(uint, sample_count);
   trace_dump_arg(uint, storage_sample_count);
   trace_dump_arg(uint, tex_usage);

   result = screen->is_format_supported(screen, format, target, sample_count,
                                        storage_sample_count, tex_usage);

   trace_dump_ret(bool, result);

   trace_dump_call_end();

   return result;
}

static bool
trace_screen_is_video_format_supported(struct pipe_screen *_screen,
                                       enum pipe_format format,
                                       enum pipe_video_profile profile,
                                       enum pipe_video_entrypoint entrypoint)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   bool result;

   trace_dump_call_begin("pipe_screen", "is_video_format_supported");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(format, format);
   trace_dump_arg_enum(pipe_video_profile, profile);
   trace_dump_arg_enum(pipe_video_entrypoint, entrypoint);

   result = screen->is_video_format_supported(screen, format, profile, entrypoint);

   trace_dump_ret(bool, result);

   trace_dump_call_end();

   return result;
}

static void
trace_screen_driver_thread_add_job(struct pipe_screen *_screen,
                                   void *data, struct util_queue_fence *fence,
                                   pipe_driver_thread_func execute,
                                   pipe_driver_thread_func cleanup,
                                   const size_t job_size)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;

   trace_dump_call_begin("pipe_screen", "driver_thread_add_job");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, data);
   trace_dump_arg(ptr, fence);

   screen->driver_thread_add_job(screen, data, fence, execute, cleanup, job_size);

   trace_dump_call_end();
}

static void
trace_context_replace_buffer_storage(struct pipe_context *_pipe,
                                     struct pipe_resource *dst,
                                     struct pipe_resource *src,
                                     unsigned num_rebinds,
                                     uint32_t rebind_mask,
                                     unsigned delete_buffer_id)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "replace_buffer_storage");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, dst);
   trace_dump_arg(ptr, src);
   trace_dump_arg(uint, num_rebinds);
   trace_dump_arg(uint, rebind_mask);
   trace_dump_arg(uint, delete_buffer_id);
   trace_dump_call_end();

   tr_ctx->replace_buffer_storage(pipe, dst, src, num_rebinds, rebind_mask, delete_buffer_id);
}

static struct pipe_fence_handle *
trace_context_create_fence(struct pipe_context *_pipe, struct tc_unflushed_batch_token *token)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "create_fence");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, token);

   struct pipe_fence_handle *ret = tr_ctx->create_fence(pipe, token);
   trace_dump_ret(ptr, ret);
   trace_dump_call_end();

   return ret;
}

static bool
trace_context_is_resource_busy(struct pipe_screen *_screen,
                               struct pipe_resource *resource,
                               unsigned usage)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   bool result;

   trace_dump_call_begin("pipe_screen", "is_resource_busy");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, resource);
   trace_dump_arg(uint, usage);

   result = tr_scr->is_resource_busy(screen, resource, usage);

   trace_dump_ret(bool, result);

   trace_dump_call_end();

   return result;
}

struct pipe_context *
trace_context_create_threaded(struct pipe_screen *screen, struct pipe_context *pipe,
                              tc_replace_buffer_storage_func *replace_buffer,
                              struct threaded_context_options *options)
{
   if (!trace_screens)
      return pipe;

   struct hash_entry *he = _mesa_hash_table_search(trace_screens, screen);
   if (!he)
      return pipe;
   struct trace_screen *tr_scr = trace_screen(he->data);

   if (tr_scr->trace_tc)
      return pipe;

   struct pipe_context *ctx = trace_context_create(tr_scr, pipe);
   if (!ctx)
      return pipe;

   struct trace_context *tr_ctx = trace_context(ctx);
   tr_ctx->replace_buffer_storage = *replace_buffer;
   tr_ctx->create_fence = options->create_fence;
   tr_scr->is_resource_busy = options->is_resource_busy;
   tr_ctx->threaded = true;
   *replace_buffer = trace_context_replace_buffer_storage;
   if (options->create_fence)
      options->create_fence = trace_context_create_fence;
   if (options->is_resource_busy)
      options->is_resource_busy = trace_context_is_resource_busy;
   return ctx;
}

static struct pipe_context *
trace_screen_context_create(struct pipe_screen *_screen, void *priv,
                            unsigned flags)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   struct pipe_context *result;

   result = screen->context_create(screen, priv, flags);

   trace_dump_call_begin("pipe_screen", "context_create");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, priv);
   trace_dump_arg(uint, flags);

   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   if (result && (tr_scr->trace_tc || result->draw_vbo != tc_draw_vbo))
      result = trace_context_create(tr_scr, result);

   return result;
}


static void
trace_screen_flush_frontbuffer(struct pipe_screen *_screen,
                               struct pipe_context *_pipe,
                               struct pipe_resource *resource,
                               unsigned level, unsigned layer,
                               void *context_private,
                               struct pipe_box *sub_box)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   struct pipe_context *pipe = _pipe ? trace_get_possibly_threaded_context(_pipe) : NULL;

   trace_dump_call_begin("pipe_screen", "flush_frontbuffer");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, resource);
   trace_dump_arg(uint, level);
   trace_dump_arg(uint, layer);
   /* XXX: hide, as there is nothing we can do with this
   trace_dump_arg(ptr, context_private);
   */

   trace_dump_call_end();

   screen->flush_frontbuffer(screen, pipe, resource, level, layer, context_private, sub_box);
}


static void
trace_screen_get_driver_uuid(struct pipe_screen *_screen, char *uuid)
{
   struct pipe_screen *screen = trace_screen(_screen)->screen;

   trace_dump_call_begin("pipe_screen", "get_driver_uuid");
   trace_dump_arg(ptr, screen);

   screen->get_driver_uuid(screen, uuid);

   trace_dump_ret(string, uuid);
   trace_dump_call_end();
}

static void
trace_screen_get_device_uuid(struct pipe_screen *_screen, char *uuid)
{
   struct pipe_screen *screen = trace_screen(_screen)->screen;

   trace_dump_call_begin("pipe_screen", "get_device_uuid");
   trace_dump_arg(ptr, screen);

   screen->get_device_uuid(screen, uuid);

   trace_dump_ret(string, uuid);
   trace_dump_call_end();
}

static void
trace_screen_get_device_luid(struct pipe_screen *_screen, char *luid)
{
   struct pipe_screen *screen = trace_screen(_screen)->screen;

   trace_dump_call_begin("pipe_screen", "get_device_luid");
   trace_dump_arg(ptr, screen);

   screen->get_device_luid(screen, luid);

   trace_dump_ret(string, luid);
   trace_dump_call_end();
}

static uint32_t
trace_screen_get_device_node_mask(struct pipe_screen *_screen)
{
   struct pipe_screen *screen = trace_screen(_screen)->screen;
   uint32_t result;

   trace_dump_call_begin("pipe_screen", "get_device_node_mask");
   trace_dump_arg(ptr, screen);

   result = screen->get_device_node_mask(screen);

   trace_dump_ret(uint, result);
   trace_dump_call_end();

   return result;
}


/********************************************************************
 * texture
 */

static void *
trace_screen_map_memory(struct pipe_screen *_screen,
                        struct pipe_memory_allocation *pmem)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   void *result;

   trace_dump_call_begin("pipe_screen", "map_memory");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, pmem);

   result = screen->map_memory(screen, pmem);

   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   return result;
}

static void
trace_screen_unmap_memory(struct pipe_screen *_screen,
                          struct pipe_memory_allocation *pmem)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;

   trace_dump_call_begin("pipe_screen", "unmap_memory");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, pmem);

   screen->unmap_memory(screen, pmem);


   trace_dump_call_end();
}

static struct pipe_memory_allocation *
trace_screen_allocate_memory(struct pipe_screen *_screen,
                             uint64_t size)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   struct pipe_memory_allocation *result;

   trace_dump_call_begin("pipe_screen", "allocate_memory");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(uint, size);

   result = screen->allocate_memory(screen, size);

   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   return result;
}

static struct pipe_memory_allocation *
trace_screen_allocate_memory_fd(struct pipe_screen *_screen,
                                uint64_t size,
                                int *fd)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   struct pipe_memory_allocation *result;

   trace_dump_call_begin("pipe_screen", "allocate_memory_fd");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(uint, size);
   trace_dump_arg(ptr, fd);

   result = screen->allocate_memory_fd(screen, size, fd);

   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   return result;
}

static void
trace_screen_free_memory(struct pipe_screen *_screen,
                         struct pipe_memory_allocation *pmem)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;

   trace_dump_call_begin("pipe_screen", "free_memory");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, pmem);

   screen->free_memory(screen, pmem);


   trace_dump_call_end();
}

static void
trace_screen_free_memory_fd(struct pipe_screen *_screen,
                         struct pipe_memory_allocation *pmem)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;

   trace_dump_call_begin("pipe_screen", "free_memory_fd");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, pmem);

   screen->free_memory_fd(screen, pmem);


   trace_dump_call_end();
}

static bool
trace_screen_resource_bind_backing(struct pipe_screen *_screen,
                                   struct pipe_resource *resource,
                                   struct pipe_memory_allocation *pmem,
                                   uint64_t offset)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   bool result;

   trace_dump_call_begin("pipe_screen", "resource_bind_backing");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, resource);
   trace_dump_arg(ptr, pmem);
   trace_dump_arg(uint, offset);

   result = screen->resource_bind_backing(screen, resource, pmem, offset);

   trace_dump_ret(bool, result);

   trace_dump_call_end();

   return result;
}

static struct pipe_resource *
trace_screen_resource_create_unbacked(struct pipe_screen *_screen,
                                      const struct pipe_resource *templat,
                                      uint64_t *size_required)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   struct pipe_resource *result;

   trace_dump_call_begin("pipe_screen", "resource_create_unbacked");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(resource_template, templat);

   result = screen->resource_create_unbacked(screen, templat, size_required);

   trace_dump_ret_begin();
   trace_dump_uint(*size_required);
   trace_dump_ret_end();
   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   if (result)
      result->screen = _screen;
   return result;
}

static struct pipe_resource *
trace_screen_resource_create(struct pipe_screen *_screen,
                             const struct pipe_resource *templat)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   struct pipe_resource *result;

   trace_dump_call_begin("pipe_screen", "resource_create");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(resource_template, templat);

   result = screen->resource_create(screen, templat);

   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   if (result)
      result->screen = _screen;
   return result;
}

static struct pipe_resource *
trace_screen_resource_create_drawable(struct pipe_screen *_screen,
                                      const struct pipe_resource *templat,
                                      const void *loader_data)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   struct pipe_resource *result;

   trace_dump_call_begin("pipe_screen", "resource_create_drawable");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(resource_template, templat);
   trace_dump_arg(ptr, loader_data);

   result = screen->resource_create_drawable(screen, templat, loader_data);

   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   if (result)
      result->screen = _screen;
   return result;
}

static struct pipe_resource *
trace_screen_resource_create_with_modifiers(struct pipe_screen *_screen, const struct pipe_resource *templat,
                                            const uint64_t *modifiers, int modifiers_count)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   struct pipe_resource *result;

   trace_dump_call_begin("pipe_screen", "resource_create_with_modifiers");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(resource_template, templat);
   trace_dump_arg_array(uint, modifiers, modifiers_count);

   result = screen->resource_create_with_modifiers(screen, templat, modifiers, modifiers_count);

   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   if (result)
      result->screen = _screen;
   return result;
}

static struct pipe_resource *
trace_screen_resource_from_handle(struct pipe_screen *_screen,
                                  const struct pipe_resource *templ,
                                  struct winsys_handle *handle,
                                  unsigned usage)
{
   struct trace_screen *tr_screen = trace_screen(_screen);
   struct pipe_screen *screen = tr_screen->screen;
   struct pipe_resource *result;

   trace_dump_call_begin("pipe_screen", "resource_from_handle");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(resource_template, templ);
   trace_dump_arg(winsys_handle, handle);
   trace_dump_arg(uint, usage);

   result = screen->resource_from_handle(screen, templ, handle, usage);

   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   if (result)
      result->screen = _screen;
   return result;
}

static bool
trace_screen_check_resource_capability(struct pipe_screen *_screen,
                                       struct pipe_resource *resource,
                                       unsigned bind)
{
   struct pipe_screen *screen = trace_screen(_screen)->screen;

   return screen->check_resource_capability(screen, resource, bind);
}

static bool
trace_screen_resource_get_handle(struct pipe_screen *_screen,
                                 struct pipe_context *_pipe,
                                 struct pipe_resource *resource,
                                 struct winsys_handle *handle,
                                 unsigned usage)
{
   struct trace_screen *tr_screen = trace_screen(_screen);
   struct pipe_context *pipe = _pipe ? trace_get_possibly_threaded_context(_pipe) : NULL;
   struct pipe_screen *screen = tr_screen->screen;
   bool result;

   trace_dump_call_begin("pipe_screen", "resource_get_handle");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, resource);
   trace_dump_arg(uint, usage);

   result = screen->resource_get_handle(screen, pipe, resource, handle, usage);

   trace_dump_arg(winsys_handle, handle);
   trace_dump_ret(bool, result);

   trace_dump_call_end();

   return result;
}

static bool
trace_screen_resource_get_param(struct pipe_screen *_screen,
                                struct pipe_context *_pipe,
                                struct pipe_resource *resource,
                                unsigned plane,
                                unsigned layer,
                                unsigned level,
                                enum pipe_resource_param param,
                                unsigned handle_usage,
                                uint64_t *value)
{
   struct trace_screen *tr_screen = trace_screen(_screen);
   struct pipe_context *pipe = _pipe ? trace_get_possibly_threaded_context(_pipe) : NULL;
   struct pipe_screen *screen = tr_screen->screen;
   bool result;

   trace_dump_call_begin("pipe_screen", "resource_get_param");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, resource);
   trace_dump_arg(uint, plane);
   trace_dump_arg(uint, layer);
   trace_dump_arg(uint, level);
   trace_dump_arg_enum(pipe_resource_param, param);
   trace_dump_arg(uint, handle_usage);

   result = screen->resource_get_param(screen, pipe,
                                       resource, plane, layer, level, param,
                                       handle_usage, value);

   trace_dump_arg(uint, *value);
   trace_dump_ret(bool, result);

   trace_dump_call_end();

   return result;
}

static void
trace_screen_resource_get_info(struct pipe_screen *_screen,
                               struct pipe_resource *resource,
                               unsigned *stride,
                               unsigned *offset)
{
   struct trace_screen *tr_screen = trace_screen(_screen);
   struct pipe_screen *screen = tr_screen->screen;

   trace_dump_call_begin("pipe_screen", "resource_get_info");
   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, resource);

   screen->resource_get_info(screen, resource, stride, offset);

   trace_dump_arg(uint, *stride);
   trace_dump_arg(uint, *offset);

   trace_dump_call_end();
}

static struct pipe_resource *
trace_screen_resource_from_memobj(struct pipe_screen *_screen,
                                  const struct pipe_resource *templ,
                                  struct pipe_memory_object *memobj,
                                  uint64_t offset)
{
   struct pipe_screen *screen = trace_screen(_screen)->screen;

   trace_dump_call_begin("pipe_screen", "resource_from_memobj");
   trace_dump_arg(ptr, screen);
   trace_dump_arg(resource_template, templ);
   trace_dump_arg(ptr, memobj);
   trace_dump_arg(uint, offset);

   struct pipe_resource *res =
      screen->resource_from_memobj(screen, templ, memobj, offset);

   if (!res)
      return NULL;
   res->screen = _screen;

   trace_dump_ret(ptr, res);
   trace_dump_call_end();
   return res;
}

static void
trace_screen_resource_changed(struct pipe_screen *_screen,
                              struct pipe_resource *resource)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;

   trace_dump_call_begin("pipe_screen", "resource_changed");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, resource);

   if (screen->resource_changed)
      screen->resource_changed(screen, resource);

   trace_dump_call_end();
}

static void
trace_screen_resource_destroy(struct pipe_screen *_screen,
			      struct pipe_resource *resource)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;

   /* Don't trace this, because due to the lack of pipe_resource wrapping,
    * we can get this call from inside of driver calls, which would try
    * to lock an already-locked mutex.
    */
   screen->resource_destroy(screen, resource);
}


/********************************************************************
 * fence
 */


static void
trace_screen_fence_reference(struct pipe_screen *_screen,
                             struct pipe_fence_handle **pdst,
                             struct pipe_fence_handle *src)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   struct pipe_fence_handle *dst;

   assert(pdst);
   dst = *pdst;

   trace_dump_call_begin("pipe_screen", "fence_reference");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, dst);
   trace_dump_arg(ptr, src);

   screen->fence_reference(screen, pdst, src);

   trace_dump_call_end();
}


static int
trace_screen_fence_get_fd(struct pipe_screen *_screen,
                          struct pipe_fence_handle *fence)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   int result;

   trace_dump_call_begin("pipe_screen", "fence_get_fd");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, fence);

   result = screen->fence_get_fd(screen, fence);

   trace_dump_ret(int, result);

   trace_dump_call_end();

   return result;
}

static void
trace_screen_create_fence_win32(struct pipe_screen *_screen,
                                struct pipe_fence_handle **fence,
                                void *handle,
                                const void *name,
                                enum pipe_fd_type type)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;

   trace_dump_call_begin("pipe_screen", "create_fence_win32");

   trace_dump_arg(ptr, screen);
   if (fence)
      trace_dump_arg(ptr, *fence);
   trace_dump_arg(ptr, handle);
   trace_dump_arg(ptr, name);
   trace_dump_arg_enum(pipe_fd_type, type);

   trace_dump_call_end();

   screen->create_fence_win32(screen, fence, handle, name, type);
}


static bool
trace_screen_fence_finish(struct pipe_screen *_screen,
                          struct pipe_context *_ctx,
                          struct pipe_fence_handle *fence,
                          uint64_t timeout)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   struct pipe_context *ctx = _ctx ? trace_get_possibly_threaded_context(_ctx) : NULL;
   int result;

   result = screen->fence_finish(screen, ctx, fence, timeout);


   trace_dump_call_begin("pipe_screen", "fence_finish");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, ctx);
   trace_dump_arg(ptr, fence);
   trace_dump_arg(uint, timeout);

   trace_dump_ret(bool, result);

   trace_dump_call_end();

   return result;
}


/********************************************************************
 * memobj
 */

static struct pipe_memory_object *
trace_screen_memobj_create_from_handle(struct pipe_screen *_screen,
                                       struct winsys_handle *handle,
                                       bool dedicated)
{
   struct pipe_screen *screen = trace_screen(_screen)->screen;

   trace_dump_call_begin("pipe_screen", "memobj_create_from_handle");
   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, handle);
   trace_dump_arg(bool, dedicated);

   struct pipe_memory_object *res =
      screen->memobj_create_from_handle(screen, handle, dedicated);

   trace_dump_ret(ptr, res);
   trace_dump_call_end();

   return res;
}

static void
trace_screen_memobj_destroy(struct pipe_screen *_screen,
                            struct pipe_memory_object *memobj)
{
   struct pipe_screen *screen = trace_screen(_screen)->screen;

   trace_dump_call_begin("pipe_screen", "memobj_destroy");
   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, memobj);
   trace_dump_call_end();

   screen->memobj_destroy(screen, memobj);
}


/********************************************************************
 * screen
 */

static uint64_t
trace_screen_get_timestamp(struct pipe_screen *_screen)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;
   uint64_t result;

   trace_dump_call_begin("pipe_screen", "get_timestamp");
   trace_dump_arg(ptr, screen);

   result = screen->get_timestamp(screen);

   trace_dump_ret(uint, result);
   trace_dump_call_end();

   return result;
}

static char *
trace_screen_finalize_nir(struct pipe_screen *_screen, void *nir)
{
   struct pipe_screen *screen = trace_screen(_screen)->screen;

   return screen->finalize_nir(screen, nir);
}

static void
trace_screen_destroy(struct pipe_screen *_screen)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;

   trace_dump_call_begin("pipe_screen", "destroy");
   trace_dump_arg(ptr, screen);
   trace_dump_call_end();

   if (trace_screens) {
      struct hash_entry *he = _mesa_hash_table_search(trace_screens, screen);
      if (he) {
         _mesa_hash_table_remove(trace_screens, he);
         if (!_mesa_hash_table_num_entries(trace_screens)) {
            _mesa_hash_table_destroy(trace_screens, NULL);
            trace_screens = NULL;
         }
      }
   }

   screen->destroy(screen);

   FREE(tr_scr);
}

static void
trace_screen_query_memory_info(struct pipe_screen *_screen, struct pipe_memory_info *info)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;

   trace_dump_call_begin("pipe_screen", "query_memory_info");

   trace_dump_arg(ptr, screen);

   screen->query_memory_info(screen, info);

   trace_dump_ret(memory_info, info);

   trace_dump_call_end();
}

static void
trace_screen_query_dmabuf_modifiers(struct pipe_screen *_screen, enum pipe_format format, int max, uint64_t *modifiers, unsigned int *external_only, int *count)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;

   trace_dump_call_begin("pipe_screen", "query_dmabuf_modifiers");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(format, format);
   trace_dump_arg(int, max);

   screen->query_dmabuf_modifiers(screen, format, max, modifiers, external_only, count);

   if (max)
      trace_dump_arg_array(uint, modifiers, *count);
   else
      trace_dump_arg_array(uint, modifiers, max);
   trace_dump_arg_array(uint, external_only, max);
   trace_dump_ret_begin();
   trace_dump_uint(*count);
   trace_dump_ret_end();

   trace_dump_call_end();
}

static bool
trace_screen_is_compute_copy_faster(struct pipe_screen *_screen, enum pipe_format src_format,
                                    enum pipe_format dst_format, unsigned width, unsigned height,
                                    unsigned depth, bool cpu)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;

   trace_dump_call_begin("pipe_screen", "is_compute_copy_faster");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(format, src_format);
   trace_dump_arg(format, dst_format);
   trace_dump_arg(uint, width);
   trace_dump_arg(uint, height);
   trace_dump_arg(uint, depth);
   trace_dump_arg(bool, cpu);

   bool ret = screen->is_compute_copy_faster(screen, src_format, dst_format, width, height, depth, cpu);

   trace_dump_ret(bool, ret);

   trace_dump_call_end();
   return ret;
}

static bool
trace_screen_is_dmabuf_modifier_supported(struct pipe_screen *_screen, uint64_t modifier, enum pipe_format format, bool *external_only)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;

   trace_dump_call_begin("pipe_screen", "is_dmabuf_modifier_supported");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(uint, modifier);
   trace_dump_arg(format, format);

   bool ret = screen->is_dmabuf_modifier_supported(screen, modifier, format, external_only);

   trace_dump_arg_begin("external_only");
   trace_dump_bool(external_only ? *external_only : false);
   trace_dump_arg_end();

   trace_dump_ret(bool, ret);

   trace_dump_call_end();
   return ret;
}

static unsigned int
trace_screen_get_dmabuf_modifier_planes(struct pipe_screen *_screen, uint64_t modifier, enum pipe_format format)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;

   trace_dump_call_begin("pipe_screen", "get_dmabuf_modifier_planes");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(uint, modifier);
   trace_dump_arg(format, format);

   unsigned ret = screen->get_dmabuf_modifier_planes(screen, modifier, format);

   trace_dump_ret(uint, ret);

   trace_dump_call_end();
   return ret;
}

static int
trace_screen_get_sparse_texture_virtual_page_size(struct pipe_screen *_screen,
                                                  enum pipe_texture_target target,
                                                  bool multi_sample,
                                                  enum pipe_format format,
                                                  unsigned offset, unsigned size,
                                                  int *x, int *y, int *z)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;

   trace_dump_call_begin("pipe_screen", "get_sparse_texture_virtual_page_size");

   trace_dump_arg(ptr, screen);
   trace_dump_arg_enum(pipe_texture_target, target);
   trace_dump_arg(format, format);
   trace_dump_arg(uint, offset);
   trace_dump_arg(uint, size);

   int ret = screen->get_sparse_texture_virtual_page_size(screen, target, multi_sample,
                                                          format, offset, size, x, y, z);

   if (x)
      trace_dump_arg(uint, *x);
   else
      trace_dump_arg(ptr, x);
   if (y)
      trace_dump_arg(uint, *y);
   else
      trace_dump_arg(ptr, y);
   if (z)
      trace_dump_arg(uint, *z);
   else
      trace_dump_arg(ptr, z);

   trace_dump_ret(int, ret);

   trace_dump_call_end();
   return ret;
}

static struct pipe_vertex_state *
trace_screen_create_vertex_state(struct pipe_screen *_screen,
                                 struct pipe_vertex_buffer *buffer,
                                 const struct pipe_vertex_element *elements,
                                 unsigned num_elements,
                                 struct pipe_resource *indexbuf,
                                 uint32_t full_velem_mask)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;

   trace_dump_call_begin("pipe_screen", "create_vertex_state");

   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, buffer->buffer.resource);
   trace_dump_arg(vertex_buffer, buffer);
   trace_dump_arg_begin("elements");
   trace_dump_struct_array(vertex_element, elements, num_elements);
   trace_dump_arg_end();
   trace_dump_arg(uint, num_elements);
   trace_dump_arg(ptr, indexbuf);
   trace_dump_arg(uint, full_velem_mask);

   struct pipe_vertex_state *vstate =
      screen->create_vertex_state(screen, buffer, elements, num_elements,
                                  indexbuf, full_velem_mask);
   trace_dump_ret(ptr, vstate);
   trace_dump_call_end();
   return vstate;
}

static void trace_screen_vertex_state_destroy(struct pipe_screen *_screen,
                                              struct pipe_vertex_state *state)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;

   trace_dump_call_begin("pipe_screen", "vertex_state_destroy");
   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, state);
   trace_dump_call_end();

   screen->vertex_state_destroy(screen, state);
}

static void trace_screen_set_fence_timeline_value(struct pipe_screen *_screen,
                                                  struct pipe_fence_handle *fence,
                                                  uint64_t value)
{
   struct trace_screen *tr_scr = trace_screen(_screen);
   struct pipe_screen *screen = tr_scr->screen;

   trace_dump_call_begin("pipe_screen", "set_fence_timeline_value");
   trace_dump_arg(ptr, screen);
   trace_dump_arg(ptr, fence);
   trace_dump_arg(uint, value);
   trace_dump_call_end();

   screen->set_fence_timeline_value(screen, fence, value);
}

bool
trace_enabled(void)
{
   static bool firstrun = true;

   if (!firstrun)
      return trace;
   firstrun = false;

   if(trace_dump_trace_begin()) {
      trace_dumping_start();
      trace = true;
   }

   return trace;
}

struct pipe_screen *
trace_screen_create(struct pipe_screen *screen)
{
   struct trace_screen *tr_scr;

   /* if zink+lavapipe is enabled, ensure that only one driver is traced */
   const char *driver = debug_get_option("MESA_LOADER_DRIVER_OVERRIDE", NULL);
   if (driver && !strcmp(driver, "zink")) {
      /* the user wants zink: check whether they want to trace zink or lavapipe */
      bool trace_lavapipe = debug_get_bool_option("ZINK_TRACE_LAVAPIPE", false);
      if (!strncmp(screen->get_name(screen), "zink", 4)) {
         /* this is the zink screen: only trace if lavapipe tracing is disabled */
         if (trace_lavapipe)
            return screen;
      } else {
         /* this is the llvmpipe screen: only trace if lavapipe tracing is enabled */
         if (!trace_lavapipe)
            return screen;
      }
   }

   if (!trace_enabled())
      goto error1;

   trace_dump_call_begin("", "pipe_screen_create");

   tr_scr = CALLOC_STRUCT(trace_screen);
   if (!tr_scr)
      goto error2;

#define SCR_INIT(_member) \
   tr_scr->base._member = screen->_member ? trace_screen_##_member : NULL

   tr_scr->base.destroy = trace_screen_destroy;
   tr_scr->base.get_name = trace_screen_get_name;
   tr_scr->base.get_vendor = trace_screen_get_vendor;
   tr_scr->base.get_device_vendor = trace_screen_get_device_vendor;
   SCR_INIT(get_compiler_options);
   SCR_INIT(get_disk_shader_cache);
   tr_scr->base.get_param = trace_screen_get_param;
   tr_scr->base.get_shader_param = trace_screen_get_shader_param;
   tr_scr->base.get_paramf = trace_screen_get_paramf;
   tr_scr->base.get_compute_param = trace_screen_get_compute_param;
   SCR_INIT(get_video_param);
   tr_scr->base.is_format_supported = trace_screen_is_format_supported;
   SCR_INIT(is_video_format_supported);
   assert(screen->context_create);
   tr_scr->base.context_create = trace_screen_context_create;
   tr_scr->base.resource_create = trace_screen_resource_create;
   SCR_INIT(resource_create_with_modifiers);
   tr_scr->base.resource_create_unbacked = trace_screen_resource_create_unbacked;
   SCR_INIT(resource_create_drawable);
   tr_scr->base.resource_bind_backing = trace_screen_resource_bind_backing;
   tr_scr->base.resource_from_handle = trace_screen_resource_from_handle;
   tr_scr->base.allocate_memory = trace_screen_allocate_memory;
   SCR_INIT(allocate_memory_fd);
   tr_scr->base.free_memory = trace_screen_free_memory;
   SCR_INIT(free_memory_fd);
   tr_scr->base.map_memory = trace_screen_map_memory;
   tr_scr->base.unmap_memory = trace_screen_unmap_memory;
   SCR_INIT(query_memory_info);
   SCR_INIT(query_dmabuf_modifiers);
   SCR_INIT(is_compute_copy_faster);
   SCR_INIT(is_dmabuf_modifier_supported);
   SCR_INIT(get_dmabuf_modifier_planes);
   SCR_INIT(check_resource_capability);
   tr_scr->base.resource_get_handle = trace_screen_resource_get_handle;
   SCR_INIT(resource_get_param);
   SCR_INIT(resource_get_info);
   SCR_INIT(resource_from_memobj);
   SCR_INIT(resource_changed);
   tr_scr->base.resource_destroy = trace_screen_resource_destroy;
   tr_scr->base.fence_reference = trace_screen_fence_reference;
   SCR_INIT(fence_get_fd);
   SCR_INIT(create_fence_win32);
   tr_scr->base.fence_finish = trace_screen_fence_finish;
   SCR_INIT(memobj_create_from_handle);
   SCR_INIT(memobj_destroy);
   tr_scr->base.flush_frontbuffer = trace_screen_flush_frontbuffer;
   tr_scr->base.get_timestamp = trace_screen_get_timestamp;
   SCR_INIT(get_driver_uuid);
   SCR_INIT(get_device_uuid);
   SCR_INIT(get_device_luid);
   SCR_INIT(get_device_node_mask);
   SCR_INIT(finalize_nir);
   SCR_INIT(create_vertex_state);
   SCR_INIT(vertex_state_destroy);
   tr_scr->base.transfer_helper = screen->transfer_helper;
   SCR_INIT(get_sparse_texture_virtual_page_size);
   SCR_INIT(set_fence_timeline_value);
   SCR_INIT(driver_thread_add_job);

   tr_scr->screen = screen;

   trace_dump_ret(ptr, screen);
   trace_dump_call_end();

   if (!trace_screens)
      trace_screens = _mesa_hash_table_create(NULL, _mesa_hash_pointer, _mesa_key_pointer_equal);
   _mesa_hash_table_insert(trace_screens, screen, tr_scr);

   tr_scr->trace_tc = debug_get_bool_option("GALLIUM_TRACE_TC", false);

   return &tr_scr->base;

error2:
   trace_dump_ret(ptr, screen);
   trace_dump_call_end();
error1:
   return screen;
}


struct trace_screen *
trace_screen(struct pipe_screen *screen)
{
   assert(screen);
   assert(screen->destroy == trace_screen_destroy);
   return (struct trace_screen *)screen;
}

struct pipe_screen *
trace_screen_unwrap(struct pipe_screen *_screen)
{
   if (_screen->destroy != trace_screen_destroy)
      return _screen;
   struct trace_screen *tr_scr = trace_screen(_screen);
   return tr_scr->screen;
}
