/*
 * Copyright © 2016 Broadcom
 * Copyright © 2020 Google LLC
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

/* Unit test for disassembly of instructions.
 *
 * The goal is to take instructions we've seen the blob produce, and test that
 * we can disassemble them correctly.  For the next person investigating the
 * behavior of this instruction, please include the testcase it was generated
 * from, and the qcom disassembly as a comment if it differs from what we
 * produce.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util/macros.h"

#include "ir3.h"
#include "ir3_assembler.h"
#include "ir3_shader.h"

#include "isa/isa.h"

/* clang-format off */
/* Note: @anholt's 4xx disasm was done on an a418 Nexus 5x */
#define INSTR_4XX(i, d, ...) { .gpu_id = 420, .instr = #i, .expected = d, __VA_ARGS__ }
#define INSTR_5XX(i, d, ...) { .gpu_id = 540, .instr = #i, .expected = d, __VA_ARGS__ }
#define INSTR_6XX(i, d, ...) { .gpu_id = 630, .instr = #i, .expected = d, __VA_ARGS__ }
#define INSTR_7XX(i, d, ...) { .chip_id = 0x07030001, .instr = #i, .expected = d, __VA_ARGS__ }
/* clang-format on */

static const struct test {
   int gpu_id;
   int chip_id;
   const char *instr;
   const char *expected;
   /**
    * Do we expect asm parse fail (ie. for things not (yet) supported by
    * ir3_parser.y)
    */
   bool parse_fail;
} tests[] = {
   /* clang-format off */
   /* cat0 */
   INSTR_6XX(00000000_00000000, "nop"),
   INSTR_6XX(00000200_00000000, "(rpt2)nop"),
   INSTR_6XX(00010000_00000000, "(eq)nop"),
   INSTR_6XX(03000000_00000000, "end"),
   INSTR_6XX(00800000_00000004, "br p0.x, #4"),
   INSTR_6XX(00800000_fffffffc, "br p0.x, #-4"),
   INSTR_6XX(00900000_00000003, "br !p0.x, #3"),
   INSTR_6XX(03820000_00000015, "shps #21"), /* emit */
   INSTR_6XX(04021000_00000000, "(ss)shpe"), /* cut */
   INSTR_6XX(02220000_00000004, "getlast.w8 #4"),
   INSTR_6XX(02820000_00000014, "getone #20"), /* kill p0.x */
   INSTR_6XX(00906020_00000007, "brao !p0.x, !p0.y, #7"),
   INSTR_6XX(00804040_00000003, "braa p0.x, p0.y, #3"),
   INSTR_6XX(07820000_00000000, "prede"),
   INSTR_6XX(00800063_0000001e, "brac.3 #30"),
   INSTR_6XX(06820000_00000000, "predt p0.x"),
   INSTR_6XX(07020000_00000000, "predf p0.x"),
   INSTR_6XX(07820000_00000000, "prede"),

   /* cat1 */
   INSTR_6XX(20244000_00000020, "mov.f32f32 r0.x, c8.x"),
   INSTR_6XX(20200000_00000020, "mov.f16f16 hr0.x, hc8.x"),
   INSTR_6XX(20150000_00000000, "cov.s32s16 hr0.x, r0.x"),
   INSTR_6XX(20156004_00000c11, "(ul)mov.s32s32 r1.x, c<a0.x + 17>"),
   INSTR_6XX(201100f4_00000000, "mova a0.x, hr0.x"),
   INSTR_6XX(20244905_00000410, "(rpt1)mov.f32f32 r1.y, (r)c260.x"),
   INSTR_6XX(20174004_00000008, "mov.s32s32 r<a0.x + 4>, r2.x"),
   INSTR_6XX(20130000_00000005, "mov.s16s16 hr<a0.x>, hr1.y"),
   INSTR_6XX(20110004_00000800, "mov.s16s16 hr1.x, hr<a0.x>"),
   /* dEQP-VK.subgroups.ballot.compute.compute */
   INSTR_6XX(260cc3c0_00000000, "movmsk.w128 r48.x"), /* movmsk.w128 sr48.x */

   INSTR_6XX(240cc004_00030201, "swz.u32u32 r1.x, r0.w, r0.y, r0.z"),
   INSTR_6XX(2400c105_04030201, "gat.f16u32 r1.y, hr0.y, hr0.z, hr0.w, hr1.x"),
   INSTR_6XX(240c0205_04030201, "sct.u32f16 hr1.y, hr0.z, hr0.w, hr1.x, r0.y"),
   INSTR_6XX(2400c205_04030201, "sct.f16u32 r1.y, r0.z, r0.w, r1.x, hr0.y"),

   INSTR_6XX(20510005_0000ffff, "mov.s16s16 hr1.y, -1"),
   INSTR_6XX(20400005_00003900, "mov.f16f16 hr1.y, h(0.625000)"),
   INSTR_6XX(20400006_00003800, "mov.f16f16 hr1.z, h(0.500000)"),
   INSTR_6XX(204880f5_00000000, "mova1 a1.x, 0"),

   INSTR_7XX(2004c005_00000405, "cov.f32u32 r1.y, (last)r1.y"),

   /* cat2 */
   INSTR_6XX(40104002_0c210001, "add.f hr0.z, r0.y, c<a0.x + 33>"),
   INSTR_6XX(40b80804_10408004, "(nop3) cmps.f.lt r1.x, (abs)r1.x, c16.x"),
   INSTR_6XX(47308a02_00002000, "(rpt2)bary.f (ei)r0.z, (r)0, r0.x"),
   INSTR_6XX(47348000_00002000, "flat.b (ei)r0.x, 0, r0.x"),
   INSTR_6XX(43480801_00008001, "(nop3) absneg.s hr0.y, (abs)hr0.y"),
   INSTR_6XX(50600004_2c010004, "(sy)mul.f hr1.x, hr1.x, h(0.5)"),
   INSTR_6XX(42280807_27ff0000, "(nop3) add.s hr1.w, hr0.x, h(-1)"),
   INSTR_6XX(40a500f8_2c000004, "cmps.f.ne p0.x, hr1.x, h(0.0)"),
   INSTR_6XX(438000f8_20010009, "and.b p0.x, hr2.y, h(1)"),
   INSTR_6XX(438000f9_00020001, "and.b p0.y, hr0.y, hr0.z"),
   INSTR_6XX(40080902_50200006, "(rpt1)add.f hr0.z, (r)hr1.z, (neg)(r)hc8.x"),
   INSTR_6XX(42380c01_00040001, "(sat)(nop3) add.s r0.y, r0.y, r1.x"),
   INSTR_6XX(42480000_48801086, "(nop2) sub.u hr0.x, hc33.z, (neg)hr<a0.x + 128>"),
   INSTR_6XX(46b00001_00001020, "clz.b r0.y, c8.x"),
   INSTR_6XX(46700009_00000009, "bfrev.b r2.y, r2.y"),

   INSTR_7XX(42380800_04010400, "(nop3) add.s r0.x, (last)r0.x, (last)r0.y"),
   INSTR_7XX(42930000_04000406, "cmps.u.ge r0.x, (last)r1.z, (last)r0.x"),

   /* cat3 */
   INSTR_6XX(66000000_10421041, "sel.f16 hr0.x, hc16.y, hr0.x, hc16.z"),
   INSTR_6XX(64848109_109a9099, "(rpt1)sel.b32 r2.y, c38.y, (r)r2.y, c38.z"),
   INSTR_6XX(64810904_30521036, "(rpt1)sel.b32 r1.x, (r)c13.z, r0.z, (r)c20.z"),
   INSTR_6XX(64818902_20041032, "(rpt1)sel.b32 r0.z, (r)c12.z, r0.w, (r)r1.x"),
   INSTR_6XX(63820005_10315030, "mad.f32 r1.y, (neg)c12.x, r1.x, c12.y"),
   INSTR_6XX(62050009_00091000, "mad.u24 r2.y, c0.x, r2.z, r2.y"),
   INSTR_6XX(61828008_00081033, "madsh.m16 r2.x, c12.w, r1.y, r2.x"),
   INSTR_6XX(65900820_100cb008, "(nop3) shlg hr8.x, 8, hr8.x, 12"), /* (nop3) shlg.b16 hr8.x, (r)8, (r)hr8.x, 12; */
   INSTR_6XX(65ae085c_0002a001, "(nop3) shlg hr23.x, hr0.y, hr23.x, hr0.z"), /* not seen in blob */
   INSTR_6XX(65900820_0c0aac05, "(nop3) shlg hr8.x, hc<a0.x + 5>, hr8.x, hc<a0.x + 10>"), /* not seen in blob */
   INSTR_6XX(65ae0c5c_0002a001, "(nop3) shlg r23.x, r0.y, r23.x, r0.z"), /* (nop3) shlg.b32 r23.x, (r)r0.y, (r)r23.x, r0.z */
   INSTR_6XX(64018802_0002e003, "(nop3) shrm hr0.z, (neg)hr0.w, hr0.w, hr0.z"),
   INSTR_6XX(64818802_0002e003, "(nop3) shlm hr0.z, (neg)hr0.w, hr0.w, hr0.z"),
   INSTR_6XX(65018802_0002e003, "(nop3) shrg hr0.z, (neg)hr0.w, hr0.w, hr0.z"),
   INSTR_6XX(66018802_0002e003, "(nop3) andg hr0.z, (neg)hr0.w, hr0.w, hr0.z"),
   INSTR_6XX(67018802_1002e003, "(nop3) wmm hr0.z, (neg)hr0.w, hr0.w, 2"), /* (nop3) wmm.f16f16 hr0.z, (abs)(r)hr0.w, (r)hr0.w, 2 */
   INSTR_6XX(67018c02_1002e003, "(nop3) wmm.accu hr0.z, (neg)hr0.w, hr0.w, 2"),
   INSTR_6XX(6701c802_9002a003, "(nop3) wmm r0.z, r0.w, r0.w, 2"), /* (nop3) wmm.f32f32 r0.z, (r)r0.w, (r)r0.w, 2 */
   /* custom test with qcom_dot8 function from cl_qcom_dot_product8 */
   INSTR_6XX(66818c02_0002e003, "(sat)(nop3) dp2acc.mixed.low r0.z, r0.w, r0.w, r0.z"), /* (nop3) dp2acc (sat)r0.z, (signed)(low)(r)r0.w, (low)(r)r0.w, r0.z */
   INSTR_6XX(6681c802_8002a003, "(nop3) dp4acc.unsigned.low r0.z, r0.w, r0.w, (neg)r0.z"), /* (nop3) dp4acc r0.z, (unsigned)(r)r0.w, (r)r0.w, (neg)r0.z */

   INSTR_7XX(61808000_04020400, "madsh.m16 r0.x, (last)r0.x, r0.y, (last)r0.z"),
   INSTR_7XX(64838806_04088406, "(nop3) sel.b32 r1.z, (last)r1.z, r1.w, (last)r2.x"),

   /* cat4 */
   INSTR_6XX(8010000a_00000003, "rcp r2.z, r0.w"),

   /* cat5 */
   /* dEQP-VK.glsl.derivate.dfdx.uniform_if.float_mediump */
   INSTR_6XX(a3801102_00000001, "dsx (f32)(x)r0.z, r0.x"), /* dsx (f32)(xOOO)r0.z, r0.x */
   /* dEQP-VK.glsl.derivate.dfdy.uniform_if.float_mediump */
   INSTR_6XX(a3c01102_00000001, "dsy (f32)(x)r0.z, r0.x"), /* dsy (f32)(xOOO)r0.z, r0.x */
   /* dEQP-VK.glsl.derivate.dfdxfine.uniform_loop.float_highp */
   INSTR_6XX(a6001105_00000001, "dsxpp.1 (x)r1.y, r0.x"), /* dsxpp.1 (xOOO)r1.y, r0.x */
   INSTR_6XX(a6201105_00000001, "dsxpp.1.p (x)r1.y, r0.x"), /* dsxpp.1 (xOOO)r1.y, r0.x */

   INSTR_6XX(a2802f00_00000001, "getsize (u16)(xyzw)hr0.x, r0.x, t#0"),
   INSTR_6XX(a0c89f04_c4600005, "sam.base1 (f32)(xyzw)r1.x, r0.z, s#3, t#2"),  /* sam.s2en.mode6.base1 (f32)(xyzw)r1.x, r0.z, 35 */
   INSTR_6XX(a1c85f00_c0200005, "getlod.base0 (s32)(xyzw)r0.x, r0.z, s#1, t#0"),  /* getlod.s2en.mode6.base0 (s32)(xyzw)r0.x, r0.z, 1 */
   INSTR_6XX(a1000f00_00000004, "samb (f16)(xyzw)hr0.x, hr0.z, hr0.x, s#0, t#0"),
   INSTR_6XX(a1000f00_00000003, "samb (f16)(xyzw)hr0.x, r0.y, r0.x, s#0, t#0"),
   INSTR_6XX(a0c00f00_04400002, "sam (f16)(xyzw)hr0.x, hr0.y, s#2, t#2"),
   INSTR_6XX(a6c02f00_00000000, "rgetinfo (u16)(xyzw)hr0.x"),
   INSTR_6XX(a3482f08_c0000000, "getinfo.base0 (u16)(xyzw)hr2.x, t#0"),
   /* dEQP-GLES31.functional.texture.texture_buffer.render.as_fragment_texture.buffer_size_65536 */
   INSTR_5XX(a2c03102_00000000, "getbuf (u32)(x)r0.z, t#0"),
   INSTR_6XX(a0c81f00_e0200005, "sam.base0 (f32)(xyzw)r0.x, r0.z, s#1, a1.x"),
   INSTR_6XX(a0c81108_e2000001, "sam.base0 (f32)(x)r2.x, r0.x, s#16, a1.x"),
   INSTR_6XX(a048d107_cc080a07, "isaml.base3 (s32)(x)r1.w, r0.w, r1.y, s#0, t#6"),
   INSTR_6XX(a048d107_e0080a07, "isaml.base3 (s32)(x)r1.w, r0.w, r1.y, s#0, a1.x"),
   INSTR_6XX(a1481606_e4803035, "saml.base0 (f32)(yz)r1.z, r6.z, r6.x, s#36, a1.x"),

   INSTR_7XX(a0081f02_e2000001, "isam.base0 (f32)(xyzw)r0.z, r0.x, t#16, a1.x"),
   INSTR_7XX(a148310d_e028302c, "saml.base2 (u32)(x)r3.y, hr5.z, hr6.x, t#1, a1.x"),

   /* dEQP-VK.subgroups.arithmetic.compute.subgroupadd_float */
   INSTR_6XX(a7c03102_00100003, "brcst.active.w8 (u32)(x)r0.z, r0.y"), /* brcst.active.w8 (u32)(xOOO)r0.z, r0.y */
   /* dEQP-VK.subgroups.quad.graphics.subgroupquadbroadcast_int */
   INSTR_6XX(b7e03107_00000401, "(sy)quad_shuffle.brcst (u32)(x)r1.w, r0.x, r0.z"), /* (sy)quad_shuffle.brcst (u32)(xOOO)r1.w, r0.x, r0.z */
   /* dEQP-VK.subgroups.quad.graphics.subgroupquadswapdiagonal_int */
   INSTR_6XX(b7e03104_00180001, "(sy)quad_shuffle.diag (u32)(x)r1.x, r0.x"), /* (sy)quad_shuffle.diag (u32)(xOOO)r1.x, r0.x */

   INSTR_6XX(a7000000_00000000, "tcinv"),

   /* cat6 */

   INSTR_5XX(c6e60000_00010600, "ldgb.untyped.4d.u32.1 r0.x, g[0], r1.x, r0.x"), /* ldgb.a.untyped.1dtype.u32.1 r0.x, g[r1.x], r0.x, 0 */
   INSTR_5XX(d7660204_02000a01, "(sy)stib.typed.2d.u32.1 g[1], r0.x, r0.z, r1.x"), /* (sy)stib.a.u32.2d.1 g[r1.x], r0.x, r0.z, 1.  r1.x is offset in ibo, r0.x is value*/
   /* dEQP-VK.image.load_store.1d_array.r8g8b8a8_unorm */
   INSTR_5XX(c1a20006_0600ba01, "ldib.typed.2d.f32.4 r1.z, g[0], r0.z, r1.z"), /* ldib.a.f32.2d.4 r1.z, g[r0.z], r1.z, 0.  r0.z is offset in ibo as src.  r1.z */
   /* dEQP-VK.image.load_store.3d.r32g32b32a32_sint */
   INSTR_5XX(c1aa0003_0500fc01, "ldib.typed.3d.s32.4 r0.w, g[0], r0.w, r1.y"), /* ldib.a.s32.3d.4 r0.w, g[r0.w], r1.y, 0.  r0.w is offset in ibo as src, and dst */
   /* dEQP-VK.binding_model.shader_access.primary_cmd_buf.storage_image.vertex.descriptor_array.3d */
   INSTR_5XX(c1a20204_0401fc01, "ldib.typed.3d.f32.4 r1.x, g[1], r1.w, r1.x"), /* ldib.a.f32.3d.4 r1.x, g[r1.w], r1.x, 1 */
   /* dEQP-VK.binding_model.shader_access.secondary_cmd_buf.with_push.storage_texel_buffer.vertex_fragment.single_descriptor.offset_zero */
   INSTR_5XX(c1a20005_0501be01, "ldib.typed.4d.f32.4 r1.y, g[0], r1.z, r1.y"), /* ldib.a.f32.1dtype.4 r1.y, g[r1.z], r1.y, 0 */
   /* dEQP-VK.texture.filtering.cube.formats.r8g8b8a8_snorm_nearest */
   INSTR_5XX(c1a60200_0000ba01, "ldib.typed.2d.u32.4 r0.x, g[1], r0.z, r0.x"), /* ldib.a.u32.2d.4 r0.x, g[r0.z], r0.x, 1 */

   // TODO is this a real instruction?  Or float -6.0 ?
   // INSTR_6XX(c0c00000_00000000, "stg.f16 g[hr0.x], hr0.x, hr0.x", .parse_fail=true),
   /* dEQP-GLES31.functional.tessellation.invariance.outer_edge_symmetry.isolines_equal_spacing_ccw */
   INSTR_6XX(c0d20906_02800004, "stg.a.f32 g[r1.x+(r1.z)<<2], r0.z, 2"), /* stg.a.f32 g[r1.x+(r1.z<<2)], r0.z, 2 */
   INSTR_6XX(c0da052e_01800042, "stg.a.s32 g[r0.z+(r11.z)<<2], r8.y, 1"), /* stg.a.s32 g[r0.z+(r11.z<<2)], r8.y, 1 */
   INSTR_6XX(c0dc052e_01800042, "stg.a.u8 g[r0.z+(r11.z)<<2], hr8.y, 1"),
   INSTR_6XX(c0ca0505_03800042, "stg.s32 g[r0.z+5], r8.y, 3"),
   INSTR_6XX(c0ca0500_03800042, "stg.s32 g[r0.z], r8.y, 3"),
   INSTR_6XX(c0ca0531_03800242, "stg.s32 g[r0.z+305], r8.y, 3"),
   INSTR_5XX(c0ce0100_02800000, "stg.s8 g[r0.x], hr0.x, 2"),
   INSTR_5XX(c0c00100_02800000, "stg.f16 g[r0.x], hr0.x, 2"),

   /* Customely crafted */
   INSTR_6XX(c0d61104_01800228, "stg.a.u32 g[r2.x+(r1.x+1)<<2], r5.x, 1"),
   INSTR_6XX(c0d61104_01802628, "stg.a.u32 g[r2.x+r1.x<<4+3<<2], r5.x, 1"),

   INSTR_7XX(c0d20505_07bfc006, "stg.a.f32 g[r0.z+r1.y+255], r0.w, 7"),
   INSTR_7XX(c0d20507_04812006, "stg.a.f32 g[c0.z+r1.w+4], r0.w, 4"),

   INSTR_6XX(c0020011_04c08023, "ldg.a.f32 r4.y, g[r0.z+(r4.y)<<2], 4"), /* ldg.a.f32 r4.y, g[r0.z+(r4.y<<2)], 4 */
   INSTR_6XX(c0060006_01c18017, "ldg.a.u32 r1.z, g[r1.z+(r2.w)<<2], 1"), /* ldg.a.u32 r1.z, g[r1.z+(r2.w<<2)], 1 */
   INSTR_6XX(c0060006_0181800f, "ldg.u32 r1.z, g[r1.z+7], 1"),
   INSTR_6XX(c0060006_01818001, "ldg.u32 r1.z, g[r1.z], 1"),
   INSTR_6XX(c0060003_0180c269, "ldg.u32 r0.w, g[r0.w+308], 1"),
   INSTR_6XX(c0040003_0180c269, "ldg.u16 hr0.w, g[r0.w+308], 1"),

   /* Found in TCS/TES shaders of GTA V */
   INSTR_6XX(c0020007_03c1420f, "ldg.a.f32 r1.w, g[r1.y+(r1.w+1)<<2], 3"), /* ldg.a.f32 r1.w, g[r1.y+((r1.w+1)<<2)], 3 */

   /* Customely crafted */
   INSTR_6XX(c0020007_03c1740f, "ldg.a.f32 r1.w, g[r1.y+r1.w<<5+2<<2], 3"),

   INSTR_6XX(c0020011_04c08023, "ldg.a.f32 r4.y, g[r0.z+(r4.y)<<2], 4"), /* ldg.a.f32 r4.y, g[r0.z+(r4.y<<2)], 4 */
   INSTR_6XX(c0060006_01c18017, "ldg.a.u32 r1.z, g[r1.z+(r2.w)<<2], 1"), /* ldg.a.u32 r1.z, g[r1.z+(r2.w<<2)], 1 */
   INSTR_6XX(c0000006_01c18017, "ldg.a.f16 hr1.z, g[r1.z+(r2.w)<<2], 1"),
   INSTR_6XX(c0060006_0181800f, "ldg.u32 r1.z, g[r1.z+7], 1"),
   INSTR_6XX(c0060006_01818001, "ldg.u32 r1.z, g[r1.z], 1"),

   INSTR_7XX(c0020411_04c08023, "ldg.a.f32 r4.y, g[r0.z+r4.y+2], 4"),
   INSTR_7XX(c0004006_01c1a017, "ldg.a.f16 hr1.z, g[c1.z+r2.w+32], 1"),

   /* dEQP-GLES3.functional.ubo.random.basic_arrays.0 */
   INSTR_6XX(c7020020_01800000, "stc.f32 c[32], r0.x, 1"), /* stc c[32], r0.x, 1 */
   /* dEQP-VK.image.image_size.cube_array.readonly_writeonly_1x1x12 */
   INSTR_6XX(c7060020_03800000, "stc.u32 c[32], r0.x, 3"), /* stc c[32], r0.x, 3 */
   /* A660 EQP-VK.robustness.robustness2.push.notemplate.r32i.unroll.nonvolatile.sampled_image.no_fmt_qual.img.samples_1.1d.frag */
   /* TODO: stc has a similar to stsc DST range */
   /* INSTR_6XX(c702026e_0480025c, "stc.u32 c[366], r11.z, 4"), */ /* stc c[366], r11.z, 4 */

   /* dEQP-VK.pipeline.monolithic.extended_dynamic_state.two_draws_static.stencil_state_face_both_single_gt_replace_clear_102_ref_103_depthfail */
   INSTR_7XX(c7420000_0cc00000, "stsc.f32 c[0], 0, 12"),
   /* dEQP-VK.pipeline.monolithic.push_constant.graphics_pipeline.overlap_4_shaders_vert_tess_frag */
   INSTR_7XX(c7420000_08c00020, "stsc.f32 c[0], 16, 8"),
   INSTR_7XX(c742006e_08c00220, "stsc.f32 c[366], 16, 8"),

   /* custom */
   INSTR_6XX(c7060100_03800000, "stc.u32 c[a1.x], r0.x, 3"), /* stc c[a1.x], r0.x, 3 */
   INSTR_6XX(c7060120_03800000, "stc.u32 c[a1.x+32], r0.x, 3"), /* stc c[a1.x+32], r0.x, 3 */

   /* dEQP-VK.image.image_size.cube_array.readonly_writeonly_1x1x12 */
   INSTR_6XX(c0260200_03676100, "stib.b.untyped.1d.u32.3.imm.base0 r0.x, r0.w, 1"), /* stib.untyped.u32.1d.3.mode4.base0 r0.x, r0.w, 1 */

   INSTR_6XX(c0240402_00674100, "stib.b.untyped.1d.u16.1.imm.base0 hr0.z, r0.x, 2"),
#if 0
   /* TODO blob sometimes/frequently sets b0, although there does not seem
    * to be an obvious pattern and our encoding never sets it.  AFAICT it
    * is a dontcare bit
    */
   /* dEQP-VK.texture.filtering.cube.formats.a8b8g8r8_srgb_nearest_mipmap_nearest.txt */
   INSTR_6XX(c0220200_0361b801, "ldib.b.typed.1d.f32.4.imm r0.x, r0.w, 1"), /* ldib.f32.1d.4.mode0.base0 r0.x, r0.w, 1 */
#else
   /* dEQP-VK.texture.filtering.cube.formats.a8b8g8r8_srgb_nearest_mipmap_nearest.txt */
   INSTR_6XX(c0220200_0361b800, "ldib.b.typed.1d.f32.4.imm r0.x, r0.w, 1"), /* ldib.f32.1d.4.mode0.base0 r0.x, r0.w, 1 */
#endif

   /* dEQP-GLES31.functional.tessellation.invariance.outer_edge_symmetry.isolines_equal_spacing_ccw */
   INSTR_6XX(c2c21100_04800006, "stlw.f32 l[r2.x], r0.w, 4"),
   INSTR_6XX(c2c20f00_01800004, "stlw.f32 l[r1.w], r0.z, 1"),
   INSTR_6XX(c2860003_02808011, "ldlw.u32 r0.w, l[r0.z+8], 2"),

   /* dEQP-VK.compute.basic.shared_var_single_group */
   INSTR_6XX(c1060500_01800008, "stl.u32 l[r0.z], r1.x, 1"),
   INSTR_6XX(c0460001_01804001, "ldl.u32 r0.y, l[r0.y], 1"),

   INSTR_6XX(c0860018_03820001, "ldp.u32 r6.x, p[r2.x], 3"),
   INSTR_6XX(c0420002_01808019, "ldl.f32 r0.z, l[r0.z+12], 1"),
   INSTR_6XX(c1021710_04800000, "stl.f32 l[r2.w+16], r0.x, 4"),
   INSTR_6XX(d7c60011_03c00000, "(sy)ldlv.u32 r4.y, l[0], 3"),

   /* resinfo */
   INSTR_6XX(c0260000_0063c200, "resinfo.b.untyped.2d.u32.1.imm r0.x, 0"), /* resinfo.u32.2d.mode0.base0 r0.x, 0 */
   /* dEQP-GLES31.functional.image_load_store.buffer.image_size.writeonly_7.txt */
   INSTR_6XX(c0260000_0063c000, "resinfo.b.untyped.1d.u32.1.imm r0.x, 0"), /* resinfo.u32.1d.mode0.base0 r0.x, 0 */
   /* dEQP-VK.image.image_size.2d.readonly_12x34.txt */
   INSTR_6XX(c0260000_0063c300, "resinfo.b.untyped.2d.u32.1.imm.base0 r0.x, 0"), /* resinfo.u32.2d.mode4.base0 r0.x, 0 */
   /* Custom test */
   INSTR_6XX(c0260000_0063c382, "resinfo.b.untyped.2d.u32.1.nonuniform.base1 r0.x, r0.x"), /* resinfo.u32.2d.mode6.base1 r0.x, r0.x */

   /* dEQP-GLES31.functional.image_load_store.2d.image_size.readonly_writeonly_32x32.txt */
   INSTR_5XX(c3e60000_00000200, "resinfo.u32.2d r0.x, g[0]"), /* resinfo.u32.2d r0.x, 0 */
#if 0
   /* TODO our encoding differs in b11 ('typed'), which seems to be a dontcare bit */
   /* dEQP-GLES31.functional.image_load_store.buffer.image_size.readonly_writeonly_7 */
   INSTR_5XX(c3e60000_00000e00, "resinfo.u32.4d r0.x, g[0]"), /* resinfo.u32.1dtype r0.x, 0 */
   /* dEQP-GLES31.functional.image_load_store.3d.image_size.readonly_writeonly_12x34x56 */
   INSTR_5XX(c3e60000_00000c00, "resinfo.u32.3d r0.x, g[0]"), /* resinfo.u32.3d r0.x, 0 */
#else
   /* dEQP-GLES31.functional.image_load_store.buffer.image_size.readonly_writeonly_7 */
   INSTR_5XX(c3e60000_00000600, "resinfo.u32.4d r0.x, g[0]"), /* resinfo.u32.1dtype r0.x, 0 */
   /* dEQP-GLES31.functional.image_load_store.2d.image_size.readonly_writeonly_32x32.txt */
   INSTR_5XX(c3e60000_00000400, "resinfo.u32.3d r0.x, g[0]"), /* resinfo.u32.3d r0.x, 0 */
#endif

   /* ldgb */
   /* dEQP-GLES31.functional.ssbo.layout.single_basic_type.packed.mediump_vec4 */
   INSTR_5XX(c6e20000_06003600, "ldgb.untyped.4d.f32.4 r0.x, g[0], r0.x, r1.z"), /* ldgb.a.untyped.1dtype.f32.4 r0.x, g[r0.x], r1.z, 0 */
   /* dEQP-GLES31.functional.ssbo.layout.single_basic_type.packed.mediump_ivec4 */
   INSTR_5XX(c6ea0000_06003600, "ldgb.untyped.4d.s32.4 r0.x, g[0], r0.x, r1.z"), /* ldgb.a.untyped.1dtype.s32.4 r0.x, g[r0.x], r1.z, 0 */
   /* dEQP-GLES31.functional.ssbo.layout.single_basic_type.packed.mediump_float */
   INSTR_5XX(c6e20000_02000600, "ldgb.untyped.4d.f32.1 r0.x, g[0], r0.x, r0.z"), /* ldgb.a.untyped.1dtype.f32.1 r0.x, g[r0.x], r0.z, 0 */
   /* dEQP-GLES31.functional.ssbo.layout.random.vector_types.0 */
   INSTR_5XX(c6ea0008_14002600, "ldgb.untyped.4d.s32.3 r2.x, g[0], r0.x, r5.x"), /* ldgb.a.untyped.1dtype.s32.3 r2.x, g[r0.x], r5.x, 0 */
   INSTR_5XX(c6ea0204_1401a600, "ldgb.untyped.4d.s32.3 r1.x, g[1], r1.z, r5.x"), /* ldgb.a.untyped.1dtype.s32.3 r1.x, g[r1.z], r5.x, 1 */

   /* stgb */
   INSTR_5XX(c7220028_0480000d, "stgb.untyped.1d.f32.1 g[0], r1.z, 4, r10.x"), /* stgb.untyped.1d.1 g[r10.x], r1.z, 4, r0.x */
   INSTR_5XX(c7260023_02800009, "stgb.untyped.1d.u32.1 g[0], r1.x, 2, r8.w"),  /* stgb.untyped.1d.1 g[r8.w], r1.x, 2, r0.x */

   /* discard stuff */
   INSTR_6XX(42b400f8_20010004, "cmps.s.eq p0.x, r1.x, 1"),
   INSTR_6XX(02800000_00000000, "kill p0.x"),

   /* Immediates */
   INSTR_6XX(40100007_68000008, "add.f r1.w, r2.x, (neg)(0.0)"),
   INSTR_6XX(40100007_68010008, "add.f r1.w, r2.x, (neg)(0.5)"),
   INSTR_6XX(40100007_68020008, "add.f r1.w, r2.x, (neg)(1.0)"),
   INSTR_6XX(40100007_68030008, "add.f r1.w, r2.x, (neg)(2.0)"),
   INSTR_6XX(40100007_68040008, "add.f r1.w, r2.x, (neg)(e)"),
   INSTR_6XX(40100007_68050008, "add.f r1.w, r2.x, (neg)(pi)"),
   INSTR_6XX(40100007_68060008, "add.f r1.w, r2.x, (neg)(1/pi)"),
   INSTR_6XX(40100007_68070008, "add.f r1.w, r2.x, (neg)(1/log2(e))"),
   INSTR_6XX(40100007_68080008, "add.f r1.w, r2.x, (neg)(log2(e))"),
   INSTR_6XX(40100007_68090008, "add.f r1.w, r2.x, (neg)(1/log2(10))"),
   INSTR_6XX(40100007_680a0008, "add.f r1.w, r2.x, (neg)(log2(10))"),
   INSTR_6XX(40100007_680b0008, "add.f r1.w, r2.x, (neg)(4.0)"),

   /* LDC.  Our disasm differs greatly from qcom here, and we've got some
    * important info they lack(?!), but same goes the other way.
    */
   /* dEQP-GLES31.functional.shaders.opaque_type_indexing.ubo.uniform_fragment */
   INSTR_6XX(c0260000_00c78040, "ldc.offset0.1.uniform r0.x, 0, r0.x"), /* ldc.1.mode1.base0 r0.x, 0, r0.x */
   INSTR_6XX(c0260201_00c78040, "ldc.offset0.1.uniform r0.y, 0, r0.y"), /* ldc.1.mode1.base0 r0.y, 0, r0.y */
   /* dEQP-GLES31.functional.shaders.opaque_type_indexing.ubo.dynamically_uniform_fragment  */
   INSTR_6XX(c0260000_00c78080, "ldc.offset0.1.nonuniform r0.x, 0, r0.x"), /* ldc.1.mode2.base0 r0.x, 0, r0.x */
   INSTR_6XX(c0260201_00c78080, "ldc.offset0.1.nonuniform r0.y, 0, r0.y"), /* ldc.1.mode2.base0 r0.y, 0, r0.y */

   /* a4xx-a5xx has the exact same instrs in
    * dEQP-GLES31.functional.shaders.opaque_type_indexing.ubo.(dynamically_)uniform_fragment
    * with no change based on the mode. Note that we can't decode this yet.
    */
   /* INSTR_4XX(c7860000_00810001), */ /* ldc.1 r0.x, g[r1.x], 0, r0.x */
   /* INSTR_5XX(c7860000_00800000), */ /* ldc.a.1 r0.x, g[r0.x], 0, r0.x */

   /* custom */
   INSTR_6XX(c0260201_ffc78080, "ldc.offset0.1.nonuniform r0.y, 255, r0.y"), /* ldc.1.mode2.base0 r0.y, 255, r0.y */

   /* custom shaders, loading .x, .y, .z, .w from an array of vec4 in block 0 */
   INSTR_6XX(c0260000_00478000, "ldc.offset0.1.imm r0.x, r0.x, 0"), /* ldc.1.mode0.base0 r0.x, r0.x, 0 */
   INSTR_6XX(c0260000_00478200, "ldc.offset1.1.imm r0.x, r0.x, 0"), /* ldc.1.mode0.base0 r0.x, r0.x, 0 */
   INSTR_6XX(c0260000_00478400, "ldc.offset2.1.imm r0.x, r0.x, 0"), /* ldc.1.mode0.base0 r0.x, r0.x, 0 */
   INSTR_6XX(c0260000_00478600, "ldc.offset3.1.imm r0.x, r0.x, 0"), /* ldc.1.mode0.base0 r0.x, r0.x, 0 */

   /* dEQP-VK.glsl.conditionals.if.if_else_vertex */
   INSTR_6XX(c0360000_00c78100, "ldc.1.k.imm.base0 c[a1.x], 0, 0"), /* ldc.1.k.mode4.base0 c[a1.x], 0, 0 */
   /* custom */
   INSTR_6XX(c0360003_00c78100, "ldc.4.k.imm.base0 c[a1.x], 0, 0"), /* ldc.4.k.mode4.base0 c[a1.x], 0, 0 */

   /* dEQP-VK.glsl.struct.local.nested_struct_array_dynamic_index_fragment */
   INSTR_6XX(c1425b50_01803e02, "stp.f32 p[r11.y-176], r0.y, 1"),
   INSTR_6XX(c1425b98_02803e14, "stp.f32 p[r11.y-104], r2.z, 2"),
   INSTR_6XX(c1465ba0_01803e2a, "stp.u32 p[r11.y-96], r5.y, 1"),
   INSTR_6XX(c0860008_01860001, "ldp.u32 r2.x, p[r6.x], 1"),
   /* Custom stp based on above to catch a disasm bug. */
   INSTR_6XX(c1465b00_0180022a, "stp.u32 p[r11.y+256], r5.y, 1"),

   /* Atomic: */
#if 0
   /* TODO our encoding differs in b53 for these two */
   INSTR_5XX(c4f60002_00008001, "atomic.s.inc.untyped.1d.u32.1.g r0.z, g[0], r0.z, r0.x, r0.x"),
   INSTR_5XX(c4360205_03000001, "atomic.s.add.untyped.1d.u32.1.g r1.y, g[1], r0.x, r0.w, r0.x"),
#else
   INSTR_5XX(c4f60002_00008001, "atomic.s.inc.untyped.1d.u32.1.g r0.z, g[0], r0.z, r0.x, r0.x"),
   INSTR_5XX(c4360205_03000001, "atomic.s.add.untyped.1d.u32.1.g r1.y, g[1], r0.x, r0.w, r0.x"),
#endif
   INSTR_6XX(d5c60003_03008001, "(sy)atomic.max.untyped.1d.u32.1.l r0.w, l[r0.z], r0.w"),

   /* dEQP-VK.glsl.atomic_operations.add_unsigned_compute_reference */
   INSTR_6XX(c4160002_02000001, "atomic.g.add.untyped.1d.u32.1.g r0.z, r0.x, r0.z"),

   /* Bindless atomic: */
   INSTR_6XX(c03a0003_01640000, "atomic.b.add.untyped.1d.s32.1.imm r0.w, r0.y, 0"), /* atomic.b.add.g.s32.1d.mode0.base0 r0.w,r0.y,0 */
   INSTR_6XX(c03a0003_01660000, "atomic.b.and.untyped.1d.s32.1.imm r0.w, r0.y, 0"), /* atomic.b.and.g.s32.1d.mode0.base0 r0.w,r0.y,0 */
   INSTR_6XX(c0360000_0365c800, "atomic.b.max.typed.1d.u32.1.imm r0.x, r0.w, 0"),   /* atomic.b.max.g.u32.1d.mode0.base0 r0.x,r0.w,0 */

   /* dEQP-GLES31.functional.shaders.opaque_type_indexing.sampler.const_literal.fragment.sampler2d */
   INSTR_6XX(a0c01f04_0cc00005, "sam (f32)(xyzw)r1.x, r0.z, s#6, t#6"),

   /* dEQP-GLES31.functional.shaders.opaque_type_indexing.sampler.uniform.fragment.sampler2d */
   INSTR_4XX(a0c81f02_00800001, "sam.s2en.uniform (f32)(xyzw)r0.z, r0.x, hr1.x"), /* sam.s2en.mode0 (f32)(xyzw)r0.z, r0.x, hr1.x */ /* same for 5xx */
   INSTR_6XX(a0c81f07_0100000b, "sam.s2en.uniform (f32)(xyzw)r1.w, r1.y, hr2.x"), /* sam.s2en.mode0 (f32)(xyzw)r1.w, r1.y, hr2.x */

   /* dEQP-GLES31.functional.shaders.opaque_type_indexing.sampler.dynamically_uniform.fragment.sampler2d */
   INSTR_4XX(a0c81f02_80800001, "sam.s2en.nonuniform (f32)(xyzw)r0.z, r0.x, hr1.x"), /* sam.s2en.uniform (f32)(xyzw)r0.z, r0.x, hr1.x */ /* same for 5xx */
   INSTR_6XX(a0c81f07_8100000b, "sam.s2en.nonuniform (f32)(xyzw)r1.w, r1.y, hr2.x"), /* sam.s2en.mode4 (f32)(xyzw)r1.w, r1.y, hr2.x */

   /* NonUniform: */
   /* dEQP-VK.descriptor_indexing.storage_buffer */
   INSTR_6XX(c0260c0a_0a61b180, "ldib.b.untyped.1d.u32.4.nonuniform.base0 r2.z, r2.z, r1.z"),
   INSTR_6XX(d0260e0a_09677180, "(sy)stib.b.untyped.1d.u32.4.nonuniform.base0 r2.z, r2.y, r1.w"),
   /* dEQP-VK.descriptor_indexing.uniform_texel_buffer */
   INSTR_6XX(a0481f00_40000405, "isaml.s2en.nonuniform.base0 (f32)(xyzw)r0.x, r0.z, r0.z, r0.x"),
   /* dEQP-VK.descriptor_indexing.storage_image */
   INSTR_6XX(d0360c04_02640b80, "(sy)atomic.b.add.typed.2d.u32.1.nonuniform.base0 r1.x, r0.z, r1.z"),
   /* dEQP-VK.descriptor_indexing.sampler */
   INSTR_6XX(a0c81f00_40000005, "sam.s2en.nonuniform.base0 (f32)(xyzw)r0.x, r0.z, r0.x"),

   /* dEQP-VK.subgroups.quad.graphics.subgroupquadbroadcast_int */
   INSTR_6XX(c0260001_00c98000, "getfiberid.u32 r0.y"),

   /* Custom test since we've never seen the blob emit these. */
   INSTR_6XX(c0260004_00490000, "getspid.u32 r1.x"),
   INSTR_6XX(c0260005_00494000, "getwid.u32 r1.y"),

   /* cat7 */

   /* dEQP-VK.compute.basic.ssbo_local_barrier_single_invocation */
   INSTR_6XX(e0fa0000_00000000, "fence.g.l.r.w"),
   INSTR_6XX(e09a0000_00000000, "fence.r.w"),
   INSTR_6XX(f0420000_00000000, "(sy)bar.g"),
   INSTR_6XX(e1080000_00000000, "sleep.l"),
   INSTR_6XX(e2080000_00000000, "dccln.all"),
   /* dEQP-VK.memory_model.message_passing.core11.u32.coherent.fence_fence.atomicwrite.device.payload_local.buffer.guard_local.buffer.comp */
   INSTR_7XX(e2d20000_00000000, "ccinv"),

   INSTR_7XX(e3c20000_00000000, "lock"),
   INSTR_7XX(fbc21000_00000000, "(sy)(ss)(jp)lock"),

   /* dEQP-VK.pipeline.monolithic.sampler.border_swizzle.r4g4b4a4_unorm_pack16.rg1a.opaque_white.gather_1.no_swizzle_hint */
   INSTR_7XX(e45401a0_bfba7736, "alias.tex.b32.1 r40.x, (-1.456763)"),
   /* dEQP-VK.synchronization.op.single_queue.event.write_draw_indexed_read_image_geometry.image_128x128_r32g32b32a32_sfloat */
   INSTR_7XX(e44c0009_00000007, "alias.tex.b32.0 r2.y, c1.w"),

   INSTR_6XX(ffffffff_ffffffff, "raw 0xFFFFFFFFFFFFFFFF"),
   /* clang-format on */
};

static void
trim(char *string)
{
   for (int len = strlen(string); len > 0 && string[len - 1] == '\n'; len--)
      string[len - 1] = 0;
}

int
main(int argc, char **argv)
{
   int retval = 0;
   int decode_fails = 0, asm_fails = 0, encode_fails = 0;
   const int output_size = 4096;
   char *disasm_output = malloc(output_size);
   FILE *fdisasm = fmemopen(disasm_output, output_size, "w+");
   if (!fdisasm) {
      fprintf(stderr, "failed to fmemopen\n");
      return 1;
   }

   struct ir3_compiler *compilers[10] = {};
   struct fd_dev_id dev_ids[ARRAY_SIZE(compilers)];

   for (int i = 0; i < ARRAY_SIZE(tests); i++) {
      const struct test *test = &tests[i];
      printf("Testing a%d %s: \"%s\"...\n", test->gpu_id, test->instr,
             test->expected);

      struct fd_dev_id dev_id = {
         .gpu_id = test->gpu_id,
         .chip_id = test->chip_id,
      };

      const struct fd_dev_info *dev_info = fd_dev_info_raw(&dev_id);

      rewind(fdisasm);
      memset(disasm_output, 0, output_size);

      /*
       * Test disassembly:
       */

      uint32_t code[2] = {
         strtoll(&test->instr[9], NULL, 16),
         strtoll(&test->instr[0], NULL, 16),
      };
      isa_disasm(code, 8, fdisasm,
                 &(struct isa_decode_options){
                    .gpu_id = dev_info->chip * 100,
                    .show_errors = true,
                    .no_match_cb = print_raw,
                 });
      fflush(fdisasm);

      trim(disasm_output);

      if (strcmp(disasm_output, test->expected) != 0) {
         printf("FAIL: disasm\n");
         printf("  Expected: \"%s\"\n", test->expected);
         printf("  Got:      \"%s\"\n", disasm_output);
         retval = 1;
         decode_fails++;
      }

      /*
       * Test assembly, which should result in the identical binary:
       */

      if (!compilers[dev_info->chip]) {
         dev_ids[dev_info->chip].gpu_id = test->gpu_id;
         dev_ids[dev_info->chip].chip_id = test->chip_id;
         compilers[dev_info->chip] =
            ir3_compiler_create(NULL, &dev_ids[dev_info->chip],
                                fd_dev_info_raw(&dev_ids[dev_info->chip]),
                                &(struct ir3_compiler_options){});
      }

      FILE *fasm =
         fmemopen((void *)test->expected, strlen(test->expected), "r");

      struct ir3_kernel_info info = {};
      struct ir3_shader *shader = ir3_parse_asm(compilers[dev_info->chip], &info, fasm);
      fclose(fasm);
      if (!shader) {
         printf("FAIL: %sexpected assembler fail\n",
                test->parse_fail ? "" : "un");
         asm_fails++;
         /* If this is an instruction that the asm parser is not expected
          * to handle, don't count it as a fail.
          */
         if (!test->parse_fail)
            retval = 1;
         continue;
      } else if (test->parse_fail) {
         /* If asm parse starts passing, and we don't expect that, flag
          * it as a fail so we don't forget to update the test vector:
          */
         printf(
            "FAIL: unexpected parse success, please remove '.parse_fail=true'\n");
         retval = 1;
      }

      struct ir3_shader_variant *v = shader->variants;
      if (memcmp(v->bin, code, sizeof(code))) {
         printf("FAIL: assembler\n");
         printf("  Expected: %08x_%08x\n", code[1], code[0]);
         printf("  Got:      %08x_%08x\n", v->bin[1], v->bin[0]);
         retval = 1;
         encode_fails++;
      }

      ir3_shader_destroy(shader);
   }

   if (decode_fails)
      printf("%d/%d decode fails\n", decode_fails, (int)ARRAY_SIZE(tests));
   if (asm_fails)
      printf("%d/%d assembler fails\n", asm_fails, (int)ARRAY_SIZE(tests));
   if (encode_fails)
      printf("%d/%d encode fails\n", encode_fails, (int)ARRAY_SIZE(tests));

   if (retval) {
      printf("FAILED!\n");
   } else {
      printf("PASSED!\n");
   }

   for (unsigned i = 0; i < ARRAY_SIZE(compilers); i++) {
      if (!compilers[i])
         continue;
      ir3_compiler_destroy(compilers[i]);
   }

   fclose(fdisasm);
   free(disasm_output);

   return retval;
}
