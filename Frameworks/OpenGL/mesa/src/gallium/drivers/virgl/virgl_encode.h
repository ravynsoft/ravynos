/*
 * Copyright 2014, 2015 Red Hat.
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
 */
#ifndef VIRGL_ENCODE_H
#define VIRGL_ENCODE_H

#include "pipe/p_defines.h"
#include "pipe/p_state.h"

#include "virgl_winsys.h"
#include "virtio-gpu/virgl_protocol.h"

struct tgsi_token;

struct virgl_context;
struct virgl_resource;
struct virgl_screen;
struct virgl_transfer;
struct virgl_sampler_view;
struct virgl_video_codec;
struct virgl_video_buffer;
struct virgl_vertex_elements_state;

struct virgl_surface {
   struct pipe_surface base;
   uint32_t handle;
};

struct virgl_indexbuf {
   unsigned offset;
   unsigned index_size;  /**< size of an index, in bytes */
   struct pipe_resource *buffer; /**< the actual buffer */
   const void *user_buffer;  /**< pointer to a user buffer if buffer == NULL */
};

static inline struct virgl_surface *virgl_surface(struct pipe_surface *surf)
{
   return (struct virgl_surface *)surf;
}

static inline void virgl_encoder_write_dword(struct virgl_cmd_buf *state,
                                            uint32_t dword)
{
   state->buf[state->cdw++] = dword;
}

static inline void virgl_encoder_write_qword(struct virgl_cmd_buf *state,
                                            uint64_t qword)
{
   memcpy(state->buf + state->cdw, &qword, sizeof(uint64_t));
   state->cdw += 2;
}

static inline void virgl_encoder_write_block(struct virgl_cmd_buf *state,
                                            const uint8_t *ptr, uint32_t len)
{
   int x;
   memcpy(state->buf + state->cdw, ptr, len);
   x = (len % 4);
   if (x) {
      uint8_t *mp = (uint8_t *)(state->buf + state->cdw);
      mp += len;
      memset(mp, 0, x);
   }
   state->cdw += (len + 3) / 4;
}

extern int virgl_encode_blend_state(struct virgl_context *ctx,
                                   uint32_t handle,
                                   const struct pipe_blend_state *blend_state);
extern int virgl_encode_rasterizer_state(struct virgl_context *ctx,
                                         uint32_t handle,
                                         const struct pipe_rasterizer_state *state);

extern int virgl_encode_shader_state(struct virgl_context *ctx,
                                     uint32_t handle,
                                     enum pipe_shader_type type,
                                     const struct pipe_stream_output_info *so_info,
                                     uint32_t cs_req_local_mem,
                                     const struct tgsi_token *tokens);

int virgl_encode_stream_output_info(struct virgl_context *ctx,
                                   uint32_t handle,
                                   uint32_t type,
                                   const struct pipe_shader_state *shader);

int virgl_encoder_set_so_targets(struct virgl_context *ctx,
                                unsigned num_targets,
                                struct pipe_stream_output_target **targets,
                                unsigned append_bitmask);

int virgl_encoder_create_so_target(struct virgl_context *ctx,
                                  uint32_t handle,
                                  struct virgl_resource *res,
                                  unsigned buffer_offset,
                                  unsigned buffer_size);

int virgl_encode_clear(struct virgl_context *ctx,
                      unsigned buffers,
                      const union pipe_color_union *color,
                      double depth, unsigned stencil);

int virgl_encode_clear_texture(struct virgl_context *ctx,
                               struct virgl_resource *res,
                               unsigned int level,
                               const struct pipe_box *box,
                               const void *data);

int virgl_encode_bind_object(struct virgl_context *ctx,
                            uint32_t handle, uint32_t object);
int virgl_encode_delete_object(struct virgl_context *ctx,
                              uint32_t handle, uint32_t object);

int virgl_encoder_set_framebuffer_state(struct virgl_context *ctx,
                                       const struct pipe_framebuffer_state *state);
int virgl_encoder_set_viewport_states(struct virgl_context *ctx,
                                      int start_slot,
                                      int num_viewports,
                                      const struct pipe_viewport_state *states);

int virgl_encoder_draw_vbo(struct virgl_context *ctx,
                           const struct pipe_draw_info *info,
                           unsigned drawid_offset,
                           const struct pipe_draw_indirect_info *indirect,
                           const struct pipe_draw_start_count_bias *draw);


int virgl_encoder_create_surface(struct virgl_context *ctx,
                                uint32_t handle,
                                struct virgl_resource *res,
                                const struct pipe_surface *templat);

int virgl_encoder_flush_frontbuffer(struct virgl_context *ctx,
                                   struct virgl_resource *res);

int virgl_encoder_create_vertex_elements(struct virgl_context *ctx,
                                        uint32_t handle,
                                        unsigned num_elements,
                                        const struct pipe_vertex_element *element);

int virgl_encoder_set_vertex_buffers(struct virgl_context *ctx,
                                    unsigned num_buffers,
                                    const struct pipe_vertex_buffer *buffers);

int virgl_encode_sampler_state(struct virgl_context *ctx,
                              uint32_t handle,
                              const struct pipe_sampler_state *state);
int virgl_encode_sampler_view(struct virgl_context *ctx,
                             uint32_t handle,
                             struct virgl_resource *res,
                             const struct pipe_sampler_view *state);

int virgl_encode_set_sampler_views(struct virgl_context *ctx,
                                  enum pipe_shader_type shader_type,
                                  uint32_t start_slot,
                                  uint32_t num_views,
                                  struct virgl_sampler_view **views);

int virgl_encode_bind_sampler_states(struct virgl_context *ctx,
                                    enum pipe_shader_type shader_type,
                                    uint32_t start_slot,
                                    uint32_t num_handles,
                                    uint32_t *handles);

int virgl_encoder_set_index_buffer(struct virgl_context *ctx,
                                  const struct virgl_indexbuf *ib);

uint32_t virgl_object_assign_handle(void);

int virgl_encoder_write_constant_buffer(struct virgl_context *ctx,
                                       enum pipe_shader_type shader,
                                       uint32_t index,
                                       uint32_t size,
                                       const void *data);

int virgl_encoder_set_uniform_buffer(struct virgl_context *ctx,
                                     enum pipe_shader_type shader,
                                     uint32_t index,
                                     uint32_t offset,
                                     uint32_t length,
                                     struct virgl_resource *res);
int virgl_encode_dsa_state(struct virgl_context *ctx,
                          uint32_t handle,
                          const struct pipe_depth_stencil_alpha_state *dsa_state);

int virgl_encoder_set_stencil_ref(struct virgl_context *ctx,
                                 const struct pipe_stencil_ref *ref);

int virgl_encoder_set_blend_color(struct virgl_context *ctx,
                                 const struct pipe_blend_color *color);

int virgl_encoder_set_scissor_state(struct virgl_context *ctx,
                                    unsigned start_slot,
                                    int num_scissors,
                                    const struct pipe_scissor_state *ss);

void virgl_encoder_set_polygon_stipple(struct virgl_context *ctx,
                                      const struct pipe_poly_stipple *ps);

void virgl_encoder_set_sample_mask(struct virgl_context *ctx,
                                  unsigned sample_mask);

void virgl_encoder_set_min_samples(struct virgl_context *ctx,
                                  unsigned min_samples);

void virgl_encoder_set_clip_state(struct virgl_context *ctx,
                                 const struct pipe_clip_state *clip);

int virgl_encode_resource_copy_region(struct virgl_context *ctx,
                                     struct virgl_resource *dst_res,
                                     unsigned dst_level,
                                     unsigned dstx, unsigned dsty, unsigned dstz,
                                     struct virgl_resource *src_res,
                                     unsigned src_level,
                                     const struct pipe_box *src_box);

int virgl_encode_blit(struct virgl_context *ctx,
                     struct virgl_resource *dst_res,
                     struct virgl_resource *src_res,
                     const struct pipe_blit_info *blit);

int virgl_encoder_create_query(struct virgl_context *ctx,
                              uint32_t handle,
                              uint query_type,
                              uint query_index,
                              struct virgl_resource *res,
                              uint32_t offset);

int virgl_encoder_begin_query(struct virgl_context *ctx,
                             uint32_t handle);
int virgl_encoder_end_query(struct virgl_context *ctx,
                           uint32_t handle);
int virgl_encoder_get_query_result(struct virgl_context *ctx,
                                  uint32_t handle, bool wait);

int virgl_encoder_render_condition(struct virgl_context *ctx,
                                  uint32_t handle, bool condition,
                                  enum pipe_render_cond_flag mode);

int virgl_encoder_set_sub_ctx(struct virgl_context *ctx, uint32_t sub_ctx_id);
int virgl_encoder_create_sub_ctx(struct virgl_context *ctx, uint32_t sub_ctx_id);
int virgl_encoder_destroy_sub_ctx(struct virgl_context *ctx, uint32_t sub_ctx_id);

int virgl_encode_link_shader(struct virgl_context *ctx, uint32_t *handles);

int virgl_encode_bind_shader(struct virgl_context *ctx,
                             uint32_t handle,
                             enum pipe_shader_type type);

int virgl_encode_set_tess_state(struct virgl_context *ctx,
                                const float outer[4],
                                const float inner[2]);

int virgl_encode_set_shader_buffers(struct virgl_context *ctx,
                                    enum pipe_shader_type shader,
                                    unsigned start_slot, unsigned count,
                                    const struct pipe_shader_buffer *buffers);
int virgl_encode_set_shader_images(struct virgl_context *ctx,
                                   enum pipe_shader_type shader,
                                   unsigned start_slot, unsigned count,
                                   const struct pipe_image_view *images);
int virgl_encode_set_hw_atomic_buffers(struct virgl_context *ctx,
                                       unsigned start_slot, unsigned count,
                                       const struct pipe_shader_buffer *buffers);
int virgl_encode_memory_barrier(struct virgl_context *ctx,
                                unsigned flags);
int virgl_encode_launch_grid(struct virgl_context *ctx,
                             const struct pipe_grid_info *grid_info);
int virgl_encode_texture_barrier(struct virgl_context *ctx,
                                 unsigned flags);

int virgl_encode_host_debug_flagstring(struct virgl_context *ctx,
                                  const char *envname);

int virgl_encode_get_query_result_qbo(struct virgl_context *ctx,
                                      uint32_t handle,
                                      struct virgl_resource *res, bool wait,
                                      uint32_t result_type,
                                      uint32_t offset,
                                      uint32_t index);

void virgl_encode_transfer(struct virgl_screen *vs, struct virgl_cmd_buf *buf,
                           struct virgl_transfer *trans, uint32_t direction);

void virgl_encode_copy_transfer(struct virgl_context *ctx,
                                struct virgl_transfer *trans);

void virgl_encode_end_transfers(struct virgl_cmd_buf *buf);

int virgl_encode_tweak(struct virgl_context *ctx, enum vrend_tweak_type tweak, uint32_t value);

void virgl_encode_get_memory_info(struct virgl_context *ctx, struct virgl_resource *res);

void virgl_encode_emit_string_marker(struct virgl_context *ctx, const char *message,
                                       int len);

void virgl_encode_create_video_codec(struct virgl_context *ctx,
                                     struct virgl_video_codec *cdc);

void virgl_encode_destroy_video_codec(struct virgl_context *ctx,
                                      struct virgl_video_codec *cdc);

void virgl_encode_create_video_buffer(struct virgl_context *ctx,
                                      struct virgl_video_buffer *buf);

void virgl_encode_destroy_video_buffer(struct virgl_context *ctx,
                                       struct virgl_video_buffer *buf);

void virgl_encode_begin_frame(struct virgl_context *ctx,
                              struct virgl_video_codec *cdc,
                              struct virgl_video_buffer *buf);

void virgl_encode_decode_bitstream(struct virgl_context *ctx,
                                   struct virgl_video_codec *cdc,
                                   struct virgl_video_buffer *buf,
                                   void *desc, uint32_t desc_size);

void virgl_encode_encode_bitstream(struct virgl_context *ctx,
                                   struct virgl_video_codec *cdc,
                                   struct virgl_video_buffer *buf,
                                   struct virgl_resource *tgt);

void virgl_encode_end_frame(struct virgl_context *ctx,
                            struct virgl_video_codec *cdc,
                            struct virgl_video_buffer *buf);

int virgl_encode_clear_surface(struct virgl_context *ctx,
                               struct pipe_surface *surf,
                               unsigned buffers,
                               const union pipe_color_union *color,
                               unsigned dstx, unsigned dsty,
                               unsigned width, unsigned height,
                               bool render_condition_enabled);

enum virgl_formats pipe_to_virgl_format(enum pipe_format format);
enum pipe_format virgl_to_pipe_format(enum virgl_formats format);
#endif
