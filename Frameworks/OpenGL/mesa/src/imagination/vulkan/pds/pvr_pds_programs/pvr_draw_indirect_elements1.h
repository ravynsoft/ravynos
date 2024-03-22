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

#ifndef PVR_DRAW_INDIRECTELEMENTS1_H
#define PVR_DRAW_INDIRECTELEMENTS1_H

/* Initially generated from ARB_draw_indirect_elements.pds */

static const uint32_t pvr_draw_indirect_elements1_code[21] = {
   0xd0000000, /* LD              const[0].64: dst(?) <= mem(?) */
   0xd1000000, /* WDF              */
   0x9000a120, /* ADD32           ptemp[0].32 = const[2].32 + temp[4].32  */
   0x04181823, /* MAD             temp[6].64 = (temp[3].32 * const[3].32) +
                                               const[4].64 */
   0x53383008, /* SFTLP32         temp[8].32 = (temp[7].32 | const[6].32) << 0
                */
   0x50343009, /* SFTLP32         temp[9].32 = temp[6].32 << 0 */
   0x04103843, /* MAD             temp[6].64 = (temp[2].32 * const[7].32) +
                                               const[8].64 */
   0x50343002, /* SFTLP32         temp[2].32 = temp[6].32 << 0 */
   0x500c080a, /* SFTLP32         temp[10].32 = temp[1].32 << 0 */
   0x9120828b, /* ADD32           temp[11].32 = temp[2].32 - const[10].32  */
   0x5024200c, /* SFTLP32         temp[12].32 = temp[4].32 << 0 */
   0xc8000001, /* BRA             if keep 1 ( setc = p0 ) */
   0xd19c0000, /* LIMM            temp[7].32 = 0000 */
   0x50141006, /* SFTLP32         temp[6].32 = temp[2].32 << 0 */
   0xb18c0000, /* CMP             P0 = (temp[6].64 = 0000) */
   0xd9a80000, /* LIMM            ? temp[10].32 = 0000 */
   0xd9ac0000, /* LIMM            ? temp[11].32 = 0000 */
   0xd0800006, /* ST              const[12].64: mem(?) <= src(?) */
   0xd0000007, /* LD              const[14].64: dst(?) <= mem(?) */
   0xd1000000, /* WDF              */
   0xf4024003, /* DOUT            doutv = temp[0].64, const[2].32; HALT */
};

static const struct pvr_psc_program_output pvr_draw_indirect_elements1_program = {
   pvr_draw_indirect_elements1_code, /* code segment */
   0, /* constant mappings, zeroed since we use the macros below */
   7, /* number of constant mappings */

   16, /* size of data segment, in dwords, aligned to 4 */
   24, /* size of code segment, in dwords, aligned to 4 */
   20, /* size of temp segment, in dwords, aligned to 4 */
   16, /* size of data segment, in dwords */
   21, /* size of code segment, in dwords */
   20, /* size of temp segment, in dwords */
   NULL /* function pointer to write data segment */
};

#define pvr_write_draw_indirect_elements1_di_data(buffer, addr, device) \
   do {                                                                 \
      uint64_t data = ((addr) | (0x60000000000ULL) |                    \
                       ENABLE_SLC_MCU_CACHE_CONTROLS(device));          \
      PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ELEMENTS", data, 0);            \
      memcpy(buffer + 0, &data, sizeof(data));                          \
   } while (0)
#define pvr_write_draw_indirect_elements1_write_vdm(buffer, addr) \
   do {                                                           \
      uint64_t data = ((addr) | (0x2050000000000ULL));            \
      PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ELEMENTS", data, 0);      \
      memcpy(buffer + 12, &data, sizeof(data));                   \
   } while (0)
#define pvr_write_draw_indirect_elements1_flush_vdm(buffer, addr) \
   do {                                                           \
      uint64_t data = ((addr) | (0x3960000000000ULL));            \
      PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ELEMENTS", data, 0);      \
      memcpy(buffer + 14, &data, sizeof(data));                   \
   } while (0)
#define pvr_write_draw_indirect_elements1_idx_stride(buffer, value) \
   do {                                                             \
      uint32_t data = value;                                        \
      PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ELEMENTS", data, 0);        \
      memcpy(buffer + 3, &data, sizeof(data));                      \
   } while (0)
#define pvr_write_draw_indirect_elements1_idx_base(buffer, value) \
   do {                                                           \
      uint64_t data = value;                                      \
      PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ELEMENTS", data, 0);      \
      memcpy(buffer + 4, &data, sizeof(data));                    \
   } while (0)
#define pvr_write_draw_indirect_elements1_idx_header(buffer, value) \
   do {                                                             \
      uint32_t data = value;                                        \
      PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ELEMENTS", data, 0);        \
      memcpy(buffer + 6, &data, sizeof(data));                      \
   } while (0)
#define pvr_write_draw_indirect_elements1_num_views(buffer, value) \
   do {                                                            \
      uint32_t data = value;                                       \
      PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ELEMENTS", data, 0);       \
      memcpy(buffer + 7, &data, sizeof(data));                     \
   } while (0)
#define pvr_write_draw_indirect_elements1_immediates(buffer)    \
   do {                                                         \
      {                                                         \
         uint32_t data = 0x0;                                   \
         PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ELEMENTS", data, 0); \
         memcpy(buffer + 2, &data, sizeof(data));               \
      }                                                         \
      {                                                         \
         uint64_t data = 0x0;                                   \
         PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ELEMENTS", data, 0); \
         memcpy(buffer + 8, &data, sizeof(data));               \
      }                                                         \
      {                                                         \
         uint32_t data = 0x1;                                   \
         PVR_PDS_PRINT_DATA("DRAW_INDIRECT_ELEMENTS", data, 0); \
         memcpy(buffer + 10, &data, sizeof(data));              \
      }                                                         \
   } while (0)
#endif
