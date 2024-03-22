
#ifndef __NV50_STATEOBJ_H__
#define __NV50_STATEOBJ_H__

#include "pipe/p_state.h"

#define NV50_SCISSORS_CLIPPING

#define SB_BEGIN_3D(so, m, s) \
   (so)->state[(so)->size++] = NV50_FIFO_PKHDR(NV50_3D(m), s)

#define SB_BEGIN_3D_(so, m, s) \
   (so)->state[(so)->size++] = NV50_FIFO_PKHDR(SUBC_3D(m), s)

#define SB_DATA(so, u) (so)->state[(so)->size++] = (u)

#include "nv50/nv50_stateobj_tex.h"

struct nv50_blend_stateobj {
   struct pipe_blend_state pipe;
   int size;
   uint32_t state[84]; // TODO: allocate less if !independent_blend_enable
};

struct nv50_rasterizer_stateobj {
   struct pipe_rasterizer_state pipe;
   int size;
   uint32_t state[49];
};

struct nv50_zsa_stateobj {
   struct pipe_depth_stencil_alpha_state pipe;
   int size;
   uint32_t state[38];
};

struct nv50_constbuf {
   union {
      struct pipe_resource *buf;
      const uint8_t *data;
   } u;
   uint32_t size; /* max 65536 */
   uint32_t offset;
   bool user; /* should only be true if u.data is valid and non-NULL */
};

struct nv50_vertex_element {
   struct pipe_vertex_element pipe;
   uint32_t state;
};

struct nv50_vertex_stateobj {
   uint32_t min_instance_div[PIPE_MAX_ATTRIBS];
   uint16_t vb_access_size[PIPE_MAX_ATTRIBS];
   uint16_t strides[PIPE_MAX_ATTRIBS];
   struct translate *translate;
   unsigned num_elements;
   uint32_t instance_elts;
   uint32_t instance_bufs;
   uint32_t vbo_constant;
   bool need_conversion;
   unsigned vertex_size;
   unsigned packet_vertex_limit;
   struct nv50_vertex_element element[0];
};

struct nv50_window_rect_stateobj {
   bool inclusive;
   unsigned rects;
   struct pipe_scissor_state rect[PIPE_MAX_WINDOW_RECTANGLES];
};

struct nv50_so_target {
   struct pipe_stream_output_target pipe;
   struct pipe_query *pq;
   unsigned stride;
   bool clean;
};

static inline struct nv50_so_target *
nv50_so_target(struct pipe_stream_output_target *ptarg)
{
   return (struct nv50_so_target *)ptarg;
}

#endif
