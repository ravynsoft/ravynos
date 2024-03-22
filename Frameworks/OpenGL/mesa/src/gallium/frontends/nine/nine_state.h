/*
 * Copyright 2011 Joakim Sindholt <opensource@zhasha.com>
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
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef _NINE_STATE_H_
#define _NINE_STATE_H_

#include "d3d9.h"
#include "iunknown.h"
#include "nine_defines.h"
#include "pipe/p_state.h"
#include "util/list.h"

#define NINED3DSAMP_MINLOD (D3DSAMP_DMAPOFFSET + 1)
#define NINED3DSAMP_SHADOW (D3DSAMP_DMAPOFFSET + 2)
#define NINED3DSAMP_CUBETEX (D3DSAMP_DMAPOFFSET + 3)

#define NINED3DRS_VSPOINTSIZE (D3DRS_BLENDOPALPHA + 1)
#define NINED3DRS_RTMASK      (D3DRS_BLENDOPALPHA + 2)
/* ALPHACOVERAGE:
 * bit 0: enable alpha coverage
 * bit 1: ATOC is on
 */
#define NINED3DRS_ALPHACOVERAGE  (D3DRS_BLENDOPALPHA + 3)
#define NINED3DRS_MULTISAMPLE  (D3DRS_BLENDOPALPHA + 4)
#define NINED3DRS_FETCH4  (D3DRS_BLENDOPALPHA + 5)
#define NINED3DRS_EMULATED_ALPHATEST  (D3DRS_BLENDOPALPHA + 6)
#define NINED3DRS_POSITIONT (D3DRS_BLENDOPALPHA + 7)

#define D3DRS_LAST       D3DRS_BLENDOPALPHA
#define D3DSAMP_LAST     D3DSAMP_DMAPOFFSET
#define NINED3DRS_LAST   NINED3DRS_POSITIONT /* 217 */
#define NINED3DSAMP_LAST NINED3DSAMP_CUBETEX /* 16 */
#define NINED3DTSS_LAST  D3DTSS_CONSTANT
#define NINED3DTS_LAST   D3DTS_WORLDMATRIX(255)

#define D3DRS_COUNT       (D3DRS_LAST + 1)
#define D3DSAMP_COUNT     (D3DSAMP_LAST + 1)
#define NINED3DRS_COUNT   (NINED3DRS_LAST + 1)
#define NINED3DSAMP_COUNT (NINED3DSAMP_LAST + 1)
#define NINED3DTSS_COUNT  (NINED3DTSS_LAST + 1)
#define NINED3DTS_COUNT   (NINED3DTS_LAST + 1)

#define NINE_STATE_FB          (1 <<  0)
#define NINE_STATE_VIEWPORT    (1 <<  1)
#define NINE_STATE_SCISSOR     (1 <<  2)
#define NINE_STATE_RASTERIZER  (1 <<  3)
#define NINE_STATE_BLEND       (1 <<  4)
#define NINE_STATE_DSA         (1 <<  5)
#define NINE_STATE_VS          (1 <<  6)
#define NINE_STATE_VS_CONST    (1 <<  7)
#define NINE_STATE_PS          (1 <<  8)
#define NINE_STATE_PS_CONST    (1 <<  9)
#define NINE_STATE_TEXTURE     (1 << 10)
#define NINE_STATE_SAMPLER     (1 << 11)
#define NINE_STATE_VDECL       (1 << 12)
#define NINE_STATE_IDXBUF      (1 << 13)
#define NINE_STATE_STREAMFREQ  (1 << 14)
#define NINE_STATE_BLEND_COLOR (1 << 17)
#define NINE_STATE_STENCIL_REF (1 << 18)
#define NINE_STATE_SAMPLE_MASK (1 << 19)
#define NINE_STATE_FF          (0x1f << 20)
#define NINE_STATE_FF_VS       (0x17 << 20)
#define NINE_STATE_FF_PS       (0x08 << 20)
#define NINE_STATE_FF_LIGHTING (1 << 20)
#define NINE_STATE_FF_MATERIAL (1 << 21)
#define NINE_STATE_FF_VSTRANSF (1 << 22)
#define NINE_STATE_FF_PS_CONSTS (1 << 23)
#define NINE_STATE_FF_VS_OTHER  (1 << 24)
#define NINE_STATE_VS_PARAMS_MISC  (1 << 25)
#define NINE_STATE_PS_PARAMS_MISC (1 << 26)
#define NINE_STATE_MULTISAMPLE (1 << 27)
#define NINE_STATE_SWVP        (1 << 28)
#define NINE_STATE_ALL          0x1fffffff
#define NINE_STATE_UNHANDLED   (1 << 29)

/* These states affect the ff shader key,
 * which we recompute everytime. */
#define NINE_STATE_FF_SHADER    0

#define NINE_STATE_COMMIT_DSA  (1 << 0)
#define NINE_STATE_COMMIT_RASTERIZER (1 << 1)
#define NINE_STATE_COMMIT_BLEND (1 << 2)
#define NINE_STATE_COMMIT_CONST_VS (1 << 3)
#define NINE_STATE_COMMIT_CONST_PS (1 << 4)
#define NINE_STATE_COMMIT_VS (1 << 5)
#define NINE_STATE_COMMIT_PS (1 << 6)


#define NINE_MAX_SIMULTANEOUS_RENDERTARGETS 4
#define NINE_MAX_CONST_F_PS3 224
#define NINE_MAX_CONST_F   256
#define NINE_MAX_CONST_I   16
#define NINE_MAX_CONST_B   16
#define NINE_MAX_CONST_F_SWVP   8192
#define NINE_MAX_CONST_I_SWVP   2048
#define NINE_MAX_CONST_B_SWVP   2048
#define NINE_MAX_CONST_VS_SPE_OFFSET (NINE_MAX_CONST_F + (NINE_MAX_CONST_I + NINE_MAX_CONST_B / 4)) /* B consts count only 1/4 th */
#define NINE_MAX_CONST_SWVP_SPE_OFFSET 3564 /* No app will read that far */
#define NINE_MAX_CONST_VS_SPE 9
#define NINE_MAX_CONST_ALL_VS (NINE_MAX_CONST_VS_SPE_OFFSET + NINE_MAX_CONST_VS_SPE)
#define NINE_MAX_CONST_PS_SPE_OFFSET (NINE_MAX_CONST_F_PS3 + (NINE_MAX_CONST_I + NINE_MAX_CONST_B / 4))
/* bumpmap_vars (12), fog (2), D3DRS_ALPHAREF (1) */
#define NINE_MAX_CONST_PS_SPE 15
#define NINE_MAX_CONST_ALL_PS (NINE_MAX_CONST_PS_SPE_OFFSET + NINE_MAX_CONST_PS_SPE)

#define NINE_CONST_I_BASE(nconstf) \
    ((nconstf)        * 4 * sizeof(float))
#define NINE_CONST_B_BASE(nconstf) \
    ((nconstf)        * 4 * sizeof(float) + \
     NINE_MAX_CONST_I * 4 * sizeof(int))

#define VS_CONST_F_SIZE(device) (device->may_swvp ? (NINE_MAX_CONST_F_SWVP * sizeof(float[4])) : (NINE_MAX_CONST_F * sizeof(float[4])))
#define VS_CONST_I_SIZE(device) (device->may_swvp ? (NINE_MAX_CONST_I_SWVP * sizeof(int[4])) : (NINE_MAX_CONST_I * sizeof(int[4])))
#define VS_CONST_B_SIZE(device) (device->may_swvp ? (NINE_MAX_CONST_B_SWVP * sizeof(BOOL)) : (NINE_MAX_CONST_B * sizeof(BOOL)))


#define NINE_MAX_TEXTURE_STAGES 8

#define NINE_MAX_LIGHTS        65536
#define NINE_MAX_LIGHTS_ACTIVE 8

#define NINED3DLIGHT_INVALID (D3DLIGHT_DIRECTIONAL + 1)

#define NINE_MAX_SAMPLERS_PS 16
#define NINE_MAX_SAMPLERS_VS  4
#define NINE_MAX_SAMPLERS    21 /* PS + DMAP + VS */
#define NINE_SAMPLER_PS(s)  ( 0 + (s))
#define NINE_SAMPLER_DMAP    16
#define NINE_SAMPLER_VS(s)  (17 + (s))
#define NINE_PS_SAMPLERS_MASK 0x00ffff
#define NINE_VS_SAMPLERS_MASK 0x1e0000

struct nine_ff_state {
    struct {
        uint32_t tex_stage[NINE_MAX_TEXTURE_STAGES][(NINED3DTSS_COUNT + 31) / 32]; /* stateblocks only */
        uint32_t transform[(NINED3DTS_COUNT + 31) / 32];
    } changed;

    D3DMATRIX *transform; /* access only via nine_state_access_transform */
    unsigned num_transforms;

    /* XXX: Do state blocks just change the set of active lights or do we
     * have to store which lights have been disabled, too ?
     */
    D3DLIGHT9 *light;
    uint16_t active_light[NINE_MAX_LIGHTS_ACTIVE]; /* 8 */
    unsigned num_lights;
    unsigned num_lights_active;

    D3DMATERIAL9 material;

    DWORD tex_stage[NINE_MAX_TEXTURE_STAGES][NINED3DTSS_COUNT];
};

struct nine_state
{
    struct {
        uint32_t group;
        uint32_t rs[(NINED3DRS_COUNT + 31) / 32];
        uint32_t vtxbuf;
        uint32_t stream_freq;
        uint32_t texture;
        uint16_t sampler[NINE_MAX_SAMPLERS];
        struct nine_range *vs_const_f;
        struct nine_range *ps_const_f;
        struct nine_range *vs_const_i;
        uint16_t ps_const_i; /* NINE_MAX_CONST_I == 16 */
        struct nine_range *vs_const_b;
        uint16_t ps_const_b; /* NINE_MAX_CONST_B == 16 */
        uint8_t ucp;
    } changed; /* stateblocks only */

    struct NineSurface9 *rt[NINE_MAX_SIMULTANEOUS_RENDERTARGETS];
    struct NineSurface9 *ds;

    D3DVIEWPORT9 viewport;

    struct pipe_scissor_state scissor;

    /* NOTE: vs, ps will be NULL for FF and are set in device->ff.vs,ps instead
     *  (XXX: or is it better to reference FF shaders here, too ?)
     * NOTE: const_f contains extra space for const_i,b to use as user constbuf
     */
    struct NineVertexShader9 *vs;
    float *vs_const_f;
    int   *vs_const_i;
    BOOL  *vs_const_b;
    float *vs_lconstf_temp; /* ProcessVertices */

    struct NinePixelShader9 *ps;
    float *ps_const_f;
    int    ps_const_i[NINE_MAX_CONST_I][4];
    BOOL   ps_const_b[NINE_MAX_CONST_B];

    struct NineVertexDeclaration9 *vdecl;

    struct NineIndexBuffer9   *idxbuf;
    struct NineVertexBuffer9  *stream[PIPE_MAX_ATTRIBS];
    uint32_t stream_mask; /* i bit set for *stream[i] not NULL */
    struct pipe_vertex_buffer  vtxbuf[PIPE_MAX_ATTRIBS]; /* vtxbuf.buffer unused */
    unsigned last_vtxbuf_count;
    uint16_t                   vtxstride[PIPE_MAX_ATTRIBS];
    UINT stream_freq[PIPE_MAX_ATTRIBS];

    struct pipe_clip_state clip;

    DWORD rs_advertised[NINED3DRS_COUNT]; /* the ones apps get with GetRenderState */

    struct NineBaseTexture9 *texture[NINE_MAX_SAMPLERS]; /* PS, DMAP, VS */

    DWORD samp_advertised[NINE_MAX_SAMPLERS][D3DSAMP_COUNT];

    struct nine_ff_state ff;
};

struct nine_context {
    struct {
        uint32_t group;
        uint16_t sampler[NINE_MAX_SAMPLERS];
        uint32_t vtxbuf;
        BOOL vs_const_f;
        BOOL vs_const_i;
        BOOL vs_const_b;
        BOOL ps_const_f;
        BOOL ps_const_i;
        BOOL ps_const_b;
        BOOL ucp;
    } changed;

    uint32_t bumpmap_vars[6 * NINE_MAX_TEXTURE_STAGES];

    struct NineSurface9 *rt[NINE_MAX_SIMULTANEOUS_RENDERTARGETS];
    struct NineSurface9 *ds;

    struct {
        void *vs;
        unsigned *vs_const_ranges;
        unsigned vs_const_used_size;
        void *ps;
        unsigned *ps_const_ranges;
        unsigned ps_const_used_size;
    } cso_shader;

    struct pipe_context *pipe;
    struct cso_context *cso;

    uint8_t rt_mask;

    D3DVIEWPORT9 viewport;

    struct pipe_scissor_state scissor;

    struct NineVertexShader9 *vs;
    BOOL programmable_vs;
    float *vs_const_f;
    float *vs_const_f_swvp;
    int   *vs_const_i;
    BOOL  *vs_const_b;
    float *vs_lconstf_temp;

    struct NinePixelShader9 *ps;
    float *ps_const_f;
    int    ps_const_i[NINE_MAX_CONST_I][4];
    BOOL   ps_const_b[NINE_MAX_CONST_B];
    BOOL   zfog;

    struct NineVertexDeclaration9 *vdecl;

    struct pipe_vertex_buffer vtxbuf[PIPE_MAX_ATTRIBS];
    uint32_t vtxbuf_mask; /* i bit set for context->vtxbuf[i].buffer.resource not NULL */
    uint32_t last_vtxbuf_count;
    uint16_t vtxstride[PIPE_MAX_ATTRIBS];
    UINT stream_freq[PIPE_MAX_ATTRIBS];
    uint32_t stream_instancedata_mask; /* derived from stream_freq */
    uint32_t stream_usage_mask; /* derived from VS and vdecl */

    struct pipe_resource *idxbuf;
    unsigned index_offset;
    unsigned index_size;

    struct pipe_clip_state clip;

    DWORD rs[NINED3DRS_COUNT];

    struct {
        BOOL enabled;
        BOOL shadow;
        DWORD lod;
        D3DRESOURCETYPE type;
        struct pipe_resource *resource;
        struct pipe_sampler_view *view[2];
        uint8_t pstype;
    } texture[NINE_MAX_SAMPLERS];

    DWORD samp[NINE_MAX_SAMPLERS][NINED3DSAMP_COUNT];

    uint32_t samplers_shadow;
    uint32_t samplers_fetch4;

    uint8_t bound_samplers_mask_vs;
    uint8_t enabled_samplers_mask_vs;
    uint8_t enabled_sampler_count_vs;
    uint8_t enabled_sampler_count_ps;
    uint16_t bound_samplers_mask_ps;
    uint16_t enabled_samplers_mask_ps;

    int dummy_vbo_bound_at; /* -1 = not bound , >= 0 = bound index */

    bool inline_constants;

    struct nine_ff_state ff;

    /* software vertex processing */
    bool swvp;

    uint32_t commit;
    struct {
        struct pipe_framebuffer_state fb;
        struct pipe_depth_stencil_alpha_state dsa;
        struct pipe_rasterizer_state rast;
        struct pipe_blend_state blend;
        struct pipe_constant_buffer cb_vs;
        struct pipe_constant_buffer cb0_swvp;
        struct pipe_constant_buffer cb1_swvp;
        struct pipe_constant_buffer cb2_swvp;
        struct pipe_constant_buffer cb3_swvp;
        struct pipe_constant_buffer cb_ps;
        struct pipe_constant_buffer cb_vs_ff;
        struct pipe_constant_buffer cb_ps_ff;
    } pipe_data;
};

struct nine_state_sw_internal {
    struct pipe_transfer *transfers_so[4];
};

struct nine_clipplane {
    float plane[4];
};
/* map D3DRS -> NINE_STATE_x
 */
extern const uint32_t nine_render_state_group[NINED3DRS_COUNT];

/* for D3DSBT_PIXEL/VERTEX:
 */
extern const uint32_t nine_render_states_pixel[(NINED3DRS_COUNT + 31) / 32];
extern const uint32_t nine_render_states_vertex[(NINED3DRS_COUNT + 31) / 32];

struct NineDevice9;

/* Internal multithreading: When enabled, the nine_context functions
 * will append work to a worker thread when possible. Only the worker
 * thread can access struct nine_context. */

void
nine_context_set_stream_source_apply(struct NineDevice9 *device,
                                    UINT StreamNumber,
                                    struct pipe_resource *res,
                                    UINT OffsetInBytes,
                                    UINT Stride);

void
nine_context_set_indices_apply(struct NineDevice9 *device,
                               struct pipe_resource *res,
                               UINT IndexSize,
                               UINT OffsetInBytes);

void
nine_context_set_render_state(struct NineDevice9 *device,
                              D3DRENDERSTATETYPE State,
                              DWORD Value);

void
nine_context_set_texture(struct NineDevice9 *device,
                         DWORD Stage,
                         struct NineBaseTexture9 *tex);

void
nine_context_set_sampler_state(struct NineDevice9 *device,
                               DWORD Sampler,
                               D3DSAMPLERSTATETYPE Type,
                               DWORD Value);

void
nine_context_set_stream_source(struct NineDevice9 *device,
                               UINT StreamNumber,
                               struct NineVertexBuffer9 *pVBuf9,
                               UINT OffsetInBytes,
                               UINT Stride);

void
nine_context_set_stream_source_freq(struct NineDevice9 *device,
                                    UINT StreamNumber,
                                    UINT Setting);

void
nine_context_set_indices(struct NineDevice9 *device,
                         struct NineIndexBuffer9 *idxbuf);

void
nine_context_set_vertex_declaration(struct NineDevice9 *device,
                                    struct NineVertexDeclaration9 *vdecl);

void
nine_context_set_vertex_shader(struct NineDevice9 *device,
                               struct NineVertexShader9 *pShader);

void
nine_context_set_vertex_shader_constant_f(struct NineDevice9 *device,
                                          UINT StartRegister,
                                          const float *pConstantData,
                                          const unsigned pConstantData_size,
                                          UINT Vector4fCount);

void
nine_context_set_vertex_shader_constant_i(struct NineDevice9 *device,
                                          UINT StartRegister,
                                          const int *pConstantData,
                                          const unsigned pConstantData_size,
                                          UINT Vector4iCount);

void
nine_context_set_vertex_shader_constant_b(struct NineDevice9 *device,
                                          UINT StartRegister,
                                          const BOOL *pConstantData,
                                          const unsigned pConstantData_size,
                                          UINT BoolCount);

void
nine_context_set_pixel_shader(struct NineDevice9 *device,
                              struct NinePixelShader9* ps);

void
nine_context_set_pixel_shader_constant_f(struct NineDevice9 *device,
                                        UINT StartRegister,
                                        const float *pConstantData,
                                        const unsigned pConstantData_size,
                                        UINT Vector4fCount);

void
nine_context_set_pixel_shader_constant_i(struct NineDevice9 *device,
                                         UINT StartRegister,
                                         const int *pConstantData,
                                         const unsigned pConstantData_size,
                                         UINT Vector4iCount);

void
nine_context_set_pixel_shader_constant_b(struct NineDevice9 *device,
                                         UINT StartRegister,
                                         const BOOL *pConstantData,
                                         const unsigned pConstantData_size,
                                         UINT BoolCount);

void
nine_context_set_viewport(struct NineDevice9 *device,
                          const D3DVIEWPORT9 *viewport);

void
nine_context_set_scissor(struct NineDevice9 *device,
                         const struct pipe_scissor_state *scissor);

void
nine_context_set_transform(struct NineDevice9 *device,
                           D3DTRANSFORMSTATETYPE State,
                           const D3DMATRIX *pMatrix);

void
nine_context_set_material(struct NineDevice9 *device,
                          const D3DMATERIAL9 *pMaterial);

void
nine_context_set_light(struct NineDevice9 *device,
                       DWORD Index,
                       const D3DLIGHT9 *pLight);

void
nine_context_light_enable(struct NineDevice9 *device,
                          DWORD Index,
                          BOOL Enable);

void
nine_context_set_texture_stage_state(struct NineDevice9 *device,
                                     DWORD Stage,
                                     D3DTEXTURESTAGESTATETYPE Type,
                                     DWORD Value);

void
nine_context_set_render_target(struct NineDevice9 *device,
                               DWORD RenderTargetIndex,
                               struct NineSurface9 *rt);

void
nine_context_set_depth_stencil(struct NineDevice9 *device,
                               struct NineSurface9 *ds);

void
nine_context_set_clip_plane(struct NineDevice9 *device,
                            DWORD Index,
                            const struct nine_clipplane *pPlane);

void
nine_context_set_swvp(struct NineDevice9 *device,
                      bool swvp);

void
nine_context_apply_stateblock(struct NineDevice9 *device,
                              const struct nine_state *src);

void
nine_context_clear_fb(struct NineDevice9 *device, DWORD Count,
                      const D3DRECT *pRects, DWORD Flags,
                      D3DCOLOR Color, float Z, DWORD Stencil);

void
nine_context_draw_primitive(struct NineDevice9 *device,
                            D3DPRIMITIVETYPE PrimitiveType,
                            UINT StartVertex,
                            UINT PrimitiveCount);

void
nine_context_draw_indexed_primitive(struct NineDevice9 *device,
                                    D3DPRIMITIVETYPE PrimitiveType,
                                    INT BaseVertexIndex,
                                    UINT MinVertexIndex,
                                    UINT NumVertices,
                                    UINT StartIndex,
                                    UINT PrimitiveCount);

void
nine_context_draw_indexed_primitive_from_vtxbuf_idxbuf(struct NineDevice9 *device,
                                                       D3DPRIMITIVETYPE PrimitiveType,
                                                       UINT MinVertexIndex,
                                                       UINT NumVertices,
                                                       UINT PrimitiveCount,
                                                       unsigned vbuf_stride,
                                                       struct pipe_vertex_buffer *vbuf,
                                                       struct pipe_resource *ibuf,
                                                       void *user_ibuf,
                                                       unsigned index_offset,
                                                       unsigned index_size);

void
nine_context_resource_copy_region(struct NineDevice9 *device,
                                  struct NineUnknown *dst,
                                  struct NineUnknown *src,
                                  struct pipe_resource* dst_res,
                                  unsigned dst_level,
                                  const struct pipe_box *dst_box,
                                  struct pipe_resource* src_res,
                                  unsigned src_level,
                                  const struct pipe_box *src_box);

void
nine_context_blit(struct NineDevice9 *device,
                  struct NineUnknown *dst,
                  struct NineUnknown *src,
                  struct pipe_blit_info *blit);

void
nine_context_clear_render_target(struct NineDevice9 *device,
                                 struct NineSurface9 *surface,
                                 D3DCOLOR color,
                                 UINT x,
                                 UINT y,
                                 UINT width,
                                 UINT height);

void
nine_context_gen_mipmap(struct NineDevice9 *device,
                        struct NineUnknown *dst,
                        struct pipe_resource *res,
                        UINT base_level, UINT last_level,
                        UINT first_layer, UINT last_layer,
                        UINT filter);

void
nine_context_range_upload(struct NineDevice9 *device,
                          unsigned *counter,
                          struct NineUnknown *src_ref,
                          struct pipe_resource *res,
                          unsigned offset,
                          unsigned size,
                          unsigned usage,
                          const void *data);

void
nine_context_box_upload(struct NineDevice9 *device,
                        unsigned *counter,
                        struct NineUnknown *src_ref,
                        struct pipe_resource *res,
                        unsigned level,
                        const struct pipe_box *dst_box,
                        enum pipe_format src_format,
                        const void *src, unsigned src_stride,
                        unsigned src_layer_stride,
                        const struct pipe_box *src_box);

struct pipe_query *
nine_context_create_query(struct NineDevice9 *device, unsigned query_type);

void
nine_context_destroy_query(struct NineDevice9 *device, struct pipe_query *query);

void
nine_context_begin_query(struct NineDevice9 *device, unsigned *counter, struct pipe_query *query);

void
nine_context_end_query(struct NineDevice9 *device, unsigned *counter, struct pipe_query *query);

bool
nine_context_get_query_result(struct NineDevice9 *device, struct pipe_query *query,
                              unsigned *counter, bool flush, bool wait,
                              union pipe_query_result *result);

void
nine_context_pipe_flush(struct NineDevice9 *device);

void nine_state_restore_non_cso(struct NineDevice9 *device);
void nine_state_set_defaults(struct NineDevice9 *, const D3DCAPS9 *,
                             bool is_reset);
void nine_device_state_clear(struct NineDevice9 *);
void nine_context_clear(struct NineDevice9 *);
void nine_context_update_state(struct NineDevice9 *);

void nine_state_init_sw(struct NineDevice9 *device);
void nine_state_prepare_draw_sw(struct NineDevice9 *device,
                                struct NineVertexDeclaration9 *vdecl_out,
                                int start_vertice,
                                int num_vertices,
                                struct pipe_stream_output_info *so);
void nine_state_after_draw_sw(struct NineDevice9 *device);
void nine_state_destroy_sw(struct NineDevice9 *device);

void
nine_state_resize_transform(struct nine_ff_state *ff_state, unsigned N);

/* If @alloc is FALSE, the return value may be a const identity matrix.
 * Therefore, do not modify if you set alloc to FALSE !
 */
D3DMATRIX *
nine_state_access_transform(struct nine_ff_state *, D3DTRANSFORMSTATETYPE,
                            bool alloc);

HRESULT
nine_state_set_light(struct nine_ff_state *, DWORD, const D3DLIGHT9 *);

HRESULT
nine_state_light_enable(struct nine_ff_state *,
                        DWORD, BOOL);

const char *nine_d3drs_to_string(DWORD State);

/* CSMT functions */
struct csmt_context;

struct csmt_context *
nine_csmt_create( struct NineDevice9 *This );

void
nine_csmt_destroy( struct NineDevice9 *This, struct csmt_context *ctx );

/* Flushes and waits everything is executed */
void
nine_csmt_process( struct NineDevice9 *This );

/* Flushes and doesn't wait */
void
nine_csmt_flush( struct NineDevice9 *This );

/* Get the pipe_context (should not be called from the worker thread).
 * All the work in the worker thread is finished before returning. */
struct pipe_context *
nine_context_get_pipe( struct NineDevice9 *device );

/* Can be called from all threads */
struct pipe_context *
nine_context_get_pipe_multithread( struct NineDevice9 *device );


/* Get the pipe_context (should not be called from the worker thread).
 * All the work in the worker thread is paused before returning.
 * It is neccessary to release in order to restart the thread.
 * This is intended for use of the nine_context pipe_context that don't
 * need the worker thread to finish all queued job. */
struct pipe_context *
nine_context_get_pipe_acquire( struct NineDevice9 *device );

void
nine_context_get_pipe_release( struct NineDevice9 *device );

bool
nine_context_is_worker( struct NineDevice9 *device );

#endif /* _NINE_STATE_H_ */
