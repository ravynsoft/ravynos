/*
 * Copyright 2023 Alyssa Rosenzweig
 * Copyright 2023 Valve Corporation
 * SPDX-License-Identifier: MIT
 */
#include "libagx.h"
#include <agx_pack.h>

uint3
libagx_txs(constant struct agx_texture_packed *ptr, uint16_t lod,
           unsigned nr_comps, bool is_buffer, bool is_1d, bool is_2d,
           bool is_cube, bool is_array)
{
   agx_unpack(NULL, ptr, TEXTURE, d);

   /* From the Vulkan spec:
    *
    *    OpImageQuery*...  return 0 if the bound descriptor is a null descriptor
    */
   if (d.null)
      return 0;

   /* Buffer textures are lowered to 2D so the original size is irrecoverable.
    * Instead, we stash it in the software-defined section.
    */
   if (is_buffer)
      return d.software_defined;

   /* Load standard dimensions */
   uint3 size = (uint3)(d.width, d.height, d.depth);
   lod += d.first_level;

   /* Linear 2D arrays are special.
    *
    * TODO: Optimize this, since linear 2D arrays aren't needed for APIs and
    * this just gets used internally for blits.
    */
   if (is_2d && is_array && d.layout == AGX_LAYOUT_LINEAR)
      size.z = d.depth_linear;

   /* 1D Arrays have their second component as the layer count */
   if (is_1d && is_array)
      size.y = size.z;

   /* Adjust for LOD, do not adjust array size */
   for (uint c = 0; c < (nr_comps - (uint)is_array); ++c)
      size[c] = max(size[c] >> lod, 1u);

   /* Cube maps have equal width and height, we save some instructions by only
    * reading one. Dead code elimination will remove the redundant instructions.
    */
   if (is_cube)
      size.y = size.x;

   return size;
}

uint
libagx_texture_samples(constant struct agx_texture_packed *ptr)
{
   agx_unpack(NULL, ptr, TEXTURE, d);

   /* As above */
   if (d.null)
      return 0;

   /* We may assume the input is multisampled, so just check the samples */
   return (d.samples == AGX_SAMPLE_COUNT_2) ? 2 : 4;
}

uint
libagx_texture_levels(constant struct agx_texture_packed *ptr)
{
   agx_unpack(NULL, ptr, TEXTURE, d);

   /* As above */
   if (d.null)
      return 0;
   else
      return (d.last_level - d.first_level) + 1;
}

/*
 * Fix robustness behaviour of txf with out-of-bounds LOD. The hardware
 * returns the correct out-of-bounds colour for out-of-bounds coordinates,
 * just not LODs. So translate out-of-bounds LOD into an out-of-bounds
 * coordinate to get correct behaviour in 1 instruction.
 *
 * Returns the fixed X-coordinate.
 *
 * TODO: This looks like it might be an erratum workaround on G13 (Apple does
 * it), maybe check if G15 is affected.
 */
uint
libagx_lower_txf_robustness(constant struct agx_texture_packed *ptr, ushort lod,
                            uint x)
{
   agx_unpack(NULL, ptr, TEXTURE, d);

   bool oob = (lod > (d.last_level - d.first_level));
   return oob ? 0xFFFF : x;
}

static uint32_t
calculate_twiddled_coordinates(ushort2 coord, uint16_t tile_w_px,
                               uint16_t tile_h_px, uint32_t width_tl)
{
   /* Modulo by the tile width/height to get the offsets within the tile */
   ushort2 tile_mask_vec = (ushort2)(tile_w_px - 1, tile_h_px - 1);
   uint32_t tile_mask = upsample(tile_mask_vec.y, tile_mask_vec.x);
   uint32_t coord_xy = upsample(coord.y, coord.x);
   ushort2 offs_px = as_ushort2(coord_xy & tile_mask);
   uint32_t offset_within_tile_px = nir_interleave_agx(offs_px.x, offs_px.y);

   /* Get the coordinates of the corner of the tile */
   ushort2 tile_px = as_ushort2(coord_xy & ~tile_mask);

   /* tile row start (px) =
    *   (y // tile height) * (# of tiles/row) * (# of pix/tile) =
    *   align_down(y, tile height) / tile height * width_tl *tile width *
    *        tile height =
    *   align_down(y, tile height) * width_tl * tile width
    */
   uint32_t tile_row_start_px = tile_px.y * width_tl * tile_w_px;

   /* tile column start (px) =
    *   (x // tile width) * (# of pix/tile) =
    *   align_down(x, tile width) / tile width * tile width * tile height =
    *   align_down(x, tile width) * tile height
    */
   uint32_t tile_col_start_px = tile_px.x * tile_h_px;

   /* Get the total offset */
   return tile_row_start_px + tile_col_start_px + offset_within_tile_px;
}

uint64_t
libagx_image_texel_address(constant const struct agx_atomic_software_packed *ptr,
                           uint4 coord, uint sample_idx,
                           uint bytes_per_sample_B, bool is_msaa,
                           bool is_layered, bool return_index)
{
   agx_unpack(NULL, ptr, ATOMIC_SOFTWARE, d);

   /* We do not allow atomics on linear 2D or linear 2D arrays, as there are no
    * known use cases. So we're twiddled in this path.
    */
   uint total_px = calculate_twiddled_coordinates(
      convert_ushort2(coord.xy), d.tile_width, d.tile_height, d.tiles_per_row);

   if (is_layered)
      total_px += coord.z * d.layer_stride_pixels;

   uint sample_count = is_msaa ? d.sample_count : 1;
   uint total_sa = (total_px * d.sample_count) + sample_idx;

   if (return_index)
      return total_sa;
   else
      return d.base + (uint64_t)(total_sa * bytes_per_sample_B);
}

uint64_t
libagx_buffer_texel_address(
   constant const struct agx_pbe_buffer_software_packed *ptr, uint4 coord,
   uint bytes_per_pixel_B)
{
   agx_unpack(NULL, ptr, PBE_BUFFER_SOFTWARE, d);
   return d.base + (uint64_t)(coord.x * bytes_per_pixel_B);
}

/* Buffer texture lowerings */
bool
libagx_texture_is_rgb32(constant struct agx_texture_packed *ptr)
{
   agx_unpack(NULL, ptr, TEXTURE, d);
   return d.channels == AGX_CHANNELS_R32G32B32_EMULATED;
}

uint4
libagx_texture_load_rgb32(constant struct agx_texture_packed *ptr, uint coord,
                          bool is_float)
{
   agx_unpack(NULL, ptr, TEXTURE, d);
   global uint3 *data = (global uint3 *)(d.address + 12 * coord);

   return (uint4)(*data, is_float ? as_uint(1.0f) : 1);
}
