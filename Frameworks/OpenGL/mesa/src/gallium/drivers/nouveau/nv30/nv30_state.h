#ifndef __NV30_STATE_H__
#define __NV30_STATE_H__

#include "pipe/p_state.h"
#include "tgsi/tgsi_scan.h"
#include "util/u_dynarray.h"

#define NV30_QUERY_ZCULL_0 (PIPE_QUERY_TYPES + 0)
#define NV30_QUERY_ZCULL_1 (PIPE_QUERY_TYPES + 1)
#define NV30_QUERY_ZCULL_2 (PIPE_QUERY_TYPES + 2)
#define NV30_QUERY_ZCULL_3 (PIPE_QUERY_TYPES + 3)

#define SB_DATA(so, u)        (so)->data[(so)->size++] = (u)
#define SB_MTHD30(so, mthd, size)                                          \
   SB_DATA((so), ((size) << 18) | (7 << 13) | NV30_3D_##mthd)
#define SB_MTHD35(so, mthd, size)                                          \
   SB_DATA((so), ((size) << 18) | (7 << 13) | NV35_3D_##mthd)
#define SB_MTHD40(so, mthd, size)                                          \
   SB_DATA((so), ((size) << 18) | (7 << 13) | NV40_3D_##mthd)

struct nv30_blend_stateobj {
   struct pipe_blend_state pipe;
   unsigned data[16];
   unsigned size;
};

struct nv30_rasterizer_stateobj {
   struct pipe_rasterizer_state pipe;
   unsigned data[32];
   unsigned size;
};

struct nv30_zsa_stateobj {
   struct pipe_depth_stencil_alpha_state pipe;
   unsigned data[36];
   unsigned size;
};

struct nv30_sampler_state {
   struct pipe_sampler_state pipe;
   unsigned fmt;
   unsigned wrap;
   unsigned en;
   unsigned filt;
   unsigned bcol;
   /* 4.8 */
   unsigned min_lod;
   unsigned max_lod;
};

struct nv30_sampler_view {
   struct pipe_sampler_view pipe;
   unsigned fmt;
   unsigned swz;
   unsigned filt;
   unsigned filt_mask;
   unsigned wrap;
   unsigned wrap_mask;
   unsigned npot_size0;
   unsigned npot_size1;
   /* 4.8 */
   unsigned base_lod;
   unsigned high_lod;
};

struct nv30_shader_reloc {
   unsigned location;
   int target;
};

struct nv30_vertprog_exec {
   uint32_t data[4];
};

struct nv30_vertprog_data {
   int index; /* immediates == -1 */
   float value[4];
};

struct nv30_vertprog {
   struct pipe_shader_state pipe;
   struct tgsi_shader_info info;

   struct draw_vertex_shader *draw;
   bool translated;
   unsigned enabled_ucps;
   uint16_t texcoord[10];

   struct util_dynarray branch_relocs;
   struct nv30_vertprog_exec *insns;
   unsigned nr_insns;

   struct util_dynarray const_relocs;
   struct nv30_vertprog_data *consts;
   unsigned nr_consts;

   struct nouveau_heap *exec;
   struct nouveau_heap *data;
   uint32_t ir;
   uint32_t or;
   void *nvfx;
};

struct nv30_fragprog_data {
   unsigned offset;
   unsigned index;
};

struct nv30_fragprog {
   struct pipe_shader_state pipe;
   struct tgsi_shader_info info;

   struct draw_fragment_shader *draw;
   bool translated;

   uint32_t *insn;
   unsigned insn_len;

   uint16_t texcoord[10];
   struct nv30_fragprog_data *consts;
   unsigned nr_consts;

   struct pipe_resource *buffer;
   uint32_t vp_or; /* appended to VP_RESULT_EN */
   uint32_t fp_control;
   uint32_t point_sprite_control;
   uint32_t coord_conventions;
   uint32_t texcoords;
   uint32_t rt_enable;
};

struct nv30_vertex_element {
   unsigned state;
};

struct nv30_vertex_stateobj {
   struct pipe_vertex_element pipe[PIPE_MAX_ATTRIBS];
   struct translate *translate;
   bool need_conversion;
   uint16_t strides[PIPE_MAX_ATTRIBS];
   unsigned num_elements;
   unsigned vtx_size;
   unsigned vtx_per_packet_max;
   struct nv30_vertex_element element[];
};

#endif
