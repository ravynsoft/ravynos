/*
 * (C) Copyright IBM Corporation 2002, 2004
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file glxextensions.h
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */

#ifndef GLX_GLXEXTENSIONS_H
#define GLX_GLXEXTENSIONS_H

#ifdef __cplusplus
extern "C" {
#endif

enum
{
   ARB_context_flush_control_bit = 0,
   ARB_create_context_bit,
   ARB_create_context_no_error_bit,
   ARB_create_context_profile_bit,
   ARB_create_context_robustness_bit,
   ARB_fbconfig_float_bit,
   ARB_get_proc_address_bit,
   ARB_multisample_bit,
   EXT_buffer_age_bit,
   EXT_create_context_es2_profile_bit,
   EXT_create_context_es_profile_bit,
   EXT_fbconfig_packed_float_bit,
   EXT_framebuffer_sRGB_bit,
   EXT_import_context_bit,
   EXT_no_config_context_bit,
   EXT_swap_control_bit,
   EXT_swap_control_tear_bit,
   EXT_texture_from_pixmap_bit,
   EXT_visual_info_bit,
   EXT_visual_rating_bit,
   ATI_pixel_format_float_bit,
   INTEL_swap_event_bit,
   MESA_copy_sub_buffer_bit,
   MESA_gl_interop_bit,
   MESA_query_renderer_bit,
   MESA_swap_control_bit,
   MESA_swap_frame_usage_bit,
   NV_float_buffer_bit,
   OML_sync_control_bit,
   SGIS_multisample_bit,
   SGIX_fbconfig_bit,
   SGIX_pbuffer_bit,
   SGIX_visual_select_group_bit,
   SGI_make_current_read_bit,
   SGI_swap_control_bit,
   SGI_video_sync_bit,

   __NUM_GLX_EXTS,
};

#define __GLX_EXT_BYTES   ((__NUM_GLX_EXTS + 7) / 8)

/* From the GLX perspective, the ARB and EXT extensions are identical.  Use a
 * single bit for both.
 */
#define ARB_framebuffer_sRGB_bit EXT_framebuffer_sRGB_bit

enum
{
   GL_ARB_depth_texture_bit = 0,
   GL_ARB_draw_buffers_bit,
   GL_ARB_fragment_program_bit,
   GL_ARB_fragment_program_shadow_bit,
   GL_ARB_framebuffer_object_bit,
   GL_ARB_imaging_bit,
   GL_ARB_multisample_bit,
   GL_ARB_multitexture_bit,
   GL_ARB_occlusion_query_bit,
   GL_ARB_point_parameters_bit,
   GL_ARB_point_sprite_bit,
   GL_ARB_shadow_bit,
   GL_ARB_shadow_ambient_bit,
   GL_ARB_texture_border_clamp_bit,
   GL_ARB_texture_cube_map_bit,
   GL_ARB_texture_compression_bit,
   GL_ARB_texture_env_add_bit,
   GL_ARB_texture_env_combine_bit,
   GL_ARB_texture_env_crossbar_bit,
   GL_ARB_texture_env_dot3_bit,
   GL_ARB_texture_filter_anisotropic_bit,
   GL_ARB_texture_mirrored_repeat_bit,
   GL_ARB_texture_non_power_of_two_bit,
   GL_ARB_texture_rectangle_bit,
   GL_ARB_texture_rg_bit,
   GL_ARB_transpose_matrix_bit,
   GL_ARB_vertex_buffer_object_bit,
   GL_ARB_vertex_program_bit,
   GL_ARB_window_pos_bit,
   GL_EXT_abgr_bit,
   GL_EXT_bgra_bit,
   GL_EXT_blend_color_bit,
   GL_EXT_blend_equation_separate_bit,
   GL_EXT_blend_func_separate_bit,
   GL_EXT_blend_logic_op_bit,
   GL_EXT_blend_minmax_bit,
   GL_EXT_blend_subtract_bit,
   GL_EXT_clip_volume_hint_bit,
   GL_EXT_compiled_vertex_array_bit,
   GL_EXT_convolution_bit,
   GL_EXT_copy_texture_bit,
   GL_EXT_cull_vertex_bit,
   GL_EXT_depth_bounds_test_bit,
   GL_EXT_draw_range_elements_bit,
   GL_EXT_fog_coord_bit,
   GL_EXT_framebuffer_blit_bit,
   GL_EXT_framebuffer_multisample_bit,
   GL_EXT_framebuffer_object_bit,
   GL_EXT_framebuffer_sRGB_bit,
   GL_EXT_multi_draw_arrays_bit,
   GL_EXT_packed_depth_stencil_bit,
   GL_EXT_packed_pixels_bit,
   GL_EXT_paletted_texture_bit,
   GL_EXT_pixel_buffer_object_bit,
   GL_EXT_polygon_offset_bit,
   GL_EXT_rescale_normal_bit,
   GL_EXT_secondary_color_bit,
   GL_EXT_separate_specular_color_bit,
   GL_EXT_shadow_funcs_bit,
   GL_EXT_shared_texture_palette_bit,
   GL_EXT_stencil_two_side_bit,
   GL_EXT_stencil_wrap_bit,
   GL_EXT_subtexture_bit,
   GL_EXT_texture_bit,
   GL_EXT_texture3D_bit,
   GL_EXT_texture_compression_dxt1_bit,
   GL_EXT_texture_compression_s3tc_bit,
   GL_EXT_texture_edge_clamp_bit,
   GL_EXT_texture_env_combine_bit,
   GL_EXT_texture_env_dot3_bit,
   GL_EXT_texture_integer_bit,
   GL_EXT_texture_lod_bit,
   GL_EXT_texture_lod_bias_bit,
   GL_EXT_texture_mirror_clamp_bit,
   GL_EXT_vertex_array_bit,
   GL_3DFX_texture_compression_FXT1_bit,
   GL_APPLE_packed_pixels_bit,
   GL_APPLE_ycbcr_422_bit,
   GL_ATI_text_fragment_shader_bit,
   GL_ATI_texture_env_combine3_bit,
   GL_ATI_texture_float_bit,
   GL_ATI_texture_mirror_once_bit,
   GL_HP_convolution_border_modes_bit,
   GL_HP_occlusion_test_bit,
   GL_IBM_cull_vertex_bit,
   GL_IBM_pixel_filter_hint_bit,
   GL_IBM_rasterpos_clip_bit,
   GL_IBM_texture_clamp_nodraw_bit,
   GL_INGR_interlace_read_bit,
   GL_MESA_pack_invert_bit,
   GL_MESA_ycbcr_texture_bit,
   GL_NV_blend_square_bit,
   GL_NV_copy_depth_to_color_bit,
   GL_NV_depth_clamp_bit,
   GL_NV_fog_distance_bit,
   GL_NV_fragment_program_bit,
   GL_NV_fragment_program_option_bit,
   GL_NV_fragment_program2_bit,
   GL_NV_light_max_exponent_bit,
   GL_NV_multisample_filter_hint_bit,
   GL_NV_packed_depth_stencil_bit,
   GL_NV_point_sprite_bit,
   GL_NV_texgen_reflection_bit,
   GL_NV_texture_compression_vtc_bit,
   GL_NV_texture_env_combine4_bit,
   GL_NV_vertex_program_bit,
   GL_NV_vertex_program1_1_bit,
   GL_NV_vertex_program2_bit,
   GL_NV_vertex_program2_option_bit,
   GL_NV_vertex_program3_bit,
   GL_OES_compressed_paletted_texture_bit,
   GL_OES_read_format_bit,
   GL_SGI_color_matrix_bit,
   GL_SGI_color_table_bit,
   GL_SGI_texture_color_table_bit,
   GL_SGIS_generate_mipmap_bit,
   GL_SGIS_multisample_bit,
   GL_SGIS_texture_lod_bit,
   GL_SGIX_blend_alpha_minmax_bit,
   GL_SGIX_clipmap_bit,
   GL_SGIX_depth_texture_bit,
   GL_SGIX_fog_offset_bit,
   GL_SGIX_shadow_bit,
   GL_SGIX_texture_coordinate_clamp_bit,
   GL_SGIX_texture_lod_bias_bit,
   GL_SGIX_texture_range_bit,
   GL_SGIX_texture_scale_bias_bit,
   GL_SGIX_vertex_preclip_bit,
   GL_SGIX_vertex_preclip_hint_bit,
   GL_SGIX_ycrcb_bit,
   GL_SUN_convolution_border_modes_bit,
   GL_SUN_slice_accum_bit,

   /* This *MUST* go here.  If it gets put after the duplicate values it will
    * get the value after the last duplicate.
    */
   __NUM_GL_EXTS,


   /* Alias extension bits.  These extensions exist in either vendor-specific
    * or EXT form and were later promoted to either EXT or ARB form.  In all
    * cases, the meaning (to GLX) is *exactly* the same.  That's why
    * EXT_texture_env_combine is *NOT* an alias of ARB_texture_env_combine and
    * EXT_texture_env_dot3 is *NOT* an alias of ARB_texture_env_dot3.  Be
    * careful!  When in doubt, src/mesa/main/extensions.c is a great reference.
    */

   GL_ATI_blend_equation_separate_bit = GL_EXT_blend_equation_separate_bit,
   GL_ATI_draw_buffers_bit = GL_ARB_draw_buffers_bit,
   GL_ATIX_texture_env_combine3_bit = GL_ATI_texture_env_combine3_bit,
   GL_EXT_point_parameters_bit = GL_ARB_point_parameters_bit,
   GL_EXT_texture_env_add_bit = GL_ARB_texture_env_add_bit,
   GL_EXT_texture_filter_anisotropic_bit = GL_ARB_texture_filter_anisotropic_bit,
   GL_EXT_texture_rectangle_bit = GL_ARB_texture_rectangle_bit,
   GL_IBM_texture_mirrored_repeat_bit = GL_ARB_texture_mirrored_repeat_bit,
   GL_INGR_blend_func_separate_bit = GL_EXT_blend_func_separate_bit,
   GL_MESA_window_pos_bit = GL_ARB_window_pos_bit,
   GL_NV_texture_rectangle_bit = GL_ARB_texture_rectangle_bit,
   GL_SGIS_texture_border_clamp_bit = GL_ARB_texture_border_clamp_bit,
   GL_SGIS_texture_edge_clamp_bit = GL_EXT_texture_edge_clamp_bit,
   GL_SGIX_shadow_ambient_bit = GL_ARB_shadow_ambient_bit,
   GL_SUN_multi_draw_arrays_bit = GL_EXT_multi_draw_arrays_bit
};

#define __GL_EXT_BYTES   ((__NUM_GL_EXTS + 7) / 8)

struct glx_screen;
struct glx_context;

extern GLboolean __glXExtensionBitIsEnabled(struct glx_screen *psc,
                                            unsigned bit);
extern const char *__glXGetClientExtensions(Display *dpy);
extern void __glXCalculateUsableExtensions(struct glx_screen *psc,
                                           GLboolean
                                           display_is_direct_capable);

extern void __glXParseExtensionOverride(struct glx_screen *psc,
                                        const char *override);
extern void __IndirectGlParseExtensionOverride(struct glx_screen *psc,
                                               const char *override);
extern void __glXCalculateUsableGLExtensions(struct glx_context *gc,
                                             const char *server_string);
extern char *__glXGetClientGLExtensionString(int screen);

extern GLboolean __glExtensionBitIsEnabled(struct glx_context *gc,
                                           unsigned bit);

extern void
__glXEnableDirectExtension(struct glx_screen *psc, const char *name);


/* GLX_ALIAS should be used for functions with a non-void return type.
   GLX_ALIAS_VOID is for functions with a void return type. */
# ifdef HAVE_FUNC_ATTRIBUTE_ALIAS
#  define GLX_ALIAS(return_type, real_func, proto_args, args, aliased_func) \
   return_type  real_func  proto_args                                   \
   __attribute__ ((alias( # aliased_func ) ));
#  define GLX_ALIAS_VOID(real_func, proto_args, args, aliased_func) \
   GLX_ALIAS(void, real_func, proto_args, args, aliased_func)
# else
#  define GLX_ALIAS(return_type, real_func, proto_args, args, aliased_func) \
   return_type  real_func  proto_args                                   \
   { return aliased_func args ; }
#  define GLX_ALIAS_VOID(real_func, proto_args, args, aliased_func) \
   void  real_func  proto_args                                      \
   { aliased_func args ; }
# endif /* HAVE_FUNC_ATTRIBUTE_ALIAS */

#ifdef __cplusplus
}
#endif

#endif /* GLX_GLXEXTENSIONS_H */
