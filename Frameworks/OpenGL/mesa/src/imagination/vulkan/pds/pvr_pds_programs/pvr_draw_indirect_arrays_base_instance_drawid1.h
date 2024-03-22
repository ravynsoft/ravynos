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

#ifndef PVR_DRAW_INDIRECTARRAYS_BASE_INSTANCE_DRAWID1_H
#define PVR_DRAW_INDIRECTARRAYS_BASE_INSTANCE_DRAWID1_H

/* Initially generated from ARB_draw_indirect_arrays.pds */

static const uint32_t pvr_draw_indirect_arrays_base_instance_drawid1_code[18] = {
   0xd0000000, /* LD              const[0].64: dst(?) <= mem(?) */
   0xd1000000, /* WDF              */
   0xc8000001, /* BRA             if keep 1 ( setc = p0 ) */
   0xd19c0000, /* LIMM            temp[7].32 = 0000 */
   0x50141006, /* SFTLP32         temp[6].32 = temp[2].32 << 0 */
   0xb18c0000, /* CMP             P0 = (temp[6].64 = 0000) */
   0xd9840000, /* LIMM            ? temp[1].32 = 0000 */
   0xd9880000, /* LIMM            ? temp[2].32 = 0000 */
   0x04101024, /* MAD             temp[8].64 = (temp[2].32 * const[2].32) +
                                               const[4].64 */
   0x50444002, /* SFTLP32         temp[2].32 = temp[8].32 << 0 */
   0x912080c2, /* ADD32           temp[2].32 = temp[2].32 - const[3].32  */
   0x9001a0e0, /* ADD32           ptemp[0].32 = const[6].32 + temp[3].32  */
   0xd0800004, /* ST              const[8].64: mem(?) <= src(?) */
   0x9001a121, /* ADD32           ptemp[1].32 = const[6].32 + temp[4].32  */
   0x9030c0e3, /* ADD32           ptemp[3].32 = ptemp[3].32 + const[3].32  */
   0xd0000005, /* LD              const[10].64: dst(?) <= mem(?) */
   0xd1000000, /* WDF              */
   0xf4064003, /* DOUT            doutv = temp[0].64, const[6].32; HALT */
};

static const struct pvr_psc_program_output
   pvr_draw_indirect_arrays_base_instance_drawid1_program = {
      pvr_draw_indirect_arrays_base_instance_drawid1_code, /* code segment */
      0, /* constant mappings, zeroed since we use the macros below */
      4, /* number of constant mappings */

      12, /* size of data segment, in dwords, aligned to 4 */
      20, /* size of code segment, in dwords, aligned to 4 */
      12, /* size of temp segment, in dwords, aligned to 4 */
      12, /* size of data segment, in dwords */
      18, /* size of code segment, in dwords */
      12, /* size of temp segment, in dwords */
      NULL /* function pointer to write data segment */
   };

#define pvr_write_draw_indirect_arrays_base_instance_drawid1_di_data(buffer, \
                                                                     addr,   \
                                                                     device) \
   do {                                                                      \
      uint64_t data = ((addr) | (0x60000000000ULL) |                         \
                       ENABLE_SLC_MCU_CACHE_CONTROLS(device));               \
      PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ARRAYS", data, 0);                   \
      memcpy(buffer + 0, &data, sizeof(data));                               \
   } while (0)
#define pvr_write_draw_indirect_arrays_base_instance_drawid1_write_vdm(buffer, \
                                                                       addr)   \
   do {                                                                        \
      uint64_t data = ((addr) | (0x430000000000ULL));                          \
      PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ARRAYS", data, 0);                     \
      memcpy(buffer + 8, &data, sizeof(data));                                 \
   } while (0)
#define pvr_write_draw_indirect_arrays_base_instance_drawid1_flush_vdm(buffer, \
                                                                       addr)   \
   do {                                                                        \
      uint64_t data = ((addr) | (0x2140000000000ULL));                         \
      PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ARRAYS", data, 0);                     \
      memcpy(buffer + 10, &data, sizeof(data));                                \
   } while (0)
#define pvr_write_draw_indirect_arrays_base_instance_drawid1_num_views(buffer, \
                                                                       value)  \
   do {                                                                        \
      uint32_t data = value;                                                   \
      PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ARRAYS", data, 0);                     \
      memcpy(buffer + 2, &data, sizeof(data));                                 \
   } while (0)
#define pvr_write_draw_indirect_arrays_base_instance_drawid1_immediates( \
   buffer)                                                               \
   do {                                                                  \
      {                                                                  \
         uint64_t data = 0x0;                                            \
         PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ARRAYS", data, 0);            \
         memcpy(buffer + 4, &data, sizeof(data));                        \
      }                                                                  \
      {                                                                  \
         uint32_t data = 0x1;                                            \
         PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ARRAYS", data, 0);            \
         memcpy(buffer + 3, &data, sizeof(data));                        \
      }                                                                  \
      {                                                                  \
         uint32_t data = 0x0;                                            \
         PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ARRAYS", data, 0);            \
         memcpy(buffer + 6, &data, sizeof(data));                        \
      }                                                                  \
   } while (0)
#endif
