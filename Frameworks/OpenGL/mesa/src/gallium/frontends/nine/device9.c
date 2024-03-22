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

#include "device9.h"
#include "stateblock9.h"
#include "surface9.h"
#include "swapchain9.h"
#include "swapchain9ex.h"
#include "indexbuffer9.h"
#include "vertexbuffer9.h"
#include "vertexdeclaration9.h"
#include "vertexshader9.h"
#include "pixelshader9.h"
#include "query9.h"
#include "texture9.h"
#include "cubetexture9.h"
#include "volumetexture9.h"
#include "nine_buffer_upload.h"
#include "nine_helpers.h"
#include "nine_memory_helper.h"
#include "nine_pipe.h"
#include "nine_ff.h"
#include "nine_dump.h"
#include "nine_limits.h"

#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "util/detect.h"
#include "util/macros.h"
#include "util/u_math.h"
#include "util/u_inlines.h"
#include "util/u_hash_table.h"
#include "util/format/u_format.h"
#include "util/u_surface.h"
#include "util/u_upload_mgr.h"
#include "hud/hud_context.h"
#include "compiler/glsl_types.h"

#include "cso_cache/cso_context.h"

#define DBG_CHANNEL DBG_DEVICE

#if DETECT_CC_GCC && (DETECT_ARCH_X86 || DETECT_ARCH_X86_64)

static void nine_setup_fpu()
{
    uint16_t c;

    __asm__ __volatile__ ("fnstcw %0" : "=m" (*&c));

    /* clear the control word */
    c &= 0xF0C0;
    /* d3d9 doc/wine tests: mask all exceptions, use single-precision
     * and round to nearest */
    c |= 0x003F;

    __asm__ __volatile__ ("fldcw %0" : : "m" (*&c));
}

static void nine_setup_set_fpu(uint16_t val)
{
    __asm__ __volatile__ ("fldcw %0" : : "m" (*&val));
}

static uint16_t nine_setup_get_fpu()
{
    uint16_t c;

    __asm__ __volatile__ ("fnstcw %0" : "=m" (*&c));
    return c;
}

#else

static void nine_setup_fpu(void)
{
    WARN_ONCE("FPU setup not supported on non-x86 platforms\n");
}

static void nine_setup_set_fpu(UNUSED uint16_t val)
{
    WARN_ONCE("FPU setup not supported on non-x86 platforms\n");
}

static uint16_t nine_setup_get_fpu()
{
    WARN_ONCE("FPU setup not supported on non-x86 platforms\n");
    return 0;
}

#endif

struct pipe_resource *
nine_resource_create_with_retry( struct NineDevice9 *This,
                                 struct pipe_screen *screen,
                                 const struct pipe_resource *templat )
{
    struct pipe_resource *res;
    res = screen->resource_create(screen, templat);
    if (res)
        return res;
    /* Allocation failed, retry after freeing some resources
     * Note: Shouldn't be called from the worker thread */
    if (!This)
        return NULL;
    /* Evict resources we can evict */
    NineDevice9_EvictManagedResourcesInternal(This);
    /* Execute anything pending, such that some
     * deleted resources can be actually freed */
    nine_csmt_process(This);
    /* We could also finish the context, if needed */
    return screen->resource_create(screen, templat);
}

void
NineDevice9_SetDefaultState( struct NineDevice9 *This, bool is_reset )
{
    struct NineSurface9 *refSurf = NULL;

    DBG("This=%p is_reset=%d\n", This, (int) is_reset);

    assert(!This->is_recording);

    nine_state_set_defaults(This, &This->caps, is_reset);

    refSurf = This->swapchains[0]->buffers[0];
    assert(refSurf);

    This->state.viewport.X = 0;
    This->state.viewport.Y = 0;
    This->state.viewport.Width = refSurf->desc.Width;
    This->state.viewport.Height = refSurf->desc.Height;

    nine_context_set_viewport(This, &This->state.viewport);

    This->state.scissor.minx = 0;
    This->state.scissor.miny = 0;
    This->state.scissor.maxx = refSurf->desc.Width;
    This->state.scissor.maxy = refSurf->desc.Height;

    nine_context_set_scissor(This, &This->state.scissor);

    if (This->nswapchains && This->swapchains[0]->params.EnableAutoDepthStencil) {
        nine_context_set_render_state(This, D3DRS_ZENABLE, true);
        This->state.rs_advertised[D3DRS_ZENABLE] = true;
    }
    if (This->state.rs_advertised[D3DRS_ZENABLE])
        NineDevice9_SetDepthStencilSurface(
            This, (IDirect3DSurface9 *)This->swapchains[0]->zsbuf);
}

#define GET_PCAP(n) pScreen->get_param(pScreen, PIPE_CAP_##n)
HRESULT
NineDevice9_ctor( struct NineDevice9 *This,
                  struct NineUnknownParams *pParams,
                  struct pipe_screen *pScreen,
                  D3DDEVICE_CREATION_PARAMETERS *pCreationParameters,
                  D3DCAPS9 *pCaps,
                  D3DPRESENT_PARAMETERS *pPresentationParameters,
                  IDirect3D9 *pD3D9,
                  ID3DPresentGroup *pPresentationGroup,
                  struct d3dadapter9_context *pCTX,
                  bool ex,
                  D3DDISPLAYMODEEX *pFullscreenDisplayMode,
                  int minorVersionNum )
{
    unsigned i;
    uint16_t fpu_cw = 0;
    HRESULT hr = NineUnknown_ctor(&This->base, pParams);

    DBG("This=%p pParams=%p pScreen=%p pCreationParameters=%p pCaps=%p pPresentationParameters=%p "
        "pD3D9=%p pPresentationGroup=%p pCTX=%p ex=%d pFullscreenDisplayMode=%p\n",
        This, pParams, pScreen, pCreationParameters, pCaps, pPresentationParameters, pD3D9,
        pPresentationGroup, pCTX, (int) ex, pFullscreenDisplayMode);

    if (FAILED(hr)) { return hr; }

    /* NIR shaders need to use GLSL types so let's initialize them here */
    glsl_type_singleton_init_or_ref();

    list_inithead(&This->update_buffers);
    list_inithead(&This->update_textures);
    list_inithead(&This->managed_buffers);
    list_inithead(&This->managed_textures);

    This->screen = pScreen;
    This->screen_sw = pCTX->ref;
    This->caps = *pCaps;
    This->d3d9 = pD3D9;
    This->params = *pCreationParameters;
    This->ex = ex;
    This->present = pPresentationGroup;
    This->minor_version_num = minorVersionNum;

    /* Ex */
    This->gpu_priority = 0;
    This->max_frame_latency = 3;

    IDirect3D9_AddRef(This->d3d9);
    ID3DPresentGroup_AddRef(This->present);

    if (!(This->params.BehaviorFlags & D3DCREATE_FPU_PRESERVE)) {
        nine_setup_fpu();
    } else {
        /* Software renderer initialization needs exceptions masked */
        fpu_cw = nine_setup_get_fpu();
        nine_setup_set_fpu(fpu_cw | 0x007f);
    }

    if (This->params.BehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING) {
        DBG("Application asked full Software Vertex Processing.\n");
        This->swvp = true;
        This->may_swvp = true;
    } else
        This->swvp = false;
    if (This->params.BehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING) {
        DBG("Application asked mixed Software Vertex Processing.\n");
        This->may_swvp = true;
    }
    This->context.swvp = This->swvp;
    /* TODO: check if swvp is resetted by device Resets */

    if (This->may_swvp &&
        (This->screen->get_shader_param(This->screen, PIPE_SHADER_VERTEX,
                                        PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE)
                                     < (NINE_MAX_CONST_F_SWVP/2) * sizeof(float[4]) ||
         This->screen->get_shader_param(This->screen, PIPE_SHADER_VERTEX,
                                        PIPE_SHADER_CAP_MAX_CONST_BUFFERS) < 5)) {
        /* Note: We just go on, some apps never use the abilities of
         * swvp, and just set more constants than allowed at init.
         * Only cards we support that are affected are the r500 */
        WARN("Card unable to handle Software Vertex Processing. Game may fail\n");
    }

    /* When may_swvp, SetConstant* limits are different */
    if (This->may_swvp)
        This->caps.MaxVertexShaderConst = NINE_MAX_CONST_F_SWVP;

    This->pure = !!(This->params.BehaviorFlags & D3DCREATE_PUREDEVICE);

    This->context.pipe = This->screen->context_create(This->screen, NULL, PIPE_CONTEXT_PREFER_THREADED);
    This->pipe_secondary = This->screen->context_create(This->screen, NULL, 0);
    if (!This->context.pipe || !This->pipe_secondary) { return E_OUTOFMEMORY; } /* guess */
    This->pipe_sw = This->screen_sw->context_create(This->screen_sw, NULL, PIPE_CONTEXT_PREFER_THREADED);
    if (!This->pipe_sw) { return E_OUTOFMEMORY; }

    This->context.cso = cso_create_context(This->context.pipe, CSO_NO_USER_VERTEX_BUFFERS);
    if (!This->context.cso) { return E_OUTOFMEMORY; } /* also a guess */
    This->cso_sw = cso_create_context(This->pipe_sw, 0);
    if (!This->cso_sw) { return E_OUTOFMEMORY; }

    /* Create first, it messes up our state. */
    This->hud = hud_create(This->context.cso, NULL, NULL, NULL); /* NULL result is fine */

    This->allocator = nine_allocator_create(This, pCTX->memfd_virtualsizelimit);

    /* Available memory counter. Updated only for allocations with this device
     * instance. This is the Win 7 behavior.
     * Win XP shares this counter across multiple devices. */
    This->available_texture_mem = This->screen->get_param(This->screen, PIPE_CAP_VIDEO_MEMORY);
    This->available_texture_mem =  (pCTX->override_vram_size >= 0) ?
        (long long)pCTX->override_vram_size : This->available_texture_mem;
    This->available_texture_mem <<= 20;

    /* We cap texture memory usage to 95% of what is reported free initially
     * This helps get closer Win behaviour. For example VertexBuffer allocation
     * still succeeds when texture allocation fails. */
    This->available_texture_limit = This->available_texture_mem * 5LL / 100LL;

    This->frame_count = 0; /* Used to check if events occur the same frame */

    /* create implicit swapchains */
    This->nswapchains = ID3DPresentGroup_GetMultiheadCount(This->present);
    This->swapchains = CALLOC(This->nswapchains,
                              sizeof(struct NineSwapChain9 *));
    if (!This->swapchains) { return E_OUTOFMEMORY; }

    for (i = 0; i < This->nswapchains; ++i) {
        ID3DPresent *present;

        hr = ID3DPresentGroup_GetPresent(This->present, i, &present);
        if (FAILED(hr))
            return hr;

        if (ex) {
            D3DDISPLAYMODEEX *mode = NULL;
            struct NineSwapChain9Ex **ret =
                (struct NineSwapChain9Ex **)&This->swapchains[i];

            if (pFullscreenDisplayMode) mode = &(pFullscreenDisplayMode[i]);
            /* when this is a Device9Ex, it should create SwapChain9Exs */
            hr = NineSwapChain9Ex_new(This, true, present,
                                      &pPresentationParameters[i], pCTX,
                                      This->params.hFocusWindow, mode, ret);
        } else {
            hr = NineSwapChain9_new(This, true, present,
                                    &pPresentationParameters[i], pCTX,
                                    This->params.hFocusWindow,
                                    &This->swapchains[i]);
        }

        ID3DPresent_Release(present);
        if (FAILED(hr))
            return hr;
        NineUnknown_ConvertRefToBind(NineUnknown(This->swapchains[i]));

        hr = NineSwapChain9_GetBackBuffer(This->swapchains[i], 0,
                                          D3DBACKBUFFER_TYPE_MONO,
                                          (IDirect3DSurface9 **)
                                          &This->state.rt[i]);
        if (FAILED(hr))
            return hr;
        NineUnknown_ConvertRefToBind(NineUnknown(This->state.rt[i]));
        nine_bind(&This->context.rt[i], This->state.rt[i]);
    }

    /* Initialize CSMT */
    /* r600, radeonsi and iris are thread safe. */
    if (pCTX->csmt_force == 1)
        This->csmt_active = true;
    else if (pCTX->csmt_force == 0)
        This->csmt_active = false;
    else if (strstr(pScreen->get_name(pScreen), "AMD") != NULL)
        This->csmt_active = true;
    else if (strstr(pScreen->get_name(pScreen), "Intel") != NULL)
        This->csmt_active = true;

    /* We rely on u_upload_mgr using persistent coherent buffers (which don't
     * require flush to work in multi-pipe_context scenario) for vertex and
     * index buffers */
    if (!GET_PCAP(BUFFER_MAP_PERSISTENT_COHERENT))
        This->csmt_active = false;

    if (This->csmt_active) {
        This->csmt_ctx = nine_csmt_create(This);
        if (!This->csmt_ctx)
            return E_OUTOFMEMORY;
    }

    if (This->csmt_active)
        DBG("\033[1;32mCSMT is active\033[0m\n");

    This->workarounds.dynamic_texture_workaround = pCTX->dynamic_texture_workaround;

    /* Due to the pb_cache, in some cases the buffer_upload path can increase GTT usage/virtual memory.
     * As the performance gain is negligible when csmt is off, disable it in this case.
     * That way csmt_force=0 can be used as a workaround to reduce GTT usage/virtual memory. */
    This->buffer_upload = This->csmt_active ? nine_upload_create(This->pipe_secondary, 4 * 1024 * 1024, 4) : NULL;

    /* Initialize a dummy VBO to be used when a vertex declaration does not
     * specify all the inputs needed by vertex shader, on win default behavior
     * is to pass 0,0,0,0 to the shader */
    {
        struct pipe_transfer *transfer;
        struct pipe_resource tmpl;
        struct pipe_box box;
        unsigned char *data;

        memset(&tmpl, 0, sizeof(tmpl));
        tmpl.target = PIPE_BUFFER;
        tmpl.format = PIPE_FORMAT_R8_UNORM;
        tmpl.width0 = 16; /* 4 floats */
        tmpl.height0 = 1;
        tmpl.depth0 = 1;
        tmpl.array_size = 1;
        tmpl.last_level = 0;
        tmpl.nr_samples = 0;
        tmpl.usage = PIPE_USAGE_DEFAULT;
        tmpl.bind = PIPE_BIND_VERTEX_BUFFER;
        tmpl.flags = 0;
        This->dummy_vbo = pScreen->resource_create(pScreen, &tmpl);

        if (!This->dummy_vbo)
            return D3DERR_OUTOFVIDEOMEMORY;

        u_box_1d(0, 16, &box);
        data = This->context.pipe->buffer_map(This->context.pipe, This->dummy_vbo, 0,
                                        PIPE_MAP_WRITE |
                                        PIPE_MAP_DISCARD_WHOLE_RESOURCE,
                                        &box, &transfer);
        assert(data);
        assert(transfer);
        memset(data, 0, 16);
        This->context.pipe->buffer_unmap(This->context.pipe, transfer);

        /* initialize dummy_vbo_sw */
        if (pScreen != This->screen_sw) {

            This->dummy_vbo_sw = This->screen_sw->resource_create(This->screen_sw, &tmpl);
            if (!This->dummy_vbo_sw)
                return D3DERR_OUTOFVIDEOMEMORY;

            u_box_1d(0, 16, &box);
            data = This->pipe_sw->buffer_map(This->pipe_sw, This->dummy_vbo_sw, 0,
                                       PIPE_MAP_WRITE |
                                       PIPE_MAP_DISCARD_WHOLE_RESOURCE,
                                       &box, &transfer);
            assert(data);
            assert(transfer);
            memset(data, 0, 16);
            This->pipe_sw->buffer_unmap(This->pipe_sw, transfer);
        } else {
            This->dummy_vbo_sw = This->dummy_vbo;
        }
    }

    This->cursor.software = false;
    This->cursor.hotspot.x = -1;
    This->cursor.hotspot.y = -1;
    This->cursor.w = This->cursor.h = 0;
    This->cursor.visible = false;
    if (ID3DPresent_GetCursorPos(This->swapchains[0]->present, &This->cursor.pos) != S_OK) {
        This->cursor.pos.x = 0;
        This->cursor.pos.y = 0;
    }

    {
        struct pipe_resource tmpl;
        memset(&tmpl, 0, sizeof(tmpl));
        tmpl.target = PIPE_TEXTURE_2D;
        tmpl.format = PIPE_FORMAT_R8G8B8A8_UNORM;
        tmpl.width0 = 64;
        tmpl.height0 = 64;
        tmpl.depth0 = 1;
        tmpl.array_size = 1;
        tmpl.last_level = 0;
        tmpl.nr_samples = 0;
        tmpl.usage = PIPE_USAGE_DEFAULT;
        tmpl.bind = PIPE_BIND_CURSOR | PIPE_BIND_SAMPLER_VIEW;
        tmpl.flags = 0;

        This->cursor.image = pScreen->resource_create(pScreen, &tmpl);
        if (!This->cursor.image)
            return D3DERR_OUTOFVIDEOMEMORY;

        /* For uploading 32x32 (argb) cursor */
        This->cursor.hw_upload_temp = MALLOC(32 * 4 * 32);
        if (!This->cursor.hw_upload_temp)
            return D3DERR_OUTOFVIDEOMEMORY;
    }

    /* Create constant buffers. */
    {
        unsigned max_const_vs;

        /* vs 3.0: >= 256 float constants, but for cards with exactly 256 slots,
         * we have to take in some more slots for int and bool*/
        max_const_vs = _min(pScreen->get_shader_param(pScreen, PIPE_SHADER_VERTEX,
                                PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE) /
                                sizeof(float[4]),
                            NINE_MAX_CONST_ALL_VS);
        /* ps 3.0: 224 float constants. All cards supported support at least
         * 256 constants for ps */

        if (max_const_vs == NINE_MAX_CONST_ALL_VS)
            This->max_vs_const_f = NINE_MAX_CONST_F;
        else /* Do not count SPE constants as we won't use them */
            This->max_vs_const_f = max_const_vs -
                               (NINE_MAX_CONST_I + NINE_MAX_CONST_B / 4);

        This->vs_const_size = max_const_vs * sizeof(float[4]);
        This->ps_const_size = NINE_MAX_CONST_ALL_PS * sizeof(float[4]);
        /* Include space for I,B constants for user constbuf. */
        if (This->may_swvp) {
            This->state.vs_const_f = CALLOC(NINE_MAX_CONST_F_SWVP * sizeof(float[4]),1);
            This->context.vs_const_f_swvp = CALLOC(NINE_MAX_CONST_F_SWVP * sizeof(float[4]),1);
            if (!This->context.vs_const_f_swvp)
                return E_OUTOFMEMORY;
            This->state.vs_lconstf_temp = CALLOC(NINE_MAX_CONST_F_SWVP * sizeof(float[4]),1);
            This->context.vs_lconstf_temp = CALLOC(NINE_MAX_CONST_F_SWVP * sizeof(float[4]),1);
            This->state.vs_const_i = CALLOC(NINE_MAX_CONST_I_SWVP * sizeof(int[4]), 1);
            This->context.vs_const_i = CALLOC(NINE_MAX_CONST_I_SWVP * sizeof(int[4]), 1);
            This->state.vs_const_b = CALLOC(NINE_MAX_CONST_B_SWVP * sizeof(BOOL), 1);
            This->context.vs_const_b = CALLOC(NINE_MAX_CONST_B_SWVP * sizeof(BOOL), 1);
        } else {
            This->state.vs_const_f = CALLOC(NINE_MAX_CONST_F * sizeof(float[4]), 1);
            This->context.vs_const_f_swvp = NULL;
            This->state.vs_lconstf_temp = CALLOC(This->vs_const_size,1);
            This->context.vs_lconstf_temp = CALLOC(This->vs_const_size,1);
            This->state.vs_const_i = CALLOC(NINE_MAX_CONST_I * sizeof(int[4]), 1);
            This->context.vs_const_i = CALLOC(NINE_MAX_CONST_I * sizeof(int[4]), 1);
            This->state.vs_const_b = CALLOC(NINE_MAX_CONST_B * sizeof(BOOL), 1);
            This->context.vs_const_b = CALLOC(NINE_MAX_CONST_B * sizeof(BOOL), 1);
        }
        This->context.vs_const_f = CALLOC(This->vs_const_size, 1);
        This->state.ps_const_f = CALLOC(This->ps_const_size, 1);
        This->context.ps_const_f = CALLOC(This->ps_const_size, 1);
        if (!This->state.vs_const_f || !This->context.vs_const_f ||
            !This->state.ps_const_f || !This->context.ps_const_f ||
            !This->state.vs_lconstf_temp || !This->context.vs_lconstf_temp ||
            !This->state.vs_const_i || !This->context.vs_const_i ||
            !This->state.vs_const_b || !This->context.vs_const_b)
            return E_OUTOFMEMORY;

        if (strstr(pScreen->get_name(pScreen), "AMD") ||
            strstr(pScreen->get_name(pScreen), "ATI")) {
            This->driver_bugs.buggy_barycentrics = true;
        }
    }

    /* allocate dummy texture/sampler for when there are missing ones bound */
    {
        struct pipe_resource tmplt;
        struct pipe_sampler_view templ;
        struct pipe_sampler_state samp;
        memset(&tmplt, 0, sizeof(tmplt));
        memset(&samp, 0, sizeof(samp));

        tmplt.target = PIPE_TEXTURE_2D;
        tmplt.width0 = 1;
        tmplt.height0 = 1;
        tmplt.depth0 = 1;
        tmplt.last_level = 0;
        tmplt.array_size = 1;
        tmplt.usage = PIPE_USAGE_DEFAULT;
        tmplt.flags = 0;
        tmplt.format = PIPE_FORMAT_B8G8R8A8_UNORM;
        tmplt.bind = PIPE_BIND_SAMPLER_VIEW;
        tmplt.nr_samples = 0;

        This->dummy_texture = This->screen->resource_create(This->screen, &tmplt);
        if (!This->dummy_texture)
            return D3DERR_DRIVERINTERNALERROR;

        templ.format = PIPE_FORMAT_B8G8R8A8_UNORM;
        templ.u.tex.first_layer = 0;
        templ.u.tex.last_layer = 0;
        templ.u.tex.first_level = 0;
        templ.u.tex.last_level = 0;
        templ.swizzle_r = PIPE_SWIZZLE_0;
        templ.swizzle_g = PIPE_SWIZZLE_0;
        templ.swizzle_b = PIPE_SWIZZLE_0;
        templ.swizzle_a = PIPE_SWIZZLE_1;
        templ.target = This->dummy_texture->target;

        This->dummy_sampler_view = This->context.pipe->create_sampler_view(This->context.pipe, This->dummy_texture, &templ);
        if (!This->dummy_sampler_view)
            return D3DERR_DRIVERINTERNALERROR;

        samp.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
        samp.max_lod = 15.0f;
        samp.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
        samp.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
        samp.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
        samp.min_img_filter = PIPE_TEX_FILTER_NEAREST;
        samp.mag_img_filter = PIPE_TEX_FILTER_NEAREST;
        samp.compare_mode = PIPE_TEX_COMPARE_NONE;
        samp.compare_func = PIPE_FUNC_LEQUAL;
        samp.unnormalized_coords = 0;
        samp.seamless_cube_map = 0;
        This->dummy_sampler_state = samp;
    }

    /* Allocate upload helper for drivers that suck (from st pov ;). */

    This->driver_caps.user_sw_vbufs = This->screen_sw->get_param(This->screen_sw, PIPE_CAP_USER_VERTEX_BUFFERS);
    This->vertex_uploader = This->csmt_active ? This->pipe_secondary->stream_uploader : This->context.pipe->stream_uploader;
    This->driver_caps.window_space_position_support = GET_PCAP(VS_WINDOW_SPACE_POSITION);
    This->driver_caps.disabling_depth_clipping_support = GET_PCAP(DEPTH_CLIP_DISABLE);
    This->driver_caps.vs_integer = pScreen->get_shader_param(pScreen, PIPE_SHADER_VERTEX, PIPE_SHADER_CAP_INTEGERS);
    This->driver_caps.ps_integer = pScreen->get_shader_param(pScreen, PIPE_SHADER_FRAGMENT, PIPE_SHADER_CAP_INTEGERS);
    This->driver_caps.offset_units_unscaled = GET_PCAP(POLYGON_OFFSET_UNITS_UNSCALED);
    This->driver_caps.alpha_test_emulation = !GET_PCAP(ALPHA_TEST);
    /* Always write pointsize output when the driver doesn't support point_size_per_vertex = 0.
     * TODO: Only generate pointsize for draw calls that need it */
    This->driver_caps.always_output_pointsize = !GET_PCAP(POINT_SIZE_FIXED);
    This->driver_caps.emulate_ucp = !(GET_PCAP(CLIP_PLANES) == 1 || GET_PCAP(CLIP_PLANES) >= 8);
    This->driver_caps.shader_emulate_features =  pCTX->force_emulation;

    if (pCTX->force_emulation) {
        This->driver_caps.user_sw_vbufs = false;
        This->driver_caps.window_space_position_support = false;
        This->driver_caps.alpha_test_emulation = true;
        This->driver_caps.always_output_pointsize = true;
        This->driver_caps.emulate_ucp = true;
    }

    /* Disable SPE constants if there is no room for them */
    if (This->max_vs_const_f != NINE_MAX_CONST_F) {
        This->driver_caps.always_output_pointsize = false;
        This->driver_caps.emulate_ucp = false;
    }

    This->context.inline_constants = pCTX->shader_inline_constants;
    /* Code would be needed when integers are not available to correctly
     * handle the conversion of integer constants */
    This->context.inline_constants &= This->driver_caps.vs_integer && This->driver_caps.ps_integer;

    nine_ff_init(This); /* initialize fixed function code */

    NineDevice9_SetDefaultState(This, false);

    {
        struct pipe_poly_stipple stipple;
        memset(&stipple, ~0, sizeof(stipple));
        This->context.pipe->set_polygon_stipple(This->context.pipe, &stipple);
    }

    This->update = &This->state;

    nine_state_init_sw(This);

    ID3DPresentGroup_Release(This->present);
    nine_context_update_state(This); /* Some drivers needs states to be initialized */
    nine_csmt_process(This);

    if (This->params.BehaviorFlags & D3DCREATE_FPU_PRESERVE)
        nine_setup_set_fpu(fpu_cw);

    return D3D_OK;
}
#undef GET_PCAP

void
NineDevice9_dtor( struct NineDevice9 *This )
{
    unsigned i;

    DBG("This=%p\n", This);

    /* Flush all pending commands to get refcount right,
     * and properly release bound objects. It is ok to still
     * execute commands while we are in device dtor, because
     * we haven't released anything yet. Note that no pending
     * command can increase the device refcount. */
    if (This->csmt_active && This->csmt_ctx) {
        nine_csmt_process(This);
        nine_csmt_destroy(This, This->csmt_ctx);
        This->csmt_active = false;
        This->csmt_ctx = NULL;
    }

    nine_ff_fini(This);
    nine_state_destroy_sw(This);
    nine_device_state_clear(This);
    nine_context_clear(This);

    nine_bind(&This->record, NULL);

    pipe_sampler_view_reference(&This->dummy_sampler_view, NULL);
    pipe_resource_reference(&This->dummy_texture, NULL);
    pipe_resource_reference(&This->dummy_vbo, NULL);
    if (This->screen != This->screen_sw)
        pipe_resource_reference(&This->dummy_vbo_sw, NULL);
    FREE(This->state.vs_const_f);
    FREE(This->context.vs_const_f);
    FREE(This->state.ps_const_f);
    FREE(This->context.ps_const_f);
    FREE(This->state.vs_lconstf_temp);
    FREE(This->context.vs_lconstf_temp);
    FREE(This->state.vs_const_i);
    FREE(This->context.vs_const_i);
    FREE(This->state.vs_const_b);
    FREE(This->context.vs_const_b);
    FREE(This->context.vs_const_f_swvp);

    pipe_resource_reference(&This->cursor.image, NULL);
    FREE(This->cursor.hw_upload_temp);

    if (This->swapchains) {
        for (i = 0; i < This->nswapchains; ++i)
            if (This->swapchains[i])
                NineUnknown_Unbind(NineUnknown(This->swapchains[i]));
        FREE(This->swapchains);
    }

    if (This->buffer_upload)
        nine_upload_destroy(This->buffer_upload);

    if (This->allocator)
        nine_allocator_destroy(This->allocator);

    /* Destroy cso first */
    if (This->context.cso) { cso_destroy_context(This->context.cso); }
    if (This->cso_sw) { cso_destroy_context(This->cso_sw); }
    if (This->context.pipe && This->context.pipe->destroy) { This->context.pipe->destroy(This->context.pipe); }
    if (This->pipe_secondary && This->pipe_secondary->destroy) { This->pipe_secondary->destroy(This->pipe_secondary); }
    if (This->pipe_sw && This->pipe_sw->destroy) { This->pipe_sw->destroy(This->pipe_sw); }

    if (This->present) { ID3DPresentGroup_Release(This->present); }
    if (This->d3d9) { IDirect3D9_Release(This->d3d9); }

    NineUnknown_dtor(&This->base);
    glsl_type_singleton_decref();
}

struct pipe_screen *
NineDevice9_GetScreen( struct NineDevice9 *This )
{
    return This->screen;
}

struct pipe_context *
NineDevice9_GetPipe( struct NineDevice9 *This )
{
    return nine_context_get_pipe(This);
}

const D3DCAPS9 *
NineDevice9_GetCaps( struct NineDevice9 *This )
{
    return &This->caps;
}

static inline void
NineDevice9_PauseRecording( struct NineDevice9 *This )
{
    if (This->record) {
        This->update = &This->state;
        This->is_recording = false;
    }
}

static inline void
NineDevice9_ResumeRecording( struct NineDevice9 *This )
{
    if (This->record) {
        This->update = &This->record->state;
        This->is_recording = true;
    }
}

HRESULT NINE_WINAPI
NineDevice9_TestCooperativeLevel( struct NineDevice9 *This )
{
    if (NineSwapChain9_GetOccluded(This->swapchains[0])) {
        This->device_needs_reset = true;
        return D3DERR_DEVICELOST;
    } else if (NineSwapChain9_ResolutionMismatch(This->swapchains[0])) {
        This->device_needs_reset = true;
        return D3DERR_DEVICENOTRESET;
    } else if (This->device_needs_reset) {
        return D3DERR_DEVICENOTRESET;
    }

    return D3D_OK;
}

UINT NINE_WINAPI
NineDevice9_GetAvailableTextureMem( struct NineDevice9 *This )
{
    /* To prevent overflows - Not sure how this should be handled */
    return (UINT)MIN2(This->available_texture_mem, (long long)(UINT_MAX - (64 << 20))); /* 64 MB margin */
}

void
NineDevice9_EvictManagedResourcesInternal( struct NineDevice9 *This )
{
    struct NineBaseTexture9 *tex;

    DBG("This=%p\n", This);

    /* This function is called internally when an allocation fails.
     * We are supposed to release old unused managed textures/buffers,
     * until we have enough space for the allocation.
     * For now just release everything, except the bound textures,
     * as this function can be called when uploading bound textures.
     */
    LIST_FOR_EACH_ENTRY(tex, &This->managed_textures, list2) {
        if (!tex->bind_count)
            NineBaseTexture9_UnLoad(tex);
    }
}

HRESULT NINE_WINAPI
NineDevice9_EvictManagedResources( struct NineDevice9 *This )
{
    struct NineBaseTexture9 *tex;
    struct NineBuffer9 *buf;

    DBG("This=%p\n", This);
    LIST_FOR_EACH_ENTRY(tex, &This->managed_textures, list2) {
        NineBaseTexture9_UnLoad(tex);
    }
    /* Vertex/index buffers don't take a lot of space and aren't accounted
     * for d3d memory usage. Instead of actually freeing from memory,
     * just mark the buffer dirty to trigger a re-upload later. We
     * could just ignore, but some bad behaving apps could rely on it (if
     * they write outside the locked regions typically). */
    LIST_FOR_EACH_ENTRY(buf, &This->managed_buffers, managed.list2) {
        NineBuffer9_SetDirty(buf);
    }

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetDirect3D( struct NineDevice9 *This,
                         IDirect3D9 **ppD3D9 )
{
    user_assert(ppD3D9 != NULL, E_POINTER);
    IDirect3D9_AddRef(This->d3d9);
    *ppD3D9 = This->d3d9;
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetDeviceCaps( struct NineDevice9 *This,
                           D3DCAPS9 *pCaps )
{
    user_assert(pCaps != NULL, D3DERR_INVALIDCALL);
    *pCaps = This->caps;
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetDisplayMode( struct NineDevice9 *This,
                            UINT iSwapChain,
                            D3DDISPLAYMODE *pMode )
{
    DBG("This=%p iSwapChain=%u pMode=%p\n", This, iSwapChain, pMode);

    user_assert(iSwapChain < This->nswapchains, D3DERR_INVALIDCALL);

    return NineSwapChain9_GetDisplayMode(This->swapchains[iSwapChain], pMode);
}

HRESULT NINE_WINAPI
NineDevice9_GetCreationParameters( struct NineDevice9 *This,
                                   D3DDEVICE_CREATION_PARAMETERS *pParameters )
{
    user_assert(pParameters != NULL, D3DERR_INVALIDCALL);
    *pParameters = This->params;
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetCursorProperties( struct NineDevice9 *This,
                                 UINT XHotSpot,
                                 UINT YHotSpot,
                                 IDirect3DSurface9 *pCursorBitmap )
{
    struct NineSurface9 *surf = NineSurface9(pCursorBitmap);
    struct pipe_context *pipe = NineDevice9_GetPipe(This);
    struct pipe_box box;
    struct pipe_transfer *transfer;
    BOOL hw_cursor;
    void *ptr;

    DBG_FLAG(DBG_SWAPCHAIN, "This=%p XHotSpot=%u YHotSpot=%u "
             "pCursorBitmap=%p\n", This, XHotSpot, YHotSpot, pCursorBitmap);

    user_assert(pCursorBitmap, D3DERR_INVALIDCALL);
    user_assert(surf->desc.Format == D3DFMT_A8R8G8B8, D3DERR_INVALIDCALL);

    if (This->swapchains[0]->params.Windowed) {
        This->cursor.w = MIN2(surf->desc.Width, 32);
        This->cursor.h = MIN2(surf->desc.Height, 32);
        hw_cursor = 1; /* always use hw cursor for windowed mode */
    } else {
        This->cursor.w = MIN2(surf->desc.Width, This->cursor.image->width0);
        This->cursor.h = MIN2(surf->desc.Height, This->cursor.image->height0);
        hw_cursor = This->cursor.w == 32 && This->cursor.h == 32;
    }

    u_box_origin_2d(This->cursor.w, This->cursor.h, &box);

    ptr = pipe->texture_map(pipe, This->cursor.image, 0,
                             PIPE_MAP_WRITE |
                             PIPE_MAP_DISCARD_WHOLE_RESOURCE,
                             &box, &transfer);
    if (!ptr)
        ret_err("Failed to update cursor image.\n", D3DERR_DRIVERINTERNALERROR);

    This->cursor.hotspot.x = XHotSpot;
    This->cursor.hotspot.y = YHotSpot;

    /* Copy cursor image to internal storage. */
    {
        D3DLOCKED_RECT lock;
        HRESULT hr;

        hr = NineSurface9_LockRect(surf, &lock, NULL, D3DLOCK_READONLY);
        if (FAILED(hr))
            ret_err("Failed to map cursor source image.\n",
                    D3DERR_DRIVERINTERNALERROR);

        util_format_unpack_rgba_8unorm_rect(surf->base.info.format, ptr, transfer->stride,
                                   lock.pBits, lock.Pitch,
                                   This->cursor.w, This->cursor.h);

        if (hw_cursor) {
            void *data = lock.pBits;
            /* SetCursor assumes 32x32 argb with pitch 128 */
            if (lock.Pitch != 128) {
                util_format_unpack_rgba_8unorm_rect(surf->base.info.format,
                                           This->cursor.hw_upload_temp, 128,
                                           lock.pBits, lock.Pitch,
                                           32, 32);
                data = This->cursor.hw_upload_temp;
            }
            hw_cursor = ID3DPresent_SetCursor(This->swapchains[0]->present,
                                              data,
                                              &This->cursor.hotspot,
                                              This->cursor.visible) == D3D_OK;
        }

        NineSurface9_UnlockRect(surf);
    }
    pipe->texture_unmap(pipe, transfer);

    /* hide cursor if we emulate it */
    if (!hw_cursor)
        ID3DPresent_SetCursor(This->swapchains[0]->present, NULL, NULL, false);
    This->cursor.software = !hw_cursor;

    return D3D_OK;
}

void NINE_WINAPI
NineDevice9_SetCursorPosition( struct NineDevice9 *This,
                               int X,
                               int Y,
                               DWORD Flags )
{
    struct NineSwapChain9 *swap = This->swapchains[0];

    DBG("This=%p X=%d Y=%d Flags=%d\n", This, X, Y, Flags);

    /* present >= v1.4 handles this itself */
    if (This->minor_version_num < 4) {
        if (This->cursor.pos.x == X && This->cursor.pos.y == Y)
            return;
    }

    This->cursor.pos.x = X;
    This->cursor.pos.y = Y;

    if (!This->cursor.software)
        This->cursor.software = ID3DPresent_SetCursorPos(swap->present, &This->cursor.pos) != D3D_OK;
}

BOOL NINE_WINAPI
NineDevice9_ShowCursor( struct NineDevice9 *This,
                        BOOL bShow )
{
    BOOL old = This->cursor.visible;

    DBG("This=%p bShow=%d\n", This, (int) bShow);

    /* No-op until a cursor is set in d3d */
    if (This->cursor.hotspot.x == -1)
        return old;

    This->cursor.visible = bShow;
    /* Note: Don't optimize by avoiding the call if This->cursor.visible
     * hasn't changed. One has to keep in mind the app may do SetCursor
     * calls outside d3d, thus such an optimization affects behaviour. */
    if (!This->cursor.software)
        This->cursor.software = ID3DPresent_SetCursor(This->swapchains[0]->present, NULL, NULL, bShow) != D3D_OK;

    return old;
}

HRESULT NINE_WINAPI
NineDevice9_CreateAdditionalSwapChain( struct NineDevice9 *This,
                                       D3DPRESENT_PARAMETERS *pPresentationParameters,
                                       IDirect3DSwapChain9 **pSwapChain )
{
    struct NineSwapChain9 *swapchain, *tmplt = This->swapchains[0];
    ID3DPresent *present;
    HRESULT hr;

    DBG("This=%p pPresentationParameters=%p pSwapChain=%p\n",
        This, pPresentationParameters, pSwapChain);

    user_assert(pPresentationParameters, D3DERR_INVALIDCALL);
    user_assert(pSwapChain != NULL, D3DERR_INVALIDCALL);
    user_assert(tmplt->params.Windowed && pPresentationParameters->Windowed, D3DERR_INVALIDCALL);

    /* TODO: this deserves more tests */
    if (!pPresentationParameters->hDeviceWindow)
        pPresentationParameters->hDeviceWindow = This->params.hFocusWindow;

    hr = ID3DPresentGroup_CreateAdditionalPresent(This->present, pPresentationParameters, &present);

    if (FAILED(hr))
        return hr;

    hr = NineSwapChain9_new(This, false, present, pPresentationParameters,
                            tmplt->actx,
                            tmplt->params.hDeviceWindow,
                            &swapchain);
    if (FAILED(hr))
        return hr;

    *pSwapChain = (IDirect3DSwapChain9 *)swapchain;
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetSwapChain( struct NineDevice9 *This,
                          UINT iSwapChain,
                          IDirect3DSwapChain9 **pSwapChain )
{
    user_assert(pSwapChain != NULL, D3DERR_INVALIDCALL);

    *pSwapChain = NULL;
    user_assert(iSwapChain < This->nswapchains, D3DERR_INVALIDCALL);

    NineUnknown_AddRef(NineUnknown(This->swapchains[iSwapChain]));
    *pSwapChain = (IDirect3DSwapChain9 *)This->swapchains[iSwapChain];

    return D3D_OK;
}

UINT NINE_WINAPI
NineDevice9_GetNumberOfSwapChains( struct NineDevice9 *This )
{
    return This->nswapchains;
}

HRESULT NINE_WINAPI
NineDevice9_Reset( struct NineDevice9 *This,
                   D3DPRESENT_PARAMETERS *pPresentationParameters )
{
    HRESULT hr = D3D_OK;
    unsigned i;

    DBG("This=%p pPresentationParameters=%p\n", This, pPresentationParameters);

    user_assert(pPresentationParameters != NULL, D3DERR_INVALIDCALL);

    if (NineSwapChain9_GetOccluded(This->swapchains[0])) {
        This->device_needs_reset = true;
        return D3DERR_DEVICELOST;
    }

    for (i = 0; i < This->nswapchains; ++i) {
        D3DPRESENT_PARAMETERS *params = &pPresentationParameters[i];
        hr = NineSwapChain9_Resize(This->swapchains[i], params, NULL);
        if (hr != D3D_OK)
            break;
    }

    nine_csmt_process(This);
    nine_device_state_clear(This);
    nine_context_clear(This);

    NineDevice9_SetDefaultState(This, true);
    NineDevice9_SetRenderTarget(
        This, 0, (IDirect3DSurface9 *)This->swapchains[0]->buffers[0]);
    /* XXX: better use GetBackBuffer here ? */

    This->device_needs_reset = (hr != D3D_OK);
    return hr;
}

HRESULT NINE_WINAPI
NineDevice9_Present( struct NineDevice9 *This,
                     const RECT *pSourceRect,
                     const RECT *pDestRect,
                     HWND hDestWindowOverride,
                     const RGNDATA *pDirtyRegion )
{
    unsigned i;
    HRESULT hr;

    DBG("This=%p pSourceRect=%p pDestRect=%p hDestWindowOverride=%p pDirtyRegion=%p\n",
        This, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);

    /* XXX is this right? */
    for (i = 0; i < This->nswapchains; ++i) {
        hr = NineSwapChain9_Present(This->swapchains[i], pSourceRect, pDestRect,
                                    hDestWindowOverride, pDirtyRegion, 0);
        if (FAILED(hr)) { return hr; }
    }

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetBackBuffer( struct NineDevice9 *This,
                           UINT iSwapChain,
                           UINT iBackBuffer,
                           D3DBACKBUFFER_TYPE Type,
                           IDirect3DSurface9 **ppBackBuffer )
{
    user_assert(ppBackBuffer != NULL, D3DERR_INVALIDCALL);
    /* return NULL on error */
    *ppBackBuffer = NULL;
    user_assert(iSwapChain < This->nswapchains, D3DERR_INVALIDCALL);

    return NineSwapChain9_GetBackBuffer(This->swapchains[iSwapChain],
                                        iBackBuffer, Type, ppBackBuffer);
}

HRESULT NINE_WINAPI
NineDevice9_GetRasterStatus( struct NineDevice9 *This,
                             UINT iSwapChain,
                             D3DRASTER_STATUS *pRasterStatus )
{
    user_assert(pRasterStatus != NULL, D3DERR_INVALIDCALL);
    user_assert(iSwapChain < This->nswapchains, D3DERR_INVALIDCALL);

    return NineSwapChain9_GetRasterStatus(This->swapchains[iSwapChain],
                                          pRasterStatus);
}

HRESULT NINE_WINAPI
NineDevice9_SetDialogBoxMode( struct NineDevice9 *This,
                              BOOL bEnableDialogs )
{
    STUB(D3DERR_INVALIDCALL);
}

void NINE_WINAPI
NineDevice9_SetGammaRamp( struct NineDevice9 *This,
                          UINT iSwapChain,
                          DWORD Flags,
                          const D3DGAMMARAMP *pRamp )
{
    DBG("This=%p iSwapChain=%u Flags=%x pRamp=%p\n", This,
        iSwapChain, Flags, pRamp);

    user_warn(iSwapChain >= This->nswapchains);
    user_warn(!pRamp);

    if (pRamp && (iSwapChain < This->nswapchains)) {
        struct NineSwapChain9 *swap = This->swapchains[iSwapChain];
        swap->gamma = *pRamp;
        ID3DPresent_SetGammaRamp(swap->present, pRamp, swap->params.hDeviceWindow);
    }
}

void NINE_WINAPI
NineDevice9_GetGammaRamp( struct NineDevice9 *This,
                          UINT iSwapChain,
                          D3DGAMMARAMP *pRamp )
{
    DBG("This=%p iSwapChain=%u pRamp=%p\n", This, iSwapChain, pRamp);

    user_warn(iSwapChain >= This->nswapchains);
    user_warn(!pRamp);

    if (pRamp && (iSwapChain < This->nswapchains))
        *pRamp = This->swapchains[iSwapChain]->gamma;
}

HRESULT NINE_WINAPI
NineDevice9_CreateTexture( struct NineDevice9 *This,
                           UINT Width,
                           UINT Height,
                           UINT Levels,
                           DWORD Usage,
                           D3DFORMAT Format,
                           D3DPOOL Pool,
                           IDirect3DTexture9 **ppTexture,
                           HANDLE *pSharedHandle )
{
    struct NineTexture9 *tex;
    HRESULT hr;

    DBG("This=%p Width=%u Height=%u Levels=%u Usage=%s Format=%s Pool=%s "
        "ppOut=%p pSharedHandle=%p\n", This, Width, Height, Levels,
        nine_D3DUSAGE_to_str(Usage), d3dformat_to_string(Format),
        nine_D3DPOOL_to_str(Pool), ppTexture, pSharedHandle);

    user_assert(ppTexture != NULL, D3DERR_INVALIDCALL);

    Usage &= D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_DEPTHSTENCIL | D3DUSAGE_DMAP |
             D3DUSAGE_DYNAMIC | D3DUSAGE_NONSECURE | D3DUSAGE_RENDERTARGET |
             D3DUSAGE_SOFTWAREPROCESSING | D3DUSAGE_TEXTAPI;

    *ppTexture = NULL;

    hr = NineTexture9_new(This, Width, Height, Levels, Usage, Format, Pool,
                          &tex, pSharedHandle);
    if (SUCCEEDED(hr))
        *ppTexture = (IDirect3DTexture9 *)tex;

    return hr;
}

HRESULT NINE_WINAPI
NineDevice9_CreateVolumeTexture( struct NineDevice9 *This,
                                 UINT Width,
                                 UINT Height,
                                 UINT Depth,
                                 UINT Levels,
                                 DWORD Usage,
                                 D3DFORMAT Format,
                                 D3DPOOL Pool,
                                 IDirect3DVolumeTexture9 **ppVolumeTexture,
                                 HANDLE *pSharedHandle )
{
    struct NineVolumeTexture9 *tex;
    HRESULT hr;

    DBG("This=%p Width=%u Height=%u Depth=%u Levels=%u Usage=%s Format=%s Pool=%s "
        "ppOut=%p pSharedHandle=%p\n", This, Width, Height, Depth, Levels,
        nine_D3DUSAGE_to_str(Usage), d3dformat_to_string(Format),
        nine_D3DPOOL_to_str(Pool), ppVolumeTexture, pSharedHandle);

    user_assert(ppVolumeTexture != NULL, D3DERR_INVALIDCALL);

    Usage &= D3DUSAGE_DYNAMIC | D3DUSAGE_NONSECURE |
             D3DUSAGE_SOFTWAREPROCESSING;

    *ppVolumeTexture = NULL;

    hr = NineVolumeTexture9_new(This, Width, Height, Depth, Levels,
                                Usage, Format, Pool, &tex, pSharedHandle);
    if (SUCCEEDED(hr))
        *ppVolumeTexture = (IDirect3DVolumeTexture9 *)tex;

    return hr;
}

HRESULT NINE_WINAPI
NineDevice9_CreateCubeTexture( struct NineDevice9 *This,
                               UINT EdgeLength,
                               UINT Levels,
                               DWORD Usage,
                               D3DFORMAT Format,
                               D3DPOOL Pool,
                               IDirect3DCubeTexture9 **ppCubeTexture,
                               HANDLE *pSharedHandle )
{
    struct NineCubeTexture9 *tex;
    HRESULT hr;

    DBG("This=%p EdgeLength=%u Levels=%u Usage=%s Format=%s Pool=%s ppOut=%p "
        "pSharedHandle=%p\n", This, EdgeLength, Levels,
        nine_D3DUSAGE_to_str(Usage), d3dformat_to_string(Format),
        nine_D3DPOOL_to_str(Pool), ppCubeTexture, pSharedHandle);

    user_assert(ppCubeTexture != NULL, D3DERR_INVALIDCALL);

    Usage &= D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_DEPTHSTENCIL | D3DUSAGE_DYNAMIC |
             D3DUSAGE_NONSECURE | D3DUSAGE_RENDERTARGET |
             D3DUSAGE_SOFTWAREPROCESSING;

    *ppCubeTexture = NULL;

    hr = NineCubeTexture9_new(This, EdgeLength, Levels, Usage, Format, Pool,
                              &tex, pSharedHandle);
    if (SUCCEEDED(hr))
        *ppCubeTexture = (IDirect3DCubeTexture9 *)tex;

    return hr;
}

HRESULT NINE_WINAPI
NineDevice9_CreateVertexBuffer( struct NineDevice9 *This,
                                UINT Length,
                                DWORD Usage,
                                DWORD FVF,
                                D3DPOOL Pool,
                                IDirect3DVertexBuffer9 **ppVertexBuffer,
                                HANDLE *pSharedHandle )
{
    struct NineVertexBuffer9 *buf;
    HRESULT hr;
    D3DVERTEXBUFFER_DESC desc;

    DBG("This=%p Length=%u Usage=%x FVF=%x Pool=%u ppOut=%p pSharedHandle=%p\n",
        This, Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);

    user_assert(ppVertexBuffer != NULL, D3DERR_INVALIDCALL);
    user_assert(!pSharedHandle || Pool == D3DPOOL_DEFAULT, D3DERR_NOTAVAILABLE);

    desc.Format = D3DFMT_VERTEXDATA;
    desc.Type = D3DRTYPE_VERTEXBUFFER;
    desc.Usage = Usage &
        (D3DUSAGE_DONOTCLIP | D3DUSAGE_DYNAMIC | D3DUSAGE_NONSECURE |
         D3DUSAGE_NPATCHES | D3DUSAGE_POINTS | D3DUSAGE_RTPATCHES |
         D3DUSAGE_SOFTWAREPROCESSING | D3DUSAGE_TEXTAPI |
         D3DUSAGE_WRITEONLY);
    desc.Pool = Pool;
    desc.Size = Length;
    desc.FVF = FVF;

    user_assert(!pSharedHandle || Pool == D3DPOOL_DEFAULT, D3DERR_INVALIDCALL);
    user_assert(desc.Usage == Usage, D3DERR_INVALIDCALL);

    hr = NineVertexBuffer9_new(This, &desc, &buf);
    if (SUCCEEDED(hr))
        *ppVertexBuffer = (IDirect3DVertexBuffer9 *)buf;
    return hr;
}

HRESULT NINE_WINAPI
NineDevice9_CreateIndexBuffer( struct NineDevice9 *This,
                               UINT Length,
                               DWORD Usage,
                               D3DFORMAT Format,
                               D3DPOOL Pool,
                               IDirect3DIndexBuffer9 **ppIndexBuffer,
                               HANDLE *pSharedHandle )
{
    struct NineIndexBuffer9 *buf;
    HRESULT hr;
    D3DINDEXBUFFER_DESC desc;

    DBG("This=%p Length=%u Usage=%x Format=%s Pool=%u ppOut=%p "
        "pSharedHandle=%p\n", This, Length, Usage,
        d3dformat_to_string(Format), Pool, ppIndexBuffer, pSharedHandle);

    user_assert(ppIndexBuffer != NULL, D3DERR_INVALIDCALL);
    user_assert(!pSharedHandle || Pool == D3DPOOL_DEFAULT, D3DERR_NOTAVAILABLE);

    desc.Format = Format;
    desc.Type = D3DRTYPE_INDEXBUFFER;
    desc.Usage = Usage &
        (D3DUSAGE_DONOTCLIP | D3DUSAGE_DYNAMIC | D3DUSAGE_NONSECURE |
         D3DUSAGE_NPATCHES | D3DUSAGE_POINTS | D3DUSAGE_RTPATCHES |
         D3DUSAGE_SOFTWAREPROCESSING | D3DUSAGE_WRITEONLY);
    desc.Pool = Pool;
    desc.Size = Length;

    user_assert(!pSharedHandle || Pool == D3DPOOL_DEFAULT, D3DERR_INVALIDCALL);
    user_assert(desc.Usage == Usage, D3DERR_INVALIDCALL);

    hr = NineIndexBuffer9_new(This, &desc, &buf);
    if (SUCCEEDED(hr))
        *ppIndexBuffer = (IDirect3DIndexBuffer9 *)buf;
    return hr;
}

static HRESULT
create_zs_or_rt_surface(struct NineDevice9 *This,
                        unsigned type, /* 0 = RT, 1 = ZS, 2 = plain */
                        D3DPOOL Pool,
                        UINT Width, UINT Height,
                        D3DFORMAT Format,
                        D3DMULTISAMPLE_TYPE MultiSample,
                        DWORD MultisampleQuality,
                        BOOL Discard_or_Lockable,
                        IDirect3DSurface9 **ppSurface,
                        HANDLE *pSharedHandle)
{
    struct NineSurface9 *surface;
    HRESULT hr;
    D3DSURFACE_DESC desc;

    DBG("This=%p type=%u Pool=%s Width=%u Height=%u Format=%s MS=%u Quality=%u "
        "Discard_or_Lockable=%i ppSurface=%p pSharedHandle=%p\n",
        This, type, nine_D3DPOOL_to_str(Pool), Width, Height,
        d3dformat_to_string(Format), MultiSample, MultisampleQuality,
        Discard_or_Lockable, ppSurface, pSharedHandle);

    if (pSharedHandle)
      DBG("FIXME Used shared handle! This option isn't probably handled correctly!\n");

    user_assert(Width && Height, D3DERR_INVALIDCALL);
    user_assert(Pool != D3DPOOL_MANAGED, D3DERR_INVALIDCALL);

    desc.Format = Format;
    desc.Type = D3DRTYPE_SURFACE;
    desc.Usage = 0;
    desc.Pool = Pool;
    desc.MultiSampleType = MultiSample;
    desc.MultiSampleQuality = MultisampleQuality;
    desc.Width = Width;
    desc.Height = Height;
    switch (type) {
    case 0: desc.Usage = D3DUSAGE_RENDERTARGET; break;
    case 1: desc.Usage = D3DUSAGE_DEPTHSTENCIL; break;
    default: assert(type == 2); break;
    }

    hr = NineSurface9_new(This, NULL, NULL, NULL, 0, 0, 0, &desc, &surface);
    if (SUCCEEDED(hr)) {
        *ppSurface = (IDirect3DSurface9 *)surface;

        if (surface->base.resource && Discard_or_Lockable && (type != 1))
            surface->base.resource->flags |= NINE_RESOURCE_FLAG_LOCKABLE;
    }

    return hr;
}

HRESULT NINE_WINAPI
NineDevice9_CreateRenderTarget( struct NineDevice9 *This,
                                UINT Width,
                                UINT Height,
                                D3DFORMAT Format,
                                D3DMULTISAMPLE_TYPE MultiSample,
                                DWORD MultisampleQuality,
                                BOOL Lockable,
                                IDirect3DSurface9 **ppSurface,
                                HANDLE *pSharedHandle )
{
    user_assert(ppSurface != NULL, D3DERR_INVALIDCALL);
    *ppSurface = NULL;
    return create_zs_or_rt_surface(This, 0, D3DPOOL_DEFAULT,
                                   Width, Height, Format,
                                   MultiSample, MultisampleQuality,
                                   Lockable, ppSurface, pSharedHandle);
}

HRESULT NINE_WINAPI
NineDevice9_CreateDepthStencilSurface( struct NineDevice9 *This,
                                       UINT Width,
                                       UINT Height,
                                       D3DFORMAT Format,
                                       D3DMULTISAMPLE_TYPE MultiSample,
                                       DWORD MultisampleQuality,
                                       BOOL Discard,
                                       IDirect3DSurface9 **ppSurface,
                                       HANDLE *pSharedHandle )
{
    user_assert(ppSurface != NULL, D3DERR_INVALIDCALL);
    *ppSurface = NULL;
    if (!depth_stencil_format(Format))
        return D3DERR_NOTAVAILABLE;
    return create_zs_or_rt_surface(This, 1, D3DPOOL_DEFAULT,
                                   Width, Height, Format,
                                   MultiSample, MultisampleQuality,
                                   Discard, ppSurface, pSharedHandle);
}

HRESULT NINE_WINAPI
NineDevice9_UpdateSurface( struct NineDevice9 *This,
                           IDirect3DSurface9 *pSourceSurface,
                           const RECT *pSourceRect,
                           IDirect3DSurface9 *pDestinationSurface,
                           const POINT *pDestPoint )
{
    struct NineSurface9 *dst = NineSurface9(pDestinationSurface);
    struct NineSurface9 *src = NineSurface9(pSourceSurface);
    int copy_width, copy_height;
    RECT destRect;

    DBG("This=%p pSourceSurface=%p pDestinationSurface=%p "
        "pSourceRect=%p pDestPoint=%p\n", This,
        pSourceSurface, pDestinationSurface, pSourceRect, pDestPoint);
    if (pSourceRect)
        DBG("pSourceRect = (%u,%u)-(%u,%u)\n",
            pSourceRect->left, pSourceRect->top,
            pSourceRect->right, pSourceRect->bottom);
    if (pDestPoint)
        DBG("pDestPoint = (%u,%u)\n", pDestPoint->x, pDestPoint->y);

    user_assert(dst && src, D3DERR_INVALIDCALL);

    user_assert(dst->base.pool == D3DPOOL_DEFAULT, D3DERR_INVALIDCALL);
    user_assert(src->base.pool == D3DPOOL_SYSTEMMEM, D3DERR_INVALIDCALL);

    user_assert(dst->desc.MultiSampleType == D3DMULTISAMPLE_NONE, D3DERR_INVALIDCALL);
    user_assert(src->desc.MultiSampleType == D3DMULTISAMPLE_NONE, D3DERR_INVALIDCALL);

    user_assert(!src->lock_count, D3DERR_INVALIDCALL);
    user_assert(!dst->lock_count, D3DERR_INVALIDCALL);

    user_assert(dst->desc.Format == src->desc.Format, D3DERR_INVALIDCALL);
    user_assert(!depth_stencil_format(dst->desc.Format), D3DERR_INVALIDCALL);

    if (pSourceRect) {
        copy_width = pSourceRect->right - pSourceRect->left;
        copy_height = pSourceRect->bottom - pSourceRect->top;

        user_assert(pSourceRect->left >= 0 &&
                    copy_width > 0 &&
                    pSourceRect->right <= src->desc.Width &&
                    pSourceRect->top >= 0 &&
                    copy_height > 0 &&
                    pSourceRect->bottom <= src->desc.Height,
                    D3DERR_INVALIDCALL);
    } else {
        copy_width = src->desc.Width;
        copy_height = src->desc.Height;
    }

    destRect.right = copy_width;
    destRect.bottom = copy_height;

    if (pDestPoint) {
        user_assert(pDestPoint->x >= 0 && pDestPoint->y >= 0,
                    D3DERR_INVALIDCALL);
        destRect.right += pDestPoint->x;
        destRect.bottom += pDestPoint->y;
    }

    user_assert(destRect.right <= dst->desc.Width &&
                destRect.bottom <= dst->desc.Height,
                D3DERR_INVALIDCALL);

    if (compressed_format(dst->desc.Format)) {
        const unsigned w = util_format_get_blockwidth(dst->base.info.format);
        const unsigned h = util_format_get_blockheight(dst->base.info.format);

        if (pDestPoint) {
            user_assert(!(pDestPoint->x % w) && !(pDestPoint->y % h),
                        D3DERR_INVALIDCALL);
        }

        if (pSourceRect) {
            user_assert(!(pSourceRect->left % w) && !(pSourceRect->top % h),
                        D3DERR_INVALIDCALL);
        }
        if (!(copy_width == src->desc.Width &&
              copy_width == dst->desc.Width &&
              copy_height == src->desc.Height &&
              copy_height == dst->desc.Height)) {
            user_assert(!(copy_width  % w) && !(copy_height % h),
                        D3DERR_INVALIDCALL);
        }
    }

    NineSurface9_CopyMemToDefault(dst, src, pDestPoint, pSourceRect);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_UpdateTexture( struct NineDevice9 *This,
                           IDirect3DBaseTexture9 *pSourceTexture,
                           IDirect3DBaseTexture9 *pDestinationTexture )
{
    struct NineBaseTexture9 *dstb = NineBaseTexture9(pDestinationTexture);
    struct NineBaseTexture9 *srcb = NineBaseTexture9(pSourceTexture);
    unsigned l, m;
    unsigned last_src_level, last_dst_level;
    RECT rect;

    DBG("This=%p pSourceTexture=%p pDestinationTexture=%p\n", This,
        pSourceTexture, pDestinationTexture);

    user_assert(pSourceTexture && pDestinationTexture, D3DERR_INVALIDCALL);
    user_assert(pSourceTexture != pDestinationTexture, D3DERR_INVALIDCALL);

    user_assert(dstb->base.pool == D3DPOOL_DEFAULT, D3DERR_INVALIDCALL);
    user_assert(srcb->base.pool == D3DPOOL_SYSTEMMEM, D3DERR_INVALIDCALL);
    user_assert(dstb->base.type == srcb->base.type, D3DERR_INVALIDCALL);
    user_assert(!(srcb->base.usage & D3DUSAGE_AUTOGENMIPMAP) ||
                dstb->base.usage & D3DUSAGE_AUTOGENMIPMAP, D3DERR_INVALIDCALL);

    /* Spec: Failure if
     * . Different formats
     * . Fewer src levels than dst levels (if the opposite, only matching levels
     *   are supposed to be copied)
     * . Levels do not match
     * DDI: Actually the above should pass because of legacy applications
     * Do what you want about these, but you shouldn't crash.
     * However driver can expect that the top dimension is greater for src than dst.
     * Wine tests: Every combination that passes the initial checks should pass.
     * . Different formats => conversion driver and format dependent.
     * . 1 level, but size not matching => copy is done (and even crash if src bigger
     * than dst. For the case where dst bigger, wine doesn't test if a stretch is applied
     * or if a subrect is copied).
     * . 8x8 4 sublevels -> 7x7 2 sublevels => driver dependent, On NV seems to be 4x4 subrect
     * copied to 7x7.
     *
     * From these, the proposal is:
     * . Different formats -> use util_format_translate to translate if possible for surfaces.
     * Accept ARGB/XRGB for Volumes. Do nothing for the other combinations
     * . First level copied -> the first level such that src is smaller or equal to dst first level
     * . number of levels copied -> as long as it fits and textures have levels
     * That should satisfy the constraints (and instead of crashing for some cases we return D3D_OK)
     */

    last_src_level = srcb->level_count-1;
    last_dst_level = dstb->level_count-1;

    for (m = 0; m <= last_src_level; ++m) {
        unsigned w = u_minify(srcb->base.info.width0, m);
        unsigned h = u_minify(srcb->base.info.height0, m);
        unsigned d = u_minify(srcb->base.info.depth0, m);

        if (w <= dstb->base.info.width0 &&
            h <= dstb->base.info.height0 &&
            d <= dstb->base.info.depth0)
            break;
    }
    user_assert(m <= last_src_level, D3D_OK);

    last_dst_level = MIN2(srcb->base.info.last_level - m, last_dst_level);

    if (dstb->base.type == D3DRTYPE_TEXTURE) {
        struct NineTexture9 *dst = NineTexture9(dstb);
        struct NineTexture9 *src = NineTexture9(srcb);

        if (src->dirty_rect.width == 0)
            return D3D_OK;

        pipe_box_to_rect(&rect, &src->dirty_rect);
        for (l = 0; l < m; ++l)
            rect_minify_inclusive(&rect);

        for (l = 0; l <= last_dst_level; ++l, ++m) {
            fit_rect_format_inclusive(dst->base.base.info.format,
                                      &rect,
                                      dst->surfaces[l]->desc.Width,
                                      dst->surfaces[l]->desc.Height);
            NineSurface9_CopyMemToDefault(dst->surfaces[l],
                                          src->surfaces[m],
                                          (POINT *)&rect,
                                          &rect);
            rect_minify_inclusive(&rect);
        }
        u_box_origin_2d(0, 0, &src->dirty_rect);
    } else
    if (dstb->base.type == D3DRTYPE_CUBETEXTURE) {
        struct NineCubeTexture9 *dst = NineCubeTexture9(dstb);
        struct NineCubeTexture9 *src = NineCubeTexture9(srcb);
        unsigned z;

        /* GPUs usually have them stored as arrays of mip-mapped 2D textures. */
        for (z = 0; z < 6; ++z) {
            if (src->dirty_rect[z].width == 0)
                continue;

            pipe_box_to_rect(&rect, &src->dirty_rect[z]);
            for (l = 0; l < m; ++l)
                rect_minify_inclusive(&rect);

            for (l = 0; l <= last_dst_level; ++l, ++m) {
                fit_rect_format_inclusive(dst->base.base.info.format,
                                          &rect,
                                          dst->surfaces[l * 6 + z]->desc.Width,
                                          dst->surfaces[l * 6 + z]->desc.Height);
                NineSurface9_CopyMemToDefault(dst->surfaces[l * 6 + z],
                                              src->surfaces[m * 6 + z],
                                              (POINT *)&rect,
                                              &rect);
                rect_minify_inclusive(&rect);
            }
            u_box_origin_2d(0, 0, &src->dirty_rect[z]);
            m -= l;
        }
    } else
    if (dstb->base.type == D3DRTYPE_VOLUMETEXTURE) {
        struct NineVolumeTexture9 *dst = NineVolumeTexture9(dstb);
        struct NineVolumeTexture9 *src = NineVolumeTexture9(srcb);

        if (src->dirty_box.width == 0)
            return D3D_OK;
        for (l = 0; l <= last_dst_level; ++l, ++m)
            NineVolume9_CopyMemToDefault(dst->volumes[l],
                                         src->volumes[m], 0, 0, 0, NULL);
        u_box_3d(0, 0, 0, 0, 0, 0, &src->dirty_box);
    } else{
        assert(!"invalid texture type");
    }

    if (dstb->base.usage & D3DUSAGE_AUTOGENMIPMAP) {
        dstb->dirty_mip = true;
        NineBaseTexture9_GenerateMipSubLevels(dstb);
    }

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetRenderTargetData( struct NineDevice9 *This,
                                 IDirect3DSurface9 *pRenderTarget,
                                 IDirect3DSurface9 *pDestSurface )
{
    struct NineSurface9 *dst = NineSurface9(pDestSurface);
    struct NineSurface9 *src = NineSurface9(pRenderTarget);

    DBG("This=%p pRenderTarget=%p pDestSurface=%p\n",
        This, pRenderTarget, pDestSurface);

    user_assert(pRenderTarget && pDestSurface, D3DERR_INVALIDCALL);

    user_assert(dst->desc.Pool == D3DPOOL_SYSTEMMEM, D3DERR_INVALIDCALL);
    user_assert(src->desc.Pool == D3DPOOL_DEFAULT, D3DERR_INVALIDCALL);

    user_assert(dst->desc.MultiSampleType < 2, D3DERR_INVALIDCALL);
    user_assert(src->desc.MultiSampleType < 2, D3DERR_INVALIDCALL);

    user_assert(src->desc.Width == dst->desc.Width, D3DERR_INVALIDCALL);
    user_assert(src->desc.Height == dst->desc.Height, D3DERR_INVALIDCALL);

    user_assert(src->desc.Format != D3DFMT_NULL, D3DERR_INVALIDCALL);

    NineSurface9_CopyDefaultToMem(dst, src);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetFrontBufferData( struct NineDevice9 *This,
                                UINT iSwapChain,
                                IDirect3DSurface9 *pDestSurface )
{
    DBG("This=%p iSwapChain=%u pDestSurface=%p\n", This,
        iSwapChain, pDestSurface);

    user_assert(pDestSurface != NULL, D3DERR_INVALIDCALL);
    user_assert(iSwapChain < This->nswapchains, D3DERR_INVALIDCALL);

    return NineSwapChain9_GetFrontBufferData(This->swapchains[iSwapChain],
                                             pDestSurface);
}

HRESULT NINE_WINAPI
NineDevice9_StretchRect( struct NineDevice9 *This,
                         IDirect3DSurface9 *pSourceSurface,
                         const RECT *pSourceRect,
                         IDirect3DSurface9 *pDestSurface,
                         const RECT *pDestRect,
                         D3DTEXTUREFILTERTYPE Filter )
{
    struct pipe_screen *screen = This->screen;
    struct NineSurface9 *dst = NineSurface9(pDestSurface);
    struct NineSurface9 *src = NineSurface9(pSourceSurface);
    struct pipe_resource *dst_res, *src_res;
    bool zs;
    struct pipe_blit_info blit;
    bool scaled, clamped, ms, flip_x = false, flip_y = false;

    DBG("This=%p pSourceSurface=%p pSourceRect=%p pDestSurface=%p "
        "pDestRect=%p Filter=%u\n",
        This, pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
    if (pSourceRect)
        DBG("pSourceRect=(%u,%u)-(%u,%u)\n",
            pSourceRect->left, pSourceRect->top,
            pSourceRect->right, pSourceRect->bottom);
    if (pDestRect)
        DBG("pDestRect=(%u,%u)-(%u,%u)\n", pDestRect->left, pDestRect->top,
            pDestRect->right, pDestRect->bottom);

    user_assert(pSourceSurface && pDestSurface, D3DERR_INVALIDCALL);
    user_assert(dst->base.pool == D3DPOOL_DEFAULT &&
                src->base.pool == D3DPOOL_DEFAULT, D3DERR_INVALIDCALL);

    dst_res = NineSurface9_GetResource(dst);
    src_res = NineSurface9_GetResource(src);
    zs = util_format_is_depth_or_stencil(dst_res->format);
    user_assert(!zs || !This->in_scene, D3DERR_INVALIDCALL);
    user_assert(!zs || !pSourceRect ||
                (pSourceRect->left == 0 &&
                 pSourceRect->top == 0 &&
                 pSourceRect->right == src->desc.Width &&
                 pSourceRect->bottom == src->desc.Height), D3DERR_INVALIDCALL);
    user_assert(!zs || !pDestRect ||
                (pDestRect->left == 0 &&
                 pDestRect->top == 0 &&
                 pDestRect->right == dst->desc.Width &&
                 pDestRect->bottom == dst->desc.Height), D3DERR_INVALIDCALL);
    user_assert(!zs ||
                (dst->desc.Width == src->desc.Width &&
                 dst->desc.Height == src->desc.Height), D3DERR_INVALIDCALL);
    user_assert(zs || !util_format_is_depth_or_stencil(src_res->format),
                D3DERR_INVALIDCALL);
    user_assert(!zs || dst->desc.Format == src->desc.Format,
                D3DERR_INVALIDCALL);
    user_assert(screen->is_format_supported(screen, src_res->format,
                                            src_res->target,
                                            src_res->nr_samples,
                                            src_res->nr_storage_samples,
                                            PIPE_BIND_SAMPLER_VIEW),
                D3DERR_INVALIDCALL);

    /* We might want to permit these, but wine thinks we shouldn't. */
    user_assert(!pDestRect ||
                (pDestRect->left <= pDestRect->right &&
                 pDestRect->top <= pDestRect->bottom), D3DERR_INVALIDCALL);
    user_assert(!pSourceRect ||
                (pSourceRect->left <= pSourceRect->right &&
                 pSourceRect->top <= pSourceRect->bottom), D3DERR_INVALIDCALL);

    memset(&blit, 0, sizeof(blit));
    blit.dst.resource = dst_res;
    blit.dst.level = dst->level;
    blit.dst.box.z = dst->layer;
    blit.dst.box.depth = 1;
    blit.dst.format = dst_res->format;
    if (pDestRect) {
        flip_x = pDestRect->left > pDestRect->right;
        if (flip_x) {
            blit.dst.box.x = pDestRect->right;
            blit.dst.box.width = pDestRect->left - pDestRect->right;
        } else {
            blit.dst.box.x = pDestRect->left;
            blit.dst.box.width = pDestRect->right - pDestRect->left;
        }
        flip_y = pDestRect->top > pDestRect->bottom;
        if (flip_y) {
            blit.dst.box.y = pDestRect->bottom;
            blit.dst.box.height = pDestRect->top - pDestRect->bottom;
        } else {
            blit.dst.box.y = pDestRect->top;
            blit.dst.box.height = pDestRect->bottom - pDestRect->top;
        }
    } else {
        blit.dst.box.x = 0;
        blit.dst.box.y = 0;
        blit.dst.box.width = dst->desc.Width;
        blit.dst.box.height = dst->desc.Height;
    }
    blit.src.resource = src_res;
    blit.src.level = src->level;
    blit.src.box.z = src->layer;
    blit.src.box.depth = 1;
    blit.src.format = src_res->format;
    if (pSourceRect) {
        if (flip_x ^ (pSourceRect->left > pSourceRect->right)) {
            blit.src.box.x = pSourceRect->right;
            blit.src.box.width = pSourceRect->left - pSourceRect->right;
        } else {
            blit.src.box.x = pSourceRect->left;
            blit.src.box.width = pSourceRect->right - pSourceRect->left;
        }
        if (flip_y ^ (pSourceRect->top > pSourceRect->bottom)) {
            blit.src.box.y = pSourceRect->bottom;
            blit.src.box.height = pSourceRect->top - pSourceRect->bottom;
        } else {
            blit.src.box.y = pSourceRect->top;
            blit.src.box.height = pSourceRect->bottom - pSourceRect->top;
        }
    } else {
        blit.src.box.x = flip_x ? src->desc.Width : 0;
        blit.src.box.y = flip_y ? src->desc.Height : 0;
        blit.src.box.width = flip_x ? -src->desc.Width : src->desc.Width;
        blit.src.box.height = flip_y ? -src->desc.Height : src->desc.Height;
    }
    blit.mask = zs ? PIPE_MASK_ZS : PIPE_MASK_RGBA;
    blit.filter = Filter == D3DTEXF_LINEAR ?
       PIPE_TEX_FILTER_LINEAR : PIPE_TEX_FILTER_NEAREST;
    blit.scissor_enable = false;
    blit.alpha_blend = false;

    /* If both of a src and dst dimension are negative, flip them. */
    if (blit.dst.box.width < 0 && blit.src.box.width < 0) {
        blit.dst.box.width = -blit.dst.box.width;
        blit.src.box.width = -blit.src.box.width;
    }
    if (blit.dst.box.height < 0 && blit.src.box.height < 0) {
        blit.dst.box.height = -blit.dst.box.height;
        blit.src.box.height = -blit.src.box.height;
    }
    scaled =
        blit.dst.box.width != blit.src.box.width ||
        blit.dst.box.height != blit.src.box.height;

    user_assert(!scaled || dst != src, D3DERR_INVALIDCALL);
    user_assert(!scaled ||
                !NineSurface9_IsOffscreenPlain(dst), D3DERR_INVALIDCALL);
    user_assert(!NineSurface9_IsOffscreenPlain(dst) ||
                NineSurface9_IsOffscreenPlain(src), D3DERR_INVALIDCALL);
    user_assert(NineSurface9_IsOffscreenPlain(dst) ||
                dst->desc.Usage & (D3DUSAGE_RENDERTARGET | D3DUSAGE_DEPTHSTENCIL),
                D3DERR_INVALIDCALL);
    user_assert(!scaled ||
                (!util_format_is_compressed(dst->base.info.format) &&
                 !util_format_is_compressed(src->base.info.format)),
                D3DERR_INVALIDCALL);

    user_warn(src == dst &&
              u_box_test_intersection_2d(&blit.src.box, &blit.dst.box));

    /* Check for clipping/clamping: */
    {
        struct pipe_box box;
        int xy;

        xy = u_box_clip_2d(&box, &blit.dst.box,
                           dst->desc.Width, dst->desc.Height);
        if (xy < 0)
            return D3D_OK;
        if (xy == 0)
            xy = u_box_clip_2d(&box, &blit.src.box,
                               src->desc.Width, src->desc.Height);
        clamped = !!xy;
    }

    ms = (dst->desc.MultiSampleType != src->desc.MultiSampleType) ||
         (dst->desc.MultiSampleQuality != src->desc.MultiSampleQuality);

    if (clamped || scaled || (blit.dst.format != blit.src.format) || ms) {
        DBG("using pipe->blit()\n");
        /* TODO: software scaling */
        user_assert(screen->is_format_supported(screen, dst_res->format,
                                                dst_res->target,
                                                dst_res->nr_samples,
                                                dst_res->nr_storage_samples,
                                                zs ? PIPE_BIND_DEPTH_STENCIL :
                                                PIPE_BIND_RENDER_TARGET),
                    D3DERR_INVALIDCALL);

        nine_context_blit(This, (struct NineUnknown *)dst,
                          (struct NineUnknown *)src, &blit);
    } else {
        assert(blit.dst.box.x >= 0 && blit.dst.box.y >= 0 &&
               blit.src.box.x >= 0 && blit.src.box.y >= 0 &&
               blit.dst.box.x + blit.dst.box.width <= dst->desc.Width &&
               blit.src.box.x + blit.src.box.width <= src->desc.Width &&
               blit.dst.box.y + blit.dst.box.height <= dst->desc.Height &&
               blit.src.box.y + blit.src.box.height <= src->desc.Height);
        /* Or drivers might crash ... */
        DBG("Using resource_copy_region.\n");
        nine_context_resource_copy_region(This, (struct NineUnknown *)dst,
                                          (struct NineUnknown *)src,
                                          blit.dst.resource, blit.dst.level,
                                          &blit.dst.box,
                                          blit.src.resource, blit.src.level,
                                          &blit.src.box);
    }

    /* Communicate the container it needs to update sublevels - if apply */
    NineSurface9_MarkContainerDirty(dst);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_ColorFill( struct NineDevice9 *This,
                       IDirect3DSurface9 *pSurface,
                       const RECT *pRect,
                       D3DCOLOR color )
{
    struct NineSurface9 *surf = NineSurface9(pSurface);
    unsigned x, y, w, h;

    DBG("This=%p pSurface=%p pRect=%p color=%08x\n", This,
        pSurface, pRect, color);
    if (pRect)
        DBG("pRect=(%u,%u)-(%u,%u)\n", pRect->left, pRect->top,
            pRect->right, pRect->bottom);

    user_assert(pSurface != NULL, D3DERR_INVALIDCALL);

    user_assert(surf->base.pool == D3DPOOL_DEFAULT, D3DERR_INVALIDCALL);

    user_assert((surf->base.usage & D3DUSAGE_RENDERTARGET) ||
                NineSurface9_IsOffscreenPlain(surf), D3DERR_INVALIDCALL);

    user_assert(surf->desc.Format != D3DFMT_NULL, D3D_OK);

    if (pRect) {
        x = pRect->left;
        y = pRect->top;
        w = pRect->right - pRect->left;
        h = pRect->bottom - pRect->top;
        /* Wine tests: */
        if (compressed_format(surf->desc.Format)) {
           const unsigned bw = util_format_get_blockwidth(surf->base.info.format);
           const unsigned bh = util_format_get_blockheight(surf->base.info.format);

           user_assert(!(x % bw) && !(y % bh) && !(w % bw) && !(h % bh),
                       D3DERR_INVALIDCALL);
        }
    } else{
        x = 0;
        y = 0;
        w = surf->desc.Width;
        h = surf->desc.Height;
    }

    if (surf->base.info.bind & PIPE_BIND_RENDER_TARGET) {
        nine_context_clear_render_target(This, surf, color, x, y, w, h);
    } else {
        D3DLOCKED_RECT lock;
        union util_color uc;
        HRESULT hr;
        /* XXX: lock pRect and fix util_fill_rect */
        hr = NineSurface9_LockRect(surf, &lock, NULL, pRect ? 0 : D3DLOCK_DISCARD);
        if (FAILED(hr))
            return hr;
        util_pack_color_ub(color >> 16, color >> 8, color >> 0, color >> 24,
                           surf->base.info.format, &uc);
        util_fill_rect(lock.pBits, surf->base.info.format,lock.Pitch,
                       x, y, w, h, &uc);
        NineSurface9_UnlockRect(surf);
    }

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_CreateOffscreenPlainSurface( struct NineDevice9 *This,
                                         UINT Width,
                                         UINT Height,
                                         D3DFORMAT Format,
                                         D3DPOOL Pool,
                                         IDirect3DSurface9 **ppSurface,
                                         HANDLE *pSharedHandle )
{
    HRESULT hr;

    DBG("This=%p Width=%u Height=%u Format=%s(0x%x) Pool=%u "
        "ppSurface=%p pSharedHandle=%p\n", This,
        Width, Height, d3dformat_to_string(Format), Format, Pool,
        ppSurface, pSharedHandle);

    user_assert(ppSurface != NULL, D3DERR_INVALIDCALL);
    *ppSurface = NULL;
    user_assert(!pSharedHandle || Pool == D3DPOOL_DEFAULT
                               || Pool == D3DPOOL_SYSTEMMEM, D3DERR_INVALIDCALL);
    user_assert(Pool != D3DPOOL_MANAGED, D3DERR_INVALIDCALL);

    /* Can be used with StretchRect and ColorFill. It's also always lockable.
     */
    hr = create_zs_or_rt_surface(This, 2, Pool, Width, Height,
                                 Format,
                                 D3DMULTISAMPLE_NONE, 0,
                                 true,
                                 ppSurface, pSharedHandle);
    if (FAILED(hr))
        DBG("Failed to create surface.\n");
    return hr;
}

HRESULT NINE_WINAPI
NineDevice9_SetRenderTarget( struct NineDevice9 *This,
                             DWORD RenderTargetIndex,
                             IDirect3DSurface9 *pRenderTarget )
{
    struct NineSurface9 *rt = NineSurface9(pRenderTarget);
    const unsigned i = RenderTargetIndex;

    DBG("This=%p RenderTargetIndex=%u pRenderTarget=%p\n", This,
        RenderTargetIndex, pRenderTarget);

    user_assert(i < This->caps.NumSimultaneousRTs, D3DERR_INVALIDCALL);
    user_assert(i != 0 || pRenderTarget, D3DERR_INVALIDCALL);
    user_assert(!pRenderTarget ||
                rt->desc.Usage & D3DUSAGE_RENDERTARGET, D3DERR_INVALIDCALL);

    if (i == 0) {
        This->state.viewport.X = 0;
        This->state.viewport.Y = 0;
        This->state.viewport.Width = rt->desc.Width;
        This->state.viewport.Height = rt->desc.Height;
        This->state.viewport.MinZ = 0.0f;
        This->state.viewport.MaxZ = 1.0f;

        This->state.scissor.minx = 0;
        This->state.scissor.miny = 0;
        This->state.scissor.maxx = rt->desc.Width;
        This->state.scissor.maxy = rt->desc.Height;
        nine_context_set_viewport(This, &This->state.viewport);
        nine_context_set_scissor(This, &This->state.scissor);
    }

    if (This->state.rt[i] != NineSurface9(pRenderTarget))
        nine_bind(&This->state.rt[i], pRenderTarget);

    nine_context_set_render_target(This, i, rt);
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetRenderTarget( struct NineDevice9 *This,
                             DWORD RenderTargetIndex,
                             IDirect3DSurface9 **ppRenderTarget )
{
    const unsigned i = RenderTargetIndex;

    user_assert(i < This->caps.NumSimultaneousRTs, D3DERR_INVALIDCALL);
    user_assert(ppRenderTarget, D3DERR_INVALIDCALL);

    *ppRenderTarget = (IDirect3DSurface9 *)This->state.rt[i];
    if (!This->state.rt[i])
        return D3DERR_NOTFOUND;

    NineUnknown_AddRef(NineUnknown(This->state.rt[i]));
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetDepthStencilSurface( struct NineDevice9 *This,
                                    IDirect3DSurface9 *pNewZStencil )
{
    struct NineSurface9 *ds = NineSurface9(pNewZStencil);
    DBG("This=%p pNewZStencil=%p\n", This, pNewZStencil);

    user_assert(!ds || util_format_is_depth_or_stencil(ds->base.info.format),
                D3DERR_INVALIDCALL);

    if (This->state.ds != ds) {
        nine_bind(&This->state.ds, ds);
        nine_context_set_depth_stencil(This, ds);
    }
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetDepthStencilSurface( struct NineDevice9 *This,
                                    IDirect3DSurface9 **ppZStencilSurface )
{
    user_assert(ppZStencilSurface, D3DERR_INVALIDCALL);

    *ppZStencilSurface = (IDirect3DSurface9 *)This->state.ds;
    if (!This->state.ds)
        return D3DERR_NOTFOUND;

    NineUnknown_AddRef(NineUnknown(This->state.ds));
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_BeginScene( struct NineDevice9 *This )
{
    DBG("This=%p\n", This);
    user_assert(!This->in_scene, D3DERR_INVALIDCALL);
    This->in_scene = true;
    /* Do we want to do anything else here ? */
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_EndScene( struct NineDevice9 *This )
{
    DBG("This=%p\n", This);
    user_assert(This->in_scene, D3DERR_INVALIDCALL);
    This->in_scene = false;
    This->end_scene_since_present++;
    /* EndScene() is supposed to flush the GPU commands.
     * The idea is to flush ahead of the Present() call.
     * (Apps could take advantage of this by inserting CPU
     * work between EndScene() and Present()).
     * Most apps will have one EndScene per frame.
     * Some will have 2 or 3.
     * Some bad behaving apps do a lot of them.
     * As flushing has a cost, do it only once. */
    if (This->end_scene_since_present <= 1) {
        nine_context_pipe_flush(This);
        nine_csmt_flush(This);
    }
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_Clear( struct NineDevice9 *This,
                   DWORD Count,
                   const D3DRECT *pRects,
                   DWORD Flags,
                   D3DCOLOR Color,
                   float Z,
                   DWORD Stencil )
{
    struct NineSurface9 *zsbuf_surf = This->state.ds;

    DBG("This=%p Count=%u pRects=%p Flags=%x Color=%08x Z=%f Stencil=%x\n",
        This, Count, pRects, Flags, Color, Z, Stencil);

    user_assert(This->state.ds || !(Flags & NINED3DCLEAR_DEPTHSTENCIL),
                D3DERR_INVALIDCALL);
    user_assert(!(Flags & D3DCLEAR_STENCIL) ||
                (zsbuf_surf &&
                 util_format_is_depth_and_stencil(zsbuf_surf->base.info.format)),
                D3DERR_INVALIDCALL);
#ifdef NINE_STRICT
    user_assert((Count && pRects) || (!Count && !pRects), D3DERR_INVALIDCALL);
#else
    user_warn((pRects && !Count) || (!pRects && Count));
    if (pRects && !Count)
        return D3D_OK;
    if (!pRects)
        Count = 0;
#endif

    nine_context_clear_fb(This, Count, pRects, Flags, Color, Z, Stencil);
    return D3D_OK;
}

static void
nine_D3DMATRIX_print(const D3DMATRIX *M)
{
    DBG("\n(%f %f %f %f)\n"
        "(%f %f %f %f)\n"
        "(%f %f %f %f)\n"
        "(%f %f %f %f)\n",
        M->m[0][0], M->m[0][1], M->m[0][2], M->m[0][3],
        M->m[1][0], M->m[1][1], M->m[1][2], M->m[1][3],
        M->m[2][0], M->m[2][1], M->m[2][2], M->m[2][3],
        M->m[3][0], M->m[3][1], M->m[3][2], M->m[3][3]);
}

HRESULT NINE_WINAPI
NineDevice9_SetTransform( struct NineDevice9 *This,
                          D3DTRANSFORMSTATETYPE State,
                          const D3DMATRIX *pMatrix )
{
    struct nine_state *state = This->update;
    D3DMATRIX *M = nine_state_access_transform(&state->ff, State, true);

    DBG("This=%p State=%d pMatrix=%p\n", This, State, pMatrix);

    user_assert(pMatrix, D3DERR_INVALIDCALL);
    user_assert(M, D3DERR_INVALIDCALL);
    nine_D3DMATRIX_print(pMatrix);

    *M = *pMatrix;
    if (unlikely(This->is_recording)) {
        state->ff.changed.transform[State / 32] |= 1 << (State % 32);
        state->changed.group |= NINE_STATE_FF_VSTRANSF;
    } else
        nine_context_set_transform(This, State, pMatrix);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetTransform( struct NineDevice9 *This,
                          D3DTRANSFORMSTATETYPE State,
                          D3DMATRIX *pMatrix )
{
    D3DMATRIX *M;

    user_assert(!This->pure, D3DERR_INVALIDCALL);
    M = nine_state_access_transform(&This->state.ff, State, false);
    user_assert(pMatrix, D3DERR_INVALIDCALL);
    user_assert(M, D3DERR_INVALIDCALL);
    *pMatrix = *M;
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_MultiplyTransform( struct NineDevice9 *This,
                               D3DTRANSFORMSTATETYPE State,
                               const D3DMATRIX *pMatrix )
{
    struct nine_state *state = This->update;
    D3DMATRIX T;
    D3DMATRIX *M = nine_state_access_transform(&state->ff, State, true);

    DBG("This=%p State=%d pMatrix=%p\n", This, State, pMatrix);

    user_assert(pMatrix, D3DERR_INVALIDCALL);
    user_assert(M, D3DERR_INVALIDCALL);

    nine_d3d_matrix_matrix_mul(&T, pMatrix, M);
    return NineDevice9_SetTransform(This, State, &T);
}

HRESULT NINE_WINAPI
NineDevice9_SetViewport( struct NineDevice9 *This,
                         const D3DVIEWPORT9 *pViewport )
{
    struct nine_state *state = This->update;

    DBG("X=%u Y=%u W=%u H=%u MinZ=%f MaxZ=%f\n",
        pViewport->X, pViewport->Y, pViewport->Width, pViewport->Height,
        pViewport->MinZ, pViewport->MaxZ);

    user_assert(pViewport != NULL, D3DERR_INVALIDCALL);
    state->viewport = *pViewport;
    nine_context_set_viewport(This, pViewport);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetViewport( struct NineDevice9 *This,
                         D3DVIEWPORT9 *pViewport )
{
    user_assert(pViewport != NULL, D3DERR_INVALIDCALL);
    *pViewport = This->state.viewport;
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetMaterial( struct NineDevice9 *This,
                         const D3DMATERIAL9 *pMaterial )
{
    struct nine_state *state = This->update;

    DBG("This=%p pMaterial=%p\n", This, pMaterial);
    if (pMaterial)
        nine_dump_D3DMATERIAL9(DBG_FF, pMaterial);

    user_assert(pMaterial, E_POINTER);

    state->ff.material = *pMaterial;
    if (unlikely(This->is_recording))
        state->changed.group |= NINE_STATE_FF_MATERIAL;
    else
        nine_context_set_material(This, pMaterial);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetMaterial( struct NineDevice9 *This,
                         D3DMATERIAL9 *pMaterial )
{
    user_assert(!This->pure, D3DERR_INVALIDCALL);
    user_assert(pMaterial, E_POINTER);
    *pMaterial = This->state.ff.material;
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetLight( struct NineDevice9 *This,
                      DWORD Index,
                      const D3DLIGHT9 *pLight )
{
    struct nine_state *state = This->update;
    HRESULT hr;

    DBG("This=%p Index=%u pLight=%p\n", This, Index, pLight);
    if (pLight)
        nine_dump_D3DLIGHT9(DBG_FF, pLight);

    user_assert(pLight, D3DERR_INVALIDCALL);
    user_assert(pLight->Type < NINED3DLIGHT_INVALID, D3DERR_INVALIDCALL);

    user_assert(Index < NINE_MAX_LIGHTS, D3DERR_INVALIDCALL); /* sanity */

    hr = nine_state_set_light(&state->ff, Index, pLight);
    if (hr != D3D_OK)
        return hr;

    if (pLight->Type != D3DLIGHT_DIRECTIONAL &&
        pLight->Attenuation0 == 0.0f &&
        pLight->Attenuation1 == 0.0f &&
        pLight->Attenuation2 == 0.0f) {
        DBG("Warning: all D3DLIGHT9.Attenuation[i] are 0\n");
    }

    if (unlikely(This->is_recording))
        state->changed.group |= NINE_STATE_FF_LIGHTING;
    else
        nine_context_set_light(This, Index, pLight);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetLight( struct NineDevice9 *This,
                      DWORD Index,
                      D3DLIGHT9 *pLight )
{
    const struct nine_state *state = &This->state;

    user_assert(!This->pure, D3DERR_INVALIDCALL);
    user_assert(pLight, D3DERR_INVALIDCALL);
    user_assert(Index < state->ff.num_lights, D3DERR_INVALIDCALL);
    user_assert(state->ff.light[Index].Type < NINED3DLIGHT_INVALID,
                D3DERR_INVALIDCALL);

    *pLight = state->ff.light[Index];

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_LightEnable( struct NineDevice9 *This,
                         DWORD Index,
                         BOOL Enable )
{
    struct nine_state *state = This->update;

    DBG("This=%p Index=%u Enable=%i\n", This, Index, Enable);

    if (Index >= state->ff.num_lights ||
        state->ff.light[Index].Type == NINED3DLIGHT_INVALID) {
        /* This should create a default light. */
        D3DLIGHT9 light;
        memset(&light, 0, sizeof(light));
        light.Type = D3DLIGHT_DIRECTIONAL;
        light.Diffuse.r = 1.0f;
        light.Diffuse.g = 1.0f;
        light.Diffuse.b = 1.0f;
        light.Direction.z = 1.0f;
        NineDevice9_SetLight(This, Index, &light);
    }

    nine_state_light_enable(&state->ff, Index, Enable);
    if (likely(!This->is_recording))
        nine_context_light_enable(This, Index, Enable);
    else
        state->changed.group |= NINE_STATE_FF_LIGHTING;

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetLightEnable( struct NineDevice9 *This,
                            DWORD Index,
                            BOOL *pEnable )
{
    const struct nine_state *state = &This->state;
    unsigned i;

    user_assert(!This->pure, D3DERR_INVALIDCALL);
    user_assert(pEnable != NULL, D3DERR_INVALIDCALL);
    user_assert(Index < state->ff.num_lights, D3DERR_INVALIDCALL);
    user_assert(state->ff.light[Index].Type < NINED3DLIGHT_INVALID,
                D3DERR_INVALIDCALL);

    for (i = 0; i < state->ff.num_lights_active; ++i)
        if (state->ff.active_light[i] == Index)
            break;

    *pEnable = i != state->ff.num_lights_active ? 128 : 0; // Taken from wine

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetClipPlane( struct NineDevice9 *This,
                          DWORD Index,
                          const float *pPlane )
{
    struct nine_state *state = This->update;

    user_assert(pPlane, D3DERR_INVALIDCALL);

    DBG("This=%p Index=%u pPlane=%f %f %f %f\n", This, Index,
        pPlane[0], pPlane[1],
        pPlane[2], pPlane[3]);

    user_assert(Index < PIPE_MAX_CLIP_PLANES, D3DERR_INVALIDCALL);

    memcpy(&state->clip.ucp[Index][0], pPlane, sizeof(state->clip.ucp[0]));
    if (unlikely(This->is_recording))
        state->changed.ucp |= 1 << Index;
    else
        nine_context_set_clip_plane(This, Index, (struct nine_clipplane *)pPlane);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetClipPlane( struct NineDevice9 *This,
                          DWORD Index,
                          float *pPlane )
{
    const struct nine_state *state = &This->state;

    user_assert(!This->pure, D3DERR_INVALIDCALL);
    user_assert(pPlane != NULL, D3DERR_INVALIDCALL);
    user_assert(Index < PIPE_MAX_CLIP_PLANES, D3DERR_INVALIDCALL);

    memcpy(pPlane, &state->clip.ucp[Index][0], sizeof(state->clip.ucp[0]));
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetRenderState( struct NineDevice9 *This,
                            D3DRENDERSTATETYPE State,
                            DWORD Value )
{
    struct nine_state *state = This->update;

    DBG("This=%p State=%u(%s) Value=%08x\n", This,
        State, nine_d3drs_to_string(State), Value);

    user_assert(State < D3DRS_COUNT, D3D_OK);

    if (unlikely(This->is_recording)) {
        state->rs_advertised[State] = Value;
        /* only need to record changed render states for stateblocks */
        state->changed.rs[State / 32] |= 1 << (State % 32);
        return D3D_OK;
    }

    if (state->rs_advertised[State] == Value)
        return D3D_OK;

    state->rs_advertised[State] = Value;
    nine_context_set_render_state(This, State, Value);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetRenderState( struct NineDevice9 *This,
                            D3DRENDERSTATETYPE State,
                            DWORD *pValue )
{
    user_assert(!This->pure, D3DERR_INVALIDCALL);
    user_assert(pValue != NULL, D3DERR_INVALIDCALL);
    /* TODO: This needs tests */
    if (State >= D3DRS_COUNT) {
        *pValue = 0;
        return D3D_OK;
    }

    *pValue = This->state.rs_advertised[State];
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_CreateStateBlock( struct NineDevice9 *This,
                              D3DSTATEBLOCKTYPE Type,
                              IDirect3DStateBlock9 **ppSB )
{
    struct NineStateBlock9 *nsb;
    struct nine_state *dst;
    HRESULT hr;
    enum nine_stateblock_type type;
    unsigned s;

    DBG("This=%p Type=%u ppSB=%p\n", This, Type, ppSB);

    user_assert(ppSB != NULL, D3DERR_INVALIDCALL);
    user_assert(Type == D3DSBT_ALL ||
                Type == D3DSBT_VERTEXSTATE ||
                Type == D3DSBT_PIXELSTATE, D3DERR_INVALIDCALL);

    switch (Type) {
    case D3DSBT_VERTEXSTATE: type = NINESBT_VERTEXSTATE; break;
    case D3DSBT_PIXELSTATE:  type = NINESBT_PIXELSTATE; break;
    default:
       type = NINESBT_ALL;
       break;
    }

    hr = NineStateBlock9_new(This, &nsb, type);
    if (FAILED(hr))
       return hr;
    *ppSB = (IDirect3DStateBlock9 *)nsb;
    dst = &nsb->state;

    dst->changed.group = NINE_STATE_SAMPLER;

    if (Type == D3DSBT_ALL || Type == D3DSBT_VERTEXSTATE) {
       dst->changed.group |=
           NINE_STATE_FF_LIGHTING |
           NINE_STATE_VS | NINE_STATE_VS_CONST |
           NINE_STATE_VDECL;
       /* TODO: texture/sampler state */
       memcpy(dst->changed.rs,
              nine_render_states_vertex, sizeof(dst->changed.rs));
       nine_ranges_insert(&dst->changed.vs_const_f, 0, This->may_swvp ? NINE_MAX_CONST_F_SWVP : This->max_vs_const_f,
                          &This->range_pool);
       nine_ranges_insert(&dst->changed.vs_const_i, 0, This->may_swvp ? NINE_MAX_CONST_I_SWVP : NINE_MAX_CONST_I,
                          &This->range_pool);
       nine_ranges_insert(&dst->changed.vs_const_b, 0, This->may_swvp ? NINE_MAX_CONST_B_SWVP : NINE_MAX_CONST_B,
                          &This->range_pool);
       for (s = 0; s < NINE_MAX_SAMPLERS; ++s)
           dst->changed.sampler[s] |= 1 << D3DSAMP_DMAPOFFSET;
       if (This->state.ff.num_lights) {
           dst->ff.num_lights = This->state.ff.num_lights;
           /* zero'd -> light type won't be NINED3DLIGHT_INVALID, so
            * all currently existing lights will be captured
            */
           dst->ff.light = CALLOC(This->state.ff.num_lights,
                                  sizeof(D3DLIGHT9));
           if (!dst->ff.light) {
               nine_bind(ppSB, NULL);
               return E_OUTOFMEMORY;
           }
       }
    }
    if (Type == D3DSBT_ALL || Type == D3DSBT_PIXELSTATE) {
       dst->changed.group |=
          NINE_STATE_PS | NINE_STATE_PS_CONST | NINE_STATE_FF_PS_CONSTS;
       memcpy(dst->changed.rs,
              nine_render_states_pixel, sizeof(dst->changed.rs));
       nine_ranges_insert(&dst->changed.ps_const_f, 0, NINE_MAX_CONST_F_PS3,
                          &This->range_pool);
       dst->changed.ps_const_i = 0xffff;
       dst->changed.ps_const_b = 0xffff;
       for (s = 0; s < NINE_MAX_SAMPLERS; ++s)
           dst->changed.sampler[s] |= 0x1ffe;
       for (s = 0; s < NINE_MAX_TEXTURE_STAGES; ++s) {
           dst->ff.changed.tex_stage[s][0] |= 0xffffffff;
           dst->ff.changed.tex_stage[s][1] |= 0xffffffff;
       }
    }
    if (Type == D3DSBT_ALL) {
       dst->changed.group |=
          NINE_STATE_VIEWPORT |
          NINE_STATE_SCISSOR |
          NINE_STATE_IDXBUF |
          NINE_STATE_FF_MATERIAL |
          NINE_STATE_FF_VSTRANSF;
       memset(dst->changed.rs, ~0, (D3DRS_COUNT / 32) * sizeof(uint32_t));
       dst->changed.rs[D3DRS_LAST / 32] |= (1 << (D3DRS_COUNT % 32)) - 1;
       dst->changed.vtxbuf = (1ULL << This->caps.MaxStreams) - 1;
       dst->changed.stream_freq = dst->changed.vtxbuf;
       dst->changed.ucp = (1 << PIPE_MAX_CLIP_PLANES) - 1;
       dst->changed.texture = (1 << NINE_MAX_SAMPLERS) - 1;
       /* The doc says the projection, world, view and texture matrices
        * are saved, which would translate to:
        * dst->ff.changed.transform[0] = 0x00FF000C;
        * dst->ff.changed.transform[D3DTS_WORLD / 32] |= 1 << (D3DTS_WORLD % 32);
        * However we assume they meant save everything (which is basically just the
        * above plus the other world matrices).
        */
       dst->ff.changed.transform[0] = 0x00FF000C;
       for (s = 0; s < 8; s++)
           dst->ff.changed.transform[8+s] = ~0;
    }
    NineStateBlock9_Capture(NineStateBlock9(*ppSB));

    /* TODO: fixed function state */

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_BeginStateBlock( struct NineDevice9 *This )
{
    HRESULT hr;

    DBG("This=%p\n", This);

    user_assert(!This->record, D3DERR_INVALIDCALL);

    hr = NineStateBlock9_new(This, &This->record, NINESBT_CUSTOM);
    if (FAILED(hr))
        return hr;
    NineUnknown_ConvertRefToBind(NineUnknown(This->record));

    This->update = &This->record->state;
    This->is_recording = true;

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_EndStateBlock( struct NineDevice9 *This,
                           IDirect3DStateBlock9 **ppSB )
{
    DBG("This=%p ppSB=%p\n", This, ppSB);

    user_assert(This->record, D3DERR_INVALIDCALL);
    user_assert(ppSB != NULL, D3DERR_INVALIDCALL);

    This->update = &This->state;
    This->is_recording = false;

    NineUnknown_AddRef(NineUnknown(This->record));
    *ppSB = (IDirect3DStateBlock9 *)This->record;
    NineUnknown_Unbind(NineUnknown(This->record));
    This->record = NULL;

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetClipStatus( struct NineDevice9 *This,
                           const D3DCLIPSTATUS9 *pClipStatus )
{
    user_assert(pClipStatus, D3DERR_INVALIDCALL);
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetClipStatus( struct NineDevice9 *This,
                           D3DCLIPSTATUS9 *pClipStatus )
{
    user_assert(pClipStatus, D3DERR_INVALIDCALL);
    /* Set/GetClipStatus is supposed to get the app some infos
     * about vertices being clipped if it is using the software
     * vertex rendering. It would be too complicated to implement.
     * Probably the info is for developpers when working on their
     * applications. Else it could be for apps to know if it is worth
     * drawing some elements. In that case it makes sense to send
     * 0 for ClipUnion and 0xFFFFFFFF for ClipIntersection (basically
     * means not all vertices are clipped). Those values are known to
     * be the default if SetClipStatus is not set. Else we could return
     * what was set with SetClipStatus unchanged. */
    pClipStatus->ClipUnion = 0;
    pClipStatus->ClipIntersection = 0xFFFFFFFF;
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetTexture( struct NineDevice9 *This,
                        DWORD Stage,
                        IDirect3DBaseTexture9 **ppTexture )
{
    user_assert(Stage < NINE_MAX_SAMPLERS_PS ||
                Stage == D3DDMAPSAMPLER ||
                (Stage >= D3DVERTEXTEXTURESAMPLER0 &&
                 Stage <= D3DVERTEXTEXTURESAMPLER3), D3DERR_INVALIDCALL);
    user_assert(ppTexture, D3DERR_INVALIDCALL);

    if (Stage >= D3DDMAPSAMPLER)
        Stage = Stage - D3DDMAPSAMPLER + NINE_MAX_SAMPLERS_PS;

    *ppTexture = (IDirect3DBaseTexture9 *)This->state.texture[Stage];

    if (This->state.texture[Stage])
        NineUnknown_AddRef(NineUnknown(This->state.texture[Stage]));
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetTexture( struct NineDevice9 *This,
                        DWORD Stage,
                        IDirect3DBaseTexture9 *pTexture )
{
    struct nine_state *state = This->update;
    struct NineBaseTexture9 *tex = NineBaseTexture9(pTexture);
    struct NineBaseTexture9 *old;

    DBG("This=%p Stage=%u pTexture=%p\n", This, Stage, pTexture);

    user_assert(Stage < NINE_MAX_SAMPLERS_PS ||
                Stage == D3DDMAPSAMPLER ||
                (Stage >= D3DVERTEXTEXTURESAMPLER0 &&
                 Stage <= D3DVERTEXTEXTURESAMPLER3), D3DERR_INVALIDCALL);
    user_assert(!tex || (tex->base.pool != D3DPOOL_SCRATCH &&
                tex->base.pool != D3DPOOL_SYSTEMMEM), D3DERR_INVALIDCALL);

    if (Stage >= D3DDMAPSAMPLER)
        Stage = Stage - D3DDMAPSAMPLER + NINE_MAX_SAMPLERS_PS;

    if (This->is_recording) {
        state->changed.texture |= 1 << Stage;
        nine_bind(&state->texture[Stage], pTexture);
        return D3D_OK;
    }

    old = state->texture[Stage];
    if (old == tex)
        return D3D_OK;

    NineBindTextureToDevice(This, &state->texture[Stage], tex);

    nine_context_set_texture(This, Stage, tex);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetTextureStageState( struct NineDevice9 *This,
                                  DWORD Stage,
                                  D3DTEXTURESTAGESTATETYPE Type,
                                  DWORD *pValue )
{
    const struct nine_state *state = &This->state;

    user_assert(!This->pure, D3DERR_INVALIDCALL);
    user_assert(pValue != NULL, D3DERR_INVALIDCALL);
    user_assert(Stage < ARRAY_SIZE(state->ff.tex_stage), D3DERR_INVALIDCALL);
    user_assert(Type < ARRAY_SIZE(state->ff.tex_stage[0]), D3DERR_INVALIDCALL);

    *pValue = state->ff.tex_stage[Stage][Type];

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetTextureStageState( struct NineDevice9 *This,
                                  DWORD Stage,
                                  D3DTEXTURESTAGESTATETYPE Type,
                                  DWORD Value )
{
    struct nine_state *state = This->update;

    DBG("Stage=%u Type=%u Value=%08x\n", Stage, Type, Value);
    nine_dump_D3DTSS_value(DBG_FF, Type, Value);

    user_assert(Stage < ARRAY_SIZE(state->ff.tex_stage), D3DERR_INVALIDCALL);
    user_assert(Type < ARRAY_SIZE(state->ff.tex_stage[0]), D3DERR_INVALIDCALL);

    state->ff.tex_stage[Stage][Type] = Value;

    if (unlikely(This->is_recording)) {
        state->changed.group |= NINE_STATE_FF_PS_CONSTS;
        state->ff.changed.tex_stage[Stage][Type / 32] |= 1 << (Type % 32);
    } else
        nine_context_set_texture_stage_state(This, Stage, Type, Value);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetSamplerState( struct NineDevice9 *This,
                             DWORD Sampler,
                             D3DSAMPLERSTATETYPE Type,
                             DWORD *pValue )
{
    user_assert(!This->pure, D3DERR_INVALIDCALL);
    user_assert(pValue != NULL, D3DERR_INVALIDCALL);
    user_assert(Sampler < NINE_MAX_SAMPLERS_PS ||
                Sampler == D3DDMAPSAMPLER ||
                (Sampler >= D3DVERTEXTEXTURESAMPLER0 &&
                 Sampler <= D3DVERTEXTEXTURESAMPLER3), D3DERR_INVALIDCALL);

    if (Sampler >= D3DDMAPSAMPLER)
        Sampler = Sampler - D3DDMAPSAMPLER + NINE_MAX_SAMPLERS_PS;

    *pValue = This->state.samp_advertised[Sampler][Type];
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetSamplerState( struct NineDevice9 *This,
                             DWORD Sampler,
                             D3DSAMPLERSTATETYPE Type,
                             DWORD Value )
{
    struct nine_state *state = This->update;

    DBG("This=%p Sampler=%u Type=%s Value=%08x\n", This,
        Sampler, nine_D3DSAMP_to_str(Type), Value);

    user_assert(Sampler < NINE_MAX_SAMPLERS_PS ||
                Sampler == D3DDMAPSAMPLER ||
                (Sampler >= D3DVERTEXTEXTURESAMPLER0 &&
                 Sampler <= D3DVERTEXTEXTURESAMPLER3), D3DERR_INVALIDCALL);

    if (Sampler >= D3DDMAPSAMPLER)
        Sampler = Sampler - D3DDMAPSAMPLER + NINE_MAX_SAMPLERS_PS;

    if (unlikely(This->is_recording)) {
        state->samp_advertised[Sampler][Type] = Value;
        state->changed.group |= NINE_STATE_SAMPLER;
        state->changed.sampler[Sampler] |= 1 << Type;
        return D3D_OK;
    }

    if (state->samp_advertised[Sampler][Type] == Value)
        return D3D_OK;

    state->samp_advertised[Sampler][Type] = Value;
    nine_context_set_sampler_state(This, Sampler, Type, Value);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_ValidateDevice( struct NineDevice9 *This,
                            DWORD *pNumPasses )
{
    const struct nine_state *state = &This->state;
    unsigned i;
    unsigned w = 0, h = 0;

    DBG("This=%p pNumPasses=%p\n", This, pNumPasses);

    for (i = 0; i < ARRAY_SIZE(state->samp_advertised); ++i) {
        if (state->samp_advertised[i][D3DSAMP_MINFILTER] == D3DTEXF_NONE ||
            state->samp_advertised[i][D3DSAMP_MAGFILTER] == D3DTEXF_NONE)
            return D3DERR_UNSUPPORTEDTEXTUREFILTER;
    }

    for (i = 0; i < This->caps.NumSimultaneousRTs; ++i) {
        if (!state->rt[i])
            continue;
        if (w == 0) {
            w = state->rt[i]->desc.Width;
            h = state->rt[i]->desc.Height;
        } else
        if (state->rt[i]->desc.Width != w || state->rt[i]->desc.Height != h) {
            return D3DERR_CONFLICTINGRENDERSTATE;
        }
    }
    if (state->ds &&
        (state->rs_advertised[D3DRS_ZENABLE] || state->rs_advertised[D3DRS_STENCILENABLE])) {
        if (w != 0 &&
            (state->ds->desc.Width != w || state->ds->desc.Height != h))
            return D3DERR_CONFLICTINGRENDERSTATE;
    }

    if (pNumPasses)
        *pNumPasses = 1;

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetPaletteEntries( struct NineDevice9 *This,
                               UINT PaletteNumber,
                               const PALETTEENTRY *pEntries )
{
    STUB(D3D_OK); /* like wine */
}

HRESULT NINE_WINAPI
NineDevice9_GetPaletteEntries( struct NineDevice9 *This,
                               UINT PaletteNumber,
                               PALETTEENTRY *pEntries )
{
    STUB(D3DERR_INVALIDCALL);
}

HRESULT NINE_WINAPI
NineDevice9_SetCurrentTexturePalette( struct NineDevice9 *This,
                                      UINT PaletteNumber )
{
    STUB(D3D_OK); /* like wine */
}

HRESULT NINE_WINAPI
NineDevice9_GetCurrentTexturePalette( struct NineDevice9 *This,
                                      UINT *PaletteNumber )
{
    STUB(D3DERR_INVALIDCALL);
}

HRESULT NINE_WINAPI
NineDevice9_SetScissorRect( struct NineDevice9 *This,
                            const RECT *pRect )
{
    struct nine_state *state = This->update;

    user_assert(pRect != NULL, D3DERR_INVALIDCALL);

    DBG("x=(%u..%u) y=(%u..%u)\n",
        pRect->left, pRect->top, pRect->right, pRect->bottom);

    state->scissor.minx = pRect->left;
    state->scissor.miny = pRect->top;
    state->scissor.maxx = pRect->right;
    state->scissor.maxy = pRect->bottom;

    if (unlikely(This->is_recording))
        state->changed.group |= NINE_STATE_SCISSOR;
    else
        nine_context_set_scissor(This, &state->scissor);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetScissorRect( struct NineDevice9 *This,
                            RECT *pRect )
{
    user_assert(pRect != NULL, D3DERR_INVALIDCALL);

    pRect->left   = This->state.scissor.minx;
    pRect->top    = This->state.scissor.miny;
    pRect->right  = This->state.scissor.maxx;
    pRect->bottom = This->state.scissor.maxy;

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetSoftwareVertexProcessing( struct NineDevice9 *This,
                                         BOOL bSoftware )
{
    if (This->params.BehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING) {
        This->swvp = bSoftware;
        nine_context_set_swvp(This, bSoftware);
        return D3D_OK;
    } else
        return D3D_OK; /* msdn seems to indicate INVALIDCALL, but at least Halo expects OK */
}

BOOL NINE_WINAPI
NineDevice9_GetSoftwareVertexProcessing( struct NineDevice9 *This )
{
    return This->swvp;
}

HRESULT NINE_WINAPI
NineDevice9_SetNPatchMode( struct NineDevice9 *This,
                           float nSegments )
{
    return D3D_OK; /* Nothing to do because we don't advertise NPatch support */
}

float NINE_WINAPI
NineDevice9_GetNPatchMode( struct NineDevice9 *This )
{
    STUB(0);
}

/* TODO: only go through dirty textures */
static void
validate_textures(struct NineDevice9 *device)
{
    struct NineBaseTexture9 *tex, *ptr;
    LIST_FOR_EACH_ENTRY_SAFE(tex, ptr, &device->update_textures, list) {
        list_delinit(&tex->list);
        NineBaseTexture9_Validate(tex);
    }
}

static void
update_managed_buffers(struct NineDevice9 *device)
{
    struct NineBuffer9 *buf, *ptr;
    LIST_FOR_EACH_ENTRY_SAFE(buf, ptr, &device->update_buffers, managed.list) {
        list_delinit(&buf->managed.list);
        NineBuffer9_Upload(buf);
    }
}

static void
NineBeforeDraw( struct NineDevice9 *This )
{
    /* Upload Managed dirty content */
    validate_textures(This); /* may clobber state */
    update_managed_buffers(This);
}

static void
NineAfterDraw( struct NineDevice9 *This )
{
    unsigned i;
    struct nine_state *state = &This->state;
    unsigned ps_mask = state->ps ? state->ps->rt_mask : 1;

    /* Flag render-targets with autogenmipmap for mipmap regeneration */
    for (i = 0; i < This->caps.NumSimultaneousRTs; ++i) {
        struct NineSurface9 *rt = state->rt[i];

        if (rt && rt->desc.Format != D3DFMT_NULL && (ps_mask & (1 << i)) &&
            rt->desc.Usage & D3DUSAGE_AUTOGENMIPMAP) {
            assert(rt->texture == D3DRTYPE_TEXTURE ||
                   rt->texture == D3DRTYPE_CUBETEXTURE);
            NineBaseTexture9(rt->base.base.container)->dirty_mip = true;
        }
    }
}

#define IS_SYSTEMMEM_DYNAMIC(t) ((t) && (t)->base.pool == D3DPOOL_SYSTEMMEM && (t)->base.usage & D3DUSAGE_DYNAMIC)

/* Indicates the region needed right now for these buffers and add them to the list
 * of buffers to process in NineBeforeDraw.
 * The reason we don't call the upload right now is to generate smaller code (no
 * duplication of the NineBuffer9_Upload inline) and to have one upload (of the correct size)
 * if a vertex buffer is twice input of the draw call. */
static void
NineTrackSystemmemDynamic( struct NineBuffer9 *This, unsigned start, unsigned width )
{
    struct pipe_box box;

    if (start >= This->size)
        return; /* outside bounds, nothing to do */
    u_box_1d(start, MIN2(width, This->size-start), &box);
    u_box_union_1d(&This->managed.required_valid_region,
                   &This->managed.required_valid_region,
                   &box);
    This->managed.dirty = true;
    BASEBUF_REGISTER_UPDATE(This);
}

HRESULT NINE_WINAPI
NineDevice9_DrawPrimitive( struct NineDevice9 *This,
                           D3DPRIMITIVETYPE PrimitiveType,
                           UINT StartVertex,
                           UINT PrimitiveCount )
{
    unsigned i;
    DBG("iface %p, PrimitiveType %u, StartVertex %u, PrimitiveCount %u\n",
        This, PrimitiveType, StartVertex, PrimitiveCount);

    /* Tracking for dynamic SYSTEMMEM */
    for (i = 0; i < This->caps.MaxStreams; i++) {
        unsigned stride = This->state.vtxstride[i];
        if (IS_SYSTEMMEM_DYNAMIC((struct NineBuffer9*)This->state.stream[i])) {
            unsigned start = This->state.vtxbuf[i].buffer_offset + StartVertex * stride;
            unsigned full_size = This->state.stream[i]->base.size;
            unsigned num_vertices = prim_count_to_vertex_count(PrimitiveType, PrimitiveCount);
            unsigned size = MIN2(full_size-start, num_vertices * stride);
            if (!stride) /* Instancing. Not sure what to do. Require all */
                size = full_size;
            NineTrackSystemmemDynamic(&This->state.stream[i]->base, start, size);
        }
    }

    NineBeforeDraw(This);
    nine_context_draw_primitive(This, PrimitiveType, StartVertex, PrimitiveCount);
    NineAfterDraw(This);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_DrawIndexedPrimitive( struct NineDevice9 *This,
                                  D3DPRIMITIVETYPE PrimitiveType,
                                  INT BaseVertexIndex,
                                  UINT MinVertexIndex,
                                  UINT NumVertices,
                                  UINT StartIndex,
                                  UINT PrimitiveCount )
{
    unsigned i, num_indices;
    DBG("iface %p, PrimitiveType %u, BaseVertexIndex %u, MinVertexIndex %u "
        "NumVertices %u, StartIndex %u, PrimitiveCount %u\n",
        This, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices,
        StartIndex, PrimitiveCount);

    user_assert(This->state.idxbuf, D3DERR_INVALIDCALL);
    user_assert(This->state.vdecl, D3DERR_INVALIDCALL);

    num_indices = prim_count_to_vertex_count(PrimitiveType, PrimitiveCount);

    /* Tracking for dynamic SYSTEMMEM */
    if (IS_SYSTEMMEM_DYNAMIC(&This->state.idxbuf->base))
        NineTrackSystemmemDynamic(&This->state.idxbuf->base,
                                  StartIndex * This->state.idxbuf->index_size,
                                  num_indices * This->state.idxbuf->index_size);

    for (i = 0; i < This->caps.MaxStreams; i++) {
        if (IS_SYSTEMMEM_DYNAMIC((struct NineBuffer9*)This->state.stream[i])) {
            uint32_t stride = This->state.vtxstride[i];
            uint32_t full_size = This->state.stream[i]->base.size;
            uint32_t start, stop;

            start = MAX2(0, This->state.vtxbuf[i].buffer_offset+(MinVertexIndex+BaseVertexIndex)*stride);
            stop = This->state.vtxbuf[i].buffer_offset+(MinVertexIndex+NumVertices+BaseVertexIndex)*stride;
            stop = MIN2(stop, full_size);
            NineTrackSystemmemDynamic(&This->state.stream[i]->base,
                                      start, stop-start);
        }
    }

    NineBeforeDraw(This);
    nine_context_draw_indexed_primitive(This, PrimitiveType, BaseVertexIndex,
                                        MinVertexIndex, NumVertices, StartIndex,
                                        PrimitiveCount);
    NineAfterDraw(This);

    return D3D_OK;
}

static void
NineDevice9_SetStreamSourceNULL( struct NineDevice9 *This );

HRESULT NINE_WINAPI
NineDevice9_DrawPrimitiveUP( struct NineDevice9 *This,
                             D3DPRIMITIVETYPE PrimitiveType,
                             UINT PrimitiveCount,
                             const void *pVertexStreamZeroData,
                             UINT VertexStreamZeroStride )
{
    struct pipe_resource *resource = NULL;
    unsigned buffer_offset;
    unsigned StartVertex = 0;

    DBG("iface %p, PrimitiveType %u, PrimitiveCount %u, data %p, stride %u\n",
        This, PrimitiveType, PrimitiveCount,
        pVertexStreamZeroData, VertexStreamZeroStride);

    user_assert(pVertexStreamZeroData && VertexStreamZeroStride,
                D3DERR_INVALIDCALL);
    user_assert(PrimitiveCount, D3D_OK);

    u_upload_data(This->vertex_uploader,
                  0,
                  (prim_count_to_vertex_count(PrimitiveType, PrimitiveCount)) * VertexStreamZeroStride,
                  1,
                  pVertexStreamZeroData,
                  &buffer_offset,
                  &resource);
    u_upload_unmap(This->vertex_uploader);

    /* Optimization to skip changing the bound vertex buffer data
     * for consecutive DrawPrimitiveUp with identical VertexStreamZeroStride */
    if (VertexStreamZeroStride > 0) {
        StartVertex = buffer_offset / VertexStreamZeroStride;
        buffer_offset -= StartVertex * VertexStreamZeroStride;
    }

    nine_context_set_stream_source_apply(This, 0, resource,
                                         buffer_offset, VertexStreamZeroStride);
    pipe_resource_reference(&resource, NULL);

    NineBeforeDraw(This);
    nine_context_draw_primitive(This, PrimitiveType, StartVertex, PrimitiveCount);
    NineAfterDraw(This);

    NineDevice9_PauseRecording(This);
    NineDevice9_SetStreamSourceNULL(This);
    NineDevice9_ResumeRecording(This);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_DrawIndexedPrimitiveUP( struct NineDevice9 *This,
                                    D3DPRIMITIVETYPE PrimitiveType,
                                    UINT MinVertexIndex,
                                    UINT NumVertices,
                                    UINT PrimitiveCount,
                                    const void *pIndexData,
                                    D3DFORMAT IndexDataFormat,
                                    const void *pVertexStreamZeroData,
                                    UINT VertexStreamZeroStride )
{
    struct pipe_vertex_buffer vbuf;
    unsigned index_size = (IndexDataFormat == D3DFMT_INDEX16) ? 2 : 4;
    struct pipe_resource *ibuf = NULL;
    unsigned base;

    DBG("iface %p, PrimitiveType %u, MinVertexIndex %u, NumVertices %u "
        "PrimitiveCount %u, pIndexData %p, IndexDataFormat %u "
        "pVertexStreamZeroData %p, VertexStreamZeroStride %u\n",
        This, PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount,
        pIndexData, IndexDataFormat,
        pVertexStreamZeroData, VertexStreamZeroStride);

    user_assert(pIndexData && pVertexStreamZeroData, D3DERR_INVALIDCALL);
    user_assert(VertexStreamZeroStride, D3DERR_INVALIDCALL);
    user_assert(IndexDataFormat == D3DFMT_INDEX16 ||
                IndexDataFormat == D3DFMT_INDEX32, D3DERR_INVALIDCALL);
    user_assert(PrimitiveCount, D3D_OK);

    base = MinVertexIndex * VertexStreamZeroStride;
    vbuf.is_user_buffer = false;
    vbuf.buffer.resource = NULL;
    u_upload_data(This->vertex_uploader,
                  base,
                  NumVertices * VertexStreamZeroStride, /* XXX */
                  64,
                  (const uint8_t *)pVertexStreamZeroData + base,
                  &vbuf.buffer_offset,
                  &vbuf.buffer.resource);
    u_upload_unmap(This->vertex_uploader);
    /* Won't be used: */
    vbuf.buffer_offset -= base;

    unsigned index_offset = 0;
    u_upload_data(This->pipe_secondary->stream_uploader,
                  0,
                  (prim_count_to_vertex_count(PrimitiveType, PrimitiveCount)) * index_size,
                  64,
                  pIndexData,
                  &index_offset,
                  &ibuf);
    u_upload_unmap(This->pipe_secondary->stream_uploader);

    NineBeforeDraw(This);
    nine_context_draw_indexed_primitive_from_vtxbuf_idxbuf(This, PrimitiveType,
                                                           MinVertexIndex,
                                                           NumVertices,
                                                           PrimitiveCount,
                                                           VertexStreamZeroStride,
                                                           &vbuf,
                                                           ibuf,
                                                           ibuf ? NULL : (void*)pIndexData,
                                                           index_offset,
                                                           index_size);
    NineAfterDraw(This);

    pipe_vertex_buffer_unreference(&vbuf);
    pipe_resource_reference(&ibuf, NULL);

    NineDevice9_PauseRecording(This);
    NineDevice9_SetIndices(This, NULL);
    NineDevice9_SetStreamSourceNULL(This);
    NineDevice9_ResumeRecording(This);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_ProcessVertices( struct NineDevice9 *This,
                             UINT SrcStartIndex,
                             UINT DestIndex,
                             UINT VertexCount,
                             IDirect3DVertexBuffer9 *pDestBuffer,
                             IDirect3DVertexDeclaration9 *pVertexDecl,
                             DWORD Flags )
{
    struct pipe_screen *screen_sw = This->screen_sw;
    struct pipe_context *pipe_sw = This->pipe_sw;
    struct NineVertexDeclaration9 *vdecl = NineVertexDeclaration9(pVertexDecl);
    struct NineVertexBuffer9 *dst = NineVertexBuffer9(pDestBuffer);
    struct NineVertexShader9 *vs;
    struct pipe_resource *resource;
    struct pipe_transfer *transfer = NULL;
    struct pipe_stream_output_info so;
    struct pipe_stream_output_target *target;
    struct pipe_draw_info draw;
    struct pipe_draw_start_count_bias sc;
    struct pipe_box box;
    bool programmable_vs = This->state.vs && !(This->state.vdecl && This->state.vdecl->position_t);
    unsigned offsets[1] = {0};
    HRESULT hr;
    unsigned buffer_size;
    void *map;

    DBG("This=%p SrcStartIndex=%u DestIndex=%u VertexCount=%u "
        "pDestBuffer=%p pVertexDecl=%p Flags=%d\n",
        This, SrcStartIndex, DestIndex, VertexCount, pDestBuffer,
        pVertexDecl, Flags);

    user_assert(pDestBuffer && pVertexDecl, D3DERR_INVALIDCALL);

    if (!screen_sw->get_param(screen_sw, PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS)) {
        DBG("ProcessVertices not supported\n");
        return D3DERR_INVALIDCALL;
    }


    vs = programmable_vs ? This->state.vs : This->ff.vs;
    /* Note: version is 0 for ff */
    user_assert(vdecl || (vs->byte_code.version < 0x30 && dst->desc.FVF),
                D3DERR_INVALIDCALL);
    if (!vdecl) {
        DWORD FVF = dst->desc.FVF;
        vdecl = util_hash_table_get(This->ff.ht_fvf, &FVF);
        if (!vdecl) {
            hr = NineVertexDeclaration9_new_from_fvf(This, FVF, &vdecl);
            if (FAILED(hr))
                return hr;
            vdecl->fvf = FVF;
            _mesa_hash_table_insert(This->ff.ht_fvf, &vdecl->fvf, vdecl);
            NineUnknown_ConvertRefToBind(NineUnknown(vdecl));
        }
    }

    /* Flags: Can be 0 or D3DPV_DONOTCOPYDATA, and/or lock flags
     * D3DPV_DONOTCOPYDATA -> Has effect only for ff. In particular
     * if not set, everything from src will be used, and dst
     * must match exactly the ff vs outputs.
     * TODO: Handle all the checks, etc for ff */
    user_assert(vdecl->position_t || programmable_vs,
                D3DERR_INVALIDCALL);

    /* TODO: Support vs < 3 and ff */
    user_assert(vs->byte_code.version == 0x30,
                D3DERR_INVALIDCALL);
    /* TODO: Not hardcode the constant buffers for swvp */
    user_assert(This->may_swvp,
                D3DERR_INVALIDCALL);

    nine_state_prepare_draw_sw(This, vdecl, SrcStartIndex, VertexCount, &so);

    buffer_size = VertexCount * so.stride[0] * 4;
    {
        struct pipe_resource templ;

        memset(&templ, 0, sizeof(templ));
        templ.target = PIPE_BUFFER;
        templ.format = PIPE_FORMAT_R8_UNORM;
        templ.width0 = buffer_size;
        templ.flags = 0;
        templ.bind = PIPE_BIND_STREAM_OUTPUT;
        templ.usage = PIPE_USAGE_STREAM;
        templ.height0 = templ.depth0 = templ.array_size = 1;
        templ.last_level = templ.nr_samples = templ.nr_storage_samples = 0;

        resource = screen_sw->resource_create(screen_sw, &templ);
        if (!resource)
            return E_OUTOFMEMORY;
    }
    target = pipe_sw->create_stream_output_target(pipe_sw, resource,
                                                  0, buffer_size);
    if (!target) {
        pipe_resource_reference(&resource, NULL);
        return D3DERR_DRIVERINTERNALERROR;
    }

    draw.mode = MESA_PRIM_POINTS;
    sc.count = VertexCount;
    draw.start_instance = 0;
    draw.primitive_restart = false;
    draw.restart_index = 0;
    draw.instance_count = 1;
    draw.index_size = 0;
    sc.start = 0;
    sc.index_bias = 0;
    draw.min_index = 0;
    draw.max_index = VertexCount - 1;


    pipe_sw->set_stream_output_targets(pipe_sw, 1, &target, offsets);

    pipe_sw->draw_vbo(pipe_sw, &draw, 0, NULL, &sc, 1);

    pipe_sw->set_stream_output_targets(pipe_sw, 0, NULL, 0);
    pipe_sw->stream_output_target_destroy(pipe_sw, target);

    u_box_1d(0, VertexCount * so.stride[0] * 4, &box);
    map = pipe_sw->buffer_map(pipe_sw, resource, 0, PIPE_MAP_READ, &box,
                                &transfer);
    if (!map) {
        hr = D3DERR_DRIVERINTERNALERROR;
        goto out;
    }

    hr = NineVertexDeclaration9_ConvertStreamOutput(vdecl,
                                                    dst, DestIndex, VertexCount,
                                                    map, &so);
    if (transfer)
        pipe_sw->buffer_unmap(pipe_sw, transfer);

out:
    nine_state_after_draw_sw(This);
    pipe_resource_reference(&resource, NULL);
    return hr;
}

HRESULT NINE_WINAPI
NineDevice9_CreateVertexDeclaration( struct NineDevice9 *This,
                                     const D3DVERTEXELEMENT9 *pVertexElements,
                                     IDirect3DVertexDeclaration9 **ppDecl )
{
    struct NineVertexDeclaration9 *vdecl;

    DBG("This=%p pVertexElements=%p ppDecl=%p\n",
        This, pVertexElements, ppDecl);

    user_assert(pVertexElements && ppDecl, D3DERR_INVALIDCALL);

    HRESULT hr = NineVertexDeclaration9_new(This, pVertexElements, &vdecl);
    if (SUCCEEDED(hr))
        *ppDecl = (IDirect3DVertexDeclaration9 *)vdecl;

    return hr;
}

HRESULT NINE_WINAPI
NineDevice9_SetVertexDeclaration( struct NineDevice9 *This,
                                  IDirect3DVertexDeclaration9 *pDecl )
{
    struct nine_state *state = This->update;
    struct NineVertexDeclaration9 *vdecl = NineVertexDeclaration9(pDecl);

    DBG("This=%p pDecl=%p\n", This, pDecl);

    if (unlikely(This->is_recording)) {
        nine_bind(&state->vdecl, vdecl);
        state->changed.group |= NINE_STATE_VDECL;
        return D3D_OK;
    }

    if (state->vdecl == vdecl)
        return D3D_OK;

    nine_bind(&state->vdecl, vdecl);

    nine_context_set_vertex_declaration(This, vdecl);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetVertexDeclaration( struct NineDevice9 *This,
                                  IDirect3DVertexDeclaration9 **ppDecl )
{
    user_assert(ppDecl, D3DERR_INVALIDCALL);

    *ppDecl = (IDirect3DVertexDeclaration9 *)This->state.vdecl;
    if (*ppDecl)
        NineUnknown_AddRef(NineUnknown(*ppDecl));
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetFVF( struct NineDevice9 *This,
                    DWORD FVF )
{
    struct NineVertexDeclaration9 *vdecl;
    HRESULT hr;

    DBG("FVF = %08x\n", FVF);
    if (!FVF)
        return D3D_OK; /* like wine */

    vdecl = util_hash_table_get(This->ff.ht_fvf, &FVF);
    if (!vdecl) {
        hr = NineVertexDeclaration9_new_from_fvf(This, FVF, &vdecl);
        if (FAILED(hr))
            return hr;
        vdecl->fvf = FVF;
        _mesa_hash_table_insert(This->ff.ht_fvf, &vdecl->fvf, vdecl);
        NineUnknown_ConvertRefToBind(NineUnknown(vdecl));
    }
    return NineDevice9_SetVertexDeclaration(
        This, (IDirect3DVertexDeclaration9 *)vdecl);
}

HRESULT NINE_WINAPI
NineDevice9_GetFVF( struct NineDevice9 *This,
                    DWORD *pFVF )
{
    user_assert(pFVF != NULL, D3DERR_INVALIDCALL);
    *pFVF = This->state.vdecl ? This->state.vdecl->fvf : 0;
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_CreateVertexShader( struct NineDevice9 *This,
                                const DWORD *pFunction,
                                IDirect3DVertexShader9 **ppShader )
{
    struct NineVertexShader9 *vs;
    HRESULT hr;

    DBG("This=%p pFunction=%p ppShader=%p\n", This, pFunction, ppShader);

    user_assert(pFunction && ppShader, D3DERR_INVALIDCALL);

    hr = NineVertexShader9_new(This, &vs, pFunction, NULL);
    if (FAILED(hr))
        return hr;
    *ppShader = (IDirect3DVertexShader9 *)vs;
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetVertexShader( struct NineDevice9 *This,
                             IDirect3DVertexShader9 *pShader )
{
    struct nine_state *state = This->update;
    struct NineVertexShader9 *vs_shader = (struct NineVertexShader9*)pShader;

    DBG("This=%p pShader=%p\n", This, pShader);

    if (unlikely(This->is_recording)) {
        nine_bind(&state->vs, vs_shader);
        state->changed.group |= NINE_STATE_VS;
        return D3D_OK;
    }

    if (state->vs == vs_shader)
      return D3D_OK;

    nine_bind(&state->vs, vs_shader);

    nine_context_set_vertex_shader(This, vs_shader);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetVertexShader( struct NineDevice9 *This,
                             IDirect3DVertexShader9 **ppShader )
{
    user_assert(ppShader, D3DERR_INVALIDCALL);
    nine_reference_set(ppShader, This->state.vs);
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetVertexShaderConstantF( struct NineDevice9 *This,
                                      UINT StartRegister,
                                      const float *pConstantData,
                                      UINT Vector4fCount )
{
    struct nine_state *state = This->update;
    float *vs_const_f = state->vs_const_f;

    DBG("This=%p StartRegister=%u pConstantData=%p Vector4fCount=%u\n",
        This, StartRegister, pConstantData, Vector4fCount);

    user_assert(StartRegister                  < This->caps.MaxVertexShaderConst, D3DERR_INVALIDCALL);
    user_assert(StartRegister + Vector4fCount <= This->caps.MaxVertexShaderConst, D3DERR_INVALIDCALL);

    if (!Vector4fCount)
       return D3D_OK;
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    if (unlikely(This->is_recording)) {
        memcpy(&vs_const_f[StartRegister * 4],
               pConstantData,
               Vector4fCount * 4 * sizeof(state->vs_const_f[0]));

        nine_ranges_insert(&state->changed.vs_const_f,
                           StartRegister, StartRegister + Vector4fCount,
                           &This->range_pool);

        state->changed.group |= NINE_STATE_VS_CONST;

        return D3D_OK;
    }

    if (!memcmp(&vs_const_f[StartRegister * 4], pConstantData,
                Vector4fCount * 4 * sizeof(state->vs_const_f[0])))
        return D3D_OK;

    memcpy(&vs_const_f[StartRegister * 4],
           pConstantData,
           Vector4fCount * 4 * sizeof(state->vs_const_f[0]));

    nine_context_set_vertex_shader_constant_f(This, StartRegister, pConstantData,
                                              Vector4fCount * 4 * sizeof(state->vs_const_f[0]),
                                              Vector4fCount);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetVertexShaderConstantF( struct NineDevice9 *This,
                                      UINT StartRegister,
                                      float *pConstantData,
                                      UINT Vector4fCount )
{
    const struct nine_state *state = &This->state;

    user_assert(!This->pure, D3DERR_INVALIDCALL);
    user_assert(StartRegister                  < This->caps.MaxVertexShaderConst, D3DERR_INVALIDCALL);
    user_assert(StartRegister + Vector4fCount <= This->caps.MaxVertexShaderConst, D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    memcpy(pConstantData,
           &state->vs_const_f[StartRegister * 4],
           Vector4fCount * 4 * sizeof(state->vs_const_f[0]));

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetVertexShaderConstantI( struct NineDevice9 *This,
                                      UINT StartRegister,
                                      const int *pConstantData,
                                      UINT Vector4iCount )
{
    struct nine_state *state = This->update;
    int i;

    DBG("This=%p StartRegister=%u pConstantData=%p Vector4iCount=%u\n",
        This, StartRegister, pConstantData, Vector4iCount);

    user_assert(StartRegister < (This->may_swvp ? NINE_MAX_CONST_I_SWVP : NINE_MAX_CONST_I),
                D3DERR_INVALIDCALL);
    user_assert(StartRegister + Vector4iCount <= (This->may_swvp ? NINE_MAX_CONST_I_SWVP : NINE_MAX_CONST_I),
                D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    if (This->driver_caps.vs_integer) {
        if (!This->is_recording) {
            if (!memcmp(&state->vs_const_i[4 * StartRegister], pConstantData,
                        Vector4iCount * sizeof(int[4])))
                return D3D_OK;
        }
        memcpy(&state->vs_const_i[4 * StartRegister],
               pConstantData,
               Vector4iCount * sizeof(int[4]));
    } else {
        for (i = 0; i < Vector4iCount; i++) {
            state->vs_const_i[4 * (StartRegister + i)] = fui((float)(pConstantData[4 * i]));
            state->vs_const_i[4 * (StartRegister + i) + 1] = fui((float)(pConstantData[4 * i + 1]));
            state->vs_const_i[4 * (StartRegister + i) + 2] = fui((float)(pConstantData[4 * i + 2]));
            state->vs_const_i[4 * (StartRegister + i) + 3] = fui((float)(pConstantData[4 * i + 3]));
        }
    }

    if (unlikely(This->is_recording)) {
        nine_ranges_insert(&state->changed.vs_const_i,
                           StartRegister, StartRegister + Vector4iCount,
                           &This->range_pool);
        state->changed.group |= NINE_STATE_VS_CONST;
    } else
        nine_context_set_vertex_shader_constant_i(This, StartRegister, pConstantData,
                                                  Vector4iCount * sizeof(int[4]), Vector4iCount);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetVertexShaderConstantI( struct NineDevice9 *This,
                                      UINT StartRegister,
                                      int *pConstantData,
                                      UINT Vector4iCount )
{
    const struct nine_state *state = &This->state;
    int i;

    user_assert(!This->pure, D3DERR_INVALIDCALL);
    user_assert(StartRegister < (This->may_swvp ? NINE_MAX_CONST_I_SWVP : NINE_MAX_CONST_I),
                D3DERR_INVALIDCALL);
    user_assert(StartRegister + Vector4iCount <= (This->may_swvp ? NINE_MAX_CONST_I_SWVP : NINE_MAX_CONST_I),
                D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    if (This->driver_caps.vs_integer) {
        memcpy(pConstantData,
               &state->vs_const_i[4 * StartRegister],
               Vector4iCount * sizeof(int[4]));
    } else {
        for (i = 0; i < Vector4iCount; i++) {
            pConstantData[4 * i] = (int32_t) uif(state->vs_const_i[4 * (StartRegister + i)]);
            pConstantData[4 * i + 1] = (int32_t) uif(state->vs_const_i[4 * (StartRegister + i) + 1]);
            pConstantData[4 * i + 2] = (int32_t) uif(state->vs_const_i[4 * (StartRegister + i) + 2]);
            pConstantData[4 * i + 3] = (int32_t) uif(state->vs_const_i[4 * (StartRegister + i) + 3]);
        }
    }

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetVertexShaderConstantB( struct NineDevice9 *This,
                                      UINT StartRegister,
                                      const BOOL *pConstantData,
                                      UINT BoolCount )
{
    struct nine_state *state = This->update;
    int i;
    uint32_t bool_true = This->driver_caps.vs_integer ? 0xFFFFFFFF : fui(1.0f);

    DBG("This=%p StartRegister=%u pConstantData=%p BoolCount=%u\n",
        This, StartRegister, pConstantData, BoolCount);

    user_assert(StartRegister < (This->may_swvp ? NINE_MAX_CONST_B_SWVP : NINE_MAX_CONST_B),
                D3DERR_INVALIDCALL);
    user_assert(StartRegister + BoolCount <= (This->may_swvp ? NINE_MAX_CONST_B_SWVP : NINE_MAX_CONST_B),
                D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    if (!This->is_recording) {
        bool noChange = true;
        for (i = 0; i < BoolCount; i++) {
            if (!!state->vs_const_b[StartRegister + i] != !!pConstantData[i])
              noChange = false;
        }
        if (noChange)
            return D3D_OK;
    }

    for (i = 0; i < BoolCount; i++)
        state->vs_const_b[StartRegister + i] = pConstantData[i] ? bool_true : 0;

    if (unlikely(This->is_recording)) {
        nine_ranges_insert(&state->changed.vs_const_b,
                           StartRegister, StartRegister + BoolCount,
                           &This->range_pool);
        state->changed.group |= NINE_STATE_VS_CONST;
    } else
        nine_context_set_vertex_shader_constant_b(This, StartRegister, pConstantData,
                                                  sizeof(BOOL) * BoolCount, BoolCount);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetVertexShaderConstantB( struct NineDevice9 *This,
                                      UINT StartRegister,
                                      BOOL *pConstantData,
                                      UINT BoolCount )
{
    const struct nine_state *state = &This->state;
    int i;

    user_assert(!This->pure, D3DERR_INVALIDCALL);
    user_assert(StartRegister < (This->may_swvp ? NINE_MAX_CONST_B_SWVP : NINE_MAX_CONST_B),
                D3DERR_INVALIDCALL);
    user_assert(StartRegister + BoolCount <= (This->may_swvp ? NINE_MAX_CONST_B_SWVP : NINE_MAX_CONST_B),
                D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    for (i = 0; i < BoolCount; i++)
        pConstantData[i] = state->vs_const_b[StartRegister + i] != 0 ? true : false;

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetStreamSource( struct NineDevice9 *This,
                             UINT StreamNumber,
                             IDirect3DVertexBuffer9 *pStreamData,
                             UINT OffsetInBytes,
                             UINT Stride )
{
    struct nine_state *state = This->update;
    struct NineVertexBuffer9 *pVBuf9 = NineVertexBuffer9(pStreamData);
    const unsigned i = StreamNumber;

    DBG("This=%p StreamNumber=%u pStreamData=%p OffsetInBytes=%u Stride=%u\n",
        This, StreamNumber, pStreamData, OffsetInBytes, Stride);

    user_assert(StreamNumber < This->caps.MaxStreams, D3DERR_INVALIDCALL);
    user_assert(Stride <= This->caps.MaxStreamStride, D3DERR_INVALIDCALL);

    if (unlikely(This->is_recording)) {
        nine_bind(&state->stream[i], pStreamData);
        state->changed.vtxbuf |= 1 << StreamNumber;
        state->vtxstride[i] = Stride;
        state->vtxbuf[i].buffer_offset = OffsetInBytes;
        return D3D_OK;
    }

    if (state->stream[i] == NineVertexBuffer9(pStreamData) &&
        state->vtxstride[i] == Stride &&
        state->vtxbuf[i].buffer_offset == OffsetInBytes)
        return D3D_OK;

    state->vtxstride[i] = Stride;
    state->vtxbuf[i].buffer_offset = OffsetInBytes;

    NineBindBufferToDevice(This,
                           (struct NineBuffer9 **)&state->stream[i],
                           (struct NineBuffer9 *)pVBuf9);

    nine_context_set_stream_source(This,
                                   StreamNumber,
                                   pVBuf9,
                                   OffsetInBytes,
                                   Stride);

    return D3D_OK;
}

static void
NineDevice9_SetStreamSourceNULL( struct NineDevice9 *This )
{
    struct nine_state *state = This->update;

    DBG("This=%p\n", This);

    state->vtxstride[0] = 0;
    state->vtxbuf[0].buffer_offset = 0;

    if (!state->stream[0])
        return;

    NineBindBufferToDevice(This,
                           (struct NineBuffer9 **)&state->stream[0],
                           NULL);
}

HRESULT NINE_WINAPI
NineDevice9_GetStreamSource( struct NineDevice9 *This,
                             UINT StreamNumber,
                             IDirect3DVertexBuffer9 **ppStreamData,
                             UINT *pOffsetInBytes,
                             UINT *pStride )
{
    const struct nine_state *state = &This->state;
    const unsigned i = StreamNumber;

    user_assert(StreamNumber < This->caps.MaxStreams, D3DERR_INVALIDCALL);
    user_assert(ppStreamData && pOffsetInBytes && pStride, D3DERR_INVALIDCALL);

    nine_reference_set(ppStreamData, state->stream[i]);
    *pStride = state->vtxstride[i];
    *pOffsetInBytes = state->vtxbuf[i].buffer_offset;

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetStreamSourceFreq( struct NineDevice9 *This,
                                 UINT StreamNumber,
                                 UINT Setting )
{
    struct nine_state *state = This->update;
    /* const UINT freq = Setting & 0x7FFFFF; */

    DBG("This=%p StreamNumber=%u FrequencyParameter=0x%x\n", This,
        StreamNumber, Setting);

    user_assert(StreamNumber < This->caps.MaxStreams, D3DERR_INVALIDCALL);
    user_assert(StreamNumber != 0 || !(Setting & D3DSTREAMSOURCE_INSTANCEDATA),
                D3DERR_INVALIDCALL);
    user_assert(!((Setting & D3DSTREAMSOURCE_INSTANCEDATA) &&
                  (Setting & D3DSTREAMSOURCE_INDEXEDDATA)), D3DERR_INVALIDCALL);
    user_assert(Setting, D3DERR_INVALIDCALL);

    if (unlikely(This->is_recording)) {
        state->stream_freq[StreamNumber] = Setting;
        state->changed.stream_freq |= 1 << StreamNumber;
        return D3D_OK;
    }

    if (state->stream_freq[StreamNumber] == Setting)
        return D3D_OK;

    state->stream_freq[StreamNumber] = Setting;

    nine_context_set_stream_source_freq(This, StreamNumber, Setting);
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetStreamSourceFreq( struct NineDevice9 *This,
                                 UINT StreamNumber,
                                 UINT *pSetting )
{
    user_assert(pSetting != NULL, D3DERR_INVALIDCALL);
    user_assert(StreamNumber < This->caps.MaxStreams, D3DERR_INVALIDCALL);
    *pSetting = This->state.stream_freq[StreamNumber];
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetIndices( struct NineDevice9 *This,
                        IDirect3DIndexBuffer9 *pIndexData )
{
    struct nine_state *state = This->update;
    struct NineIndexBuffer9 *idxbuf = NineIndexBuffer9(pIndexData);

    DBG("This=%p pIndexData=%p\n", This, pIndexData);

    if (unlikely(This->is_recording)) {
        nine_bind(&state->idxbuf, idxbuf);
        state->changed.group |= NINE_STATE_IDXBUF;
        return D3D_OK;
    }

    if (state->idxbuf == idxbuf)
        return D3D_OK;

    NineBindBufferToDevice(This,
                           (struct NineBuffer9 **)&state->idxbuf,
                           (struct NineBuffer9 *)idxbuf);

    nine_context_set_indices(This, idxbuf);

    return D3D_OK;
}

/* XXX: wine/d3d9 doesn't have pBaseVertexIndex, and it doesn't make sense
 * here because it's an argument passed to the Draw calls.
 */
HRESULT NINE_WINAPI
NineDevice9_GetIndices( struct NineDevice9 *This,
                        IDirect3DIndexBuffer9 **ppIndexData)
{
    user_assert(ppIndexData, D3DERR_INVALIDCALL);
    nine_reference_set(ppIndexData, This->state.idxbuf);
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_CreatePixelShader( struct NineDevice9 *This,
                               const DWORD *pFunction,
                               IDirect3DPixelShader9 **ppShader )
{
    struct NinePixelShader9 *ps;
    HRESULT hr;

    DBG("This=%p pFunction=%p ppShader=%p\n", This, pFunction, ppShader);

    user_assert(pFunction && ppShader, D3DERR_INVALIDCALL);

    hr = NinePixelShader9_new(This, &ps, pFunction, NULL);
    if (FAILED(hr))
        return hr;
    *ppShader = (IDirect3DPixelShader9 *)ps;
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetPixelShader( struct NineDevice9 *This,
                            IDirect3DPixelShader9 *pShader )
{
    struct nine_state *state = This->update;
    struct NinePixelShader9 *ps = (struct NinePixelShader9*)pShader;

    DBG("This=%p pShader=%p\n", This, pShader);

    if (unlikely(This->is_recording)) {
        nine_bind(&state->ps, pShader);
        state->changed.group |= NINE_STATE_PS;
        return D3D_OK;
    }

    if (state->ps == ps)
        return D3D_OK;

    nine_bind(&state->ps, ps);

    nine_context_set_pixel_shader(This, ps);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetPixelShader( struct NineDevice9 *This,
                            IDirect3DPixelShader9 **ppShader )
{
    user_assert(ppShader, D3DERR_INVALIDCALL);
    nine_reference_set(ppShader, This->state.ps);
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetPixelShaderConstantF( struct NineDevice9 *This,
                                     UINT StartRegister,
                                     const float *pConstantData,
                                     UINT Vector4fCount )
{
    struct nine_state *state = This->update;

    DBG("This=%p StartRegister=%u pConstantData=%p Vector4fCount=%u\n",
        This, StartRegister, pConstantData, Vector4fCount);

    user_assert(StartRegister                  < NINE_MAX_CONST_F_PS3, D3DERR_INVALIDCALL);
    user_assert(StartRegister + Vector4fCount <= NINE_MAX_CONST_F_PS3, D3DERR_INVALIDCALL);

    if (!Vector4fCount)
       return D3D_OK;
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    if (unlikely(This->is_recording)) {
        memcpy(&state->ps_const_f[StartRegister * 4],
               pConstantData,
               Vector4fCount * 4 * sizeof(state->ps_const_f[0]));

        nine_ranges_insert(&state->changed.ps_const_f,
                           StartRegister, StartRegister + Vector4fCount,
                           &This->range_pool);

        state->changed.group |= NINE_STATE_PS_CONST;
        return D3D_OK;
    }

    if (!memcmp(&state->ps_const_f[StartRegister * 4], pConstantData,
                Vector4fCount * 4 * sizeof(state->ps_const_f[0])))
        return D3D_OK;

    memcpy(&state->ps_const_f[StartRegister * 4],
           pConstantData,
           Vector4fCount * 4 * sizeof(state->ps_const_f[0]));

    nine_context_set_pixel_shader_constant_f(This, StartRegister, pConstantData,
                                             Vector4fCount * 4 * sizeof(state->ps_const_f[0]),
                                             Vector4fCount);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetPixelShaderConstantF( struct NineDevice9 *This,
                                     UINT StartRegister,
                                     float *pConstantData,
                                     UINT Vector4fCount )
{
    const struct nine_state *state = &This->state;

    user_assert(!This->pure, D3DERR_INVALIDCALL);
    user_assert(StartRegister                  < NINE_MAX_CONST_F_PS3, D3DERR_INVALIDCALL);
    user_assert(StartRegister + Vector4fCount <= NINE_MAX_CONST_F_PS3, D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    memcpy(pConstantData,
           &state->ps_const_f[StartRegister * 4],
           Vector4fCount * 4 * sizeof(state->ps_const_f[0]));

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetPixelShaderConstantI( struct NineDevice9 *This,
                                     UINT StartRegister,
                                     const int *pConstantData,
                                     UINT Vector4iCount )
{
    struct nine_state *state = This->update;
    int i;

    DBG("This=%p StartRegister=%u pConstantData=%p Vector4iCount=%u\n",
        This, StartRegister, pConstantData, Vector4iCount);

    user_assert(StartRegister                  < NINE_MAX_CONST_I, D3DERR_INVALIDCALL);
    user_assert(StartRegister + Vector4iCount <= NINE_MAX_CONST_I, D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    if (This->driver_caps.ps_integer) {
        if (!This->is_recording) {
            if (!memcmp(&state->ps_const_i[StartRegister][0], pConstantData,
                        Vector4iCount * sizeof(state->ps_const_i[0])))
                return D3D_OK;
        }
        memcpy(&state->ps_const_i[StartRegister][0],
               pConstantData,
               Vector4iCount * sizeof(state->ps_const_i[0]));
    } else {
        for (i = 0; i < Vector4iCount; i++) {
            state->ps_const_i[StartRegister+i][0] = fui((float)(pConstantData[4*i]));
            state->ps_const_i[StartRegister+i][1] = fui((float)(pConstantData[4*i+1]));
            state->ps_const_i[StartRegister+i][2] = fui((float)(pConstantData[4*i+2]));
            state->ps_const_i[StartRegister+i][3] = fui((float)(pConstantData[4*i+3]));
        }
    }

    if (unlikely(This->is_recording)) {
        state->changed.ps_const_i |= ((1 << Vector4iCount) - 1) << StartRegister;
        state->changed.group |= NINE_STATE_PS_CONST;
    } else
        nine_context_set_pixel_shader_constant_i(This, StartRegister, pConstantData,
                                                 sizeof(state->ps_const_i[0]) * Vector4iCount, Vector4iCount);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetPixelShaderConstantI( struct NineDevice9 *This,
                                     UINT StartRegister,
                                     int *pConstantData,
                                     UINT Vector4iCount )
{
    const struct nine_state *state = &This->state;
    int i;

    user_assert(!This->pure, D3DERR_INVALIDCALL);
    user_assert(StartRegister                  < NINE_MAX_CONST_I, D3DERR_INVALIDCALL);
    user_assert(StartRegister + Vector4iCount <= NINE_MAX_CONST_I, D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    if (This->driver_caps.ps_integer) {
        memcpy(pConstantData,
               &state->ps_const_i[StartRegister][0],
               Vector4iCount * sizeof(state->ps_const_i[0]));
    } else {
        for (i = 0; i < Vector4iCount; i++) {
            pConstantData[4*i] = (int32_t) uif(state->ps_const_i[StartRegister+i][0]);
            pConstantData[4*i+1] = (int32_t) uif(state->ps_const_i[StartRegister+i][1]);
            pConstantData[4*i+2] = (int32_t) uif(state->ps_const_i[StartRegister+i][2]);
            pConstantData[4*i+3] = (int32_t) uif(state->ps_const_i[StartRegister+i][3]);
        }
    }

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_SetPixelShaderConstantB( struct NineDevice9 *This,
                                     UINT StartRegister,
                                     const BOOL *pConstantData,
                                     UINT BoolCount )
{
    struct nine_state *state = This->update;
    int i;
    uint32_t bool_true = This->driver_caps.ps_integer ? 0xFFFFFFFF : fui(1.0f);

    DBG("This=%p StartRegister=%u pConstantData=%p BoolCount=%u\n",
        This, StartRegister, pConstantData, BoolCount);

    user_assert(StartRegister              < NINE_MAX_CONST_B, D3DERR_INVALIDCALL);
    user_assert(StartRegister + BoolCount <= NINE_MAX_CONST_B, D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    if (!This->is_recording) {
        bool noChange = true;
        for (i = 0; i < BoolCount; i++) {
            if (!!state->ps_const_b[StartRegister + i] != !!pConstantData[i])
              noChange = false;
        }
        if (noChange)
            return D3D_OK;
    }

    for (i = 0; i < BoolCount; i++)
        state->ps_const_b[StartRegister + i] = pConstantData[i] ? bool_true : 0;

    if (unlikely(This->is_recording)) {
        state->changed.ps_const_b |= ((1 << BoolCount) - 1) << StartRegister;
        state->changed.group |= NINE_STATE_PS_CONST;
    } else
        nine_context_set_pixel_shader_constant_b(This, StartRegister, pConstantData,
                                                 sizeof(BOOL) * BoolCount, BoolCount);

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_GetPixelShaderConstantB( struct NineDevice9 *This,
                                     UINT StartRegister,
                                     BOOL *pConstantData,
                                     UINT BoolCount )
{
    const struct nine_state *state = &This->state;
    int i;

    user_assert(!This->pure, D3DERR_INVALIDCALL);
    user_assert(StartRegister              < NINE_MAX_CONST_B, D3DERR_INVALIDCALL);
    user_assert(StartRegister + BoolCount <= NINE_MAX_CONST_B, D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    for (i = 0; i < BoolCount; i++)
        pConstantData[i] = state->ps_const_b[StartRegister + i] ? true : false;

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9_DrawRectPatch( struct NineDevice9 *This,
                           UINT Handle,
                           const float *pNumSegs,
                           const D3DRECTPATCH_INFO *pRectPatchInfo )
{
    STUB(D3DERR_INVALIDCALL);
}

HRESULT NINE_WINAPI
NineDevice9_DrawTriPatch( struct NineDevice9 *This,
                          UINT Handle,
                          const float *pNumSegs,
                          const D3DTRIPATCH_INFO *pTriPatchInfo )
{
    STUB(D3DERR_INVALIDCALL);
}

HRESULT NINE_WINAPI
NineDevice9_DeletePatch( struct NineDevice9 *This,
                         UINT Handle )
{
    STUB(D3DERR_INVALIDCALL);
}

HRESULT NINE_WINAPI
NineDevice9_CreateQuery( struct NineDevice9 *This,
                         D3DQUERYTYPE Type,
                         IDirect3DQuery9 **ppQuery )
{
    struct NineQuery9 *query;
    HRESULT hr;

    DBG("This=%p Type=%d ppQuery=%p\n", This, Type, ppQuery);

    hr = nine_is_query_supported(This->screen, Type);
    if (!ppQuery || hr != D3D_OK)
        return hr;

    hr = NineQuery9_new(This, &query, Type);
    if (FAILED(hr))
        return hr;
    *ppQuery = (IDirect3DQuery9 *)query;
    return D3D_OK;
}

IDirect3DDevice9Vtbl NineDevice9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_Release,
    (void *)NineDevice9_TestCooperativeLevel,
    (void *)NineDevice9_GetAvailableTextureMem,
    (void *)NineDevice9_EvictManagedResources,
    (void *)NineDevice9_GetDirect3D,
    (void *)NineDevice9_GetDeviceCaps,
    (void *)NineDevice9_GetDisplayMode,
    (void *)NineDevice9_GetCreationParameters,
    (void *)NineDevice9_SetCursorProperties,
    (void *)NineDevice9_SetCursorPosition,
    (void *)NineDevice9_ShowCursor,
    (void *)NineDevice9_CreateAdditionalSwapChain,
    (void *)NineDevice9_GetSwapChain,
    (void *)NineDevice9_GetNumberOfSwapChains,
    (void *)NineDevice9_Reset,
    (void *)NineDevice9_Present,
    (void *)NineDevice9_GetBackBuffer,
    (void *)NineDevice9_GetRasterStatus,
    (void *)NineDevice9_SetDialogBoxMode,
    (void *)NineDevice9_SetGammaRamp,
    (void *)NineDevice9_GetGammaRamp,
    (void *)NineDevice9_CreateTexture,
    (void *)NineDevice9_CreateVolumeTexture,
    (void *)NineDevice9_CreateCubeTexture,
    (void *)NineDevice9_CreateVertexBuffer,
    (void *)NineDevice9_CreateIndexBuffer,
    (void *)NineDevice9_CreateRenderTarget,
    (void *)NineDevice9_CreateDepthStencilSurface,
    (void *)NineDevice9_UpdateSurface,
    (void *)NineDevice9_UpdateTexture,
    (void *)NineDevice9_GetRenderTargetData,
    (void *)NineDevice9_GetFrontBufferData,
    (void *)NineDevice9_StretchRect,
    (void *)NineDevice9_ColorFill,
    (void *)NineDevice9_CreateOffscreenPlainSurface,
    (void *)NineDevice9_SetRenderTarget,
    (void *)NineDevice9_GetRenderTarget,
    (void *)NineDevice9_SetDepthStencilSurface,
    (void *)NineDevice9_GetDepthStencilSurface,
    (void *)NineDevice9_BeginScene,
    (void *)NineDevice9_EndScene,
    (void *)NineDevice9_Clear,
    (void *)NineDevice9_SetTransform,
    (void *)NineDevice9_GetTransform,
    (void *)NineDevice9_MultiplyTransform,
    (void *)NineDevice9_SetViewport,
    (void *)NineDevice9_GetViewport,
    (void *)NineDevice9_SetMaterial,
    (void *)NineDevice9_GetMaterial,
    (void *)NineDevice9_SetLight,
    (void *)NineDevice9_GetLight,
    (void *)NineDevice9_LightEnable,
    (void *)NineDevice9_GetLightEnable,
    (void *)NineDevice9_SetClipPlane,
    (void *)NineDevice9_GetClipPlane,
    (void *)NineDevice9_SetRenderState,
    (void *)NineDevice9_GetRenderState,
    (void *)NineDevice9_CreateStateBlock,
    (void *)NineDevice9_BeginStateBlock,
    (void *)NineDevice9_EndStateBlock,
    (void *)NineDevice9_SetClipStatus,
    (void *)NineDevice9_GetClipStatus,
    (void *)NineDevice9_GetTexture,
    (void *)NineDevice9_SetTexture,
    (void *)NineDevice9_GetTextureStageState,
    (void *)NineDevice9_SetTextureStageState,
    (void *)NineDevice9_GetSamplerState,
    (void *)NineDevice9_SetSamplerState,
    (void *)NineDevice9_ValidateDevice,
    (void *)NineDevice9_SetPaletteEntries,
    (void *)NineDevice9_GetPaletteEntries,
    (void *)NineDevice9_SetCurrentTexturePalette,
    (void *)NineDevice9_GetCurrentTexturePalette,
    (void *)NineDevice9_SetScissorRect,
    (void *)NineDevice9_GetScissorRect,
    (void *)NineDevice9_SetSoftwareVertexProcessing,
    (void *)NineDevice9_GetSoftwareVertexProcessing,
    (void *)NineDevice9_SetNPatchMode,
    (void *)NineDevice9_GetNPatchMode,
    (void *)NineDevice9_DrawPrimitive,
    (void *)NineDevice9_DrawIndexedPrimitive,
    (void *)NineDevice9_DrawPrimitiveUP,
    (void *)NineDevice9_DrawIndexedPrimitiveUP,
    (void *)NineDevice9_ProcessVertices,
    (void *)NineDevice9_CreateVertexDeclaration,
    (void *)NineDevice9_SetVertexDeclaration,
    (void *)NineDevice9_GetVertexDeclaration,
    (void *)NineDevice9_SetFVF,
    (void *)NineDevice9_GetFVF,
    (void *)NineDevice9_CreateVertexShader,
    (void *)NineDevice9_SetVertexShader,
    (void *)NineDevice9_GetVertexShader,
    (void *)NineDevice9_SetVertexShaderConstantF,
    (void *)NineDevice9_GetVertexShaderConstantF,
    (void *)NineDevice9_SetVertexShaderConstantI,
    (void *)NineDevice9_GetVertexShaderConstantI,
    (void *)NineDevice9_SetVertexShaderConstantB,
    (void *)NineDevice9_GetVertexShaderConstantB,
    (void *)NineDevice9_SetStreamSource,
    (void *)NineDevice9_GetStreamSource,
    (void *)NineDevice9_SetStreamSourceFreq,
    (void *)NineDevice9_GetStreamSourceFreq,
    (void *)NineDevice9_SetIndices,
    (void *)NineDevice9_GetIndices,
    (void *)NineDevice9_CreatePixelShader,
    (void *)NineDevice9_SetPixelShader,
    (void *)NineDevice9_GetPixelShader,
    (void *)NineDevice9_SetPixelShaderConstantF,
    (void *)NineDevice9_GetPixelShaderConstantF,
    (void *)NineDevice9_SetPixelShaderConstantI,
    (void *)NineDevice9_GetPixelShaderConstantI,
    (void *)NineDevice9_SetPixelShaderConstantB,
    (void *)NineDevice9_GetPixelShaderConstantB,
    (void *)NineDevice9_DrawRectPatch,
    (void *)NineDevice9_DrawTriPatch,
    (void *)NineDevice9_DeletePatch,
    (void *)NineDevice9_CreateQuery
};

static const GUID *NineDevice9_IIDs[] = {
    &IID_IDirect3DDevice9,
    &IID_IUnknown,
    NULL
};

HRESULT
NineDevice9_new( struct pipe_screen *pScreen,
                 D3DDEVICE_CREATION_PARAMETERS *pCreationParameters,
                 D3DCAPS9 *pCaps,
                 D3DPRESENT_PARAMETERS *pPresentationParameters,
                 IDirect3D9 *pD3D9,
                 ID3DPresentGroup *pPresentationGroup,
                 struct d3dadapter9_context *pCTX,
                 bool ex,
                 D3DDISPLAYMODEEX *pFullscreenDisplayMode,
                 struct NineDevice9 **ppOut,
                 int minorVersionNum )
{
    BOOL lock;
    lock = !!(pCreationParameters->BehaviorFlags & D3DCREATE_MULTITHREADED);

    NINE_NEW(Device9, ppOut, lock, /* args */
             pScreen, pCreationParameters, pCaps,
             pPresentationParameters, pD3D9, pPresentationGroup, pCTX,
             ex, pFullscreenDisplayMode, minorVersionNum );
}
