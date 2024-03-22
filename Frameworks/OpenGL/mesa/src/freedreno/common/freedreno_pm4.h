/*
 * Copyright (C) 2012-2018 Rob Clark <robclark@freedesktop.org>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#ifndef FREEDRENO_PM4_H_
#define FREEDRENO_PM4_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CP_TYPE0_PKT 0x00000000
#define CP_TYPE2_PKT 0x80000000
#define CP_TYPE3_PKT 0xc0000000
#define CP_TYPE4_PKT 0x40000000
#define CP_TYPE7_PKT 0x70000000

#define CP_NOP_MESG 0x4D455347
#define CP_NOP_BEGN 0x4245474E
#define CP_NOP_END  0x454E4400

/*
 * Helpers for pm4 pkt header building/parsing:
 */

static inline unsigned
pm4_odd_parity_bit(unsigned val)
{
   /* See: http://graphics.stanford.edu/~seander/bithacks.html#ParityParallel
    * note that we want odd parity so 0x6996 is inverted.
    */
   val ^= val >> 16;
   val ^= val >> 8;
   val ^= val >> 4;
   val &= 0xf;
   return (~0x6996 >> val) & 1;
}

static inline uint32_t
pm4_pkt0_hdr(uint16_t regindx, uint16_t cnt)
{
   return CP_TYPE0_PKT | ((cnt - 1) << 16) | (regindx & 0x7fff);
}

static inline uint32_t
pm4_pkt3_hdr(uint8_t opcode, uint16_t cnt)
{
   return CP_TYPE3_PKT | ((cnt - 1) << 16) | ((opcode & 0xff) << 8);
}

static inline uint32_t
pm4_pkt4_hdr(uint16_t regindx, uint16_t cnt)
{
   assert(cnt < 0x7f);
   return CP_TYPE4_PKT | cnt | (pm4_odd_parity_bit(cnt) << 7) |
         ((regindx & 0x3ffff) << 8) |
         ((pm4_odd_parity_bit(regindx) << 27));
}

static inline uint32_t
pm4_pkt7_hdr(uint8_t opcode, uint16_t cnt)
{
   return CP_TYPE7_PKT | cnt | (pm4_odd_parity_bit(cnt) << 15) |
         ((opcode & 0x7f) << 16) |
         ((pm4_odd_parity_bit(opcode) << 23));
}

/*
 * Helpers for packet parsing:
 */

#define pkt_is_type0(pkt)     (((pkt)&0XC0000000) == CP_TYPE0_PKT)
#define type0_pkt_size(pkt)   ((((pkt) >> 16) & 0x3FFF) + 1)
#define type0_pkt_offset(pkt) ((pkt)&0x7FFF)

#define pkt_is_type2(pkt) ((pkt) == CP_TYPE2_PKT)

#define pkt_is_type3(pkt)                                                      \
   ((((pkt)&0xC0000000) == CP_TYPE3_PKT) && (((pkt)&0x80FE) == 0))

#define cp_type3_opcode(pkt) (((pkt) >> 8) & 0xFF)
#define type3_pkt_size(pkt)  ((((pkt) >> 16) & 0x3FFF) + 1)

static inline unsigned
pm4_calc_odd_parity_bit(unsigned val)
{
   return (0x9669 >> (0xf & ((val) ^ ((val) >> 4) ^ ((val) >> 8) ^
                             ((val) >> 12) ^ ((val) >> 16) ^ ((val) >> 20) ^
                             ((val) >> 24) ^ ((val) >> 28)))) &
          1;
}

#define pkt_is_type4(pkt)                                                      \
   ((((pkt)&0xF0000000) == CP_TYPE4_PKT) &&                                    \
    ((((pkt) >> 27) & 0x1) ==                                                  \
     pm4_calc_odd_parity_bit(type4_pkt_offset(pkt))) &&                        \
    ((((pkt) >> 7) & 0x1) == pm4_calc_odd_parity_bit(type4_pkt_size(pkt))))

#define type4_pkt_offset(pkt) (((pkt) >> 8) & 0x7FFFF)
#define type4_pkt_size(pkt)   ((pkt)&0x7F)

#define pkt_is_type7(pkt)                                                      \
   ((((pkt)&0xF0000000) == CP_TYPE7_PKT) && (((pkt)&0x0F000000) == 0) &&       \
    ((((pkt) >> 23) & 0x1) ==                                                  \
     pm4_calc_odd_parity_bit(cp_type7_opcode(pkt))) &&                         \
    ((((pkt) >> 15) & 0x1) == pm4_calc_odd_parity_bit(type7_pkt_size(pkt))))

#define cp_type7_opcode(pkt) (((pkt) >> 16) & 0x7F)
#define type7_pkt_size(pkt)  ((pkt)&0x3FFF)

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* FREEDRENO_PM4_H_ */
