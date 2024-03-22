/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PVR_DRAW_INDIRECTARRAYS2_H
#define PVR_DRAW_INDIRECTARRAYS2_H

/* Initially generated from ARB_draw_indirect_arrays.pds */

static const uint32_t pvr_draw_indirect_arrays2_code[15] = {
   0xd0000000, /* LD              const[0].64: dst(?) <= mem(?) */
   0xd1000000, /* WDF              */
   0xc8000001, /* BRA             if keep 1 ( setc = p0 ) */
   0xd1840000, /* LIMM            temp[1].32 = 0000 */
   0x501c1800, /* SFTLP32         temp[0].32 = temp[3].32 << 0 */
   0xb1800000, /* CMP             P0 = (temp[0].64 = 0000) */
   0xd9880000, /* LIMM            ? temp[2].32 = 0000 */
   0xd98c0000, /* LIMM            ? temp[3].32 = 0000 */
   0x04181023, /* MAD             temp[6].64 = (temp[3].32 * const[2].32) +
                                               const[4].64 */
   0x50343003, /* SFTLP32         temp[3].32 = temp[6].32 << 0 */
   0x9120c0c3, /* ADD32           temp[3].32 = temp[3].32 - const[3].32  */
   0xd0800003, /* ST              const[6].64: mem(?) <= src(?) */
   0xd0000004, /* LD              const[8].64: dst(?) <= mem(?) */
   0xd1000000, /* WDF              */
   0xf40a4003, /* DOUT            doutv = temp[0].64, const[10].32; HALT */
};

static const struct pvr_psc_program_output pvr_draw_indirect_arrays2_program = {
   pvr_draw_indirect_arrays2_code, /* code segment */
   0, /* constant mappings, zeroed since we use the macros below */
   4, /* number of constant mappings */

   12, /* size of data segment, in dwords, aligned to 4 */
   16, /* size of code segment, in dwords, aligned to 4 */
   12, /* size of temp segment, in dwords, aligned to 4 */
   11, /* size of data segment, in dwords */
   15, /* size of code segment, in dwords */
   10, /* size of temp segment, in dwords */
   NULL /* function pointer to write data segment */
};

#define pvr_write_draw_indirect_arrays2_di_data(buffer, addr, device) \
   do {                                                               \
      uint64_t data = ((addr) | (0x60000000000ULL) |                  \
                       ENABLE_SLC_MCU_CACHE_CONTROLS(device));        \
      PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ARRAYS", data, 0);            \
      memcpy(buffer + 0, &data, sizeof(data));                        \
   } while (0)
#define pvr_write_draw_indirect_arrays2_write_vdm(buffer, addr) \
   do {                                                         \
      uint64_t data = ((addr) | (0x830000000000ULL));           \
      PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ARRAYS", data, 0);      \
      memcpy(buffer + 6, &data, sizeof(data));                  \
   } while (0)
#define pvr_write_draw_indirect_arrays2_flush_vdm(buffer, addr) \
   do {                                                         \
      uint64_t data = ((addr) | (0x1940000000000ULL));          \
      PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ARRAYS", data, 0);      \
      memcpy(buffer + 8, &data, sizeof(data));                  \
   } while (0)
#define pvr_write_draw_indirect_arrays2_num_views(buffer, value) \
   do {                                                          \
      uint32_t data = value;                                     \
      PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ARRAYS", data, 0);       \
      memcpy(buffer + 2, &data, sizeof(data));                   \
   } while (0)
#define pvr_write_draw_indirect_arrays2_immediates(buffer)    \
   do {                                                       \
      {                                                       \
         uint64_t data = 0x0;                                 \
         PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ARRAYS", data, 0); \
         memcpy(buffer + 4, &data, sizeof(data));             \
      }                                                       \
      {                                                       \
         uint32_t data = 0x1;                                 \
         PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ARRAYS", data, 0); \
         memcpy(buffer + 3, &data, sizeof(data));             \
      }                                                       \
      {                                                       \
         uint32_t data = 0x0;                                 \
         PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ARRAYS", data, 0); \
         memcpy(buffer + 10, &data, sizeof(data));            \
      }                                                       \
   } while (0)
#endif
