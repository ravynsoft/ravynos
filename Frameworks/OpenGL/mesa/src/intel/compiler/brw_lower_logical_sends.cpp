/*
 * Copyright © 2010, 2022 Intel Corporation
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

/**
 * @file brw_lower_logical_sends.cpp
 */

#include "brw_eu.h"
#include "brw_fs.h"
#include "brw_fs_builder.h"

using namespace brw;

static void
lower_urb_read_logical_send(const fs_builder &bld, fs_inst *inst)
{
   const intel_device_info *devinfo = bld.shader->devinfo;
   const bool per_slot_present =
      inst->src[URB_LOGICAL_SRC_PER_SLOT_OFFSETS].file != BAD_FILE;

   assert(inst->size_written % REG_SIZE == 0);
   assert(inst->header_size == 0);

   fs_reg payload_sources[2];
   unsigned header_size = 0;
   payload_sources[header_size++] = inst->src[URB_LOGICAL_SRC_HANDLE];
   if (per_slot_present)
      payload_sources[header_size++] = inst->src[URB_LOGICAL_SRC_PER_SLOT_OFFSETS];

   fs_reg payload = fs_reg(VGRF, bld.shader->alloc.allocate(header_size),
                           BRW_REGISTER_TYPE_F);
   bld.LOAD_PAYLOAD(payload, payload_sources, header_size, header_size);

   inst->opcode = SHADER_OPCODE_SEND;
   inst->header_size = header_size;

   inst->sfid = BRW_SFID_URB;
   inst->desc = brw_urb_desc(devinfo,
                             GFX8_URB_OPCODE_SIMD8_READ,
                             per_slot_present,
                             false,
                             inst->offset);

   inst->mlen = header_size;
   inst->ex_desc = 0;
   inst->ex_mlen = 0;
   inst->send_is_volatile = true;

   inst->resize_sources(4);

   inst->src[0] = brw_imm_ud(0); /* desc */
   inst->src[1] = brw_imm_ud(0); /* ex_desc */
   inst->src[2] = payload;
   inst->src[3] = brw_null_reg();
}

static void
lower_urb_read_logical_send_xe2(const fs_builder &bld, fs_inst *inst)
{
   const intel_device_info *devinfo = bld.shader->devinfo;
   assert(devinfo->has_lsc);

   assert(inst->size_written % (REG_SIZE * reg_unit(devinfo)) == 0);
   assert(inst->header_size == 0);

   /* Get the logical send arguments. */
   const fs_reg handle = inst->src[URB_LOGICAL_SRC_HANDLE];

   /* Calculate the total number of components of the payload. */
   const unsigned dst_comps = inst->size_written / (REG_SIZE * reg_unit(devinfo));

   fs_reg payload = bld.vgrf(BRW_REGISTER_TYPE_UD);

   bld.MOV(payload, handle);

   /* The low 24-bits of the URB handle is a byte offset into the URB area.
    * Add the (OWord) offset of the write to this value.
    */
   if (inst->offset) {
      bld.ADD(payload, payload, brw_imm_ud(inst->offset * 16));
      inst->offset = 0;
   }

   fs_reg offsets = inst->src[URB_LOGICAL_SRC_PER_SLOT_OFFSETS];
   if (offsets.file != BAD_FILE) {
      fs_reg offsets_B = bld.vgrf(BRW_REGISTER_TYPE_UD);
      bld.SHL(offsets_B, offsets, brw_imm_ud(4)); /* OWords -> Bytes */
      bld.ADD(payload, payload, offsets_B);
   }

   inst->sfid = BRW_SFID_URB;

   assert((dst_comps >= 1 && dst_comps <= 4) || dst_comps == 8);

   inst->desc = lsc_msg_desc(devinfo, LSC_OP_LOAD, inst->exec_size,
                             LSC_ADDR_SURFTYPE_FLAT, LSC_ADDR_SIZE_A32,
                             1 /* num_coordinates */,
                             LSC_DATA_SIZE_D32, dst_comps /* num_channels */,
                             false /* transpose */,
                             LSC_CACHE(devinfo, STORE, L1UC_L3UC),
                             false /* has_dest */);


   /* Update the original instruction. */
   inst->opcode = SHADER_OPCODE_SEND;
   inst->mlen = lsc_msg_desc_src0_len(devinfo, inst->desc);
   inst->ex_mlen = 0;
   inst->header_size = 0;
   inst->send_has_side_effects = true;
   inst->send_is_volatile = false;

   inst->resize_sources(4);

   inst->src[0] = brw_imm_ud(0);
   inst->src[1] = brw_imm_ud(0);

   inst->src[2] = payload;
   inst->src[3] = brw_null_reg();
}

static void
lower_urb_write_logical_send(const fs_builder &bld, fs_inst *inst)
{
   const intel_device_info *devinfo = bld.shader->devinfo;
   const bool per_slot_present =
      inst->src[URB_LOGICAL_SRC_PER_SLOT_OFFSETS].file != BAD_FILE;
   const bool channel_mask_present =
      inst->src[URB_LOGICAL_SRC_CHANNEL_MASK].file != BAD_FILE;

   assert(inst->header_size == 0);

   const unsigned length = 1 + per_slot_present + channel_mask_present +
                           inst->components_read(URB_LOGICAL_SRC_DATA);

   fs_reg *payload_sources = new fs_reg[length];
   fs_reg payload = fs_reg(VGRF, bld.shader->alloc.allocate(length),
                           BRW_REGISTER_TYPE_F);

   unsigned header_size = 0;
   payload_sources[header_size++] = inst->src[URB_LOGICAL_SRC_HANDLE];
   if (per_slot_present)
      payload_sources[header_size++] = inst->src[URB_LOGICAL_SRC_PER_SLOT_OFFSETS];

   if (channel_mask_present)
      payload_sources[header_size++] = inst->src[URB_LOGICAL_SRC_CHANNEL_MASK];

   for (unsigned i = header_size, j = 0; i < length; i++, j++)
      payload_sources[i] = offset(inst->src[URB_LOGICAL_SRC_DATA], bld, j);

   bld.LOAD_PAYLOAD(payload, payload_sources, length, header_size);

   delete [] payload_sources;

   inst->opcode = SHADER_OPCODE_SEND;
   inst->header_size = header_size;
   inst->dst = brw_null_reg();

   inst->sfid = BRW_SFID_URB;
   inst->desc = brw_urb_desc(devinfo,
                             GFX8_URB_OPCODE_SIMD8_WRITE,
                             per_slot_present,
                             channel_mask_present,
                             inst->offset);

   inst->mlen = length;
   inst->ex_desc = 0;
   inst->ex_mlen = 0;
   inst->send_has_side_effects = true;

   inst->resize_sources(4);

   inst->src[0] = brw_imm_ud(0); /* desc */
   inst->src[1] = brw_imm_ud(0); /* ex_desc */
   inst->src[2] = payload;
   inst->src[3] = brw_null_reg();
}

static void
lower_urb_write_logical_send_xe2(const fs_builder &bld, fs_inst *inst)
{
   const intel_device_info *devinfo = bld.shader->devinfo;
   assert(devinfo->has_lsc);

   /* Get the logical send arguments. */
   const fs_reg handle = inst->src[URB_LOGICAL_SRC_HANDLE];
   const fs_reg src = inst->components_read(URB_LOGICAL_SRC_DATA) ?
      inst->src[URB_LOGICAL_SRC_DATA] : fs_reg(brw_imm_ud(0));
   assert(type_sz(src.type) == 4);

   /* Calculate the total number of components of the payload. */
   const unsigned src_comps = MAX2(1, inst->components_read(URB_LOGICAL_SRC_DATA));
   const unsigned src_sz = type_sz(src.type);

   fs_reg payload = bld.vgrf(BRW_REGISTER_TYPE_UD);

   bld.MOV(payload, handle);

   /* The low 24-bits of the URB handle is a byte offset into the URB area.
    * Add the (OWord) offset of the write to this value.
    */
   if (inst->offset) {
      bld.ADD(payload, payload, brw_imm_ud(inst->offset * 16));
      inst->offset = 0;
   }

   fs_reg offsets = inst->src[URB_LOGICAL_SRC_PER_SLOT_OFFSETS];
   if (offsets.file != BAD_FILE) {
      fs_reg offsets_B = bld.vgrf(BRW_REGISTER_TYPE_UD);
      bld.SHL(offsets_B, offsets, brw_imm_ud(4)); /* OWords -> Bytes */
      bld.ADD(payload, payload, offsets_B);
   }

   const fs_reg cmask = inst->src[URB_LOGICAL_SRC_CHANNEL_MASK];
   unsigned mask = 0;

   if (cmask.file != BAD_FILE) {
      assert(cmask.file == IMM);
      assert(cmask.type == BRW_REGISTER_TYPE_UD);
      mask = cmask.ud >> 16;
   }

   fs_reg payload2 = bld.move_to_vgrf(src, src_comps);
   const unsigned ex_mlen = (src_comps * src_sz * inst->exec_size) / REG_SIZE;

   inst->sfid = BRW_SFID_URB;

   enum lsc_opcode op = mask ? LSC_OP_STORE_CMASK : LSC_OP_STORE;
   inst->desc = lsc_msg_desc_wcmask(devinfo, op, inst->exec_size,
                             LSC_ADDR_SURFTYPE_FLAT, LSC_ADDR_SIZE_A32,
                             1 /* num_coordinates */,
                             LSC_DATA_SIZE_D32, src_comps /* num_channels */,
                             false /* transpose */,
                             LSC_CACHE(devinfo, STORE, L1UC_L3UC),
                             false /* has_dest */, mask);


   /* Update the original instruction. */
   inst->opcode = SHADER_OPCODE_SEND;
   inst->mlen = lsc_msg_desc_src0_len(devinfo, inst->desc);
   inst->ex_mlen = ex_mlen;
   inst->header_size = 0;
   inst->send_has_side_effects = true;
   inst->send_is_volatile = false;

   inst->resize_sources(4);

   inst->src[0] = brw_imm_ud(0);
   inst->src[1] = brw_imm_ud(0);

   inst->src[2] = payload;
   inst->src[3] = payload2;
}

static void
setup_color_payload(const fs_builder &bld, const brw_wm_prog_key *key,
                    fs_reg *dst, fs_reg color, unsigned components)
{
   if (key->clamp_fragment_color) {
      fs_reg tmp = bld.vgrf(BRW_REGISTER_TYPE_F, 4);
      assert(color.type == BRW_REGISTER_TYPE_F);

      for (unsigned i = 0; i < components; i++)
         set_saturate(true,
                      bld.MOV(offset(tmp, bld, i), offset(color, bld, i)));

      color = tmp;
   }

   for (unsigned i = 0; i < components; i++)
      dst[i] = offset(color, bld, i);
}

static void
lower_fb_write_logical_send(const fs_builder &bld, fs_inst *inst,
                            const struct brw_wm_prog_data *prog_data,
                            const brw_wm_prog_key *key,
                            const fs_thread_payload &payload)
{
   assert(inst->src[FB_WRITE_LOGICAL_SRC_COMPONENTS].file == IMM);
   const intel_device_info *devinfo = bld.shader->devinfo;
   const fs_reg color0 = inst->src[FB_WRITE_LOGICAL_SRC_COLOR0];
   const fs_reg color1 = inst->src[FB_WRITE_LOGICAL_SRC_COLOR1];
   const fs_reg src0_alpha = inst->src[FB_WRITE_LOGICAL_SRC_SRC0_ALPHA];
   const fs_reg src_depth = inst->src[FB_WRITE_LOGICAL_SRC_SRC_DEPTH];
   const fs_reg dst_depth = inst->src[FB_WRITE_LOGICAL_SRC_DST_DEPTH];
   const fs_reg src_stencil = inst->src[FB_WRITE_LOGICAL_SRC_SRC_STENCIL];
   fs_reg sample_mask = inst->src[FB_WRITE_LOGICAL_SRC_OMASK];
   const unsigned components =
      inst->src[FB_WRITE_LOGICAL_SRC_COMPONENTS].ud;

   assert(inst->target != 0 || src0_alpha.file == BAD_FILE);

   /* We can potentially have a message length of up to 15, so we have to set
    * base_mrf to either 0 or 1 in order to fit in m0..m15.
    */
   fs_reg sources[15];
   int header_size = 2, payload_header_size;
   unsigned length = 0;

   if (devinfo->ver < 6) {
      /* TODO: Support SIMD32 on gfx4-5 */
      assert(bld.group() < 16);

      /* For gfx4-5, we always have a header consisting of g0 and g1.  We have
       * an implied MOV from g0,g1 to the start of the message.  The MOV from
       * g0 is handled by the hardware and the MOV from g1 is provided by the
       * generator.  This is required because, on gfx4-5, the generator may
       * generate two write messages with different message lengths in order
       * to handle AA data properly.
       *
       * Also, since the pixel mask goes in the g0 portion of the message and
       * since render target writes are the last thing in the shader, we write
       * the pixel mask directly into g0 and it will get copied as part of the
       * implied write.
       */
      if (prog_data->uses_kill) {
         bld.exec_all().group(1, 0)
            .MOV(retype(brw_vec1_grf(0, 0), BRW_REGISTER_TYPE_UW),
                 brw_sample_mask_reg(bld));
      }

      assert(length == 0);
      length = 2;
   } else if ((devinfo->verx10 <= 70 &&
               prog_data->uses_kill) ||
              (devinfo->ver < 11 &&
               (color1.file != BAD_FILE || key->nr_color_regions > 1))) {
      assert(devinfo->ver < 20);

      /* From the Sandy Bridge PRM, volume 4, page 198:
       *
       *     "Dispatched Pixel Enables. One bit per pixel indicating
       *      which pixels were originally enabled when the thread was
       *      dispatched. This field is only required for the end-of-
       *      thread message and on all dual-source messages."
       */
      const fs_builder ubld = bld.exec_all().group(8, 0);

      fs_reg header = ubld.vgrf(BRW_REGISTER_TYPE_UD, 2);
      if (bld.group() < 16) {
         /* The header starts off as g0 and g1 for the first half */
         ubld.group(16, 0).MOV(header, retype(brw_vec8_grf(0, 0),
                                              BRW_REGISTER_TYPE_UD));
      } else {
         /* The header starts off as g0 and g2 for the second half */
         assert(bld.group() < 32);
         const fs_reg header_sources[2] = {
            retype(brw_vec8_grf(0, 0), BRW_REGISTER_TYPE_UD),
            retype(brw_vec8_grf(2, 0), BRW_REGISTER_TYPE_UD),
         };
         ubld.LOAD_PAYLOAD(header, header_sources, 2, 0);

         /* Gfx12 will require additional fix-ups if we ever hit this path. */
         assert(devinfo->ver < 12);
      }

      uint32_t g00_bits = 0;

      /* Set "Source0 Alpha Present to RenderTarget" bit in message
       * header.
       */
      if (src0_alpha.file != BAD_FILE)
         g00_bits |= 1 << 11;

      /* Set computes stencil to render target */
      if (prog_data->computed_stencil)
         g00_bits |= 1 << 14;

      if (g00_bits) {
         /* OR extra bits into g0.0 */
         ubld.group(1, 0).OR(component(header, 0),
                             retype(brw_vec1_grf(0, 0),
                                    BRW_REGISTER_TYPE_UD),
                             brw_imm_ud(g00_bits));
      }

      /* Set the render target index for choosing BLEND_STATE. */
      if (inst->target > 0) {
         ubld.group(1, 0).MOV(component(header, 2), brw_imm_ud(inst->target));
      }

      if (prog_data->uses_kill) {
         ubld.group(1, 0).MOV(retype(component(header, 15),
                                     BRW_REGISTER_TYPE_UW),
                              brw_sample_mask_reg(bld));
      }

      assert(length == 0);
      sources[0] = header;
      sources[1] = horiz_offset(header, 8);
      length = 2;
   }
   assert(length == 0 || length == 2);
   header_size = length;

   if (payload.aa_dest_stencil_reg[0]) {
      assert(inst->group < 16);
      sources[length] = fs_reg(VGRF, bld.shader->alloc.allocate(1));
      bld.group(8, 0).exec_all().annotate("FB write stencil/AA alpha")
         .MOV(sources[length],
              fs_reg(brw_vec8_grf(payload.aa_dest_stencil_reg[0], 0)));
      length++;
   }

   if (src0_alpha.file != BAD_FILE) {
      for (unsigned i = 0; i < bld.dispatch_width() / 8; i++) {
         const fs_builder &ubld = bld.exec_all().group(8, i)
                                    .annotate("FB write src0 alpha");
         const fs_reg tmp = ubld.vgrf(BRW_REGISTER_TYPE_F);
         ubld.MOV(tmp, horiz_offset(src0_alpha, i * 8));
         setup_color_payload(ubld, key, &sources[length], tmp, 1);
         length++;
      }
   }

   if (sample_mask.file != BAD_FILE) {
      const fs_reg tmp(VGRF, bld.shader->alloc.allocate(reg_unit(devinfo)),
                       BRW_REGISTER_TYPE_UD);

      /* Hand over gl_SampleMask.  Only the lower 16 bits of each channel are
       * relevant.  Since it's unsigned single words one vgrf is always
       * 16-wide, but only the lower or higher 8 channels will be used by the
       * hardware when doing a SIMD8 write depending on whether we have
       * selected the subspans for the first or second half respectively.
       */
      assert(sample_mask.file != BAD_FILE && type_sz(sample_mask.type) == 4);
      sample_mask.type = BRW_REGISTER_TYPE_UW;
      sample_mask.stride *= 2;

      bld.exec_all().annotate("FB write oMask")
         .MOV(horiz_offset(retype(tmp, BRW_REGISTER_TYPE_UW),
                           inst->group % (16 * reg_unit(devinfo))),
              sample_mask);

      for (unsigned i = 0; i < reg_unit(devinfo); i++)
         sources[length++] = byte_offset(tmp, REG_SIZE * i);
   }

   payload_header_size = length;

   setup_color_payload(bld, key, &sources[length], color0, components);
   length += 4;

   if (color1.file != BAD_FILE) {
      setup_color_payload(bld, key, &sources[length], color1, components);
      length += 4;
   }

   if (src_depth.file != BAD_FILE) {
      sources[length] = src_depth;
      length++;
   }

   if (dst_depth.file != BAD_FILE) {
      sources[length] = dst_depth;
      length++;
   }

   if (src_stencil.file != BAD_FILE) {
      assert(devinfo->ver >= 9);
      assert(bld.dispatch_width() == 8 * reg_unit(devinfo));

      /* XXX: src_stencil is only available on gfx9+. dst_depth is never
       * available on gfx9+. As such it's impossible to have both enabled at the
       * same time and therefore length cannot overrun the array.
       */
      assert(length < 15 * reg_unit(devinfo));

      sources[length] = bld.vgrf(BRW_REGISTER_TYPE_UD);
      bld.exec_all().annotate("FB write OS")
         .MOV(retype(sources[length], BRW_REGISTER_TYPE_UB),
              subscript(src_stencil, BRW_REGISTER_TYPE_UB, 0));
      length++;
   }

   fs_inst *load;
   if (devinfo->ver >= 7) {
      /* Send from the GRF */
      fs_reg payload = fs_reg(VGRF, -1, BRW_REGISTER_TYPE_F);
      load = bld.LOAD_PAYLOAD(payload, sources, length, payload_header_size);
      payload.nr = bld.shader->alloc.allocate(regs_written(load));
      load->dst = payload;

      uint32_t msg_ctl = brw_fb_write_msg_control(inst, prog_data);

      inst->desc =
         (inst->group / 16) << 11 | /* rt slot group */
         brw_fb_write_desc(devinfo, inst->target, msg_ctl, inst->last_rt,
                           0 /* coarse_rt_write */);

      fs_reg desc = brw_imm_ud(0);
      if (prog_data->coarse_pixel_dispatch == BRW_ALWAYS) {
         inst->desc |= (1 << 18);
      } else if (prog_data->coarse_pixel_dispatch == BRW_SOMETIMES) {
         STATIC_ASSERT(BRW_WM_MSAA_FLAG_COARSE_RT_WRITES == (1 << 18));
         const fs_builder &ubld = bld.exec_all().group(8, 0);
         desc = ubld.vgrf(BRW_REGISTER_TYPE_UD);
         ubld.AND(desc, dynamic_msaa_flags(prog_data),
                  brw_imm_ud(BRW_WM_MSAA_FLAG_COARSE_RT_WRITES));
         desc = component(desc, 0);
      }

      uint32_t ex_desc = 0;
      if (devinfo->ver >= 11) {
         /* Set the "Render Target Index" and "Src0 Alpha Present" fields
          * in the extended message descriptor, in lieu of using a header.
          */
         ex_desc = inst->target << 12 | (src0_alpha.file != BAD_FILE) << 15;

         if (key->nr_color_regions == 0)
            ex_desc |= 1 << 20; /* Null Render Target */
      }
      inst->ex_desc = ex_desc;

      inst->opcode = SHADER_OPCODE_SEND;
      inst->resize_sources(3);
      inst->sfid = GFX6_SFID_DATAPORT_RENDER_CACHE;
      inst->src[0] = desc;
      inst->src[1] = brw_imm_ud(0);
      inst->src[2] = payload;
      inst->mlen = regs_written(load);
      inst->ex_mlen = 0;
      inst->header_size = header_size;
      inst->check_tdr = true;
      inst->send_has_side_effects = true;
   } else {
      /* Send from the MRF */
      load = bld.LOAD_PAYLOAD(fs_reg(MRF, 1, BRW_REGISTER_TYPE_F),
                              sources, length, payload_header_size);

      /* On pre-SNB, we have to interlace the color values.  LOAD_PAYLOAD
       * will do this for us if we just give it a COMPR4 destination.
       */
      if (devinfo->ver < 6 && bld.dispatch_width() == 16)
         load->dst.nr |= BRW_MRF_COMPR4;

      if (devinfo->ver < 6) {
         /* Set up src[0] for the implied MOV from grf0-1 */
         inst->resize_sources(1);
         inst->src[0] = brw_vec8_grf(0, 0);
      } else {
         inst->resize_sources(0);
      }
      inst->base_mrf = 1;
      inst->opcode = FS_OPCODE_FB_WRITE;
      inst->mlen = regs_written(load);
      inst->header_size = header_size;
   }
}

static void
lower_fb_read_logical_send(const fs_builder &bld, fs_inst *inst)
{
   const intel_device_info *devinfo = bld.shader->devinfo;
   const fs_builder &ubld = bld.exec_all().group(8, 0);
   const unsigned length = 2;
   const fs_reg header = ubld.vgrf(BRW_REGISTER_TYPE_UD, length);

   if (bld.group() < 16) {
      ubld.group(16, 0).MOV(header, retype(brw_vec8_grf(0, 0),
                                           BRW_REGISTER_TYPE_UD));
   } else {
      assert(bld.group() < 32);
      const fs_reg header_sources[] = {
         retype(brw_vec8_grf(0, 0), BRW_REGISTER_TYPE_UD),
         retype(brw_vec8_grf(2, 0), BRW_REGISTER_TYPE_UD)
      };
      ubld.LOAD_PAYLOAD(header, header_sources, ARRAY_SIZE(header_sources), 0);

      if (devinfo->ver >= 12) {
         /* On Gfx12 the Viewport and Render Target Array Index fields (AKA
          * Poly 0 Info) are provided in r1.1 instead of r0.0, and the render
          * target message header format was updated accordingly -- However
          * the updated format only works for the lower 16 channels in a
          * SIMD32 thread, since the higher 16 channels want the subspan data
          * from r2 instead of r1, so we need to copy over the contents of
          * r1.1 in order to fix things up.
          */
         ubld.group(1, 0).MOV(component(header, 9),
                              retype(brw_vec1_grf(1, 1), BRW_REGISTER_TYPE_UD));
      }
   }

   /* BSpec 12470 (Gfx8-11), BSpec 47842 (Gfx12+) :
    *
    *   "Must be zero for Render Target Read message."
    *
    * For bits :
    *   - 14 : Stencil Present to Render Target
    *   - 13 : Source Depth Present to Render Target
    *   - 12 : oMask to Render Target
    *   - 11 : Source0 Alpha Present to Render Target
    */
   ubld.group(1, 0).AND(component(header, 0),
                        component(header, 0),
                        brw_imm_ud(~INTEL_MASK(14, 11)));

   inst->resize_sources(1);
   inst->src[0] = header;
   inst->opcode = FS_OPCODE_FB_READ;
   inst->mlen = length;
   inst->header_size = length;
}

static void
lower_sampler_logical_send_gfx4(const fs_builder &bld, fs_inst *inst, opcode op,
                                const fs_reg &coordinate,
                                const fs_reg &shadow_c,
                                const fs_reg &lod, const fs_reg &lod2,
                                const fs_reg &surface,
                                const fs_reg &sampler,
                                unsigned coord_components,
                                unsigned grad_components)
{
   const bool has_lod = (op == SHADER_OPCODE_TXL || op == FS_OPCODE_TXB ||
                         op == SHADER_OPCODE_TXF || op == SHADER_OPCODE_TXS);
   fs_reg msg_begin(MRF, 1, BRW_REGISTER_TYPE_F);
   fs_reg msg_end = msg_begin;

   /* g0 header. */
   msg_end = offset(msg_end, bld.group(8, 0), 1);

   for (unsigned i = 0; i < coord_components; i++)
      bld.MOV(retype(offset(msg_end, bld, i), coordinate.type),
              offset(coordinate, bld, i));

   msg_end = offset(msg_end, bld, coord_components);

   /* Messages other than SAMPLE and RESINFO in SIMD16 and TXD in SIMD8
    * require all three components to be present and zero if they are unused.
    */
   if (coord_components > 0 &&
       (has_lod || shadow_c.file != BAD_FILE ||
        (op == SHADER_OPCODE_TEX && bld.dispatch_width() == 8))) {
      assert(coord_components <= 3);
      for (unsigned i = 0; i < 3 - coord_components; i++)
         bld.MOV(offset(msg_end, bld, i), brw_imm_f(0.0f));

      msg_end = offset(msg_end, bld, 3 - coord_components);
   }

   if (op == SHADER_OPCODE_TXD) {
      /* TXD unsupported in SIMD16 mode. */
      assert(bld.dispatch_width() == 8);

      /* the slots for u and v are always present, but r is optional */
      if (coord_components < 2)
         msg_end = offset(msg_end, bld, 2 - coord_components);

      /*  P   = u, v, r
       * dPdx = dudx, dvdx, drdx
       * dPdy = dudy, dvdy, drdy
       *
       * 1-arg: Does not exist.
       *
       * 2-arg: dudx   dvdx   dudy   dvdy
       *        dPdx.x dPdx.y dPdy.x dPdy.y
       *        m4     m5     m6     m7
       *
       * 3-arg: dudx   dvdx   drdx   dudy   dvdy   drdy
       *        dPdx.x dPdx.y dPdx.z dPdy.x dPdy.y dPdy.z
       *        m5     m6     m7     m8     m9     m10
       */
      for (unsigned i = 0; i < grad_components; i++)
         bld.MOV(offset(msg_end, bld, i), offset(lod, bld, i));

      msg_end = offset(msg_end, bld, MAX2(grad_components, 2));

      for (unsigned i = 0; i < grad_components; i++)
         bld.MOV(offset(msg_end, bld, i), offset(lod2, bld, i));

      msg_end = offset(msg_end, bld, MAX2(grad_components, 2));
   }

   if (has_lod) {
      /* Bias/LOD with shadow comparator is unsupported in SIMD16 -- *Without*
       * shadow comparator (including RESINFO) it's unsupported in SIMD8 mode.
       */
      assert(shadow_c.file != BAD_FILE ? bld.dispatch_width() == 8 :
             bld.dispatch_width() == 16);

      const brw_reg_type type =
         (op == SHADER_OPCODE_TXF || op == SHADER_OPCODE_TXS ?
          BRW_REGISTER_TYPE_UD : BRW_REGISTER_TYPE_F);
      bld.MOV(retype(msg_end, type), lod);
      msg_end = offset(msg_end, bld, 1);
   }

   if (shadow_c.file != BAD_FILE) {
      if (op == SHADER_OPCODE_TEX && bld.dispatch_width() == 8) {
         /* There's no plain shadow compare message, so we use shadow
          * compare with a bias of 0.0.
          */
         bld.MOV(msg_end, brw_imm_f(0.0f));
         msg_end = offset(msg_end, bld, 1);
      }

      bld.MOV(msg_end, shadow_c);
      msg_end = offset(msg_end, bld, 1);
   }

   inst->opcode = op;
   inst->src[0] = reg_undef;
   inst->src[1] = surface;
   inst->src[2] = sampler;
   inst->resize_sources(3);
   inst->base_mrf = msg_begin.nr;
   inst->mlen = msg_end.nr - msg_begin.nr;
   inst->header_size = 1;
}

static void
lower_sampler_logical_send_gfx5(const fs_builder &bld, fs_inst *inst, opcode op,
                                const fs_reg &coordinate,
                                const fs_reg &shadow_c,
                                const fs_reg &lod, const fs_reg &lod2,
                                const fs_reg &sample_index,
                                const fs_reg &surface,
                                const fs_reg &sampler,
                                unsigned coord_components,
                                unsigned grad_components)
{
   fs_reg message(MRF, 2, BRW_REGISTER_TYPE_F);
   fs_reg msg_coords = message;
   unsigned header_size = 0;

   if (inst->offset != 0) {
      /* The offsets set up by the visitor are in the m1 header, so we can't
       * go headerless.
       */
      header_size = 1;
      message.nr--;
   }

   for (unsigned i = 0; i < coord_components; i++)
      bld.MOV(retype(offset(msg_coords, bld, i), coordinate.type),
              offset(coordinate, bld, i));

   fs_reg msg_end = offset(msg_coords, bld, coord_components);
   fs_reg msg_lod = offset(msg_coords, bld, 4);

   if (shadow_c.file != BAD_FILE) {
      fs_reg msg_shadow = msg_lod;
      bld.MOV(msg_shadow, shadow_c);
      msg_lod = offset(msg_shadow, bld, 1);
      msg_end = msg_lod;
   }

   switch (op) {
   case SHADER_OPCODE_TXL:
   case FS_OPCODE_TXB:
      bld.MOV(msg_lod, lod);
      msg_end = offset(msg_lod, bld, 1);
      break;
   case SHADER_OPCODE_TXD:
      /**
       *  P   =  u,    v,    r
       * dPdx = dudx, dvdx, drdx
       * dPdy = dudy, dvdy, drdy
       *
       * Load up these values:
       * - dudx   dudy   dvdx   dvdy   drdx   drdy
       * - dPdx.x dPdy.x dPdx.y dPdy.y dPdx.z dPdy.z
       */
      msg_end = msg_lod;
      for (unsigned i = 0; i < grad_components; i++) {
         bld.MOV(msg_end, offset(lod, bld, i));
         msg_end = offset(msg_end, bld, 1);

         bld.MOV(msg_end, offset(lod2, bld, i));
         msg_end = offset(msg_end, bld, 1);
      }
      break;
   case SHADER_OPCODE_TXS:
      msg_lod = retype(msg_end, BRW_REGISTER_TYPE_UD);
      bld.MOV(msg_lod, lod);
      msg_end = offset(msg_lod, bld, 1);
      break;
   case SHADER_OPCODE_TXF:
      msg_lod = offset(msg_coords, bld, 3);
      bld.MOV(retype(msg_lod, BRW_REGISTER_TYPE_UD), lod);
      msg_end = offset(msg_lod, bld, 1);
      break;
   case SHADER_OPCODE_TXF_CMS:
      msg_lod = offset(msg_coords, bld, 3);
      /* lod */
      bld.MOV(retype(msg_lod, BRW_REGISTER_TYPE_UD), brw_imm_ud(0u));
      /* sample index */
      bld.MOV(retype(offset(msg_lod, bld, 1), BRW_REGISTER_TYPE_UD), sample_index);
      msg_end = offset(msg_lod, bld, 2);
      break;
   default:
      break;
   }

   inst->opcode = op;
   inst->src[0] = reg_undef;
   inst->src[1] = surface;
   inst->src[2] = sampler;
   inst->resize_sources(3);
   inst->base_mrf = message.nr;
   inst->mlen = msg_end.nr - message.nr;
   inst->header_size = header_size;

   /* Message length > MAX_SAMPLER_MESSAGE_SIZE disallowed by hardware. */
   assert(inst->mlen <= MAX_SAMPLER_MESSAGE_SIZE);
}

static bool
is_high_sampler(const struct intel_device_info *devinfo, const fs_reg &sampler)
{
   if (devinfo->verx10 <= 70)
      return false;

   return sampler.file != IMM || sampler.ud >= 16;
}

static unsigned
sampler_msg_type(const intel_device_info *devinfo,
                 opcode opcode, bool shadow_compare)
{
   assert(devinfo->ver >= 5);
   switch (opcode) {
   case SHADER_OPCODE_TEX:
      return shadow_compare ? GFX5_SAMPLER_MESSAGE_SAMPLE_COMPARE :
                              GFX5_SAMPLER_MESSAGE_SAMPLE;
   case FS_OPCODE_TXB:
      return shadow_compare ? GFX5_SAMPLER_MESSAGE_SAMPLE_BIAS_COMPARE :
                              GFX5_SAMPLER_MESSAGE_SAMPLE_BIAS;
   case SHADER_OPCODE_TXL:
      return shadow_compare ? GFX5_SAMPLER_MESSAGE_SAMPLE_LOD_COMPARE :
                              GFX5_SAMPLER_MESSAGE_SAMPLE_LOD;
   case SHADER_OPCODE_TXL_LZ:
      return shadow_compare ? GFX9_SAMPLER_MESSAGE_SAMPLE_C_LZ :
                              GFX9_SAMPLER_MESSAGE_SAMPLE_LZ;
   case SHADER_OPCODE_TXS:
   case SHADER_OPCODE_IMAGE_SIZE_LOGICAL:
      return GFX5_SAMPLER_MESSAGE_SAMPLE_RESINFO;
   case SHADER_OPCODE_TXD:
      assert(!shadow_compare || devinfo->verx10 >= 75);
      return shadow_compare ? HSW_SAMPLER_MESSAGE_SAMPLE_DERIV_COMPARE :
                              GFX5_SAMPLER_MESSAGE_SAMPLE_DERIVS;
   case SHADER_OPCODE_TXF:
      return GFX5_SAMPLER_MESSAGE_SAMPLE_LD;
   case SHADER_OPCODE_TXF_LZ:
      assert(devinfo->ver >= 9);
      return GFX9_SAMPLER_MESSAGE_SAMPLE_LD_LZ;
   case SHADER_OPCODE_TXF_CMS_W:
      assert(devinfo->ver >= 9);
      return GFX9_SAMPLER_MESSAGE_SAMPLE_LD2DMS_W;
   case SHADER_OPCODE_TXF_CMS:
      return devinfo->ver >= 7 ? GFX7_SAMPLER_MESSAGE_SAMPLE_LD2DMS :
                                 GFX5_SAMPLER_MESSAGE_SAMPLE_LD;
   case SHADER_OPCODE_TXF_UMS:
      assert(devinfo->ver >= 7);
      return GFX7_SAMPLER_MESSAGE_SAMPLE_LD2DSS;
   case SHADER_OPCODE_TXF_MCS:
      assert(devinfo->ver >= 7);
      return GFX7_SAMPLER_MESSAGE_SAMPLE_LD_MCS;
   case SHADER_OPCODE_LOD:
      return GFX5_SAMPLER_MESSAGE_LOD;
   case SHADER_OPCODE_TG4:
      assert(devinfo->ver >= 7);
      return shadow_compare ? GFX7_SAMPLER_MESSAGE_SAMPLE_GATHER4_C :
                              GFX7_SAMPLER_MESSAGE_SAMPLE_GATHER4;
      break;
   case SHADER_OPCODE_TG4_OFFSET:
      assert(devinfo->ver >= 7);
      return shadow_compare ? GFX7_SAMPLER_MESSAGE_SAMPLE_GATHER4_PO_C :
                              GFX7_SAMPLER_MESSAGE_SAMPLE_GATHER4_PO;
   case SHADER_OPCODE_SAMPLEINFO:
      return GFX6_SAMPLER_MESSAGE_SAMPLE_SAMPLEINFO;
   default:
      unreachable("not reached");
   }
}

/**
 * Emit a LOAD_PAYLOAD instruction while ensuring the sources are aligned to
 * the given requested_alignment_sz.
 */
static fs_inst *
emit_load_payload_with_padding(const fs_builder &bld, const fs_reg &dst,
                               const fs_reg *src, unsigned sources,
                               unsigned header_size,
                               unsigned requested_alignment_sz)
{
   unsigned length = 0;
   unsigned num_srcs =
      sources * DIV_ROUND_UP(requested_alignment_sz, bld.dispatch_width());
   fs_reg *src_comps = new fs_reg[num_srcs];

   for (unsigned i = 0; i < header_size; i++)
      src_comps[length++] = src[i];

   for (unsigned i = header_size; i < sources; i++) {
      unsigned src_sz =
         retype(dst, src[i].type).component_size(bld.dispatch_width());
      const enum brw_reg_type padding_payload_type =
         brw_reg_type_from_bit_size(type_sz(src[i].type) * 8,
                                    BRW_REGISTER_TYPE_UD);

      src_comps[length++] = src[i];

      /* Expand the real sources if component of requested payload type is
       * larger than real source component.
       */
      if (src_sz < requested_alignment_sz) {
         for (unsigned j = 0; j < (requested_alignment_sz / src_sz) - 1; j++) {
            src_comps[length++] = retype(fs_reg(), padding_payload_type);
         }
      }
   }

   fs_inst *inst = bld.LOAD_PAYLOAD(dst, src_comps, length, header_size);
   delete[] src_comps;

   return inst;
}

static void
lower_sampler_logical_send_gfx7(const fs_builder &bld, fs_inst *inst, opcode op,
                                const fs_reg &coordinate,
                                const fs_reg &shadow_c,
                                fs_reg lod, const fs_reg &lod2,
                                const fs_reg &min_lod,
                                const fs_reg &sample_index,
                                const fs_reg &mcs,
                                const fs_reg &surface,
                                const fs_reg &sampler,
                                const fs_reg &surface_handle,
                                const fs_reg &sampler_handle,
                                const fs_reg &tg4_offset,
                                unsigned payload_type_bit_size,
                                unsigned coord_components,
                                unsigned grad_components,
                                bool residency)
{
   const brw_compiler *compiler = bld.shader->compiler;
   const intel_device_info *devinfo = bld.shader->devinfo;
   const enum brw_reg_type payload_type =
      brw_reg_type_from_bit_size(payload_type_bit_size, BRW_REGISTER_TYPE_F);
   const enum brw_reg_type payload_unsigned_type =
      brw_reg_type_from_bit_size(payload_type_bit_size, BRW_REGISTER_TYPE_UD);
   const enum brw_reg_type payload_signed_type =
      brw_reg_type_from_bit_size(payload_type_bit_size, BRW_REGISTER_TYPE_D);
   unsigned reg_width = bld.dispatch_width() / 8;
   unsigned header_size = 0, length = 0;
   fs_reg sources[1 + MAX_SAMPLER_MESSAGE_SIZE];
   for (unsigned i = 0; i < ARRAY_SIZE(sources); i++)
      sources[i] = bld.vgrf(payload_type);

   /* We must have exactly one of surface/sampler and surface/sampler_handle */
   assert((surface.file == BAD_FILE) != (surface_handle.file == BAD_FILE));
   assert((sampler.file == BAD_FILE) != (sampler_handle.file == BAD_FILE));

   if (op == SHADER_OPCODE_TG4 || op == SHADER_OPCODE_TG4_OFFSET ||
       inst->offset != 0 || inst->eot ||
       op == SHADER_OPCODE_SAMPLEINFO ||
       sampler_handle.file != BAD_FILE ||
       is_high_sampler(devinfo, sampler) ||
       residency) {
      /* For general texture offsets (no txf workaround), we need a header to
       * put them in.
       *
       * TG4 needs to place its channel select in the header, for interaction
       * with ARB_texture_swizzle.  The sampler index is only 4-bits, so for
       * larger sampler numbers we need to offset the Sampler State Pointer in
       * the header.
       */
      fs_reg header = retype(sources[0], BRW_REGISTER_TYPE_UD);
      for (header_size = 0; header_size < reg_unit(devinfo); header_size++)
         sources[length++] = byte_offset(header, REG_SIZE * header_size);

      /* If we're requesting fewer than four channels worth of response,
       * and we have an explicit header, we need to set up the sampler
       * writemask.  It's reversed from normal: 1 means "don't write".
       */
      unsigned reg_count = regs_written(inst) - reg_unit(devinfo) * residency;
      if (!inst->eot && reg_count < 4 * reg_width) {
         assert(reg_count % reg_width == 0);
         unsigned mask = ~((1 << (reg_count / reg_width)) - 1) & 0xf;
         inst->offset |= mask << 12;
      }

      if (residency)
         inst->offset |= 1 << 23; /* g0.2 bit23 : Pixel Null Mask Enable */

      /* Build the actual header */
      const fs_builder ubld = bld.exec_all().group(8 * reg_unit(devinfo), 0);
      const fs_builder ubld1 = ubld.group(1, 0);
      ubld.MOV(header, retype(brw_vec8_grf(0, 0), BRW_REGISTER_TYPE_UD));
      if (inst->offset) {
         ubld1.MOV(component(header, 2), brw_imm_ud(inst->offset));
      } else if (bld.shader->stage != MESA_SHADER_VERTEX &&
                 bld.shader->stage != MESA_SHADER_FRAGMENT) {
         /* The vertex and fragment stages have g0.2 set to 0, so
          * header0.2 is 0 when g0 is copied. Other stages may not, so we
          * must set it to 0 to avoid setting undesirable bits in the
          * message.
          */
         ubld1.MOV(component(header, 2), brw_imm_ud(0));
      }

      if (sampler_handle.file != BAD_FILE) {
         /* Bindless sampler handles aren't relative to the sampler state
          * pointer passed into the shader through SAMPLER_STATE_POINTERS_*.
          * Instead, it's an absolute pointer relative to dynamic state base
          * address.
          *
          * Sampler states are 16 bytes each and the pointer we give here has
          * to be 32-byte aligned.  In order to avoid more indirect messages
          * than required, we assume that all bindless sampler states are
          * 32-byte aligned.  This sacrifices a bit of general state base
          * address space but means we can do something more efficient in the
          * shader.
          */
         if (compiler->use_bindless_sampler_offset) {
            assert(devinfo->ver >= 11);
            ubld1.OR(component(header, 3), sampler_handle, brw_imm_ud(1));
         } else {
            ubld1.MOV(component(header, 3), sampler_handle);
         }
      } else if (is_high_sampler(devinfo, sampler)) {
         fs_reg sampler_state_ptr =
            retype(brw_vec1_grf(0, 3), BRW_REGISTER_TYPE_UD);

         /* Gfx11+ sampler message headers include bits in 4:0 which conflict
          * with the ones included in g0.3 bits 4:0.  Mask them out.
          */
         if (devinfo->ver >= 11) {
            sampler_state_ptr = ubld1.vgrf(BRW_REGISTER_TYPE_UD);
            ubld1.AND(sampler_state_ptr,
                      retype(brw_vec1_grf(0, 3), BRW_REGISTER_TYPE_UD),
                      brw_imm_ud(INTEL_MASK(31, 5)));
         }

         if (sampler.file == BRW_IMMEDIATE_VALUE) {
            assert(sampler.ud >= 16);
            const int sampler_state_size = 16; /* 16 bytes */

            ubld1.ADD(component(header, 3), sampler_state_ptr,
                      brw_imm_ud(16 * (sampler.ud / 16) * sampler_state_size));
         } else {
            fs_reg tmp = ubld1.vgrf(BRW_REGISTER_TYPE_UD);
            ubld1.AND(tmp, sampler, brw_imm_ud(0x0f0));
            ubld1.SHL(tmp, tmp, brw_imm_ud(4));
            ubld1.ADD(component(header, 3), sampler_state_ptr, tmp);
         }
      } else if (devinfo->ver >= 11) {
         /* Gfx11+ sampler message headers include bits in 4:0 which conflict
          * with the ones included in g0.3 bits 4:0.  Mask them out.
          */
         ubld1.AND(component(header, 3),
                   retype(brw_vec1_grf(0, 3), BRW_REGISTER_TYPE_UD),
                   brw_imm_ud(INTEL_MASK(31, 5)));
      }
   }

   if (shadow_c.file != BAD_FILE) {
      bld.MOV(sources[length], shadow_c);
      length++;
   }

   bool coordinate_done = false;

   /* Set up the LOD info */
   switch (op) {
   case FS_OPCODE_TXB:
   case SHADER_OPCODE_TXL:
      if (devinfo->ver >= 9 && op == SHADER_OPCODE_TXL && lod.is_zero()) {
         op = SHADER_OPCODE_TXL_LZ;
         break;
      }
      bld.MOV(sources[length], lod);
      length++;
      break;
   case SHADER_OPCODE_TXD:
      /* TXD should have been lowered in SIMD16 mode (in SIMD32 mode in
       * Xe2+).
       */
      assert(bld.dispatch_width() == (8 * reg_unit(devinfo)));

      /* Load dPdx and the coordinate together:
       * [hdr], [ref], x, dPdx.x, dPdy.x, y, dPdx.y, dPdy.y, z, dPdx.z, dPdy.z
       */
      for (unsigned i = 0; i < coord_components; i++) {
         bld.MOV(sources[length++], offset(coordinate, bld, i));

         /* For cube map array, the coordinate is (u,v,r,ai) but there are
          * only derivatives for (u, v, r).
          */
         if (i < grad_components) {
            bld.MOV(sources[length++], offset(lod, bld, i));
            bld.MOV(sources[length++], offset(lod2, bld, i));
         }
      }

      coordinate_done = true;
      break;
   case SHADER_OPCODE_TXS:
      bld.MOV(retype(sources[length], payload_unsigned_type), lod);
      length++;
      break;
   case SHADER_OPCODE_IMAGE_SIZE_LOGICAL:
      /* We need an LOD; just use 0 */
      bld.MOV(retype(sources[length], payload_unsigned_type), brw_imm_ud(0));
      length++;
      break;
   case SHADER_OPCODE_TXF:
      /* Unfortunately, the parameters for LD are intermixed: u, lod, v, r.
       * On Gfx9 they are u, v, lod, r
       */
      bld.MOV(retype(sources[length++], payload_signed_type), coordinate);

      if (devinfo->ver >= 9) {
         if (coord_components >= 2) {
            bld.MOV(retype(sources[length], payload_signed_type),
                    offset(coordinate, bld, 1));
         } else {
            sources[length] = brw_imm_d(0);
         }
         length++;
      }

      if (devinfo->ver >= 9 && lod.is_zero()) {
         op = SHADER_OPCODE_TXF_LZ;
      } else {
         bld.MOV(retype(sources[length], payload_signed_type), lod);
         length++;
      }

      for (unsigned i = devinfo->ver >= 9 ? 2 : 1; i < coord_components; i++)
         bld.MOV(retype(sources[length++], payload_signed_type),
                 offset(coordinate, bld, i));

      coordinate_done = true;
      break;

   case SHADER_OPCODE_TXF_CMS:
   case SHADER_OPCODE_TXF_CMS_W:
   case SHADER_OPCODE_TXF_UMS:
   case SHADER_OPCODE_TXF_MCS:
      if (op == SHADER_OPCODE_TXF_UMS ||
          op == SHADER_OPCODE_TXF_CMS ||
          op == SHADER_OPCODE_TXF_CMS_W) {
         bld.MOV(retype(sources[length++], payload_unsigned_type), sample_index);
      }

      /* Data from the multisample control surface. */
      if (op == SHADER_OPCODE_TXF_CMS || op == SHADER_OPCODE_TXF_CMS_W) {
         unsigned num_mcs_components = 1;

         /* From the Gfx12HP BSpec: Render Engine - 3D and GPGPU Programs -
          * Shared Functions - 3D Sampler - Messages - Message Format:
          *
          *    ld2dms_w   si  mcs0 mcs1 mcs2  mcs3  u  v  r
          */
         if (op == SHADER_OPCODE_TXF_CMS_W)
            num_mcs_components = 2;

         for (unsigned i = 0; i < num_mcs_components; ++i) {
            /* Sampler always writes 4/8 register worth of data but for ld_mcs
             * only valid data is in first two register. So with 16-bit
             * payload, we need to split 2-32bit register into 4-16-bit
             * payload.
             *
             * From the Gfx12HP BSpec: Render Engine - 3D and GPGPU Programs -
             * Shared Functions - 3D Sampler - Messages - Message Format:
             *
             *    ld2dms_w   si  mcs0 mcs1 mcs2  mcs3  u  v  r
             */
            if (devinfo->verx10 >= 125 && op == SHADER_OPCODE_TXF_CMS_W) {
               fs_reg tmp = offset(mcs, bld, i);
               bld.MOV(retype(sources[length++], payload_unsigned_type),
                       mcs.file == IMM ? mcs :
                       subscript(tmp, payload_unsigned_type, 0));
               bld.MOV(retype(sources[length++], payload_unsigned_type),
                       mcs.file == IMM ? mcs :
                       subscript(tmp, payload_unsigned_type, 1));
            } else {
               bld.MOV(retype(sources[length++], payload_unsigned_type),
                       mcs.file == IMM ? mcs : offset(mcs, bld, i));
            }
         }
      }

      /* There is no offsetting for this message; just copy in the integer
       * texture coordinates.
       */
      for (unsigned i = 0; i < coord_components; i++)
         bld.MOV(retype(sources[length++], payload_signed_type),
                 offset(coordinate, bld, i));

      coordinate_done = true;
      break;
   case SHADER_OPCODE_TG4_OFFSET:
      /* More crazy intermixing */
      for (unsigned i = 0; i < 2; i++) /* u, v */
         bld.MOV(sources[length++], offset(coordinate, bld, i));

      for (unsigned i = 0; i < 2; i++) /* offu, offv */
         bld.MOV(retype(sources[length++], payload_signed_type),
                 offset(tg4_offset, bld, i));

      if (coord_components == 3) /* r if present */
         bld.MOV(sources[length++], offset(coordinate, bld, 2));

      coordinate_done = true;
      break;
   default:
      break;
   }

   /* Set up the coordinate (except for cases where it was done above) */
   if (!coordinate_done) {
      for (unsigned i = 0; i < coord_components; i++)
         bld.MOV(retype(sources[length++], payload_type),
                 offset(coordinate, bld, i));
   }

   if (min_lod.file != BAD_FILE) {
      /* Account for all of the missing coordinate sources */
      if (op == SHADER_OPCODE_TXD && devinfo->verx10 >= 125) {
         /* On DG2 and newer platforms, sample_d can only be used with 1D and
          * 2D surfaces, so the maximum number of gradient components is 2.
          * In spite of this limitation, the Bspec lists a mysterious R
          * component before the min_lod, so the maximum coordinate components
          * is 3.
          *
          * See bspec 45942, "Enable new message layout for cube array"
          */
         length += 3 - coord_components;
         length += (2 - grad_components) * 2;
      } else {
         length += 4 - coord_components;
         if (op == SHADER_OPCODE_TXD)
            length += (3 - grad_components) * 2;
      }

      bld.MOV(sources[length++], min_lod);

      /* Wa_14014595444: Populate MLOD as parameter 5 (twice). */
       if (devinfo->verx10 == 125 && op == FS_OPCODE_TXB &&
          !inst->shadow_compare)
         bld.MOV(sources[length++], min_lod);
   }

   const fs_reg src_payload =
      fs_reg(VGRF, bld.shader->alloc.allocate(length * reg_width),
                                              BRW_REGISTER_TYPE_F);
   /* In case of 16-bit payload each component takes one full register in
    * both SIMD8H and SIMD16H modes. In both cases one reg can hold 16
    * elements. In SIMD8H case hardware simply expects the components to be
    * padded (i.e., aligned on reg boundary).
    */
   fs_inst *load_payload_inst =
      emit_load_payload_with_padding(bld, src_payload, sources, length,
                                     header_size, REG_SIZE * reg_unit(devinfo));
   unsigned mlen = load_payload_inst->size_written / REG_SIZE;
   unsigned simd_mode = 0;
   if (payload_type_bit_size == 16) {
      assert(devinfo->ver >= 11);
      simd_mode = inst->exec_size <= 8 ? GFX10_SAMPLER_SIMD_MODE_SIMD8H :
                                         GFX10_SAMPLER_SIMD_MODE_SIMD16H;
   } else {
      simd_mode = inst->exec_size <= 8 ? BRW_SAMPLER_SIMD_MODE_SIMD8 :
                                         BRW_SAMPLER_SIMD_MODE_SIMD16;
   }

   /* Generate the SEND. */
   inst->opcode = SHADER_OPCODE_SEND;
   inst->mlen = mlen;
   inst->header_size = header_size;

   const unsigned msg_type =
      sampler_msg_type(devinfo, op, inst->shadow_compare);

   inst->sfid = BRW_SFID_SAMPLER;
   if (surface.file == IMM &&
       (sampler.file == IMM || sampler_handle.file != BAD_FILE)) {
      inst->desc = brw_sampler_desc(devinfo, surface.ud,
                                    sampler.file == IMM ? sampler.ud % 16 : 0,
                                    msg_type,
                                    simd_mode,
                                    0 /* return_format unused on gfx7+ */);
      inst->src[0] = brw_imm_ud(0);
      inst->src[1] = brw_imm_ud(0);
   } else if (surface_handle.file != BAD_FILE) {
      /* Bindless surface */
      assert(devinfo->ver >= 9);
      inst->desc = brw_sampler_desc(devinfo,
                                    GFX9_BTI_BINDLESS,
                                    sampler.file == IMM ? sampler.ud % 16 : 0,
                                    msg_type,
                                    simd_mode,
                                    0 /* return_format unused on gfx7+ */);

      /* For bindless samplers, the entire address is included in the message
       * header so we can leave the portion in the message descriptor 0.
       */
      if (sampler_handle.file != BAD_FILE || sampler.file == IMM) {
         inst->src[0] = brw_imm_ud(0);
      } else {
         const fs_builder ubld = bld.group(1, 0).exec_all();
         fs_reg desc = ubld.vgrf(BRW_REGISTER_TYPE_UD);
         ubld.SHL(desc, sampler, brw_imm_ud(8));
         inst->src[0] = component(desc, 0);
      }

      /* We assume that the driver provided the handle in the top 20 bits so
       * we can use the surface handle directly as the extended descriptor.
       */
      inst->src[1] = retype(surface_handle, BRW_REGISTER_TYPE_UD);
      inst->send_ex_bso = compiler->extended_bindless_surface_offset;
   } else {
      /* Immediate portion of the descriptor */
      inst->desc = brw_sampler_desc(devinfo,
                                    0, /* surface */
                                    0, /* sampler */
                                    msg_type,
                                    simd_mode,
                                    0 /* return_format unused on gfx7+ */);
      const fs_builder ubld = bld.group(1, 0).exec_all();
      fs_reg desc = ubld.vgrf(BRW_REGISTER_TYPE_UD);
      if (surface.equals(sampler)) {
         /* This case is common in GL */
         ubld.MUL(desc, surface, brw_imm_ud(0x101));
      } else {
         if (sampler_handle.file != BAD_FILE) {
            ubld.MOV(desc, surface);
         } else if (sampler.file == IMM) {
            ubld.OR(desc, surface, brw_imm_ud(sampler.ud << 8));
         } else {
            ubld.SHL(desc, sampler, brw_imm_ud(8));
            ubld.OR(desc, desc, surface);
         }
      }
      ubld.AND(desc, desc, brw_imm_ud(0xfff));

      inst->src[0] = component(desc, 0);
      inst->src[1] = brw_imm_ud(0); /* ex_desc */
   }

   inst->ex_desc = 0;

   inst->src[2] = src_payload;
   inst->resize_sources(3);

   if (inst->eot) {
      /* EOT sampler messages don't make sense to split because it would
       * involve ending half of the thread early.
       */
      assert(inst->group == 0);
      /* We need to use SENDC for EOT sampler messages */
      inst->check_tdr = true;
      inst->send_has_side_effects = true;
   }

   /* Message length > MAX_SAMPLER_MESSAGE_SIZE disallowed by hardware. */
   assert(inst->mlen <= MAX_SAMPLER_MESSAGE_SIZE * reg_unit(devinfo));
}

static unsigned
get_sampler_msg_payload_type_bit_size(const intel_device_info *devinfo,
                                      opcode op, const fs_reg *src)
{
   unsigned src_type_size = 0;

   /* All sources need to have the same size, therefore seek the first valid
    * and take the size from there.
    */
   for (unsigned i = 0; i < TEX_LOGICAL_NUM_SRCS; i++) {
      if (src[i].file != BAD_FILE) {
         src_type_size = brw_reg_type_to_size(src[i].type);
         break;
      }
   }

   assert(src_type_size == 2 || src_type_size == 4);

#ifndef NDEBUG
   /* Make sure all sources agree. On gfx12 this doesn't hold when sampling
    * compressed multisampled surfaces. There the payload contains MCS data
    * which is already in 16-bits unlike the other parameters that need forced
    * conversion.
    */
   if (devinfo->verx10 < 125 ||
       (op != SHADER_OPCODE_TXF_CMS_W &&
        op != SHADER_OPCODE_TXF_CMS)) {
      for (unsigned i = 0; i < TEX_LOGICAL_NUM_SRCS; i++) {
         assert(src[i].file == BAD_FILE ||
                brw_reg_type_to_size(src[i].type) == src_type_size);
      }
   }
#endif

   if (devinfo->verx10 < 125)
      return src_type_size * 8;

   /* Force conversion from 32-bit sources to 16-bit payload. From the XeHP Bspec:
    * 3D and GPGPU Programs - Shared Functions - 3D Sampler - Messages - Message
    * Format [GFX12:HAS:1209977870] *
    *
    *  ld2dms_w       SIMD8H and SIMD16H Only
    *  ld_mcs         SIMD8H and SIMD16H Only
    *  ld2dms         REMOVEDBY(GEN:HAS:1406788836)
    */

   if (op == SHADER_OPCODE_TXF_CMS_W ||
       op == SHADER_OPCODE_TXF_CMS ||
       op == SHADER_OPCODE_TXF_UMS ||
       op == SHADER_OPCODE_TXF_MCS)
      src_type_size = 2;

   return src_type_size * 8;
}

static void
lower_sampler_logical_send(const fs_builder &bld, fs_inst *inst, opcode op)
{
   const intel_device_info *devinfo = bld.shader->devinfo;
   const fs_reg coordinate = inst->src[TEX_LOGICAL_SRC_COORDINATE];
   const fs_reg shadow_c = inst->src[TEX_LOGICAL_SRC_SHADOW_C];
   const fs_reg lod = inst->src[TEX_LOGICAL_SRC_LOD];
   const fs_reg lod2 = inst->src[TEX_LOGICAL_SRC_LOD2];
   const fs_reg min_lod = inst->src[TEX_LOGICAL_SRC_MIN_LOD];
   const fs_reg sample_index = inst->src[TEX_LOGICAL_SRC_SAMPLE_INDEX];
   const fs_reg mcs = inst->src[TEX_LOGICAL_SRC_MCS];
   const fs_reg surface = inst->src[TEX_LOGICAL_SRC_SURFACE];
   const fs_reg sampler = inst->src[TEX_LOGICAL_SRC_SAMPLER];
   const fs_reg surface_handle = inst->src[TEX_LOGICAL_SRC_SURFACE_HANDLE];
   const fs_reg sampler_handle = inst->src[TEX_LOGICAL_SRC_SAMPLER_HANDLE];
   const fs_reg tg4_offset = inst->src[TEX_LOGICAL_SRC_TG4_OFFSET];
   assert(inst->src[TEX_LOGICAL_SRC_COORD_COMPONENTS].file == IMM);
   const unsigned coord_components = inst->src[TEX_LOGICAL_SRC_COORD_COMPONENTS].ud;
   assert(inst->src[TEX_LOGICAL_SRC_GRAD_COMPONENTS].file == IMM);
   const unsigned grad_components = inst->src[TEX_LOGICAL_SRC_GRAD_COMPONENTS].ud;
   assert(inst->src[TEX_LOGICAL_SRC_RESIDENCY].file == IMM);
   const bool residency = inst->src[TEX_LOGICAL_SRC_RESIDENCY].ud != 0;
   /* residency is only supported on Gfx8+ */
   assert(!residency || devinfo->ver >= 8);

   if (devinfo->ver >= 7) {
      const unsigned msg_payload_type_bit_size =
         get_sampler_msg_payload_type_bit_size(devinfo, op, inst->src);

      /* 16-bit payloads are available only on gfx11+ */
      assert(msg_payload_type_bit_size != 16 || devinfo->ver >= 11);

      lower_sampler_logical_send_gfx7(bld, inst, op, coordinate,
                                      shadow_c, lod, lod2, min_lod,
                                      sample_index,
                                      mcs, surface, sampler,
                                      surface_handle, sampler_handle,
                                      tg4_offset,
                                      msg_payload_type_bit_size,
                                      coord_components, grad_components,
                                      residency);
   } else if (devinfo->ver >= 5) {
      lower_sampler_logical_send_gfx5(bld, inst, op, coordinate,
                                      shadow_c, lod, lod2, sample_index,
                                      surface, sampler,
                                      coord_components, grad_components);
   } else {
      lower_sampler_logical_send_gfx4(bld, inst, op, coordinate,
                                      shadow_c, lod, lod2,
                                      surface, sampler,
                                      coord_components, grad_components);
   }
}

/**
 * Predicate the specified instruction on the vector mask.
 */
static void
emit_predicate_on_vector_mask(const fs_builder &bld, fs_inst *inst)
{
   assert(bld.shader->stage == MESA_SHADER_FRAGMENT &&
          bld.group() == inst->group &&
          bld.dispatch_width() == inst->exec_size);

   const fs_builder ubld = bld.exec_all().group(1, 0);

   const fs_visitor &s = *bld.shader;
   const fs_reg vector_mask = ubld.vgrf(BRW_REGISTER_TYPE_UW);
   ubld.UNDEF(vector_mask);
   ubld.emit(SHADER_OPCODE_READ_SR_REG, vector_mask, brw_imm_ud(3));
   const unsigned subreg = sample_mask_flag_subreg(s);

   ubld.MOV(brw_flag_subreg(subreg + inst->group / 16), vector_mask);

   if (inst->predicate) {
      assert(inst->predicate == BRW_PREDICATE_NORMAL);
      assert(!inst->predicate_inverse);
      assert(inst->flag_subreg == 0);
      /* Combine the vector mask with the existing predicate by using a
       * vertical predication mode.
       */
      inst->predicate = BRW_PREDICATE_ALIGN1_ALLV;
   } else {
      inst->flag_subreg = subreg;
      inst->predicate = BRW_PREDICATE_NORMAL;
      inst->predicate_inverse = false;
   }
}

static void
setup_surface_descriptors(const fs_builder &bld, fs_inst *inst, uint32_t desc,
                          const fs_reg &surface, const fs_reg &surface_handle)
{
   const ASSERTED intel_device_info *devinfo = bld.shader->devinfo;
   const brw_compiler *compiler = bld.shader->compiler;

   /* We must have exactly one of surface and surface_handle */
   assert((surface.file == BAD_FILE) != (surface_handle.file == BAD_FILE));

   if (surface.file == IMM) {
      inst->desc = desc | (surface.ud & 0xff);
      inst->src[0] = brw_imm_ud(0);
      inst->src[1] = brw_imm_ud(0); /* ex_desc */
   } else if (surface_handle.file != BAD_FILE) {
      /* Bindless surface */
      assert(devinfo->ver >= 9);
      inst->desc = desc | GFX9_BTI_BINDLESS;
      inst->src[0] = brw_imm_ud(0);

      /* We assume that the driver provided the handle in the top 20 bits so
       * we can use the surface handle directly as the extended descriptor.
       */
      inst->src[1] = retype(surface_handle, BRW_REGISTER_TYPE_UD);
      inst->send_ex_bso = compiler->extended_bindless_surface_offset;
   } else {
      inst->desc = desc;
      const fs_builder ubld = bld.exec_all().group(1, 0);
      fs_reg tmp = ubld.vgrf(BRW_REGISTER_TYPE_UD);
      ubld.AND(tmp, surface, brw_imm_ud(0xff));
      inst->src[0] = component(tmp, 0);
      inst->src[1] = brw_imm_ud(0); /* ex_desc */
   }
}

static void
setup_lsc_surface_descriptors(const fs_builder &bld, fs_inst *inst,
                              uint32_t desc, const fs_reg &surface)
{
   const ASSERTED intel_device_info *devinfo = bld.shader->devinfo;
   const brw_compiler *compiler = bld.shader->compiler;

   inst->src[0] = brw_imm_ud(0); /* desc */

   enum lsc_addr_surface_type surf_type = lsc_msg_desc_addr_type(devinfo, desc);
   switch (surf_type) {
   case LSC_ADDR_SURFTYPE_BSS:
      inst->send_ex_bso = compiler->extended_bindless_surface_offset;
      /* fall-through */
   case LSC_ADDR_SURFTYPE_SS:
      assert(surface.file != BAD_FILE);
      /* We assume that the driver provided the handle in the top 20 bits so
       * we can use the surface handle directly as the extended descriptor.
       */
      inst->src[1] = retype(surface, BRW_REGISTER_TYPE_UD);
      break;

   case LSC_ADDR_SURFTYPE_BTI:
      assert(surface.file != BAD_FILE);
      if (surface.file == IMM) {
         inst->src[1] = brw_imm_ud(lsc_bti_ex_desc(devinfo, surface.ud));
      } else {
         const fs_builder ubld = bld.exec_all().group(1, 0);
         fs_reg tmp = ubld.vgrf(BRW_REGISTER_TYPE_UD);
         ubld.SHL(tmp, surface, brw_imm_ud(24));
         inst->src[1] = component(tmp, 0);
      }
      break;

   case LSC_ADDR_SURFTYPE_FLAT:
      inst->src[1] = brw_imm_ud(0);
      break;

   default:
      unreachable("Invalid LSC surface address type");
   }
}

static void
lower_surface_logical_send(const fs_builder &bld, fs_inst *inst)
{
   const brw_compiler *compiler = bld.shader->compiler;
   const intel_device_info *devinfo = bld.shader->devinfo;

   /* Get the logical send arguments. */
   const fs_reg addr = inst->src[SURFACE_LOGICAL_SRC_ADDRESS];
   const fs_reg src = inst->src[SURFACE_LOGICAL_SRC_DATA];
   const fs_reg surface = inst->src[SURFACE_LOGICAL_SRC_SURFACE];
   const fs_reg surface_handle = inst->src[SURFACE_LOGICAL_SRC_SURFACE_HANDLE];
   const UNUSED fs_reg dims = inst->src[SURFACE_LOGICAL_SRC_IMM_DIMS];
   const fs_reg arg = inst->src[SURFACE_LOGICAL_SRC_IMM_ARG];
   const fs_reg allow_sample_mask =
      inst->src[SURFACE_LOGICAL_SRC_ALLOW_SAMPLE_MASK];
   assert(arg.file == IMM);
   assert(allow_sample_mask.file == IMM);

   /* Calculate the total number of components of the payload. */
   const unsigned addr_sz = inst->components_read(SURFACE_LOGICAL_SRC_ADDRESS);
   const unsigned src_sz = inst->components_read(SURFACE_LOGICAL_SRC_DATA);

   const bool is_typed_access =
      inst->opcode == SHADER_OPCODE_TYPED_SURFACE_READ_LOGICAL ||
      inst->opcode == SHADER_OPCODE_TYPED_SURFACE_WRITE_LOGICAL ||
      inst->opcode == SHADER_OPCODE_TYPED_ATOMIC_LOGICAL;

   const bool is_surface_access = is_typed_access ||
      inst->opcode == SHADER_OPCODE_UNTYPED_SURFACE_READ_LOGICAL ||
      inst->opcode == SHADER_OPCODE_UNTYPED_SURFACE_WRITE_LOGICAL ||
      inst->opcode == SHADER_OPCODE_UNTYPED_ATOMIC_LOGICAL;

   const bool is_stateless =
      surface.file == IMM && (surface.ud == BRW_BTI_STATELESS ||
                              surface.ud == GFX8_BTI_STATELESS_NON_COHERENT);

   const bool has_side_effects = inst->has_side_effects();

   fs_reg sample_mask = allow_sample_mask.ud ? brw_sample_mask_reg(bld) :
                                               fs_reg(brw_imm_d(0xffff));

   /* From the BDW PRM Volume 7, page 147:
    *
    *  "For the Data Cache Data Port*, the header must be present for the
    *   following message types: [...] Typed read/write/atomics"
    *
    * Earlier generations have a similar wording.  Because of this restriction
    * we don't attempt to implement sample masks via predication for such
    * messages prior to Gfx9, since we have to provide a header anyway.  On
    * Gfx11+ the header has been removed so we can only use predication.
    *
    * For all stateless A32 messages, we also need a header
    */
   fs_reg header;
   if ((devinfo->ver < 9 && is_typed_access) || is_stateless) {
      fs_builder ubld = bld.exec_all().group(8, 0);
      header = ubld.vgrf(BRW_REGISTER_TYPE_UD);
      if (is_stateless) {
         assert(!is_surface_access);
         ubld.emit(SHADER_OPCODE_SCRATCH_HEADER, header);
      } else {
         ubld.MOV(header, brw_imm_d(0));
         if (is_surface_access)
            ubld.group(1, 0).MOV(component(header, 7), sample_mask);
      }
   }
   const unsigned header_sz = header.file != BAD_FILE ? 1 : 0;

   fs_reg payload, payload2;
   unsigned mlen, ex_mlen = 0;
   if (devinfo->ver >= 9 &&
       (src.file == BAD_FILE || header.file == BAD_FILE)) {
      /* We have split sends on gfx9 and above */
      if (header.file == BAD_FILE) {
         payload = bld.move_to_vgrf(addr, addr_sz);
         payload2 = bld.move_to_vgrf(src, src_sz);
         mlen = addr_sz * (inst->exec_size / 8);
         ex_mlen = src_sz * (inst->exec_size / 8);
      } else {
         assert(src.file == BAD_FILE);
         payload = header;
         payload2 = bld.move_to_vgrf(addr, addr_sz);
         mlen = header_sz;
         ex_mlen = addr_sz * (inst->exec_size / 8);
      }
   } else {
      /* Allocate space for the payload. */
      const unsigned sz = header_sz + addr_sz + src_sz;
      payload = bld.vgrf(BRW_REGISTER_TYPE_UD, sz);
      fs_reg *const components = new fs_reg[sz];
      unsigned n = 0;

      /* Construct the payload. */
      if (header.file != BAD_FILE)
         components[n++] = header;

      for (unsigned i = 0; i < addr_sz; i++)
         components[n++] = offset(addr, bld, i);

      for (unsigned i = 0; i < src_sz; i++)
         components[n++] = offset(src, bld, i);

      bld.LOAD_PAYLOAD(payload, components, sz, header_sz);
      mlen = header_sz + (addr_sz + src_sz) * inst->exec_size / 8;

      delete[] components;
   }

   /* Predicate the instruction on the sample mask if no header is
    * provided.
    */
   if ((header.file == BAD_FILE || !is_surface_access) &&
       sample_mask.file != BAD_FILE && sample_mask.file != IMM)
      brw_emit_predicate_on_sample_mask(bld, inst);

   uint32_t sfid;
   switch (inst->opcode) {
   case SHADER_OPCODE_BYTE_SCATTERED_WRITE_LOGICAL:
   case SHADER_OPCODE_BYTE_SCATTERED_READ_LOGICAL:
      /* Byte scattered opcodes go through the normal data cache */
      sfid = GFX7_SFID_DATAPORT_DATA_CACHE;
      break;

   case SHADER_OPCODE_DWORD_SCATTERED_READ_LOGICAL:
   case SHADER_OPCODE_DWORD_SCATTERED_WRITE_LOGICAL:
      sfid =  devinfo->ver >= 7 ? GFX7_SFID_DATAPORT_DATA_CACHE :
              devinfo->ver >= 6 ? GFX6_SFID_DATAPORT_RENDER_CACHE :
                                  BRW_DATAPORT_READ_TARGET_RENDER_CACHE;
      break;

   case SHADER_OPCODE_UNTYPED_SURFACE_READ_LOGICAL:
   case SHADER_OPCODE_UNTYPED_SURFACE_WRITE_LOGICAL:
   case SHADER_OPCODE_UNTYPED_ATOMIC_LOGICAL:
      /* Untyped Surface messages go through the data cache but the SFID value
       * changed on Haswell.
       */
      sfid = (devinfo->verx10 >= 75 ?
              HSW_SFID_DATAPORT_DATA_CACHE_1 :
              GFX7_SFID_DATAPORT_DATA_CACHE);
      break;

   case SHADER_OPCODE_TYPED_SURFACE_READ_LOGICAL:
   case SHADER_OPCODE_TYPED_SURFACE_WRITE_LOGICAL:
   case SHADER_OPCODE_TYPED_ATOMIC_LOGICAL:
      /* Typed surface messages go through the render cache on IVB and the
       * data cache on HSW+.
       */
      sfid = (devinfo->verx10 >= 75 ?
              HSW_SFID_DATAPORT_DATA_CACHE_1 :
              GFX6_SFID_DATAPORT_RENDER_CACHE);
      break;

   default:
      unreachable("Unsupported surface opcode");
   }

   uint32_t desc;
   switch (inst->opcode) {
   case SHADER_OPCODE_UNTYPED_SURFACE_READ_LOGICAL:
      desc = brw_dp_untyped_surface_rw_desc(devinfo, inst->exec_size,
                                            arg.ud, /* num_channels */
                                            false   /* write */);
      break;

   case SHADER_OPCODE_UNTYPED_SURFACE_WRITE_LOGICAL:
      desc = brw_dp_untyped_surface_rw_desc(devinfo, inst->exec_size,
                                            arg.ud, /* num_channels */
                                            true    /* write */);
      break;

   case SHADER_OPCODE_BYTE_SCATTERED_READ_LOGICAL:
      desc = brw_dp_byte_scattered_rw_desc(devinfo, inst->exec_size,
                                           arg.ud, /* bit_size */
                                           false   /* write */);
      break;

   case SHADER_OPCODE_BYTE_SCATTERED_WRITE_LOGICAL:
      desc = brw_dp_byte_scattered_rw_desc(devinfo, inst->exec_size,
                                           arg.ud, /* bit_size */
                                           true    /* write */);
      break;

   case SHADER_OPCODE_DWORD_SCATTERED_READ_LOGICAL:
      assert(arg.ud == 32); /* bit_size */
      desc = brw_dp_dword_scattered_rw_desc(devinfo, inst->exec_size,
                                            false  /* write */);
      break;

   case SHADER_OPCODE_DWORD_SCATTERED_WRITE_LOGICAL:
      assert(arg.ud == 32); /* bit_size */
      desc = brw_dp_dword_scattered_rw_desc(devinfo, inst->exec_size,
                                            true   /* write */);
      break;

   case SHADER_OPCODE_UNTYPED_ATOMIC_LOGICAL:
      if (lsc_opcode_is_atomic_float((enum lsc_opcode) arg.ud)) {
         desc = brw_dp_untyped_atomic_float_desc(devinfo, inst->exec_size,
                                                 lsc_op_to_legacy_atomic(arg.ud),
                                                 !inst->dst.is_null());
      } else {
         desc = brw_dp_untyped_atomic_desc(devinfo, inst->exec_size,
                                           lsc_op_to_legacy_atomic(arg.ud),
                                           !inst->dst.is_null());
      }
      break;

   case SHADER_OPCODE_TYPED_SURFACE_READ_LOGICAL:
      desc = brw_dp_typed_surface_rw_desc(devinfo, inst->exec_size, inst->group,
                                          arg.ud, /* num_channels */
                                          false   /* write */);
      break;

   case SHADER_OPCODE_TYPED_SURFACE_WRITE_LOGICAL:
      desc = brw_dp_typed_surface_rw_desc(devinfo, inst->exec_size, inst->group,
                                          arg.ud, /* num_channels */
                                          true    /* write */);
      break;

   case SHADER_OPCODE_TYPED_ATOMIC_LOGICAL:
      desc = brw_dp_typed_atomic_desc(devinfo, inst->exec_size, inst->group,
                                      lsc_op_to_legacy_atomic(arg.ud),
                                      !inst->dst.is_null());
      break;

   default:
      unreachable("Unknown surface logical instruction");
   }

   /* Update the original instruction. */
   inst->opcode = SHADER_OPCODE_SEND;
   inst->mlen = mlen;
   inst->ex_mlen = ex_mlen;
   inst->header_size = header_sz;
   inst->send_has_side_effects = has_side_effects;
   inst->send_is_volatile = !has_side_effects;
   inst->send_ex_bso = surface_handle.file != BAD_FILE &&
                       compiler->extended_bindless_surface_offset;

   /* Set up SFID and descriptors */
   inst->sfid = sfid;
   setup_surface_descriptors(bld, inst, desc, surface, surface_handle);

   inst->resize_sources(4);

   /* Finally, the payload */
   inst->src[2] = payload;
   inst->src[3] = payload2;
}

static enum lsc_data_size
lsc_bits_to_data_size(unsigned bit_size)
{
   switch (bit_size / 8) {
   case 1:  return LSC_DATA_SIZE_D8U32;
   case 2:  return LSC_DATA_SIZE_D16U32;
   case 4:  return LSC_DATA_SIZE_D32;
   case 8:  return LSC_DATA_SIZE_D64;
   default:
      unreachable("Unsupported data size.");
   }
}

static void
lower_lsc_surface_logical_send(const fs_builder &bld, fs_inst *inst)
{
   const brw_compiler *compiler = bld.shader->compiler;
   const intel_device_info *devinfo = bld.shader->devinfo;
   assert(devinfo->has_lsc);

   /* Get the logical send arguments. */
   const fs_reg addr = inst->src[SURFACE_LOGICAL_SRC_ADDRESS];
   const fs_reg src = inst->src[SURFACE_LOGICAL_SRC_DATA];
   const fs_reg surface = inst->src[SURFACE_LOGICAL_SRC_SURFACE];
   const fs_reg surface_handle = inst->src[SURFACE_LOGICAL_SRC_SURFACE_HANDLE];
   const UNUSED fs_reg dims = inst->src[SURFACE_LOGICAL_SRC_IMM_DIMS];
   const fs_reg arg = inst->src[SURFACE_LOGICAL_SRC_IMM_ARG];
   const fs_reg allow_sample_mask =
      inst->src[SURFACE_LOGICAL_SRC_ALLOW_SAMPLE_MASK];
   assert(arg.file == IMM);
   assert(allow_sample_mask.file == IMM);

   /* Calculate the total number of components of the payload. */
   const unsigned addr_sz = inst->components_read(SURFACE_LOGICAL_SRC_ADDRESS);
   const unsigned src_comps = inst->components_read(SURFACE_LOGICAL_SRC_DATA);
   const unsigned src_sz = type_sz(src.type);
   const unsigned dst_sz = type_sz(inst->dst.type);

   const bool has_side_effects = inst->has_side_effects();

   unsigned ex_mlen = 0;
   fs_reg payload, payload2;
   payload = bld.move_to_vgrf(addr, addr_sz);
   if (src.file != BAD_FILE) {
      payload2 = bld.move_to_vgrf(src, src_comps);
      ex_mlen = (src_comps * src_sz * inst->exec_size) / REG_SIZE;
   }

   /* Predicate the instruction on the sample mask if needed */
   fs_reg sample_mask = allow_sample_mask.ud ? brw_sample_mask_reg(bld) :
                                               fs_reg(brw_imm_d(0xffff));
   if (sample_mask.file != BAD_FILE && sample_mask.file != IMM)
      brw_emit_predicate_on_sample_mask(bld, inst);

   if (surface.file == IMM && surface.ud == GFX7_BTI_SLM)
      inst->sfid = GFX12_SFID_SLM;
   else
      inst->sfid = GFX12_SFID_UGM;

   /* We should have exactly one of surface and surface_handle. For scratch
    * messages generated by brw_fs_nir.cpp we also allow a special value to
    * know what heap base we should use in STATE_BASE_ADDRESS (SS = Surface
    * State Offset, or BSS = Bindless Surface State Offset).
    */
   bool non_bindless = surface.file == IMM && surface.ud == GFX125_NON_BINDLESS;
   assert((surface.file == BAD_FILE) != (surface_handle.file == BAD_FILE) ||
          (non_bindless && surface_handle.file != BAD_FILE));

   enum lsc_addr_surface_type surf_type;
   if (surface_handle.file != BAD_FILE) {
      if (surface.file == BAD_FILE) {
         assert(!non_bindless);
         surf_type = LSC_ADDR_SURFTYPE_BSS;
      } else {
         assert(surface.file == IMM &&
                (surface.ud == 0 || surface.ud == GFX125_NON_BINDLESS));
         surf_type = non_bindless ? LSC_ADDR_SURFTYPE_SS : LSC_ADDR_SURFTYPE_BSS;
      }
   } else if (surface.file == IMM && surface.ud == GFX7_BTI_SLM)
      surf_type = LSC_ADDR_SURFTYPE_FLAT;
   else
      surf_type = LSC_ADDR_SURFTYPE_BTI;

   switch (inst->opcode) {
   case SHADER_OPCODE_UNTYPED_SURFACE_READ_LOGICAL:
      inst->desc = lsc_msg_desc(devinfo, LSC_OP_LOAD_CMASK, inst->exec_size,
                                surf_type, LSC_ADDR_SIZE_A32,
                                1 /* num_coordinates */,
                                LSC_DATA_SIZE_D32, arg.ud /* num_channels */,
                                false /* transpose */,
                                LSC_CACHE(devinfo, LOAD, L1STATE_L3MOCS),
                                true /* has_dest */);
      break;
   case SHADER_OPCODE_UNTYPED_SURFACE_WRITE_LOGICAL:
      inst->desc = lsc_msg_desc(devinfo, LSC_OP_STORE_CMASK, inst->exec_size,
                                surf_type, LSC_ADDR_SIZE_A32,
                                1 /* num_coordinates */,
                                LSC_DATA_SIZE_D32, arg.ud /* num_channels */,
                                false /* transpose */,
                                LSC_CACHE(devinfo, STORE, L1STATE_L3MOCS),
                                false /* has_dest */);
      break;
   case SHADER_OPCODE_UNTYPED_ATOMIC_LOGICAL: {
      /* Bspec: Atomic instruction -> Cache section:
       *
       *    Atomic messages are always forced to "un-cacheable" in the L1
       *    cache.
       */
      enum lsc_opcode opcode = (enum lsc_opcode) arg.ud;

      inst->desc = lsc_msg_desc(devinfo, opcode, inst->exec_size,
                                surf_type, LSC_ADDR_SIZE_A32,
                                1 /* num_coordinates */,
                                lsc_bits_to_data_size(dst_sz * 8),
                                1 /* num_channels */,
                                false /* transpose */,
                                LSC_CACHE(devinfo, STORE, L1UC_L3WB),
                                !inst->dst.is_null());
      break;
   }
   case SHADER_OPCODE_BYTE_SCATTERED_READ_LOGICAL:
      inst->desc = lsc_msg_desc(devinfo, LSC_OP_LOAD, inst->exec_size,
                                surf_type, LSC_ADDR_SIZE_A32,
                                1 /* num_coordinates */,
                                lsc_bits_to_data_size(arg.ud),
                                1 /* num_channels */,
                                false /* transpose */,
                                LSC_CACHE(devinfo, LOAD, L1STATE_L3MOCS),
                                true /* has_dest */);
      break;
   case SHADER_OPCODE_BYTE_SCATTERED_WRITE_LOGICAL:
      inst->desc = lsc_msg_desc(devinfo, LSC_OP_STORE, inst->exec_size,
                                surf_type, LSC_ADDR_SIZE_A32,
                                1 /* num_coordinates */,
                                lsc_bits_to_data_size(arg.ud),
                                1 /* num_channels */,
                                false /* transpose */,
                                LSC_CACHE(devinfo, STORE, L1STATE_L3MOCS),
                                false /* has_dest */);
      break;
   default:
      unreachable("Unknown surface logical instruction");
   }

   /* Update the original instruction. */
   inst->opcode = SHADER_OPCODE_SEND;
   inst->mlen = lsc_msg_desc_src0_len(devinfo, inst->desc);
   inst->ex_mlen = ex_mlen;
   inst->header_size = 0;
   inst->send_has_side_effects = has_side_effects;
   inst->send_is_volatile = !has_side_effects;
   inst->send_ex_bso = surf_type == LSC_ADDR_SURFTYPE_BSS &&
                       compiler->extended_bindless_surface_offset;

   inst->resize_sources(4);

   if (non_bindless) {
      inst->src[0] = brw_imm_ud(0);     /* desc */
      inst->src[1] = surface_handle;    /* ex_desc */
   } else {
      setup_lsc_surface_descriptors(bld, inst, inst->desc,
                                    surface.file != BAD_FILE ?
                                    surface : surface_handle);
   }

   /* Finally, the payload */
   inst->src[2] = payload;
   inst->src[3] = payload2;
}

static void
lower_lsc_block_logical_send(const fs_builder &bld, fs_inst *inst)
{
   const brw_compiler *compiler = bld.shader->compiler;
   const intel_device_info *devinfo = bld.shader->devinfo;
   assert(devinfo->has_lsc);

   /* Get the logical send arguments. */
   const fs_reg addr = inst->src[SURFACE_LOGICAL_SRC_ADDRESS];
   const fs_reg src = inst->src[SURFACE_LOGICAL_SRC_DATA];
   const fs_reg surface = inst->src[SURFACE_LOGICAL_SRC_SURFACE];
   const fs_reg surface_handle = inst->src[SURFACE_LOGICAL_SRC_SURFACE_HANDLE];
   const fs_reg arg = inst->src[SURFACE_LOGICAL_SRC_IMM_ARG];
   assert(arg.file == IMM);
   assert(inst->src[SURFACE_LOGICAL_SRC_IMM_DIMS].file == BAD_FILE);
   assert(inst->src[SURFACE_LOGICAL_SRC_ALLOW_SAMPLE_MASK].file == BAD_FILE);

   const bool is_stateless =
      surface.file == IMM && (surface.ud == BRW_BTI_STATELESS ||
                              surface.ud == GFX8_BTI_STATELESS_NON_COHERENT);

   const bool has_side_effects = inst->has_side_effects();

   const bool write = inst->opcode == SHADER_OPCODE_OWORD_BLOCK_WRITE_LOGICAL;

   fs_builder ubld = bld.exec_all().group(1, 0);
   fs_reg stateless_ex_desc;
   if (is_stateless) {
      stateless_ex_desc = ubld.vgrf(BRW_REGISTER_TYPE_UD);
      ubld.AND(stateless_ex_desc,
               retype(brw_vec1_grf(0, 5), BRW_REGISTER_TYPE_UD),
               brw_imm_ud(INTEL_MASK(31, 10)));
   }

   fs_reg data;
   if (write) {
      const unsigned src_sz = inst->components_read(SURFACE_LOGICAL_SRC_DATA);
      data = retype(bld.move_to_vgrf(src, src_sz), BRW_REGISTER_TYPE_UD);
   }

   inst->opcode = SHADER_OPCODE_SEND;
   if (surface.file == IMM && surface.ud == GFX7_BTI_SLM)
      inst->sfid = GFX12_SFID_SLM;
   else
      inst->sfid = GFX12_SFID_UGM;
   const enum lsc_addr_surface_type surf_type =
      inst->sfid == GFX12_SFID_SLM ?
      LSC_ADDR_SURFTYPE_FLAT :
      surface.file == BAD_FILE ?
      LSC_ADDR_SURFTYPE_BSS : LSC_ADDR_SURFTYPE_BTI;
   inst->desc = lsc_msg_desc(devinfo,
                             write ? LSC_OP_STORE : LSC_OP_LOAD,
                             1 /* exec_size */,
                             surf_type,
                             LSC_ADDR_SIZE_A32,
                             1 /* num_coordinates */,
                             LSC_DATA_SIZE_D32,
                             arg.ud /* num_channels */,
                             true /* transpose */,
                             LSC_CACHE(devinfo, LOAD, L1STATE_L3MOCS),
                             !write /* has_dest */);

   inst->mlen = lsc_msg_desc_src0_len(devinfo, inst->desc);
   inst->size_written = lsc_msg_desc_dest_len(devinfo, inst->desc) * REG_SIZE;
   inst->exec_size = 1;
   inst->ex_mlen = write ? DIV_ROUND_UP(arg.ud, 8) : 0;
   inst->header_size = 0;
   inst->send_has_side_effects = has_side_effects;
   inst->send_is_volatile = !has_side_effects;
   inst->send_ex_bso = surf_type == LSC_ADDR_SURFTYPE_BSS &&
                       compiler->extended_bindless_surface_offset;

   inst->resize_sources(4);

   if (stateless_ex_desc.file != BAD_FILE) {
      inst->src[0] = brw_imm_ud(0);     /* desc */
      inst->src[1] = stateless_ex_desc; /* ex_desc */
   } else {
      setup_lsc_surface_descriptors(bld, inst, inst->desc,
                                    surface.file != BAD_FILE ?
                                    surface : surface_handle);
   }
   inst->src[2] = addr;          /* payload */
   inst->src[3] = data;          /* payload2 */
}

static void
lower_surface_block_logical_send(const fs_builder &bld, fs_inst *inst)
{
   const intel_device_info *devinfo = bld.shader->devinfo;
   assert(devinfo->ver >= 9);

   /* Get the logical send arguments. */
   const fs_reg addr = inst->src[SURFACE_LOGICAL_SRC_ADDRESS];
   const fs_reg src = inst->src[SURFACE_LOGICAL_SRC_DATA];
   const fs_reg surface = inst->src[SURFACE_LOGICAL_SRC_SURFACE];
   const fs_reg surface_handle = inst->src[SURFACE_LOGICAL_SRC_SURFACE_HANDLE];
   const fs_reg arg = inst->src[SURFACE_LOGICAL_SRC_IMM_ARG];
   assert(arg.file == IMM);
   assert(inst->src[SURFACE_LOGICAL_SRC_IMM_DIMS].file == BAD_FILE);
   assert(inst->src[SURFACE_LOGICAL_SRC_ALLOW_SAMPLE_MASK].file == BAD_FILE);

   const bool is_stateless =
      surface.file == IMM && (surface.ud == BRW_BTI_STATELESS ||
                              surface.ud == GFX8_BTI_STATELESS_NON_COHERENT);

   const bool has_side_effects = inst->has_side_effects();

   const bool align_16B =
      inst->opcode != SHADER_OPCODE_UNALIGNED_OWORD_BLOCK_READ_LOGICAL;

   const bool write = inst->opcode == SHADER_OPCODE_OWORD_BLOCK_WRITE_LOGICAL;

   /* The address is stored in the header.  See MH_A32_GO and MH_BTS_GO. */
   fs_builder ubld = bld.exec_all().group(8, 0);
   fs_reg header = ubld.vgrf(BRW_REGISTER_TYPE_UD);

   if (is_stateless)
      ubld.emit(SHADER_OPCODE_SCRATCH_HEADER, header);
   else
      ubld.MOV(header, brw_imm_d(0));

   /* Address in OWord units when aligned to OWords. */
   if (align_16B)
      ubld.group(1, 0).SHR(component(header, 2), addr, brw_imm_ud(4));
   else
      ubld.group(1, 0).MOV(component(header, 2), addr);

   fs_reg data;
   unsigned ex_mlen = 0;
   if (write) {
      const unsigned src_sz = inst->components_read(SURFACE_LOGICAL_SRC_DATA);
      data = retype(bld.move_to_vgrf(src, src_sz), BRW_REGISTER_TYPE_UD);
      ex_mlen = src_sz * type_sz(src.type) * inst->exec_size / REG_SIZE;
   }

   inst->opcode = SHADER_OPCODE_SEND;
   inst->mlen = 1;
   inst->ex_mlen = ex_mlen;
   inst->header_size = 1;
   inst->send_has_side_effects = has_side_effects;
   inst->send_is_volatile = !has_side_effects;

   inst->sfid = GFX7_SFID_DATAPORT_DATA_CACHE;

   const uint32_t desc = brw_dp_oword_block_rw_desc(devinfo, align_16B,
                                                    arg.ud, write);
   setup_surface_descriptors(bld, inst, desc, surface, surface_handle);

   inst->resize_sources(4);

   inst->src[2] = header;
   inst->src[3] = data;
}

static fs_reg
emit_a64_oword_block_header(const fs_builder &bld, const fs_reg &addr)
{
   const fs_builder ubld = bld.exec_all().group(8, 0);

   assert(type_sz(addr.type) == 8 && addr.stride == 0);

   fs_reg expanded_addr = addr;
   if (addr.file == UNIFORM) {
      /* We can't do stride 1 with the UNIFORM file, it requires stride 0 */
      expanded_addr = ubld.vgrf(BRW_REGISTER_TYPE_UQ);
      expanded_addr.stride = 0;
      ubld.MOV(expanded_addr, retype(addr, BRW_REGISTER_TYPE_UQ));
   }

   fs_reg header = ubld.vgrf(BRW_REGISTER_TYPE_UD);
   ubld.MOV(header, brw_imm_ud(0));

   /* Use a 2-wide MOV to fill out the address */
   fs_reg addr_vec2 = expanded_addr;
   addr_vec2.type = BRW_REGISTER_TYPE_UD;
   addr_vec2.stride = 1;
   ubld.group(2, 0).MOV(header, addr_vec2);

   return header;
}

static void
emit_fragment_mask(const fs_builder &bld, fs_inst *inst)
{
   assert(inst->src[A64_LOGICAL_ENABLE_HELPERS].file == IMM);
   const bool enable_helpers = inst->src[A64_LOGICAL_ENABLE_HELPERS].ud;

   /* If we're a fragment shader, we have to predicate with the sample mask to
    * avoid helper invocations to avoid helper invocations in instructions
    * with side effects, unless they are explicitly required.
    *
    * There are also special cases when we actually want to run on helpers
    * (ray queries).
    */
   assert(bld.shader->stage == MESA_SHADER_FRAGMENT);
   if (enable_helpers)
      emit_predicate_on_vector_mask(bld, inst);
   else if (inst->has_side_effects())
      brw_emit_predicate_on_sample_mask(bld, inst);
}

static void
lower_lsc_a64_logical_send(const fs_builder &bld, fs_inst *inst)
{
   const intel_device_info *devinfo = bld.shader->devinfo;

   /* Get the logical send arguments. */
   const fs_reg addr = inst->src[A64_LOGICAL_ADDRESS];
   const fs_reg src = inst->src[A64_LOGICAL_SRC];
   const unsigned src_sz = type_sz(src.type);
   const unsigned dst_sz = type_sz(inst->dst.type);

   const unsigned src_comps = inst->components_read(1);
   assert(inst->src[A64_LOGICAL_ARG].file == IMM);
   const unsigned arg = inst->src[A64_LOGICAL_ARG].ud;
   const bool has_side_effects = inst->has_side_effects();

   fs_reg payload = retype(bld.move_to_vgrf(addr, 1), BRW_REGISTER_TYPE_UD);
   fs_reg payload2 = retype(bld.move_to_vgrf(src, src_comps),
                            BRW_REGISTER_TYPE_UD);
   unsigned ex_mlen = src_comps * src_sz * inst->exec_size / REG_SIZE;

   switch (inst->opcode) {
   case SHADER_OPCODE_A64_UNTYPED_READ_LOGICAL:
      inst->desc = lsc_msg_desc(devinfo, LSC_OP_LOAD_CMASK, inst->exec_size,
                                LSC_ADDR_SURFTYPE_FLAT, LSC_ADDR_SIZE_A64,
                                1 /* num_coordinates */,
                                LSC_DATA_SIZE_D32, arg /* num_channels */,
                                false /* transpose */,
                                LSC_CACHE(devinfo, LOAD, L1STATE_L3MOCS),
                                true /* has_dest */);
      break;
   case SHADER_OPCODE_A64_UNTYPED_WRITE_LOGICAL:
      inst->desc = lsc_msg_desc(devinfo, LSC_OP_STORE_CMASK, inst->exec_size,
                                LSC_ADDR_SURFTYPE_FLAT, LSC_ADDR_SIZE_A64,
                                1 /* num_coordinates */,
                                LSC_DATA_SIZE_D32, arg /* num_channels */,
                                false /* transpose */,
                                LSC_CACHE(devinfo, STORE, L1STATE_L3MOCS),
                                false /* has_dest */);
      break;
   case SHADER_OPCODE_A64_BYTE_SCATTERED_READ_LOGICAL:
      inst->desc = lsc_msg_desc(devinfo, LSC_OP_LOAD, inst->exec_size,
                                LSC_ADDR_SURFTYPE_FLAT, LSC_ADDR_SIZE_A64,
                                1 /* num_coordinates */,
                                lsc_bits_to_data_size(arg),
                                1 /* num_channels */,
                                false /* transpose */,
                                LSC_CACHE(devinfo, LOAD, L1STATE_L3MOCS),
                                true /* has_dest */);
      break;
   case SHADER_OPCODE_A64_BYTE_SCATTERED_WRITE_LOGICAL:
      inst->desc = lsc_msg_desc(devinfo, LSC_OP_STORE, inst->exec_size,
                                LSC_ADDR_SURFTYPE_FLAT, LSC_ADDR_SIZE_A64,
                                1 /* num_coordinates */,
                                lsc_bits_to_data_size(arg),
                                1 /* num_channels */,
                                false /* transpose */,
                                LSC_CACHE(devinfo, STORE, L1STATE_L3MOCS),
                                false /* has_dest */);
      break;
   case SHADER_OPCODE_A64_UNTYPED_ATOMIC_LOGICAL: {
      /* Bspec: Atomic instruction -> Cache section:
       *
       *    Atomic messages are always forced to "un-cacheable" in the L1
       *    cache.
       */
      enum lsc_opcode opcode = (enum lsc_opcode) arg;
      inst->desc = lsc_msg_desc(devinfo, opcode, inst->exec_size,
                                LSC_ADDR_SURFTYPE_FLAT, LSC_ADDR_SIZE_A64,
                                1 /* num_coordinates */,
                                lsc_bits_to_data_size(dst_sz * 8),
                                1 /* num_channels */,
                                false /* transpose */,
                                LSC_CACHE(devinfo, STORE, L1UC_L3WB),
                                !inst->dst.is_null());
      break;
   }
   case SHADER_OPCODE_A64_OWORD_BLOCK_READ_LOGICAL:
   case SHADER_OPCODE_A64_UNALIGNED_OWORD_BLOCK_READ_LOGICAL:
      inst->exec_size = 1;
      inst->desc = lsc_msg_desc(devinfo,
                                LSC_OP_LOAD,
                                1 /* exec_size */,
                                LSC_ADDR_SURFTYPE_FLAT,
                                LSC_ADDR_SIZE_A64,
                                1 /* num_coordinates */,
                                LSC_DATA_SIZE_D32,
                                arg /* num_channels */,
                                true /* transpose */,
                                LSC_CACHE(devinfo, LOAD, L1STATE_L3MOCS),
                                true /* has_dest */);
      break;
   case SHADER_OPCODE_A64_OWORD_BLOCK_WRITE_LOGICAL:
      inst->exec_size = 1;
      inst->desc = lsc_msg_desc(devinfo,
                                LSC_OP_STORE,
                                1 /* exec_size */,
                                LSC_ADDR_SURFTYPE_FLAT,
                                LSC_ADDR_SIZE_A64,
                                1 /* num_coordinates */,
                                LSC_DATA_SIZE_D32,
                                arg /* num_channels */,
                                true /* transpose */,
                                LSC_CACHE(devinfo, LOAD, L1STATE_L3MOCS),
                                false /* has_dest */);

      break;
   default:
      unreachable("Unknown A64 logical instruction");
   }

   if (bld.shader->stage == MESA_SHADER_FRAGMENT)
      emit_fragment_mask(bld, inst);

   /* Update the original instruction. */
   inst->opcode = SHADER_OPCODE_SEND;
   inst->mlen = lsc_msg_desc_src0_len(devinfo, inst->desc);
   inst->ex_mlen = ex_mlen;
   inst->header_size = 0;
   inst->send_has_side_effects = has_side_effects;
   inst->send_is_volatile = !has_side_effects;

   /* Set up SFID and descriptors */
   inst->sfid = GFX12_SFID_UGM;
   inst->resize_sources(4);
   inst->src[0] = brw_imm_ud(0); /* desc */
   inst->src[1] = brw_imm_ud(0); /* ex_desc */
   inst->src[2] = payload;
   inst->src[3] = payload2;
}

static void
lower_a64_logical_send(const fs_builder &bld, fs_inst *inst)
{
   const intel_device_info *devinfo = bld.shader->devinfo;

   const fs_reg addr = inst->src[A64_LOGICAL_ADDRESS];
   const fs_reg src = inst->src[A64_LOGICAL_SRC];
   const unsigned src_comps = inst->components_read(1);
   assert(inst->src[A64_LOGICAL_ARG].file == IMM);
   const unsigned arg = inst->src[A64_LOGICAL_ARG].ud;
   const bool has_side_effects = inst->has_side_effects();

   fs_reg payload, payload2;
   unsigned mlen, ex_mlen = 0, header_size = 0;
   if (inst->opcode == SHADER_OPCODE_A64_OWORD_BLOCK_READ_LOGICAL ||
       inst->opcode == SHADER_OPCODE_A64_OWORD_BLOCK_WRITE_LOGICAL ||
       inst->opcode == SHADER_OPCODE_A64_UNALIGNED_OWORD_BLOCK_READ_LOGICAL) {
      assert(devinfo->ver >= 9);

      /* OWORD messages only take a scalar address in a header */
      mlen = 1;
      header_size = 1;
      payload = emit_a64_oword_block_header(bld, addr);

      if (inst->opcode == SHADER_OPCODE_A64_OWORD_BLOCK_WRITE_LOGICAL) {
         ex_mlen = src_comps * type_sz(src.type) * inst->exec_size / REG_SIZE;
         payload2 = retype(bld.move_to_vgrf(src, src_comps),
                           BRW_REGISTER_TYPE_UD);
      }
   } else if (devinfo->ver >= 9) {
      /* On Skylake and above, we have SENDS */
      mlen = 2 * (inst->exec_size / 8);
      ex_mlen = src_comps * type_sz(src.type) * inst->exec_size / REG_SIZE;
      payload = retype(bld.move_to_vgrf(addr, 1), BRW_REGISTER_TYPE_UD);
      payload2 = retype(bld.move_to_vgrf(src, src_comps),
                        BRW_REGISTER_TYPE_UD);
   } else {
      /* Add two because the address is 64-bit */
      const unsigned dwords = 2 + src_comps;
      mlen = dwords * (inst->exec_size / 8);

      fs_reg sources[5];

      sources[0] = addr;

      for (unsigned i = 0; i < src_comps; i++)
         sources[1 + i] = offset(src, bld, i);

      payload = bld.vgrf(BRW_REGISTER_TYPE_UD, dwords);
      bld.LOAD_PAYLOAD(payload, sources, 1 + src_comps, 0);
   }

   uint32_t desc;
   switch (inst->opcode) {
   case SHADER_OPCODE_A64_UNTYPED_READ_LOGICAL:
      desc = brw_dp_a64_untyped_surface_rw_desc(devinfo, inst->exec_size,
                                                arg,   /* num_channels */
                                                false  /* write */);
      break;

   case SHADER_OPCODE_A64_UNTYPED_WRITE_LOGICAL:
      desc = brw_dp_a64_untyped_surface_rw_desc(devinfo, inst->exec_size,
                                                arg,   /* num_channels */
                                                true   /* write */);
      break;

   case SHADER_OPCODE_A64_OWORD_BLOCK_READ_LOGICAL:
      desc = brw_dp_a64_oword_block_rw_desc(devinfo,
                                            true,    /* align_16B */
                                            arg,     /* num_dwords */
                                            false    /* write */);
      break;

   case SHADER_OPCODE_A64_UNALIGNED_OWORD_BLOCK_READ_LOGICAL:
      desc = brw_dp_a64_oword_block_rw_desc(devinfo,
                                            false,   /* align_16B */
                                            arg,     /* num_dwords */
                                            false    /* write */);
      break;

   case SHADER_OPCODE_A64_OWORD_BLOCK_WRITE_LOGICAL:
      desc = brw_dp_a64_oword_block_rw_desc(devinfo,
                                            true,    /* align_16B */
                                            arg,     /* num_dwords */
                                            true     /* write */);
      break;

   case SHADER_OPCODE_A64_BYTE_SCATTERED_READ_LOGICAL:
      desc = brw_dp_a64_byte_scattered_rw_desc(devinfo, inst->exec_size,
                                               arg,   /* bit_size */
                                               false  /* write */);
      break;

   case SHADER_OPCODE_A64_BYTE_SCATTERED_WRITE_LOGICAL:
      desc = brw_dp_a64_byte_scattered_rw_desc(devinfo, inst->exec_size,
                                               arg,   /* bit_size */
                                               true   /* write */);
      break;

   case SHADER_OPCODE_A64_UNTYPED_ATOMIC_LOGICAL:
      if (lsc_opcode_is_atomic_float((enum lsc_opcode) arg)) {
         desc =
            brw_dp_a64_untyped_atomic_float_desc(devinfo, inst->exec_size,
                                                 type_sz(inst->dst.type) * 8,
                                                 lsc_op_to_legacy_atomic(arg),
                                                 !inst->dst.is_null());
      } else {
         desc = brw_dp_a64_untyped_atomic_desc(devinfo, inst->exec_size,
                                               type_sz(inst->dst.type) * 8,
                                               lsc_op_to_legacy_atomic(arg),
                                               !inst->dst.is_null());
      }
      break;

   default:
      unreachable("Unknown A64 logical instruction");
   }

   if (bld.shader->stage == MESA_SHADER_FRAGMENT)
      emit_fragment_mask(bld, inst);

   /* Update the original instruction. */
   inst->opcode = SHADER_OPCODE_SEND;
   inst->mlen = mlen;
   inst->ex_mlen = ex_mlen;
   inst->header_size = header_size;
   inst->send_has_side_effects = has_side_effects;
   inst->send_is_volatile = !has_side_effects;

   /* Set up SFID and descriptors */
   inst->sfid = HSW_SFID_DATAPORT_DATA_CACHE_1;
   inst->desc = desc;
   inst->resize_sources(4);
   inst->src[0] = brw_imm_ud(0); /* desc */
   inst->src[1] = brw_imm_ud(0); /* ex_desc */
   inst->src[2] = payload;
   inst->src[3] = payload2;
}

static void
lower_lsc_varying_pull_constant_logical_send(const fs_builder &bld,
                                             fs_inst *inst)
{
   const intel_device_info *devinfo = bld.shader->devinfo;
   ASSERTED const brw_compiler *compiler = bld.shader->compiler;

   fs_reg surface        = inst->src[PULL_VARYING_CONSTANT_SRC_SURFACE];
   fs_reg surface_handle = inst->src[PULL_VARYING_CONSTANT_SRC_SURFACE_HANDLE];
   fs_reg offset_B       = inst->src[PULL_VARYING_CONSTANT_SRC_OFFSET];
   fs_reg alignment_B    = inst->src[PULL_VARYING_CONSTANT_SRC_ALIGNMENT];

   /* We are switching the instruction from an ALU-like instruction to a
    * send-from-grf instruction.  Since sends can't handle strides or
    * source modifiers, we have to make a copy of the offset source.
    */
   fs_reg ubo_offset = bld.move_to_vgrf(offset_B, 1);

   enum lsc_addr_surface_type surf_type =
      surface_handle.file == BAD_FILE ?
      LSC_ADDR_SURFTYPE_BTI : LSC_ADDR_SURFTYPE_BSS;

   assert(alignment_B.file == BRW_IMMEDIATE_VALUE);
   unsigned alignment = alignment_B.ud;

   inst->opcode = SHADER_OPCODE_SEND;
   inst->sfid = GFX12_SFID_UGM;
   inst->resize_sources(3);
   inst->send_ex_bso = surf_type == LSC_ADDR_SURFTYPE_BSS &&
                       compiler->extended_bindless_surface_offset;

   assert(!compiler->indirect_ubos_use_sampler);

   inst->src[0] = brw_imm_ud(0);
   inst->src[2] = ubo_offset; /* payload */

   if (alignment >= 4) {
      inst->desc =
         lsc_msg_desc(devinfo, LSC_OP_LOAD_CMASK, inst->exec_size,
                      surf_type, LSC_ADDR_SIZE_A32,
                      1 /* num_coordinates */,
                      LSC_DATA_SIZE_D32,
                      4 /* num_channels */,
                      false /* transpose */,
                      LSC_CACHE(devinfo, LOAD, L1STATE_L3MOCS),
                      true /* has_dest */);
      inst->mlen = lsc_msg_desc_src0_len(devinfo, inst->desc);

      setup_lsc_surface_descriptors(bld, inst, inst->desc,
                                    surface.file != BAD_FILE ?
                                    surface : surface_handle);
   } else {
      inst->desc =
         lsc_msg_desc(devinfo, LSC_OP_LOAD, inst->exec_size,
                      surf_type, LSC_ADDR_SIZE_A32,
                      1 /* num_coordinates */,
                      LSC_DATA_SIZE_D32,
                      1 /* num_channels */,
                      false /* transpose */,
                      LSC_CACHE(devinfo, LOAD, L1STATE_L3MOCS),
                      true /* has_dest */);
      inst->mlen = lsc_msg_desc_src0_len(devinfo, inst->desc);

      setup_lsc_surface_descriptors(bld, inst, inst->desc,
                                    surface.file != BAD_FILE ?
                                    surface : surface_handle);

      /* The byte scattered messages can only read one dword at a time so
       * we have to duplicate the message 4 times to read the full vec4.
       * Hopefully, dead code will clean up the mess if some of them aren't
       * needed.
       */
      assert(inst->size_written == 16 * inst->exec_size);
      inst->size_written /= 4;
      for (unsigned c = 1; c < 4; c++) {
         /* Emit a copy of the instruction because we're about to modify
          * it.  Because this loop starts at 1, we will emit copies for the
          * first 3 and the final one will be the modified instruction.
          */
         bld.emit(*inst);

         /* Offset the source */
         inst->src[2] = bld.vgrf(BRW_REGISTER_TYPE_UD);
         bld.ADD(inst->src[2], ubo_offset, brw_imm_ud(c * 4));

         /* Offset the destination */
         inst->dst = offset(inst->dst, bld, 1);
      }
   }
}

static void
lower_varying_pull_constant_logical_send(const fs_builder &bld, fs_inst *inst)
{
   const intel_device_info *devinfo = bld.shader->devinfo;
   const brw_compiler *compiler = bld.shader->compiler;

   if (devinfo->ver >= 7) {
      fs_reg surface = inst->src[PULL_VARYING_CONSTANT_SRC_SURFACE];
      fs_reg surface_handle = inst->src[PULL_VARYING_CONSTANT_SRC_SURFACE_HANDLE];
      fs_reg offset_B = inst->src[PULL_VARYING_CONSTANT_SRC_OFFSET];

      /* We are switching the instruction from an ALU-like instruction to a
       * send-from-grf instruction.  Since sends can't handle strides or
       * source modifiers, we have to make a copy of the offset source.
       */
      fs_reg ubo_offset = bld.vgrf(BRW_REGISTER_TYPE_UD);
      bld.MOV(ubo_offset, offset_B);

      assert(inst->src[PULL_VARYING_CONSTANT_SRC_ALIGNMENT].file == BRW_IMMEDIATE_VALUE);
      unsigned alignment = inst->src[PULL_VARYING_CONSTANT_SRC_ALIGNMENT].ud;

      inst->opcode = SHADER_OPCODE_SEND;
      inst->mlen = inst->exec_size / 8;
      inst->resize_sources(3);

      /* src[0] & src[1] are filled by setup_surface_descriptors() */
      inst->src[2] = ubo_offset; /* payload */

      if (compiler->indirect_ubos_use_sampler) {
         const unsigned simd_mode =
            inst->exec_size <= 8 ? BRW_SAMPLER_SIMD_MODE_SIMD8 :
                                   BRW_SAMPLER_SIMD_MODE_SIMD16;
         const uint32_t desc = brw_sampler_desc(devinfo, 0, 0,
                                                GFX5_SAMPLER_MESSAGE_SAMPLE_LD,
                                                simd_mode, 0);

         inst->sfid = BRW_SFID_SAMPLER;
         setup_surface_descriptors(bld, inst, desc, surface, surface_handle);
      } else if (alignment >= 4) {
         const uint32_t desc =
            brw_dp_untyped_surface_rw_desc(devinfo, inst->exec_size,
                                           4, /* num_channels */
                                           false   /* write */);

         inst->sfid = (devinfo->verx10 >= 75 ?
                       HSW_SFID_DATAPORT_DATA_CACHE_1 :
                       GFX7_SFID_DATAPORT_DATA_CACHE);
         setup_surface_descriptors(bld, inst, desc, surface, surface_handle);
      } else {
         const uint32_t desc =
            brw_dp_byte_scattered_rw_desc(devinfo, inst->exec_size,
                                          32,     /* bit_size */
                                          false   /* write */);

         inst->sfid = GFX7_SFID_DATAPORT_DATA_CACHE;
         setup_surface_descriptors(bld, inst, desc, surface, surface_handle);

         /* The byte scattered messages can only read one dword at a time so
          * we have to duplicate the message 4 times to read the full vec4.
          * Hopefully, dead code will clean up the mess if some of them aren't
          * needed.
          */
         assert(inst->size_written == 16 * inst->exec_size);
         inst->size_written /= 4;
         for (unsigned c = 1; c < 4; c++) {
            /* Emit a copy of the instruction because we're about to modify
             * it.  Because this loop starts at 1, we will emit copies for the
             * first 3 and the final one will be the modified instruction.
             */
            bld.emit(*inst);

            /* Offset the source */
            inst->src[2] = bld.vgrf(BRW_REGISTER_TYPE_UD);
            bld.ADD(inst->src[2], ubo_offset, brw_imm_ud(c * 4));

            /* Offset the destination */
            inst->dst = offset(inst->dst, bld, 1);
         }
      }
   } else {
      fs_reg surface = inst->src[PULL_VARYING_CONSTANT_SRC_SURFACE];
      fs_reg offset = inst->src[PULL_VARYING_CONSTANT_SRC_OFFSET];
      assert(inst->src[PULL_VARYING_CONSTANT_SRC_SURFACE_HANDLE].file == BAD_FILE);

      const fs_reg payload(MRF, FIRST_PULL_LOAD_MRF(devinfo->ver),
                           BRW_REGISTER_TYPE_UD);

      bld.MOV(byte_offset(payload, REG_SIZE), offset);

      inst->opcode = FS_OPCODE_VARYING_PULL_CONSTANT_LOAD_GFX4;
      inst->base_mrf = payload.nr;
      inst->header_size = 1;
      inst->mlen = 1 + inst->exec_size / 8;

      inst->resize_sources(1);
      inst->src[0] = surface;
   }
}

static void
lower_math_logical_send(const fs_builder &bld, fs_inst *inst)
{
   assert(bld.shader->devinfo->ver < 6);

   inst->base_mrf = 2;
   inst->mlen = inst->sources * inst->exec_size / 8;

   if (inst->sources > 1) {
      /* From the Ironlake PRM, Volume 4, Part 1, Section 6.1.13
       * "Message Payload":
       *
       * "Operand0[7].  For the INT DIV functions, this operand is the
       *  denominator."
       *  ...
       * "Operand1[7].  For the INT DIV functions, this operand is the
       *  numerator."
       */
      const bool is_int_div = inst->opcode != SHADER_OPCODE_POW;
      const fs_reg src0 = is_int_div ? inst->src[1] : inst->src[0];
      const fs_reg src1 = is_int_div ? inst->src[0] : inst->src[1];

      inst->resize_sources(1);
      inst->src[0] = src0;

      assert(inst->exec_size == 8);
      bld.MOV(fs_reg(MRF, inst->base_mrf + 1, src1.type), src1);
   }
}

static void
lower_interpolator_logical_send(const fs_builder &bld, fs_inst *inst,
                                const struct brw_wm_prog_key *wm_prog_key,
                                const struct brw_wm_prog_data *wm_prog_data)
{
   const intel_device_info *devinfo = bld.shader->devinfo;

   /* We have to send something */
   fs_reg payload = brw_vec8_grf(0, 0);
   unsigned mlen = 1;

   unsigned mode;
   switch (inst->opcode) {
   case FS_OPCODE_INTERPOLATE_AT_SAMPLE:
      assert(inst->src[INTERP_SRC_OFFSET].file == BAD_FILE);
      mode = GFX7_PIXEL_INTERPOLATOR_LOC_SAMPLE;
      break;

   case FS_OPCODE_INTERPOLATE_AT_SHARED_OFFSET:
      assert(inst->src[INTERP_SRC_OFFSET].file == BAD_FILE);
      mode = GFX7_PIXEL_INTERPOLATOR_LOC_SHARED_OFFSET;
      break;

   case FS_OPCODE_INTERPOLATE_AT_PER_SLOT_OFFSET:
      payload = inst->src[INTERP_SRC_OFFSET];
      mlen = 2 * inst->exec_size / 8;
      mode = GFX7_PIXEL_INTERPOLATOR_LOC_PER_SLOT_OFFSET;
      break;

   default:
      unreachable("Invalid interpolator instruction");
   }

   const bool dynamic_mode =
      inst->src[INTERP_SRC_DYNAMIC_MODE].file != BAD_FILE;

   fs_reg desc = inst->src[INTERP_SRC_MSG_DESC];
   uint32_t desc_imm =
      brw_pixel_interp_desc(devinfo,
                            /* Leave the mode at 0 if persample_dispatch is
                             * dynamic, it will be ORed in below.
                             */
                            dynamic_mode ? 0 : mode,
                            inst->pi_noperspective,
                            false /* coarse_pixel_rate */,
                            inst->exec_size, inst->group);

   if (wm_prog_data->coarse_pixel_dispatch == BRW_ALWAYS) {
      desc_imm |= (1 << 15);
   } else if (wm_prog_data->coarse_pixel_dispatch == BRW_SOMETIMES) {
      STATIC_ASSERT(BRW_WM_MSAA_FLAG_COARSE_PI_MSG == (1 << 15));
      fs_reg orig_desc = desc;
      const fs_builder &ubld = bld.exec_all().group(8, 0);
      desc = ubld.vgrf(BRW_REGISTER_TYPE_UD);
      ubld.AND(desc, dynamic_msaa_flags(wm_prog_data),
               brw_imm_ud(BRW_WM_MSAA_FLAG_COARSE_PI_MSG));

      /* And, if it's AT_OFFSET, we might have a non-trivial descriptor */
      if (orig_desc.file == IMM) {
         desc_imm |= orig_desc.ud;
      } else {
         ubld.OR(desc, desc, orig_desc);
      }
   }

   /* If persample_dispatch is dynamic, select the interpolation mode
    * dynamically and OR into the descriptor to complete the static part
    * generated by brw_pixel_interp_desc().
    *
    * Why does this work? If you look at the SKL PRMs, Volume 7:
    * 3D-Media-GPGPU, Shared Functions Pixel Interpolater, you'll see that
    *
    *   - "Per Message Offset” Message Descriptor
    *   - “Sample Position Offset” Message Descriptor
    *
    * have different formats. Fortunately, a fragment shader dispatched at
    * pixel rate, will have gl_SampleID = 0 & gl_NumSamples = 1. So the value
    * we pack in “Sample Position Offset” will be a 0 and will cover the X/Y
    * components of "Per Message Offset”, which will give us the pixel offset 0x0.
    */
   if (dynamic_mode) {
      fs_reg orig_desc = desc;
      const fs_builder &ubld = bld.exec_all().group(8, 0);
      desc = ubld.vgrf(BRW_REGISTER_TYPE_UD);

      /* The predicate should have been built in brw_fs_nir.cpp when emitting
       * NIR code. This guarantees that we do not have incorrect interactions
       * with the flag register holding the predication result.
       */
      if (orig_desc.file == IMM) {
         /* Not using SEL here because we would generate an instruction with 2
          * immediate sources which is not supported by HW.
          */
         set_predicate_inv(BRW_PREDICATE_NORMAL, false,
                           ubld.MOV(desc, brw_imm_ud(orig_desc.ud |
                                                     GFX7_PIXEL_INTERPOLATOR_LOC_SAMPLE << 12)));
         set_predicate_inv(BRW_PREDICATE_NORMAL, true,
                           ubld.MOV(desc, brw_imm_ud(orig_desc.ud |
                                                     GFX7_PIXEL_INTERPOLATOR_LOC_SHARED_OFFSET << 12)));
      } else {
         set_predicate_inv(BRW_PREDICATE_NORMAL, false,
                           ubld.OR(desc, orig_desc,
                                   brw_imm_ud(GFX7_PIXEL_INTERPOLATOR_LOC_SAMPLE << 12)));
         set_predicate_inv(BRW_PREDICATE_NORMAL, true,
                           ubld.OR(desc, orig_desc,
                                   brw_imm_ud(GFX7_PIXEL_INTERPOLATOR_LOC_SHARED_OFFSET << 12)));
      }
   }

   assert(bld.shader->devinfo->ver >= 7);
   inst->opcode = SHADER_OPCODE_SEND;
   inst->sfid = GFX7_SFID_PIXEL_INTERPOLATOR;
   inst->desc = desc_imm;
   inst->ex_desc = 0;
   inst->mlen = mlen;
   inst->ex_mlen = 0;
   inst->send_has_side_effects = false;
   inst->send_is_volatile = false;

   inst->resize_sources(3);
   inst->src[0] = component(desc, 0);
   inst->src[1] = brw_imm_ud(0); /* ex_desc */
   inst->src[2] = payload;
}

static void
lower_btd_logical_send(const fs_builder &bld, fs_inst *inst)
{
   const intel_device_info *devinfo = bld.shader->devinfo;
   fs_reg global_addr = inst->src[0];
   const fs_reg btd_record = inst->src[1];

   const unsigned unit = reg_unit(devinfo);
   const unsigned mlen = 2 * unit;
   const fs_builder ubld = bld.exec_all();
   fs_reg header = ubld.vgrf(BRW_REGISTER_TYPE_UD, 2 * unit);

   ubld.MOV(header, brw_imm_ud(0));
   switch (inst->opcode) {
   case SHADER_OPCODE_BTD_SPAWN_LOGICAL:
      assert(type_sz(global_addr.type) == 8 && global_addr.stride == 0);
      global_addr.type = BRW_REGISTER_TYPE_UD;
      global_addr.stride = 1;
      ubld.group(2, 0).MOV(header, global_addr);
      break;

   case SHADER_OPCODE_BTD_RETIRE_LOGICAL:
      /* The bottom bit is the Stack ID release bit */
      ubld.group(1, 0).MOV(header, brw_imm_ud(1));
      break;

   default:
      unreachable("Invalid BTD message");
   }

   /* Stack IDs are always in R1 regardless of whether we're coming from a
    * bindless shader or a regular compute shader.
    */
   fs_reg stack_ids = retype(offset(header, bld, 1), BRW_REGISTER_TYPE_UW);
   bld.exec_all().MOV(stack_ids, retype(brw_vec8_grf(1 * unit, 0),
                                        BRW_REGISTER_TYPE_UW));

   unsigned ex_mlen = 0;
   fs_reg payload;
   if (inst->opcode == SHADER_OPCODE_BTD_SPAWN_LOGICAL) {
      ex_mlen = 2 * (inst->exec_size / 8);
      payload = bld.move_to_vgrf(btd_record, 1);
   } else {
      assert(inst->opcode == SHADER_OPCODE_BTD_RETIRE_LOGICAL);
      /* All these messages take a BTD and things complain if we don't provide
       * one for RETIRE.  However, it shouldn't ever actually get used so fill
       * it with zero.
       */
      ex_mlen = 2 * (inst->exec_size / 8);
      payload = bld.move_to_vgrf(brw_imm_uq(0), 1);
   }

   /* Update the original instruction. */
   inst->opcode = SHADER_OPCODE_SEND;
   inst->mlen = mlen;
   inst->ex_mlen = ex_mlen;
   inst->header_size = 0; /* HW docs require has_header = false */
   inst->send_has_side_effects = true;
   inst->send_is_volatile = false;

   /* Set up SFID and descriptors */
   inst->sfid = GEN_RT_SFID_BINDLESS_THREAD_DISPATCH;
   inst->desc = brw_btd_spawn_desc(devinfo, inst->exec_size,
                                   GEN_RT_BTD_MESSAGE_SPAWN);
   inst->resize_sources(4);
   inst->src[0] = brw_imm_ud(0); /* desc */
   inst->src[1] = brw_imm_ud(0); /* ex_desc */
   inst->src[2] = header;
   inst->src[3] = payload;
}

static void
lower_trace_ray_logical_send(const fs_builder &bld, fs_inst *inst)
{
   const intel_device_info *devinfo = bld.shader->devinfo;
   /* The emit_uniformize() in brw_fs_nir.cpp will generate an horizontal
    * stride of 0. Below we're doing a MOV() in SIMD2. Since we can't use UQ/Q
    * types in on Gfx12.5, we need to tweak the stride with a value of 1 dword
    * so that the MOV operates on 2 components rather than twice the same
    * component.
    */
   fs_reg globals_addr = retype(inst->src[RT_LOGICAL_SRC_GLOBALS], BRW_REGISTER_TYPE_UD);
   globals_addr.stride = 1;
   const fs_reg bvh_level =
      inst->src[RT_LOGICAL_SRC_BVH_LEVEL].file == BRW_IMMEDIATE_VALUE ?
      inst->src[RT_LOGICAL_SRC_BVH_LEVEL] :
      bld.move_to_vgrf(inst->src[RT_LOGICAL_SRC_BVH_LEVEL],
                       inst->components_read(RT_LOGICAL_SRC_BVH_LEVEL));
   const fs_reg trace_ray_control =
      inst->src[RT_LOGICAL_SRC_TRACE_RAY_CONTROL].file == BRW_IMMEDIATE_VALUE ?
      inst->src[RT_LOGICAL_SRC_TRACE_RAY_CONTROL] :
      bld.move_to_vgrf(inst->src[RT_LOGICAL_SRC_TRACE_RAY_CONTROL],
                       inst->components_read(RT_LOGICAL_SRC_TRACE_RAY_CONTROL));
   const fs_reg synchronous_src = inst->src[RT_LOGICAL_SRC_SYNCHRONOUS];
   assert(synchronous_src.file == BRW_IMMEDIATE_VALUE);
   const bool synchronous = synchronous_src.ud;

   const unsigned unit = reg_unit(devinfo);
   const unsigned mlen = unit;
   const fs_builder ubld = bld.exec_all();
   fs_reg header = ubld.vgrf(BRW_REGISTER_TYPE_UD);
   ubld.MOV(header, brw_imm_ud(0));
   ubld.group(2, 0).MOV(header, globals_addr);
   if (synchronous)
      ubld.group(1, 0).MOV(byte_offset(header, 16), brw_imm_ud(synchronous));

   const unsigned ex_mlen = inst->exec_size / 8;
   fs_reg payload = bld.vgrf(BRW_REGISTER_TYPE_UD);
   if (bvh_level.file == BRW_IMMEDIATE_VALUE &&
       trace_ray_control.file == BRW_IMMEDIATE_VALUE) {
      bld.MOV(payload, brw_imm_ud(SET_BITS(trace_ray_control.ud, 9, 8) |
                                  (bvh_level.ud & 0x7)));
   } else {
      bld.SHL(payload, trace_ray_control, brw_imm_ud(8));
      bld.OR(payload, payload, bvh_level);
   }

   /* When doing synchronous traversal, the HW implicitly computes the
    * stack_id using the following formula :
    *
    *    EUID[3:0] & THREAD_ID[2:0] & SIMD_LANE_ID[3:0]
    *
    * Only in the asynchronous case we need to set the stack_id given from the
    * payload register.
    */
   if (!synchronous) {
      bld.AND(subscript(payload, BRW_REGISTER_TYPE_UW, 1),
              retype(brw_vec8_grf(1 * unit, 0), BRW_REGISTER_TYPE_UW),
              brw_imm_uw(0x7ff));
   }

   /* Update the original instruction. */
   inst->opcode = SHADER_OPCODE_SEND;
   inst->mlen = mlen;
   inst->ex_mlen = ex_mlen;
   inst->header_size = 0; /* HW docs require has_header = false */
   inst->send_has_side_effects = true;
   inst->send_is_volatile = false;

   /* Set up SFID and descriptors */
   inst->sfid = GEN_RT_SFID_RAY_TRACE_ACCELERATOR;
   inst->desc = brw_rt_trace_ray_desc(devinfo, inst->exec_size);
   inst->resize_sources(4);
   inst->src[0] = brw_imm_ud(0); /* desc */
   inst->src[1] = brw_imm_ud(0); /* ex_desc */
   inst->src[2] = header;
   inst->src[3] = payload;
}

static void
lower_get_buffer_size(const fs_builder &bld, fs_inst *inst)
{
   const intel_device_info *devinfo = bld.shader->devinfo;
   assert(devinfo->ver >= 7);
   /* Since we can only execute this instruction on uniform bti/surface
    * handles, brw_fs_nir.cpp should already have limited this to SIMD8.
    */
   assert(inst->exec_size == (devinfo->ver < 20 ? 8 : 16));

   fs_reg surface = inst->src[GET_BUFFER_SIZE_SRC_SURFACE];
   fs_reg surface_handle = inst->src[GET_BUFFER_SIZE_SRC_SURFACE_HANDLE];
   fs_reg lod = inst->src[GET_BUFFER_SIZE_SRC_LOD];

   inst->opcode = SHADER_OPCODE_SEND;
   inst->mlen = inst->exec_size / 8;
   inst->resize_sources(3);
   inst->ex_mlen = 0;
   inst->ex_desc = 0;

   /* src[0] & src[1] are filled by setup_surface_descriptors() */
   inst->src[2] = lod;

   const uint32_t return_format = devinfo->ver >= 8 ?
      GFX8_SAMPLER_RETURN_FORMAT_32BITS : BRW_SAMPLER_RETURN_FORMAT_SINT32;

   const uint32_t desc = brw_sampler_desc(devinfo, 0, 0,
                                          GFX5_SAMPLER_MESSAGE_SAMPLE_RESINFO,
                                          BRW_SAMPLER_SIMD_MODE_SIMD8,
                                          return_format);

   inst->dst = retype(inst->dst, BRW_REGISTER_TYPE_UW);
   inst->sfid = BRW_SFID_SAMPLER;
   setup_surface_descriptors(bld, inst, desc, surface, surface_handle);
}

bool
fs_visitor::lower_logical_sends()
{
   bool progress = false;

   foreach_block_and_inst_safe(block, fs_inst, inst, cfg) {
      const fs_builder ibld(this, block, inst);

      switch (inst->opcode) {
      case FS_OPCODE_FB_WRITE_LOGICAL:
         assert(stage == MESA_SHADER_FRAGMENT);
         lower_fb_write_logical_send(ibld, inst,
                                     brw_wm_prog_data(prog_data),
                                     (const brw_wm_prog_key *)key,
                                     fs_payload());
         break;

      case FS_OPCODE_FB_READ_LOGICAL:
         lower_fb_read_logical_send(ibld, inst);
         break;

      case SHADER_OPCODE_TEX_LOGICAL:
         lower_sampler_logical_send(ibld, inst, SHADER_OPCODE_TEX);
         break;

      case SHADER_OPCODE_TXD_LOGICAL:
         lower_sampler_logical_send(ibld, inst, SHADER_OPCODE_TXD);
         break;

      case SHADER_OPCODE_TXF_LOGICAL:
         lower_sampler_logical_send(ibld, inst, SHADER_OPCODE_TXF);
         break;

      case SHADER_OPCODE_TXL_LOGICAL:
         lower_sampler_logical_send(ibld, inst, SHADER_OPCODE_TXL);
         break;

      case SHADER_OPCODE_TXS_LOGICAL:
         lower_sampler_logical_send(ibld, inst, SHADER_OPCODE_TXS);
         break;

      case SHADER_OPCODE_IMAGE_SIZE_LOGICAL:
         lower_sampler_logical_send(ibld, inst,
                                    SHADER_OPCODE_IMAGE_SIZE_LOGICAL);
         break;

      case FS_OPCODE_TXB_LOGICAL:
         lower_sampler_logical_send(ibld, inst, FS_OPCODE_TXB);
         break;

      case SHADER_OPCODE_TXF_CMS_LOGICAL:
         lower_sampler_logical_send(ibld, inst, SHADER_OPCODE_TXF_CMS);
         break;

      case SHADER_OPCODE_TXF_CMS_W_LOGICAL:
      case SHADER_OPCODE_TXF_CMS_W_GFX12_LOGICAL:
         lower_sampler_logical_send(ibld, inst, SHADER_OPCODE_TXF_CMS_W);
         break;

      case SHADER_OPCODE_TXF_UMS_LOGICAL:
         lower_sampler_logical_send(ibld, inst, SHADER_OPCODE_TXF_UMS);
         break;

      case SHADER_OPCODE_TXF_MCS_LOGICAL:
         lower_sampler_logical_send(ibld, inst, SHADER_OPCODE_TXF_MCS);
         break;

      case SHADER_OPCODE_LOD_LOGICAL:
         lower_sampler_logical_send(ibld, inst, SHADER_OPCODE_LOD);
         break;

      case SHADER_OPCODE_TG4_LOGICAL:
         lower_sampler_logical_send(ibld, inst, SHADER_OPCODE_TG4);
         break;

      case SHADER_OPCODE_TG4_OFFSET_LOGICAL:
         lower_sampler_logical_send(ibld, inst, SHADER_OPCODE_TG4_OFFSET);
         break;

      case SHADER_OPCODE_SAMPLEINFO_LOGICAL:
         lower_sampler_logical_send(ibld, inst, SHADER_OPCODE_SAMPLEINFO);
         break;

      case SHADER_OPCODE_GET_BUFFER_SIZE:
         lower_get_buffer_size(ibld, inst);
         break;

      case SHADER_OPCODE_UNTYPED_SURFACE_READ_LOGICAL:
      case SHADER_OPCODE_UNTYPED_SURFACE_WRITE_LOGICAL:
      case SHADER_OPCODE_UNTYPED_ATOMIC_LOGICAL:
      case SHADER_OPCODE_BYTE_SCATTERED_READ_LOGICAL:
      case SHADER_OPCODE_BYTE_SCATTERED_WRITE_LOGICAL:
         if (devinfo->has_lsc) {
            lower_lsc_surface_logical_send(ibld, inst);
            break;
         }
      case SHADER_OPCODE_DWORD_SCATTERED_READ_LOGICAL:
      case SHADER_OPCODE_DWORD_SCATTERED_WRITE_LOGICAL:
      case SHADER_OPCODE_TYPED_SURFACE_READ_LOGICAL:
      case SHADER_OPCODE_TYPED_SURFACE_WRITE_LOGICAL:
      case SHADER_OPCODE_TYPED_ATOMIC_LOGICAL:
         lower_surface_logical_send(ibld, inst);
         break;

      case SHADER_OPCODE_UNALIGNED_OWORD_BLOCK_READ_LOGICAL:
      case SHADER_OPCODE_OWORD_BLOCK_WRITE_LOGICAL:
         if (devinfo->has_lsc) {
            lower_lsc_block_logical_send(ibld, inst);
            break;
         }
         lower_surface_block_logical_send(ibld, inst);
         break;

      case SHADER_OPCODE_A64_UNTYPED_WRITE_LOGICAL:
      case SHADER_OPCODE_A64_UNTYPED_READ_LOGICAL:
      case SHADER_OPCODE_A64_BYTE_SCATTERED_WRITE_LOGICAL:
      case SHADER_OPCODE_A64_BYTE_SCATTERED_READ_LOGICAL:
      case SHADER_OPCODE_A64_UNTYPED_ATOMIC_LOGICAL:
      case SHADER_OPCODE_A64_OWORD_BLOCK_READ_LOGICAL:
      case SHADER_OPCODE_A64_UNALIGNED_OWORD_BLOCK_READ_LOGICAL:
      case SHADER_OPCODE_A64_OWORD_BLOCK_WRITE_LOGICAL:
         if (devinfo->has_lsc) {
            lower_lsc_a64_logical_send(ibld, inst);
            break;
         }
         lower_a64_logical_send(ibld, inst);
         break;

      case FS_OPCODE_VARYING_PULL_CONSTANT_LOAD_LOGICAL:
         if (devinfo->has_lsc && !compiler->indirect_ubos_use_sampler)
            lower_lsc_varying_pull_constant_logical_send(ibld, inst);
         else
            lower_varying_pull_constant_logical_send(ibld, inst);
         break;

      case SHADER_OPCODE_RCP:
      case SHADER_OPCODE_RSQ:
      case SHADER_OPCODE_SQRT:
      case SHADER_OPCODE_EXP2:
      case SHADER_OPCODE_LOG2:
      case SHADER_OPCODE_SIN:
      case SHADER_OPCODE_COS:
      case SHADER_OPCODE_POW:
      case SHADER_OPCODE_INT_QUOTIENT:
      case SHADER_OPCODE_INT_REMAINDER:
         /* The math opcodes are overloaded for the send-like and
          * expression-like instructions which seems kind of icky.  Gfx6+ has
          * a native (but rather quirky) MATH instruction so we don't need to
          * do anything here.  On Gfx4-5 we'll have to lower the Gfx6-like
          * logical instructions (which we can easily recognize because they
          * have mlen = 0) into send-like virtual instructions.
          */
         if (devinfo->ver < 6 && inst->mlen == 0) {
            lower_math_logical_send(ibld, inst);
            break;

         } else {
            continue;
         }

      case FS_OPCODE_INTERPOLATE_AT_SAMPLE:
      case FS_OPCODE_INTERPOLATE_AT_SHARED_OFFSET:
      case FS_OPCODE_INTERPOLATE_AT_PER_SLOT_OFFSET:
         lower_interpolator_logical_send(ibld, inst,
                                         (const brw_wm_prog_key *)key,
                                         brw_wm_prog_data(prog_data));
         break;

      case SHADER_OPCODE_BTD_SPAWN_LOGICAL:
      case SHADER_OPCODE_BTD_RETIRE_LOGICAL:
         lower_btd_logical_send(ibld, inst);
         break;

      case RT_OPCODE_TRACE_RAY_LOGICAL:
         lower_trace_ray_logical_send(ibld, inst);
         break;

      case SHADER_OPCODE_URB_READ_LOGICAL:
         if (devinfo->ver < 20)
            lower_urb_read_logical_send(ibld, inst);
         else
            lower_urb_read_logical_send_xe2(ibld, inst);
         break;

      case SHADER_OPCODE_URB_WRITE_LOGICAL:
         if (devinfo->ver < 20)
            lower_urb_write_logical_send(ibld, inst);
         else
            lower_urb_write_logical_send_xe2(ibld, inst);

         break;

      default:
         continue;
      }

      progress = true;
   }

   if (progress)
      invalidate_analysis(DEPENDENCY_INSTRUCTIONS | DEPENDENCY_VARIABLES);

   return progress;
}

/**
 * Turns the generic expression-style uniform pull constant load instruction
 * into a hardware-specific series of instructions for loading a pull
 * constant.
 *
 * The expression style allows the CSE pass before this to optimize out
 * repeated loads from the same offset, and gives the pre-register-allocation
 * scheduling full flexibility, while the conversion to native instructions
 * allows the post-register-allocation scheduler the best information
 * possible.
 *
 * Note that execution masking for setting up pull constant loads is special:
 * the channels that need to be written are unrelated to the current execution
 * mask, since a later instruction will use one of the result channels as a
 * source operand for all 8 or 16 of its channels.
 */
bool
fs_visitor::lower_uniform_pull_constant_loads()
{
   bool progress = false;

   foreach_block_and_inst (block, fs_inst, inst, cfg) {
      if (inst->opcode != FS_OPCODE_UNIFORM_PULL_CONSTANT_LOAD)
         continue;

      const fs_reg surface = inst->src[PULL_UNIFORM_CONSTANT_SRC_SURFACE];
      const fs_reg surface_handle = inst->src[PULL_UNIFORM_CONSTANT_SRC_SURFACE_HANDLE];
      const fs_reg offset_B = inst->src[PULL_UNIFORM_CONSTANT_SRC_OFFSET];
      const fs_reg size_B = inst->src[PULL_UNIFORM_CONSTANT_SRC_SIZE];
      assert(surface.file == BAD_FILE || surface_handle.file == BAD_FILE);
      assert(offset_B.file == IMM);
      assert(size_B.file == IMM);

      if (devinfo->has_lsc) {
         const fs_builder ubld =
            fs_builder(this, block, inst).group(8, 0).exec_all();

         const fs_reg payload = ubld.vgrf(BRW_REGISTER_TYPE_UD);
         ubld.MOV(payload, offset_B);

         inst->sfid = GFX12_SFID_UGM;
         inst->desc = lsc_msg_desc(devinfo, LSC_OP_LOAD,
                                   1 /* simd_size */,
                                   surface_handle.file == BAD_FILE ?
                                   LSC_ADDR_SURFTYPE_BTI :
                                   LSC_ADDR_SURFTYPE_BSS,
                                   LSC_ADDR_SIZE_A32,
                                   1 /* num_coordinates */,
                                   LSC_DATA_SIZE_D32,
                                   inst->size_written / 4,
                                   true /* transpose */,
                                   LSC_CACHE(devinfo, LOAD, L1STATE_L3MOCS),
                                   true /* has_dest */);

         /* Update the original instruction. */
         inst->opcode = SHADER_OPCODE_SEND;
         inst->mlen = lsc_msg_desc_src0_len(devinfo, inst->desc);
         inst->send_ex_bso = surface_handle.file != BAD_FILE &&
                             compiler->extended_bindless_surface_offset;
         inst->ex_mlen = 0;
         inst->header_size = 0;
         inst->send_has_side_effects = false;
         inst->send_is_volatile = true;
         inst->exec_size = 1;

         /* Finally, the payload */

         inst->resize_sources(3);
         setup_lsc_surface_descriptors(ubld, inst, inst->desc,
                                       surface.file != BAD_FILE ?
                                       surface : surface_handle);
         inst->src[2] = payload;

         invalidate_analysis(DEPENDENCY_INSTRUCTIONS | DEPENDENCY_VARIABLES);
      } else if (devinfo->ver >= 7) {
         const fs_builder ubld = fs_builder(this, block, inst).exec_all();
         fs_reg header = fs_builder(this, 8).exec_all().vgrf(BRW_REGISTER_TYPE_UD);

         ubld.group(8, 0).MOV(header,
                              retype(brw_vec8_grf(0, 0), BRW_REGISTER_TYPE_UD));
         ubld.group(1, 0).MOV(component(header, 2),
                              brw_imm_ud(offset_B.ud / 16));

         inst->sfid = GFX6_SFID_DATAPORT_CONSTANT_CACHE;
         inst->opcode = SHADER_OPCODE_SEND;
         inst->header_size = 1;
         inst->mlen = 1;

         uint32_t desc =
            brw_dp_oword_block_rw_desc(devinfo, true /* align_16B */,
                                       size_B.ud / 4, false /* write */);

         inst->resize_sources(4);

         setup_surface_descriptors(ubld, inst, desc, surface, surface_handle);

         inst->src[2] = header;
         inst->src[3] = fs_reg(); /* unused for reads */

         invalidate_analysis(DEPENDENCY_INSTRUCTIONS | DEPENDENCY_VARIABLES);
      } else {
         assert(surface_handle.file == BAD_FILE);
         /* Before register allocation, we didn't tell the scheduler about the
          * MRF we use.  We know it's safe to use this MRF because nothing
          * else does except for register spill/unspill, which generates and
          * uses its MRF within a single IR instruction.
          */
         inst->base_mrf = FIRST_PULL_LOAD_MRF(devinfo->ver) + 1;
         inst->mlen = 1;
      }

      progress = true;
   }

   return progress;
}
