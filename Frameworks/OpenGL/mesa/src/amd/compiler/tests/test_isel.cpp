/*
 * Copyright © 2020 Valve Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */
#include <llvm/Config/llvm-config.h>

#include "helpers.h"
#include "test_isel-spirv.h"

using namespace aco;

BEGIN_TEST(isel.interp.simple)
   QoShaderModuleCreateInfo vs = qoShaderModuleCreateInfoGLSL(VERTEX,
      layout(location = 0) in vec4 in_color;
      layout(location = 0) out vec4 out_color;
      void main() { out_color = in_color;
      }
   );
   QoShaderModuleCreateInfo fs = qoShaderModuleCreateInfoGLSL(FRAGMENT,
      layout(location = 0) in vec4 in_color;
      layout(location = 0) out vec4 out_color;
      void main() {
         //>> v1: %a_tmp = v_interp_p1_f32 %bx, %pm:m0 attr0.w
         //! v1: %a = v_interp_p2_f32 %by, %pm:m0, (kill)%a_tmp attr0.w
         //! v1: %r_tmp = v_interp_p1_f32 %bx, %pm:m0 attr0.x
         //! v1: %r = v_interp_p2_f32 %by, %pm:m0, (kill)%r_tmp attr0.x
         //! v1: %g_tmp = v_interp_p1_f32 %bx, %pm:m0 attr0.y
         //! v1: %g = v_interp_p2_f32 %by, %pm:m0, (kill)%g_tmp attr0.y
         //! v1: %b_tmp = v_interp_p1_f32 (kill)%bx, %pm:m0 attr0.z
         //! v1: %b = v_interp_p2_f32 (kill)%by, (kill)%pm:m0, (kill)%b_tmp attr0.z
         //! exp (kill)%r, (kill)%g, (kill)%b, (kill)%a mrt0
         out_color = in_color;
      }
   );

   PipelineBuilder pbld(get_vk_device(GFX9));
   pbld.add_vsfs(vs, fs);
   pbld.print_ir(VK_SHADER_STAGE_FRAGMENT_BIT, "ACO IR");
END_TEST

BEGIN_TEST(isel.compute.simple)
   for (unsigned i = GFX7; i <= GFX8; i++) {
      if (!set_variant((amd_gfx_level)i))
         continue;

      QoShaderModuleCreateInfo cs = qoShaderModuleCreateInfoGLSL(COMPUTE,
         layout(local_size_x=1) in;
         layout(binding=0) buffer Buf {
            uint res;
         };
         void main() {
            //>> v1: %data = p_parallelcopy 42
            //! buffer_store_dword (kill)%_, v1: undef, 0, (kill)%data disable_wqm storage:buffer
            res = 42;
         }
      );

      PipelineBuilder pbld(get_vk_device((amd_gfx_level)i));
      pbld.add_cs(cs);
      pbld.print_ir(VK_SHADER_STAGE_COMPUTE_BIT, "ACO IR", true);
   }
END_TEST

BEGIN_TEST(isel.gs.no_outputs)
   for (unsigned i = GFX8; i <= GFX10; i++) {
      if (!set_variant((amd_gfx_level)i))
         continue;

      QoShaderModuleCreateInfo vs = qoShaderModuleCreateInfoGLSL(VERTEX,
         void main() {}
      );

      QoShaderModuleCreateInfo gs = qoShaderModuleCreateInfoGLSL(GEOMETRY,
         layout(points) in;
         layout(points, max_vertices = 1) out;

         void main() {
            EmitVertex();
            EndPrimitive();
         }
      );

      PipelineBuilder pbld(get_vk_device((amd_gfx_level)i));
      pbld.add_stage(VK_SHADER_STAGE_VERTEX_BIT, vs);
      pbld.add_stage(VK_SHADER_STAGE_GEOMETRY_BIT, gs);
      pbld.create_pipeline();

      //! success
      fprintf(output, "success\n");
   }
END_TEST

BEGIN_TEST(isel.gs.no_verts)
   for (unsigned i = GFX8; i <= GFX10; i++) {
      if (!set_variant((amd_gfx_level)i))
         continue;

      QoShaderModuleCreateInfo vs = qoShaderModuleCreateInfoGLSL(VERTEX,
         void main() {}
      );

      QoShaderModuleCreateInfo gs = qoShaderModuleCreateInfoGLSL(GEOMETRY,
         layout(points) in;
         layout(points, max_vertices = 0) out;

         void main() {}
      );

      PipelineBuilder pbld(get_vk_device((amd_gfx_level)i));
      pbld.add_stage(VK_SHADER_STAGE_VERTEX_BIT, vs);
      pbld.add_stage(VK_SHADER_STAGE_GEOMETRY_BIT, gs);
      pbld.create_pipeline();

      //! success
      fprintf(output, "success\n");
   }
END_TEST

BEGIN_TEST(isel.sparse.clause)
   for (unsigned i = GFX10_3; i <= GFX10_3; i++) {
      if (!set_variant((amd_gfx_level)i))
         continue;

      QoShaderModuleCreateInfo cs = qoShaderModuleCreateInfoGLSL(COMPUTE,
         QO_EXTENSION GL_ARB_sparse_texture2 : require
         layout(local_size_x=1) in;
         layout(binding=0) uniform sampler2D tex;
         layout(binding=1) buffer Buf {
            vec4 res[4];
            uint code[4];
         };
         void main() {
            //! llvm_version: #llvm_ver
            //; if llvm_ver >= 12:
            //;    funcs['sample_res'] = lambda _: 'v[#_:#_]'
            //;    funcs['sample_coords'] = lambda _: '[v#_, v#_, v#_]'
            //; else:
            //;    funcs['sample_res'] = lambda _: 'v#_'
            //;    funcs['sample_coords'] = lambda _: '[v#_, v#_, v#_, v#_]'
            //>> v5: (noCSE)%zero0 = p_create_vector 0, 0, 0, 0, 0
            //>> v5: %_ = image_sample_lz_o %_, %_, (kill)%zero0, (kill)%_, %_, %_ dmask:xyzw 2d tfe
            //>> v5: (noCSE)%zero1 = p_create_vector 0, 0, 0, 0, 0
            //>> v5: %_ = image_sample_lz_o %_, %_, (kill)%zero1, (kill)%_, %_, %_ dmask:xyzw 2d tfe
            //>> v5: (noCSE)%zero2 = p_create_vector 0, 0, 0, 0, 0
            //>> v5: %_ = image_sample_lz_o %_, %_, (kill)%zero2, (kill)%_, %_, %_ dmask:xyzw 2d tfe
            //>> v5: (noCSE)%zero3 = p_create_vector 0, 0, 0, 0, 0
            //>> v5: %_ = image_sample_lz_o (kill)%_, (kill)%_, (kill)%zero3, (kill)%_, (kill)%_, (kill)%_ dmask:xyzw 2d tfe
            //>> s_clause 0x3
            //! image_sample_lz_o @sample_res, @sample_coords, @s256(img), @s128(samp) dmask:0xf dim:SQ_RSRC_IMG_2D tfe
            //! image_sample_lz_o @sample_res, @sample_coords, @s256(img), @s128(samp) dmask:0xf dim:SQ_RSRC_IMG_2D tfe
            //! image_sample_lz_o @sample_res, @sample_coords, @s256(img), @s128(samp) dmask:0xf dim:SQ_RSRC_IMG_2D tfe
            //! image_sample_lz_o @sample_res, @sample_coords, @s256(img), @s128(samp) dmask:0xf dim:SQ_RSRC_IMG_2D tfe
            code[0] = sparseTextureOffsetARB(tex, vec2(0.5), ivec2(1, 0), res[0]);
            code[1] = sparseTextureOffsetARB(tex, vec2(0.5), ivec2(2, 0), res[1]);
            code[2] = sparseTextureOffsetARB(tex, vec2(0.5), ivec2(3, 0), res[2]);
            code[3] = sparseTextureOffsetARB(tex, vec2(0.5), ivec2(4, 0), res[3]);
         }
      );

      fprintf(output, "llvm_version: %u\n", LLVM_VERSION_MAJOR);

      PipelineBuilder pbld(get_vk_device((amd_gfx_level)i));
      pbld.add_cs(cs);
      pbld.print_ir(VK_SHADER_STAGE_COMPUTE_BIT, "ACO IR", true);
      pbld.print_ir(VK_SHADER_STAGE_COMPUTE_BIT, "Assembly", true);
   }
END_TEST

BEGIN_TEST(isel.discard_early_exit.mrtz)
   QoShaderModuleCreateInfo vs = qoShaderModuleCreateInfoGLSL(VERTEX,
      void main() {}
   );
   QoShaderModuleCreateInfo fs = qoShaderModuleCreateInfoGLSL(FRAGMENT,
      void main() {
         if (gl_FragCoord.w > 0.5)
            discard;
         gl_FragDepth = 1.0 / gl_FragCoord.z;
      }
   );

   /* On GFX11, the discard early exit must use mrtz if the shader exports only depth. */
   //>> exp mrtz v0, off, off, off done ; $_ $_
   //! s_endpgm                         ; $_
   //! BB1:
   //! exp mrtz off, off, off, off done ; $_ $_
   //! s_endpgm                         ; $_

   PipelineBuilder pbld(get_vk_device(GFX11));
   pbld.add_vsfs(vs, fs);
   pbld.print_ir(VK_SHADER_STAGE_FRAGMENT_BIT, "Assembly");
END_TEST

BEGIN_TEST(isel.discard_early_exit.mrt0)
   QoShaderModuleCreateInfo vs = qoShaderModuleCreateInfoGLSL(VERTEX,
      void main() {}
   );
   QoShaderModuleCreateInfo fs = qoShaderModuleCreateInfoGLSL(FRAGMENT,
      layout(location = 0) out vec4 out_color;
      void main() {
         if (gl_FragCoord.w > 0.5)
            discard;
         out_color = vec4(1.0 / gl_FragCoord.z);
      }
   );

   /* On GFX11, the discard early exit must use mrt0 if the shader exports color. */
   //>> exp mrt0 v0, v0, v0, v0 done    ; $_ $_
   //! s_endpgm                         ; $_
   //! BB1:
   //! exp mrt0 off, off, off, off done ; $_ $_
   //! s_endpgm                         ; $_

   PipelineBuilder pbld(get_vk_device(GFX11));
   pbld.add_vsfs(vs, fs);
   pbld.print_ir(VK_SHADER_STAGE_FRAGMENT_BIT, "Assembly");
END_TEST

BEGIN_TEST(isel.s_bfe_mask_bits)
   QoShaderModuleCreateInfo cs = qoShaderModuleCreateInfoGLSL(COMPUTE,
      layout(local_size_x=1) in;
      layout(binding=0) buffer Buf {
         int res;
      };
      void main() {
         //>> s1: %bits, s1: (kill)%_:scc = s_and_b32 (kill)%_, 31
         //! s1: %src1 = s_pack_ll_b32_b16 0, (kill)%bits
         //! s1: %_, s1: (kill)%_:scc = s_bfe_i32 0xdeadbeef, (kill)%src1
         res = bitfieldExtract(0xdeadbeef, 0, res & 0x1f);
      }
   );

   PipelineBuilder pbld(get_vk_device(GFX10_3));
   pbld.add_cs(cs);
   pbld.print_ir(VK_SHADER_STAGE_COMPUTE_BIT, "ACO IR", true);
END_TEST
