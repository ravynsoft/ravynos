/*
 * Copyright © Microsoft Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "u_driconf.h"

void
u_driconf_fill_st_options(struct st_config_options *options,
                          const struct driOptionCache *optionCache)
{
#define query_option_impl(option, type) \
   options->option = driQueryOption##type(optionCache, #option)
#define query_bool_option(option) query_option_impl(option, b)
#define query_int_option(option) query_option_impl(option, i)
#define query_string_option(option) \
   do { \
      char *option = driQueryOptionstr(optionCache, #option); \
      if (*option) \
         options->option = strdup(option); \
   } while (0)

   query_bool_option(disable_blend_func_extended);
   query_bool_option(disable_arb_gpu_shader5);
   query_bool_option(disable_glsl_line_continuations);
   query_bool_option(disable_uniform_array_resize);
   query_string_option(alias_shader_extension);
   query_bool_option(allow_vertex_texture_bias);
   query_bool_option(force_compat_shaders);
   query_bool_option(force_glsl_extensions_warn);
   query_int_option(force_glsl_version);
   query_bool_option(allow_extra_pp_tokens);
   query_bool_option(allow_glsl_extension_directive_midshader);
   query_bool_option(allow_glsl_120_subset_in_110);
   query_bool_option(allow_glsl_builtin_const_expression);
   query_bool_option(allow_glsl_relaxed_es);
   query_bool_option(allow_glsl_builtin_variable_redeclaration);
   query_bool_option(allow_higher_compat_version);
   query_bool_option(allow_glsl_compat_shaders);
   query_bool_option(glsl_ignore_write_to_readonly_var);
   query_bool_option(glsl_zero_init);
   query_bool_option(force_integer_tex_nearest);
   query_bool_option(vs_position_always_invariant);
   query_bool_option(vs_position_always_precise);
   query_bool_option(force_glsl_abs_sqrt);
   query_bool_option(allow_glsl_cross_stage_interpolation_mismatch);
   query_bool_option(do_dce_before_clip_cull_analysis);
   query_bool_option(allow_draw_out_of_order);
   query_bool_option(glthread_nop_check_framebuffer_status);
   query_bool_option(ignore_map_unsynchronized);
   query_bool_option(ignore_discard_framebuffer);
   query_bool_option(force_gl_names_reuse);
   query_bool_option(force_gl_map_buffer_synchronized);
   query_bool_option(transcode_etc);
   query_bool_option(transcode_astc);
   query_string_option(force_gl_vendor);
   query_string_option(force_gl_renderer);
   query_string_option(mesa_extension_override);
   query_bool_option(allow_multisampled_copyteximage);

   driComputeOptionsSha1(optionCache, options->config_options_sha1);
}
