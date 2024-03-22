/*
 * Copyright © 2021 Raspberry Pi Ltd
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
 */

#include "v3dv_private.h"
#include "broadcom/common/v3d_macros.h"
#include "broadcom/cle/v3dx_pack.h"
#include "broadcom/compiler/v3d_compiler.h"

/*
 * Packs and ensure bo for the shader state (the latter can be temporal).
 */
static void
pack_texture_shader_state_helper(struct v3dv_device *device,
                                 struct v3dv_image_view *image_view,
                                 bool for_cube_map_array_storage)
{
   assert(!for_cube_map_array_storage ||
          image_view->vk.view_type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY);
   const uint32_t index = for_cube_map_array_storage ? 1 : 0;

   assert(image_view->vk.image);
   const struct v3dv_image *image = (struct v3dv_image *) image_view->vk.image;

   assert(image->vk.samples == VK_SAMPLE_COUNT_1_BIT ||
          image->vk.samples == VK_SAMPLE_COUNT_4_BIT);
   const uint32_t msaa_scale = image->vk.samples == VK_SAMPLE_COUNT_1_BIT ? 1 : 2;

   for (uint8_t plane = 0; plane < image_view->plane_count; plane++) {
      uint8_t iplane = image_view->planes[plane].image_plane;
      v3dvx_pack(image_view->planes[plane].texture_shader_state[index], TEXTURE_SHADER_STATE, tex) {

         tex.level_0_is_strictly_uif =
            (image->planes[iplane].slices[0].tiling == V3D_TILING_UIF_XOR ||
             image->planes[iplane].slices[0].tiling == V3D_TILING_UIF_NO_XOR);

         tex.level_0_xor_enable = (image->planes[iplane].slices[0].tiling == V3D_TILING_UIF_XOR);

         if (tex.level_0_is_strictly_uif)
            tex.level_0_ub_pad = image->planes[iplane].slices[0].ub_pad;

         /* FIXME: v3d never sets uif_xor_disable, but uses it on the following
          * check so let's set the default value
          */
         tex.uif_xor_disable = false;
         if (tex.uif_xor_disable ||
             tex.level_0_is_strictly_uif) {
            tex.extended = true;
         }

         tex.base_level = image_view->vk.base_mip_level;
         tex.max_level = image_view->vk.base_mip_level +
                         image_view->vk.level_count - 1;

         tex.swizzle_r = v3d_translate_pipe_swizzle(image_view->planes[plane].swizzle[0]);
         tex.swizzle_g = v3d_translate_pipe_swizzle(image_view->planes[plane].swizzle[1]);
         tex.swizzle_b = v3d_translate_pipe_swizzle(image_view->planes[plane].swizzle[2]);
         tex.swizzle_a = v3d_translate_pipe_swizzle(image_view->planes[plane].swizzle[3]);

         tex.texture_type = image_view->format->planes[plane].tex_type;

         if (image->vk.image_type == VK_IMAGE_TYPE_3D) {
            tex.image_depth = image->vk.extent.depth;
         } else {
            tex.image_depth = image_view->vk.layer_count;
         }

         /* Empirical testing with CTS shows that when we are sampling from cube
          * arrays we want to set image depth to layers / 6, but not when doing
          * image load/store.
          */
         if (image_view->vk.view_type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY &&
             !for_cube_map_array_storage) {
            assert(tex.image_depth % 6 == 0);
            tex.image_depth /= 6;
         }

         tex.image_height = image->planes[iplane].height * msaa_scale;
         tex.image_width = image->planes[iplane].width * msaa_scale;

         /* On 4.x, the height of a 1D texture is redefined to be the
          * upper 14 bits of the width (which is only usable with txf).
          */
         if (image->vk.image_type == VK_IMAGE_TYPE_1D)
            tex.image_height = tex.image_width >> 14;

         tex.image_width &= (1 << 14) - 1;
         tex.image_height &= (1 << 14) - 1;

         tex.array_stride_64_byte_aligned = image->planes[iplane].cube_map_stride / 64;

         /* At this point we don't have the job. That's the reason the first
          * parameter is NULL, to avoid a crash when cl_pack_emit_reloc tries to
          * add the bo to the job. This also means that we need to add manually
          * the image bo to the job using the texture.
          */
         const uint32_t base_offset =
            image->planes[iplane].mem->bo->offset +
            v3dv_layer_offset(image, 0, image_view->vk.base_array_layer,
                              iplane);
         tex.texture_base_pointer = v3dv_cl_address(NULL, base_offset);

         bool is_srgb = vk_format_is_srgb(image_view->vk.format);

         /* V3D 4.x doesn't have the reverse and swap_r/b bits, so we compose
          * the reverse and/or swap_r/b swizzle from the format table with the
          * image view swizzle. This, however, doesn't work for border colors,
          * for that there is the reverse_standard_border_color.
          *
          * In v3d 7.x, however, there is no reverse_standard_border_color bit,
          * since the reverse and swap_r/b bits also affect border colors. It is
          * because of this that we absolutely need to use these bits with
          * reversed and swpaped formats, since that's the only way to ensure
          * correct border colors. In that case we don't want to program the
          * swizzle to the composition of the format swizzle and the view
          * swizzle like we do in v3d 4.x, since the format swizzle is applied
          * via the reverse and swap_r/b bits.
          */
#if V3D_VERSION == 42
         tex.srgb = is_srgb;
         tex.reverse_standard_border_color =
            image_view->planes[plane].channel_reverse;
#endif
#if V3D_VERSION >= 71
         tex.transfer_func = is_srgb ? TRANSFER_FUNC_SRGB : TRANSFER_FUNC_NONE;

         tex.reverse = image_view->planes[plane].channel_reverse;
         tex.r_b_swap = image_view->planes[plane].swap_rb;

         if (tex.reverse || tex.r_b_swap) {
            tex.swizzle_r =
               v3d_translate_pipe_swizzle(image_view->view_swizzle[0]);
            tex.swizzle_g =
               v3d_translate_pipe_swizzle(image_view->view_swizzle[1]);
            tex.swizzle_b =
               v3d_translate_pipe_swizzle(image_view->view_swizzle[2]);
            tex.swizzle_a =
               v3d_translate_pipe_swizzle(image_view->view_swizzle[3]);
         }

         tex.chroma_offset_x = 1;
         tex.chroma_offset_y = 1;
         /* See comment in XML field definition for rationale of the shifts */
         tex.texture_base_pointer_cb = base_offset >> 6;
         tex.texture_base_pointer_cr = base_offset >> 6;
#endif
      }
   }
}

void
v3dX(pack_texture_shader_state)(struct v3dv_device *device,
                                struct v3dv_image_view *iview)
{
   pack_texture_shader_state_helper(device, iview, false);
   if (iview->vk.view_type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY)
      pack_texture_shader_state_helper(device, iview, true);
}

void
v3dX(pack_texture_shader_state_from_buffer_view)(struct v3dv_device *device,
                                                 struct v3dv_buffer_view *buffer_view)
{
   assert(buffer_view->buffer);
   const struct v3dv_buffer *buffer = buffer_view->buffer;

   v3dvx_pack(buffer_view->texture_shader_state, TEXTURE_SHADER_STATE, tex) {
      tex.swizzle_r =
         v3d_translate_pipe_swizzle(buffer_view->format->planes[0].swizzle[0]);
      tex.swizzle_g =
         v3d_translate_pipe_swizzle(buffer_view->format->planes[0].swizzle[1]);
      tex.swizzle_b =
         v3d_translate_pipe_swizzle(buffer_view->format->planes[0].swizzle[2]);
      tex.swizzle_a =
         v3d_translate_pipe_swizzle(buffer_view->format->planes[0].swizzle[3]);

      tex.image_depth = 1;

      /* On 4.x, the height of a 1D texture is redefined to be the upper 14
       * bits of the width (which is only usable with txf) (or in other words,
       * we are providing a 28 bit field for size, but split on the usual
       * 14bit height/width).
       */
      tex.image_width = buffer_view->num_elements;
      tex.image_height = tex.image_width >> 14;
      tex.image_width &= (1 << 14) - 1;
      tex.image_height &= (1 << 14) - 1;

      assert(buffer_view->format->plane_count == 1);
      tex.texture_type = buffer_view->format->planes[0].tex_type;

      bool is_srgb = vk_format_is_srgb(buffer_view->vk_format);
#if V3D_VERSION == 42
      tex.srgb = is_srgb;
#endif
#if V3D_VERSION >= 71
      tex.transfer_func = is_srgb ? TRANSFER_FUNC_SRGB : TRANSFER_FUNC_NONE;
#endif

      /* At this point we don't have the job. That's the reason the first
       * parameter is NULL, to avoid a crash when cl_pack_emit_reloc tries to
       * add the bo to the job. This also means that we need to add manually
       * the image bo to the job using the texture.
       */
      const uint32_t base_offset =
         buffer->mem->bo->offset +
         buffer->mem_offset +
         buffer_view->offset;

      tex.texture_base_pointer = v3dv_cl_address(NULL, base_offset);

#if V3D_VERSION >= 71
      tex.chroma_offset_x = 1;
      tex.chroma_offset_y = 1;
      /* See comment in XML field definition for rationale of the shifts */
      tex.texture_base_pointer_cb = base_offset >> 6;
      tex.texture_base_pointer_cr = base_offset >> 6;
#endif
   }
}
