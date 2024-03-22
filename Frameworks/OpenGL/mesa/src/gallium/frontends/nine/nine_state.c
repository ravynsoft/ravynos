/*
 * Copyright 2011 Joakim Sindholt <opensource@zhasha.com>
 * Copyright 2013 Christoph Bumiller
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

#define NINE_STATE

#include "device9.h"
#include "swapchain9.h"
#include "basetexture9.h"
#include "buffer9.h"
#include "indexbuffer9.h"
#include "surface9.h"
#include "vertexbuffer9.h"
#include "vertexdeclaration9.h"
#include "vertexshader9.h"
#include "pixelshader9.h"
#include "nine_pipe.h"
#include "nine_ff.h"
#include "nine_limits.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "cso_cache/cso_context.h"
#include "util/u_atomic.h"
#include "util/u_upload_mgr.h"
#include "util/u_math.h"
#include "util/u_box.h"
#include "util/u_simple_shaders.h"
#include "util/u_gen_mipmap.h"

/* CSMT headers */
#include "nine_queue.h"
#include "nine_csmt_helper.h"
#include "util/u_thread.h"

#define DBG_CHANNEL DBG_DEVICE

/* Nine CSMT */

struct csmt_instruction {
    int (* func)(struct NineDevice9 *This, struct csmt_instruction *instr);
};

struct csmt_context {
    thrd_t worker;
    struct nine_queue_pool* pool;
    BOOL terminate;
    cnd_t event_processed;
    mtx_t mutex_processed;
    struct NineDevice9 *device;
    BOOL processed;
    BOOL toPause;
    BOOL hasPaused;
    mtx_t thread_running;
    mtx_t thread_resume;
};

/* Wait for instruction to be processed.
 * Caller has to ensure that only one thread waits at time.
 */
static void
nine_csmt_wait_processed(struct csmt_context *ctx)
{
    mtx_lock(&ctx->mutex_processed);
    while (!p_atomic_read(&ctx->processed)) {
        cnd_wait(&ctx->event_processed, &ctx->mutex_processed);
    }
    mtx_unlock(&ctx->mutex_processed);
}

/* CSMT worker thread */
static
int
nine_csmt_worker(void *arg)
{
    struct csmt_context *ctx = arg;
    struct csmt_instruction *instr;
    DBG("CSMT worker spawned\n");

    u_thread_setname("CSMT-Worker");

    while (1) {
        nine_queue_wait_flush(ctx->pool);
        mtx_lock(&ctx->thread_running);

        /* Get instruction. NULL on empty cmdbuf. */
        while (!p_atomic_read(&ctx->terminate) &&
               (instr = (struct csmt_instruction *)nine_queue_get(ctx->pool))) {

            /* decode */
            if (instr->func(ctx->device, instr)) {
                mtx_lock(&ctx->mutex_processed);
                p_atomic_set(&ctx->processed, true);
                cnd_signal(&ctx->event_processed);
                mtx_unlock(&ctx->mutex_processed);
            }
            if (p_atomic_read(&ctx->toPause)) {
                mtx_unlock(&ctx->thread_running);
                /* will wait here the thread can be resumed */
                mtx_lock(&ctx->thread_resume);
                mtx_lock(&ctx->thread_running);
                mtx_unlock(&ctx->thread_resume);
            }
        }

        mtx_unlock(&ctx->thread_running);
        if (p_atomic_read(&ctx->terminate)) {
            mtx_lock(&ctx->mutex_processed);
            p_atomic_set(&ctx->processed, true);
            cnd_signal(&ctx->event_processed);
            mtx_unlock(&ctx->mutex_processed);
            break;
        }
    }

    DBG("CSMT worker destroyed\n");
    return 0;
}

/* Create a CSMT context.
 * Spawns a worker thread.
 */
struct csmt_context *
nine_csmt_create( struct NineDevice9 *This )
{
    struct csmt_context *ctx;

    ctx = CALLOC_STRUCT(csmt_context);
    if (!ctx)
        return NULL;

    ctx->pool = nine_queue_create();
    if (!ctx->pool) {
        FREE(ctx);
        return NULL;
    }
    cnd_init(&ctx->event_processed);
    (void) mtx_init(&ctx->mutex_processed, mtx_plain);
    (void) mtx_init(&ctx->thread_running, mtx_plain);
    (void) mtx_init(&ctx->thread_resume, mtx_plain);

#if defined(DEBUG) || !defined(NDEBUG)
    u_thread_setname("Main thread");
#endif

    ctx->device = This;

    if (thrd_success != u_thread_create(&ctx->worker, nine_csmt_worker, ctx)) {
        nine_queue_delete(ctx->pool);
        FREE(ctx);
        return NULL;
    }

    DBG("Returning context %p\n", ctx);

    return ctx;
}

static int
nop_func( struct NineDevice9 *This, struct csmt_instruction *instr )
{
    (void) This;
    (void) instr;

    return 1;
}

/* Push nop instruction and flush the queue.
 * Waits for the worker to complete. */
void
nine_csmt_process( struct NineDevice9 *device )
{
    struct csmt_instruction* instr;
    struct csmt_context *ctx = device->csmt_ctx;

    if (!device->csmt_active)
        return;

    if (nine_queue_isempty(ctx->pool))
        return;

    DBG("device=%p\n", device);

    /* NOP */
    instr = nine_queue_alloc(ctx->pool, sizeof(struct csmt_instruction));
    assert(instr);
    instr->func = nop_func;

    p_atomic_set(&ctx->processed, false);
    nine_queue_flush(ctx->pool);

    nine_csmt_wait_processed(ctx);
}

void
nine_csmt_flush( struct NineDevice9* device )
{
    if (!device->csmt_active)
        return;

    nine_queue_flush(device->csmt_ctx->pool);
}


/* Destroys a CSMT context.
 * Waits for the worker thread to terminate.
 */
void
nine_csmt_destroy( struct NineDevice9 *device, struct csmt_context *ctx )
{
    struct csmt_instruction* instr;
    thrd_t render_thread = ctx->worker;

    DBG("device=%p ctx=%p\n", device, ctx);

    /* Push nop and flush the queue. */
    instr = nine_queue_alloc(ctx->pool, sizeof(struct csmt_instruction));
    assert(instr);
    instr->func = nop_func;

    p_atomic_set(&ctx->processed, false);
    /* Signal worker to terminate. */
    p_atomic_set(&ctx->terminate, true);
    nine_queue_flush(ctx->pool);

    nine_csmt_wait_processed(ctx);
    nine_queue_delete(ctx->pool);

    mtx_destroy(&ctx->thread_resume);
    mtx_destroy(&ctx->thread_running);

    mtx_destroy(&ctx->mutex_processed);
    cnd_destroy(&ctx->event_processed);

    FREE(ctx);

    thrd_join(render_thread, NULL);
}

static void
nine_csmt_pause( struct NineDevice9 *device )
{
    struct csmt_context *ctx = device->csmt_ctx;

    if (!device->csmt_active)
        return;

    /* No need to pause the thread */
    if (nine_queue_no_flushed_work(ctx->pool))
        return;

    mtx_lock(&ctx->thread_resume);
    p_atomic_set(&ctx->toPause, true);

    /* Wait the thread is paused */
    mtx_lock(&ctx->thread_running);
    ctx->hasPaused = true;
    p_atomic_set(&ctx->toPause, false);
}

static void
nine_csmt_resume( struct NineDevice9 *device )
{
    struct csmt_context *ctx = device->csmt_ctx;

    if (!device->csmt_active)
        return;

    if (!ctx->hasPaused)
        return;

    ctx->hasPaused = false;
    mtx_unlock(&ctx->thread_running);
    mtx_unlock(&ctx->thread_resume);
}

struct pipe_context *
nine_context_get_pipe( struct NineDevice9 *device )
{
    nine_csmt_process(device);
    return device->context.pipe;
}

struct pipe_context *
nine_context_get_pipe_multithread( struct NineDevice9 *device )
{
    struct csmt_context *ctx = device->csmt_ctx;

    if (!device->csmt_active)
        return device->context.pipe;

    if (!u_thread_is_self(ctx->worker))
        nine_csmt_process(device);

    return device->context.pipe;
}

struct pipe_context *
nine_context_get_pipe_acquire( struct NineDevice9 *device )
{
    nine_csmt_pause(device);
    return device->context.pipe;
}

void
nine_context_get_pipe_release( struct NineDevice9 *device )
{
    nine_csmt_resume(device);
}

bool
nine_context_is_worker( struct NineDevice9 *device )
{
    struct csmt_context *ctx = device->csmt_ctx;

    if (!device->csmt_active)
        return false;

    return u_thread_is_self(ctx->worker);
}

/* Nine state functions */

/* Check if some states need to be set dirty */

static inline DWORD
check_multisample(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;
    DWORD *rs = context->rs;
    struct NineSurface9 *rt0 = context->rt[0];
    bool multisampled_target;
    DWORD new_value;

    multisampled_target = rt0 && rt0->desc.MultiSampleType >= 1;
    if (rt0 && rt0->desc.Format == D3DFMT_NULL && context->ds)
        multisampled_target = context->ds->desc.MultiSampleType >= 1;
    new_value = (multisampled_target && rs[D3DRS_MULTISAMPLEANTIALIAS]) ? 1 : 0;
    if (rs[NINED3DRS_MULTISAMPLE] != new_value) {
        rs[NINED3DRS_MULTISAMPLE] = new_value;
        return NINE_STATE_RASTERIZER;
    }
    return 0;
}

/* State preparation only */

static inline void
prepare_blend(struct NineDevice9 *device)
{
    nine_convert_blend_state(&device->context.pipe_data.blend, device->context.rs);
    device->context.commit |= NINE_STATE_COMMIT_BLEND;
}

static inline void
prepare_dsa(struct NineDevice9 *device)
{
    nine_convert_dsa_state(&device->context.pipe_data.dsa, device->context.rs);
    device->context.commit |= NINE_STATE_COMMIT_DSA;
}

static inline void
prepare_rasterizer(struct NineDevice9 *device)
{
    nine_convert_rasterizer_state(device, &device->context.pipe_data.rast, device->context.rs);
    device->context.commit |= NINE_STATE_COMMIT_RASTERIZER;
}

static void
prepare_vs_constants_userbuf_swvp(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;

    if (device->driver_caps.emulate_ucp) {
        /* TODO: Avoid memcpy all time by storing directly into the array */
        memcpy(&context->vs_const_f[4 * NINE_MAX_CONST_SWVP_SPE_OFFSET], &context->clip.ucp, sizeof(context->clip));
        context->changed.vs_const_f = 1; /* TODO optimize */
    }

    if (device->driver_caps.always_output_pointsize) {
        context->vs_const_f[4 * (NINE_MAX_CONST_SWVP_SPE_OFFSET + 8)] =
            CLAMP(asfloat(context->rs[D3DRS_POINTSIZE]),
                asfloat(context->rs[D3DRS_POINTSIZE_MIN]),
                asfloat(context->rs[D3DRS_POINTSIZE_MAX]));
        context->changed.vs_const_f = 1; /* TODO optimize */
    }

    if (context->changed.vs_const_f || context->changed.group & NINE_STATE_SWVP) {
        struct pipe_constant_buffer cb;

        cb.buffer_offset = 0;
        cb.buffer_size = 4096 * sizeof(float[4]);
        cb.user_buffer = context->vs_const_f_swvp;

        if (context->vs->lconstf.ranges) {
            const struct nine_lconstf *lconstf = &(context->vs->lconstf);
            const struct nine_range *r = lconstf->ranges;
            unsigned n = 0;
            float *dst = context->vs_lconstf_temp;
            float *src = (float *)cb.user_buffer;
            memcpy(dst, src, cb.buffer_size);
            while (r) {
                unsigned p = r->bgn;
                unsigned c = r->end - r->bgn;
                memcpy(&dst[p * 4], &lconstf->data[n * 4], c * 4 * sizeof(float));
                n += c;
                r = r->next;
            }
            cb.user_buffer = dst;
        }

        context->pipe_data.cb0_swvp.buffer_offset = cb.buffer_offset;
        context->pipe_data.cb0_swvp.buffer_size = cb.buffer_size;
        context->pipe_data.cb0_swvp.user_buffer = cb.user_buffer;

        cb.user_buffer = (int8_t *)cb.user_buffer + 4096 * sizeof(float[4]);
        context->pipe_data.cb1_swvp.buffer_offset = cb.buffer_offset;
        context->pipe_data.cb1_swvp.buffer_size = cb.buffer_size;
        context->pipe_data.cb1_swvp.user_buffer = cb.user_buffer;

        context->changed.vs_const_f = 0;
    }

    if (context->changed.vs_const_i || context->changed.group & NINE_STATE_SWVP) {
        struct pipe_constant_buffer cb;

        cb.buffer_offset = 0;
        cb.buffer_size = 2048 * sizeof(float[4]);
        cb.user_buffer = context->vs_const_i;

        context->pipe_data.cb2_swvp.buffer_offset = cb.buffer_offset;
        context->pipe_data.cb2_swvp.buffer_size = cb.buffer_size;
        context->pipe_data.cb2_swvp.user_buffer = cb.user_buffer;
        context->changed.vs_const_i = 0;
    }

    if (context->changed.vs_const_b || context->changed.group & NINE_STATE_SWVP) {
        struct pipe_constant_buffer cb;

        cb.buffer_offset = 0;
        cb.buffer_size = 512 * sizeof(float[4]);
        cb.user_buffer = context->vs_const_b;

        context->pipe_data.cb3_swvp.buffer_offset = cb.buffer_offset;
        context->pipe_data.cb3_swvp.buffer_size = cb.buffer_size;
        context->pipe_data.cb3_swvp.user_buffer = cb.user_buffer;
        context->changed.vs_const_b = 0;
    }

    context->changed.group &= ~NINE_STATE_VS_CONST;
    context->commit |= NINE_STATE_COMMIT_CONST_VS;
}

static void
prepare_vs_constants_userbuf(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;
    uint8_t *upload_ptr = NULL;
    struct pipe_constant_buffer cb;
    cb.buffer = NULL;
    cb.buffer_offset = 0;
    cb.buffer_size = context->cso_shader.vs_const_used_size;
    cb.user_buffer = context->vs_const_f;

    if (context->swvp) {
        prepare_vs_constants_userbuf_swvp(device);
        return;
    }

    if (device->driver_caps.emulate_ucp) {
        /* TODO: Avoid memcpy all time by storing directly into the array */
        memcpy(&context->vs_const_f[4 * NINE_MAX_CONST_VS_SPE_OFFSET], &context->clip.ucp, sizeof(context->clip));
    }
    if (device->driver_caps.always_output_pointsize) {
        context->vs_const_f[4 * (NINE_MAX_CONST_VS_SPE_OFFSET + 8)] =
            CLAMP(asfloat(context->rs[D3DRS_POINTSIZE]),
                asfloat(context->rs[D3DRS_POINTSIZE_MIN]),
                asfloat(context->rs[D3DRS_POINTSIZE_MAX]));
    }

    if (context->changed.vs_const_i || context->changed.group & NINE_STATE_SWVP) {
        int *idst = (int *)&context->vs_const_f[4 * device->max_vs_const_f];
        memcpy(idst, context->vs_const_i, NINE_MAX_CONST_I * sizeof(int[4]));
        context->changed.vs_const_i = 0;
    }

    if (context->changed.vs_const_b || context->changed.group & NINE_STATE_SWVP) {
        int *idst = (int *)&context->vs_const_f[4 * device->max_vs_const_f];
        uint32_t *bdst = (uint32_t *)&idst[4 * NINE_MAX_CONST_I];
        memcpy(bdst, context->vs_const_b, NINE_MAX_CONST_B * sizeof(BOOL));
        context->changed.vs_const_b = 0;
    }

    if (!cb.buffer_size)
        return;

    if (context->vs->lconstf.ranges) {
        /* TODO: Can we make it so that we don't have to copy everything ? */
        const struct nine_lconstf *lconstf =  &(context->vs->lconstf);
        const struct nine_range *r = lconstf->ranges;
        unsigned n = 0;
        float *dst = context->vs_lconstf_temp;
        float *src = (float *)cb.user_buffer;
        memcpy(dst, src, cb.buffer_size);
        while (r) {
            unsigned p = r->bgn;
            unsigned c = r->end - r->bgn;
            memcpy(&dst[p * 4], &lconstf->data[n * 4], c * 4 * sizeof(float));
            n += c;
            r = r->next;
        }
        cb.user_buffer = dst;
    }

    /* Note: We probably don't want to do separate memcpy to
     * upload_ptr directly, if we have to copy some constants
     * at random locations (context->vs->lconstf.ranges),
     * to have efficient WC. Thus for this case we really want
     * that intermediate buffer. */

    u_upload_alloc(context->pipe->const_uploader,
                  0,
                  cb.buffer_size,
                  256, /* Be conservative about alignment */
                  &(cb.buffer_offset),
                  &(cb.buffer),
                  (void**)&upload_ptr);

    assert(cb.buffer && upload_ptr);

    if (!context->cso_shader.vs_const_ranges) {
        memcpy(upload_ptr, cb.user_buffer, cb.buffer_size);
    } else {
        unsigned i = 0;
        unsigned offset = 0;
        while (context->cso_shader.vs_const_ranges[i*2+1] != 0) {
            memcpy(upload_ptr+offset,
                   &((float*)cb.user_buffer)[4*context->cso_shader.vs_const_ranges[i*2]],
                   context->cso_shader.vs_const_ranges[i*2+1] * sizeof(float[4]));
            offset += context->cso_shader.vs_const_ranges[i*2+1] * sizeof(float[4]);
            i++;
        }
    }

    u_upload_unmap(context->pipe->const_uploader);
    cb.user_buffer = NULL;

    /* Free previous resource */
    pipe_resource_reference(&context->pipe_data.cb_vs.buffer, NULL);

    context->pipe_data.cb_vs = cb;
    context->changed.vs_const_f = 0;

    context->changed.group &= ~NINE_STATE_VS_CONST;
    context->commit |= NINE_STATE_COMMIT_CONST_VS;
}

static void
prepare_ps_constants_userbuf(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;
    uint8_t *upload_ptr = NULL;
    struct pipe_constant_buffer cb;
    cb.buffer = NULL;
    cb.buffer_offset = 0;
    cb.buffer_size = context->cso_shader.ps_const_used_size;
    cb.user_buffer = context->ps_const_f;

    if (context->changed.ps_const_i) {
        int *idst = (int *)&context->ps_const_f[4 * NINE_MAX_CONST_F_PS3];
        memcpy(idst, context->ps_const_i, sizeof(context->ps_const_i));
        context->changed.ps_const_i = 0;
    }
    if (context->changed.ps_const_b) {
        int *idst = (int *)&context->ps_const_f[4 * NINE_MAX_CONST_F_PS3];
        uint32_t *bdst = (uint32_t *)&idst[4 * NINE_MAX_CONST_I];
        memcpy(bdst, context->ps_const_b, sizeof(context->ps_const_b));
        context->changed.ps_const_b = 0;
    }

    /* Upload special constants needed to implement PS1.x instructions like TEXBEM,TEXBEML and BEM */
    if (context->ps->bumpenvmat_needed)
        memcpy(&context->ps_const_f[4 * NINE_MAX_CONST_PS_SPE_OFFSET], &device->context.bumpmap_vars, sizeof(device->context.bumpmap_vars));

    if (context->ps->byte_code.version < 0x30 &&
        context->rs[D3DRS_FOGENABLE]) {
        float *dst = &context->ps_const_f[4 * (NINE_MAX_CONST_PS_SPE_OFFSET + 12)];

        d3dcolor_to_rgba(dst, context->rs[D3DRS_FOGCOLOR]);
        if (context->rs[D3DRS_FOGTABLEMODE] == D3DFOG_LINEAR) {
            dst[4] = asfloat(context->rs[D3DRS_FOGEND]);
            dst[5] = 1.0f / (asfloat(context->rs[D3DRS_FOGEND]) - asfloat(context->rs[D3DRS_FOGSTART]));
        } else if (context->rs[D3DRS_FOGTABLEMODE] != D3DFOG_NONE) {
            dst[4] = asfloat(context->rs[D3DRS_FOGDENSITY]);
        }
    }

    context->ps_const_f[4 * (NINE_MAX_CONST_PS_SPE_OFFSET + 14)] = context->rs[D3DRS_ALPHAREF] / 255.f;

    if (!cb.buffer_size)
        return;

    u_upload_alloc(context->pipe->const_uploader,
                  0,
                  cb.buffer_size,
                  256, /* Be conservative about alignment */
                  &(cb.buffer_offset),
                  &(cb.buffer),
                  (void**)&upload_ptr);

    assert(cb.buffer && upload_ptr);

    if (!context->cso_shader.ps_const_ranges) {
        memcpy(upload_ptr, cb.user_buffer, cb.buffer_size);
    } else {
        unsigned i = 0;
        unsigned offset = 0;
        while (context->cso_shader.ps_const_ranges[i*2+1] != 0) {
            memcpy(upload_ptr+offset,
                   &((float*)cb.user_buffer)[4*context->cso_shader.ps_const_ranges[i*2]],
                   context->cso_shader.ps_const_ranges[i*2+1] * sizeof(float[4]));
            offset += context->cso_shader.ps_const_ranges[i*2+1] * sizeof(float[4]);
            i++;
        }
    }

    u_upload_unmap(context->pipe->const_uploader);
    cb.user_buffer = NULL;

    /* Free previous resource */
    pipe_resource_reference(&context->pipe_data.cb_ps.buffer, NULL);

    context->pipe_data.cb_ps = cb;
    context->changed.ps_const_f = 0;

    context->changed.group &= ~NINE_STATE_PS_CONST;
    context->commit |= NINE_STATE_COMMIT_CONST_PS;
}

static inline uint32_t
prepare_vs(struct NineDevice9 *device, uint8_t shader_changed)
{
    struct nine_context *context = &device->context;
    struct NineVertexShader9 *vs = context->vs;
    uint32_t changed_group = 0;
    int has_key_changed = 0;

    if (likely(context->programmable_vs))
        has_key_changed = NineVertexShader9_UpdateKey(vs, device);

    if (!shader_changed && !has_key_changed)
        return 0;

    /* likely because we dislike FF */
    if (likely(context->programmable_vs)) {
        context->cso_shader.vs = NineVertexShader9_GetVariant(vs,
                                                              &context->cso_shader.vs_const_ranges,
                                                              &context->cso_shader.vs_const_used_size);
    } else {
        vs = device->ff.vs;
        context->cso_shader.vs = vs->ff_cso;
    }

    if (context->rs[NINED3DRS_VSPOINTSIZE] != vs->point_size) {
        context->rs[NINED3DRS_VSPOINTSIZE] = vs->point_size;
        changed_group |= NINE_STATE_RASTERIZER;
    }
    if (context->rs[NINED3DRS_POSITIONT] != vs->position_t) {
        context->rs[NINED3DRS_POSITIONT] = vs->position_t;
        if (!device->driver_caps.window_space_position_support &&
            device->driver_caps.disabling_depth_clipping_support)
            changed_group |= NINE_STATE_RASTERIZER;
    }

    if ((context->bound_samplers_mask_vs & vs->sampler_mask) != vs->sampler_mask)
        /* Bound dummy sampler. */
        changed_group |= NINE_STATE_SAMPLER;

    context->commit |= NINE_STATE_COMMIT_VS;
    return changed_group;
}

static inline uint32_t
prepare_ps(struct NineDevice9 *device, uint8_t shader_changed)
{
    struct nine_context *context = &device->context;
    struct NinePixelShader9 *ps = context->ps;
    uint32_t changed_group = 0;
    int has_key_changed = 0;

    if (likely(ps))
        has_key_changed = NinePixelShader9_UpdateKey(ps, context);

    if (!shader_changed && !has_key_changed)
        return 0;

    if (likely(ps)) {
        context->cso_shader.ps = NinePixelShader9_GetVariant(ps,
                                                             &context->cso_shader.ps_const_ranges,
                                                             &context->cso_shader.ps_const_used_size);
    } else {
        ps = device->ff.ps;
        context->cso_shader.ps = ps->ff_cso;
    }

    if ((context->bound_samplers_mask_ps & ps->sampler_mask) != ps->sampler_mask)
        /* Bound dummy sampler. */
        changed_group |= NINE_STATE_SAMPLER;

    context->commit |= NINE_STATE_COMMIT_PS;
    return changed_group;
}

/* State preparation incremental */

/* State preparation + State commit */

static void
update_framebuffer(struct NineDevice9 *device, bool is_clear)
{
    struct nine_context *context = &device->context;
    struct pipe_context *pipe = context->pipe;
    struct pipe_framebuffer_state *fb = &context->pipe_data.fb;
    unsigned i;
    struct NineSurface9 *rt0 = context->rt[0];
    unsigned w = rt0->desc.Width;
    unsigned h = rt0->desc.Height;
    unsigned nr_samples = rt0->base.info.nr_samples;
    unsigned ps_mask = context->ps ? context->ps->rt_mask : 1;
    unsigned mask = is_clear ? 0xf : ps_mask;
    const int sRGB = context->rs[D3DRS_SRGBWRITEENABLE] ? 1 : 0;

    DBG("\n");

    context->rt_mask = 0x0;
    fb->nr_cbufs = 0;

    /* all render targets must have the same size and the depth buffer must be
     * bigger. Multisample has to match, according to spec. But some apps do
     * things wrong there, and no error is returned. The behaviour they get
     * apparently is that depth buffer is disabled if it doesn't match.
     * Surely the same for render targets. */

    /* Special case: D3DFMT_NULL is used to bound no real render target,
     * but render to depth buffer. We have to not take into account the render
     * target info. TODO: know what should happen when there are several render targers
     * and the first one is D3DFMT_NULL */
    if (rt0->desc.Format == D3DFMT_NULL && context->ds) {
        w = context->ds->desc.Width;
        h = context->ds->desc.Height;
        nr_samples = context->ds->base.info.nr_samples;
    }

    for (i = 0; i < device->caps.NumSimultaneousRTs; ++i) {
        struct NineSurface9 *rt = context->rt[i];

        if (rt && rt->desc.Format != D3DFMT_NULL && (mask & (1 << i)) &&
            rt->desc.Width == w && rt->desc.Height == h &&
            rt->base.info.nr_samples == nr_samples) {
            fb->cbufs[i] = NineSurface9_GetSurface(rt, sRGB);
            context->rt_mask |= 1 << i;
            fb->nr_cbufs = i + 1;
        } else {
            /* Color outputs must match RT slot,
             * drivers will have to handle NULL entries for GL, too.
             */
            fb->cbufs[i] = NULL;
        }
    }

    if (context->ds && context->ds->desc.Width >= w &&
        context->ds->desc.Height >= h &&
        context->ds->base.info.nr_samples == nr_samples) {
        fb->zsbuf = NineSurface9_GetSurface(context->ds, 0);
    } else {
        fb->zsbuf = NULL;
    }

    fb->width = w;
    fb->height = h;

    pipe->set_framebuffer_state(pipe, fb); /* XXX: cso ? */

    if (is_clear && context->rt_mask == ps_mask)
        context->changed.group &= ~NINE_STATE_FB;
}

static void
update_viewport(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;
    const D3DVIEWPORT9 *vport = &context->viewport;
    struct pipe_viewport_state pvport;

    /* D3D coordinates are:
     * -1 .. +1 for X,Y and
     *  0 .. +1 for Z (we use pipe_rasterizer_state.clip_halfz)
     */
    pvport.scale[0] = (float)vport->Width * 0.5f;
    pvport.scale[1] = (float)vport->Height * -0.5f;
    pvport.scale[2] = vport->MaxZ - vport->MinZ;
    pvport.translate[0] = (float)vport->Width * 0.5f + (float)vport->X;
    pvport.translate[1] = (float)vport->Height * 0.5f + (float)vport->Y;
    pvport.translate[2] = vport->MinZ;
    pvport.swizzle_x = PIPE_VIEWPORT_SWIZZLE_POSITIVE_X;
    pvport.swizzle_y = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Y;
    pvport.swizzle_z = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Z;
    pvport.swizzle_w = PIPE_VIEWPORT_SWIZZLE_POSITIVE_W;

    /* We found R600 and SI cards have some imprecision
     * on the barycentric coordinates used for interpolation.
     * Some shaders rely on having something precise.
     * We found that the proprietary driver has the imprecision issue,
     * except when the render target width and height are powers of two.
     * It is using some sort of workaround for these cases
     * which covers likely all the cases the applications rely
     * on something precise.
     * We haven't found the workaround, but it seems like it's better
     * for applications if the imprecision is biased towards infinity
     * instead of -infinity (which is what measured). So shift slightly
     * the viewport: not enough to change rasterization result (in particular
     * for multisampling), but enough to make the imprecision biased
     * towards infinity. We do this shift only if render target width and
     * height are powers of two.
     * Solves 'red shadows' bug on UE3 games.
     */
    if (device->driver_bugs.buggy_barycentrics &&
        ((vport->Width & (vport->Width-1)) == 0) &&
        ((vport->Height & (vport->Height-1)) == 0)) {
        pvport.translate[0] -= 1.0f / 128.0f;
        pvport.translate[1] -= 1.0f / 128.0f;
    }

    cso_set_viewport(context->cso, &pvport);
}

/* Loop through VS inputs and pick the vertex elements with the declared
 * usage from the vertex declaration, then insert the instance divisor from
 * the stream source frequency setting.
 */
static void
update_vertex_elements(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;
    const struct NineVertexDeclaration9 *vdecl = device->context.vdecl;
    const struct NineVertexShader9 *vs;
    unsigned n, b, i;
    int index;
    int8_t vdecl_index_map[16]; /* vs->num_inputs <= 16 */
    uint16_t used_streams = 0;
    int dummy_vbo_stream = -1;
    BOOL need_dummy_vbo = false;
    struct cso_velems_state ve;
    unsigned vtxbuf_mask;
    unsigned vtxbuf_holes_map[PIPE_MAX_ATTRIBS];

    context->stream_usage_mask = 0;
    memset(vdecl_index_map, -1, 16);
    vs = context->programmable_vs ? context->vs : device->ff.vs;

    if (vdecl) {
        for (n = 0; n < vs->num_inputs; ++n) {
            DBG("looking up input %u (usage %u) from vdecl(%p)\n",
                n, vs->input_map[n].ndecl, vdecl);

            for (i = 0; i < vdecl->nelems; i++) {
                if (vdecl->usage_map[i] == vs->input_map[n].ndecl) {
                    vdecl_index_map[n] = i;
                    used_streams |= BITFIELD_BIT(vdecl->elems[i].vertex_buffer_index);
                    break;
                }
            }
            if (vdecl_index_map[n] < 0)
                need_dummy_vbo = true;
        }
    } else {
        /* No vertex declaration. Likely will never happen in practice,
         * but we need not crash on this */
        need_dummy_vbo = true;
    }

    if (need_dummy_vbo) {
        u_foreach_bit(bit, BITFIELD_MASK(device->caps.MaxStreams) & ~used_streams) {
            dummy_vbo_stream = bit;
            break;
        }
    }
    /* there are fewer vertex shader inputs than stream slots,
     * so if we need a slot for the dummy vbo, we should have found one */
    assert (!need_dummy_vbo || dummy_vbo_stream != -1);

    /* calculate vtxbuf_holes_map to match for update_vertex_buffers() function */
    i = 0;
    vtxbuf_mask = context->vtxbuf_mask |
        (need_dummy_vbo ? BITFIELD_BIT(dummy_vbo_stream) : 0);
    u_foreach_bit(bit, vtxbuf_mask)
        vtxbuf_holes_map[bit] = i++;

    for (n = 0; n < vs->num_inputs; ++n) {
        index = vdecl_index_map[n];
        if (index >= 0) {
            ve.velems[n] = vdecl->elems[index];
            ve.velems[n].vertex_buffer_index =
                vtxbuf_holes_map[ve.velems[n].vertex_buffer_index];
            b = ve.velems[n].vertex_buffer_index;
            ve.velems[n].src_stride = context->vtxstride[b];
            context->stream_usage_mask |= 1 << b;
            /* XXX wine just uses 1 here: */
            if (context->stream_freq[b] & D3DSTREAMSOURCE_INSTANCEDATA)
                ve.velems[n].instance_divisor = context->stream_freq[b] & 0x7FFFFF;
        } else {
            /* if the vertex declaration is incomplete compared to what the
             * vertex shader needs, we bind a dummy vbo with 0 0 0 0.
             * This is not precised by the spec, but is the behaviour
             * tested on win */
            ve.velems[n].vertex_buffer_index = vtxbuf_holes_map[dummy_vbo_stream];
            ve.velems[n].src_format = PIPE_FORMAT_R32G32B32A32_FLOAT;
            ve.velems[n].src_offset = 0;
            ve.velems[n].src_stride = 0;
            ve.velems[n].instance_divisor = 0;
            ve.velems[n].dual_slot = false;
        }
    }

    if (context->dummy_vbo_bound_at != dummy_vbo_stream) {
        if (context->dummy_vbo_bound_at >= 0)
            context->changed.vtxbuf |= 1 << context->dummy_vbo_bound_at;
        if (dummy_vbo_stream >= 0)
            context->changed.vtxbuf |= 1 << dummy_vbo_stream;
        context->dummy_vbo_bound_at = dummy_vbo_stream;
    }

    ve.count = vs->num_inputs;
    cso_set_vertex_elements(context->cso, &ve);
}

static void
update_vertex_buffers(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;
    struct pipe_context *pipe = context->pipe;
    struct pipe_vertex_buffer vbuffer[PIPE_MAX_ATTRIBS];
    unsigned vtxbuf_count;
    unsigned trailing_count;
    unsigned mask, i, vtxbuf_i;

    mask = context->vtxbuf_mask |
        ((context->dummy_vbo_bound_at >= 0) ? BITFIELD_BIT(context->dummy_vbo_bound_at) : 0);
    vtxbuf_count = util_bitcount(mask);

    DBG("mask=%x\n", mask);
    for (i = 0; mask; i++) {
        vtxbuf_i = u_bit_scan(&mask);
        if (vtxbuf_i == context->dummy_vbo_bound_at) {
            vbuffer[i].buffer.resource = device->dummy_vbo;
            vbuffer[i].is_user_buffer = false;
            vbuffer[i].buffer_offset = 0;
        } else {
            memcpy(&vbuffer[i], &context->vtxbuf[vtxbuf_i], sizeof(struct pipe_vertex_buffer));
        }
    }

    trailing_count = (context->last_vtxbuf_count <= vtxbuf_count) ? 0 :
        context->last_vtxbuf_count - vtxbuf_count;
    if (vtxbuf_count)
        pipe->set_vertex_buffers(pipe, vtxbuf_count, trailing_count, false, vbuffer);
    else
        pipe->set_vertex_buffers(pipe, 0, trailing_count, false, NULL);

    context->last_vtxbuf_count = vtxbuf_count;
    context->changed.vtxbuf = 0;
}

static inline bool
update_sampler_derived(struct nine_context *context, unsigned s)
{
    bool changed = false;

    if (context->samp[s][NINED3DSAMP_SHADOW] != context->texture[s].shadow) {
        changed = true;
        context->samp[s][NINED3DSAMP_SHADOW] = context->texture[s].shadow;
    }

    if (context->samp[s][NINED3DSAMP_CUBETEX] !=
        (context->texture[s].type == D3DRTYPE_CUBETEXTURE)) {
        changed = true;
        context->samp[s][NINED3DSAMP_CUBETEX] =
                context->texture[s].type == D3DRTYPE_CUBETEXTURE;
    }

    if (context->samp[s][D3DSAMP_MIPFILTER] != D3DTEXF_NONE) {
        int lod = context->samp[s][D3DSAMP_MAXMIPLEVEL] - context->texture[s].lod;
        if (lod < 0)
            lod = 0;
        if (context->samp[s][NINED3DSAMP_MINLOD] != lod) {
            changed = true;
            context->samp[s][NINED3DSAMP_MINLOD] = lod;
        }
    } else {
        context->changed.sampler[s] &= ~0x300; /* lod changes irrelevant */
    }

    return changed;
}

/* TODO: add sRGB override to pipe_sampler_state ? */
static void
update_textures_and_samplers(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;
    struct pipe_context *pipe = context->pipe;
    struct pipe_sampler_view *view[NINE_MAX_SAMPLERS];
    unsigned num_textures = 0;
    bool commit_samplers;
    uint16_t sampler_mask = context->ps ? context->ps->sampler_mask :
                            device->ff.ps->sampler_mask;

    commit_samplers = false;
    const uint16_t ps_mask = sampler_mask | context->enabled_samplers_mask_ps;
    context->bound_samplers_mask_ps = ps_mask;
    num_textures = util_last_bit(ps_mask);
    /* iterate over the enabled samplers */
    u_foreach_bit(i, context->enabled_samplers_mask_ps) {
        const unsigned s = NINE_SAMPLER_PS(i);
        int sRGB = context->samp[s][D3DSAMP_SRGBTEXTURE] ? 1 : 0;

        view[i] = context->texture[s].view[sRGB];

        if (update_sampler_derived(context, s) || (context->changed.sampler[s] & 0x05fe)) {
            context->changed.sampler[s] = 0;
            commit_samplers = true;
            nine_convert_sampler_state(context->cso, s, context->samp[s]);
        }
    }
    /* iterate over the dummy samplers */
    u_foreach_bit(i, sampler_mask & ~context->enabled_samplers_mask_ps) {
        const unsigned s = NINE_SAMPLER_PS(i);
        /* Bind dummy sampler. We do not bind dummy sampler when
         * it is not needed because it could add overhead. The
         * dummy sampler should have r=g=b=0 and a=1. We do not
         * unbind dummy sampler directly when they are not needed
         * anymore, but they're going to be removed as long as texture
         * or sampler states are changed. */
        view[i] = device->dummy_sampler_view;

        cso_single_sampler(context->cso, PIPE_SHADER_FRAGMENT,
                           s - NINE_SAMPLER_PS(0), &device->dummy_sampler_state);

        commit_samplers = true;
        context->changed.sampler[s] = ~0;
    }
    /* fill in unused samplers */
    u_foreach_bit(i, BITFIELD_MASK(num_textures) & ~ps_mask)
       view[i] = NULL;

    pipe->set_sampler_views(pipe, PIPE_SHADER_FRAGMENT, 0, num_textures,
                            num_textures < context->enabled_sampler_count_ps ? context->enabled_sampler_count_ps - num_textures : 0,
                            false, view);
    context->enabled_sampler_count_ps = num_textures;

    if (commit_samplers)
        cso_single_sampler_done(context->cso, PIPE_SHADER_FRAGMENT);

    commit_samplers = false;
    sampler_mask = context->programmable_vs ? context->vs->sampler_mask : 0;
    const uint16_t vs_mask = sampler_mask | context->enabled_samplers_mask_vs;
    context->bound_samplers_mask_vs = vs_mask;
    num_textures = util_last_bit(vs_mask);
    u_foreach_bit(i, context->enabled_samplers_mask_vs) {
        const unsigned s = NINE_SAMPLER_VS(i);
        int sRGB = context->samp[s][D3DSAMP_SRGBTEXTURE] ? 1 : 0;

        view[i] = context->texture[s].view[sRGB];

        if (update_sampler_derived(context, s) || (context->changed.sampler[s] & 0x05fe)) {
            context->changed.sampler[s] = 0;
            commit_samplers = true;
            nine_convert_sampler_state(context->cso, s, context->samp[s]);
        }
    }
    u_foreach_bit(i, sampler_mask & ~context->enabled_samplers_mask_vs) {
        const unsigned s = NINE_SAMPLER_VS(i);
        /* Bind dummy sampler. We do not bind dummy sampler when
         * it is not needed because it could add overhead. The
         * dummy sampler should have r=g=b=0 and a=1. We do not
         * unbind dummy sampler directly when they are not needed
         * anymore, but they're going to be removed as long as texture
         * or sampler states are changed. */
        view[i] = device->dummy_sampler_view;

        cso_single_sampler(context->cso, PIPE_SHADER_VERTEX,
                           s - NINE_SAMPLER_VS(0), &device->dummy_sampler_state);

        commit_samplers = true;
        context->changed.sampler[s] = ~0;
    }
    /* fill in unused samplers */
    u_foreach_bit(i, BITFIELD_MASK(num_textures) & ~vs_mask)
       view[i] = NULL;

    pipe->set_sampler_views(pipe, PIPE_SHADER_VERTEX, 0, num_textures,
                            num_textures < context->enabled_sampler_count_vs ? context->enabled_sampler_count_vs - num_textures : 0,
                            false, view);
    context->enabled_sampler_count_vs = num_textures;

    if (commit_samplers)
        cso_single_sampler_done(context->cso, PIPE_SHADER_VERTEX);
}

/* State commit only */

static inline void
commit_blend(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;

    cso_set_blend(context->cso, &context->pipe_data.blend);
}

static inline void
commit_dsa(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;

    cso_set_depth_stencil_alpha(context->cso, &context->pipe_data.dsa);
}

static inline void
commit_scissor(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;
    struct pipe_context *pipe = context->pipe;

    pipe->set_scissor_states(pipe, 0, 1, &context->scissor);
}

static inline void
commit_rasterizer(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;

    cso_set_rasterizer(context->cso, &context->pipe_data.rast);
}

static inline void
commit_vs_constants(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;
    struct pipe_context *pipe = context->pipe;

    if (unlikely(!context->programmable_vs))
        pipe->set_constant_buffer(pipe, PIPE_SHADER_VERTEX, 0, false, &context->pipe_data.cb_vs_ff);
    else {
        if (context->swvp) {
            pipe->set_constant_buffer(pipe, PIPE_SHADER_VERTEX, 0, false, &context->pipe_data.cb0_swvp);
            pipe->set_constant_buffer(pipe, PIPE_SHADER_VERTEX, 1, false, &context->pipe_data.cb1_swvp);
            pipe->set_constant_buffer(pipe, PIPE_SHADER_VERTEX, 2, false, &context->pipe_data.cb2_swvp);
            pipe->set_constant_buffer(pipe, PIPE_SHADER_VERTEX, 3, false, &context->pipe_data.cb3_swvp);
        } else {
            pipe->set_constant_buffer(pipe, PIPE_SHADER_VERTEX, 0, false, &context->pipe_data.cb_vs);
        }
    }
}

static inline void
commit_ps_constants(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;
    struct pipe_context *pipe = context->pipe;

    if (unlikely(!context->ps))
        pipe->set_constant_buffer(pipe, PIPE_SHADER_FRAGMENT, 0, false, &context->pipe_data.cb_ps_ff);
    else
        pipe->set_constant_buffer(pipe, PIPE_SHADER_FRAGMENT, 0, false, &context->pipe_data.cb_ps);
}

static inline void
commit_vs(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;
    assert(context->cso_shader.vs);

    context->pipe->bind_vs_state(context->pipe, context->cso_shader.vs);
}


static inline void
commit_ps(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;

    context->pipe->bind_fs_state(context->pipe, context->cso_shader.ps);
}
/* State Update */

#define NINE_STATE_SHADER_CHANGE_VS \
   (NINE_STATE_VS |         \
    NINE_STATE_TEXTURE |    \
    NINE_STATE_VS_PARAMS_MISC | \
    NINE_STATE_SWVP)

#define NINE_STATE_SHADER_CHANGE_PS \
   (NINE_STATE_PS |         \
    NINE_STATE_TEXTURE |    \
    NINE_STATE_PS_PARAMS_MISC)

#define NINE_STATE_FREQUENT \
   (NINE_STATE_RASTERIZER | \
    NINE_STATE_TEXTURE |    \
    NINE_STATE_SAMPLER |    \
    NINE_STATE_VS_CONST |   \
    NINE_STATE_PS_CONST |   \
    NINE_STATE_MULTISAMPLE)

#define NINE_STATE_COMMON \
   (NINE_STATE_FB |       \
    NINE_STATE_BLEND |    \
    NINE_STATE_DSA |      \
    NINE_STATE_VIEWPORT | \
    NINE_STATE_VDECL |    \
    NINE_STATE_IDXBUF |   \
    NINE_STATE_STREAMFREQ)

#define NINE_STATE_RARE      \
   (NINE_STATE_SCISSOR |     \
    NINE_STATE_BLEND_COLOR | \
    NINE_STATE_STENCIL_REF | \
    NINE_STATE_SAMPLE_MASK)

static void
nine_update_state(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;
    struct pipe_context *pipe = context->pipe;
    uint32_t group;

    DBG("changed state groups: %x\n", context->changed.group);

    /* NOTE: We may want to use the cso cache for everything, or let
     * NineDevice9.RestoreNonCSOState actually set the states, then we wouldn't
     * have to care about state being clobbered here and could merge this back
     * into update_textures. Except, we also need to re-validate textures that
     * may be dirty anyway, even if no texture bindings changed.
     */

    /* ff_update may change VS/PS dirty bits */
    if (unlikely(!context->programmable_vs || !context->ps))
        nine_ff_update(device);
    group = context->changed.group;

    if (group & (NINE_STATE_SHADER_CHANGE_VS | NINE_STATE_SHADER_CHANGE_PS)) {
        if (group & NINE_STATE_SHADER_CHANGE_VS)
            group |= prepare_vs(device, (group & NINE_STATE_VS) != 0); /* may set NINE_STATE_RASTERIZER and NINE_STATE_SAMPLER*/
        if (group & NINE_STATE_SHADER_CHANGE_PS)
            group |= prepare_ps(device, (group & NINE_STATE_PS) != 0);
    }

    if (group & (NINE_STATE_COMMON | NINE_STATE_VS)) {
        if (group & NINE_STATE_FB)
            update_framebuffer(device, false);
        if (group & NINE_STATE_BLEND)
            prepare_blend(device);
        if (group & NINE_STATE_DSA)
            prepare_dsa(device);
        if (group & NINE_STATE_VIEWPORT)
            update_viewport(device);
        if (group & (NINE_STATE_VDECL | NINE_STATE_VS | NINE_STATE_STREAMFREQ))
            update_vertex_elements(device);
    }

    if (likely(group & (NINE_STATE_FREQUENT | NINE_STATE_VS | NINE_STATE_PS | NINE_STATE_SWVP))) {
        if (group & NINE_STATE_MULTISAMPLE)
            group |= check_multisample(device);
        if (group & NINE_STATE_RASTERIZER)
            prepare_rasterizer(device);
        if (group & (NINE_STATE_TEXTURE | NINE_STATE_SAMPLER))
            update_textures_and_samplers(device);
        if ((group & (NINE_STATE_VS_CONST | NINE_STATE_VS | NINE_STATE_SWVP)) && context->programmable_vs)
            prepare_vs_constants_userbuf(device);
        if ((group & (NINE_STATE_PS_CONST | NINE_STATE_PS)) && context->ps)
            prepare_ps_constants_userbuf(device);
    }

    if (context->changed.vtxbuf)
        update_vertex_buffers(device);

    if (context->commit & NINE_STATE_COMMIT_BLEND)
        commit_blend(device);
    if (context->commit & NINE_STATE_COMMIT_DSA)
        commit_dsa(device);
    if (context->commit & NINE_STATE_COMMIT_RASTERIZER)
        commit_rasterizer(device);
    if (context->commit & NINE_STATE_COMMIT_CONST_VS)
        commit_vs_constants(device);
    if (context->commit & NINE_STATE_COMMIT_CONST_PS)
        commit_ps_constants(device);
    if (context->commit & NINE_STATE_COMMIT_VS)
        commit_vs(device);
    if (context->commit & NINE_STATE_COMMIT_PS)
        commit_ps(device);

    context->commit = 0;

    if (unlikely(context->changed.ucp)) {
        pipe->set_clip_state(pipe, &context->clip);
        context->changed.ucp = false;
    }

    if (unlikely(group & NINE_STATE_RARE)) {
        if (group & NINE_STATE_SCISSOR)
            commit_scissor(device);
        if (group & NINE_STATE_BLEND_COLOR) {
            struct pipe_blend_color color;
            d3dcolor_to_rgba(&color.color[0], context->rs[D3DRS_BLENDFACTOR]);
            pipe->set_blend_color(pipe, &color);
        }
        if (group & NINE_STATE_SAMPLE_MASK) {
            if (context->rt[0]->desc.MultiSampleType <= D3DMULTISAMPLE_NONMASKABLE) {
                pipe->set_sample_mask(pipe, ~0);
            } else {
                pipe->set_sample_mask(pipe, context->rs[D3DRS_MULTISAMPLEMASK]);
            }
        }
        if (group & NINE_STATE_STENCIL_REF) {
            struct pipe_stencil_ref ref;
            ref.ref_value[0] = context->rs[D3DRS_STENCILREF];
            ref.ref_value[1] = ref.ref_value[0];
            pipe->set_stencil_ref(pipe, ref);
        }
    }

    context->changed.group &=
        (NINE_STATE_FF | NINE_STATE_VS_CONST | NINE_STATE_PS_CONST);

    DBG("finished\n");
}

#define RESZ_CODE 0x7fa05000

static void
NineDevice9_ResolveZ( struct NineDevice9 *device )
{
    struct nine_context *context = &device->context;
    const struct util_format_description *desc;
    struct NineSurface9 *source = context->ds;
    struct pipe_resource *src, *dst;
    struct pipe_blit_info blit;

    DBG("RESZ resolve\n");

    if (!source || !context->texture[0].enabled ||
        context->texture[0].type != D3DRTYPE_TEXTURE)
        return;

    src = source->base.resource;
    dst = context->texture[0].resource;

    if (!src || !dst)
        return;

    /* check dst is depth format. we know already for src */
    desc = util_format_description(dst->format);
    if (desc->colorspace != UTIL_FORMAT_COLORSPACE_ZS)
        return;

    memset(&blit, 0, sizeof(blit));
    blit.src.resource = src;
    blit.src.level = 0;
    blit.src.format = src->format;
    blit.src.box.z = 0;
    blit.src.box.depth = 1;
    blit.src.box.x = 0;
    blit.src.box.y = 0;
    blit.src.box.width = src->width0;
    blit.src.box.height = src->height0;

    blit.dst.resource = dst;
    blit.dst.level = 0;
    blit.dst.format = dst->format;
    blit.dst.box.z = 0;
    blit.dst.box.depth = 1;
    blit.dst.box.x = 0;
    blit.dst.box.y = 0;
    blit.dst.box.width = dst->width0;
    blit.dst.box.height = dst->height0;

    blit.mask = PIPE_MASK_ZS;
    blit.filter = PIPE_TEX_FILTER_NEAREST;
    blit.scissor_enable = false;

    context->pipe->blit(context->pipe, &blit);
}

#define ALPHA_TO_COVERAGE_ENABLE   MAKEFOURCC('A', '2', 'M', '1')
#define ALPHA_TO_COVERAGE_DISABLE  MAKEFOURCC('A', '2', 'M', '0')
#define FETCH4_ENABLE              MAKEFOURCC('G', 'E', 'T', '4')
#define FETCH4_DISABLE             MAKEFOURCC('G', 'E', 'T', '1')

/* Nine_context functions.
 * Serialized through CSMT macros.
 */

static void
nine_context_set_texture_apply(struct NineDevice9 *device,
                               DWORD stage,
                               DWORD fetch4_shadow_enabled,
                               DWORD lod,
                               D3DRESOURCETYPE type,
                               uint8_t pstype,
                               struct pipe_resource *res,
                               struct pipe_sampler_view *view0,
                               struct pipe_sampler_view *view1);

static void
nine_context_set_pixel_shader_constant_i_transformed(struct NineDevice9 *device,
                                                     UINT StartRegister,
                                                     const int *pConstantData,
                                                     unsigned pConstantData_size,
                                                     UINT Vector4iCount);

CSMT_ITEM_NO_WAIT(nine_context_set_render_state,
                  ARG_VAL(D3DRENDERSTATETYPE, State),
                  ARG_VAL(DWORD, Value))
{
    struct nine_context *context = &device->context;

    /* Amd hacks (equivalent to GL extensions) */
    if (unlikely(State == D3DRS_POINTSIZE)) {
        if (Value == RESZ_CODE) {
            NineDevice9_ResolveZ(device);
            return;
        }

        /* NINED3DRS_ALPHACOVERAGE:
         * bit 0: NVIDIA alpha to coverage
         * bit 1: NVIDIA ATOC state active
         * bit 2: AMD alpha to coverage
         * These need to be separate else the set of states to
         * disable NVIDIA alpha to coverage can disable the AMD one */
        if (Value == ALPHA_TO_COVERAGE_ENABLE ||
            Value == ALPHA_TO_COVERAGE_DISABLE) {
            context->rs[NINED3DRS_ALPHACOVERAGE] &= 3;
            context->rs[NINED3DRS_ALPHACOVERAGE] |= (Value == ALPHA_TO_COVERAGE_ENABLE) ? 4 : 0;
            context->changed.group |= NINE_STATE_BLEND;
            return;
        }
    }

    /* NV hack */
    if (unlikely(State == D3DRS_ADAPTIVETESS_Y)) {
        if (Value == D3DFMT_ATOC || (Value == D3DFMT_UNKNOWN && context->rs[NINED3DRS_ALPHACOVERAGE] & 3)) {
            context->rs[NINED3DRS_ALPHACOVERAGE] &= 4;
            context->rs[NINED3DRS_ALPHACOVERAGE] |=
                ((Value == D3DFMT_ATOC) ? 3 : 0) & (context->rs[D3DRS_ALPHATESTENABLE] ? 3 : 2);
            context->changed.group |= NINE_STATE_BLEND;
            return;
        }
    }
    if (unlikely(State == D3DRS_ALPHATESTENABLE && (context->rs[NINED3DRS_ALPHACOVERAGE] & 2))) {
        context->rs[NINED3DRS_ALPHACOVERAGE] &= 6;
        context->rs[NINED3DRS_ALPHACOVERAGE] |= (Value ? 1 : 0);
    }

    context->rs[State] = nine_fix_render_state_value(State, Value);
    context->changed.group |= nine_render_state_group[State];

    if (device->driver_caps.alpha_test_emulation) {
        if (State == D3DRS_ALPHATESTENABLE || State == D3DRS_ALPHAFUNC) {
            context->rs[NINED3DRS_EMULATED_ALPHATEST] = context->rs[D3DRS_ALPHATESTENABLE] ?
                d3dcmpfunc_to_pipe_func(context->rs[D3DRS_ALPHAFUNC]) : 7;
            context->changed.group |= NINE_STATE_PS_PARAMS_MISC | NINE_STATE_PS_CONST | NINE_STATE_FF_SHADER;
        }
        if (State == D3DRS_ALPHAREF)
            context->changed.group |= NINE_STATE_PS_CONST | NINE_STATE_FF_PS_CONSTS;
    }

    if (device->driver_caps.always_output_pointsize) {
        if (State == D3DRS_POINTSIZE || State == D3DRS_POINTSIZE_MIN || State == D3DRS_POINTSIZE_MAX)
            context->changed.group |= NINE_STATE_VS_CONST;
    }

    if (device->driver_caps.emulate_ucp && State == D3DRS_CLIPPLANEENABLE)
        context->changed.group |= NINE_STATE_VS_PARAMS_MISC | NINE_STATE_VS_CONST;
}

CSMT_ITEM_NO_WAIT(nine_context_set_texture_apply,
                  ARG_VAL(DWORD, stage),
                  ARG_VAL(DWORD, fetch4_shadow_enabled),
                  ARG_VAL(DWORD, lod),
                  ARG_VAL(D3DRESOURCETYPE, type),
                  ARG_VAL(uint8_t, pstype),
                  ARG_BIND_RES(struct pipe_resource, res),
                  ARG_BIND_VIEW(struct pipe_sampler_view, view0),
                  ARG_BIND_VIEW(struct pipe_sampler_view, view1))
{
    struct nine_context *context = &device->context;
    uint enabled = fetch4_shadow_enabled & 1;
    uint shadow = (fetch4_shadow_enabled >> 1) & 1;
    uint fetch4_compatible = (fetch4_shadow_enabled >> 2) & 1;

    context->texture[stage].enabled = enabled;
    if (enabled) {
       if (stage < NINE_MAX_SAMPLERS_PS)
          context->enabled_samplers_mask_ps |= BITFIELD_BIT(stage - NINE_SAMPLER_PS(0));
       else if (stage >= NINE_SAMPLER_VS(0))
          context->enabled_samplers_mask_vs |= BITFIELD_BIT(stage - NINE_SAMPLER_VS(0));
    } else {
       if (stage < NINE_MAX_SAMPLERS_PS)
          context->enabled_samplers_mask_ps &= ~BITFIELD_BIT(stage - NINE_SAMPLER_PS(0));
       else if (stage >= NINE_SAMPLER_VS(0))
          context->enabled_samplers_mask_vs &= ~BITFIELD_BIT(stage - NINE_SAMPLER_VS(0));
    }
    context->samplers_shadow &= ~(1 << stage);
    context->samplers_shadow |= shadow << stage;
    context->samplers_fetch4 &= ~(1 << stage);
    context->samplers_fetch4 |= fetch4_compatible << stage;
    context->texture[stage].shadow = shadow;
    context->texture[stage].lod = lod;
    context->texture[stage].type = type;
    context->texture[stage].pstype = pstype;
    pipe_resource_reference(&context->texture[stage].resource, res);
    pipe_sampler_view_reference(&context->texture[stage].view[0], view0);
    pipe_sampler_view_reference(&context->texture[stage].view[1], view1);

    context->changed.group |= NINE_STATE_TEXTURE;
}

void
nine_context_set_texture(struct NineDevice9 *device,
                         DWORD Stage,
                         struct NineBaseTexture9 *tex)
{
    DWORD fetch4_shadow_enabled = 0;
    DWORD lod = 0;
    D3DRESOURCETYPE type = D3DRTYPE_TEXTURE;
    uint8_t pstype = 0;
    struct pipe_resource *res = NULL;
    struct pipe_sampler_view *view0 = NULL, *view1 = NULL;

    /* For managed pool, the data can be initially incomplete.
     * In that case, the texture is rebound later
     * (in NineBaseTexture9_Validate/NineBaseTexture9_UploadSelf). */
    if (tex && tex->base.resource) {
        fetch4_shadow_enabled = 1;
        fetch4_shadow_enabled |= tex->shadow << 1;
        fetch4_shadow_enabled |= tex->fetch4_compatible << 2;
        lod = tex->managed.lod;
        type = tex->base.type;
        pstype = tex->pstype;
        res = tex->base.resource;
        view0 = NineBaseTexture9_GetSamplerView(tex, 0);
        view1 = NineBaseTexture9_GetSamplerView(tex, 1);
    }

    nine_context_set_texture_apply(device, Stage,
                                   fetch4_shadow_enabled,
                                   lod, type, pstype,
                                   res, view0, view1);
}

CSMT_ITEM_NO_WAIT(nine_context_set_sampler_state,
                  ARG_VAL(DWORD, Sampler),
                  ARG_VAL(D3DSAMPLERSTATETYPE, Type),
                  ARG_VAL(DWORD, Value))
{
    struct nine_context *context = &device->context;

    if (unlikely(Type == D3DSAMP_MIPMAPLODBIAS)) {
        if (Value == FETCH4_ENABLE ||
            Value == FETCH4_DISABLE) {
            context->rs[NINED3DRS_FETCH4] &= ~(1 << Sampler);
            context->rs[NINED3DRS_FETCH4] |= (Value == FETCH4_ENABLE) << Sampler;
            context->changed.group |= NINE_STATE_PS_PARAMS_MISC;
            if (Value == FETCH4_ENABLE)
                WARN_ONCE("FETCH4 support is incomplete. Please report if buggy shadows.");
            return;
        }
    }

    if (unlikely(!nine_check_sampler_state_value(Type, Value)))
        return;

    context->samp[Sampler][Type] = Value;
    context->changed.group |= NINE_STATE_SAMPLER;
    context->changed.sampler[Sampler] |= 1 << Type;
}

CSMT_ITEM_NO_WAIT(nine_context_set_stream_source_apply,
                  ARG_VAL(UINT, StreamNumber),
                  ARG_BIND_RES(struct pipe_resource, res),
                  ARG_VAL(UINT, OffsetInBytes),
                  ARG_VAL(UINT, Stride))
{
    struct nine_context *context = &device->context;
    const unsigned i = StreamNumber;

    /* For normal draws, these tests are useless,
     * but not for *Up draws */
    if (context->vtxbuf[i].buffer.resource == res &&
        context->vtxbuf[i].buffer_offset == OffsetInBytes &&
        context->vtxstride[i] == Stride)
        return;

    if (context->vtxstride[i] != Stride) {
        context->vtxstride[i] = Stride;
        context->changed.group |= NINE_STATE_VDECL;
    }
    context->vtxbuf[i].buffer_offset = OffsetInBytes;
    pipe_resource_reference(&context->vtxbuf[i].buffer.resource, res);

    context->changed.vtxbuf |= 1 << StreamNumber;
    if (res)
        context->vtxbuf_mask |= 1 << StreamNumber;
    else
        context->vtxbuf_mask &= ~(1 << StreamNumber);
}

void
nine_context_set_stream_source(struct NineDevice9 *device,
                               UINT StreamNumber,
                               struct NineVertexBuffer9 *pVBuf9,
                               UINT OffsetInBytes,
                               UINT Stride)
{
    struct pipe_resource *res = NULL;
    unsigned offset = 0;

    if (pVBuf9)
        res = NineVertexBuffer9_GetResource(pVBuf9, &offset);
    /* in the future when there is internal offset, add it
     * to OffsetInBytes */

    nine_context_set_stream_source_apply(device, StreamNumber,
                                         res, offset + OffsetInBytes,
                                         Stride);
}

CSMT_ITEM_NO_WAIT(nine_context_set_stream_source_freq,
                  ARG_VAL(UINT, StreamNumber),
                  ARG_VAL(UINT, Setting))
{
    struct nine_context *context = &device->context;

    context->stream_freq[StreamNumber] = Setting;

    if (Setting & D3DSTREAMSOURCE_INSTANCEDATA)
        context->stream_instancedata_mask |= 1 << StreamNumber;
    else
        context->stream_instancedata_mask &= ~(1 << StreamNumber);

    if (StreamNumber != 0)
        context->changed.group |= NINE_STATE_STREAMFREQ;
}

CSMT_ITEM_NO_WAIT(nine_context_set_indices_apply,
                  ARG_BIND_RES(struct pipe_resource, res),
                  ARG_VAL(UINT, IndexSize),
                  ARG_VAL(UINT, OffsetInBytes))
{
    struct nine_context *context = &device->context;

    context->index_size = IndexSize;
    context->index_offset = OffsetInBytes;
    pipe_resource_reference(&context->idxbuf, res);

    context->changed.group |= NINE_STATE_IDXBUF;
}

void
nine_context_set_indices(struct NineDevice9 *device,
                         struct NineIndexBuffer9 *idxbuf)
{
    struct pipe_resource *res = NULL;
    UINT IndexSize = 0;
    unsigned OffsetInBytes = 0;

    if (idxbuf) {
        res = NineIndexBuffer9_GetBuffer(idxbuf, &OffsetInBytes);
        IndexSize = idxbuf->index_size;
    }

    nine_context_set_indices_apply(device, res, IndexSize, OffsetInBytes);
}

CSMT_ITEM_NO_WAIT(nine_context_set_vertex_declaration,
                  ARG_BIND_REF(struct NineVertexDeclaration9, vdecl))
{
    struct nine_context *context = &device->context;
    BOOL was_programmable_vs = context->programmable_vs;

    nine_bind(&context->vdecl, vdecl);

    context->programmable_vs = context->vs && !(context->vdecl && context->vdecl->position_t);
    if (was_programmable_vs != context->programmable_vs) {
        context->commit |= NINE_STATE_COMMIT_CONST_VS;
        context->changed.group |= NINE_STATE_VS;
    }

    context->changed.group |= NINE_STATE_VDECL;
}

CSMT_ITEM_NO_WAIT(nine_context_set_vertex_shader,
                  ARG_BIND_REF(struct NineVertexShader9, pShader))
{
    struct nine_context *context = &device->context;
    BOOL was_programmable_vs = context->programmable_vs;

    nine_bind(&context->vs, pShader);

    context->programmable_vs = context->vs && !(context->vdecl && context->vdecl->position_t);

    /* ff -> non-ff: commit back non-ff constants */
    if (!was_programmable_vs && context->programmable_vs)
        context->commit |= NINE_STATE_COMMIT_CONST_VS;

    context->changed.group |= NINE_STATE_VS;
}

CSMT_ITEM_NO_WAIT(nine_context_set_vertex_shader_constant_f,
                  ARG_VAL(UINT, StartRegister),
                  ARG_MEM(float, pConstantData),
                  ARG_MEM_SIZE(unsigned, pConstantData_size),
                  ARG_VAL(UINT, Vector4fCount))
{
    struct nine_context *context = &device->context;
    float *vs_const_f = device->may_swvp ? context->vs_const_f_swvp : context->vs_const_f;

    memcpy(&vs_const_f[StartRegister * 4],
           pConstantData,
           pConstantData_size);

    if (device->may_swvp) {
        Vector4fCount = MIN2(StartRegister + Vector4fCount, NINE_MAX_CONST_F) - StartRegister;
        if (StartRegister < NINE_MAX_CONST_F)
            memcpy(&context->vs_const_f[StartRegister * 4],
                   pConstantData,
                   Vector4fCount * 4 * sizeof(context->vs_const_f[0]));
    }

    context->changed.vs_const_f = true;
    context->changed.group |= NINE_STATE_VS_CONST;
}

CSMT_ITEM_NO_WAIT(nine_context_set_vertex_shader_constant_i,
                  ARG_VAL(UINT, StartRegister),
                  ARG_MEM(int, pConstantData),
                  ARG_MEM_SIZE(unsigned, pConstantData_size),
                  ARG_VAL(UINT, Vector4iCount))
{
    struct nine_context *context = &device->context;
    int i;

    if (device->driver_caps.vs_integer) {
        memcpy(&context->vs_const_i[4 * StartRegister],
               pConstantData,
               pConstantData_size);
    } else {
        for (i = 0; i < Vector4iCount; i++) {
            context->vs_const_i[4 * (StartRegister + i)] = fui((float)(pConstantData[4 * i]));
            context->vs_const_i[4 * (StartRegister + i) + 1] = fui((float)(pConstantData[4 * i + 1]));
            context->vs_const_i[4 * (StartRegister + i) + 2] = fui((float)(pConstantData[4 * i + 2]));
            context->vs_const_i[4 * (StartRegister + i) + 3] = fui((float)(pConstantData[4 * i + 3]));
        }
    }

    context->changed.vs_const_i = true;
    context->changed.group |= NINE_STATE_VS_CONST | NINE_STATE_VS_PARAMS_MISC;
}

CSMT_ITEM_NO_WAIT(nine_context_set_vertex_shader_constant_b,
                  ARG_VAL(UINT, StartRegister),
                  ARG_MEM(BOOL, pConstantData),
                  ARG_MEM_SIZE(unsigned, pConstantData_size),
                  ARG_VAL(UINT, BoolCount))
{
    struct nine_context *context = &device->context;
    int i;
    uint32_t bool_true = device->driver_caps.vs_integer ? 0xFFFFFFFF : fui(1.0f);

    (void) pConstantData_size;

    for (i = 0; i < BoolCount; i++)
        context->vs_const_b[StartRegister + i] = pConstantData[i] ? bool_true : 0;

    context->changed.vs_const_b = true;
    context->changed.group |= NINE_STATE_VS_CONST | NINE_STATE_VS_PARAMS_MISC;
}

CSMT_ITEM_NO_WAIT(nine_context_set_pixel_shader,
                  ARG_BIND_REF(struct NinePixelShader9, ps))
{
    struct nine_context *context = &device->context;
    unsigned old_mask = context->ps ? context->ps->rt_mask : 1;
    unsigned mask;

    /* ff -> non-ff: commit back non-ff constants */
    if (!context->ps && ps)
        context->commit |= NINE_STATE_COMMIT_CONST_PS;

    nine_bind(&context->ps, ps);

    context->changed.group |= NINE_STATE_PS;

    mask = context->ps ? context->ps->rt_mask : 1;
    /* We need to update cbufs if the pixel shader would
     * write to different render targets */
    if (mask != old_mask)
        context->changed.group |= NINE_STATE_FB;
}

CSMT_ITEM_NO_WAIT(nine_context_set_pixel_shader_constant_f,
                  ARG_VAL(UINT, StartRegister),
                  ARG_MEM(float, pConstantData),
                  ARG_MEM_SIZE(unsigned, pConstantData_size),
                  ARG_VAL(UINT, Vector4fCount))
{
    struct nine_context *context = &device->context;

    memcpy(&context->ps_const_f[StartRegister * 4],
           pConstantData,
           pConstantData_size);

    context->changed.ps_const_f = true;
    context->changed.group |= NINE_STATE_PS_CONST;
}

/* For stateblocks */
CSMT_ITEM_NO_WAIT(nine_context_set_pixel_shader_constant_i_transformed,
                  ARG_VAL(UINT, StartRegister),
                  ARG_MEM(int, pConstantData),
                  ARG_MEM_SIZE(unsigned, pConstantData_size),
                  ARG_VAL(UINT, Vector4iCount))
{
    struct nine_context *context = &device->context;

    memcpy(&context->ps_const_i[StartRegister][0],
           pConstantData,
           Vector4iCount * sizeof(context->ps_const_i[0]));

    context->changed.ps_const_i = true;
    context->changed.group |= NINE_STATE_PS_CONST | NINE_STATE_PS_PARAMS_MISC;
}

CSMT_ITEM_NO_WAIT(nine_context_set_pixel_shader_constant_i,
                  ARG_VAL(UINT, StartRegister),
                  ARG_MEM(int, pConstantData),
                  ARG_MEM_SIZE(unsigned, pConstantData_size),
                  ARG_VAL(UINT, Vector4iCount))
{
    struct nine_context *context = &device->context;
    int i;

    if (device->driver_caps.ps_integer) {
        memcpy(&context->ps_const_i[StartRegister][0],
               pConstantData,
               pConstantData_size);
    } else {
        for (i = 0; i < Vector4iCount; i++) {
            context->ps_const_i[StartRegister+i][0] = fui((float)(pConstantData[4*i]));
            context->ps_const_i[StartRegister+i][1] = fui((float)(pConstantData[4*i+1]));
            context->ps_const_i[StartRegister+i][2] = fui((float)(pConstantData[4*i+2]));
            context->ps_const_i[StartRegister+i][3] = fui((float)(pConstantData[4*i+3]));
        }
    }
    context->changed.ps_const_i = true;
    context->changed.group |= NINE_STATE_PS_CONST | NINE_STATE_PS_PARAMS_MISC;
}

CSMT_ITEM_NO_WAIT(nine_context_set_pixel_shader_constant_b,
                  ARG_VAL(UINT, StartRegister),
                  ARG_MEM(BOOL, pConstantData),
                  ARG_MEM_SIZE(unsigned, pConstantData_size),
                  ARG_VAL(UINT, BoolCount))
{
    struct nine_context *context = &device->context;
    int i;
    uint32_t bool_true = device->driver_caps.ps_integer ? 0xFFFFFFFF : fui(1.0f);

    (void) pConstantData_size;

    for (i = 0; i < BoolCount; i++)
        context->ps_const_b[StartRegister + i] = pConstantData[i] ? bool_true : 0;

    context->changed.ps_const_b = true;
    context->changed.group |= NINE_STATE_PS_CONST | NINE_STATE_PS_PARAMS_MISC;
}

/* XXX: use resource, as resource might change */
CSMT_ITEM_NO_WAIT(nine_context_set_render_target,
                  ARG_VAL(DWORD, RenderTargetIndex),
                  ARG_BIND_REF(struct NineSurface9, rt))
{
    struct nine_context *context = &device->context;
    const unsigned i = RenderTargetIndex;

    if (i == 0) {
        context->changed.group |= NINE_STATE_MULTISAMPLE;

        if (context->rt[0] &&
            (context->rt[0]->desc.MultiSampleType <= D3DMULTISAMPLE_NONMASKABLE) !=
            (rt->desc.MultiSampleType <= D3DMULTISAMPLE_NONMASKABLE))
            context->changed.group |= NINE_STATE_SAMPLE_MASK;
    }

    if (context->rt[i] != rt) {
       nine_bind(&context->rt[i], rt);
       context->changed.group |= NINE_STATE_FB;
    }
}

/* XXX: use resource instead of ds, as resource might change */
CSMT_ITEM_NO_WAIT(nine_context_set_depth_stencil,
                  ARG_BIND_REF(struct NineSurface9, ds))
{
    struct nine_context *context = &device->context;

    nine_bind(&context->ds, ds);
    context->changed.group |= NINE_STATE_FB;
}

CSMT_ITEM_NO_WAIT(nine_context_set_viewport,
                  ARG_COPY_REF(D3DVIEWPORT9, viewport))
{
    struct nine_context *context = &device->context;

    if (!memcmp(viewport, &context->viewport, sizeof(context->viewport)))
        return;

    context->viewport = *viewport;
    context->changed.group |= NINE_STATE_VIEWPORT;
}

CSMT_ITEM_NO_WAIT(nine_context_set_scissor,
                  ARG_COPY_REF(struct pipe_scissor_state, scissor))
{
    struct nine_context *context = &device->context;

    if (!memcmp(scissor, &context->scissor, sizeof(context->scissor)))
        return;

    context->scissor = *scissor;
    context->changed.group |= NINE_STATE_SCISSOR;
}

CSMT_ITEM_NO_WAIT(nine_context_set_transform,
                  ARG_VAL(D3DTRANSFORMSTATETYPE, State),
                  ARG_COPY_REF(D3DMATRIX, pMatrix))
{
    struct nine_context *context = &device->context;
    D3DMATRIX *M = nine_state_access_transform(&context->ff, State, true);

    *M = *pMatrix;
    if (State == D3DTS_PROJECTION) {
        BOOL prev_zfog = context->zfog;
        /* Pixel fog (with WFOG advertised): source is either Z or W.
         * W is the source if the projection matrix is not orthogonal.
         * Tests on Win 10 seem to indicate _34
         * and _33 are checked against 0, 1. */
        context->zfog = (M->_34 == 0.0f &&
                         M->_44 == 1.0f);
        if (context->zfog != prev_zfog)
            context->changed.group |= NINE_STATE_PS_PARAMS_MISC;
    }
    context->ff.changed.transform[State / 32] |= 1 << (State % 32);
    context->changed.group |= NINE_STATE_FF;
}

CSMT_ITEM_NO_WAIT(nine_context_set_material,
                  ARG_COPY_REF(D3DMATERIAL9, pMaterial))
{
    struct nine_context *context = &device->context;

    context->ff.material = *pMaterial;
    context->changed.group |= NINE_STATE_FF_MATERIAL;
}

CSMT_ITEM_NO_WAIT(nine_context_set_light,
                  ARG_VAL(DWORD, Index),
                  ARG_COPY_REF(D3DLIGHT9, pLight))
{
    struct nine_context *context = &device->context;

    (void)nine_state_set_light(&context->ff, Index, pLight);
    context->changed.group |= NINE_STATE_FF_LIGHTING;
}


/* For stateblocks */
static void
nine_context_light_enable_stateblock(struct NineDevice9 *device,
                                     const uint16_t active_light[NINE_MAX_LIGHTS_ACTIVE], /* TODO: use pointer that convey size for csmt */
                                     unsigned int num_lights_active)
{
    struct nine_context *context = &device->context;

    /* TODO: Use CSMT_* to avoid calling nine_csmt_process */
    nine_csmt_process(device);
    memcpy(context->ff.active_light, active_light, NINE_MAX_LIGHTS_ACTIVE * sizeof(context->ff.active_light[0]));
    context->ff.num_lights_active = num_lights_active;
    context->changed.group |= NINE_STATE_FF_LIGHTING;
}

CSMT_ITEM_NO_WAIT(nine_context_light_enable,
                  ARG_VAL(DWORD, Index),
                  ARG_VAL(BOOL, Enable))
{
    struct nine_context *context = &device->context;

    nine_state_light_enable(&context->ff, Index, Enable);
    context->changed.group |= NINE_STATE_FF_LIGHTING;
}

CSMT_ITEM_NO_WAIT(nine_context_set_texture_stage_state,
                  ARG_VAL(DWORD, Stage),
                  ARG_VAL(D3DTEXTURESTAGESTATETYPE, Type),
                  ARG_VAL(DWORD, Value))
{
    struct nine_context *context = &device->context;
    int bumpmap_index = -1;

    context->ff.tex_stage[Stage][Type] = Value;
    switch (Type) {
    case D3DTSS_BUMPENVMAT00:
        bumpmap_index = 4 * Stage;
        break;
    case D3DTSS_BUMPENVMAT01:
        bumpmap_index = 4 * Stage + 1;
        break;
    case D3DTSS_BUMPENVMAT10:
        bumpmap_index = 4 * Stage + 2;
        break;
    case D3DTSS_BUMPENVMAT11:
        bumpmap_index = 4 * Stage + 3;
        break;
    case D3DTSS_BUMPENVLSCALE:
        bumpmap_index = 4 * 8 + 2 * Stage;
        break;
    case D3DTSS_BUMPENVLOFFSET:
        bumpmap_index = 4 * 8 + 2 * Stage + 1;
        break;
    case D3DTSS_TEXTURETRANSFORMFLAGS:
        context->changed.group |= NINE_STATE_PS_PARAMS_MISC;
        break;
    default:
        break;
    }

    if (bumpmap_index >= 0) {
        context->bumpmap_vars[bumpmap_index] = Value;
        context->changed.group |= NINE_STATE_PS_CONST;
    }

    context->changed.group |= NINE_STATE_FF_PS_CONSTS;
    context->ff.changed.tex_stage[Stage][Type / 32] |= 1 << (Type % 32);
}

CSMT_ITEM_NO_WAIT(nine_context_set_clip_plane,
                  ARG_VAL(DWORD, Index),
                  ARG_COPY_REF(struct nine_clipplane, pPlane))
{
    struct nine_context *context = &device->context;

    memcpy(&context->clip.ucp[Index][0], pPlane, sizeof(context->clip.ucp[0]));
    if (!device->driver_caps.emulate_ucp)
        context->changed.ucp = true;
    else
        context->changed.group |= NINE_STATE_FF_VS_OTHER | NINE_STATE_VS_CONST;
}

CSMT_ITEM_NO_WAIT(nine_context_set_swvp,
                  ARG_VAL(bool, swvp))
{
    struct nine_context *context = &device->context;

    context->swvp = swvp;
    context->changed.group |= NINE_STATE_SWVP;
}

/* Do not write to nine_context directly. Slower,
 * but works with csmt. TODO: write a special csmt version that
 * would record the list of commands as much as possible,
 * and use the version above else.
 */
void
nine_context_apply_stateblock(struct NineDevice9 *device,
                              const struct nine_state *src)
{
    int i;

    /* No need to apply src->changed.group, since all calls do
    * set context->changed.group */

    for (i = 0; i < ARRAY_SIZE(src->changed.rs); ++i) {
        uint32_t m = src->changed.rs[i];
        while (m) {
            const int r = ffs(m) - 1;
            m &= ~(1 << r);
            nine_context_set_render_state(device, i * 32 + r, src->rs_advertised[i * 32 + r]);
        }
    }

    /* Textures */
    if (src->changed.texture) {
        uint32_t m = src->changed.texture;
        unsigned s;

        for (s = 0; m; ++s, m >>= 1) {
            struct NineBaseTexture9 *tex = src->texture[s];
            if (!(m & 1))
                continue;
            nine_context_set_texture(device, s, tex);
        }
    }

    /* Sampler state */
    if (src->changed.group & NINE_STATE_SAMPLER) {
        unsigned s;

        for (s = 0; s < NINE_MAX_SAMPLERS; ++s) {
            uint32_t m = src->changed.sampler[s];
            while (m) {
                const int i = ffs(m) - 1;
                m &= ~(1 << i);
                nine_context_set_sampler_state(device, s, i, src->samp_advertised[s][i]);
            }
        }
    }

    /* Vertex buffers */
    if (src->changed.vtxbuf | src->changed.stream_freq) {
        uint32_t m = src->changed.vtxbuf | src->changed.stream_freq;
        for (i = 0; m; ++i, m >>= 1) {
            if (src->changed.vtxbuf & (1 << i))
                nine_context_set_stream_source(device, i, src->stream[i], src->vtxbuf[i].buffer_offset, src->vtxstride[i]);
            if (src->changed.stream_freq & (1 << i))
                nine_context_set_stream_source_freq(device, i, src->stream_freq[i]);
        }
    }

    /* Index buffer */
    if (src->changed.group & NINE_STATE_IDXBUF)
        nine_context_set_indices(device, src->idxbuf);

    /* Vertex declaration */
    if ((src->changed.group & NINE_STATE_VDECL) && src->vdecl)
        nine_context_set_vertex_declaration(device, src->vdecl);

    /* Vertex shader */
    if (src->changed.group & NINE_STATE_VS)
        nine_context_set_vertex_shader(device, src->vs);

    /* Pixel shader */
    if (src->changed.group & NINE_STATE_PS)
        nine_context_set_pixel_shader(device, src->ps);

    /* Vertex constants */
    if (src->changed.group & NINE_STATE_VS_CONST) {
        struct nine_range *r;
        for (r = src->changed.vs_const_f; r; r = r->next)
            nine_context_set_vertex_shader_constant_f(device, r->bgn,
                                                      &src->vs_const_f[r->bgn * 4],
                                                      sizeof(float[4]) * (r->end - r->bgn),
                                                      r->end - r->bgn);
        for (r = src->changed.vs_const_i; r; r = r->next)
            nine_context_set_vertex_shader_constant_i(device, r->bgn,
                                                      &src->vs_const_i[r->bgn * 4],
                                                      sizeof(int[4]) * (r->end - r->bgn),
                                                      r->end - r->bgn);
        for (r = src->changed.vs_const_b; r; r = r->next)
            nine_context_set_vertex_shader_constant_b(device, r->bgn,
                                                      &src->vs_const_b[r->bgn * 4],
                                                      sizeof(BOOL) * (r->end - r->bgn),
                                                      r->end - r->bgn);
    }

    /* Pixel constants */
    if (src->changed.group & NINE_STATE_PS_CONST) {
        struct nine_range *r;
        for (r = src->changed.ps_const_f; r; r = r->next)
            nine_context_set_pixel_shader_constant_f(device, r->bgn,
                                                     &src->ps_const_f[r->bgn * 4],
                                                     sizeof(float[4]) * (r->end - r->bgn),
                                                     r->end - r->bgn);
        if (src->changed.ps_const_i) {
            uint16_t m = src->changed.ps_const_i;
            for (i = ffs(m) - 1, m >>= i; m; ++i, m >>= 1)
                if (m & 1)
                    nine_context_set_pixel_shader_constant_i_transformed(device, i,
                                                                         src->ps_const_i[i], sizeof(int[4]), 1);
        }
        if (src->changed.ps_const_b) {
            uint16_t m = src->changed.ps_const_b;
            for (i = ffs(m) - 1, m >>= i; m; ++i, m >>= 1)
                if (m & 1)
                    nine_context_set_pixel_shader_constant_b(device, i,
                                                             &src->ps_const_b[i], sizeof(BOOL), 1);
        }
    }

    /* Viewport */
    if (src->changed.group & NINE_STATE_VIEWPORT)
        nine_context_set_viewport(device, &src->viewport);

    /* Scissor */
    if (src->changed.group & NINE_STATE_SCISSOR)
        nine_context_set_scissor(device, &src->scissor);

    /* User Clip Planes */
    if (src->changed.ucp)
        for (i = 0; i < PIPE_MAX_CLIP_PLANES; ++i)
            if (src->changed.ucp & (1 << i))
                nine_context_set_clip_plane(device, i, (struct nine_clipplane*)&src->clip.ucp[i][0]);

    if (!(src->changed.group & NINE_STATE_FF))
        return;

    /* Fixed function state. */

    if (src->changed.group & NINE_STATE_FF_MATERIAL)
        nine_context_set_material(device, &src->ff.material);

    if (src->changed.group & NINE_STATE_FF_PS_CONSTS) {
        unsigned s;
        for (s = 0; s < NINE_MAX_TEXTURE_STAGES; ++s) {
            for (i = 0; i < NINED3DTSS_COUNT; ++i)
                if (src->ff.changed.tex_stage[s][i / 32] & (1 << (i % 32)))
                   nine_context_set_texture_stage_state(device, s, i, src->ff.tex_stage[s][i]);
        }
    }
    if (src->changed.group & NINE_STATE_FF_LIGHTING) {
        for (i = 0; i < src->ff.num_lights; ++i)
            if (src->ff.light[i].Type != NINED3DLIGHT_INVALID)
                nine_context_set_light(device, i, &src->ff.light[i]);

        nine_context_light_enable_stateblock(device, src->ff.active_light, src->ff.num_lights_active);
    }
    if (src->changed.group & NINE_STATE_FF_VSTRANSF) {
        for (i = 0; i < ARRAY_SIZE(src->ff.changed.transform); ++i) {
            unsigned s;
            if (!src->ff.changed.transform[i])
                continue;
            for (s = i * 32; s < (i * 32 + 32); ++s) {
                if (!(src->ff.changed.transform[i] & (1 << (s % 32))))
                    continue;
                /* MaxVertexBlendMatrixIndex is 8, which means
                 * we don't read past index D3DTS_WORLDMATRIX(8).
                 * swvp is supposed to allow all 256, but we don't
                 * implement it for now. */
                if (s > D3DTS_WORLDMATRIX(8))
                    break;
                nine_context_set_transform(device, s,
                                           nine_state_access_transform(
                                               (struct nine_ff_state *)&src->ff,
                                                                       s, false));
            }
        }
    }
}

static void
nine_update_state_framebuffer_clear(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;

    if (context->changed.group & NINE_STATE_FB)
        update_framebuffer(device, true);
}

CSMT_ITEM_NO_WAIT(nine_context_clear_fb,
                  ARG_VAL(DWORD, Count),
                  ARG_COPY_REF(D3DRECT, pRects),
                  ARG_VAL(DWORD, Flags),
                  ARG_VAL(D3DCOLOR, Color),
                  ARG_VAL(float, Z),
                  ARG_VAL(DWORD, Stencil))
{
    struct nine_context *context = &device->context;
    const int sRGB = context->rs[D3DRS_SRGBWRITEENABLE] ? 1 : 0;
    struct pipe_surface *cbuf, *zsbuf;
    struct pipe_context *pipe = context->pipe;
    struct NineSurface9 *zsbuf_surf = context->ds;
    struct NineSurface9 *rt;
    unsigned bufs = 0;
    unsigned r, i;
    union pipe_color_union rgba;
    unsigned rt_mask = 0;
    D3DRECT rect;

    nine_update_state_framebuffer_clear(device);

    if (Flags & D3DCLEAR_TARGET) bufs |= PIPE_CLEAR_COLOR;
    /* Ignore Z buffer if not bound */
    if (context->pipe_data.fb.zsbuf != NULL) {
        if (Flags & D3DCLEAR_ZBUFFER) bufs |= PIPE_CLEAR_DEPTH;
        if (Flags & D3DCLEAR_STENCIL) bufs |= PIPE_CLEAR_STENCIL;
    }
    if (!bufs)
        return;
    d3dcolor_to_pipe_color_union(&rgba, Color);

    rect.x1 = context->viewport.X;
    rect.y1 = context->viewport.Y;
    rect.x2 = context->viewport.Width + rect.x1;
    rect.y2 = context->viewport.Height + rect.y1;

    /* Both rectangles apply, which is weird, but that's D3D9. */
    if (context->rs[D3DRS_SCISSORTESTENABLE]) {
        rect.x1 = MAX2(rect.x1, context->scissor.minx);
        rect.y1 = MAX2(rect.y1, context->scissor.miny);
        rect.x2 = MIN2(rect.x2, context->scissor.maxx);
        rect.y2 = MIN2(rect.y2, context->scissor.maxy);
    }

    if (Count) {
        /* Maybe apps like to specify a large rect ? */
        if (pRects[0].x1 <= rect.x1 && pRects[0].x2 >= rect.x2 &&
            pRects[0].y1 <= rect.y1 && pRects[0].y2 >= rect.y2) {
            DBG("First rect covers viewport.\n");
            Count = 0;
            pRects = NULL;
        }
    }

    if (rect.x1 >= context->pipe_data.fb.width || rect.y1 >= context->pipe_data.fb.height)
        return;

    for (i = 0; i < device->caps.NumSimultaneousRTs; ++i) {
        if (context->rt[i] && context->rt[i]->desc.Format != D3DFMT_NULL)
            rt_mask |= 1 << i;
    }

    /* fast path, clears everything at once */
    if (!Count &&
        (!(bufs & PIPE_CLEAR_COLOR) || (rt_mask == context->rt_mask)) &&
        rect.x1 == 0 && rect.y1 == 0 &&
        /* Case we clear only render target. Check clear region vs rt. */
        ((!(bufs & (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL)) &&
         rect.x2 >= context->pipe_data.fb.width &&
         rect.y2 >= context->pipe_data.fb.height) ||
        /* Case we clear depth buffer (and eventually rt too).
         * depth buffer size is always >= rt size. Compare to clear region */
        ((bufs & (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL)) &&
         rect.x2 >= zsbuf_surf->desc.Width &&
         rect.y2 >= zsbuf_surf->desc.Height))) {
        DBG("Clear fast path\n");
        pipe->clear(pipe, bufs, NULL, &rgba, Z, Stencil);
        return;
    }

    if (!Count) {
        Count = 1;
        pRects = &rect;
    }

    for (i = 0; i < device->caps.NumSimultaneousRTs; ++i) {
        rt = context->rt[i];
        if (!rt || rt->desc.Format == D3DFMT_NULL ||
            !(bufs & PIPE_CLEAR_COLOR))
            continue; /* save space, compiler should hoist this */
        cbuf = NineSurface9_GetSurface(rt, sRGB);
        for (r = 0; r < Count; ++r) {
            /* Don't trust users to pass these in the right order. */
            unsigned x1 = MIN2(pRects[r].x1, pRects[r].x2);
            unsigned y1 = MIN2(pRects[r].y1, pRects[r].y2);
            unsigned x2 = MAX2(pRects[r].x1, pRects[r].x2);
            unsigned y2 = MAX2(pRects[r].y1, pRects[r].y2);
#ifndef NINE_LAX
            /* Drop negative rectangles (like wine expects). */
            if (pRects[r].x1 > pRects[r].x2) continue;
            if (pRects[r].y1 > pRects[r].y2) continue;
#endif

            x1 = MAX2(x1, rect.x1);
            y1 = MAX2(y1, rect.y1);
            x2 = MIN3(x2, rect.x2, rt->desc.Width);
            y2 = MIN3(y2, rect.y2, rt->desc.Height);

            DBG("Clearing (%u..%u)x(%u..%u)\n", x1, x2, y1, y2);
            pipe->clear_render_target(pipe, cbuf, &rgba,
                                      x1, y1, x2 - x1, y2 - y1, false);
        }
    }
    if (!(bufs & PIPE_CLEAR_DEPTHSTENCIL))
        return;

    bufs &= PIPE_CLEAR_DEPTHSTENCIL;

    for (r = 0; r < Count; ++r) {
        unsigned x1 = MIN2(pRects[r].x1, pRects[r].x2);
        unsigned y1 = MIN2(pRects[r].y1, pRects[r].y2);
        unsigned x2 = MAX2(pRects[r].x1, pRects[r].x2);
        unsigned y2 = MAX2(pRects[r].y1, pRects[r].y2);
#ifndef NINE_LAX
        /* Drop negative rectangles. */
        if (pRects[r].x1 > pRects[r].x2) continue;
        if (pRects[r].y1 > pRects[r].y2) continue;
#endif

        x1 = MIN2(x1, rect.x1);
        y1 = MIN2(y1, rect.y1);
        x2 = MIN3(x2, rect.x2, zsbuf_surf->desc.Width);
        y2 = MIN3(y2, rect.y2, zsbuf_surf->desc.Height);

        zsbuf = NineSurface9_GetSurface(zsbuf_surf, 0);
        assert(zsbuf);
        pipe->clear_depth_stencil(pipe, zsbuf, bufs, Z, Stencil,
                                  x1, y1, x2 - x1, y2 - y1, false);
    }
    return;
}


static inline void
init_draw_info(struct pipe_draw_info *info,
               struct pipe_draw_start_count_bias *draw,
               struct NineDevice9 *dev, D3DPRIMITIVETYPE type, UINT count)
{
    info->mode = d3dprimitivetype_to_pipe_prim(type);
    draw->count = prim_count_to_vertex_count(type, count);
    info->start_instance = 0;
    info->instance_count = 1;
    if (dev->context.stream_instancedata_mask & dev->context.stream_usage_mask)
        info->instance_count = MAX2(dev->context.stream_freq[0] & 0x7FFFFF, 1);
    info->primitive_restart = false;
    info->has_user_indices = false;
    info->take_index_buffer_ownership = false;
    info->index_bias_varies = false;
    info->increment_draw_id = false;
    info->was_line_loop = false;
    info->restart_index = 0;
    info->view_mask = 0;
}

CSMT_ITEM_NO_WAIT(nine_context_draw_primitive,
                  ARG_VAL(D3DPRIMITIVETYPE, PrimitiveType),
                  ARG_VAL(UINT, StartVertex),
                  ARG_VAL(UINT, PrimitiveCount))
{
    struct nine_context *context = &device->context;
    struct pipe_draw_info info;
    struct pipe_draw_start_count_bias draw;

    if (context->vs && context->vs->swvp_only && !context->swvp)
        return;

    nine_update_state(device);

    init_draw_info(&info, &draw, device, PrimitiveType, PrimitiveCount);
    info.index_size = 0;
    draw.start = StartVertex;
    draw.index_bias = 0;
    info.min_index = draw.start;
    info.max_index = draw.start + draw.count - 1;
    info.index.resource = NULL;

    context->pipe->draw_vbo(context->pipe, &info, 0, NULL, &draw, 1);
}

CSMT_ITEM_NO_WAIT(nine_context_draw_indexed_primitive,
                  ARG_VAL(D3DPRIMITIVETYPE, PrimitiveType),
                  ARG_VAL(INT, BaseVertexIndex),
                  ARG_VAL(UINT, MinVertexIndex),
                  ARG_VAL(UINT, NumVertices),
                  ARG_VAL(UINT, StartIndex),
                  ARG_VAL(UINT, PrimitiveCount))
{
    struct nine_context *context = &device->context;
    struct pipe_draw_info info;
    struct pipe_draw_start_count_bias draw;

    if (context->vs && context->vs->swvp_only && !context->swvp)
        return;

    nine_update_state(device);

    init_draw_info(&info, &draw, device, PrimitiveType, PrimitiveCount);
    info.index_size = context->index_size;
    draw.start = context->index_offset / context->index_size + StartIndex;
    draw.index_bias = BaseVertexIndex;
    info.index_bounds_valid = true;
    /* These don't include index bias: */
    info.min_index = MinVertexIndex;
    info.max_index = MinVertexIndex + NumVertices - 1;
    info.index.resource = context->idxbuf;

    context->pipe->draw_vbo(context->pipe, &info, 0, NULL, &draw, 1);
}

CSMT_ITEM_NO_WAIT(nine_context_draw_indexed_primitive_from_vtxbuf_idxbuf,
                  ARG_VAL(D3DPRIMITIVETYPE, PrimitiveType),
                  ARG_VAL(UINT, MinVertexIndex),
                  ARG_VAL(UINT, NumVertices),
                  ARG_VAL(UINT, PrimitiveCount),
                  ARG_VAL(UINT, vbuf_stride),
                  ARG_BIND_VBUF(struct pipe_vertex_buffer, vbuf),
                  ARG_BIND_RES(struct pipe_resource, ibuf),
                  ARG_VAL(void *, user_ibuf),
                  ARG_VAL(UINT, index_offset),
                  ARG_VAL(UINT, index_size))
{
    struct nine_context *context = &device->context;
    struct pipe_draw_info info;
    struct pipe_draw_start_count_bias draw;

    if (context->vs && context->vs->swvp_only && !context->swvp)
        return;

    if (context->vtxstride[0] != vbuf_stride) {
        context->vtxstride[0] = vbuf_stride;
        /* force elements update for stride.
         * We don't need to restore the old value,
         * as the caller set its to 0 after the call */
        context->changed.group |= NINE_STATE_VDECL;
    }

    nine_update_state(device);

    init_draw_info(&info, &draw, device, PrimitiveType, PrimitiveCount);
    info.index_size = index_size;
    draw.start = index_offset / info.index_size;
    draw.index_bias = 0;
    info.index_bounds_valid = true;
    info.min_index = MinVertexIndex;
    info.max_index = MinVertexIndex + NumVertices - 1;
    info.has_user_indices = ibuf == NULL;
    if (ibuf)
        info.index.resource = ibuf;
    else
        info.index.user = user_ibuf;

    context->pipe->set_vertex_buffers(context->pipe, 1, 0, false, vbuf);
    context->changed.vtxbuf |= 1;

    context->pipe->draw_vbo(context->pipe, &info, 0, NULL, &draw, 1);
}

CSMT_ITEM_NO_WAIT(nine_context_resource_copy_region,
                  ARG_BIND_REF(struct NineUnknown, dst),
                  ARG_BIND_REF(struct NineUnknown, src),
                  ARG_BIND_RES(struct pipe_resource, dst_res),
                  ARG_VAL(unsigned, dst_level),
                  ARG_COPY_REF(struct pipe_box, dst_box),
                  ARG_BIND_RES(struct pipe_resource, src_res),
                  ARG_VAL(unsigned, src_level),
                  ARG_COPY_REF(struct pipe_box, src_box))
{
    struct nine_context *context = &device->context;

    (void) dst;
    (void) src;

    context->pipe->resource_copy_region(context->pipe,
            dst_res, dst_level,
            dst_box->x, dst_box->y, dst_box->z,
            src_res, src_level,
            src_box);
}

CSMT_ITEM_NO_WAIT(nine_context_blit,
                  ARG_BIND_REF(struct NineUnknown, dst),
                  ARG_BIND_REF(struct NineUnknown, src),
                  ARG_BIND_BLIT(struct pipe_blit_info, blit))
{
    struct nine_context *context = &device->context;

    (void) dst;
    (void) src;

    context->pipe->blit(context->pipe, blit);
}

CSMT_ITEM_NO_WAIT(nine_context_clear_render_target,
                  ARG_BIND_REF(struct NineSurface9, surface),
                  ARG_VAL(D3DCOLOR, color),
                  ARG_VAL(UINT, x),
                  ARG_VAL(UINT, y),
                  ARG_VAL(UINT, width),
                  ARG_VAL(UINT, height))
{
    struct nine_context *context = &device->context;
    struct pipe_surface *surf;
    union pipe_color_union rgba;

    d3dcolor_to_pipe_color_union(&rgba, color);
    surf = NineSurface9_GetSurface(surface, 0);
    context->pipe->clear_render_target(context->pipe, surf, &rgba, x, y, width, height, false);
}

CSMT_ITEM_NO_WAIT(nine_context_gen_mipmap,
                  ARG_BIND_REF(struct NineUnknown, dst),
                  ARG_BIND_RES(struct pipe_resource, res),
                  ARG_VAL(UINT, base_level),
                  ARG_VAL(UINT, last_level),
                  ARG_VAL(UINT, first_layer),
                  ARG_VAL(UINT, last_layer),
                  ARG_VAL(UINT, filter))
{
    struct nine_context *context = &device->context;

    /* We just bind dst for the bind count */
    (void)dst;

    util_gen_mipmap(context->pipe, res, res->format, base_level,
                    last_level, first_layer, last_layer, filter);
}

CSMT_ITEM_NO_WAIT_WITH_COUNTER(nine_context_range_upload,
                               ARG_BIND_REF(struct NineUnknown, src_ref),
                               ARG_BIND_RES(struct pipe_resource, res),
                               ARG_VAL(unsigned, offset),
                               ARG_VAL(unsigned, size),
                               ARG_VAL(unsigned, usage),
                               ARG_VAL(const void *, data))
{
    struct nine_context *context = &device->context;

    /* Binding src_ref avoids release before upload */
    (void)src_ref;

    context->pipe->buffer_subdata(context->pipe, res, usage, offset, size, data);
}

CSMT_ITEM_NO_WAIT_WITH_COUNTER(nine_context_box_upload,
                               ARG_BIND_REF(struct NineUnknown, src_ref),
                               ARG_BIND_RES(struct pipe_resource, res),
                               ARG_VAL(unsigned, level),
                               ARG_COPY_REF(struct pipe_box, dst_box),
                               ARG_VAL(enum pipe_format, src_format),
                               ARG_VAL(const void *, src),
                               ARG_VAL(unsigned, src_stride),
                               ARG_VAL(unsigned, src_layer_stride),
                               ARG_COPY_REF(struct pipe_box, src_box))
{
    struct nine_context *context = &device->context;
    struct pipe_context *pipe = context->pipe;
    struct pipe_transfer *transfer = NULL;
    uint8_t *map;

    /* Binding src_ref avoids release before upload */
    (void)src_ref;

    if (is_ATI1_ATI2(src_format)) {
        const unsigned bw = util_format_get_blockwidth(src_format);
        const unsigned bh = util_format_get_blockheight(src_format);
        /* For these formats, the allocate surface can be too small to contain
         * a block. Yet we can be asked to upload such surfaces.
         * It is ok for these surfaces to have buggy content,
         * but we should avoid crashing.
         * Calling util_format_translate_3d would read out of bounds. */
        if (dst_box->width < bw || dst_box->height < bh)
            return;
    }

    map = pipe->texture_map(pipe,
                             res,
                             level,
                             PIPE_MAP_WRITE | PIPE_MAP_DISCARD_RANGE,
                             dst_box, &transfer);
    if (!map)
        return;

    /* Note: if formats are the sames, it will revert
     * to normal memcpy */
    (void) util_format_translate_3d(res->format,
                                    map, transfer->stride,
                                    transfer->layer_stride,
                                    0, 0, 0,
                                    src_format,
                                    src, src_stride,
                                    src_layer_stride,
                                    src_box->x, src_box->y, src_box->z,
                                    dst_box->width, dst_box->height,
                                    dst_box->depth);

    pipe_texture_unmap(pipe, transfer);
}

struct pipe_query *
nine_context_create_query(struct NineDevice9 *device, unsigned query_type)
{
    struct pipe_context *pipe;
    struct pipe_query *res;

    pipe = nine_context_get_pipe_acquire(device);
    res = pipe->create_query(pipe, query_type, 0);
    nine_context_get_pipe_release(device);
    return res;
}

CSMT_ITEM_DO_WAIT(nine_context_destroy_query,
                  ARG_REF(struct pipe_query, query))
{
    struct nine_context *context = &device->context;

    context->pipe->destroy_query(context->pipe, query);
}

CSMT_ITEM_NO_WAIT_WITH_COUNTER(nine_context_begin_query,
                               ARG_REF(struct pipe_query, query))
{
    struct nine_context *context = &device->context;

    (void) context->pipe->begin_query(context->pipe, query);
}

CSMT_ITEM_NO_WAIT_WITH_COUNTER(nine_context_end_query,
                               ARG_REF(struct pipe_query, query))
{
    struct nine_context *context = &device->context;

    (void) context->pipe->end_query(context->pipe, query);
}

bool
nine_context_get_query_result(struct NineDevice9 *device, struct pipe_query *query,
                              unsigned *counter, bool flush, bool wait,
                              union pipe_query_result *result)
{
    struct pipe_context *pipe;
    bool ret;

    if (wait)
        nine_csmt_process(device);
    else if (p_atomic_read(counter) > 0) {
        if (flush && device->csmt_active)
            nine_queue_flush(device->csmt_ctx->pool);
        DBG("Pending begin/end. Returning\n");
        return false;
    }

    pipe = nine_context_get_pipe_acquire(device);
    ret = pipe->get_query_result(pipe, query, wait, result);
    nine_context_get_pipe_release(device);

    DBG("Query result %s\n", ret ? "found" : "not yet available");
    return ret;
}

CSMT_ITEM_NO_WAIT(nine_context_pipe_flush)
{
    struct nine_context *context = &device->context;

    context->pipe->flush(context->pipe, NULL, PIPE_FLUSH_ASYNC);
}

/* State defaults */

static const DWORD nine_render_state_defaults[NINED3DRS_LAST + 1] =
{
 /* [D3DRS_ZENABLE] = D3DZB_TRUE; wine: auto_depth_stencil */
    [D3DRS_ZENABLE] = D3DZB_FALSE,
    [D3DRS_FILLMODE] = D3DFILL_SOLID,
    [D3DRS_SHADEMODE] = D3DSHADE_GOURAUD,
/*  [D3DRS_LINEPATTERN] = 0x00000000, */
    [D3DRS_ZWRITEENABLE] = true,
    [D3DRS_ALPHATESTENABLE] = false,
    [D3DRS_LASTPIXEL] = true,
    [D3DRS_SRCBLEND] = D3DBLEND_ONE,
    [D3DRS_DESTBLEND] = D3DBLEND_ZERO,
    [D3DRS_CULLMODE] = D3DCULL_CCW,
    [D3DRS_ZFUNC] = D3DCMP_LESSEQUAL,
    [D3DRS_ALPHAFUNC] = D3DCMP_ALWAYS,
    [D3DRS_ALPHAREF] = 0,
    [D3DRS_DITHERENABLE] = false,
    [D3DRS_ALPHABLENDENABLE] = false,
    [D3DRS_FOGENABLE] = false,
    [D3DRS_SPECULARENABLE] = false,
/*  [D3DRS_ZVISIBLE] = 0, */
    [D3DRS_FOGCOLOR] = 0,
    [D3DRS_FOGTABLEMODE] = D3DFOG_NONE,
    [D3DRS_FOGSTART] = 0x00000000,
    [D3DRS_FOGEND] = 0x3F800000,
    [D3DRS_FOGDENSITY] = 0x3F800000,
/*  [D3DRS_EDGEANTIALIAS] = FALSE, */
    [D3DRS_RANGEFOGENABLE] = false,
    [D3DRS_STENCILENABLE] = false,
    [D3DRS_STENCILFAIL] = D3DSTENCILOP_KEEP,
    [D3DRS_STENCILZFAIL] = D3DSTENCILOP_KEEP,
    [D3DRS_STENCILPASS] = D3DSTENCILOP_KEEP,
    [D3DRS_STENCILREF] = 0,
    [D3DRS_STENCILMASK] = 0xFFFFFFFF,
    [D3DRS_STENCILFUNC] = D3DCMP_ALWAYS,
    [D3DRS_STENCILWRITEMASK] = 0xFFFFFFFF,
    [D3DRS_TEXTUREFACTOR] = 0xFFFFFFFF,
    [D3DRS_WRAP0] = 0,
    [D3DRS_WRAP1] = 0,
    [D3DRS_WRAP2] = 0,
    [D3DRS_WRAP3] = 0,
    [D3DRS_WRAP4] = 0,
    [D3DRS_WRAP5] = 0,
    [D3DRS_WRAP6] = 0,
    [D3DRS_WRAP7] = 0,
    [D3DRS_CLIPPING] = true,
    [D3DRS_LIGHTING] = true,
    [D3DRS_AMBIENT] = 0,
    [D3DRS_FOGVERTEXMODE] = D3DFOG_NONE,
    [D3DRS_COLORVERTEX] = true,
    [D3DRS_LOCALVIEWER] = true,
    [D3DRS_NORMALIZENORMALS] = false,
    [D3DRS_DIFFUSEMATERIALSOURCE] = D3DMCS_COLOR1,
    [D3DRS_SPECULARMATERIALSOURCE] = D3DMCS_COLOR2,
    [D3DRS_AMBIENTMATERIALSOURCE] = D3DMCS_MATERIAL,
    [D3DRS_EMISSIVEMATERIALSOURCE] = D3DMCS_MATERIAL,
    [D3DRS_VERTEXBLEND] = D3DVBF_DISABLE,
    [D3DRS_CLIPPLANEENABLE] = 0,
/*  [D3DRS_SOFTWAREVERTEXPROCESSING] = FALSE, */
    [D3DRS_POINTSIZE] = 0x3F800000,
    [D3DRS_POINTSIZE_MIN] = 0x3F800000,
    [D3DRS_POINTSPRITEENABLE] = false,
    [D3DRS_POINTSCALEENABLE] = false,
    [D3DRS_POINTSCALE_A] = 0x3F800000,
    [D3DRS_POINTSCALE_B] = 0x00000000,
    [D3DRS_POINTSCALE_C] = 0x00000000,
    [D3DRS_MULTISAMPLEANTIALIAS] = true,
    [D3DRS_MULTISAMPLEMASK] = 0xFFFFFFFF,
    [D3DRS_PATCHEDGESTYLE] = D3DPATCHEDGE_DISCRETE,
/*  [D3DRS_PATCHSEGMENTS] = 0x3F800000, */
    [D3DRS_DEBUGMONITORTOKEN] = 0xDEADCAFE,
    [D3DRS_POINTSIZE_MAX] = 0x3F800000, /* depends on cap */
    [D3DRS_INDEXEDVERTEXBLENDENABLE] = false,
    [D3DRS_COLORWRITEENABLE] = 0x0000000f,
    [D3DRS_TWEENFACTOR] = 0x00000000,
    [D3DRS_BLENDOP] = D3DBLENDOP_ADD,
    [D3DRS_POSITIONDEGREE] = D3DDEGREE_CUBIC,
    [D3DRS_NORMALDEGREE] = D3DDEGREE_LINEAR,
    [D3DRS_SCISSORTESTENABLE] = false,
    [D3DRS_SLOPESCALEDEPTHBIAS] = 0,
    [D3DRS_MINTESSELLATIONLEVEL] = 0x3F800000,
    [D3DRS_MAXTESSELLATIONLEVEL] = 0x3F800000,
    [D3DRS_ANTIALIASEDLINEENABLE] = false,
    [D3DRS_ADAPTIVETESS_X] = 0x00000000,
    [D3DRS_ADAPTIVETESS_Y] = 0x00000000,
    [D3DRS_ADAPTIVETESS_Z] = 0x3F800000,
    [D3DRS_ADAPTIVETESS_W] = 0x00000000,
    [D3DRS_ENABLEADAPTIVETESSELLATION] = false,
    [D3DRS_TWOSIDEDSTENCILMODE] = false,
    [D3DRS_CCW_STENCILFAIL] = D3DSTENCILOP_KEEP,
    [D3DRS_CCW_STENCILZFAIL] = D3DSTENCILOP_KEEP,
    [D3DRS_CCW_STENCILPASS] = D3DSTENCILOP_KEEP,
    [D3DRS_CCW_STENCILFUNC] = D3DCMP_ALWAYS,
    [D3DRS_COLORWRITEENABLE1] = 0x0000000F,
    [D3DRS_COLORWRITEENABLE2] = 0x0000000F,
    [D3DRS_COLORWRITEENABLE3] = 0x0000000F,
    [D3DRS_BLENDFACTOR] = 0xFFFFFFFF,
    [D3DRS_SRGBWRITEENABLE] = 0,
    [D3DRS_DEPTHBIAS] = 0,
    [D3DRS_WRAP8] = 0,
    [D3DRS_WRAP9] = 0,
    [D3DRS_WRAP10] = 0,
    [D3DRS_WRAP11] = 0,
    [D3DRS_WRAP12] = 0,
    [D3DRS_WRAP13] = 0,
    [D3DRS_WRAP14] = 0,
    [D3DRS_WRAP15] = 0,
    [D3DRS_SEPARATEALPHABLENDENABLE] = false,
    [D3DRS_SRCBLENDALPHA] = D3DBLEND_ONE,
    [D3DRS_DESTBLENDALPHA] = D3DBLEND_ZERO,
    [D3DRS_BLENDOPALPHA] = D3DBLENDOP_ADD,
    [NINED3DRS_VSPOINTSIZE] = false,
    [NINED3DRS_RTMASK] = 0xf,
    [NINED3DRS_ALPHACOVERAGE] = false,
    [NINED3DRS_MULTISAMPLE] = false,
    [NINED3DRS_FETCH4] = 0,
    [NINED3DRS_EMULATED_ALPHATEST] = 7 /* ALWAYS pass */
};
static const DWORD nine_tex_stage_state_defaults[NINED3DTSS_LAST + 1] =
{
    [D3DTSS_COLOROP] = D3DTOP_DISABLE,
    [D3DTSS_ALPHAOP] = D3DTOP_DISABLE,
    [D3DTSS_COLORARG1] = D3DTA_TEXTURE,
    [D3DTSS_COLORARG2] = D3DTA_CURRENT,
    [D3DTSS_COLORARG0] = D3DTA_CURRENT,
    [D3DTSS_ALPHAARG1] = D3DTA_TEXTURE,
    [D3DTSS_ALPHAARG2] = D3DTA_CURRENT,
    [D3DTSS_ALPHAARG0] = D3DTA_CURRENT,
    [D3DTSS_RESULTARG] = D3DTA_CURRENT,
    [D3DTSS_BUMPENVMAT00] = 0,
    [D3DTSS_BUMPENVMAT01] = 0,
    [D3DTSS_BUMPENVMAT10] = 0,
    [D3DTSS_BUMPENVMAT11] = 0,
    [D3DTSS_BUMPENVLSCALE] = 0,
    [D3DTSS_BUMPENVLOFFSET] = 0,
    [D3DTSS_TEXCOORDINDEX] = 0,
    [D3DTSS_TEXTURETRANSFORMFLAGS] = D3DTTFF_DISABLE,
};
static const DWORD nine_samp_state_defaults[NINED3DSAMP_LAST + 1] =
{
    [D3DSAMP_ADDRESSU] = D3DTADDRESS_WRAP,
    [D3DSAMP_ADDRESSV] = D3DTADDRESS_WRAP,
    [D3DSAMP_ADDRESSW] = D3DTADDRESS_WRAP,
    [D3DSAMP_BORDERCOLOR] = 0,
    [D3DSAMP_MAGFILTER] = D3DTEXF_POINT,
    [D3DSAMP_MINFILTER] = D3DTEXF_POINT,
    [D3DSAMP_MIPFILTER] = D3DTEXF_NONE,
    [D3DSAMP_MIPMAPLODBIAS] = 0,
    [D3DSAMP_MAXMIPLEVEL] = 0,
    [D3DSAMP_MAXANISOTROPY] = 1,
    [D3DSAMP_SRGBTEXTURE] = 0,
    [D3DSAMP_ELEMENTINDEX] = 0,
    [D3DSAMP_DMAPOFFSET] = 0,
    [NINED3DSAMP_MINLOD] = 0,
    [NINED3DSAMP_SHADOW] = 0,
    [NINED3DSAMP_CUBETEX] = 0
};

/* Note: The following 4 functions assume there is no
 * pending commands */

void nine_state_restore_non_cso(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;

    context->changed.group = NINE_STATE_ALL; /* TODO: we can remove states that have prepared commits */
    context->changed.vtxbuf = (1ULL << device->caps.MaxStreams) - 1;
    context->changed.ucp = true;
    context->commit |= 0xffffffff; /* re-commit everything */
    context->enabled_sampler_count_vs = 0;
    context->enabled_sampler_count_ps = 0;
}

void
nine_state_set_defaults(struct NineDevice9 *device, const D3DCAPS9 *caps,
                        bool is_reset)
{
    struct nine_state *state = &device->state;
    struct nine_context *context = &device->context;
    unsigned s;

    /* Initialize defaults.
     */
    memcpy(context->rs, nine_render_state_defaults, sizeof(context->rs));

    for (s = 0; s < ARRAY_SIZE(state->ff.tex_stage); ++s) {
        memcpy(&state->ff.tex_stage[s], nine_tex_stage_state_defaults,
               sizeof(state->ff.tex_stage[s]));
        state->ff.tex_stage[s][D3DTSS_TEXCOORDINDEX] = s;
    }
    state->ff.tex_stage[0][D3DTSS_COLOROP] = D3DTOP_MODULATE;
    state->ff.tex_stage[0][D3DTSS_ALPHAOP] = D3DTOP_SELECTARG1;

    for (s = 0; s < ARRAY_SIZE(state->ff.tex_stage); ++s)
        memcpy(&context->ff.tex_stage[s], state->ff.tex_stage[s],
               sizeof(state->ff.tex_stage[s]));

    memset(&context->bumpmap_vars, 0, sizeof(context->bumpmap_vars));

    for (s = 0; s < NINE_MAX_SAMPLERS; ++s) {
        memcpy(&context->samp[s], nine_samp_state_defaults,
               sizeof(context->samp[s]));
        memcpy(&state->samp_advertised[s], nine_samp_state_defaults,
               sizeof(state->samp_advertised[s]));
    }

    memset(state->vs_const_f, 0, VS_CONST_F_SIZE(device));
    memset(context->vs_const_f, 0, device->vs_const_size);
    if (context->vs_const_f_swvp)
        memset(context->vs_const_f_swvp, 0, NINE_MAX_CONST_F_SWVP * sizeof(float[4]));
    memset(state->vs_const_i, 0, VS_CONST_I_SIZE(device));
    memset(context->vs_const_i, 0, VS_CONST_I_SIZE(device));
    memset(state->vs_const_b, 0, VS_CONST_B_SIZE(device));
    memset(context->vs_const_b, 0, VS_CONST_B_SIZE(device));
    memset(state->ps_const_f, 0, device->ps_const_size);
    memset(context->ps_const_f, 0, device->ps_const_size);
    memset(state->ps_const_i, 0, sizeof(state->ps_const_i));
    memset(context->ps_const_i, 0, sizeof(context->ps_const_i));
    memset(state->ps_const_b, 0, sizeof(state->ps_const_b));
    memset(context->ps_const_b, 0, sizeof(context->ps_const_b));
    context->zfog = false; /* Guess from wine tests: both true or false are ok */

    /* Cap dependent initial state:
     */
    context->rs[D3DRS_POINTSIZE_MAX] = fui(caps->MaxPointSize);

    memcpy(state->rs_advertised, context->rs, sizeof(context->rs));

    /* Set changed flags to initialize driver.
     */
    context->changed.group = NINE_STATE_ALL;
    context->changed.vtxbuf = (1ULL << device->caps.MaxStreams) - 1;
    context->changed.ucp = true;

    context->ff.changed.transform[0] = ~0;
    context->ff.changed.transform[D3DTS_WORLD / 32] |= 1 << (D3DTS_WORLD % 32);

    if (!is_reset) {
        state->viewport.MinZ = context->viewport.MinZ = 0.0f;
        state->viewport.MaxZ = context->viewport.MaxZ = 1.0f;
    }

    for (s = 0; s < NINE_MAX_SAMPLERS; ++s)
        context->changed.sampler[s] = ~0;

    if (!is_reset)
        context->dummy_vbo_bound_at = -1;
}

void
nine_device_state_clear(struct NineDevice9 *device)
{
    struct nine_state *state = &device->state;
    unsigned i;

    for (i = 0; i < ARRAY_SIZE(state->rt); ++i)
       nine_bind(&state->rt[i], NULL);
    nine_bind(&state->ds, NULL);
    nine_bind(&state->vs, NULL);
    nine_bind(&state->ps, NULL);
    nine_bind(&state->vdecl, NULL);
    for (i = 0; i < PIPE_MAX_ATTRIBS; ++i)
        NineBindBufferToDevice(device,
                               (struct NineBuffer9 **)&state->stream[i],
                               NULL);
    NineBindBufferToDevice(device,
                           (struct NineBuffer9 **)&state->idxbuf,
                           NULL);

    for (i = 0; i < NINE_MAX_SAMPLERS; ++i)
        NineBindTextureToDevice(device, &state->texture[i], NULL);
}

void
nine_context_clear(struct NineDevice9 *device)
{
    struct nine_context *context = &device->context;
    struct pipe_context *pipe = context->pipe;
    struct cso_context *cso = context->cso;
    unsigned i;

    /* Early device ctor failure. Nothing to do */
    if (!pipe || !cso)
        return;

    context->vtxbuf_mask = 0;

    pipe->bind_vs_state(pipe, NULL);
    pipe->bind_fs_state(pipe, NULL);

    /* Don't unbind constant buffers, they're device-private and
     * do not change on Reset.
     */

    cso_set_samplers(cso, PIPE_SHADER_VERTEX, 0, NULL);
    cso_set_samplers(cso, PIPE_SHADER_FRAGMENT, 0, NULL);
    context->enabled_sampler_count_vs = 0;
    context->enabled_sampler_count_ps = 0;

    pipe->set_sampler_views(pipe, PIPE_SHADER_VERTEX, 0, 0,
                            NINE_MAX_SAMPLERS_VS, false, NULL);
    pipe->set_sampler_views(pipe, PIPE_SHADER_FRAGMENT, 0, 0,
                            NINE_MAX_SAMPLERS_PS, false, NULL);

    pipe->set_vertex_buffers(pipe, 0, device->caps.MaxStreams, false, NULL);

    for (i = 0; i < ARRAY_SIZE(context->rt); ++i)
       nine_bind(&context->rt[i], NULL);
    nine_bind(&context->ds, NULL);
    nine_bind(&context->vs, NULL);
    nine_bind(&context->ps, NULL);
    nine_bind(&context->vdecl, NULL);
    for (i = 0; i < PIPE_MAX_ATTRIBS; ++i)
        pipe_vertex_buffer_unreference(&context->vtxbuf[i]);
    pipe_resource_reference(&context->idxbuf, NULL);
    pipe_resource_reference(&context->pipe_data.cb_vs.buffer, NULL);
    pipe_resource_reference(&context->pipe_data.cb_ps.buffer, NULL);

    for (i = 0; i < NINE_MAX_SAMPLERS; ++i) {
        context->texture[i].enabled = false;
        pipe_resource_reference(&context->texture[i].resource,
                                NULL);
        pipe_sampler_view_reference(&context->texture[i].view[0],
                                    NULL);
        pipe_sampler_view_reference(&context->texture[i].view[1],
                                    NULL);
    }
}

void
nine_context_update_state(struct NineDevice9 *device)
{
    nine_update_state(device);
}

void
nine_state_init_sw(struct NineDevice9 *device)
{
    struct pipe_context *pipe_sw = device->pipe_sw;
    struct pipe_rasterizer_state rast;
    struct pipe_blend_state blend;
    struct pipe_depth_stencil_alpha_state dsa;
    struct pipe_framebuffer_state fb;

    /* Only used with Streamout */
    memset(&rast, 0, sizeof(rast));
    rast.rasterizer_discard = true;
    rast.point_quad_rasterization = 1; /* to make llvmpipe happy */
    cso_set_rasterizer(device->cso_sw, &rast);

    /* dummy settings */
    memset(&blend, 0, sizeof(blend));
    memset(&dsa, 0, sizeof(dsa));
    memset(&fb, 0, sizeof(fb));
    cso_set_blend(device->cso_sw, &blend);
    cso_set_depth_stencil_alpha(device->cso_sw, &dsa);
    cso_set_framebuffer(device->cso_sw, &fb);
    cso_set_viewport_dims(device->cso_sw, 1.0, 1.0, false);
    cso_set_fragment_shader_handle(device->cso_sw, util_make_empty_fragment_shader(pipe_sw));
}

/* There is duplication with update_vertex_elements.
 * TODO: Share the code */

static int
update_vertex_elements_sw(struct NineDevice9 *device)
{
    struct nine_state *state = &device->state;
    const struct NineVertexDeclaration9 *vdecl = device->state.vdecl;
    const struct NineVertexShader9 *vs;
    unsigned n, b, i, j;
    int index;
    int8_t vdecl_index_map[16]; /* vs->num_inputs <= 16 */
    int8_t used_streams[device->caps.MaxStreams];
    BOOL need_dummy_vbo = false;
    int dummy_vbo_stream = -1;
    struct cso_velems_state ve;
    bool programmable_vs = state->vs && !(state->vdecl && state->vdecl->position_t);
    unsigned vtxbuf_holes_map[PIPE_MAX_ATTRIBS];

    memset(vdecl_index_map, -1, 16);
    memset(used_streams, 0, device->caps.MaxStreams);
    vs = programmable_vs ? device->state.vs : device->ff.vs;

    if (vdecl) {
        for (n = 0; n < vs->num_inputs; ++n) {
            DBG("looking up input %u (usage %u) from vdecl(%p)\n",
                n, vs->input_map[n].ndecl, vdecl);

            for (i = 0; i < vdecl->nelems; i++) {
                if (vdecl->usage_map[i] == vs->input_map[n].ndecl) {
                    vdecl_index_map[n] = i;
                    used_streams[vdecl->elems[i].vertex_buffer_index] = 1;
                    break;
                }
            }
            if (vdecl_index_map[n] < 0)
                need_dummy_vbo = true;
        }
    } else {
        /* No vertex declaration. Likely will never happen in practice,
         * but we need not crash on this */
        need_dummy_vbo = true;
    }

    if (need_dummy_vbo) {
        for (i = 0; i < device->caps.MaxStreams; i++) {
            if (!used_streams[i]) {
                dummy_vbo_stream = i;
                break;
            }
        }
    }
    /* there are less vertex shader inputs than stream slots,
     * so if we need a slot for the dummy vbo, we should have found one */
    assert (!need_dummy_vbo || dummy_vbo_stream != -1);

    /* calculate vtxbuf_holes_map to match for update_vertex_buffers_sw() function */
    j = 0;
    for (i = 0; i < device->caps.MaxStreams; i++) {
        if (state->stream[i] || (need_dummy_vbo && dummy_vbo_stream == i)) {
            vtxbuf_holes_map[i] = j;
            j++;
        }
    }

    for (n = 0; n < vs->num_inputs; ++n) {
        index = vdecl_index_map[n];
        if (index >= 0) {
            ve.velems[n] = vdecl->elems[index];
            ve.velems[n].vertex_buffer_index =
                vtxbuf_holes_map[ve.velems[n].vertex_buffer_index];
            b = ve.velems[n].vertex_buffer_index;
            /* XXX wine just uses 1 here: */
            if (state->stream_freq[b] & D3DSTREAMSOURCE_INSTANCEDATA)
                ve.velems[n].instance_divisor = state->stream_freq[b] & 0x7FFFFF;
        } else {
            /* if the vertex declaration is incomplete compared to what the
             * vertex shader needs, we bind a dummy vbo with 0 0 0 0.
             * This is not precised by the spec, but is the behaviour
             * tested on win */
            ve.velems[n].vertex_buffer_index = vtxbuf_holes_map[dummy_vbo_stream];
            ve.velems[n].src_format = PIPE_FORMAT_R32G32B32A32_FLOAT;
            ve.velems[n].src_offset = 0;
            ve.velems[n].instance_divisor = 0;
            ve.velems[n].dual_slot = false;
        }
    }

    ve.count = vs->num_inputs;
    cso_set_vertex_elements(device->cso_sw, &ve);
    return dummy_vbo_stream;
}

static void
update_vertex_buffers_sw(struct NineDevice9 *device, int dummy_vbo_stream,
                         int start_vertice, int num_vertices)
{
    struct pipe_context *pipe = nine_context_get_pipe_acquire(device);
    struct pipe_context *pipe_sw = device->pipe_sw;
    struct nine_state *state = &device->state;
    struct nine_state_sw_internal *sw_internal = &device->state_sw_internal;
    struct pipe_vertex_buffer vbuffer[PIPE_MAX_ATTRIBS];
    unsigned vtxbuf_count = 0;
    unsigned i, j = 0;

    for (i = 0; i < device->caps.MaxStreams; i++) {
        if (dummy_vbo_stream == i) {
            vbuffer[j].buffer.resource = NULL;
            pipe_resource_reference(&vbuffer[j].buffer.resource, device->dummy_vbo_sw);
            vbuffer[j].is_user_buffer = false;
            vbuffer[j].buffer_offset = 0;
            j++;
        } else if (state->stream[i]) {
            unsigned offset;
            struct pipe_resource *buf;
            struct pipe_box box;
            void *userbuf;

            vbuffer[j] = state->vtxbuf[i];
            buf = NineVertexBuffer9_GetResource(state->stream[i], &offset);

            DBG("Locking %p (offset %d, length %d)\n", buf,
                vbuffer[j].buffer_offset, num_vertices * state->vtxstride[i]);

            u_box_1d(vbuffer[j].buffer_offset + offset + start_vertice *
                     state->vtxstride[i], num_vertices * state->vtxstride[i], &box);

            userbuf = pipe->buffer_map(pipe, buf, 0, PIPE_MAP_READ, &box,
                                       &(sw_internal->transfers_so[i]));
            vbuffer[j].is_user_buffer = true;
            vbuffer[j].buffer.user = userbuf;

            if (!device->driver_caps.user_sw_vbufs) {
                vbuffer[j].buffer.resource = NULL;
                vbuffer[j].is_user_buffer = false;
                u_upload_data(device->pipe_sw->stream_uploader,
                              0,
                              box.width,
                              16,
                              userbuf,
                              &(vbuffer[j].buffer_offset),
                              &(vbuffer[j].buffer.resource));
                u_upload_unmap(device->pipe_sw->stream_uploader);
            }
            j++;
        }
    }

    vtxbuf_count = j;
    pipe_sw->set_vertex_buffers(pipe_sw, vtxbuf_count, device->caps.MaxStreams-vtxbuf_count, true, vbuffer);

    nine_context_get_pipe_release(device);
}

static void
update_vs_constants_sw(struct NineDevice9 *device)
{
    struct nine_state *state = &device->state;
    struct pipe_context *pipe_sw = device->pipe_sw;

    DBG("updating\n");

    {
        struct pipe_constant_buffer cb;
        const void *buf;

        cb.buffer = NULL;
        cb.buffer_offset = 0;
        cb.buffer_size = 4096 * sizeof(float[4]);
        cb.user_buffer = state->vs_const_f;

        if (state->vs->lconstf.ranges) {
            const struct nine_lconstf *lconstf =  &device->state.vs->lconstf;
            const struct nine_range *r = lconstf->ranges;
            unsigned n = 0;
            float *dst = device->state.vs_lconstf_temp;
            float *src = (float *)cb.user_buffer;
            memcpy(dst, src, 8192 * sizeof(float[4]));
            while (r) {
                unsigned p = r->bgn;
                unsigned c = r->end - r->bgn;
                memcpy(&dst[p * 4], &lconstf->data[n * 4], c * 4 * sizeof(float));
                n += c;
                r = r->next;
            }
            cb.user_buffer = dst;
        }

        buf = cb.user_buffer;

        pipe_sw->set_constant_buffer(pipe_sw, PIPE_SHADER_VERTEX, 0, false, &cb);
        if (cb.buffer)
            pipe_resource_reference(&cb.buffer, NULL);

        cb.user_buffer = (int8_t *)buf + 4096 * sizeof(float[4]);

        pipe_sw->set_constant_buffer(pipe_sw, PIPE_SHADER_VERTEX, 1, false, &cb);
        if (cb.buffer)
            pipe_resource_reference(&cb.buffer, NULL);
    }

    {
        struct pipe_constant_buffer cb;

        cb.buffer = NULL;
        cb.buffer_offset = 0;
        cb.buffer_size = 2048 * sizeof(float[4]);
        cb.user_buffer = state->vs_const_i;

        pipe_sw->set_constant_buffer(pipe_sw, PIPE_SHADER_VERTEX, 2, false, &cb);
        if (cb.buffer)
            pipe_resource_reference(&cb.buffer, NULL);
    }

    {
        struct pipe_constant_buffer cb;

        cb.buffer = NULL;
        cb.buffer_offset = 0;
        cb.buffer_size = 512 * sizeof(float[4]);
        cb.user_buffer = state->vs_const_b;

        pipe_sw->set_constant_buffer(pipe_sw, PIPE_SHADER_VERTEX, 3, false, &cb);
        if (cb.buffer)
            pipe_resource_reference(&cb.buffer, NULL);
    }

    {
        struct pipe_constant_buffer cb;
        const D3DVIEWPORT9 *vport = &device->state.viewport;
        float viewport_data[8] = {(float)vport->Width * 0.5f,
            (float)vport->Height * -0.5f, vport->MaxZ - vport->MinZ, 0.f,
            (float)vport->Width * 0.5f + (float)vport->X,
            (float)vport->Height * 0.5f + (float)vport->Y,
            vport->MinZ, 0.f};

        cb.buffer = NULL;
        cb.buffer_offset = 0;
        cb.buffer_size = 2 * sizeof(float[4]);
        cb.user_buffer = viewport_data;

        {
            u_upload_data(device->pipe_sw->const_uploader,
                          0,
                          cb.buffer_size,
                          16,
                          cb.user_buffer,
                          &(cb.buffer_offset),
                          &(cb.buffer));
            u_upload_unmap(device->pipe_sw->const_uploader);
            cb.user_buffer = NULL;
        }

        pipe_sw->set_constant_buffer(pipe_sw, PIPE_SHADER_VERTEX, 4, false, &cb);
        if (cb.buffer)
            pipe_resource_reference(&cb.buffer, NULL);
    }

}

void
nine_state_prepare_draw_sw(struct NineDevice9 *device, struct NineVertexDeclaration9 *vdecl_out,
                           int start_vertice, int num_vertices, struct pipe_stream_output_info *so)
{
    struct nine_state *state = &device->state;
    bool programmable_vs = state->vs && !(state->vdecl && state->vdecl->position_t);
    struct NineVertexShader9 *vs = programmable_vs ? device->state.vs : device->ff.vs;
    int dummy_vbo_stream;

    assert(programmable_vs);

    DBG("Preparing draw\n");
    cso_set_vertex_shader_handle(device->cso_sw,
                                 NineVertexShader9_GetVariantProcessVertices(vs, vdecl_out, so));
    dummy_vbo_stream = update_vertex_elements_sw(device);
    update_vertex_buffers_sw(device, dummy_vbo_stream, start_vertice, num_vertices);
    update_vs_constants_sw(device);
    DBG("Preparation succeeded\n");
}

void
nine_state_after_draw_sw(struct NineDevice9 *device)
{
    struct nine_state_sw_internal *sw_internal = &device->state_sw_internal;
    struct pipe_context *pipe = nine_context_get_pipe_acquire(device);
    struct pipe_context *pipe_sw = device->pipe_sw;
    int i;

    pipe_sw->set_vertex_buffers(pipe_sw, 0, device->caps.MaxStreams, false, NULL);
    for (i = 0; i < 4; i++) {
        if (sw_internal->transfers_so[i])
            pipe->buffer_unmap(pipe, sw_internal->transfers_so[i]);
        sw_internal->transfers_so[i] = NULL;
    }
    nine_context_get_pipe_release(device);
}

void
nine_state_destroy_sw(struct NineDevice9 *device)
{
    (void) device;
    /* Everything destroyed with cso */
}

/*
static const DWORD nine_render_states_pixel[] =
{
    D3DRS_ALPHABLENDENABLE,
    D3DRS_ALPHAFUNC,
    D3DRS_ALPHAREF,
    D3DRS_ALPHATESTENABLE,
    D3DRS_ANTIALIASEDLINEENABLE,
    D3DRS_BLENDFACTOR,
    D3DRS_BLENDOP,
    D3DRS_BLENDOPALPHA,
    D3DRS_CCW_STENCILFAIL,
    D3DRS_CCW_STENCILPASS,
    D3DRS_CCW_STENCILZFAIL,
    D3DRS_COLORWRITEENABLE,
    D3DRS_COLORWRITEENABLE1,
    D3DRS_COLORWRITEENABLE2,
    D3DRS_COLORWRITEENABLE3,
    D3DRS_DEPTHBIAS,
    D3DRS_DESTBLEND,
    D3DRS_DESTBLENDALPHA,
    D3DRS_DITHERENABLE,
    D3DRS_FILLMODE,
    D3DRS_FOGDENSITY,
    D3DRS_FOGEND,
    D3DRS_FOGSTART,
    D3DRS_LASTPIXEL,
    D3DRS_SCISSORTESTENABLE,
    D3DRS_SEPARATEALPHABLENDENABLE,
    D3DRS_SHADEMODE,
    D3DRS_SLOPESCALEDEPTHBIAS,
    D3DRS_SRCBLEND,
    D3DRS_SRCBLENDALPHA,
    D3DRS_SRGBWRITEENABLE,
    D3DRS_STENCILENABLE,
    D3DRS_STENCILFAIL,
    D3DRS_STENCILFUNC,
    D3DRS_STENCILMASK,
    D3DRS_STENCILPASS,
    D3DRS_STENCILREF,
    D3DRS_STENCILWRITEMASK,
    D3DRS_STENCILZFAIL,
    D3DRS_TEXTUREFACTOR,
    D3DRS_TWOSIDEDSTENCILMODE,
    D3DRS_WRAP0,
    D3DRS_WRAP1,
    D3DRS_WRAP10,
    D3DRS_WRAP11,
    D3DRS_WRAP12,
    D3DRS_WRAP13,
    D3DRS_WRAP14,
    D3DRS_WRAP15,
    D3DRS_WRAP2,
    D3DRS_WRAP3,
    D3DRS_WRAP4,
    D3DRS_WRAP5,
    D3DRS_WRAP6,
    D3DRS_WRAP7,
    D3DRS_WRAP8,
    D3DRS_WRAP9,
    D3DRS_ZENABLE,
    D3DRS_ZFUNC,
    D3DRS_ZWRITEENABLE
};
*/
const uint32_t nine_render_states_pixel[(NINED3DRS_LAST + 31) / 32] =
{
    0x0f99c380, 0x1ff00070, 0x00000000, 0x00000000,
    0x000000ff, 0xde01c900, 0x0003ffcf
};

/*
static const DWORD nine_render_states_vertex[] =
{
    D3DRS_ADAPTIVETESS_W,
    D3DRS_ADAPTIVETESS_X,
    D3DRS_ADAPTIVETESS_Y,
    D3DRS_ADAPTIVETESS_Z,
    D3DRS_AMBIENT,
    D3DRS_AMBIENTMATERIALSOURCE,
    D3DRS_CLIPPING,
    D3DRS_CLIPPLANEENABLE,
    D3DRS_COLORVERTEX,
    D3DRS_CULLMODE,
    D3DRS_DIFFUSEMATERIALSOURCE,
    D3DRS_EMISSIVEMATERIALSOURCE,
    D3DRS_ENABLEADAPTIVETESSELLATION,
    D3DRS_FOGCOLOR,
    D3DRS_FOGDENSITY,
    D3DRS_FOGENABLE,
    D3DRS_FOGEND,
    D3DRS_FOGSTART,
    D3DRS_FOGTABLEMODE,
    D3DRS_FOGVERTEXMODE,
    D3DRS_INDEXEDVERTEXBLENDENABLE,
    D3DRS_LIGHTING,
    D3DRS_LOCALVIEWER,
    D3DRS_MAXTESSELLATIONLEVEL,
    D3DRS_MINTESSELLATIONLEVEL,
    D3DRS_MULTISAMPLEANTIALIAS,
    D3DRS_MULTISAMPLEMASK,
    D3DRS_NORMALDEGREE,
    D3DRS_NORMALIZENORMALS,
    D3DRS_PATCHEDGESTYLE,
    D3DRS_POINTSCALE_A,
    D3DRS_POINTSCALE_B,
    D3DRS_POINTSCALE_C,
    D3DRS_POINTSCALEENABLE,
    D3DRS_POINTSIZE,
    D3DRS_POINTSIZE_MAX,
    D3DRS_POINTSIZE_MIN,
    D3DRS_POINTSPRITEENABLE,
    D3DRS_POSITIONDEGREE,
    D3DRS_RANGEFOGENABLE,
    D3DRS_SHADEMODE,
    D3DRS_SPECULARENABLE,
    D3DRS_SPECULARMATERIALSOURCE,
    D3DRS_TWEENFACTOR,
    D3DRS_VERTEXBLEND
};
*/
const uint32_t nine_render_states_vertex[(NINED3DRS_LAST + 31) / 32] =
{
    0x30400200, 0x0001007c, 0x00000000, 0x00000000,
    0xfd9efb00, 0x01fc34cf, 0x00000000
};

/* TODO: put in the right values */
const uint32_t nine_render_state_group[NINED3DRS_LAST + 1] =
{
    [D3DRS_ZENABLE] = NINE_STATE_DSA | NINE_STATE_MULTISAMPLE,
    [D3DRS_FILLMODE] = NINE_STATE_RASTERIZER,
    [D3DRS_SHADEMODE] = NINE_STATE_RASTERIZER | NINE_STATE_PS_PARAMS_MISC,
    [D3DRS_ZWRITEENABLE] = NINE_STATE_DSA,
    [D3DRS_ALPHATESTENABLE] = NINE_STATE_DSA,
    [D3DRS_LASTPIXEL] = NINE_STATE_RASTERIZER,
    [D3DRS_SRCBLEND] = NINE_STATE_BLEND,
    [D3DRS_DESTBLEND] = NINE_STATE_BLEND,
    [D3DRS_CULLMODE] = NINE_STATE_RASTERIZER,
    [D3DRS_ZFUNC] = NINE_STATE_DSA,
    [D3DRS_ALPHAREF] = NINE_STATE_DSA,
    [D3DRS_ALPHAFUNC] = NINE_STATE_DSA,
    [D3DRS_DITHERENABLE] = NINE_STATE_BLEND,
    [D3DRS_ALPHABLENDENABLE] = NINE_STATE_BLEND,
    [D3DRS_FOGENABLE] = NINE_STATE_FF_SHADER | NINE_STATE_VS_PARAMS_MISC | NINE_STATE_PS_PARAMS_MISC | NINE_STATE_PS_CONST,
    [D3DRS_SPECULARENABLE] = NINE_STATE_FF_LIGHTING,
    [D3DRS_FOGCOLOR] = NINE_STATE_FF_PS_CONSTS | NINE_STATE_PS_CONST,
    [D3DRS_FOGTABLEMODE] = NINE_STATE_FF_SHADER | NINE_STATE_PS_PARAMS_MISC | NINE_STATE_PS_CONST,
    [D3DRS_FOGSTART] = NINE_STATE_FF_VS_OTHER | NINE_STATE_FF_PS_CONSTS | NINE_STATE_PS_CONST,
    [D3DRS_FOGEND] = NINE_STATE_FF_VS_OTHER | NINE_STATE_FF_PS_CONSTS | NINE_STATE_PS_CONST,
    [D3DRS_FOGDENSITY] = NINE_STATE_FF_VS_OTHER | NINE_STATE_FF_PS_CONSTS | NINE_STATE_PS_CONST,
    [D3DRS_RANGEFOGENABLE] = NINE_STATE_FF_SHADER,
    [D3DRS_STENCILENABLE] = NINE_STATE_DSA | NINE_STATE_MULTISAMPLE,
    [D3DRS_STENCILFAIL] = NINE_STATE_DSA,
    [D3DRS_STENCILZFAIL] = NINE_STATE_DSA,
    [D3DRS_STENCILPASS] = NINE_STATE_DSA,
    [D3DRS_STENCILFUNC] = NINE_STATE_DSA,
    [D3DRS_STENCILREF] = NINE_STATE_STENCIL_REF,
    [D3DRS_STENCILMASK] = NINE_STATE_DSA,
    [D3DRS_STENCILWRITEMASK] = NINE_STATE_DSA,
    [D3DRS_TEXTUREFACTOR] = NINE_STATE_FF_PS_CONSTS,
    [D3DRS_WRAP0] = NINE_STATE_UNHANDLED, /* cylindrical wrap is crazy */
    [D3DRS_WRAP1] = NINE_STATE_UNHANDLED,
    [D3DRS_WRAP2] = NINE_STATE_UNHANDLED,
    [D3DRS_WRAP3] = NINE_STATE_UNHANDLED,
    [D3DRS_WRAP4] = NINE_STATE_UNHANDLED,
    [D3DRS_WRAP5] = NINE_STATE_UNHANDLED,
    [D3DRS_WRAP6] = NINE_STATE_UNHANDLED,
    [D3DRS_WRAP7] = NINE_STATE_UNHANDLED,
    [D3DRS_CLIPPING] = 0, /* software vertex processing only */
    [D3DRS_LIGHTING] = NINE_STATE_FF_LIGHTING,
    [D3DRS_AMBIENT] = NINE_STATE_FF_LIGHTING | NINE_STATE_FF_MATERIAL,
    [D3DRS_FOGVERTEXMODE] = NINE_STATE_FF_SHADER,
    [D3DRS_COLORVERTEX] = NINE_STATE_FF_LIGHTING,
    [D3DRS_LOCALVIEWER] = NINE_STATE_FF_LIGHTING,
    [D3DRS_NORMALIZENORMALS] = NINE_STATE_FF_SHADER,
    [D3DRS_DIFFUSEMATERIALSOURCE] = NINE_STATE_FF_LIGHTING,
    [D3DRS_SPECULARMATERIALSOURCE] = NINE_STATE_FF_LIGHTING,
    [D3DRS_AMBIENTMATERIALSOURCE] = NINE_STATE_FF_LIGHTING,
    [D3DRS_EMISSIVEMATERIALSOURCE] = NINE_STATE_FF_LIGHTING,
    [D3DRS_VERTEXBLEND] = NINE_STATE_FF_SHADER,
    [D3DRS_CLIPPLANEENABLE] = NINE_STATE_RASTERIZER,
    [D3DRS_POINTSIZE] = NINE_STATE_RASTERIZER | NINE_STATE_FF_VS_OTHER,
    [D3DRS_POINTSIZE_MIN] = NINE_STATE_RASTERIZER | NINE_STATE_FF_VS_OTHER | NINE_STATE_VS_PARAMS_MISC,
    [D3DRS_POINTSPRITEENABLE] = NINE_STATE_RASTERIZER,
    [D3DRS_POINTSCALEENABLE] = NINE_STATE_FF_SHADER,
    [D3DRS_POINTSCALE_A] = NINE_STATE_FF_VS_OTHER,
    [D3DRS_POINTSCALE_B] = NINE_STATE_FF_VS_OTHER,
    [D3DRS_POINTSCALE_C] = NINE_STATE_FF_VS_OTHER,
    [D3DRS_MULTISAMPLEANTIALIAS] = NINE_STATE_MULTISAMPLE,
    [D3DRS_MULTISAMPLEMASK] = NINE_STATE_SAMPLE_MASK,
    [D3DRS_PATCHEDGESTYLE] = NINE_STATE_UNHANDLED,
    [D3DRS_DEBUGMONITORTOKEN] = NINE_STATE_UNHANDLED,
    [D3DRS_POINTSIZE_MAX] = NINE_STATE_RASTERIZER | NINE_STATE_FF_VS_OTHER | NINE_STATE_VS_PARAMS_MISC,
    [D3DRS_INDEXEDVERTEXBLENDENABLE] = NINE_STATE_FF_SHADER,
    [D3DRS_COLORWRITEENABLE] = NINE_STATE_BLEND,
    [D3DRS_TWEENFACTOR] = NINE_STATE_FF_VS_OTHER,
    [D3DRS_BLENDOP] = NINE_STATE_BLEND,
    [D3DRS_POSITIONDEGREE] = NINE_STATE_UNHANDLED,
    [D3DRS_NORMALDEGREE] = NINE_STATE_UNHANDLED,
    [D3DRS_SCISSORTESTENABLE] = NINE_STATE_RASTERIZER,
    [D3DRS_SLOPESCALEDEPTHBIAS] = NINE_STATE_RASTERIZER,
    [D3DRS_ANTIALIASEDLINEENABLE] = NINE_STATE_RASTERIZER,
    [D3DRS_MINTESSELLATIONLEVEL] = NINE_STATE_UNHANDLED,
    [D3DRS_MAXTESSELLATIONLEVEL] = NINE_STATE_UNHANDLED,
    [D3DRS_ADAPTIVETESS_X] = NINE_STATE_UNHANDLED,
    [D3DRS_ADAPTIVETESS_Y] = NINE_STATE_UNHANDLED,
    [D3DRS_ADAPTIVETESS_Z] = NINE_STATE_UNHANDLED,
    [D3DRS_ADAPTIVETESS_W] = NINE_STATE_UNHANDLED,
    [D3DRS_ENABLEADAPTIVETESSELLATION] = NINE_STATE_UNHANDLED,
    [D3DRS_TWOSIDEDSTENCILMODE] = NINE_STATE_DSA,
    [D3DRS_CCW_STENCILFAIL] = NINE_STATE_DSA,
    [D3DRS_CCW_STENCILZFAIL] = NINE_STATE_DSA,
    [D3DRS_CCW_STENCILPASS] = NINE_STATE_DSA,
    [D3DRS_CCW_STENCILFUNC] = NINE_STATE_DSA,
    [D3DRS_COLORWRITEENABLE1] = NINE_STATE_BLEND,
    [D3DRS_COLORWRITEENABLE2] = NINE_STATE_BLEND,
    [D3DRS_COLORWRITEENABLE3] = NINE_STATE_BLEND,
    [D3DRS_BLENDFACTOR] = NINE_STATE_BLEND_COLOR,
    [D3DRS_SRGBWRITEENABLE] = NINE_STATE_FB,
    [D3DRS_DEPTHBIAS] = NINE_STATE_RASTERIZER,
    [D3DRS_WRAP8] = NINE_STATE_UNHANDLED, /* cylwrap has to be done via GP */
    [D3DRS_WRAP9] = NINE_STATE_UNHANDLED,
    [D3DRS_WRAP10] = NINE_STATE_UNHANDLED,
    [D3DRS_WRAP11] = NINE_STATE_UNHANDLED,
    [D3DRS_WRAP12] = NINE_STATE_UNHANDLED,
    [D3DRS_WRAP13] = NINE_STATE_UNHANDLED,
    [D3DRS_WRAP14] = NINE_STATE_UNHANDLED,
    [D3DRS_WRAP15] = NINE_STATE_UNHANDLED,
    [D3DRS_SEPARATEALPHABLENDENABLE] = NINE_STATE_BLEND,
    [D3DRS_SRCBLENDALPHA] = NINE_STATE_BLEND,
    [D3DRS_DESTBLENDALPHA] = NINE_STATE_BLEND,
    [D3DRS_BLENDOPALPHA] = NINE_STATE_BLEND
};

/* Misc */

static D3DMATRIX nine_state_identity = { .m[0] = { 1, 0, 0, 0 },
                                         .m[1] = { 0, 1, 0, 0 },
                                         .m[2] = { 0, 0, 1, 0 },
                                         .m[3] = { 0, 0, 0, 1 } };

void
nine_state_resize_transform(struct nine_ff_state *ff_state, unsigned N)
{
    unsigned n = ff_state->num_transforms;

    if (N <= n)
        return;

    ff_state->transform = REALLOC(ff_state->transform,
                                  n * sizeof(D3DMATRIX),
                                  N * sizeof(D3DMATRIX));
    for (; n < N; ++n)
        ff_state->transform[n] = nine_state_identity;
    ff_state->num_transforms = N;
}

D3DMATRIX *
nine_state_access_transform(struct nine_ff_state *ff_state, D3DTRANSFORMSTATETYPE t,
                            bool alloc)
{
    unsigned index;

    switch (t) {
    case D3DTS_VIEW: index = 0; break;
    case D3DTS_PROJECTION: index = 1; break;
    case D3DTS_TEXTURE0: index = 2; break;
    case D3DTS_TEXTURE1: index = 3; break;
    case D3DTS_TEXTURE2: index = 4; break;
    case D3DTS_TEXTURE3: index = 5; break;
    case D3DTS_TEXTURE4: index = 6; break;
    case D3DTS_TEXTURE5: index = 7; break;
    case D3DTS_TEXTURE6: index = 8; break;
    case D3DTS_TEXTURE7: index = 9; break;
    default:
        if (!(t >= D3DTS_WORLDMATRIX(0) && t <= D3DTS_WORLDMATRIX(255)))
            return NULL;
        index = 10 + (t - D3DTS_WORLDMATRIX(0));
        break;
    }

    if (index >= ff_state->num_transforms) {
        if (!alloc)
            return &nine_state_identity;
        nine_state_resize_transform(ff_state, index + 1);
    }
    return &ff_state->transform[index];
}

HRESULT
nine_state_set_light(struct nine_ff_state *ff_state, DWORD Index,
                     const D3DLIGHT9 *pLight)
{
    if (Index >= ff_state->num_lights) {
        unsigned n = ff_state->num_lights;
        unsigned N = Index + 1;

        ff_state->light = REALLOC(ff_state->light, n * sizeof(D3DLIGHT9),
                                                   N * sizeof(D3DLIGHT9));
        if (!ff_state->light)
            return E_OUTOFMEMORY;
        ff_state->num_lights = N;

        for (; n < Index; ++n) {
            memset(&ff_state->light[n], 0, sizeof(D3DLIGHT9));
            ff_state->light[n].Type = (D3DLIGHTTYPE)NINED3DLIGHT_INVALID;
        }
    }
    ff_state->light[Index] = *pLight;

    if (pLight->Type == D3DLIGHT_SPOT && pLight->Theta >= pLight->Phi) {
        DBG("Warning: clamping D3DLIGHT9.Theta\n");
        ff_state->light[Index].Theta = ff_state->light[Index].Phi;
    }
    return D3D_OK;
}

HRESULT
nine_state_light_enable(struct nine_ff_state *ff_state,
                        DWORD Index, BOOL Enable)
{
    unsigned i;

    user_assert(Index < ff_state->num_lights, D3DERR_INVALIDCALL);

    for (i = 0; i < ff_state->num_lights_active; ++i) {
        if (ff_state->active_light[i] == Index)
            break;
    }

    if (Enable) {
        if (i < ff_state->num_lights_active)
            return D3D_OK;
        /* XXX wine thinks this should still succeed:
         */
        user_assert(i < NINE_MAX_LIGHTS_ACTIVE, D3DERR_INVALIDCALL);

        ff_state->active_light[i] = Index;
        ff_state->num_lights_active++;
    } else {
        if (i == ff_state->num_lights_active)
            return D3D_OK;
        --ff_state->num_lights_active;
        for (; i < ff_state->num_lights_active; ++i)
            ff_state->active_light[i] = ff_state->active_light[i + 1];
    }

    return D3D_OK;
}

#define D3DRS_TO_STRING_CASE(n) case D3DRS_##n: return "D3DRS_"#n
const char *nine_d3drs_to_string(DWORD State)
{
    switch (State) {
    D3DRS_TO_STRING_CASE(ZENABLE);
    D3DRS_TO_STRING_CASE(FILLMODE);
    D3DRS_TO_STRING_CASE(SHADEMODE);
    D3DRS_TO_STRING_CASE(ZWRITEENABLE);
    D3DRS_TO_STRING_CASE(ALPHATESTENABLE);
    D3DRS_TO_STRING_CASE(LASTPIXEL);
    D3DRS_TO_STRING_CASE(SRCBLEND);
    D3DRS_TO_STRING_CASE(DESTBLEND);
    D3DRS_TO_STRING_CASE(CULLMODE);
    D3DRS_TO_STRING_CASE(ZFUNC);
    D3DRS_TO_STRING_CASE(ALPHAREF);
    D3DRS_TO_STRING_CASE(ALPHAFUNC);
    D3DRS_TO_STRING_CASE(DITHERENABLE);
    D3DRS_TO_STRING_CASE(ALPHABLENDENABLE);
    D3DRS_TO_STRING_CASE(FOGENABLE);
    D3DRS_TO_STRING_CASE(SPECULARENABLE);
    D3DRS_TO_STRING_CASE(FOGCOLOR);
    D3DRS_TO_STRING_CASE(FOGTABLEMODE);
    D3DRS_TO_STRING_CASE(FOGSTART);
    D3DRS_TO_STRING_CASE(FOGEND);
    D3DRS_TO_STRING_CASE(FOGDENSITY);
    D3DRS_TO_STRING_CASE(RANGEFOGENABLE);
    D3DRS_TO_STRING_CASE(STENCILENABLE);
    D3DRS_TO_STRING_CASE(STENCILFAIL);
    D3DRS_TO_STRING_CASE(STENCILZFAIL);
    D3DRS_TO_STRING_CASE(STENCILPASS);
    D3DRS_TO_STRING_CASE(STENCILFUNC);
    D3DRS_TO_STRING_CASE(STENCILREF);
    D3DRS_TO_STRING_CASE(STENCILMASK);
    D3DRS_TO_STRING_CASE(STENCILWRITEMASK);
    D3DRS_TO_STRING_CASE(TEXTUREFACTOR);
    D3DRS_TO_STRING_CASE(WRAP0);
    D3DRS_TO_STRING_CASE(WRAP1);
    D3DRS_TO_STRING_CASE(WRAP2);
    D3DRS_TO_STRING_CASE(WRAP3);
    D3DRS_TO_STRING_CASE(WRAP4);
    D3DRS_TO_STRING_CASE(WRAP5);
    D3DRS_TO_STRING_CASE(WRAP6);
    D3DRS_TO_STRING_CASE(WRAP7);
    D3DRS_TO_STRING_CASE(CLIPPING);
    D3DRS_TO_STRING_CASE(LIGHTING);
    D3DRS_TO_STRING_CASE(AMBIENT);
    D3DRS_TO_STRING_CASE(FOGVERTEXMODE);
    D3DRS_TO_STRING_CASE(COLORVERTEX);
    D3DRS_TO_STRING_CASE(LOCALVIEWER);
    D3DRS_TO_STRING_CASE(NORMALIZENORMALS);
    D3DRS_TO_STRING_CASE(DIFFUSEMATERIALSOURCE);
    D3DRS_TO_STRING_CASE(SPECULARMATERIALSOURCE);
    D3DRS_TO_STRING_CASE(AMBIENTMATERIALSOURCE);
    D3DRS_TO_STRING_CASE(EMISSIVEMATERIALSOURCE);
    D3DRS_TO_STRING_CASE(VERTEXBLEND);
    D3DRS_TO_STRING_CASE(CLIPPLANEENABLE);
    D3DRS_TO_STRING_CASE(POINTSIZE);
    D3DRS_TO_STRING_CASE(POINTSIZE_MIN);
    D3DRS_TO_STRING_CASE(POINTSPRITEENABLE);
    D3DRS_TO_STRING_CASE(POINTSCALEENABLE);
    D3DRS_TO_STRING_CASE(POINTSCALE_A);
    D3DRS_TO_STRING_CASE(POINTSCALE_B);
    D3DRS_TO_STRING_CASE(POINTSCALE_C);
    D3DRS_TO_STRING_CASE(MULTISAMPLEANTIALIAS);
    D3DRS_TO_STRING_CASE(MULTISAMPLEMASK);
    D3DRS_TO_STRING_CASE(PATCHEDGESTYLE);
    D3DRS_TO_STRING_CASE(DEBUGMONITORTOKEN);
    D3DRS_TO_STRING_CASE(POINTSIZE_MAX);
    D3DRS_TO_STRING_CASE(INDEXEDVERTEXBLENDENABLE);
    D3DRS_TO_STRING_CASE(COLORWRITEENABLE);
    D3DRS_TO_STRING_CASE(TWEENFACTOR);
    D3DRS_TO_STRING_CASE(BLENDOP);
    D3DRS_TO_STRING_CASE(POSITIONDEGREE);
    D3DRS_TO_STRING_CASE(NORMALDEGREE);
    D3DRS_TO_STRING_CASE(SCISSORTESTENABLE);
    D3DRS_TO_STRING_CASE(SLOPESCALEDEPTHBIAS);
    D3DRS_TO_STRING_CASE(ANTIALIASEDLINEENABLE);
    D3DRS_TO_STRING_CASE(MINTESSELLATIONLEVEL);
    D3DRS_TO_STRING_CASE(MAXTESSELLATIONLEVEL);
    D3DRS_TO_STRING_CASE(ADAPTIVETESS_X);
    D3DRS_TO_STRING_CASE(ADAPTIVETESS_Y);
    D3DRS_TO_STRING_CASE(ADAPTIVETESS_Z);
    D3DRS_TO_STRING_CASE(ADAPTIVETESS_W);
    D3DRS_TO_STRING_CASE(ENABLEADAPTIVETESSELLATION);
    D3DRS_TO_STRING_CASE(TWOSIDEDSTENCILMODE);
    D3DRS_TO_STRING_CASE(CCW_STENCILFAIL);
    D3DRS_TO_STRING_CASE(CCW_STENCILZFAIL);
    D3DRS_TO_STRING_CASE(CCW_STENCILPASS);
    D3DRS_TO_STRING_CASE(CCW_STENCILFUNC);
    D3DRS_TO_STRING_CASE(COLORWRITEENABLE1);
    D3DRS_TO_STRING_CASE(COLORWRITEENABLE2);
    D3DRS_TO_STRING_CASE(COLORWRITEENABLE3);
    D3DRS_TO_STRING_CASE(BLENDFACTOR);
    D3DRS_TO_STRING_CASE(SRGBWRITEENABLE);
    D3DRS_TO_STRING_CASE(DEPTHBIAS);
    D3DRS_TO_STRING_CASE(WRAP8);
    D3DRS_TO_STRING_CASE(WRAP9);
    D3DRS_TO_STRING_CASE(WRAP10);
    D3DRS_TO_STRING_CASE(WRAP11);
    D3DRS_TO_STRING_CASE(WRAP12);
    D3DRS_TO_STRING_CASE(WRAP13);
    D3DRS_TO_STRING_CASE(WRAP14);
    D3DRS_TO_STRING_CASE(WRAP15);
    D3DRS_TO_STRING_CASE(SEPARATEALPHABLENDENABLE);
    D3DRS_TO_STRING_CASE(SRCBLENDALPHA);
    D3DRS_TO_STRING_CASE(DESTBLENDALPHA);
    D3DRS_TO_STRING_CASE(BLENDOPALPHA);
    default:
        return "(invalid)";
    }
}
