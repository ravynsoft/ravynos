/* Render (non-compute) states must be first. */
ST_STATE(ST_NEW_DSA, st_update_depth_stencil_alpha)
ST_STATE(ST_NEW_CLIP_STATE, st_update_clip)

ST_STATE(ST_NEW_FS_STATE, st_update_fp)
ST_STATE(ST_NEW_GS_STATE, st_update_gp)
ST_STATE(ST_NEW_TES_STATE, st_update_tep)
ST_STATE(ST_NEW_TCS_STATE, st_update_tcp)
ST_STATE(ST_NEW_VS_STATE, st_update_vp)

ST_STATE(ST_NEW_POLY_STIPPLE, st_update_polygon_stipple)
ST_STATE(ST_NEW_WINDOW_RECTANGLES, st_update_window_rectangles)
ST_STATE(ST_NEW_BLEND_COLOR, st_update_blend_color)

ST_STATE(ST_NEW_VS_SAMPLER_VIEWS, st_update_vertex_textures)
ST_STATE(ST_NEW_FS_SAMPLER_VIEWS, st_update_fragment_textures)
ST_STATE(ST_NEW_GS_SAMPLER_VIEWS, st_update_geometry_textures)
ST_STATE(ST_NEW_TCS_SAMPLER_VIEWS, st_update_tessctrl_textures)
ST_STATE(ST_NEW_TES_SAMPLER_VIEWS, st_update_tesseval_textures)

/* Non-compute samplers. */
ST_STATE(ST_NEW_VS_SAMPLERS, st_update_vertex_samplers) /* depends on update_*_texture for swizzle */
ST_STATE(ST_NEW_TCS_SAMPLERS, st_update_tessctrl_samplers) /* depends on update_*_texture for swizzle */
ST_STATE(ST_NEW_TES_SAMPLERS, st_update_tesseval_samplers) /* depends on update_*_texture for swizzle */
ST_STATE(ST_NEW_GS_SAMPLERS, st_update_geometry_samplers) /* depends on update_*_texture for swizzle */
ST_STATE(ST_NEW_FS_SAMPLERS, st_update_fragment_samplers) /* depends on update_*_texture for swizzle */

ST_STATE(ST_NEW_VS_IMAGES, st_bind_vs_images)
ST_STATE(ST_NEW_TCS_IMAGES, st_bind_tcs_images)
ST_STATE(ST_NEW_TES_IMAGES, st_bind_tes_images)
ST_STATE(ST_NEW_GS_IMAGES, st_bind_gs_images)
ST_STATE(ST_NEW_FS_IMAGES, st_bind_fs_images)

ST_STATE(ST_NEW_FB_STATE, st_update_framebuffer_state) /* depends on update_*_texture and bind_*_images */
ST_STATE(ST_NEW_BLEND, st_update_blend) /* depends on update_framebuffer_state */
ST_STATE(ST_NEW_RASTERIZER, st_update_rasterizer) /* depends on update_framebuffer_state */
ST_STATE(ST_NEW_SAMPLE_STATE, st_update_sample_state) /* depends on update_framebuffer_state */
ST_STATE(ST_NEW_SAMPLE_SHADING, st_update_sample_shading)
ST_STATE(ST_NEW_SCISSOR, st_update_scissor) /* depends on update_framebuffer_state */
ST_STATE(ST_NEW_VIEWPORT, st_update_viewport) /* depends on update_framebuffer_state */

ST_STATE(ST_NEW_VS_CONSTANTS, st_update_vs_constants)
ST_STATE(ST_NEW_TCS_CONSTANTS, st_update_tcs_constants)
ST_STATE(ST_NEW_TES_CONSTANTS, st_update_tes_constants)
ST_STATE(ST_NEW_GS_CONSTANTS, st_update_gs_constants)
ST_STATE(ST_NEW_FS_CONSTANTS, st_update_fs_constants)

ST_STATE(ST_NEW_VS_UBOS, st_bind_vs_ubos)
ST_STATE(ST_NEW_TCS_UBOS, st_bind_tcs_ubos)
ST_STATE(ST_NEW_TES_UBOS, st_bind_tes_ubos)
ST_STATE(ST_NEW_FS_UBOS, st_bind_fs_ubos)
ST_STATE(ST_NEW_GS_UBOS, st_bind_gs_ubos)

ST_STATE(ST_NEW_VS_ATOMICS, st_bind_vs_atomics)
ST_STATE(ST_NEW_TCS_ATOMICS, st_bind_tcs_atomics)
ST_STATE(ST_NEW_TES_ATOMICS, st_bind_tes_atomics)
ST_STATE(ST_NEW_FS_ATOMICS, st_bind_fs_atomics)
ST_STATE(ST_NEW_GS_ATOMICS, st_bind_gs_atomics)

/* SSBOs depend on the _atomics having been updated first in the
 * !has_hw_atomics case.
 */
ST_STATE(ST_NEW_VS_SSBOS, st_bind_vs_ssbos)
ST_STATE(ST_NEW_TCS_SSBOS, st_bind_tcs_ssbos)
ST_STATE(ST_NEW_TES_SSBOS, st_bind_tes_ssbos)
ST_STATE(ST_NEW_FS_SSBOS, st_bind_fs_ssbos)
ST_STATE(ST_NEW_GS_SSBOS, st_bind_gs_ssbos)

ST_STATE(ST_NEW_PIXEL_TRANSFER, st_update_pixel_transfer)
ST_STATE(ST_NEW_TESS_STATE, st_update_tess)

ST_STATE(ST_NEW_HW_ATOMICS, st_bind_hw_atomic_buffers)

/* this must be done after the vertex program update */
ST_STATE(ST_NEW_VERTEX_ARRAYS, st_update_array)

/* Compute states must be last. */
ST_STATE(ST_NEW_CS_STATE, st_update_cp)
ST_STATE(ST_NEW_CS_SAMPLER_VIEWS, st_update_compute_textures)
ST_STATE(ST_NEW_CS_SAMPLERS, st_update_compute_samplers) /* depends on update_compute_texture for swizzle */
ST_STATE(ST_NEW_CS_CONSTANTS, st_update_cs_constants)
ST_STATE(ST_NEW_CS_UBOS, st_bind_cs_ubos)
ST_STATE(ST_NEW_CS_ATOMICS, st_bind_cs_atomics)
ST_STATE(ST_NEW_CS_SSBOS, st_bind_cs_ssbos)
ST_STATE(ST_NEW_CS_IMAGES, st_bind_cs_images)
