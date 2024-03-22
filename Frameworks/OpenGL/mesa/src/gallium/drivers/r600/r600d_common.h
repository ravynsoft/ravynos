/*
 * Copyright 2013 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Marek Olšák <maraeo@gmail.com>
 */

#ifndef R600D_COMMON_H
#define R600D_COMMON_H

#define R600_CONFIG_REG_OFFSET	0x08000
#define R600_CONTEXT_REG_OFFSET 0x28000
#define SI_SH_REG_OFFSET                     0x0000B000
#define SI_SH_REG_END                        0x0000C000
#define CIK_UCONFIG_REG_OFFSET               0x00030000
#define CIK_UCONFIG_REG_END                  0x00038000

#define PKT_TYPE_S(x)                   (((unsigned)(x) & 0x3) << 30)
#define PKT_COUNT_S(x)                  (((unsigned)(x) & 0x3FFF) << 16)
#define PKT3_IT_OPCODE_S(x)             (((unsigned)(x) & 0xFF) << 8)
#define PKT3_PREDICATE(x)               (((x) >> 0) & 0x1)
#define PKT3(op, count, predicate) (PKT_TYPE_S(3) | PKT_COUNT_S(count) | PKT3_IT_OPCODE_S(op) | PKT3_PREDICATE(predicate))

#define PKT3_NOP                               0x10
#define PKT3_SET_PREDICATION                   0x20
#define PKT3_STRMOUT_BUFFER_UPDATE             0x34
#define		STRMOUT_STORE_BUFFER_FILLED_SIZE	1
#define		STRMOUT_OFFSET_SOURCE(x)	(((unsigned)(x) & 0x3) << 1)
#define			STRMOUT_OFFSET_FROM_PACKET		0
#define			STRMOUT_OFFSET_FROM_VGT_FILLED_SIZE	1
#define			STRMOUT_OFFSET_FROM_MEM			2
#define			STRMOUT_OFFSET_NONE			3
#define		STRMOUT_SELECT_BUFFER(x)	(((unsigned)(x) & 0x3) << 8)
#define PKT3_WAIT_REG_MEM                      0x3C
#define		WAIT_REG_MEM_EQUAL		3
#define		WAIT_REG_MEM_GEQUAL		5
#define		WAIT_REG_MEM_MEMORY		(1 << 4)
#define         WAIT_REG_MEM_MEM_SPACE(x)       (((unsigned)(x) & 0x3) << 4)
#define PKT3_COPY_DATA			       0x40
#define		COPY_DATA_SRC_SEL(x)		((x) & 0xf)
#define			COPY_DATA_REG		0
#define			COPY_DATA_MEM		1
#define                 COPY_DATA_PERF          4
#define                 COPY_DATA_IMM           5
#define                 COPY_DATA_TIMESTAMP     9
#define		COPY_DATA_DST_SEL(x)		(((unsigned)(x) & 0xf) << 8)
#define                 COPY_DATA_MEM_ASYNC     5
#define		COPY_DATA_COUNT_SEL		(1 << 16)
#define		COPY_DATA_WR_CONFIRM		(1 << 20)
#define PKT3_EVENT_WRITE                       0x46
#define PKT3_EVENT_WRITE_EOP                   0x47
#define         EOP_INT_SEL(x)                          ((x) << 24)
#define			EOP_INT_SEL_NONE			0
#define			EOP_INT_SEL_SEND_DATA_AFTER_WR_CONFIRM	3
#define         EOP_DATA_SEL(x)                         ((x) << 29)
#define			EOP_DATA_SEL_DISCARD		0
#define			EOP_DATA_SEL_VALUE_32BIT	1
#define			EOP_DATA_SEL_VALUE_64BIT	2
#define			EOP_DATA_SEL_TIMESTAMP		3
#define PKT3_RELEASE_MEM                       0x49 /* GFX9+ */
#define PKT3_SET_CONFIG_REG		       0x68
#define PKT3_SET_CONTEXT_REG		       0x69
#define PKT3_STRMOUT_BASE_UPDATE	       0x72 /* r700 only */
#define PKT3_SURFACE_BASE_UPDATE               0x73 /* r600 only */
#define		SURFACE_BASE_UPDATE_DEPTH      (1 << 0)
#define		SURFACE_BASE_UPDATE_COLOR(x)   (2 << (x))
#define		SURFACE_BASE_UPDATE_COLOR_NUM(x) (((1 << x) - 1) << 1)
#define		SURFACE_BASE_UPDATE_STRMOUT(x) (0x200 << (x))
#define PKT3_SET_SH_REG                        0x76 /* SI and later */
#define PKT3_SET_UCONFIG_REG                   0x79 /* GFX7 and later */

#define EVENT_TYPE_SAMPLE_STREAMOUTSTATS1      0x1 /* EG and later */
#define EVENT_TYPE_SAMPLE_STREAMOUTSTATS2      0x2 /* EG and later */
#define EVENT_TYPE_SAMPLE_STREAMOUTSTATS3      0x3 /* EG and later */
#define EVENT_TYPE_PS_PARTIAL_FLUSH            0x10
#define EVENT_TYPE_CACHE_FLUSH_AND_INV_TS_EVENT 0x14
#define EVENT_TYPE_ZPASS_DONE                  0x15
#define EVENT_TYPE_CACHE_FLUSH_AND_INV_EVENT   0x16
#define EVENT_TYPE_PERFCOUNTER_START            0x17
#define EVENT_TYPE_PERFCOUNTER_STOP             0x18
#define EVENT_TYPE_PIPELINESTAT_START		25
#define EVENT_TYPE_PIPELINESTAT_STOP		26
#define EVENT_TYPE_PERFCOUNTER_SAMPLE           0x1B
#define EVENT_TYPE_SAMPLE_PIPELINESTAT		30
#define EVENT_TYPE_SO_VGTSTREAMOUT_FLUSH	0x1f
#define EVENT_TYPE_SAMPLE_STREAMOUTSTATS	0x20
#define EVENT_TYPE_BOTTOM_OF_PIPE_TS		40
#define EVENT_TYPE_FLUSH_AND_INV_DB_META       0x2c /* supported on r700+ */
#define EVENT_TYPE_FLUSH_AND_INV_CB_META	46 /* supported on r700+ */
#define		EVENT_TYPE(x)                           ((x) << 0)
#define		EVENT_INDEX(x)                          ((x) << 8)
                /* 0 - any non-TS event
		 * 1 - ZPASS_DONE
		 * 2 - SAMPLE_PIPELINESTAT
		 * 3 - SAMPLE_STREAMOUTSTAT*
		 * 4 - *S_PARTIAL_FLUSH
		 * 5 - TS events
		 */

#define PREDICATION_OP_CLEAR 0x0
#define PREDICATION_OP_ZPASS 0x1
#define PREDICATION_OP_PRIMCOUNT 0x2
#define PREDICATION_OP_BOOL64 0x3
#define PRED_OP(x) ((x) << 16)
#define PREDICATION_CONTINUE (1 << 31)
#define PREDICATION_HINT_WAIT (0 << 12)
#define PREDICATION_HINT_NOWAIT_DRAW (1 << 12)
#define PREDICATION_DRAW_NOT_VISIBLE (0 << 8)
#define PREDICATION_DRAW_VISIBLE (1 << 8)

#define     V_0280A0_SWAP_STD                          0x00000000
#define     V_0280A0_SWAP_ALT                          0x00000001
#define     V_0280A0_SWAP_STD_REV                      0x00000002
#define     V_0280A0_SWAP_ALT_REV                      0x00000003

#define   EG_S_028C70_FAST_CLEAR(x)                       (((unsigned)(x) & 0x1) << 17)
#define   SI_S_028C70_FAST_CLEAR(x)                       (((unsigned)(x) & 0x1) << 13)

#define R600_MAX_ALU_CONST_BUFFERS 16

#endif
