/*
 * Copyright (C) 2005 Ben Skeggs.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "r300_fragprog.h"

#include <stdio.h>

#include "r300_reg.h"

static void presub_string(char out[10], unsigned int inst)
{
	switch(inst & 0x600000){
	case R300_ALU_SRCP_1_MINUS_2_SRC0:
		sprintf(out, "bias");
		break;
	case R300_ALU_SRCP_SRC1_MINUS_SRC0:
		sprintf(out, "sub");
		break;
	case R300_ALU_SRCP_SRC1_PLUS_SRC0:
		sprintf(out, "add");
		break;
	case R300_ALU_SRCP_1_MINUS_SRC0:
		sprintf(out, "inv ");
		break;
	}
}

static int get_msb(unsigned int bit, unsigned int r400_ext_addr)
{
	return (r400_ext_addr & bit) ? 1 << 5 : 0;
}

/* just some random things... */
void r300FragmentProgramDump(struct radeon_compiler *c, void *user)
{
	struct r300_fragment_program_compiler *compiler = (struct r300_fragment_program_compiler*)c;
	struct r300_fragment_program_code *code = &compiler->code->code.r300;
	int n, i, j;
	static int pc = 0;

	fprintf(stderr, "pc=%d*************************************\n", pc++);

	fprintf(stderr, "Hardware program\n");
	fprintf(stderr, "----------------\n");
	if (c->is_r400) {
		fprintf(stderr, "code_offset_ext: %08x\n", code->r400_code_offset_ext);
	}

	for (n = 0; n <= (code->config & 3); n++) {
		uint32_t code_addr = code->code_addr[3 - (code->config & 3) + n];
		unsigned int alu_offset = ((code_addr & R300_ALU_START_MASK) >> R300_ALU_START_SHIFT) +
				(((code->r400_code_offset_ext >> (24 - (n * 6))) & 0x7) << 6);
		unsigned int alu_end = ((code_addr & R300_ALU_SIZE_MASK) >> R300_ALU_SIZE_SHIFT) +
				(((code->r400_code_offset_ext >> (27 - (n * 6))) & 0x7) << 6);
		int tex_offset = (code_addr & R300_TEX_START_MASK) >> R300_TEX_START_SHIFT;
		int tex_end = (code_addr & R300_TEX_SIZE_MASK) >> R300_TEX_SIZE_SHIFT;

		fprintf(stderr, "NODE %d: alu_offset: %u, tex_offset: %d, "
			"alu_end: %u, tex_end: %d  (code_addr: %08x)\n", n,
			alu_offset, tex_offset, alu_end, tex_end, code_addr);

		if (n > 0 || (code->config & R300_PFS_CNTL_FIRST_NODE_HAS_TEX)) {
			fprintf(stderr, "  TEX:\n");
			for (i = tex_offset;
			     i <= tex_offset + tex_end;
			     ++i) {
				const char *instr;

				switch ((code->tex.
					 inst[i] >> R300_TEX_INST_SHIFT) &
					15) {
				case R300_TEX_OP_LD:
					instr = "TEX";
					break;
				case R300_TEX_OP_KIL:
					instr = "KIL";
					break;
				case R300_TEX_OP_TXP:
					instr = "TXP";
					break;
				case R300_TEX_OP_TXB:
					instr = "TXB";
					break;
				default:
					instr = "UNKNOWN";
				}

				fprintf(stderr,
					"    %s t%i, %c%i, texture[%i]   (%08x)\n",
					instr,
					(code->tex.
					 inst[i] >> R300_DST_ADDR_SHIFT) & 31,
					't',
					(code->tex.
					 inst[i] >> R300_SRC_ADDR_SHIFT) & 31,
					(code->tex.
					 inst[i] & R300_TEX_ID_MASK) >>
					R300_TEX_ID_SHIFT,
					code->tex.inst[i]);
			}
		}

		for (i = alu_offset;
		     i <= alu_offset + alu_end; ++i) {
			char srcc[4][10], dstc[20];
			char srca[4][10], dsta[20];
			char argc[3][20];
			char arga[3][20];
			char flags[5], tmp[10];

			for (j = 0; j < 3; ++j) {
				int regc = code->alu.inst[i].rgb_addr >> (j * 6);
				int rega = code->alu.inst[i].alpha_addr >> (j * 6);
				int msbc = get_msb(R400_ADDR_EXT_RGB_MSB_BIT(j),
					code->alu.inst[i].r400_ext_addr);
				int msba = get_msb(R400_ADDR_EXT_A_MSB_BIT(j),
					code->alu.inst[i].r400_ext_addr);

				sprintf(srcc[j], "%c%i",
					(regc & 32) ? 'c' : 't', (regc & 31) | msbc);
				sprintf(srca[j], "%c%i",
					(rega & 32) ? 'c' : 't', (rega & 31) | msba);
			}

			dstc[0] = 0;
			sprintf(flags, "%s%s%s",
				(code->alu.inst[i].
				 rgb_addr & R300_ALU_DSTC_REG_X) ? "x" : "",
				(code->alu.inst[i].
				 rgb_addr & R300_ALU_DSTC_REG_Y) ? "y" : "",
				(code->alu.inst[i].
				 rgb_addr & R300_ALU_DSTC_REG_Z) ? "z" : "");
			if (flags[0] != 0) {
				unsigned int msb = get_msb(
					R400_ADDRD_EXT_RGB_MSB_BIT,
					code->alu.inst[i].r400_ext_addr);

				sprintf(dstc, "t%i.%s ",
					((code->alu.inst[i].
					 rgb_addr >> R300_ALU_DSTC_SHIFT)
					 & 31) | msb,
					flags);
			}
			sprintf(flags, "%s%s%s",
				(code->alu.inst[i].
				 rgb_addr & R300_ALU_DSTC_OUTPUT_X) ? "x" : "",
				(code->alu.inst[i].
				 rgb_addr & R300_ALU_DSTC_OUTPUT_Y) ? "y" : "",
				(code->alu.inst[i].
				 rgb_addr & R300_ALU_DSTC_OUTPUT_Z) ? "z" : "");
			if (flags[0] != 0) {
				sprintf(tmp, "o%i.%s",
					(code->alu.inst[i].
					 rgb_addr >> 29) & 3,
					flags);
				strcat(dstc, tmp);
			}
			/* Presub */
			presub_string(srcc[3], code->alu.inst[i].rgb_inst);
			presub_string(srca[3], code->alu.inst[i].alpha_inst);

			dsta[0] = 0;
			if (code->alu.inst[i].alpha_addr & R300_ALU_DSTA_REG) {
				unsigned int msb = get_msb(
					R400_ADDRD_EXT_A_MSB_BIT,
					code->alu.inst[i].r400_ext_addr);
				sprintf(dsta, "t%i.w ",
					((code->alu.inst[i].
					 alpha_addr >> R300_ALU_DSTA_SHIFT) & 31)
					 | msb);
			}
			if (code->alu.inst[i].alpha_addr & R300_ALU_DSTA_OUTPUT) {
				sprintf(tmp, "o%i.w ",
					(code->alu.inst[i].
					 alpha_addr >> 25) & 3);
				strcat(dsta, tmp);
			}
			if (code->alu.inst[i].alpha_addr & R300_ALU_DSTA_DEPTH) {
				strcat(dsta, "Z");
			}

			fprintf(stderr,
				"%3i: xyz: %3s %3s %3s %5s-> %-20s (%08x)\n"
				"       w: %3s %3s %3s %5s-> %-20s (%08x)\n", i,
				srcc[0], srcc[1], srcc[2], srcc[3], dstc,
				code->alu.inst[i].rgb_addr, srca[0], srca[1],
				srca[2], srca[3], dsta,
				code->alu.inst[i].alpha_addr);

			for (j = 0; j < 3; ++j) {
				int regc = code->alu.inst[i].rgb_inst >> (j * 7);
				int rega = code->alu.inst[i].alpha_inst >> (j * 7);
				int d;
				char buf[20];

				d = regc & 31;
				if (d < 12) {
					switch (d % 4) {
					case R300_ALU_ARGC_SRC0C_XYZ:
						sprintf(buf, "%s.xyz",
							srcc[d / 4]);
						break;
					case R300_ALU_ARGC_SRC0C_XXX:
						sprintf(buf, "%s.xxx",
							srcc[d / 4]);
						break;
					case R300_ALU_ARGC_SRC0C_YYY:
						sprintf(buf, "%s.yyy",
							srcc[d / 4]);
						break;
					case R300_ALU_ARGC_SRC0C_ZZZ:
						sprintf(buf, "%s.zzz",
							srcc[d / 4]);
						break;
					}
				} else if (d < 15) {
					sprintf(buf, "%s.www", srca[d - 12]);
				} else if (d < 20 ) {
					switch(d) {
					case R300_ALU_ARGC_SRCP_XYZ:
						sprintf(buf, "srcp.xyz");
						break;
					case R300_ALU_ARGC_SRCP_XXX:
						sprintf(buf, "srcp.xxx");
						break;
					case R300_ALU_ARGC_SRCP_YYY:
						sprintf(buf, "srcp.yyy");
						break;
					case R300_ALU_ARGC_SRCP_ZZZ:
						sprintf(buf, "srcp.zzz");
						break;
					case R300_ALU_ARGC_SRCP_WWW:
						sprintf(buf, "srcp.www");
						break;
					}
				} else if (d == 20) {
					sprintf(buf, "0.0");
				} else if (d == 21) {
					sprintf(buf, "1.0");
				} else if (d == 22) {
					sprintf(buf, "0.5");
				} else if (d >= 23 && d < 32) {
					d -= 23;
					switch (d / 3) {
					case 0:
						sprintf(buf, "%s.yzx",
							srcc[d % 3]);
						break;
					case 1:
						sprintf(buf, "%s.zxy",
							srcc[d % 3]);
						break;
					case 2:
						sprintf(buf, "%s.Wzy",
							srcc[d % 3]);
						break;
					}
				} else {
					sprintf(buf, "%i", d);
				}

				sprintf(argc[j], "%s%s%s%s",
					(regc & 32) ? "-" : "",
					(regc & 64) ? "|" : "",
					buf, (regc & 64) ? "|" : "");

				d = rega & 31;
				if (d < 9) {
					sprintf(buf, "%s.%c", srcc[d / 3],
						'x' + (char)(d % 3));
				} else if (d < 12) {
					sprintf(buf, "%s.w", srca[d - 9]);
				} else if (d < 16) {
					switch(d) {
					case R300_ALU_ARGA_SRCP_X:
						sprintf(buf, "srcp.x");
						break;
					case R300_ALU_ARGA_SRCP_Y:
						sprintf(buf, "srcp.y");
						break;
					case R300_ALU_ARGA_SRCP_Z:
						sprintf(buf, "srcp.z");
						break;
					case R300_ALU_ARGA_SRCP_W:
						sprintf(buf, "srcp.w");
						break;
					}
				} else if (d == 16) {
					sprintf(buf, "0.0");
				} else if (d == 17) {
					sprintf(buf, "1.0");
				} else if (d == 18) {
					sprintf(buf, "0.5");
				} else {
					sprintf(buf, "%i", d);
				}

				sprintf(arga[j], "%s%s%s%s",
					(rega & 32) ? "-" : "",
					(rega & 64) ? "|" : "",
					buf, (rega & 64) ? "|" : "");
			}

			fprintf(stderr, "     xyz: %8s %8s %8s    op: %08x %s\n"
				"       w: %8s %8s %8s    op: %08x\n",
				argc[0], argc[1], argc[2],
				code->alu.inst[i].rgb_inst,
				code->alu.inst[i].rgb_inst & R300_ALU_INSERT_NOP ?
				"NOP" : "",
				arga[0], arga[1],arga[2],
				code->alu.inst[i].alpha_inst);
		}
	}
}
