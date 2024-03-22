/*
 * Copyright © 2016 Broadcom
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

#include <string.h>
#include "util/macros.h"
#include "util/bitscan.h"

#include "broadcom/common/v3d_device_info.h"
#include "qpu_instr.h"

#ifndef QPU_MASK
#define QPU_MASK(high, low) ((((uint64_t)1<<((high)-(low)+1))-1)<<(low))
/* Using the GNU statement expression extension */
#define QPU_SET_FIELD(value, field)                                       \
        ({                                                                \
                uint64_t fieldval = (uint64_t)(value) << field ## _SHIFT; \
                assert((fieldval & ~ field ## _MASK) == 0);               \
                fieldval & field ## _MASK;                                \
         })

#define QPU_GET_FIELD(word, field) ((uint32_t)(((word)  & field ## _MASK) >> field ## _SHIFT))

#define QPU_UPDATE_FIELD(inst, value, field)                              \
        (((inst) & ~(field ## _MASK)) | QPU_SET_FIELD(value, field))
#endif /* QPU_MASK */

#define V3D_QPU_OP_MUL_SHIFT                58
#define V3D_QPU_OP_MUL_MASK                 QPU_MASK(63, 58)

#define V3D_QPU_SIG_SHIFT                   53
#define V3D_QPU_SIG_MASK                    QPU_MASK(57, 53)

#define V3D_QPU_COND_SHIFT                  46
#define V3D_QPU_COND_MASK                   QPU_MASK(52, 46)
#define V3D_QPU_COND_SIG_MAGIC_ADDR         (1 << 6)

#define V3D_QPU_MM                          QPU_MASK(45, 45)
#define V3D_QPU_MA                          QPU_MASK(44, 44)

#define V3D_QPU_WADDR_M_SHIFT               38
#define V3D_QPU_WADDR_M_MASK                QPU_MASK(43, 38)

#define V3D_QPU_BRANCH_ADDR_LOW_SHIFT       35
#define V3D_QPU_BRANCH_ADDR_LOW_MASK        QPU_MASK(55, 35)

#define V3D_QPU_WADDR_A_SHIFT               32
#define V3D_QPU_WADDR_A_MASK                QPU_MASK(37, 32)

#define V3D_QPU_BRANCH_COND_SHIFT           32
#define V3D_QPU_BRANCH_COND_MASK            QPU_MASK(34, 32)

#define V3D_QPU_BRANCH_ADDR_HIGH_SHIFT      24
#define V3D_QPU_BRANCH_ADDR_HIGH_MASK       QPU_MASK(31, 24)

#define V3D_QPU_OP_ADD_SHIFT                24
#define V3D_QPU_OP_ADD_MASK                 QPU_MASK(31, 24)

#define V3D_QPU_MUL_B_SHIFT                 21
#define V3D_QPU_MUL_B_MASK                  QPU_MASK(23, 21)

#define V3D_QPU_BRANCH_MSFIGN_SHIFT         21
#define V3D_QPU_BRANCH_MSFIGN_MASK          QPU_MASK(22, 21)

#define V3D_QPU_MUL_A_SHIFT                 18
#define V3D_QPU_MUL_A_MASK                  QPU_MASK(20, 18)

#define V3D_QPU_RADDR_C_SHIFT               18
#define V3D_QPU_RADDR_C_MASK                QPU_MASK(23, 18)

#define V3D_QPU_ADD_B_SHIFT                 15
#define V3D_QPU_ADD_B_MASK                  QPU_MASK(17, 15)

#define V3D_QPU_BRANCH_BDU_SHIFT            15
#define V3D_QPU_BRANCH_BDU_MASK             QPU_MASK(17, 15)

#define V3D_QPU_BRANCH_UB                   QPU_MASK(14, 14)

#define V3D_QPU_ADD_A_SHIFT                 12
#define V3D_QPU_ADD_A_MASK                  QPU_MASK(14, 12)

#define V3D_QPU_BRANCH_BDI_SHIFT            12
#define V3D_QPU_BRANCH_BDI_MASK             QPU_MASK(13, 12)

#define V3D_QPU_RADDR_D_SHIFT               12
#define V3D_QPU_RADDR_D_MASK                QPU_MASK(17, 12)

#define V3D_QPU_RADDR_A_SHIFT               6
#define V3D_QPU_RADDR_A_MASK                QPU_MASK(11, 6)

#define V3D_QPU_RADDR_B_SHIFT               0
#define V3D_QPU_RADDR_B_MASK                QPU_MASK(5, 0)

#define THRSW .thrsw = true
#define LDUNIF .ldunif = true
#define LDUNIFRF .ldunifrf = true
#define LDUNIFA .ldunifa = true
#define LDUNIFARF .ldunifarf = true
#define LDTMU .ldtmu = true
#define LDVARY .ldvary = true
#define LDVPM .ldvpm = true
#define LDTLB .ldtlb = true
#define LDTLBU .ldtlbu = true
#define UCB .ucb = true
#define ROT .rotate = true
#define WRTMUC .wrtmuc = true
#define SMIMM_A .small_imm_a = true
#define SMIMM_B .small_imm_b = true
#define SMIMM_C .small_imm_c = true
#define SMIMM_D .small_imm_d = true

static const struct v3d_qpu_sig v33_sig_map[] = {
        /*      MISC   R3       R4      R5 */
        [0]  = {                               },
        [1]  = { THRSW,                        },
        [2]  = {                        LDUNIF },
        [3]  = { THRSW,                 LDUNIF },
        [4]  = {                LDTMU,         },
        [5]  = { THRSW,         LDTMU,         },
        [6]  = {                LDTMU,  LDUNIF },
        [7]  = { THRSW,         LDTMU,  LDUNIF },
        [8]  = {        LDVARY,                },
        [9]  = { THRSW, LDVARY,                },
        [10] = {        LDVARY,         LDUNIF },
        [11] = { THRSW, LDVARY,         LDUNIF },
        [12] = {        LDVARY, LDTMU,         },
        [13] = { THRSW, LDVARY, LDTMU,         },
        [14] = { SMIMM_B, LDVARY,              },
        [15] = { SMIMM_B,                      },
        [16] = {        LDTLB,                 },
        [17] = {        LDTLBU,                },
        /* 18-21 reserved */
        [22] = { UCB,                          },
        [23] = { ROT,                          },
        [24] = {        LDVPM,                 },
        [25] = { THRSW, LDVPM,                 },
        [26] = {        LDVPM,          LDUNIF },
        [27] = { THRSW, LDVPM,          LDUNIF },
        [28] = {        LDVPM, LDTMU,          },
        [29] = { THRSW, LDVPM, LDTMU,          },
        [30] = { SMIMM_B, LDVPM,               },
        [31] = { SMIMM_B,                      },
};

static const struct v3d_qpu_sig v40_sig_map[] = {
        /*      MISC    R3      R4      R5 */
        [0]  = {                               },
        [1]  = { THRSW,                        },
        [2]  = {                        LDUNIF },
        [3]  = { THRSW,                 LDUNIF },
        [4]  = {                LDTMU,         },
        [5]  = { THRSW,         LDTMU,         },
        [6]  = {                LDTMU,  LDUNIF },
        [7]  = { THRSW,         LDTMU,  LDUNIF },
        [8]  = {        LDVARY,                },
        [9]  = { THRSW, LDVARY,                },
        [10] = {        LDVARY,         LDUNIF },
        [11] = { THRSW, LDVARY,         LDUNIF },
        /* 12-13 reserved */
        [14] = { SMIMM_B, LDVARY,              },
        [15] = { SMIMM_B,                      },
        [16] = {        LDTLB,                 },
        [17] = {        LDTLBU,                },
        [18] = {                        WRTMUC },
        [19] = { THRSW,                 WRTMUC },
        [20] = {        LDVARY,         WRTMUC },
        [21] = { THRSW, LDVARY,         WRTMUC },
        [22] = { UCB,                          },
        [23] = { ROT,                          },
        /* 24-30 reserved */
        [31] = { SMIMM_B,       LDTMU,         },
};

static const struct v3d_qpu_sig v41_sig_map[] = {
        /*      MISC       phys    R5 */
        [0]  = {                          },
        [1]  = { THRSW,                   },
        [2]  = {                   LDUNIF },
        [3]  = { THRSW,            LDUNIF },
        [4]  = {           LDTMU,         },
        [5]  = { THRSW,    LDTMU,         },
        [6]  = {           LDTMU,  LDUNIF },
        [7]  = { THRSW,    LDTMU,  LDUNIF },
        [8]  = {           LDVARY,        },
        [9]  = { THRSW,    LDVARY,        },
        [10] = {           LDVARY, LDUNIF },
        [11] = { THRSW,    LDVARY, LDUNIF },
        [12] = { LDUNIFRF                 },
        [13] = { THRSW,    LDUNIFRF       },
        [14] = { SMIMM_B,    LDVARY       },
        [15] = { SMIMM_B,                 },
        [16] = {           LDTLB,         },
        [17] = {           LDTLBU,        },
        [18] = {                          WRTMUC },
        [19] = { THRSW,                   WRTMUC },
        [20] = {           LDVARY,        WRTMUC },
        [21] = { THRSW,    LDVARY,        WRTMUC },
        [22] = { UCB,                     },
        [23] = { ROT,                     },
        [24] = {                   LDUNIFA},
        [25] = { LDUNIFARF                },
        /* 26-30 reserved */
        [31] = { SMIMM_B,          LDTMU, },
};


static const struct v3d_qpu_sig v71_sig_map[] = {
        /*      MISC       phys    RF0 */
        [0]  = {                          },
        [1]  = { THRSW,                   },
        [2]  = {                   LDUNIF },
        [3]  = { THRSW,            LDUNIF },
        [4]  = {           LDTMU,         },
        [5]  = { THRSW,    LDTMU,         },
        [6]  = {           LDTMU,  LDUNIF },
        [7]  = { THRSW,    LDTMU,  LDUNIF },
        [8]  = {           LDVARY,        },
        [9]  = { THRSW,    LDVARY,        },
        [10] = {           LDVARY, LDUNIF },
        [11] = { THRSW,    LDVARY, LDUNIF },
        [12] = { LDUNIFRF                 },
        [13] = { THRSW,    LDUNIFRF       },
        [14] = { SMIMM_A,                 },
        [15] = { SMIMM_B,                 },
        [16] = {           LDTLB,         },
        [17] = {           LDTLBU,        },
        [18] = {                          WRTMUC },
        [19] = { THRSW,                   WRTMUC },
        [20] = {           LDVARY,        WRTMUC },
        [21] = { THRSW,    LDVARY,        WRTMUC },
        [22] = { UCB,                     },
        /* 23 reserved */
        [24] = {                   LDUNIFA},
        [25] = { LDUNIFARF                },
        /* 26-29 reserved */
        [30] = { SMIMM_C,                 },
        [31] = { SMIMM_D,                 },
};

bool
v3d_qpu_sig_unpack(const struct v3d_device_info *devinfo,
                   uint32_t packed_sig,
                   struct v3d_qpu_sig *sig)
{
        if (packed_sig >= ARRAY_SIZE(v33_sig_map))
                return false;

        if (devinfo->ver >= 71)
                *sig = v71_sig_map[packed_sig];
        else if (devinfo->ver >= 41)
                *sig = v41_sig_map[packed_sig];
        else if (devinfo->ver == 40)
                *sig = v40_sig_map[packed_sig];
        else
                *sig = v33_sig_map[packed_sig];

        /* Signals with zeroed unpacked contents after element 0 are reserved. */
        return (packed_sig == 0 ||
                memcmp(sig, &v33_sig_map[0], sizeof(*sig)) != 0);
}

bool
v3d_qpu_sig_pack(const struct v3d_device_info *devinfo,
                 const struct v3d_qpu_sig *sig,
                 uint32_t *packed_sig)
{
        static const struct v3d_qpu_sig *map;

        if (devinfo->ver >= 71)
                map = v71_sig_map;
        else if (devinfo->ver >= 41)
                map = v41_sig_map;
        else if (devinfo->ver == 40)
                map = v40_sig_map;
        else
                map = v33_sig_map;

        for (int i = 0; i < ARRAY_SIZE(v33_sig_map); i++) {
                if (memcmp(&map[i], sig, sizeof(*sig)) == 0) {
                        *packed_sig = i;
                        return true;
                }
        }

        return false;
}

static const uint32_t small_immediates[] = {
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15,
        -16, -15, -14, -13,
        -12, -11, -10, -9,
        -8, -7, -6, -5,
        -4, -3, -2, -1,
        0x3b800000, /* 2.0^-8 */
        0x3c000000, /* 2.0^-7 */
        0x3c800000, /* 2.0^-6 */
        0x3d000000, /* 2.0^-5 */
        0x3d800000, /* 2.0^-4 */
        0x3e000000, /* 2.0^-3 */
        0x3e800000, /* 2.0^-2 */
        0x3f000000, /* 2.0^-1 */
        0x3f800000, /* 2.0^0 */
        0x40000000, /* 2.0^1 */
        0x40800000, /* 2.0^2 */
        0x41000000, /* 2.0^3 */
        0x41800000, /* 2.0^4 */
        0x42000000, /* 2.0^5 */
        0x42800000, /* 2.0^6 */
        0x43000000, /* 2.0^7 */
};

bool
v3d_qpu_small_imm_unpack(const struct v3d_device_info *devinfo,
                         uint32_t packed_small_immediate,
                         uint32_t *small_immediate)
{
        if (packed_small_immediate >= ARRAY_SIZE(small_immediates))
                return false;

        *small_immediate = small_immediates[packed_small_immediate];
        return true;
}

bool
v3d_qpu_small_imm_pack(const struct v3d_device_info *devinfo,
                       uint32_t value,
                       uint32_t *packed_small_immediate)
{
        STATIC_ASSERT(ARRAY_SIZE(small_immediates) == 48);

        for (int i = 0; i < ARRAY_SIZE(small_immediates); i++) {
                if (small_immediates[i] == value) {
                        *packed_small_immediate = i;
                        return true;
                }
        }

        return false;
}

bool
v3d_qpu_flags_unpack(const struct v3d_device_info *devinfo,
                     uint32_t packed_cond,
                     struct v3d_qpu_flags *cond)
{
        static const enum v3d_qpu_cond cond_map[4] = {
                [0] = V3D_QPU_COND_IFA,
                [1] = V3D_QPU_COND_IFB,
                [2] = V3D_QPU_COND_IFNA,
                [3] = V3D_QPU_COND_IFNB,
        };

        cond->ac = V3D_QPU_COND_NONE;
        cond->mc = V3D_QPU_COND_NONE;
        cond->apf = V3D_QPU_PF_NONE;
        cond->mpf = V3D_QPU_PF_NONE;
        cond->auf = V3D_QPU_UF_NONE;
        cond->muf = V3D_QPU_UF_NONE;

        if (packed_cond == 0) {
                return true;
        } else if (packed_cond >> 2 == 0) {
                cond->apf = packed_cond & 0x3;
        } else if (packed_cond >> 4 == 0) {
                cond->auf = (packed_cond & 0xf) - 4 + V3D_QPU_UF_ANDZ;
        } else if (packed_cond == 0x10) {
                return false;
        } else if (packed_cond >> 2 == 0x4) {
                cond->mpf = packed_cond & 0x3;
        } else if (packed_cond >> 4 == 0x1) {
                cond->muf = (packed_cond & 0xf) - 4 + V3D_QPU_UF_ANDZ;
        } else if (packed_cond >> 4 == 0x2) {
                cond->ac = ((packed_cond >> 2) & 0x3) + V3D_QPU_COND_IFA;
                cond->mpf = packed_cond & 0x3;
        } else if (packed_cond >> 4 == 0x3) {
                cond->mc = ((packed_cond >> 2) & 0x3) + V3D_QPU_COND_IFA;
                cond->apf = packed_cond & 0x3;
        } else if (packed_cond >> 6) {
                cond->mc = cond_map[(packed_cond >> 4) & 0x3];
                if (((packed_cond >> 2) & 0x3) == 0) {
                        cond->ac = cond_map[packed_cond & 0x3];
                } else {
                        cond->auf = (packed_cond & 0xf) - 4 + V3D_QPU_UF_ANDZ;
                }
        }

        return true;
}

bool
v3d_qpu_flags_pack(const struct v3d_device_info *devinfo,
                   const struct v3d_qpu_flags *cond,
                   uint32_t *packed_cond)
{
#define AC (1 << 0)
#define MC (1 << 1)
#define APF (1 << 2)
#define MPF (1 << 3)
#define AUF (1 << 4)
#define MUF (1 << 5)
        static const struct {
                uint8_t flags_present;
                uint8_t bits;
        } flags_table[] = {
                { 0,        0 },
                { APF,      0 },
                { AUF,      0 },
                { MPF,      (1 << 4) },
                { MUF,      (1 << 4) },
                { AC,       (1 << 5) },
                { AC | MPF, (1 << 5) },
                { MC,       (1 << 5) | (1 << 4) },
                { MC | APF, (1 << 5) | (1 << 4) },
                { MC | AC,  (1 << 6) },
                { MC | AUF, (1 << 6) },
        };

        uint8_t flags_present = 0;
        if (cond->ac != V3D_QPU_COND_NONE)
                flags_present |= AC;
        if (cond->mc != V3D_QPU_COND_NONE)
                flags_present |= MC;
        if (cond->apf != V3D_QPU_PF_NONE)
                flags_present |= APF;
        if (cond->mpf != V3D_QPU_PF_NONE)
                flags_present |= MPF;
        if (cond->auf != V3D_QPU_UF_NONE)
                flags_present |= AUF;
        if (cond->muf != V3D_QPU_UF_NONE)
                flags_present |= MUF;

        for (int i = 0; i < ARRAY_SIZE(flags_table); i++) {
                if (flags_table[i].flags_present != flags_present)
                        continue;

                *packed_cond = flags_table[i].bits;

                *packed_cond |= cond->apf;
                *packed_cond |= cond->mpf;

                if (flags_present & AUF)
                        *packed_cond |= cond->auf - V3D_QPU_UF_ANDZ + 4;
                if (flags_present & MUF)
                        *packed_cond |= cond->muf - V3D_QPU_UF_ANDZ + 4;

                if (flags_present & AC) {
                        if (*packed_cond & (1 << 6))
                                *packed_cond |= cond->ac - V3D_QPU_COND_IFA;
                        else
                                *packed_cond |= (cond->ac -
                                                 V3D_QPU_COND_IFA) << 2;
                }

                if (flags_present & MC) {
                        if (*packed_cond & (1 << 6))
                                *packed_cond |= (cond->mc -
                                                 V3D_QPU_COND_IFA) << 4;
                        else
                                *packed_cond |= (cond->mc -
                                                 V3D_QPU_COND_IFA) << 2;
                }

                return true;
        }

        return false;
}

/* Make a mapping of the table of opcodes in the spec.  The opcode is
 * determined by a combination of the opcode field, and in the case of 0 or
 * 1-arg opcodes, the mux (version <= 42) or raddr (version >= 71) field as
 * well.
 */
#define OP_MASK(val) BITFIELD64_BIT(val)
#define OP_RANGE(bot, top) BITFIELD64_RANGE(bot, top - bot + 1)
#define ANYMUX OP_RANGE(0, 7)
#define ANYOPMASK OP_RANGE(0, 63)

struct opcode_desc {
        uint8_t opcode_first;
        uint8_t opcode_last;

        union {
                struct {
                        uint8_t b_mask;
                        uint8_t a_mask;
                } mux;
                uint64_t raddr_mask;
        };

        uint8_t op;

        /* first_ver == 0 if it's the same across all V3D versions.
         * first_ver == X, last_ver == 0 if it's the same for all V3D versions
         *   starting from X
         * first_ver == X, last_ver == Y if it's the same for all V3D versions
         *   on the range X through Y
         */
        uint8_t first_ver;
        uint8_t last_ver;
};

static const struct opcode_desc add_ops_v33[] = {
        /* FADD is FADDNF depending on the order of the mux_a/mux_b. */
        { 0,   47,  .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_FADD },
        { 0,   47,  .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_FADDNF },
        { 53,  55,  .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_VFPACK },
        { 56,  56,  .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_ADD },
        { 57,  59,  .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_VFPACK },
        { 60,  60,  .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_SUB },
        { 61,  63,  .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_VFPACK },
        { 64,  111, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_FSUB },
        { 120, 120, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_MIN },
        { 121, 121, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_MAX },
        { 122, 122, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_UMIN },
        { 123, 123, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_UMAX },
        { 124, 124, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_SHL },
        { 125, 125, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_SHR },
        { 126, 126, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_ASR },
        { 127, 127, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_ROR },
        /* FMIN is instead FMAX depending on the order of the mux_a/mux_b. */
        { 128, 175, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_FMIN },
        { 128, 175, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_FMAX },
        { 176, 180, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_VFMIN },

        { 181, 181, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_AND },
        { 182, 182, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_OR },
        { 183, 183, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_XOR },

        { 184, 184, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_VADD },
        { 185, 185, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_VSUB },
        { 186, 186, .mux.b_mask = OP_MASK(0), .mux.a_mask = ANYMUX, V3D_QPU_A_NOT },
        { 186, 186, .mux.b_mask = OP_MASK(1), .mux.a_mask = ANYMUX, V3D_QPU_A_NEG },
        { 186, 186, .mux.b_mask = OP_MASK(2), .mux.a_mask = ANYMUX, V3D_QPU_A_FLAPUSH },
        { 186, 186, .mux.b_mask = OP_MASK(3), .mux.a_mask = ANYMUX, V3D_QPU_A_FLBPUSH },
        { 186, 186, .mux.b_mask = OP_MASK(4), .mux.a_mask = ANYMUX, V3D_QPU_A_FLPOP },
        { 186, 186, .mux.b_mask = OP_MASK(5), .mux.a_mask = ANYMUX, V3D_QPU_A_RECIP },
        { 186, 186, .mux.b_mask = OP_MASK(6), .mux.a_mask = ANYMUX, V3D_QPU_A_SETMSF },
        { 186, 186, .mux.b_mask = OP_MASK(7), .mux.a_mask = ANYMUX, V3D_QPU_A_SETREVF },
        { 187, 187, .mux.b_mask = OP_MASK(0), .mux.a_mask = OP_MASK(0), V3D_QPU_A_NOP, 0 },
        { 187, 187, .mux.b_mask = OP_MASK(0), .mux.a_mask = OP_MASK(1), V3D_QPU_A_TIDX },
        { 187, 187, .mux.b_mask = OP_MASK(0), .mux.a_mask = OP_MASK(2), V3D_QPU_A_EIDX },
        { 187, 187, .mux.b_mask = OP_MASK(0), .mux.a_mask = OP_MASK(3), V3D_QPU_A_LR },
        { 187, 187, .mux.b_mask = OP_MASK(0), .mux.a_mask = OP_MASK(4), V3D_QPU_A_VFLA },
        { 187, 187, .mux.b_mask = OP_MASK(0), .mux.a_mask = OP_MASK(5), V3D_QPU_A_VFLNA },
        { 187, 187, .mux.b_mask = OP_MASK(0), .mux.a_mask = OP_MASK(6), V3D_QPU_A_VFLB },
        { 187, 187, .mux.b_mask = OP_MASK(0), .mux.a_mask = OP_MASK(7), V3D_QPU_A_VFLNB },

        { 187, 187, .mux.b_mask = OP_MASK(1), .mux.a_mask = OP_RANGE(0, 2), V3D_QPU_A_FXCD },
        { 187, 187, .mux.b_mask = OP_MASK(1), .mux.a_mask = OP_MASK(3), V3D_QPU_A_XCD },
        { 187, 187, .mux.b_mask = OP_MASK(1), .mux.a_mask = OP_RANGE(4, 6), V3D_QPU_A_FYCD },
        { 187, 187, .mux.b_mask = OP_MASK(1), .mux.a_mask = OP_MASK(7), V3D_QPU_A_YCD },

        { 187, 187, .mux.b_mask = OP_MASK(2), .mux.a_mask = OP_MASK(0), V3D_QPU_A_MSF },
        { 187, 187, .mux.b_mask = OP_MASK(2), .mux.a_mask = OP_MASK(1), V3D_QPU_A_REVF },
        { 187, 187, .mux.b_mask = OP_MASK(2), .mux.a_mask = OP_MASK(2), V3D_QPU_A_VDWWT, 33 },
        { 187, 187, .mux.b_mask = OP_MASK(2), .mux.a_mask = OP_MASK(2), V3D_QPU_A_IID, 40 },
        { 187, 187, .mux.b_mask = OP_MASK(2), .mux.a_mask = OP_MASK(3), V3D_QPU_A_SAMPID, 40 },
        { 187, 187, .mux.b_mask = OP_MASK(2), .mux.a_mask = OP_MASK(4), V3D_QPU_A_BARRIERID, 40 },
        { 187, 187, .mux.b_mask = OP_MASK(2), .mux.a_mask = OP_MASK(5), V3D_QPU_A_TMUWT },
        { 187, 187, .mux.b_mask = OP_MASK(2), .mux.a_mask = OP_MASK(6), V3D_QPU_A_VPMWT },
        { 187, 187, .mux.b_mask = OP_MASK(2), .mux.a_mask = OP_MASK(7), V3D_QPU_A_FLAFIRST, 41 },
        { 187, 187, .mux.b_mask = OP_MASK(3), .mux.a_mask = OP_MASK(0), V3D_QPU_A_FLNAFIRST, 41 },
        { 187, 187, .mux.b_mask = OP_MASK(3), .mux.a_mask = ANYMUX, V3D_QPU_A_VPMSETUP, 33 },

        { 188, 188, .mux.b_mask = OP_MASK(0), .mux.a_mask = ANYMUX, V3D_QPU_A_LDVPMV_IN, 40 },
        { 188, 188, .mux.b_mask = OP_MASK(0), .mux.a_mask = ANYMUX, V3D_QPU_A_LDVPMV_OUT, 40 },
        { 188, 188, .mux.b_mask = OP_MASK(1), .mux.a_mask = ANYMUX, V3D_QPU_A_LDVPMD_IN, 40 },
        { 188, 188, .mux.b_mask = OP_MASK(1), .mux.a_mask = ANYMUX, V3D_QPU_A_LDVPMD_OUT, 40 },
        { 188, 188, .mux.b_mask = OP_MASK(2), .mux.a_mask = ANYMUX, V3D_QPU_A_LDVPMP, 40 },
        { 188, 188, .mux.b_mask = OP_MASK(3), .mux.a_mask = ANYMUX, V3D_QPU_A_RSQRT, 41 },
        { 188, 188, .mux.b_mask = OP_MASK(4), .mux.a_mask = ANYMUX, V3D_QPU_A_EXP, 41 },
        { 188, 188, .mux.b_mask = OP_MASK(5), .mux.a_mask = ANYMUX, V3D_QPU_A_LOG, 41 },
        { 188, 188, .mux.b_mask = OP_MASK(6), .mux.a_mask = ANYMUX, V3D_QPU_A_SIN, 41 },
        { 188, 188, .mux.b_mask = OP_MASK(7), .mux.a_mask = ANYMUX, V3D_QPU_A_RSQRT2, 41 },
        { 189, 189, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_LDVPMG_IN, 40 },
        { 189, 189, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_LDVPMG_OUT, 40 },

        /* FIXME: MORE COMPLICATED */
        /* { 190, 191, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_VFMOVABSNEGNAB }, */

        { 192, 239, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_FCMP },
        { 240, 244, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_VFMAX },

        { 245, 245, .mux.b_mask = OP_RANGE(0, 2), .mux.a_mask = ANYMUX, V3D_QPU_A_FROUND },
        { 245, 245, .mux.b_mask = OP_MASK(3), .mux.a_mask = ANYMUX, V3D_QPU_A_FTOIN },
        { 245, 245, .mux.b_mask = OP_RANGE(4, 6), .mux.a_mask = ANYMUX, V3D_QPU_A_FTRUNC },
        { 245, 245, .mux.b_mask = OP_MASK(7), .mux.a_mask = ANYMUX, V3D_QPU_A_FTOIZ },
        { 246, 246, .mux.b_mask = OP_RANGE(0, 2), .mux.a_mask = ANYMUX, V3D_QPU_A_FFLOOR },
        { 246, 246, .mux.b_mask = OP_MASK(3), .mux.a_mask = ANYMUX, V3D_QPU_A_FTOUZ },
        { 246, 246, .mux.b_mask = OP_RANGE(4, 6), .mux.a_mask = ANYMUX, V3D_QPU_A_FCEIL },
        { 246, 246, .mux.b_mask = OP_MASK(7), .mux.a_mask = ANYMUX, V3D_QPU_A_FTOC },

        { 247, 247, .mux.b_mask = OP_RANGE(0, 2), .mux.a_mask = ANYMUX, V3D_QPU_A_FDX },
        { 247, 247, .mux.b_mask = OP_RANGE(4, 6), .mux.a_mask = ANYMUX, V3D_QPU_A_FDY },

        /* The stvpms are distinguished by the waddr field. */
        { 248, 248, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_STVPMV },
        { 248, 248, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_STVPMD },
        { 248, 248, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_A_STVPMP },

        { 252, 252, .mux.b_mask = OP_RANGE(0, 2), .mux.a_mask = ANYMUX, V3D_QPU_A_ITOF },
        { 252, 252, .mux.b_mask = OP_MASK(3), .mux.a_mask = ANYMUX, V3D_QPU_A_CLZ },
        { 252, 252, .mux.b_mask = OP_RANGE(4, 6), .mux.a_mask = ANYMUX, V3D_QPU_A_UTOF },
};

static const struct opcode_desc mul_ops_v33[] = {
        { 1, 1, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_M_ADD },
        { 2, 2, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_M_SUB },
        { 3, 3, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_M_UMUL24 },
        { 4, 8, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_M_VFMUL },
        { 9, 9, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_M_SMUL24 },
        { 10, 10, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_M_MULTOP },
        { 14, 14, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_M_FMOV, 33, 42 },
        { 15, 15, .mux.b_mask = OP_RANGE(0, 3), ANYMUX, V3D_QPU_M_FMOV, 33, 42},
        { 15, 15, .mux.b_mask = OP_MASK(4), .mux.a_mask = OP_MASK(0), V3D_QPU_M_NOP, 33, 42 },
        { 15, 15, .mux.b_mask = OP_MASK(7), .mux.a_mask = ANYMUX, V3D_QPU_M_MOV, 33, 42 },

        { 16, 63, .mux.b_mask = ANYMUX, .mux.a_mask = ANYMUX, V3D_QPU_M_FMUL },
};

/* Note that it would have been possible to define all the add/mul opcodes in
 * just one table, using the first_ver/last_ver. But taking into account that
 * for v71 there were a lot of changes, it was more tidy this way. Also right
 * now we are doing a linear search on those tables, so this maintains the
 * tables smaller.
 *
 * Just in case we merge the tables, we define the first_ver as 71 for those
 * opcodes that changed on v71
 */
static const struct opcode_desc add_ops_v71[] = {
        /* FADD is FADDNF depending on the order of the raddr_a/raddr_b. */
        { 0,   47,  .raddr_mask = ANYOPMASK, V3D_QPU_A_FADD },
        { 0,   47,  .raddr_mask = ANYOPMASK, V3D_QPU_A_FADDNF },
        { 53,  55,  .raddr_mask = ANYOPMASK, V3D_QPU_A_VFPACK },
        { 56,  56,  .raddr_mask = ANYOPMASK, V3D_QPU_A_ADD },
        { 57,  59,  .raddr_mask = ANYOPMASK, V3D_QPU_A_VFPACK },
        { 60,  60,  .raddr_mask = ANYOPMASK, V3D_QPU_A_SUB },
        { 61,  63,  .raddr_mask = ANYOPMASK, V3D_QPU_A_VFPACK },
        { 64,  111, .raddr_mask = ANYOPMASK, V3D_QPU_A_FSUB },
        { 120, 120, .raddr_mask = ANYOPMASK, V3D_QPU_A_MIN },
        { 121, 121, .raddr_mask = ANYOPMASK, V3D_QPU_A_MAX },
        { 122, 122, .raddr_mask = ANYOPMASK, V3D_QPU_A_UMIN },
        { 123, 123, .raddr_mask = ANYOPMASK, V3D_QPU_A_UMAX },
        { 124, 124, .raddr_mask = ANYOPMASK, V3D_QPU_A_SHL },
        { 125, 125, .raddr_mask = ANYOPMASK, V3D_QPU_A_SHR },
        { 126, 126, .raddr_mask = ANYOPMASK, V3D_QPU_A_ASR },
        { 127, 127, .raddr_mask = ANYOPMASK, V3D_QPU_A_ROR },
        /* FMIN is instead FMAX depending on the raddr_a/b order. */
        { 128, 175, .raddr_mask = ANYOPMASK, V3D_QPU_A_FMIN },
        { 128, 175, .raddr_mask = ANYOPMASK, V3D_QPU_A_FMAX },
        { 176, 180, .raddr_mask = ANYOPMASK, V3D_QPU_A_VFMIN },

        { 181, 181, .raddr_mask = ANYOPMASK, V3D_QPU_A_AND },
        { 182, 182, .raddr_mask = ANYOPMASK, V3D_QPU_A_OR },
        { 183, 183, .raddr_mask = ANYOPMASK, V3D_QPU_A_XOR },
        { 184, 184, .raddr_mask = ANYOPMASK, V3D_QPU_A_VADD },
        { 185, 185, .raddr_mask = ANYOPMASK, V3D_QPU_A_VSUB },

        { 186, 186, .raddr_mask = OP_MASK(0), V3D_QPU_A_NOT },
        { 186, 186, .raddr_mask = OP_MASK(1), V3D_QPU_A_NEG },
        { 186, 186, .raddr_mask = OP_MASK(2), V3D_QPU_A_FLAPUSH },
        { 186, 186, .raddr_mask = OP_MASK(3), V3D_QPU_A_FLBPUSH },
        { 186, 186, .raddr_mask = OP_MASK(4), V3D_QPU_A_FLPOP },
        { 186, 186, .raddr_mask = OP_MASK(5), V3D_QPU_A_CLZ },
        { 186, 186, .raddr_mask = OP_MASK(6), V3D_QPU_A_SETMSF },
        { 186, 186, .raddr_mask = OP_MASK(7), V3D_QPU_A_SETREVF },

        { 187, 187, .raddr_mask = OP_MASK(0), V3D_QPU_A_NOP, 0 },
        { 187, 187, .raddr_mask = OP_MASK(1), V3D_QPU_A_TIDX },
        { 187, 187, .raddr_mask = OP_MASK(2), V3D_QPU_A_EIDX },
        { 187, 187, .raddr_mask = OP_MASK(3), V3D_QPU_A_LR },
        { 187, 187, .raddr_mask = OP_MASK(4), V3D_QPU_A_VFLA },
        { 187, 187, .raddr_mask = OP_MASK(5), V3D_QPU_A_VFLNA },
        { 187, 187, .raddr_mask = OP_MASK(6), V3D_QPU_A_VFLB },
        { 187, 187, .raddr_mask = OP_MASK(7), V3D_QPU_A_VFLNB },
        { 187, 187, .raddr_mask = OP_MASK(8), V3D_QPU_A_XCD },
        { 187, 187, .raddr_mask = OP_MASK(9), V3D_QPU_A_YCD },
        { 187, 187, .raddr_mask = OP_MASK(10), V3D_QPU_A_MSF },
        { 187, 187, .raddr_mask = OP_MASK(11), V3D_QPU_A_REVF },
        { 187, 187, .raddr_mask = OP_MASK(12), V3D_QPU_A_IID },
        { 187, 187, .raddr_mask = OP_MASK(13), V3D_QPU_A_SAMPID },
        { 187, 187, .raddr_mask = OP_MASK(14), V3D_QPU_A_BARRIERID },
        { 187, 187, .raddr_mask = OP_MASK(15), V3D_QPU_A_TMUWT },
        { 187, 187, .raddr_mask = OP_MASK(16), V3D_QPU_A_VPMWT },
        { 187, 187, .raddr_mask = OP_MASK(17), V3D_QPU_A_FLAFIRST },
        { 187, 187, .raddr_mask = OP_MASK(18), V3D_QPU_A_FLNAFIRST },

        { 187, 187, .raddr_mask = OP_RANGE(32, 34), V3D_QPU_A_FXCD },
        { 187, 187, .raddr_mask = OP_RANGE(36, 38), V3D_QPU_A_FYCD },

        { 188, 188, .raddr_mask = OP_MASK(0), V3D_QPU_A_LDVPMV_IN, 71 },
        { 188, 188, .raddr_mask = OP_MASK(1), V3D_QPU_A_LDVPMD_IN, 71 },
        { 188, 188, .raddr_mask = OP_MASK(2), V3D_QPU_A_LDVPMP, 71 },

        { 188, 188, .raddr_mask = OP_MASK(32), V3D_QPU_A_RECIP, 71 },
        { 188, 188, .raddr_mask = OP_MASK(33), V3D_QPU_A_RSQRT, 71 },
        { 188, 188, .raddr_mask = OP_MASK(34), V3D_QPU_A_EXP, 71 },
        { 188, 188, .raddr_mask = OP_MASK(35), V3D_QPU_A_LOG, 71 },
        { 188, 188, .raddr_mask = OP_MASK(36), V3D_QPU_A_SIN, 71 },
        { 188, 188, .raddr_mask = OP_MASK(37), V3D_QPU_A_RSQRT2, 71 },

        { 189, 189, .raddr_mask = ANYOPMASK, V3D_QPU_A_LDVPMG_IN, 71 },

        /* The stvpms are distinguished by the waddr field. */
        { 190, 190, .raddr_mask = ANYOPMASK, V3D_QPU_A_STVPMV, 71},
        { 190, 190, .raddr_mask = ANYOPMASK, V3D_QPU_A_STVPMD, 71},
        { 190, 190, .raddr_mask = ANYOPMASK, V3D_QPU_A_STVPMP, 71},

        { 192, 207, .raddr_mask = ANYOPMASK, V3D_QPU_A_FCMP, 71 },

        { 245, 245, .raddr_mask = OP_RANGE(0, 2),   V3D_QPU_A_FROUND, 71 },
        { 245, 245, .raddr_mask = OP_RANGE(4, 6),   V3D_QPU_A_FROUND, 71 },
        { 245, 245, .raddr_mask = OP_RANGE(8, 10),  V3D_QPU_A_FROUND, 71 },
        { 245, 245, .raddr_mask = OP_RANGE(12, 14), V3D_QPU_A_FROUND, 71 },

        { 245, 245, .raddr_mask = OP_MASK(3),  V3D_QPU_A_FTOIN, 71 },
        { 245, 245, .raddr_mask = OP_MASK(7),  V3D_QPU_A_FTOIN, 71 },
        { 245, 245, .raddr_mask = OP_MASK(11), V3D_QPU_A_FTOIN, 71 },
        { 245, 245, .raddr_mask = OP_MASK(15), V3D_QPU_A_FTOIN, 71 },

        { 245, 245, .raddr_mask = OP_RANGE(16, 18), V3D_QPU_A_FTRUNC, 71 },
        { 245, 245, .raddr_mask = OP_RANGE(20, 22), V3D_QPU_A_FTRUNC, 71 },
        { 245, 245, .raddr_mask = OP_RANGE(24, 26), V3D_QPU_A_FTRUNC, 71 },
        { 245, 245, .raddr_mask = OP_RANGE(28, 30), V3D_QPU_A_FTRUNC, 71 },

        { 245, 245, .raddr_mask = OP_MASK(19), V3D_QPU_A_FTOIZ, 71 },
        { 245, 245, .raddr_mask = OP_MASK(23), V3D_QPU_A_FTOIZ, 71 },
        { 245, 245, .raddr_mask = OP_MASK(27), V3D_QPU_A_FTOIZ, 71 },
        { 245, 245, .raddr_mask = OP_MASK(31), V3D_QPU_A_FTOIZ, 71 },

        { 245, 245, .raddr_mask = OP_RANGE(32, 34), V3D_QPU_A_FFLOOR, 71 },
        { 245, 245, .raddr_mask = OP_RANGE(36, 38), V3D_QPU_A_FFLOOR, 71 },
        { 245, 245, .raddr_mask = OP_RANGE(40, 42), V3D_QPU_A_FFLOOR, 71 },
        { 245, 245, .raddr_mask = OP_RANGE(44, 46), V3D_QPU_A_FFLOOR, 71 },

        { 245, 245, .raddr_mask = OP_MASK(35), V3D_QPU_A_FTOUZ, 71 },
        { 245, 245, .raddr_mask = OP_MASK(39), V3D_QPU_A_FTOUZ, 71 },
        { 245, 245, .raddr_mask = OP_MASK(43), V3D_QPU_A_FTOUZ, 71 },
        { 245, 245, .raddr_mask = OP_MASK(47), V3D_QPU_A_FTOUZ, 71 },

        { 245, 245, .raddr_mask = OP_RANGE(48, 50), V3D_QPU_A_FCEIL, 71 },
        { 245, 245, .raddr_mask = OP_RANGE(52, 54), V3D_QPU_A_FCEIL, 71 },
        { 245, 245, .raddr_mask = OP_RANGE(56, 58), V3D_QPU_A_FCEIL, 71 },
        { 245, 245, .raddr_mask = OP_RANGE(60, 62), V3D_QPU_A_FCEIL, 71 },

        { 245, 245, .raddr_mask = OP_MASK(51), V3D_QPU_A_FTOC },
        { 245, 245, .raddr_mask = OP_MASK(55), V3D_QPU_A_FTOC },
        { 245, 245, .raddr_mask = OP_MASK(59), V3D_QPU_A_FTOC },
        { 245, 245, .raddr_mask = OP_MASK(63), V3D_QPU_A_FTOC },

        { 246, 246, .raddr_mask = OP_RANGE(0, 2),   V3D_QPU_A_FDX, 71 },
        { 246, 246, .raddr_mask = OP_RANGE(4, 6),   V3D_QPU_A_FDX, 71 },
        { 246, 246, .raddr_mask = OP_RANGE(8, 10),  V3D_QPU_A_FDX, 71 },
        { 246, 246, .raddr_mask = OP_RANGE(12, 14), V3D_QPU_A_FDX, 71 },
        { 246, 246, .raddr_mask = OP_RANGE(16, 18), V3D_QPU_A_FDY, 71 },
        { 246, 246, .raddr_mask = OP_RANGE(20, 22), V3D_QPU_A_FDY, 71 },
        { 246, 246, .raddr_mask = OP_RANGE(24, 26), V3D_QPU_A_FDY, 71 },
        { 246, 246, .raddr_mask = OP_RANGE(28, 30), V3D_QPU_A_FDY, 71 },

        { 246, 246, .raddr_mask = OP_RANGE(32, 34), V3D_QPU_A_ITOF, 71 },
        { 246, 246, .raddr_mask = OP_RANGE(36, 38), V3D_QPU_A_UTOF, 71 },

        { 247, 247, .raddr_mask = ANYOPMASK, V3D_QPU_A_VPACK, 71 },
        { 248, 248, .raddr_mask = ANYOPMASK, V3D_QPU_A_V8PACK, 71 },

        { 249, 249, .raddr_mask = OP_RANGE(0, 2),   V3D_QPU_A_FMOV, 71 },
        { 249, 249, .raddr_mask = OP_RANGE(4, 6),   V3D_QPU_A_FMOV, 71 },
        { 249, 249, .raddr_mask = OP_RANGE(8, 10),  V3D_QPU_A_FMOV, 71 },
        { 249, 249, .raddr_mask = OP_RANGE(12, 14), V3D_QPU_A_FMOV, 71 },
        { 249, 249, .raddr_mask = OP_RANGE(16, 18), V3D_QPU_A_FMOV, 71 },
        { 249, 249, .raddr_mask = OP_RANGE(20, 22), V3D_QPU_A_FMOV, 71 },
        { 249, 249, .raddr_mask = OP_RANGE(24, 26), V3D_QPU_A_FMOV, 71 },

        { 249, 249, .raddr_mask = OP_MASK(3),  V3D_QPU_A_MOV, 71 },
        { 249, 249, .raddr_mask = OP_MASK(7),  V3D_QPU_A_MOV, 71 },
        { 249, 249, .raddr_mask = OP_MASK(11), V3D_QPU_A_MOV, 71 },
        { 249, 249, .raddr_mask = OP_MASK(15), V3D_QPU_A_MOV, 71 },
        { 249, 249, .raddr_mask = OP_MASK(19), V3D_QPU_A_MOV, 71 },

        { 250, 250, .raddr_mask = ANYOPMASK, V3D_QPU_A_V10PACK, 71 },
        { 251, 251, .raddr_mask = ANYOPMASK, V3D_QPU_A_V11FPACK, 71 },
};

static const struct opcode_desc mul_ops_v71[] = {
        /* For V3D 7.1, second mask field would be ignored */
        { 1, 1, .raddr_mask = ANYOPMASK, V3D_QPU_M_ADD, 71 },
        { 2, 2, .raddr_mask = ANYOPMASK, V3D_QPU_M_SUB, 71 },
        { 3, 3, .raddr_mask = ANYOPMASK, V3D_QPU_M_UMUL24, 71 },
        { 3, 3, .raddr_mask = ANYOPMASK, V3D_QPU_M_UMUL24, 71 },
        { 4, 8, .raddr_mask = ANYOPMASK, V3D_QPU_M_VFMUL, 71 },
        { 9, 9, .raddr_mask = ANYOPMASK, V3D_QPU_M_SMUL24, 71 },
        { 10, 10, .raddr_mask = ANYOPMASK, V3D_QPU_M_MULTOP, 71 },

        { 14, 14, .raddr_mask = OP_RANGE(0, 2),   V3D_QPU_M_FMOV, 71 },
        { 14, 14, .raddr_mask = OP_RANGE(4, 6),   V3D_QPU_M_FMOV, 71 },
        { 14, 14, .raddr_mask = OP_RANGE(8, 10),  V3D_QPU_M_FMOV, 71 },
        { 14, 14, .raddr_mask = OP_RANGE(12, 14), V3D_QPU_M_FMOV, 71 },
        { 14, 14, .raddr_mask = OP_RANGE(16, 18), V3D_QPU_M_FMOV, 71 },
        { 14, 14, .raddr_mask = OP_RANGE(20, 22), V3D_QPU_M_FMOV, 71 },

        { 14, 14, .raddr_mask = OP_MASK(3),  V3D_QPU_M_MOV, 71 },
        { 14, 14, .raddr_mask = OP_MASK(7),  V3D_QPU_M_MOV, 71 },
        { 14, 14, .raddr_mask = OP_MASK(11), V3D_QPU_M_MOV, 71 },
        { 14, 14, .raddr_mask = OP_MASK(15), V3D_QPU_M_MOV, 71 },
        { 14, 14, .raddr_mask = OP_MASK(19), V3D_QPU_M_MOV, 71 },

        { 14, 14, .raddr_mask = OP_MASK(32), V3D_QPU_M_FTOUNORM16, 71 },
        { 14, 14, .raddr_mask = OP_MASK(33), V3D_QPU_M_FTOSNORM16, 71 },
        { 14, 14, .raddr_mask = OP_MASK(34), V3D_QPU_M_VFTOUNORM8, 71 },
        { 14, 14, .raddr_mask = OP_MASK(35), V3D_QPU_M_VFTOSNORM8, 71 },
        { 14, 14, .raddr_mask = OP_MASK(48), V3D_QPU_M_VFTOUNORM10LO, 71 },
        { 14, 14, .raddr_mask = OP_MASK(49), V3D_QPU_M_VFTOUNORM10HI, 71 },

        { 14, 14, .raddr_mask = OP_MASK(63), V3D_QPU_M_NOP, 71 },

        { 16, 63, .raddr_mask = ANYOPMASK, V3D_QPU_M_FMUL },
};

/* Returns true if op_desc should be filtered out based on devinfo->ver
 * against op_desc->first_ver and op_desc->last_ver. Check notes about
 * first_ver/last_ver on struct opcode_desc comments.
 */
static bool
opcode_invalid_in_version(const struct v3d_device_info *devinfo,
                          const uint8_t first_ver,
                          const uint8_t last_ver)
{
        return (first_ver != 0 && devinfo->ver < first_ver) ||
                (last_ver != 0  && devinfo->ver > last_ver);
}

/* Note that we pass as parameters mux_a, mux_b and raddr, even if depending
 * on the devinfo->ver some would be ignored. We do this way just to avoid
 * having two really similar lookup_opcode methods
 */
static const struct opcode_desc *
lookup_opcode_from_packed(const struct v3d_device_info *devinfo,
                          const struct opcode_desc *opcodes,
                          size_t num_opcodes, uint32_t opcode,
                          uint32_t mux_a, uint32_t mux_b,
                          uint32_t raddr)
{
        for (int i = 0; i < num_opcodes; i++) {
                const struct opcode_desc *op_desc = &opcodes[i];

                if (opcode < op_desc->opcode_first ||
                    opcode > op_desc->opcode_last)
                        continue;

                if (opcode_invalid_in_version(devinfo, op_desc->first_ver, op_desc->last_ver))
                        continue;

                if (devinfo->ver < 71) {
                        if (!(op_desc->mux.b_mask & (1 << mux_b)))
                                continue;

                        if (!(op_desc->mux.a_mask & (1 << mux_a)))
                                continue;
                } else {
                        if (!(op_desc->raddr_mask & ((uint64_t) 1 << raddr)))
                                continue;
                }

                return op_desc;
        }

        return NULL;
}

static bool
v3d_qpu_float32_unpack_unpack(uint32_t packed,
                              enum v3d_qpu_input_unpack *unpacked)
{
        switch (packed) {
        case 0:
                *unpacked = V3D_QPU_UNPACK_ABS;
                return true;
        case 1:
                *unpacked = V3D_QPU_UNPACK_NONE;
                return true;
        case 2:
                *unpacked = V3D_QPU_UNPACK_L;
                return true;
        case 3:
                *unpacked = V3D_QPU_UNPACK_H;
                return true;
        default:
                return false;
        }
}

static bool
v3d_qpu_float32_unpack_pack(enum v3d_qpu_input_unpack unpacked,
                            uint32_t *packed)
{
        switch (unpacked) {
        case V3D_QPU_UNPACK_ABS:
                *packed = 0;
                return true;
        case V3D_QPU_UNPACK_NONE:
                *packed = 1;
                return true;
        case V3D_QPU_UNPACK_L:
                *packed = 2;
                return true;
        case V3D_QPU_UNPACK_H:
                *packed = 3;
                return true;
        default:
                return false;
        }
}

static bool
v3d_qpu_int32_unpack_unpack(uint32_t packed,
                            enum v3d_qpu_input_unpack *unpacked)
{
        switch (packed) {
        case 0:
                *unpacked = V3D_QPU_UNPACK_NONE;
                return true;
        case 1:
                *unpacked = V3D_QPU_UNPACK_UL;
                return true;
        case 2:
                *unpacked = V3D_QPU_UNPACK_UH;
                return true;
        case 3:
                *unpacked = V3D_QPU_UNPACK_IL;
                return true;
        case 4:
                *unpacked = V3D_QPU_UNPACK_IH;
                return true;
        default:
                return false;
        }
}

static bool
v3d_qpu_int32_unpack_pack(enum v3d_qpu_input_unpack unpacked,
                          uint32_t *packed)
{
        switch (unpacked) {
        case V3D_QPU_UNPACK_NONE:
                *packed = 0;
                return true;
        case V3D_QPU_UNPACK_UL:
                *packed = 1;
                return true;
        case V3D_QPU_UNPACK_UH:
                *packed = 2;
                return true;
        case V3D_QPU_UNPACK_IL:
                *packed = 3;
                return true;
        case V3D_QPU_UNPACK_IH:
                *packed = 4;
                return true;
        default:
                return false;
        }
}

static bool
v3d_qpu_float16_unpack_unpack(uint32_t packed,
                              enum v3d_qpu_input_unpack *unpacked)
{
        switch (packed) {
        case 0:
                *unpacked = V3D_QPU_UNPACK_NONE;
                return true;
        case 1:
                *unpacked = V3D_QPU_UNPACK_REPLICATE_32F_16;
                return true;
        case 2:
                *unpacked = V3D_QPU_UNPACK_REPLICATE_L_16;
                return true;
        case 3:
                *unpacked = V3D_QPU_UNPACK_REPLICATE_H_16;
                return true;
        case 4:
                *unpacked = V3D_QPU_UNPACK_SWAP_16;
                return true;
        default:
                return false;
        }
}

static bool
v3d_qpu_float16_unpack_pack(enum v3d_qpu_input_unpack unpacked,
                            uint32_t *packed)
{
        switch (unpacked) {
        case V3D_QPU_UNPACK_NONE:
                *packed = 0;
                return true;
        case V3D_QPU_UNPACK_REPLICATE_32F_16:
                *packed = 1;
                return true;
        case V3D_QPU_UNPACK_REPLICATE_L_16:
                *packed = 2;
                return true;
        case V3D_QPU_UNPACK_REPLICATE_H_16:
                *packed = 3;
                return true;
        case V3D_QPU_UNPACK_SWAP_16:
                *packed = 4;
                return true;
        default:
                return false;
        }
}

static bool
v3d_qpu_float32_pack_pack(enum v3d_qpu_output_pack pack,
                          uint32_t *packed)
{
        switch (pack) {
        case V3D_QPU_PACK_NONE:
                *packed = 0;
                return true;
        case V3D_QPU_PACK_L:
                *packed = 1;
                return true;
        case V3D_QPU_PACK_H:
                *packed = 2;
                return true;
        default:
                return false;
        }
}

static bool
v3d33_qpu_add_unpack(const struct v3d_device_info *devinfo, uint64_t packed_inst,
                     struct v3d_qpu_instr *instr)
{
        uint32_t op = QPU_GET_FIELD(packed_inst, V3D_QPU_OP_ADD);
        uint32_t mux_a = QPU_GET_FIELD(packed_inst, V3D_QPU_ADD_A);
        uint32_t mux_b = QPU_GET_FIELD(packed_inst, V3D_QPU_ADD_B);
        uint32_t waddr = QPU_GET_FIELD(packed_inst, V3D_QPU_WADDR_A);

        uint32_t map_op = op;
        /* Some big clusters of opcodes are replicated with unpack
         * flags
         */
        if (map_op >= 249 && map_op <= 251)
                map_op = (map_op - 249 + 245);
        if (map_op >= 253 && map_op <= 255)
                map_op = (map_op - 253 + 245);

        const struct opcode_desc *desc =
                lookup_opcode_from_packed(devinfo, add_ops_v33,
                                          ARRAY_SIZE(add_ops_v33),
                                          map_op, mux_a, mux_b, 0);

        if (!desc)
                return false;

        instr->alu.add.op = desc->op;

        /* FADD/FADDNF and FMIN/FMAX are determined by the orders of the
         * operands.
         */
        if (((op >> 2) & 3) * 8 + mux_a > (op & 3) * 8 + mux_b) {
                if (instr->alu.add.op == V3D_QPU_A_FMIN)
                        instr->alu.add.op = V3D_QPU_A_FMAX;
                if (instr->alu.add.op == V3D_QPU_A_FADD)
                        instr->alu.add.op = V3D_QPU_A_FADDNF;
        }

        /* Some QPU ops require a bit more than just basic opcode and mux a/b
         * comparisons to distinguish them.
         */
        switch (instr->alu.add.op) {
        case V3D_QPU_A_STVPMV:
        case V3D_QPU_A_STVPMD:
        case V3D_QPU_A_STVPMP:
                switch (waddr) {
                case 0:
                        instr->alu.add.op = V3D_QPU_A_STVPMV;
                        break;
                case 1:
                        instr->alu.add.op = V3D_QPU_A_STVPMD;
                        break;
                case 2:
                        instr->alu.add.op = V3D_QPU_A_STVPMP;
                        break;
                default:
                        return false;
                }
                break;
        default:
                break;
        }

        switch (instr->alu.add.op) {
        case V3D_QPU_A_FADD:
        case V3D_QPU_A_FADDNF:
        case V3D_QPU_A_FSUB:
        case V3D_QPU_A_FMIN:
        case V3D_QPU_A_FMAX:
        case V3D_QPU_A_FCMP:
        case V3D_QPU_A_VFPACK:
                if (instr->alu.add.op != V3D_QPU_A_VFPACK)
                        instr->alu.add.output_pack = (op >> 4) & 0x3;
                else
                        instr->alu.add.output_pack = V3D_QPU_PACK_NONE;

                if (!v3d_qpu_float32_unpack_unpack((op >> 2) & 0x3,
                                                   &instr->alu.add.a.unpack)) {
                        return false;
                }

                if (!v3d_qpu_float32_unpack_unpack((op >> 0) & 0x3,
                                                   &instr->alu.add.b.unpack)) {
                        return false;
                }
                break;

        case V3D_QPU_A_FFLOOR:
        case V3D_QPU_A_FROUND:
        case V3D_QPU_A_FTRUNC:
        case V3D_QPU_A_FCEIL:
        case V3D_QPU_A_FDX:
        case V3D_QPU_A_FDY:
                instr->alu.add.output_pack = mux_b & 0x3;

                if (!v3d_qpu_float32_unpack_unpack((op >> 2) & 0x3,
                                                   &instr->alu.add.a.unpack)) {
                        return false;
                }
                break;

        case V3D_QPU_A_FTOIN:
        case V3D_QPU_A_FTOIZ:
        case V3D_QPU_A_FTOUZ:
        case V3D_QPU_A_FTOC:
                instr->alu.add.output_pack = V3D_QPU_PACK_NONE;

                if (!v3d_qpu_float32_unpack_unpack((op >> 2) & 0x3,
                                                   &instr->alu.add.a.unpack)) {
                        return false;
                }
                break;

        case V3D_QPU_A_VFMIN:
        case V3D_QPU_A_VFMAX:
                if (!v3d_qpu_float16_unpack_unpack(op & 0x7,
                                                   &instr->alu.add.a.unpack)) {
                        return false;
                }

                instr->alu.add.output_pack = V3D_QPU_PACK_NONE;
                instr->alu.add.b.unpack = V3D_QPU_UNPACK_NONE;
                break;

        default:
                instr->alu.add.output_pack = V3D_QPU_PACK_NONE;
                instr->alu.add.a.unpack = V3D_QPU_UNPACK_NONE;
                instr->alu.add.b.unpack = V3D_QPU_UNPACK_NONE;
                break;
        }

        instr->alu.add.a.mux = mux_a;
        instr->alu.add.b.mux = mux_b;
        instr->alu.add.waddr = QPU_GET_FIELD(packed_inst, V3D_QPU_WADDR_A);

        instr->alu.add.magic_write = false;
        if (packed_inst & V3D_QPU_MA) {
                switch (instr->alu.add.op) {
                case V3D_QPU_A_LDVPMV_IN:
                        instr->alu.add.op = V3D_QPU_A_LDVPMV_OUT;
                        break;
                case V3D_QPU_A_LDVPMD_IN:
                        instr->alu.add.op = V3D_QPU_A_LDVPMD_OUT;
                        break;
                case V3D_QPU_A_LDVPMG_IN:
                        instr->alu.add.op = V3D_QPU_A_LDVPMG_OUT;
                        break;
                default:
                        instr->alu.add.magic_write = true;
                        break;
                }
        }

        return true;
}

static bool
v3d71_qpu_add_unpack(const struct v3d_device_info *devinfo, uint64_t packed_inst,
                     struct v3d_qpu_instr *instr)
{
        uint32_t op = QPU_GET_FIELD(packed_inst, V3D_QPU_OP_ADD);
        uint32_t raddr_a = QPU_GET_FIELD(packed_inst, V3D_QPU_RADDR_A);
        uint32_t raddr_b = QPU_GET_FIELD(packed_inst, V3D_QPU_RADDR_B);
        uint32_t waddr = QPU_GET_FIELD(packed_inst, V3D_QPU_WADDR_A);
        uint32_t map_op = op;

        const struct opcode_desc *desc =
                lookup_opcode_from_packed(devinfo,
                                          add_ops_v71,
                                          ARRAY_SIZE(add_ops_v71),
                                          map_op, 0, 0,
                                          raddr_b);
        if (!desc)
                return false;

        instr->alu.add.op = desc->op;

        /* FADD/FADDNF and FMIN/FMAX are determined by the order of the
         * operands.
         */
        if (instr->sig.small_imm_a * 256 + ((op >> 2) & 3) * 64 + raddr_a >
            instr->sig.small_imm_b * 256 + (op & 3) * 64 + raddr_b) {
                if (instr->alu.add.op == V3D_QPU_A_FMIN)
                        instr->alu.add.op = V3D_QPU_A_FMAX;
                if (instr->alu.add.op == V3D_QPU_A_FADD)
                        instr->alu.add.op = V3D_QPU_A_FADDNF;
        }

        /* Some QPU ops require a bit more than just basic opcode and mux a/b
         * comparisons to distinguish them.
         */
        switch (instr->alu.add.op) {
        case V3D_QPU_A_STVPMV:
        case V3D_QPU_A_STVPMD:
        case V3D_QPU_A_STVPMP:
                switch (waddr) {
                case 0:
                        instr->alu.add.op = V3D_QPU_A_STVPMV;
                        break;
                case 1:
                        instr->alu.add.op = V3D_QPU_A_STVPMD;
                        break;
                case 2:
                        instr->alu.add.op = V3D_QPU_A_STVPMP;
                        break;
                default:
                        return false;
                }
                break;
        default:
                break;
        }

        switch (instr->alu.add.op) {
        case V3D_QPU_A_FADD:
        case V3D_QPU_A_FADDNF:
        case V3D_QPU_A_FSUB:
        case V3D_QPU_A_FMIN:
        case V3D_QPU_A_FMAX:
        case V3D_QPU_A_FCMP:
        case V3D_QPU_A_VFPACK:
                if (instr->alu.add.op != V3D_QPU_A_VFPACK &&
                    instr->alu.add.op != V3D_QPU_A_FCMP) {
                        instr->alu.add.output_pack = (op >> 4) & 0x3;
                } else {
                        instr->alu.add.output_pack = V3D_QPU_PACK_NONE;
                }

                if (!v3d_qpu_float32_unpack_unpack((op >> 2) & 0x3,
                                                   &instr->alu.add.a.unpack)) {
                        return false;
                }

                if (!v3d_qpu_float32_unpack_unpack((op >> 0) & 0x3,
                                                   &instr->alu.add.b.unpack)) {
                        return false;
                }
                break;

        case V3D_QPU_A_FFLOOR:
        case V3D_QPU_A_FROUND:
        case V3D_QPU_A_FTRUNC:
        case V3D_QPU_A_FCEIL:
        case V3D_QPU_A_FDX:
        case V3D_QPU_A_FDY:
                instr->alu.add.output_pack = raddr_b & 0x3;

                if (!v3d_qpu_float32_unpack_unpack((op >> 2) & 0x3,
                                                   &instr->alu.add.a.unpack)) {
                        return false;
                }
                break;

        case V3D_QPU_A_FTOIN:
        case V3D_QPU_A_FTOIZ:
        case V3D_QPU_A_FTOUZ:
        case V3D_QPU_A_FTOC:
                instr->alu.add.output_pack = V3D_QPU_PACK_NONE;

                if (!v3d_qpu_float32_unpack_unpack((raddr_b >> 2) & 0x3,
                                                   &instr->alu.add.a.unpack)) {
                        return false;
                }
                break;

        case V3D_QPU_A_VFMIN:
        case V3D_QPU_A_VFMAX:
                unreachable("pending v71 update");
                if (!v3d_qpu_float16_unpack_unpack(op & 0x7,
                                                   &instr->alu.add.a.unpack)) {
                        return false;
                }

                instr->alu.add.output_pack = V3D_QPU_PACK_NONE;
                instr->alu.add.b.unpack = V3D_QPU_UNPACK_NONE;
                break;

        case V3D_QPU_A_MOV:
                instr->alu.add.output_pack = V3D_QPU_PACK_NONE;

                if (!v3d_qpu_int32_unpack_unpack((raddr_b >> 2) & 0x7,
                                                 &instr->alu.add.a.unpack)) {
                        return false;
                }
                break;

        case V3D_QPU_A_FMOV:
                instr->alu.add.output_pack = raddr_b & 0x3;

                /* Mul alu FMOV has one additional variant */
                int32_t unpack = (raddr_b >> 2) & 0x7;
                if (unpack == 7)
                        return false;

                if (!v3d_qpu_float32_unpack_unpack(unpack,
                                                   &instr->alu.add.a.unpack)) {
                        return false;
                }
                break;

        default:
                instr->alu.add.output_pack = V3D_QPU_PACK_NONE;
                instr->alu.add.a.unpack = V3D_QPU_UNPACK_NONE;
                instr->alu.add.b.unpack = V3D_QPU_UNPACK_NONE;
                break;
        }

        instr->alu.add.a.raddr = raddr_a;
        instr->alu.add.b.raddr = raddr_b;
        instr->alu.add.waddr = QPU_GET_FIELD(packed_inst, V3D_QPU_WADDR_A);

        instr->alu.add.magic_write = false;
        if (packed_inst & V3D_QPU_MA) {
                switch (instr->alu.add.op) {
                case V3D_QPU_A_LDVPMV_IN:
                        instr->alu.add.op = V3D_QPU_A_LDVPMV_OUT;
                        break;
                case V3D_QPU_A_LDVPMD_IN:
                        instr->alu.add.op = V3D_QPU_A_LDVPMD_OUT;
                        break;
                case V3D_QPU_A_LDVPMG_IN:
                        instr->alu.add.op = V3D_QPU_A_LDVPMG_OUT;
                        break;
                default:
                        instr->alu.add.magic_write = true;
                        break;
                }
        }

        return true;
}

static bool
v3d_qpu_add_unpack(const struct v3d_device_info *devinfo, uint64_t packed_inst,
                   struct v3d_qpu_instr *instr)
{
        if (devinfo->ver < 71)
                return v3d33_qpu_add_unpack(devinfo, packed_inst, instr);
        else
                return v3d71_qpu_add_unpack(devinfo, packed_inst, instr);
}

static bool
v3d33_qpu_mul_unpack(const struct v3d_device_info *devinfo, uint64_t packed_inst,
                     struct v3d_qpu_instr *instr)
{
        uint32_t op = QPU_GET_FIELD(packed_inst, V3D_QPU_OP_MUL);
        uint32_t mux_a = QPU_GET_FIELD(packed_inst, V3D_QPU_MUL_A);
        uint32_t mux_b = QPU_GET_FIELD(packed_inst, V3D_QPU_MUL_B);

        {
                const struct opcode_desc *desc =
                        lookup_opcode_from_packed(devinfo,
                                                  mul_ops_v33,
                                                  ARRAY_SIZE(mul_ops_v33),
                                                  op, mux_a, mux_b, 0);
                if (!desc)
                        return false;

                instr->alu.mul.op = desc->op;
        }

        switch (instr->alu.mul.op) {
        case V3D_QPU_M_FMUL:
                instr->alu.mul.output_pack = ((op >> 4) & 0x3) - 1;

                if (!v3d_qpu_float32_unpack_unpack((op >> 2) & 0x3,
                                                   &instr->alu.mul.a.unpack)) {
                        return false;
                }

                if (!v3d_qpu_float32_unpack_unpack((op >> 0) & 0x3,
                                                   &instr->alu.mul.b.unpack)) {
                        return false;
                }

                break;

        case V3D_QPU_M_FMOV:
                instr->alu.mul.output_pack = (((op & 1) << 1) +
                                              ((mux_b >> 2) & 1));

                if (!v3d_qpu_float32_unpack_unpack(mux_b & 0x3,
                                                   &instr->alu.mul.a.unpack)) {
                        return false;
                }

                break;

        case V3D_QPU_M_VFMUL:
                instr->alu.mul.output_pack = V3D_QPU_PACK_NONE;

                if (!v3d_qpu_float16_unpack_unpack(((op & 0x7) - 4) & 7,
                                                   &instr->alu.mul.a.unpack)) {
                        return false;
                }

                instr->alu.mul.b.unpack = V3D_QPU_UNPACK_NONE;

                break;

        default:
                instr->alu.mul.output_pack = V3D_QPU_PACK_NONE;
                instr->alu.mul.a.unpack = V3D_QPU_UNPACK_NONE;
                instr->alu.mul.b.unpack = V3D_QPU_UNPACK_NONE;
                break;
        }

        instr->alu.mul.a.mux = mux_a;
        instr->alu.mul.b.mux = mux_b;
        instr->alu.mul.waddr = QPU_GET_FIELD(packed_inst, V3D_QPU_WADDR_M);
        instr->alu.mul.magic_write = packed_inst & V3D_QPU_MM;

        return true;
}

static bool
v3d71_qpu_mul_unpack(const struct v3d_device_info *devinfo, uint64_t packed_inst,
                     struct v3d_qpu_instr *instr)
{
        uint32_t op = QPU_GET_FIELD(packed_inst, V3D_QPU_OP_MUL);
        uint32_t raddr_c = QPU_GET_FIELD(packed_inst, V3D_QPU_RADDR_C);
        uint32_t raddr_d = QPU_GET_FIELD(packed_inst, V3D_QPU_RADDR_D);

        {
                const struct opcode_desc *desc =
                        lookup_opcode_from_packed(devinfo,
                                                  mul_ops_v71,
                                                  ARRAY_SIZE(mul_ops_v71),
                                                  op, 0, 0,
                                                  raddr_d);
                if (!desc)
                        return false;

                instr->alu.mul.op = desc->op;
        }

        switch (instr->alu.mul.op) {
        case V3D_QPU_M_FMUL:
                instr->alu.mul.output_pack = ((op >> 4) & 0x3) - 1;

                if (!v3d_qpu_float32_unpack_unpack((op >> 2) & 0x3,
                                                   &instr->alu.mul.a.unpack)) {
                        return false;
                }

                if (!v3d_qpu_float32_unpack_unpack((op >> 0) & 0x3,
                                                   &instr->alu.mul.b.unpack)) {
                        return false;
                }

                break;

        case V3D_QPU_M_FMOV:
                instr->alu.mul.output_pack = raddr_d & 0x3;

                if (!v3d_qpu_float32_unpack_unpack((raddr_d >> 2) & 0x7,
                                                   &instr->alu.mul.a.unpack)) {
                        return false;
                }

                break;

        case V3D_QPU_M_VFMUL:
                unreachable("pending v71 update");
                instr->alu.mul.output_pack = V3D_QPU_PACK_NONE;

                if (!v3d_qpu_float16_unpack_unpack(((op & 0x7) - 4) & 7,
                                                   &instr->alu.mul.a.unpack)) {
                        return false;
                }

                instr->alu.mul.b.unpack = V3D_QPU_UNPACK_NONE;

                break;

        case V3D_QPU_M_MOV:
                instr->alu.mul.output_pack = V3D_QPU_PACK_NONE;

                if (!v3d_qpu_int32_unpack_unpack((raddr_d >> 2) & 0x7,
                                                 &instr->alu.mul.a.unpack)) {
                        return false;
                }
                break;

        default:
                instr->alu.mul.output_pack = V3D_QPU_PACK_NONE;
                instr->alu.mul.a.unpack = V3D_QPU_UNPACK_NONE;
                instr->alu.mul.b.unpack = V3D_QPU_UNPACK_NONE;
                break;
        }

        instr->alu.mul.a.raddr = raddr_c;
        instr->alu.mul.b.raddr = raddr_d;
        instr->alu.mul.waddr = QPU_GET_FIELD(packed_inst, V3D_QPU_WADDR_M);
        instr->alu.mul.magic_write = packed_inst & V3D_QPU_MM;

        return true;
}

static bool
v3d_qpu_mul_unpack(const struct v3d_device_info *devinfo, uint64_t packed_inst,
                   struct v3d_qpu_instr *instr)
{
        if (devinfo->ver < 71)
                return v3d33_qpu_mul_unpack(devinfo, packed_inst, instr);
        else
                return v3d71_qpu_mul_unpack(devinfo, packed_inst, instr);
}

static const struct opcode_desc *
lookup_opcode_from_instr(const struct v3d_device_info *devinfo,
                         const struct opcode_desc *opcodes, size_t num_opcodes,
                         uint8_t op)
{
        for (int i = 0; i < num_opcodes; i++) {
                const struct opcode_desc *op_desc = &opcodes[i];

                if (op_desc->op != op)
                        continue;

                if (opcode_invalid_in_version(devinfo, op_desc->first_ver, op_desc->last_ver))
                        continue;

                return op_desc;
        }

        return NULL;
}

static bool
v3d33_qpu_add_pack(const struct v3d_device_info *devinfo,
                   const struct v3d_qpu_instr *instr, uint64_t *packed_instr)
{
        uint32_t waddr = instr->alu.add.waddr;
        uint32_t mux_a = instr->alu.add.a.mux;
        uint32_t mux_b = instr->alu.add.b.mux;
        int nsrc = v3d_qpu_add_op_num_src(instr->alu.add.op);
        const struct opcode_desc *desc =
                lookup_opcode_from_instr(devinfo, add_ops_v33,
                                         ARRAY_SIZE(add_ops_v33),
                                         instr->alu.add.op);

        if (!desc)
                return false;

        uint32_t opcode = desc->opcode_first;

        /* If an operation doesn't use an arg, its mux values may be used to
         * identify the operation type.
         */
        if (nsrc < 2)
                mux_b = ffs(desc->mux.b_mask) - 1;

        if (nsrc < 1)
                mux_a = ffs(desc->mux.a_mask) - 1;

        bool no_magic_write = false;

        switch (instr->alu.add.op) {
        case V3D_QPU_A_STVPMV:
                waddr = 0;
                no_magic_write = true;
                break;
        case V3D_QPU_A_STVPMD:
                waddr = 1;
                no_magic_write = true;
                break;
        case V3D_QPU_A_STVPMP:
                waddr = 2;
                no_magic_write = true;
                break;

        case V3D_QPU_A_LDVPMV_IN:
        case V3D_QPU_A_LDVPMD_IN:
        case V3D_QPU_A_LDVPMP:
        case V3D_QPU_A_LDVPMG_IN:
                assert(!instr->alu.add.magic_write);
                break;

        case V3D_QPU_A_LDVPMV_OUT:
        case V3D_QPU_A_LDVPMD_OUT:
        case V3D_QPU_A_LDVPMG_OUT:
                assert(!instr->alu.add.magic_write);
                *packed_instr |= V3D_QPU_MA;
                break;

        default:
                break;
        }

        switch (instr->alu.add.op) {
        case V3D_QPU_A_FADD:
        case V3D_QPU_A_FADDNF:
        case V3D_QPU_A_FSUB:
        case V3D_QPU_A_FMIN:
        case V3D_QPU_A_FMAX:
        case V3D_QPU_A_FCMP: {
                uint32_t output_pack;
                uint32_t a_unpack;
                uint32_t b_unpack;

                if (!v3d_qpu_float32_pack_pack(instr->alu.add.output_pack,
                                               &output_pack)) {
                        return false;
                }
                opcode |= output_pack << 4;

                if (!v3d_qpu_float32_unpack_pack(instr->alu.add.a.unpack,
                                                 &a_unpack)) {
                        return false;
                }

                if (!v3d_qpu_float32_unpack_pack(instr->alu.add.b.unpack,
                                                 &b_unpack)) {
                        return false;
                }

                /* These operations with commutative operands are
                 * distinguished by which order their operands come in.
                 */
                bool ordering = a_unpack * 8 + mux_a > b_unpack * 8 + mux_b;
                if (((instr->alu.add.op == V3D_QPU_A_FMIN ||
                      instr->alu.add.op == V3D_QPU_A_FADD) && ordering) ||
                    ((instr->alu.add.op == V3D_QPU_A_FMAX ||
                      instr->alu.add.op == V3D_QPU_A_FADDNF) && !ordering)) {
                        uint32_t temp;

                        temp = a_unpack;
                        a_unpack = b_unpack;
                        b_unpack = temp;

                        temp = mux_a;
                        mux_a = mux_b;
                        mux_b = temp;
                }

                opcode |= a_unpack << 2;
                opcode |= b_unpack << 0;

                break;
        }

        case V3D_QPU_A_VFPACK: {
                uint32_t a_unpack;
                uint32_t b_unpack;

                if (instr->alu.add.a.unpack == V3D_QPU_UNPACK_ABS ||
                    instr->alu.add.b.unpack == V3D_QPU_UNPACK_ABS) {
                        return false;
                }

                if (!v3d_qpu_float32_unpack_pack(instr->alu.add.a.unpack,
                                                 &a_unpack)) {
                        return false;
                }

                if (!v3d_qpu_float32_unpack_pack(instr->alu.add.b.unpack,
                                                 &b_unpack)) {
                        return false;
                }

                opcode = (opcode & ~(0x3 << 2)) | (a_unpack << 2);
                opcode = (opcode & ~(0x3 << 0)) | (b_unpack << 0);

                break;
        }

        case V3D_QPU_A_FFLOOR:
        case V3D_QPU_A_FROUND:
        case V3D_QPU_A_FTRUNC:
        case V3D_QPU_A_FCEIL:
        case V3D_QPU_A_FDX:
        case V3D_QPU_A_FDY: {
                uint32_t packed;

                if (!v3d_qpu_float32_pack_pack(instr->alu.add.output_pack,
                                               &packed)) {
                        return false;
                }
                mux_b |= packed;

                if (!v3d_qpu_float32_unpack_pack(instr->alu.add.a.unpack,
                                                 &packed)) {
                        return false;
                }
                if (packed == 0)
                        return false;
                opcode = (opcode & ~(0x3 << 2)) | packed << 2;
                break;
        }

        case V3D_QPU_A_FTOIN:
        case V3D_QPU_A_FTOIZ:
        case V3D_QPU_A_FTOUZ:
        case V3D_QPU_A_FTOC:
                if (instr->alu.add.output_pack != V3D_QPU_PACK_NONE)
                        return false;

                uint32_t packed;
                if (!v3d_qpu_float32_unpack_pack(instr->alu.add.a.unpack,
                                                 &packed)) {
                        return false;
                }
                if (packed == 0)
                        return false;
                opcode |= packed << 2;

                break;

        case V3D_QPU_A_VFMIN:
        case V3D_QPU_A_VFMAX:
                if (instr->alu.add.output_pack != V3D_QPU_PACK_NONE ||
                    instr->alu.add.b.unpack != V3D_QPU_UNPACK_NONE) {
                        return false;
                }

                if (!v3d_qpu_float16_unpack_pack(instr->alu.add.a.unpack,
                                                 &packed)) {
                        return false;
                }
                opcode |= packed;
                break;

        default:
                if (instr->alu.add.op != V3D_QPU_A_NOP &&
                    (instr->alu.add.output_pack != V3D_QPU_PACK_NONE ||
                     instr->alu.add.a.unpack != V3D_QPU_UNPACK_NONE ||
                     instr->alu.add.b.unpack != V3D_QPU_UNPACK_NONE)) {
                        return false;
                }
                break;
        }

        *packed_instr |= QPU_SET_FIELD(mux_a, V3D_QPU_ADD_A);
        *packed_instr |= QPU_SET_FIELD(mux_b, V3D_QPU_ADD_B);
        *packed_instr |= QPU_SET_FIELD(opcode, V3D_QPU_OP_ADD);
        *packed_instr |= QPU_SET_FIELD(waddr, V3D_QPU_WADDR_A);
        if (instr->alu.add.magic_write && !no_magic_write)
                *packed_instr |= V3D_QPU_MA;

        return true;
}

static bool
v3d71_qpu_add_pack(const struct v3d_device_info *devinfo,
                   const struct v3d_qpu_instr *instr, uint64_t *packed_instr)
{
        uint32_t waddr = instr->alu.add.waddr;
        uint32_t raddr_a = instr->alu.add.a.raddr;
        uint32_t raddr_b = instr->alu.add.b.raddr;

        int nsrc = v3d_qpu_add_op_num_src(instr->alu.add.op);
        const struct opcode_desc *desc =
                lookup_opcode_from_instr(devinfo, add_ops_v71,
                                         ARRAY_SIZE(add_ops_v71),
                                         instr->alu.add.op);
        if (!desc)
                return false;

        uint32_t opcode = desc->opcode_first;

        /* If an operation doesn't use an arg, its raddr values may be used to
         * identify the operation type.
         */
        if (nsrc < 2)
                raddr_b = ffsll(desc->raddr_mask) - 1;

        bool no_magic_write = false;

        switch (instr->alu.add.op) {
        case V3D_QPU_A_STVPMV:
                waddr = 0;
                no_magic_write = true;
                break;
        case V3D_QPU_A_STVPMD:
                waddr = 1;
                no_magic_write = true;
                break;
        case V3D_QPU_A_STVPMP:
                waddr = 2;
                no_magic_write = true;
                break;

        case V3D_QPU_A_LDVPMV_IN:
        case V3D_QPU_A_LDVPMD_IN:
        case V3D_QPU_A_LDVPMP:
        case V3D_QPU_A_LDVPMG_IN:
                assert(!instr->alu.add.magic_write);
                break;

        case V3D_QPU_A_LDVPMV_OUT:
        case V3D_QPU_A_LDVPMD_OUT:
        case V3D_QPU_A_LDVPMG_OUT:
                assert(!instr->alu.add.magic_write);
                *packed_instr |= V3D_QPU_MA;
                break;

        default:
                break;
        }

        switch (instr->alu.add.op) {
        case V3D_QPU_A_FADD:
        case V3D_QPU_A_FADDNF:
        case V3D_QPU_A_FSUB:
        case V3D_QPU_A_FMIN:
        case V3D_QPU_A_FMAX:
        case V3D_QPU_A_FCMP: {
                uint32_t output_pack;
                uint32_t a_unpack;
                uint32_t b_unpack;

                if (instr->alu.add.op != V3D_QPU_A_FCMP) {
                        if (!v3d_qpu_float32_pack_pack(instr->alu.add.output_pack,
                                                       &output_pack)) {
                                return false;
                        }
                        opcode |= output_pack << 4;
                }

                if (!v3d_qpu_float32_unpack_pack(instr->alu.add.a.unpack,
                                                 &a_unpack)) {
                        return false;
                }

                if (!v3d_qpu_float32_unpack_pack(instr->alu.add.b.unpack,
                                                 &b_unpack)) {
                        return false;
                }

                /* These operations with commutative operands are
                 * distinguished by the order of the operands come in.
                 */
                bool ordering =
                        instr->sig.small_imm_a * 256 + a_unpack * 64 + raddr_a >
                        instr->sig.small_imm_b * 256 + b_unpack * 64 + raddr_b;
                if (((instr->alu.add.op == V3D_QPU_A_FMIN ||
                      instr->alu.add.op == V3D_QPU_A_FADD) && ordering) ||
                    ((instr->alu.add.op == V3D_QPU_A_FMAX ||
                      instr->alu.add.op == V3D_QPU_A_FADDNF) && !ordering)) {
                        uint32_t temp;

                        temp = a_unpack;
                        a_unpack = b_unpack;
                        b_unpack = temp;

                        temp = raddr_a;
                        raddr_a = raddr_b;
                        raddr_b = temp;

                        /* If we are swapping raddr_a/b we also need to swap
                         * small_imm_a/b.
                         */
                        if (instr->sig.small_imm_a || instr->sig.small_imm_b) {
                                assert(instr->sig.small_imm_a !=
                                       instr->sig.small_imm_b);
                                struct v3d_qpu_sig new_sig = instr->sig;
                                new_sig.small_imm_a = !instr->sig.small_imm_a;
                                new_sig.small_imm_b = !instr->sig.small_imm_b;
                                uint32_t sig;
                                if (!v3d_qpu_sig_pack(devinfo, &new_sig, &sig))
                                    return false;
                            *packed_instr &= ~V3D_QPU_SIG_MASK;
                            *packed_instr |= QPU_SET_FIELD(sig, V3D_QPU_SIG);
                        }
                }

                opcode |= a_unpack << 2;
                opcode |= b_unpack << 0;

                break;
        }

        case V3D_QPU_A_VFPACK: {
                uint32_t a_unpack;
                uint32_t b_unpack;

                if (instr->alu.add.a.unpack == V3D_QPU_UNPACK_ABS ||
                    instr->alu.add.b.unpack == V3D_QPU_UNPACK_ABS) {
                        return false;
                }

                if (!v3d_qpu_float32_unpack_pack(instr->alu.add.a.unpack,
                                                 &a_unpack)) {
                        return false;
                }

                if (!v3d_qpu_float32_unpack_pack(instr->alu.add.b.unpack,
                                                 &b_unpack)) {
                        return false;
                }

                opcode = (opcode & ~(0x3 << 2)) | (a_unpack << 2);
                opcode = (opcode & ~(0x3 << 0)) | (b_unpack << 0);

                break;
        }

        case V3D_QPU_A_FFLOOR:
        case V3D_QPU_A_FROUND:
        case V3D_QPU_A_FTRUNC:
        case V3D_QPU_A_FCEIL:
        case V3D_QPU_A_FDX:
        case V3D_QPU_A_FDY: {
                uint32_t packed;

                if (!v3d_qpu_float32_pack_pack(instr->alu.add.output_pack,
                                               &packed)) {
                        return false;
                }
                raddr_b |= packed;

                if (!v3d_qpu_float32_unpack_pack(instr->alu.add.a.unpack,
                                                 &packed)) {
                        return false;
                }
                if (packed == 0)
                        return false;
                raddr_b = (raddr_b & ~(0x3 << 2)) | packed << 2;
                break;
        }

        case V3D_QPU_A_FTOIN:
        case V3D_QPU_A_FTOIZ:
        case V3D_QPU_A_FTOUZ:
        case V3D_QPU_A_FTOC:
                if (instr->alu.add.output_pack != V3D_QPU_PACK_NONE)
                        return false;

                uint32_t packed;
                if (!v3d_qpu_float32_unpack_pack(instr->alu.add.a.unpack,
                                                 &packed)) {
                        return false;
                }
                if (packed == 0)
                        return false;

                raddr_b |= (raddr_b & ~(0x3 << 2)) | packed << 2;

                break;

        case V3D_QPU_A_VFMIN:
        case V3D_QPU_A_VFMAX:
                if (instr->alu.add.output_pack != V3D_QPU_PACK_NONE ||
                    instr->alu.add.b.unpack != V3D_QPU_UNPACK_NONE) {
                        return false;
                }

                if (!v3d_qpu_float16_unpack_pack(instr->alu.add.a.unpack,
                                                 &packed)) {
                        return false;
                }
                opcode |= packed;
                break;

        case V3D_QPU_A_MOV: {
                uint32_t packed;

                if (instr->alu.add.output_pack != V3D_QPU_PACK_NONE)
                        return false;

                if (!v3d_qpu_int32_unpack_pack(instr->alu.add.a.unpack,
                                               &packed)) {
                        return false;
                }

                raddr_b |= packed << 2;
                break;
        }

        case V3D_QPU_A_FMOV: {
                uint32_t packed;

                if (!v3d_qpu_float32_pack_pack(instr->alu.add.output_pack,
                                               &packed)) {
                        return false;
                }
                raddr_b = packed;

                if (!v3d_qpu_float32_unpack_pack(instr->alu.add.a.unpack,
                                                 &packed)) {
                        return false;
                }
                raddr_b |= packed << 2;
                break;
        }

        default:
                if (instr->alu.add.op != V3D_QPU_A_NOP &&
                    (instr->alu.add.output_pack != V3D_QPU_PACK_NONE ||
                     instr->alu.add.a.unpack != V3D_QPU_UNPACK_NONE ||
                     instr->alu.add.b.unpack != V3D_QPU_UNPACK_NONE)) {
                        return false;
                }
                break;
        }

        *packed_instr |= QPU_SET_FIELD(raddr_a, V3D_QPU_RADDR_A);
        *packed_instr |= QPU_SET_FIELD(raddr_b, V3D_QPU_RADDR_B);
        *packed_instr |= QPU_SET_FIELD(opcode, V3D_QPU_OP_ADD);
        *packed_instr |= QPU_SET_FIELD(waddr, V3D_QPU_WADDR_A);
        if (instr->alu.add.magic_write && !no_magic_write)
                *packed_instr |= V3D_QPU_MA;

        return true;
}

static bool
v3d33_qpu_mul_pack(const struct v3d_device_info *devinfo,
                   const struct v3d_qpu_instr *instr, uint64_t *packed_instr)
{
        uint32_t mux_a = instr->alu.mul.a.mux;
        uint32_t mux_b = instr->alu.mul.b.mux;
        int nsrc = v3d_qpu_mul_op_num_src(instr->alu.mul.op);

        const struct opcode_desc *desc =
                lookup_opcode_from_instr(devinfo, mul_ops_v33,
                                         ARRAY_SIZE(mul_ops_v33),
                                         instr->alu.mul.op);

        if (!desc)
                return false;

        uint32_t opcode = desc->opcode_first;

        /* Some opcodes have a single valid value for their mux a/b, so set
         * that here.  If mux a/b determine packing, it will be set below.
         */
        if (nsrc < 2)
                mux_b = ffs(desc->mux.b_mask) - 1;

        if (nsrc < 1)
                mux_a = ffs(desc->mux.a_mask) - 1;

        switch (instr->alu.mul.op) {
        case V3D_QPU_M_FMUL: {
                uint32_t packed;

                if (!v3d_qpu_float32_pack_pack(instr->alu.mul.output_pack,
                                               &packed)) {
                        return false;
                }
                /* No need for a +1 because desc->opcode_first has a 1 in this
                 * field.
                 */
                opcode += packed << 4;

                if (!v3d_qpu_float32_unpack_pack(instr->alu.mul.a.unpack,
                                                 &packed)) {
                        return false;
                }
                opcode |= packed << 2;

                if (!v3d_qpu_float32_unpack_pack(instr->alu.mul.b.unpack,
                                                 &packed)) {
                        return false;
                }
                opcode |= packed << 0;
                break;
        }

        case V3D_QPU_M_FMOV: {
                uint32_t packed;

                if (!v3d_qpu_float32_pack_pack(instr->alu.mul.output_pack,
                                               &packed)) {
                        return false;
                }
                opcode |= (packed >> 1) & 1;
                mux_b = (packed & 1) << 2;

                if (!v3d_qpu_float32_unpack_pack(instr->alu.mul.a.unpack,
                                                 &packed)) {
                        return false;
                }
                mux_b |= packed;
                break;
        }

        case V3D_QPU_M_VFMUL: {
                uint32_t packed;

                if (instr->alu.mul.output_pack != V3D_QPU_PACK_NONE)
                        return false;

                if (!v3d_qpu_float16_unpack_pack(instr->alu.mul.a.unpack,
                                                 &packed)) {
                        return false;
                }
                if (instr->alu.mul.a.unpack == V3D_QPU_UNPACK_SWAP_16)
                        opcode = 8;
                else
                        opcode |= (packed + 4) & 7;

                if (instr->alu.mul.b.unpack != V3D_QPU_UNPACK_NONE)
                        return false;

                break;
        }

        default:
                if (instr->alu.mul.op != V3D_QPU_M_NOP &&
                    (instr->alu.mul.output_pack != V3D_QPU_PACK_NONE ||
                     instr->alu.mul.a.unpack != V3D_QPU_UNPACK_NONE ||
                     instr->alu.mul.b.unpack != V3D_QPU_UNPACK_NONE)) {
                        return false;
                }
                break;
        }

        *packed_instr |= QPU_SET_FIELD(mux_a, V3D_QPU_MUL_A);
        *packed_instr |= QPU_SET_FIELD(mux_b, V3D_QPU_MUL_B);

        *packed_instr |= QPU_SET_FIELD(opcode, V3D_QPU_OP_MUL);
        *packed_instr |= QPU_SET_FIELD(instr->alu.mul.waddr, V3D_QPU_WADDR_M);
        if (instr->alu.mul.magic_write)
                *packed_instr |= V3D_QPU_MM;

        return true;
}

static bool
v3d71_qpu_mul_pack(const struct v3d_device_info *devinfo,
                   const struct v3d_qpu_instr *instr, uint64_t *packed_instr)
{
        uint32_t raddr_c = instr->alu.mul.a.raddr;
        uint32_t raddr_d = instr->alu.mul.b.raddr;
        int nsrc = v3d_qpu_mul_op_num_src(instr->alu.mul.op);

        const struct opcode_desc *desc =
                lookup_opcode_from_instr(devinfo, mul_ops_v71,
                                         ARRAY_SIZE(mul_ops_v71),
                                         instr->alu.mul.op);
        if (!desc)
                return false;

        uint32_t opcode = desc->opcode_first;

        /* Some opcodes have a single valid value for their raddr_d, so set
         * that here.  If raddr_b determine packing, it will be set below.
         */
        if (nsrc < 2)
                raddr_d = ffsll(desc->raddr_mask) - 1;

        switch (instr->alu.mul.op) {
        case V3D_QPU_M_FMUL: {
                uint32_t packed;

                if (!v3d_qpu_float32_pack_pack(instr->alu.mul.output_pack,
                                               &packed)) {
                        return false;
                }
                /* No need for a +1 because desc->opcode_first has a 1 in this
                 * field.
                 */
                opcode += packed << 4;

                if (!v3d_qpu_float32_unpack_pack(instr->alu.mul.a.unpack,
                                                 &packed)) {
                        return false;
                }
                opcode |= packed << 2;

                if (!v3d_qpu_float32_unpack_pack(instr->alu.mul.b.unpack,
                                                 &packed)) {
                        return false;
                }
                opcode |= packed << 0;
                break;
        }

        case V3D_QPU_M_FMOV: {
                uint32_t packed;

                if (!v3d_qpu_float32_pack_pack(instr->alu.mul.output_pack,
                                               &packed)) {
                        return false;
                }
                raddr_d |= packed;

                if (!v3d_qpu_float32_unpack_pack(instr->alu.mul.a.unpack,
                                                 &packed)) {
                        return false;
                }
                raddr_d |= packed << 2;
                break;
        }

        case V3D_QPU_M_VFMUL: {
                unreachable("pending v71 update");
                uint32_t packed;

                if (instr->alu.mul.output_pack != V3D_QPU_PACK_NONE)
                        return false;

                if (!v3d_qpu_float16_unpack_pack(instr->alu.mul.a.unpack,
                                                 &packed)) {
                        return false;
                }
                if (instr->alu.mul.a.unpack == V3D_QPU_UNPACK_SWAP_16)
                        opcode = 8;
                else
                        opcode |= (packed + 4) & 7;

                if (instr->alu.mul.b.unpack != V3D_QPU_UNPACK_NONE)
                        return false;

                break;
        }

        case V3D_QPU_M_MOV: {
                uint32_t packed;

                if (instr->alu.mul.output_pack != V3D_QPU_PACK_NONE)
                        return false;

                if (!v3d_qpu_int32_unpack_pack(instr->alu.mul.a.unpack,
                                               &packed)) {
                        return false;
                }

                raddr_d |= packed << 2;
                break;
        }

        default:
                if (instr->alu.mul.op != V3D_QPU_M_NOP &&
                    (instr->alu.mul.output_pack != V3D_QPU_PACK_NONE ||
                     instr->alu.mul.a.unpack != V3D_QPU_UNPACK_NONE ||
                     instr->alu.mul.b.unpack != V3D_QPU_UNPACK_NONE)) {
                        return false;
                }
                break;
        }

        *packed_instr |= QPU_SET_FIELD(raddr_c, V3D_QPU_RADDR_C);
        *packed_instr |= QPU_SET_FIELD(raddr_d, V3D_QPU_RADDR_D);
        *packed_instr |= QPU_SET_FIELD(opcode, V3D_QPU_OP_MUL);
        *packed_instr |= QPU_SET_FIELD(instr->alu.mul.waddr, V3D_QPU_WADDR_M);
        if (instr->alu.mul.magic_write)
                *packed_instr |= V3D_QPU_MM;

        return true;
}

static bool
v3d_qpu_add_pack(const struct v3d_device_info *devinfo,
                 const struct v3d_qpu_instr *instr, uint64_t *packed_instr)
{
        if (devinfo->ver < 71)
                return v3d33_qpu_add_pack(devinfo, instr, packed_instr);
        else
                return v3d71_qpu_add_pack(devinfo, instr, packed_instr);
}

static bool
v3d_qpu_mul_pack(const struct v3d_device_info *devinfo,
                 const struct v3d_qpu_instr *instr, uint64_t *packed_instr)
{
        if (devinfo->ver < 71)
                return v3d33_qpu_mul_pack(devinfo, instr, packed_instr);
        else
                return v3d71_qpu_mul_pack(devinfo, instr, packed_instr);
}

static bool
v3d_qpu_instr_unpack_alu(const struct v3d_device_info *devinfo,
                         uint64_t packed_instr,
                         struct v3d_qpu_instr *instr)
{
        instr->type = V3D_QPU_INSTR_TYPE_ALU;

        if (!v3d_qpu_sig_unpack(devinfo,
                                QPU_GET_FIELD(packed_instr, V3D_QPU_SIG),
                                &instr->sig))
                return false;

        uint32_t packed_cond = QPU_GET_FIELD(packed_instr, V3D_QPU_COND);
        if (v3d_qpu_sig_writes_address(devinfo, &instr->sig)) {
                instr->sig_addr = packed_cond & ~V3D_QPU_COND_SIG_MAGIC_ADDR;
                instr->sig_magic = packed_cond & V3D_QPU_COND_SIG_MAGIC_ADDR;

                instr->flags.ac = V3D_QPU_COND_NONE;
                instr->flags.mc = V3D_QPU_COND_NONE;
                instr->flags.apf = V3D_QPU_PF_NONE;
                instr->flags.mpf = V3D_QPU_PF_NONE;
                instr->flags.auf = V3D_QPU_UF_NONE;
                instr->flags.muf = V3D_QPU_UF_NONE;
        } else {
                if (!v3d_qpu_flags_unpack(devinfo, packed_cond, &instr->flags))
                        return false;
        }

        if (devinfo->ver <= 71) {
                /*
                 * For v71 this will be set on add/mul unpack, as raddr are now
                 * part of v3d_qpu_input
                 */
                instr->raddr_a = QPU_GET_FIELD(packed_instr, V3D_QPU_RADDR_A);
                instr->raddr_b = QPU_GET_FIELD(packed_instr, V3D_QPU_RADDR_B);
        }

        if (!v3d_qpu_add_unpack(devinfo, packed_instr, instr))
                return false;

        if (!v3d_qpu_mul_unpack(devinfo, packed_instr, instr))
                return false;

        return true;
}

static bool
v3d_qpu_instr_unpack_branch(const struct v3d_device_info *devinfo,
                            uint64_t packed_instr,
                            struct v3d_qpu_instr *instr)
{
        instr->type = V3D_QPU_INSTR_TYPE_BRANCH;

        uint32_t cond = QPU_GET_FIELD(packed_instr, V3D_QPU_BRANCH_COND);
        if (cond == 0)
                instr->branch.cond = V3D_QPU_BRANCH_COND_ALWAYS;
        else if (V3D_QPU_BRANCH_COND_A0 + (cond - 2) <=
                 V3D_QPU_BRANCH_COND_ALLNA)
                instr->branch.cond = V3D_QPU_BRANCH_COND_A0 + (cond - 2);
        else
                return false;

        uint32_t msfign = QPU_GET_FIELD(packed_instr, V3D_QPU_BRANCH_MSFIGN);
        if (msfign == 3)
                return false;
        instr->branch.msfign = msfign;

        instr->branch.bdi = QPU_GET_FIELD(packed_instr, V3D_QPU_BRANCH_BDI);

        instr->branch.ub = packed_instr & V3D_QPU_BRANCH_UB;
        if (instr->branch.ub) {
                instr->branch.bdu = QPU_GET_FIELD(packed_instr,
                                                  V3D_QPU_BRANCH_BDU);
        }

        instr->branch.raddr_a = QPU_GET_FIELD(packed_instr,
                                              V3D_QPU_RADDR_A);

        instr->branch.offset = 0;

        instr->branch.offset +=
                QPU_GET_FIELD(packed_instr,
                              V3D_QPU_BRANCH_ADDR_LOW) << 3;

        instr->branch.offset +=
                QPU_GET_FIELD(packed_instr,
                              V3D_QPU_BRANCH_ADDR_HIGH) << 24;

        return true;
}

bool
v3d_qpu_instr_unpack(const struct v3d_device_info *devinfo,
                     uint64_t packed_instr,
                     struct v3d_qpu_instr *instr)
{
        if (QPU_GET_FIELD(packed_instr, V3D_QPU_OP_MUL) != 0) {
                return v3d_qpu_instr_unpack_alu(devinfo, packed_instr, instr);
        } else {
                uint32_t sig = QPU_GET_FIELD(packed_instr, V3D_QPU_SIG);

                if ((sig & 24) == 16) {
                        return v3d_qpu_instr_unpack_branch(devinfo, packed_instr,
                                                           instr);
                } else {
                        return false;
                }
        }
}

static bool
v3d_qpu_instr_pack_alu(const struct v3d_device_info *devinfo,
                       const struct v3d_qpu_instr *instr,
                       uint64_t *packed_instr)
{
        uint32_t sig;
        if (!v3d_qpu_sig_pack(devinfo, &instr->sig, &sig))
                return false;
        *packed_instr |= QPU_SET_FIELD(sig, V3D_QPU_SIG);

        if (instr->type == V3D_QPU_INSTR_TYPE_ALU) {
                if (devinfo->ver < 71) {
                        /*
                         * For v71 this will be set on add/mul unpack, as raddr are now
                         * part of v3d_qpu_input
                         */
                        *packed_instr |= QPU_SET_FIELD(instr->raddr_a, V3D_QPU_RADDR_A);
                        *packed_instr |= QPU_SET_FIELD(instr->raddr_b, V3D_QPU_RADDR_B);
                }

                if (!v3d_qpu_add_pack(devinfo, instr, packed_instr))
                        return false;
                if (!v3d_qpu_mul_pack(devinfo, instr, packed_instr))
                        return false;

                uint32_t flags;
                if (v3d_qpu_sig_writes_address(devinfo, &instr->sig)) {
                        if (instr->flags.ac != V3D_QPU_COND_NONE ||
                            instr->flags.mc != V3D_QPU_COND_NONE ||
                            instr->flags.apf != V3D_QPU_PF_NONE ||
                            instr->flags.mpf != V3D_QPU_PF_NONE ||
                            instr->flags.auf != V3D_QPU_UF_NONE ||
                            instr->flags.muf != V3D_QPU_UF_NONE) {
                                return false;
                        }

                        flags = instr->sig_addr;
                        if (instr->sig_magic)
                                flags |= V3D_QPU_COND_SIG_MAGIC_ADDR;
                } else {
                        if (!v3d_qpu_flags_pack(devinfo, &instr->flags, &flags))
                                return false;
                }

                *packed_instr |= QPU_SET_FIELD(flags, V3D_QPU_COND);
        } else {
                if (v3d_qpu_sig_writes_address(devinfo, &instr->sig))
                        return false;
        }

        return true;
}

static bool
v3d_qpu_instr_pack_branch(const struct v3d_device_info *devinfo,
                          const struct v3d_qpu_instr *instr,
                          uint64_t *packed_instr)
{
        *packed_instr |= QPU_SET_FIELD(16, V3D_QPU_SIG);

        if (instr->branch.cond != V3D_QPU_BRANCH_COND_ALWAYS) {
                *packed_instr |= QPU_SET_FIELD(2 + (instr->branch.cond -
                                                    V3D_QPU_BRANCH_COND_A0),
                                               V3D_QPU_BRANCH_COND);
        }

        *packed_instr |= QPU_SET_FIELD(instr->branch.msfign,
                                       V3D_QPU_BRANCH_MSFIGN);

        *packed_instr |= QPU_SET_FIELD(instr->branch.bdi,
                                       V3D_QPU_BRANCH_BDI);

        if (instr->branch.ub) {
                *packed_instr |= V3D_QPU_BRANCH_UB;
                *packed_instr |= QPU_SET_FIELD(instr->branch.bdu,
                                               V3D_QPU_BRANCH_BDU);
        }

        switch (instr->branch.bdi) {
        case V3D_QPU_BRANCH_DEST_ABS:
        case V3D_QPU_BRANCH_DEST_REL:
                *packed_instr |= QPU_SET_FIELD(instr->branch.msfign,
                                               V3D_QPU_BRANCH_MSFIGN);

                *packed_instr |= QPU_SET_FIELD((instr->branch.offset &
                                                ~0xff000000) >> 3,
                                               V3D_QPU_BRANCH_ADDR_LOW);

                *packed_instr |= QPU_SET_FIELD(instr->branch.offset >> 24,
                                               V3D_QPU_BRANCH_ADDR_HIGH);
                break;
        default:
                break;
        }

        if (instr->branch.bdi == V3D_QPU_BRANCH_DEST_REGFILE ||
            instr->branch.bdu == V3D_QPU_BRANCH_DEST_REGFILE) {
                *packed_instr |= QPU_SET_FIELD(instr->branch.raddr_a,
                                               V3D_QPU_RADDR_A);
        }

        return true;
}

bool
v3d_qpu_instr_pack(const struct v3d_device_info *devinfo,
                   const struct v3d_qpu_instr *instr,
                   uint64_t *packed_instr)
{
        *packed_instr = 0;

        switch (instr->type) {
        case V3D_QPU_INSTR_TYPE_ALU:
                return v3d_qpu_instr_pack_alu(devinfo, instr, packed_instr);
        case V3D_QPU_INSTR_TYPE_BRANCH:
                return v3d_qpu_instr_pack_branch(devinfo, instr, packed_instr);
        default:
                return false;
        }
}
