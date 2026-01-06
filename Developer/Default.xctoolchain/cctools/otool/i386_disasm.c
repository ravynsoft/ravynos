/*
 * Copyright © 2009 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1.  Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * This file contains the i386 disassembler routine used at NeXT Computer, Inc.
 * to match the the assembler used at NeXT.  It was addapted from a set of
 * source files with the following copyright which is retained below.
 */
/*
  Copyright 1988, 1989 by Intel Corporation, Santa Clara, California.

		All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Intel
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> /* cctools-port: For bcmp, bzero ... */
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/reloc.h>
#include <mach-o/x86_64/reloc.h>
#include "stuff/symbol.h"
#include "stuff/bytesex.h"
#include "stuff/llvm.h"
#include "stuff/allocate.h"
#include "otool.h"
#include "dyld_bind_info.h"
#include "ofile_print.h"
#include "i386_disasm.h"
#include "cxa_demangle.h"

#define MAX_MNEMONIC	16	/* Maximum number of chars per mnemonic, plus a byte for '\0' */
#define MAX_RESULT	14	/* Maximum number of char in a register */
				/*  result expression "(%ebx,%ecx,8)" */

#define WBIT(x)	(x & 0x1)		/* to get w bit	*/
#define REGNO(x) (x & 0x7)		/* to get 3 bit register */
#define VBIT(x)	((x)>>1 & 0x1)		/* to get 'v' bit */
#define OPSIZE(data16,wbit,maybe64) ((wbit) ? ((data16) ? 2: ((maybe64) ? 8 : 4)) : 1 )
#define REX_W(x) (((x) & 0x8) == 0x8)	/* true if the REX.W bit is set --> 64-bit operand size */
#define REX_R(x) (((x) & 0x4) == 0x4)	/* true if the REX.R bit is set --> ModRM reg extension */
#define REX_X(x) (((x) & 0x2) == 0x2)	/* true if the REX.X bit is set --> SIB index extension */
#define REX_B(x) (((x) & 0x1) == 0x1)	/* true if the REX.B bit is set --> ModRM r/m, SIB base, or opcode reg extension */

#define REG_ONLY 3	/* mode indicates a single register with	*/
			/* no displacement is an operand		*/
#define BYTEOPERAND 0	/* value of the w-bit indicating a byte		*/
			/* operand (1-byte)				*/
#define LONGOPERAND 1	/* value of the w-bit indicating a long		*/
			/* operand (2-bytes or 4-bytes)			*/
#define EBP 5
#define ESP 4

/*
 * This is the structure that is used for storing all the op code information.
 */
struct instable {
    char name[MAX_MNEMONIC];
    const struct instable *indirect;
    unsigned adr_mode;
    int flags;
    const struct instable *arch64;
};
#define	TERM	0	/* used to indicate that the 'indirect' field of the */
			/* 'instable' terminates - no pointer.	*/
#define	INVALID	{"",TERM,UNKNOWN,0}
/*
 * These are defined this way to make the initializations in the tables simpler
 * and more readable for differences between 32-bit and 64-bit architectures.
 */
#define	INVALID_32 "",TERM,UNKNOWN,0
static const struct instable op_invalid_64 = {"",TERM,/* UNKNOWN */0,0};
#define INVALID_64 (&op_invalid_64)

/* Flags */
#define HAS_SUFFIX			0x1	/* For instructions which may have a 'w', 'l', or 'q' suffix */
#define IS_POINTER_SIZED	0x2	/* For instructions which implicitly have operands which are sizeof(void *) */

static void get_operand(
    const char **symadd,
    const char **symsub,
    uint32_t *value,
    uint32_t *value_size,
    char *result,
    const cpu_type_t cputype,
    const uint32_t mode,
    const uint32_t r_m,
    const uint32_t wbit,
    const enum bool data16,
    const enum bool addr16,
    const enum bool sse2,
    const enum bool mmx,
    const unsigned int rex,
    const char *sect,
    uint64_t sect_addr,
    uint32_t *length,
    uint32_t *left,
    const uint64_t addr,
    const struct relocation_info *sorted_relocs,
    const uint32_t nsorted_relocs,
    const struct relocation_info *ext_relocs,
    const uint32_t next_relocs,
    const struct nlist *symbols,
    const struct nlist_64 *symbols64,
    const uint32_t nsymbols,
    const char *strings,
    const uint32_t strings_size,
    const struct symbol *sorted_symbols,
    const uint32_t nsorted_symbols,
    const enum bool verbose);

static void immediate(
    const char **symadd,
    const char **symsub,
    uint64_t *value,
    uint32_t value_size,
    const char *sect,
    uint64_t sect_addr,
    uint32_t *length,
    uint32_t *left,
    const cpu_type_t cputype,
    const uint64_t addr,
    const struct relocation_info *sorted_relocs,
    const uint32_t nsorted_relocs,
    const struct relocation_info *ext_relocs,
    const uint32_t next_relocs,
    const struct nlist *symbols,
    const struct nlist_64 *symbols64,
    const uint32_t nsymbols,
    const char *strings,
    const uint32_t strings_size,
    const struct symbol *sorted_symbols,
    const uint32_t nsorted_symbols,
    const enum bool verbose);

static void displacement(
    const char **symadd,
    const char **symsub,
    uint64_t *value,
    const uint32_t value_size,
    const char *sect,
    uint64_t sect_addr,
    uint32_t *length,
    uint32_t *left,
    const cpu_type_t cputype,
    const uint64_t addr,
    const struct relocation_info *sorted_relocs,
    const uint32_t nsorted_relocs,
    const struct relocation_info *ext_relocs,
    const uint32_t next_relocs,
    const struct nlist *symbols,
    const struct nlist_64 *symbols64,
    const uint32_t nsymbols,
    const char *strings,
    const uint32_t strings_size,
    const struct symbol *sorted_symbols,
    const uint32_t nsorted_symbols,
    const enum bool verbose);

static void get_symbol(
    const char **symadd,
    const char **symsub,
    uint64_t *offset,
    const cpu_type_t cputype,
    const uint64_t sect_offset,
    const uint64_t seg_offset,
    const uint64_t value,
    const struct relocation_info *relocs,
    const uint32_t nrelocs,
    const struct relocation_info *ext_relocs,
    const uint32_t next_relocs,
    const struct nlist *symbols,
    const struct nlist_64 *symbols64,
    const uint32_t nsymbols,
    const char *strings,
    const uint32_t strings_size,
    const struct symbol *sorted_symbols,
    const uint32_t nsorted_symbols,
    const enum bool verbose);

static void print_operand(
    const char *seg,
    const char *symadd,
    const char *symsub,
    uint64_t value,
    unsigned int value_size,
    const char *result,
    const char *tail);

static uint64_t get_value(
    const uint32_t size,
    const char *sect,
    uint32_t *length,
    uint32_t *left);

static void modrm_byte(
    uint32_t *mode,
    uint32_t *reg,
    uint32_t *r_m,
    unsigned char byte);

#define GET_OPERAND(symadd, symsub, value, value_size, result) \
	get_operand((symadd), (symsub), (value), (value_size), (result), \
		    cputype, mode, r_m, wbit, data16, addr16, sse2, mmx, rex, \
		    sect, sect_addr, &length, &left, addr, sorted_relocs, \
		    nsorted_relocs, ext_relocs, next_relocs, symbols, \
		    symbols64, nsymbols, strings, strings_size, \
		    sorted_symbols, nsorted_symbols, verbose)

#define DISPLACEMENT(symadd, symsub, value, value_size) \
	displacement((symadd), (symsub), (value), (value_size), sect, \
		     sect_addr, &length, &left, cputype, addr, sorted_relocs, \
		     nsorted_relocs, ext_relocs, next_relocs, symbols, \
		     symbols64, nsymbols, strings, strings_size, \
		     sorted_symbols, nsorted_symbols, verbose)

#define IMMEDIATE(symadd, symsub, value, value_size) \
	immediate((symadd), (symsub), (value), (value_size), sect, sect_addr, \
		  &length, &left, cputype, addr, sorted_relocs, \
		  nsorted_relocs, ext_relocs, next_relocs, symbols, symbols64, \
		  nsymbols, strings, strings_size, sorted_symbols, \
		  nsorted_symbols, verbose)

#define GET_SYMBOL(symadd, symsub, offset, sect_offset, seg_offset, value) \
	get_symbol((symadd), (symsub), (offset), cputype, (sect_offset), \
		   (seg_offset), (value), sorted_relocs, nsorted_relocs, \
		   ext_relocs, next_relocs, symbols, symbols64, nsymbols, \
		   strings, strings_size, sorted_symbols, nsorted_symbols, \
		   verbose)

#define GUESS_SYMBOL(value) \
	guess_symbol((value), sorted_symbols, nsorted_symbols, verbose)

/*
 * These are the instruction formats as they appear in the disassembly tables.
 * Here they are given numerical values for use in the actual disassembly of
 * an instruction.
 */
#define UNKNOWN	0
#define MRw	2
#define IMlw	3
#define IMw	4
#define IR	5
#define OA	6
#define AO	7
#define MS	8
#define SM	9
#define Mv	10
#define Mw	11
#define M	12
#define R	13
#define RA	14
#define SEG	15
#define MR	16
#define IA	17
#define MA	18
#define SD	19
#define AD	20
#define SA	21
#define D	22
#define INM	23
#define SO	24
#define BD	25
#define I	26
#define P	27
#define V	28
#define DSHIFT	29 /* for double shift that has an 8-bit immediate */
#define U	30
#define OVERRIDE 31
#define GO_ON	32
#define O	33	/* for call	*/
#define JTAB	34	/* jump table (not used at NeXT) */
#define IMUL	35	/* for 186 iimul instr  */
#define CBW 36 /* so that data16 can be evaluated for cbw and its variants */
#define MvI	37	/* for 186 logicals */
#define ENTER	38	/* for 186 enter instr  */
#define RMw	39	/* for 286 arpl instr */
#define Ib	40	/* for push immediate byte */
#define F	41	/* for 287 instructions */
#define FF	42	/* for 287 instructions */
#define DM	43	/* 16-bit data */
#define AM	44	/* 16-bit addr */
#define LSEG	45	/* for 3-bit seg reg encoding */
#define MIb	46	/* for 386 logicals */
#define SREG	47	/* for 386 special registers */
#define PREFIX	48	/* an instruction prefix like REP, LOCK */
#define INT3	49	/* The int 3 instruction, which has a fake operand */
#define DSHIFTcl 50	/* for double shift that implicitly uses %cl */
#define CWD	51	/* so that data16 can be evaluated for cwd and vars */
#define RET	52	/* single immediate 16-bit operand */
#define MOVZ	53	/* for movs and movz, with different size operands */
#define XINST	54	/* for cmpxchg and xadd */
#define BSWAP	55	/* for bswap */
#define Pi	56
#define Po	57
#define Vi	58
#define Vo	59
#define Mb	60
#define INMl	61
#define SSE2	62	/* SSE2 instruction with possible 3rd opcode byte */
#define SSE2i	63	/* SSE2 instruction with 8-bit immediate */
#define SSE2i1	64	/* SSE2 with one operand and 8-bit immediate */
#define SSE2tm	65	/* SSE2 with dest to memory */
#define SSE2tfm	66	/* SSE2 with dest to memory or memory to dest */
#define PFCH	67	/* prefetch instructions */
#define SFEN	68	/* sfence & clflush */
#define Mnol	69	/* no 'l' suffix, fildl, fistpl */
#define AMD3DNOW       70  /* 3DNow! instruction (SSE2 format with a suffix) */
#define PFCH3DNOW      71  /* 3DNow! prefetch instruction */
#define REX	72		/* 64-bit REX prefix */
#define IR64 73		/* IR with a 64-bit immediate if REX.W is set */
#define MNI 74		/* MNI instruction, differentiated by 2nd and 3rd opcode bytes */
#define MNIi 75		/* MNI instruction with 8-bit immediate, differentiated by 2nd and 3rd opcode bytes */
#define SSE4	76	/* SSE4 instruction with 3rd & 4th opcode bytes */
#define SSE4i	77	/* SSE4 instruction with 8-bit immediate */
#define SSE4itm	78	/* SSE4 with dest to memory and 8-bit immediate */
#define SSE4ifm	79	/* SSE4 with src from memory and 8-bit immediate */
#define SSE4MRw	80	/* SSE4.2 memory or register operand to register */
#define SSE4CRC	81	/* SSE4.2 crc memory or register operand to register */
#define SSE4CRCb	82	/* SSE4.2 crc byte memory or register operand to register */

/*
 * In 16-bit addressing mode:
 * Register operands may be indicated by a distinguished field.
 * An '8' bit register is selected if the 'w' bit is equal to 0,
 * and a '16' bit register is selected if the 'w' bit is equal to
 * 1 and also if there is no 'w' bit.
 */
static const char * const REG16[8][2] = {
/* w bit		0		1		*/
/* reg bits */
/* 000	*/		{"%al",		"%ax"},
/* 001  */		{"%cl",		"%cx"},
/* 010  */		{"%dl",		"%dx"},
/* 011	*/		{"%bl",		"%bx"},
/* 100	*/		{"%ah",		"%sp"},
/* 101	*/		{"%ch",		"%bp"},
/* 110	*/		{"%dh",		"%si"},
/* 111	*/		{"%bh",		"%di"}
};

/*
 * In 32-bit or 64-bit addressing mode:
 * Register operands may be indicated by a distinguished field.
 * An '8' bit register is selected if the 'w' bit is equal to 0,
 * and a '32' bit register is selected if the 'w' bit is equal to
 * 1 and also if there is no 'w' bit.
 */
static const char * const REG32[16][3] = {
/* w bit		0				1			1 + REX.W	*/
/* reg bits */
/* 0000	*/		{"%al",			"%eax",			"%rax"},
/* 0001  */		{"%cl",			"%ecx",			"%rcx"},
/* 0010  */		{"%dl",			"%edx",			"%rdx"},
/* 0011	*/		{"%bl",			"%ebx",			"%rbx"},
/* 0100	*/		{"%ah",			"%esp",			"%rsp"},
/* 0101	*/		{"%ch",			"%ebp",			"%rbp"},
/* 0110	*/		{"%dh",			"%esi",			"%rsi"},
/* 0111	*/		{"%bh",			"%edi",			"%rdi"},
/* 1000	*/		{"%r8b",		"%r8d",			"%r8"},
/* 1001 */		{"%r9b",		"%r9d",			"%r9"},
/* 1010 */		{"%r10b",		"%r10d",		"%r10"},
/* 1011	*/		{"%r11b",		"%r11d",		"%r11"},
/* 1100	*/		{"%r12b",		"%r12d",		"%r12"},
/* 1101	*/		{"%r13b",		"%r13d",		"%r13"},
/* 1110	*/		{"%r14b",		"%r14d",		"%r14"},
/* 1111	*/		{"%r15b",		"%r15d",		"%r15"}
};

/* For SSE4CRCb (i.e. crc32) instruction the byte regs when there is a REX */
static const char * const REG64_BYTE[16] = {
/* 0 */ "%al",
/* 1 */ "%cl",
/* 2 */ "%dl",
/* 3 */ "%bl",
/* 4 */ "%spl",
/* 5 */ "%bpl",
/* 6 */ "%sil",
/* 7 */ "%dil",
/* 8 */ "%r8b",
/* 9 */ "%r9b",
/* 10 */"%r10b",
/* 11 */"%r11b",
/* 12 */"%r12b",
/* 13 */"%r13b",
/* 14 */"%r14b",
/* 15 */"%r15b"
};

/*
 * In 16-bit mode:
 * This initialized array will be indexed by the 'r/m' and 'mod'
 * fields, to determine the size of the displacement in each mode.
 */
static const char dispsize16 [8][4] = {
/* mod		00	01	10	11 */
/* r/m */
/* 000 */	{0,	1,	2,	0},
/* 001 */	{0,	1,	2,	0},
/* 010 */	{0,	1,	2,	0},
/* 011 */	{0,	1,	2,	0},
/* 100 */	{0,	1,	2,	0},
/* 101 */	{0,	1,	2,	0},
/* 110 */	{2,	1,	2,	0},
/* 111 */	{0,	1,	2,	0}
};

/*
 * In 32-bit mode:
 * This initialized array will be indexed by the 'r/m' and 'mod'
 * fields, to determine the size of the displacement in this mode.
 */
static const char dispsize32 [8][4] = {
/* mod		00	01	10	11 */
/* r/m */
/* 000 */	{0,	1,	4,	0},
/* 001 */	{0,	1,	4,	0},
/* 010 */	{0,	1,	4,	0},
/* 011 */	{0,	1,	4,	0},
/* 100 */	{0,	1,	4,	0},
/* 101 */	{4,	1,	4,	0},
/* 110 */	{0,	1,	4,	0},
/* 111 */	{0,	1,	4,	0}
};

/*
 * When data16 has been specified, the following array specifies the registers
 * for the different addressing modes.  Indexed first by mode, then by register
 * number.
 */
static const char * const regname16[4][8] = {
/*reg  000        001        010        011        100    101   110     111 */
/*mod*/
/*00*/{"%bx,%si", "%bx,%di", "%bp,%si", "%bp,%di", "%si", "%di", "",    "%bx"},
/*01*/{"%bx,%si", "%bx,%di", "%bp,%si", "%bp,%di", "%si", "%di", "%bp", "%bx"},
/*10*/{"%bx,%si", "%bx,%di", "%bp,%si", "%bp,%di", "%si", "%di", "%bp", "%bx"},
/*11*/{"%ax",     "%cx",     "%dx",     "%bx",     "%sp", "%bp", "%si", "%di"}
};

/*
 * When data16 has not been specified, fields, to determine the addressing mode,
 * and will also provide strings for printing.
 */
static const char * const regname32[4][8] = {
/*reg   000     001     010     011     100     101    110     111 */
/*mod*/
/*00 */{"%eax", "%ecx", "%edx", "%ebx", "%esp", "",     "%esi", "%edi"},
/*01 */{"%eax", "%ecx", "%edx", "%ebx", "%esp", "%ebp", "%esi", "%edi"},
/*10 */{"%eax", "%ecx", "%edx", "%ebx", "%esp", "%ebp", "%esi", "%edi"},
/*11 */{"%eax", "%ecx", "%edx", "%ebx", "%esp", "%ebp", "%esi", "%edi"}
};

/*
 * When data16 has not been specified, fields, to determine the addressing mode,
 * and will also provide strings for printing.
 */
static const char * const regname64[4][16] = {
/*reg   0000    0001    0010    0011    0100    0101    0110    0111    1000    1001    1010    1011    1100    1101    1110    1111 */
/*mod*/
/*00 */{"%rax", "%rcx", "%rdx", "%rbx", "%rsp", "%rbp", "%rsi", "%rdi", "%r8",  "%r9",  "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"},
/*01 */{"%rax", "%rcx", "%rdx", "%rbx", "%rsp", "%rbp", "%rsi", "%rdi", "%r8",  "%r9",  "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"},
/*10 */{"%rax", "%rcx", "%rdx", "%rbx", "%rsp", "%rbp", "%rsi", "%rdi", "%r8",  "%r9",  "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"},
/*11 */{"%rax", "%rcx", "%rdx", "%rbx", "%rsp", "%rbp", "%rsi", "%rdi", "%r8",  "%r9",  "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"}
};

/*
 * If r/m==100 then the following byte (the s-i-b byte) must be decoded
 */
static const char * const scale_factor[4] = {
    "1",
    "2",
    "4",
    "8"
};

static const char * const indexname[8] = {
    ",%eax",
    ",%ecx",
    ",%edx",
    ",%ebx",
    "",
    ",%ebp",
    ",%esi",
    ",%edi"
};

static const char * const indexname64[16] = {
    ",%rax",
    ",%rcx",
    ",%rdx",
    ",%rbx",
    "",
    ",%rbp",
    ",%rsi",
    ",%rdi",
	",%r8",
	",%r9",
	",%r10",
	",%r11",
	",%r12",
	",%r13",
	",%r14",
	",%r15"
};

/*
 * Segment registers are selected by a two or three bit field.
 */
static const char * const SEGREG[8] = {
/* 000 */	"%es",
/* 001 */	"%cs",
/* 010 */	"%ss",
/* 011 */	"%ds",
/* 100 */	"%fs",
/* 101 */	"%gs",
/* 110 */	"%?6",
/* 111 */	"%?7",
};

/*
 * Special Registers
 */
static const char * const DEBUGREG[] = {
	"%db0", "%db1", "%db2", "%db3", "%db4", "%db5", "%db6", "%db7",
	"%db8", "%db9", "%db10", "%db11", "%db12", "%db13", "%db14", "%db15"
};

static const char * const LLVM_MC_DEBUGREG[] = {
	"%dr0", "%dr1", "%dr2", "%dr3", "%dr4", "%dr5", "%dr6", "%dr7",
	"%db8", "%db9", "%db10", "%db11", "%db12", "%db13", "%db14", "%db15"
};

static const char * const CONTROLREG[] = {
	"%cr0", "%cr1", "%cr2", "%cr3", "%cr4", "%cr5", "%cr6", "%cr7",
	"%cr8", "%cr9", "%cr10", "%cr11", "%cr12", "%cr13", "%cr14", "%cr15"
};

static const char * const LLVM_MC_32_CONTROLREG[] = {
	"%ecr0", "%ecr1", "%ecr2", "%ecr3", "%ecr4", "%ecr5", "%ecr6", "%ecr7",
	"%ecr8", "%ecr9", "%ecr10", "%ecr11", "%ecr12", "%ecr13", "%ecr14",
	"%ecr15"
};

static const char * const LLVM_MC_64_CONTROLREG[] = {
	"%rcr0", "%rcr1", "%rcr2", "%rcr3", "%rcr4", "%rcr5", "%rcr6", "%rcr7",
	"%rcr8", "%rcr9", "%rcr10", "%rcr11", "%rcr12", "%rcr13", "%rcr14",
	"%rcr15"
};

static const char * const TESTREG[8] = {
    "%tr0", "%tr1", "%tr2", "%tr3", "%tr4", "%tr5", "%tr6", "%tr7"
};

/*
 * Decode table for 0x0F00 opcodes
 */
static const struct instable op0F00[8] = {
/*  [0]  */	{"sldt",TERM,M,0},	{"str",TERM,M,0},
		{"lldt",TERM,M,0},	{"ltr",TERM,M,0},
/*  [4]  */	{"verr",TERM,M,0},	{"verw",TERM,M,0},
		INVALID,		INVALID,
};


/*
 * Decode table for 0x0F01 opcodes
 */
static const struct instable op0F01[8] = {
/*  [0]  */	{"sgdt",TERM,M,1},	{"sidt",TERM,M,1},
		{"lgdt",TERM,M,1},	{"lidt",TERM,M,1},
/*  [4]  */	{"smsw",TERM,M,0},	INVALID,
		{"lmsw",TERM,M,0},	{"invlpg",TERM,M,0},
};

/*
 * Decode table for 0x0F38 opcodes
 */
static const struct instable op0F38[256] = {
/*  [00]  */	{"pshufb",TERM,MNI,0},	{"phaddw",TERM,MNI,0},
		{"phaddd",TERM,MNI,0},	{"phaddsw",TERM,MNI,0},
/*  [04]  */	{"pmaddubsw",TERM,MNI,0},	{"phsubw",TERM,MNI,0},
		{"phsubd",TERM,MNI,0},	{"phsubsw",TERM,MNI,0},
/*  [08]  */	{"psignb",TERM,MNI,0},	{"psignw",TERM,MNI,0},
		{"psignd",TERM,MNI,0},	{"pmulhrsw",TERM,MNI,0},
/*  [0C]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [10]  */	{"pblendvb",TERM,SSE4,0},	INVALID,
		INVALID,	INVALID,
/*  [14]  */	{"blendvps",TERM,SSE4,0},{"blendvpd",TERM,SSE4,0},
		INVALID,	{"ptest",TERM,SSE4,0},
/*  [18]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [1C]  */	{"pabsb",TERM,MNI,0},	{"pabsw",TERM,MNI,0},
		{"pabsd",TERM,MNI,0},	INVALID,
/*  [20]  */	{"pmovsxbw",TERM,SSE4,0},	{"pmovsxbd",TERM,SSE4,0},
		{"pmovsxbq",TERM,SSE4,0},	{"pmovsxwd",TERM,SSE4,0},
/*  [24]  */	{"pmovsxwq",TERM,SSE4,0},	{"pmovsxdq",TERM,SSE4,0},
		INVALID,	INVALID,
/*  [28]  */	{"pmuldq",TERM,SSE4,0},		{"pcmpeqq",TERM,SSE4,0},
		{"movntdqa",TERM,SSE4,0},	{"packusdw",TERM,SSE4,0},
/*  [2C]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [30]  */	{"pmovzxbw",TERM,SSE4,0},	{"pmovzxbd",TERM,SSE4,0},
		{"pmovzxbq",TERM,SSE4,0},	{"pmovzxwd",TERM,SSE4,0},
/*  [34]  */	{"pmovzxwq",TERM,SSE4,0},	{"pmovzxdq",TERM,SSE4,0},
		INVALID,	{"pcmpgtq",TERM,SSE4,0},
/*  [38]  */	{"pminsb",TERM,SSE4,0},	{"pminsd",TERM,SSE4,0},
		{"pminuw",TERM,SSE4,0},	{"pminud",TERM,SSE4,0},
/*  [3C]  */	{"pmaxsb",TERM,SSE4,0},	{"pmaxsd",TERM,SSE4,0},
		{"pmaxuw",TERM,SSE4,0},	{"pmaxud",TERM,SSE4,0},
/*  [40]  */	{"pmulld",TERM,SSE4,0},	{"phminposuw",TERM,SSE4,0},
		INVALID,	INVALID,
/*  [44]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [48]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [4C]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [50]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [54]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [58]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [5C]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [60]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [64]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [68]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [6C]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [70]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [74]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [78]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [7C]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [80]  */	{"invept",TERM,MR,0}, {"invvpid",TERM,MR,0},
		INVALID,	INVALID,
/*  [84]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [88]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [8C]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [90]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [94]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [98]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [9C]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [A0]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [A4]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [A8]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [AC]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [B0]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [B4]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [B8]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [BC]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [C0]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [C4]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [C8]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [CC]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [D0]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [D4]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [D8]  */	INVALID,	INVALID,
		INVALID,	{"aesimc",TERM,SSE4,0},
/*  [DC]  */	{"aesenc",TERM,SSE4,0}, {"aesenclast",TERM,SSE4,0},
		{"aesdec",TERM,SSE4,0},	{"aesdeclast",TERM,SSE4,0},
/*  [E0]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [E4]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [E8]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [EC]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [F0]  */	{"crc32b",TERM,SSE4CRCb,0},	{"crc32",TERM,SSE4CRC,1},
		INVALID,	INVALID,
/*  [F4]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [F8]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [FC]  */	INVALID,	INVALID,
		INVALID,	INVALID,
};

/*
 * Decode table for 0x0F3A opcodes
 */
static const struct instable op0F3A[224] = {
/*  [00]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [04]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [08]  */	{"roundps",TERM,SSE4i,0},	{"roundpd",TERM,SSE4i,0},
		{"roundss",TERM,SSE4i,0},	{"roundsd",TERM,SSE4i,0},
/*  [0C]  */	{"blendps",TERM,SSE4i,0},	{"blendpd",TERM,SSE4i,0},
		{"pblendw",TERM,SSE4i,0},	{"palignr",TERM,MNIi,0},
/*  [10]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [14]  */	{"pextrb",TERM,SSE4itm,0},	{"pextrw",TERM,SSE4itm,0},
		{"pextr",TERM,SSE4itm,0},	{"extractps",TERM,SSE4itm,0},
/*  [18]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [1C]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [20]  */	{"pinsrb",TERM,SSE4ifm,0},	{"insertps",TERM,SSE4i,0},
		{"pinsr",TERM,SSE4ifm,0},	INVALID,
/*  [24]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [28]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [2C]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [30]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [34]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [38]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [3C]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [40]  */	{"dpps",TERM,SSE4i,0},	{"dppd",TERM,SSE4i,0},
		{"mpsadbw",TERM,SSE4i,0},	INVALID,
/*  [44]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [48]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [4C]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [50]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [54]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [58]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [5C]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [60]  */	{"pcmpestrm",TERM,SSE4i,0},	{"pcmpestri",TERM,SSE4i,0},
		{"pcmpistrm",TERM,SSE4i,0},	{"pcmpistri",TERM,SSE4i,0},
/*  [64]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [68]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [6C]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [70]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [74]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [78]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [7C]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [80]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [84]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [88]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [8C]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [90]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [94]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [98]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [9C]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [A0]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [A4]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [A8]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [AC]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [B0]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [B4]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [B8]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [BC]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [C0]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [C4]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [C8]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [CC]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [D0]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [D4]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [D8]  */	INVALID,	INVALID,
		INVALID,	INVALID,
/*  [DC]  */	INVALID,	INVALID,
		INVALID,	{"aeskeygenassist",TERM,SSE4i,0},
};

static const struct instable op_monitor = {"monitor",TERM,GO_ON,0};
static const struct instable op_mwait   = {"mwait",TERM,GO_ON,0};
static const struct instable op_rdtscp   = {"rdtscp",TERM,GO_ON,0};

/* These opcode tables entries are only used for the 64-bit architecture */
static const struct instable op_swapgs = {"swapgs",TERM,GO_ON,0};
static const struct instable op_syscall = {"syscall",TERM,GO_ON,0};
static const struct instable op_sysret = {"sysret",TERM,GO_ON,0};
static const struct instable opREX = {"",TERM,REX,0};
static const struct instable op_movsl = {"movsl",TERM,MOVZ,1};

/*
 * Decode table for 0x0F0F opcodes
 * Unlike the other decode tables, this one maps suffixes.
 */
static const struct instable op0F0F[16][16] = {
/*  [00]  */ {  INVALID,       INVALID,
               INVALID,        INVALID,
/*  [04]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [08]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [0C]  */   {"pi2fw",TERM,AMD3DNOW,0},      {"pi2fd",TERM,AMD3DNOW,0},
               INVALID,        INVALID },
/*  [10]  */ {  INVALID,       INVALID,
               INVALID,        INVALID,
/*  [14]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [18]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [1C]  */   {"pf2iw",TERM,AMD3DNOW,0},      {"pf2id",TERM,AMD3DNOW,0},
               INVALID,        INVALID },
/*  [20]  */ {  INVALID,       INVALID,
               INVALID,        INVALID,
/*  [24]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [28]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [2C]  */   INVALID,        INVALID,
               INVALID,        INVALID, },
/*  [30]  */ {  INVALID,       INVALID,
               INVALID,        INVALID,
/*  [34]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [38]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [3C]  */   INVALID,        INVALID,
               INVALID,        INVALID },
/*  [40]  */ {  INVALID,       INVALID,
               INVALID,        INVALID,
/*  [44]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [48]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [4C]  */   INVALID,        INVALID,
               INVALID,        INVALID },
/*  [50]  */ {  INVALID,       INVALID,
               INVALID,        INVALID,
/*  [54]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [58]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [5C]  */   INVALID,        INVALID,
               INVALID,        INVALID, },
/*  [60]  */ {  INVALID,       INVALID,
               INVALID,        INVALID,
/*  [64]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [68]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [6C]  */   INVALID,        INVALID,
               INVALID,        INVALID },
/*  [70]  */ {  INVALID,       INVALID,
               INVALID,        INVALID,
/*  [74]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [78]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [7C]  */   INVALID,        INVALID,
               INVALID,        INVALID },
/*  [80]  */ {  INVALID,       INVALID,
               INVALID,        INVALID,
/*  [84]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [88]  */   INVALID,        INVALID,
               {"pfnacc",TERM,AMD3DNOW,0},     INVALID,
/*  [8C]  */   INVALID,        INVALID,
               {"pfpnacc",TERM,AMD3DNOW,0},    INVALID },
/*  [90]  */ {  {"pfcmpge",TERM,AMD3DNOW,0},   INVALID,
               INVALID,        INVALID,
/*  [94]  */   {"pfmin",TERM,AMD3DNOW,0},      INVALID,
               {"pfrcp",TERM,AMD3DNOW,0},      {"pfrsqrt",TERM,AMD3DNOW,0},
/*  [98]  */   INVALID,        INVALID,
               {"pfsub",TERM,AMD3DNOW,0},      INVALID,
/*  [9C]  */   INVALID,        INVALID,
               {"pfadd",TERM,AMD3DNOW,0},      INVALID },
/*  [A0]  */ {  {"pfcmpgt",TERM,AMD3DNOW,0},   INVALID,
               INVALID,        INVALID,
/*  [A4]  */   {"pfmax",TERM,AMD3DNOW,0},      INVALID,
               {"pfrcpit1",TERM,AMD3DNOW,0},   {"pfrsqit1",TERM,AMD3DNOW,0},
/*  [A8]  */   INVALID,        INVALID,
               {"pfsubr",TERM,AMD3DNOW,0},     INVALID,
/*  [AC]  */   INVALID,        INVALID,
               {"pfacc",TERM,AMD3DNOW,0},      INVALID },
/*  [B0]  */ {  {"pfcmpeq",TERM,AMD3DNOW,0},   INVALID,
               INVALID,        INVALID,
/*  [B4]  */   {"pfmul",TERM,AMD3DNOW,0},      INVALID,
               {"pfrcpit2",TERM,AMD3DNOW,0},   {"pmulhrw",TERM,AMD3DNOW,0},
/*  [B8]  */   INVALID,        INVALID,
               INVALID,        {"pswapd",TERM,AMD3DNOW,0},
/*  [BC]  */   INVALID,        INVALID,
               INVALID,        {"pavgusb",TERM,AMD3DNOW,0} },
/*  [C0]  */ {  INVALID,       INVALID,
               INVALID,        INVALID,
/*  [C4]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [C8]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [CC]  */   INVALID,        INVALID,
               INVALID,        INVALID },
/*  [D0]  */ {  INVALID,       INVALID,
               INVALID,        INVALID,
/*  [D4]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [D8]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [DC]  */   INVALID,        INVALID,
               INVALID,        INVALID },
/*  [E0]  */ {  INVALID,       INVALID,
               INVALID,        INVALID,
/*  [E4]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [E8]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [EC]  */   INVALID,        INVALID,
               INVALID,        INVALID },
/*  [F0]  */ {  INVALID,       INVALID,
               INVALID,        INVALID,
/*  [F4]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [F8]  */   INVALID,        INVALID,
               INVALID,        INVALID,
/*  [FC]  */   INVALID,        INVALID,
               INVALID,        INVALID },
};

/*
 * Decode table for 0x0FBA opcodes
 */
static const struct instable op0FBA[8] = {
/*  [0]  */	INVALID,		INVALID,
		INVALID,		INVALID,
/*  [4]  */	{"bt",TERM,MIb,1},	{"bts",TERM,MIb,1},
		{"btr",TERM,MIb,1},	{"btc",TERM,MIb,1},
};

/*
 * Decode table for 0x0FAE opcodes
 */
static const struct instable op0FAE[8] = {
/*  [0]  */	{"fxsave",TERM,M,0},	{"fxrstor",TERM,M,0},
		{"ldmxcsr",TERM,M,0},	{"stmxcsr",TERM,M,0},
/*  [4]  */	INVALID,		{"lfence",TERM,GO_ON,0},
		{"mfence",TERM,GO_ON,0},{"clflush",TERM,SFEN,0},
};

/*
 * Decode table for 0x0F opcodes
 */
static const struct instable op0F[16][16] = {
/*  [00]  */ {  {"",op0F00,TERM,0},	{"",op0F01,TERM,0},
		{"lar",TERM,MR,0},	{"lsl",TERM,MR,0},
/*  [04]  */	INVALID,		{INVALID_32,&op_syscall},
		{"clts",TERM,GO_ON,0},  {INVALID_32,&op_sysret},
/*  [08]  */	{"invd",TERM,GO_ON,0},	{"wbinvd",TERM,GO_ON,0},
		INVALID,		{"ud2",TERM,GO_ON,0},
/*  [0C]  */	INVALID,                {"prefetch",TERM,PFCH3DNOW,1},
		{"femms",TERM,GO_ON,0},
				{"",(const struct instable *)op0F0F,TERM,0} },
/*  [10]  */ {  {"mov",TERM,SSE2,0},	{"mov",TERM,SSE2tm,0},
		{"mov",TERM,SSE2,0},	{"movl",TERM,SSE2tm,0},
/*  [14]  */	{"unpckl",TERM,SSE2,0},	{"unpckh",TERM,SSE2,0},
		{"mov",TERM,SSE2,0},	{"movh",TERM,SSE2tm,0},
/*  [18]  */	{"prefetch",TERM,PFCH,1},INVALID,
		INVALID,		INVALID,
/*  [1C]  */	INVALID,		INVALID,
		INVALID,		{"nop",TERM,M,1} },
/*  [20]  */ {  {"mov",TERM,SREG,0x03},	{"mov",TERM,SREG,0x03},
		{"mov",TERM,SREG,0x03},	{"mov",TERM,SREG,0x03},
/*  [24]  */	{"mov",TERM,SREG,0x03},	INVALID,
		{"mov",TERM,SREG,0x03},	INVALID,
/*  [28]  */	{"mova",TERM,SSE2,0},	{"mova",TERM,SSE2tm,0},
		{"cvt",TERM,SSE2,0},	{"movnt",TERM,SSE2tm,0},
/*  [2C]  */	{"cvt",TERM,SSE2,0},	{"cvt",TERM,SSE2,0} ,
		{"ucomi",TERM,SSE2,0},	{"comi",TERM,SSE2,0} },
/*  [30]  */ {  {"wrmsr",TERM,GO_ON,0},	{"rdtsc",TERM,GO_ON,0},
		{"rdmsr",TERM,GO_ON,0},	{"rdpmc",TERM,GO_ON,0},
/*  [34]  */	{"sysenter",TERM,GO_ON,0},{"sysexit",TERM,GO_ON,0},
		INVALID,		INVALID,
/*  [38]  */	{"",op0F38,TERM,0},		INVALID,
		{"",op0F3A,TERM,0},		INVALID,
/*  [3C]  */	INVALID,		INVALID,
		INVALID,		INVALID },
/*  [40]  */ {  {"cmovo",TERM,MRw,1},	{"cmovno",TERM,MRw,1},
		{"cmovb",TERM,MRw,1},	{"cmovae",TERM,MRw,1},
/*  [44]  */	{"cmove",TERM,MRw,1},	{"cmovne",TERM,MRw,1},
		{"cmovbe",TERM,MRw,1},	{"cmova",TERM,MRw,1},
/*  [48]  */	{"cmovs",TERM,MRw,1},	{"cmovns",TERM,MRw,1},
		{"cmovp",TERM,MRw,1},	{"cmovnp",TERM,MRw,1},
/*  [4C]  */	{"cmovl",TERM,MRw,1},	{"cmovge",TERM,MRw,1},
		{"cmovle",TERM,MRw,1},	{"cmovg",TERM,MRw,1} },
/*  [50]  */ {  {"movmsk",TERM,SSE2,0},	{"sqrt",TERM,SSE2,0},
		{"rsqrt",TERM,SSE2,0},	{"rcp",TERM,SSE2,0},
/*  [54]  */	{"and",TERM,SSE2,0},	{"andn",TERM,SSE2,0},
		{"or",TERM,SSE2,0},	{"xor",TERM,SSE2,0},
/*  [58]  */	{"add",TERM,SSE2,0},	{"mul",TERM,SSE2,0},
		{"cvt",TERM,SSE2,0},	{"cvt",TERM,SSE2,0},
/*  [5C]  */	{"sub",TERM,SSE2,0},	{"min",TERM,SSE2,0},
		{"div",TERM,SSE2,0},	{"max",TERM,SSE2,0} },
/*  [60]  */ {  {"punpcklbw",TERM,SSE2,0},{"punpcklwd",TERM,SSE2,0},
		{"punpckldq",TERM,SSE2,0},{"packsswb",TERM,SSE2,0},
/*  [64]  */	{"pcmpgtb",TERM,SSE2,0},{"pcmpgtw",TERM,SSE2,0},
		{"pcmpgtd",TERM,SSE2,0},{"packuswb",TERM,SSE2,0},
/*  [68]  */	{"punpckhbw",TERM,SSE2,0},{"punpckhwd",TERM,SSE2,0},
		{"punpckhdq",TERM,SSE2,0},{"packssdw",TERM,SSE2,0},
/*  [6C]  */	{"punpckl",TERM,SSE2,0},{"punpckh",TERM,SSE2,0},
		{"movd",TERM,SSE2,0},	{"mov",TERM,SSE2,0} },
/*  [70]  */ {  {"pshu",TERM,SSE2i,0},	{"ps",TERM,SSE2i1,0},
		{"ps",TERM,SSE2i1,0},	{"ps",TERM,SSE2i1,0},
/*  [74]  */	{"pcmpeqb",TERM,SSE2,0},{"pcmpeqw",TERM,SSE2,0},
		{"pcmpeqd",TERM,SSE2,0},{"emms",TERM,GO_ON,0},
/*  [78]  */	{"vmread",TERM,RMw,0},  {"vmwrite",TERM,MRw,0},
		INVALID,		INVALID,
/*  [7C]  */	{"haddp",TERM,SSE2,0},  {"hsubp",TERM,SSE2,0},
		{"mov",TERM,SSE2tfm,0},	{"mov",TERM,SSE2tm,0} },
/*  [80]  */ {  {"jo",TERM,D,0x02},	{"jno",TERM,D,0x02},
		{"jb",TERM,D,0x02},	{"jae",TERM,D,0x02},
/*  [84]  */	{"je",TERM,D,0x02},	{"jne",TERM,D,0x02},
		{"jbe",TERM,D,0x02},	{"ja",TERM,D,0x02},
/*  [88]  */	{"js",TERM,D,0x02},	{"jns",TERM,D,0x02},
		{"jp",TERM,D,0x02},	{"jnp",TERM,D,0x02},
/*  [8C]  */	{"jl",TERM,D,0x02},	{"jge",TERM,D,0x02},
		{"jle",TERM,D,0x02},	{"jg",TERM,D,0x02} },
/*  [90]  */ {  {"seto",TERM,Mb,0},	{"setno",TERM,Mb,0},
		{"setb",TERM,Mb,0},	{"setae",TERM,Mb,0},
/*  [94]  */	{"sete",TERM,Mb,0},	{"setne",TERM,Mb,0},
		{"setbe",TERM,Mb,0},	{"seta",TERM,Mb,0},
/*  [98]  */	{"sets",TERM,Mb,0},	{"setns",TERM,Mb,0},
		{"setp",TERM,Mb,0},	{"setnp",TERM,Mb,0},
/*  [9C]  */	{"setl",TERM,Mb,0},	{"setge",TERM,Mb,0},
		{"setle",TERM,Mb,0},	{"setg",TERM,Mb,0} },
/*  [A0]  */ {  {"push",TERM,LSEG,0x03},{"pop",TERM,LSEG,0x03},
		{"cpuid",TERM,GO_ON,0},	{"bt",TERM,RMw,1},
/*  [A4]  */	{"shld",TERM,DSHIFT,1},	{"shld",TERM,DSHIFTcl,1},
		INVALID,		INVALID,
/*  [A8]  */	{"push",TERM,LSEG,0x03},{"pop",TERM,LSEG,0x03},
		{"rsm",TERM,GO_ON,0, INVALID_64}, {"bts",TERM,RMw,1},
/*  [AC]  */	{"shrd",TERM,DSHIFT,1},	{"shrd",TERM,DSHIFTcl,1},
		{"",op0FAE,TERM,0},	{"imul",TERM,MRw,1} },
/*  [B0]  */ {  {"cmpxchgb",TERM,XINST,0},{"cmpxchg",TERM,XINST,1},
		{"lss",TERM,MR,0},	{"btr",TERM,RMw,1},
/*  [B4]  */	{"lfs",TERM,MR,0},	{"lgs",TERM,MR,0},
		{"movzb",TERM,MOVZ,1},	{"movzw",TERM,MOVZ,1},
/*  [B8]  */	{"popcnt",TERM,SSE4MRw,0},		INVALID,
		{"",op0FBA,TERM,0},	{"btc",TERM,RMw,1},
/*  [BC]  */	{"bsf",TERM,MRw,1},	{"bsr",TERM,MRw,1},
		{"movsb",TERM,MOVZ,1},	{"movsw",TERM,MOVZ,1} },
/*  [C0]  */ {  {"xaddb",TERM,XINST,0},	{"xadd",TERM,XINST,1},
		{"cmp",TERM,SSE2i,0},	{"movnti",TERM,RMw,0},
/*  [C4]  */	{"pinsrw",TERM,SSE2i,0},{"pextrw",TERM,SSE2i,0},
		{"shuf",TERM,SSE2i,0},	{"cmpxchg8b",TERM,M,0},
/*  [C8]  */	{"bswap",TERM,BSWAP,0},	{"bswap",TERM,BSWAP,0},
		{"bswap",TERM,BSWAP,0},	{"bswap",TERM,BSWAP,0},
/*  [CC]  */	{"bswap",TERM,BSWAP,0},	{"bswap",TERM,BSWAP,0},
		{"bswap",TERM,BSWAP,0},	{"bswap",TERM,BSWAP,0} },
/*  [D0]  */ {  {"addsubp",TERM,SSE2,0},{"psrlw",TERM,SSE2,0},
		{"psrld",TERM,SSE2,0},	{"psrlq",TERM,SSE2,0},
/*  [D4]  */	{"paddq",TERM,SSE2,0},	{"pmullw",TERM,SSE2,0},
		{"mov",TERM,SSE2tm,0},	{"pmovmskb",TERM,SSE2,0},
/*  [D8]  */	{"psubusb",TERM,SSE2,0},{"psubusw",TERM,SSE2,0},
		{"pminub",TERM,SSE2,0},	{"pand",TERM,SSE2,0},
/*  [DC]  */	{"paddusb",TERM,SSE2,0},{"paddusw",TERM,SSE2,0},
		{"pmaxub",TERM,SSE2,0},	{"pandn",TERM,SSE2,0} },
/*  [E0]  */ {  {"pavgb",TERM,SSE2,0},	{"psraw",TERM,SSE2,0},
		{"psrad",TERM,SSE2,0},	{"pavgw",TERM,SSE2,0},
/*  [E4]  */	{"pmulhuw",TERM,SSE2,0},{"pmulhw",TERM,SSE2,0},
		{"cvt",TERM,SSE2,0},	{"movn",TERM,SSE2tm,0},
/*  [E8]  */	{"psubsb",TERM,SSE2,0},	{"psubsw",TERM,SSE2,0},
		{"pminsw",TERM,SSE2,0},	{"por",TERM,SSE2,0},
/*  [EC]  */	{"paddsb",TERM,SSE2,0},	{"paddsw",TERM,SSE2,0},
		{"pmaxsw",TERM,SSE2,0},	{"pxor",TERM,SSE2,0} },
/*  [F0]  */ {  {"lddqu",TERM,SSE2,0},	{"psllw",TERM,SSE2,0},
		{"pslld",TERM,SSE2,0},	{"psllq",TERM,SSE2,0},
/*  [F4]  */	{"pmuludq",TERM,SSE2,0},{"pmaddwd",TERM,SSE2,0},
		{"psadbw",TERM,SSE2,0},	{"maskmov",TERM,SSE2,0},
/*  [F8]  */	{"psubb",TERM,SSE2,0},	{"psubw",TERM,SSE2,0},
		{"psubd",TERM,SSE2,0},	{"psubq",TERM,SSE2,0},
/*  [FC]  */	{"paddb",TERM,SSE2,0},	{"paddw",TERM,SSE2,0},
		{"paddd",TERM,SSE2,0},	INVALID },
};

/*
 * Decode table for 0x80 opcodes
 */
static const struct instable op80[8] = {
/*  [0]  */	{"addb",TERM,IMlw,0},	{"orb",TERM,IMw,0},
		{"adcb",TERM,IMlw,0},	{"sbbb",TERM,IMlw,0},
/*  [4]  */	{"andb",TERM,IMw,0},	{"subb",TERM,IMlw,0},
		{"xorb",TERM,IMw,0},	{"cmpb",TERM,IMlw,0},
};

/*
 * Decode table for 0x81 opcodes.
 */
static const struct instable op81[8] = {
/*  [0]  */	{"add",TERM,IMlw,1},	{"or",TERM,IMw,1},
		{"adc",TERM,IMlw,1},	{"sbb",TERM,IMlw,1},
/*  [4]  */	{"and",TERM,IMw,1},	{"sub",TERM,IMlw,1},
		{"xor",TERM,IMw,1},	{"cmp",TERM,IMlw,1},
};

/*
 * Decode table for 0x82 opcodes.
 */
static const struct instable op82[8] = {
/*  [0]  */	{"addb",TERM,IMlw,0},	INVALID,
		{"adcb",TERM,IMlw,0},	{"sbbb",TERM,IMlw,0},
/*  [4]  */	INVALID,		{"subb",TERM,IMlw,0},
		INVALID,		{"cmpb",TERM,IMlw,0},
};

/*
 * Decode table for 0x83 opcodes.
 */
static const struct instable op83[8] = {
/*  [0]  */	{"add",TERM,IMlw,1},	{"or",TERM,IMlw,1},
		{"adc",TERM,IMlw,1},	{"sbb",TERM,IMlw,1},
/*  [4]  */	{"and",TERM,IMlw,1},	{"sub",TERM,IMlw,1},
		{"xor",TERM,IMlw,1},	{"cmp",TERM,IMlw,1},
};

/*
 * Decode table for 0xC0 opcodes.
 */
static const struct instable opC0[8] = {
/*  [0]  */	{"rolb",TERM,MvI,0},	{"rorb",TERM,MvI,0},
		{"rclb",TERM,MvI,0},	{"rcrb",TERM,MvI,0},
/*  [4]  */	{"shlb",TERM,MvI,0},	{"shrb",TERM,MvI,0},
		INVALID,		{"sarb",TERM,MvI,0},
};

/*
 * Decode table for 0xD0 opcodes.
 */
static const struct instable opD0[8] = {
/*  [0]  */	{"rolb",TERM,Mv,0},	{"rorb",TERM,Mv,0},
		{"rclb",TERM,Mv,0},	{"rcrb",TERM,Mv,0},
/*  [4]  */	{"shlb",TERM,Mv,0},	{"shrb",TERM,Mv,0},
		INVALID,		{"sarb",TERM,Mv,0},
};

/*
 * Decode table for 0xC1 opcodes.
 * 186 instruction set
 */
static const struct instable opC1[8] = {
/*  [0]  */	{"rol",TERM,MvI,1},	{"ror",TERM,MvI,1},
		{"rcl",TERM,MvI,1},	{"rcr",TERM,MvI,1},
/*  [4]  */	{"shl",TERM,MvI,1},	{"shr",TERM,MvI,1},
		INVALID,		{"sar",TERM,MvI,1},
};

/*
 * Decode table for 0xD1 opcodes.
 */
static const struct instable opD1[8] = {
/*  [0]  */	{"rol",TERM,Mv,1},	{"ror",TERM,Mv,1},
		{"rcl",TERM,Mv,1},	{"rcr",TERM,Mv,1},
/*  [4]  */	{"shl",TERM,Mv,1},	{"shr",TERM,Mv,1},
		INVALID,		{"sar",TERM,Mv,1},
};

/*
 * Decode table for 0xD2 opcodes.
 */
static const struct instable opD2[8] = {
/*  [0]  */	{"rolb",TERM,Mv,0},	{"rorb",TERM,Mv,0},
		{"rclb",TERM,Mv,0},	{"rcrb",TERM,Mv,0},
/*  [4]  */	{"shlb",TERM,Mv,0},	{"shrb",TERM,Mv,0},
		INVALID,		{"sarb",TERM,Mv,0},
};

/*
 * Decode table for 0xD3 opcodes.
 */
static const struct instable opD3[8] = {
/*  [0]  */	{"rol",TERM,Mv,1},	{"ror",TERM,Mv,1},
		{"rcl",TERM,Mv,1},	{"rcr",TERM,Mv,1},
/*  [4]  */	{"shl",TERM,Mv,1},	{"shr",TERM,Mv,1},
		INVALID,		{"sar",TERM,Mv,1},
};

/*
 * Decode table for 0xF6 opcodes.
 */
static const struct instable opF6[8] = {
/*  [0]  */	{"testb",TERM,IMw,0},	INVALID,
		{"notb",TERM,Mw,0},	{"negb",TERM,Mw,0},
/*  [4]  */	{"mulb",TERM,MA,0},	{"imulb",TERM,MA,0},
		{"divb",TERM,MA,0},	{"idivb",TERM,MA,0},
};

/*
 * Decode table for 0xF7 opcodes.
 */
static const struct instable opF7[8] = {
/*  [0]  */	{"test",TERM,IMw,1},	INVALID,
		{"not",TERM,Mw,1},	{"neg",TERM,Mw,1},
/*  [4]  */	{"mul",TERM,MA,1},	{"imul",TERM,MA,1},
		{"div",TERM,MA,1},	{"idiv",TERM,MA,1},
};

/*
 * Decode table for 0xFE opcodes.
 */
static const struct instable opFE[8] = {
/*  [0]  */	{"incb",TERM,Mw,0},	{"decb",TERM,Mw,0},
		INVALID,		INVALID,
/*  [4]  */	INVALID,		INVALID,
		INVALID,		INVALID,
};

/*
 * Decode table for 0xFF opcodes.
 */
static const struct instable opFF[8] = {
/*  [0]  */	{"inc",TERM,Mw,1},	{"dec",TERM,Mw,1},
		{"call",TERM,INM,1},	{"lcall",TERM,INMl,1},
/*  [4]  */	{"jmp",TERM,INM,1},	{"ljmp",TERM,INMl,1},
		{"push",TERM,M,0x03},	INVALID,
};

/* for 287 instructions, which are a mess to decode */
static const struct instable opFP1n2[8][8] = {
/* bit pattern:	1101 1xxx MODxx xR/M */
/*  [0,0]  */ { {"fadds",TERM,M,0},	{"fmuls",TERM,M,0},
		{"fcoms",TERM,M,0},	{"fcomps",TERM,M,0},
/*  [0,4]  */	{"fsubs",TERM,M,0},	{"fsubrs",TERM,M,0},
		{"fdivs",TERM,M,0},	{"fdivrs",TERM,M,0} },
/*  [1,0]  */ { {"flds",TERM,M,0},	INVALID,
		{"fsts",TERM,M,0},	{"fstps",TERM,M,0},
/*  [1,4]  */	{"fldenv",TERM,M,1},	{"fldcw",TERM,M,0},
		{"fnstenv",TERM,M,1},	{"fnstcw",TERM,M,0} },
/*  [2,0]  */ { {"fiaddl",TERM,M,0},	{"fimull",TERM,M,0},
		{"ficoml",TERM,M,0},	{"ficompl",TERM,M,0},
/*  [2,4]  */	{"fisubl",TERM,M,0},	{"fisubrl",TERM,M,0},
		{"fidivl",TERM,M,0},	{"fidivrl",TERM,M,0} },
/*  [3,0]  */ { {"fildl",TERM,Mnol,0},	{"fisttpl",TERM,M,0},
		{"fistl",TERM,M,0},	{"fistpl",TERM,Mnol,0},
/*  [3,4]  */	INVALID,		{"fldt",TERM,M,0},
		INVALID,		{"fstpt",TERM,M,0} },
/*  [4,0]  */ { {"faddl",TERM,M,0},	{"fmull",TERM,M,0},
		{"fcoml",TERM,M,0},	{"fcompl",TERM,M,0},
/*  [4,1]  */	{"fsubl",TERM,M,0},	{"fsubrl",TERM,M,0},
		{"fdivl",TERM,M,0},	{"fdivrl",TERM,M,0} },
/*  [5,0]  */ { {"fldl",TERM,M,0},	{"fisttpll",TERM,M,0},
		{"fstl",TERM,M,0},	{"fstpl",TERM,M,0},
/*  [5,4]  */	{"frstor",TERM,M,1},	INVALID,
		{"fnsave",TERM,M,1},	{"fnstsw",TERM,M,0} },
/*  [6,0]  */ { {"fiadds",TERM,M,0},	{"fimuls",TERM,M,0},
		{"ficoms",TERM,M,0},	{"ficomps",TERM,M,0},
/*  [6,4]  */	{"fisubs",TERM,M,0},	{"fisubrs",TERM,M,0},
		{"fidivs",TERM,M,0},	{"fidivrs",TERM,M,0} },
/*  [7,0]  */ { {"filds",TERM,M,0},	{"fisttps",TERM,M,0},
		{"fists",TERM,M,0},	{"fistps",TERM,M,0},
/*  [7,4]  */	{"fbld",TERM,M,0},	{"fildq",TERM,M,0},
		{"fbstp",TERM,M,0},	{"fistpq",TERM,M,0} },
};

static const struct instable opFP3[8][8] = {
/* bit  pattern:	1101 1xxx 11xx xREG */
/*  [0,0]  */ { {"fadd",TERM,FF,0},	{"fmul",TERM,FF,0},
		{"fcom",TERM,F,0},	{"fcomp",TERM,F,0},
/*  [0,4]  */	{"fsub",TERM,FF,0},	{"fsubr",TERM,FF,0},
		{"fdiv",TERM,FF,0},	{"fdivr",TERM,FF,0} },
/*  [1,0]  */ { {"fld",TERM,F,0},	{"fxch",TERM,F,0},
		{"fnop",TERM,GO_ON,0},	{"fstp",TERM,F,0},
/*  [1,4]  */	INVALID,		INVALID,
		INVALID,		INVALID },
/*  [2,0]  */ { {"fcmovb",TERM,FF,0},	{"fcmove",TERM,FF,0},
		{"fcmovbe",TERM,FF,0},	{"fcmovu",TERM,FF,0},
/*  [2,4]  */	INVALID,		{"fucompp",TERM,GO_ON,0},
		INVALID,		INVALID },
/*  [3,0]  */ { {"fcmovnb",TERM,FF,0},	{"fcmovne",TERM,FF,0},
		{"fcmovnbe",TERM,FF,0},	{"fcmovnu",TERM,FF,0},
/*  [3,4]  */	INVALID,		{"fucomi",TERM,FF,0},
		{"fcomi",TERM,FF,0},	INVALID },
/*  [4,0]  */ { {"fadd",TERM,FF,0},	{"fmul",TERM,FF,0},
		{"fcom",TERM,F,0},	{"fcomp",TERM,F,0},
/*  [4,4]  */	{"fsub",TERM,FF,0},	{"fsubr",TERM,FF,0},
		{"fdiv",TERM,FF,0},	{"fdivr",TERM,FF,0} },
/*  [5,0]  */ { {"ffree",TERM,F,0},	{"fxch",TERM,F,0},
		{"fst",TERM,F,0},	{"fstp",TERM,F,0},
/*  [5,4]  */	{"fucom",TERM,F,0},	{"fucomp",TERM,F,0},
		INVALID,		INVALID },
/*  [6,0]  */ { {"faddp",TERM,FF,0},	{"fmulp",TERM,FF,0},
		{"fcomp",TERM,F,0},	{"fcompp",TERM,GO_ON,0},
/*  [6,4]  */	{"fsubp",TERM,FF,0},	{"fsubrp",TERM,FF,0},
		{"fdivp",TERM,FF,0},	{"fdivrp",TERM,FF,0} },
/*  [7,0]  */ { {"ffreep",TERM,F,0},	{"fxch",TERM,F,0},
		{"fstp",TERM,F,0},	{"fstp",TERM,F,0},
/*  [7,4]  */	{"fnstsw",TERM,M,0},	{"fucomip",TERM,FF,0},
		{"fcomip",TERM,FF,0},	INVALID },
};

static const struct instable opFP4[4][8] = {
/* bit pattern:	1101 1001 111x xxxx */
/*  [0,0]  */ { {"fchs",TERM,GO_ON,0},	{"fabs",TERM,GO_ON,0},
		INVALID,		INVALID,
/*  [0,4]  */	{"ftst",TERM,GO_ON,0},	{"fxam",TERM,GO_ON,0},
		INVALID,		INVALID },
/*  [1,0]  */ { {"fld1",TERM,GO_ON,0},	{"fldl2t",TERM,GO_ON,0},
		{"fldl2e",TERM,GO_ON,0},{"fldpi",TERM,GO_ON,0},
/*  [1,4]  */	{"fldlg2",TERM,GO_ON,0},{"fldln2",TERM,GO_ON,0},
		{"fldz",TERM,GO_ON,0},	INVALID },
/*  [2,0]  */ { {"f2xm1",TERM,GO_ON,0},	{"fyl2x",TERM,GO_ON,0},
		{"fptan",TERM,GO_ON,0},	{"fpatan",TERM,GO_ON,0},
/*  [2,4]  */	{"fxtract",TERM,GO_ON,0},{"fprem1",TERM,GO_ON,0},
		{"fdecstp",TERM,GO_ON,0},{"fincstp",TERM,GO_ON,0} },
/*  [3,0]  */ { {"fprem",TERM,GO_ON,0},	{"fyl2xp1",TERM,GO_ON,0},
		{"fsqrt",TERM,GO_ON,0},	{"fsincos",TERM,GO_ON,0},
/*  [3,4]  */	{"frndint",TERM,GO_ON,0},{"fscale",TERM,GO_ON,0},
		{"fsin",TERM,GO_ON,0},	{"fcos",TERM,GO_ON,0} },
};

static const struct instable opFP5[8] = {
/* bit pattern:	1101 1011 1110 0xxx */
/*  [0]  */	INVALID,		INVALID,
		{"fnclex",TERM,GO_ON,0},{"fninit",TERM,GO_ON,0},
/*  [4]  */	{"fsetpm",TERM,GO_ON,0},INVALID,
		INVALID,		INVALID,
};

/*
 * Main decode table for the op codes.  The first two nibbles
 * will be used as an index into the table.  If there is a
 * a need to further decode an instruction, the array to be
 * referenced is indicated with the other two entries being
 * empty.
 */
static const struct instable distable[16][16] = {
/* [0,0] */  {  {"addb",TERM,RMw,0},	{"add",TERM,RMw,1},
		{"addb",TERM,MRw,0},	{"add",TERM,MRw,1},
/* [0,4] */	{"addb",TERM,IA,0},	{"add",TERM,IA,1},
		{"push",TERM,SEG,0x03,INVALID_64},
					{"pop",TERM,SEG,0x03,INVALID_64},
/* [0,8] */	{"orb",TERM,RMw,0},	{"or",TERM,RMw,1},
		{"orb",TERM,MRw,0},	{"or",TERM,MRw,1},
/* [0,C] */	{"orb",TERM,IA,0},	{"or",TERM,IA,1},
		{"push",TERM,SEG,0x03,INVALID_64},
				    {"",(const struct instable *)op0F,TERM,0} },
/* [1,0] */  {  {"adcb",TERM,RMw,0},	{"adc",TERM,RMw,1},
		{"adcb",TERM,MRw,0},	{"adc",TERM,MRw,1},
/* [1,4] */	{"adcb",TERM,IA,0},	{"adc",TERM,IA,1},
		{"push",TERM,SEG,0x03,INVALID_64},
					{"pop",TERM,SEG,0x03,INVALID_64},
/* [1,8] */	{"sbbb",TERM,RMw,0},	{"sbb",TERM,RMw,1},
		{"sbbb",TERM,MRw,0},	{"sbb",TERM,MRw,1},
/* [1,C] */	{"sbbb",TERM,IA,0},	{"sbb",TERM,IA,1},
		{"push",TERM,SEG,0x03,INVALID_64},
					{"pop",TERM,SEG,0x03,INVALID_64} },
/* [2,0] */  {  {"andb",TERM,RMw,0},	{"and",TERM,RMw,1},
		{"andb",TERM,MRw,0},	{"and",TERM,MRw,1},
/* [2,4] */	{"andb",TERM,IA,0},	{"and",TERM,IA,1},
		{"%es:",TERM,OVERRIDE,0},
					{"daa",TERM,GO_ON,0,INVALID_64},
/* [2,8] */	{"subb",TERM,RMw,0},	{"sub",TERM,RMw,1},
		{"subb",TERM,MRw,0},	{"sub",TERM,MRw,1},
/* [2,C] */	{"subb",TERM,IA,0},	{"sub",TERM,IA,1},
		{"%cs:",TERM,OVERRIDE,0},
					{"das",TERM,GO_ON,0,INVALID_64} },
/* [3,0] */  {  {"xorb",TERM,RMw,0},	{"xor",TERM,RMw,1},
		{"xorb",TERM,MRw,0},	{"xor",TERM,MRw,1},
/* [3,4] */	{"xorb",TERM,IA,0},	{"xor",TERM,IA,1},
		{"%ss:",TERM,OVERRIDE,0},
					{"aaa",TERM,GO_ON,0,INVALID_64},
/* [3,8] */	{"cmpb",TERM,RMw,0},	{"cmp",TERM,RMw,1},
		{"cmpb",TERM,MRw,0},	{"cmp",TERM,MRw,1},
/* [3,C] */	{"cmpb",TERM,IA,0},	{"cmp",TERM,IA,1},
		{"%ds:",TERM,OVERRIDE,0},
					{"aas",TERM,GO_ON,0,INVALID_64} },
/* [4,0] */  {  {"inc",TERM,R,1,&opREX},{"inc",TERM,R,1,&opREX},
		{"inc",TERM,R,1,&opREX},{"inc",TERM,R,1,&opREX},
/* [4,4] */	{"inc",TERM,R,1,&opREX},{"inc",TERM,R,1,&opREX},
		{"inc",TERM,R,1,&opREX},{"inc",TERM,R,1,&opREX},
/* [4,8] */	{"dec",TERM,R,1,&opREX},{"dec",TERM,R,1,&opREX},
		{"dec",TERM,R,1,&opREX},{"dec",TERM,R,1,&opREX},
/* [4,C] */	{"dec",TERM,R,1,&opREX},{"dec",TERM,R,1,&opREX},
		{"dec",TERM,R,1,&opREX},{"dec",TERM,R,1,&opREX} },
/* [5,0] */  {  {"push",TERM,R,0x03},	{"push",TERM,R,0x03},
		{"push",TERM,R,0x03},	{"push",TERM,R,0x03},
/* [5,4] */	{"push",TERM,R,0x03},	{"push",TERM,R,0x03},
		{"push",TERM,R,0x03},	{"push",TERM,R,0x03},
/* [5,8] */	{"pop",TERM,R,0x03},	{"pop",TERM,R,0x03},
		{"pop",TERM,R,0x03},	{"pop",TERM,R,0x03},
/* [5,C] */	{"pop",TERM,R,0x03},	{"pop",TERM,R,0x03},
		{"pop",TERM,R,0x03},	{"pop",TERM,R,0x03} },
/* [6,0] */  {  {"pusha",TERM,GO_ON,1,INVALID_64},
					{"popa",TERM,GO_ON,1,INVALID_64},
		{"bound",TERM,MR,0,INVALID_64},
					{"arpl",TERM,RMw,0,&op_movsl},
/* [6,4] */	{"%fs:",TERM,OVERRIDE,0},
					{"%gs:",TERM,OVERRIDE,0},
		{"data16",TERM,DM,0},	{"addr16",TERM,AM,0},
/* [6,8] */	{"push",TERM,I,0x03},	{"imul",TERM,IMUL,1},
		{"push",TERM,Ib,0x03},	{"imul",TERM,IMUL,1},
/* [6,C] */	{"insb",TERM,GO_ON,0},	{"ins",TERM,GO_ON,1},
		{"outsb",TERM,GO_ON,0},	{"outs",TERM,GO_ON,1} },
/* [7,0] */  {  {"jo",TERM,BD,0},	{"jno",TERM,BD,0},
		{"jb",TERM,BD,0},	{"jae",TERM,BD,0},
/* [7,4] */	{"je",TERM,BD,0},	{"jne",TERM,BD,0},
		{"jbe",TERM,BD,0},	{"ja",TERM,BD,0},
/* [7,8] */	{"js",TERM,BD,0},	{"jns",TERM,BD,0},
		{"jp",TERM,BD,0},	{"jnp",TERM,BD,0},
/* [7,C] */	{"jl",TERM,BD,0},	{"jge",TERM,BD,0},
		{"jle",TERM,BD,0},	{"jg",TERM,BD,0} },
/* [8,0] */  {  {"",op80,TERM,0},	{"",op81,TERM,0},
		{"",op82,TERM,0},	{"",op83,TERM,0},
/* [8,4] */	{"testb",TERM,MRw,0},	{"test",TERM,MRw,1},
		{"xchgb",TERM,MRw,0},	{"xchg",TERM,MRw,1},
/* [8,8] */	{"movb",TERM,RMw,0},	{"mov",TERM,RMw,1},
		{"movb",TERM,MRw,0},	{"mov",TERM,MRw,1},
/* [8,C] */	{"mov",TERM,SM,1},	{"lea",TERM,MR,1},
		{"mov",TERM,MS,1},	{"pop",TERM,M,0x03} },
/* [9,0] */  {  {"nop",TERM,GO_ON,0},	{"xchg",TERM,RA,1},
		{"xchg",TERM,RA,1},	{"xchg",TERM,RA,1},
/* [9,4] */	{"xchg",TERM,RA,1},	{"xchg",TERM,RA,0},
		{"xchg",TERM,RA,1},	{"xchg",TERM,RA,1},
/* [9,8] */	{"",TERM,CBW,0},	{"",TERM,CWD,0},
		{"lcall",TERM,SO,0},	{"wait/",TERM,PREFIX,0},
/* [9,C] */	{"pushf",TERM,GO_ON,0},	{"popf",TERM,GO_ON,0},
		{"sahf",TERM,GO_ON,0},	{"lahf",TERM,GO_ON,0} },
/* [A,0] */  {  {"movb",TERM,OA,0},	{"mov",TERM,OA,1},
		{"movb",TERM,AO,0},	{"mov",TERM,AO,1},
/* [A,4] */	{"movsb",TERM,SD,0},	{"movs",TERM,SD,1},
		{"cmpsb",TERM,SD,0},	{"cmps",TERM,SD,1},
/* [A,8] */	{"testb",TERM,IA,0},	{"test",TERM,IA,1},
		{"stosb",TERM,AD,0},	{"stos",TERM,AD,1},
/* [A,C] */	{"lodsb",TERM,SA,0},	{"lods",TERM,SA,1},
		{"scasb",TERM,AD,0},	{"scas",TERM,AD,1} },
/* [B,0] */  {  {"movb",TERM,IR,0},	{"movb",TERM,IR,0},
		{"movb",TERM,IR,0},	{"movb",TERM,IR,0},
/* [B,4] */	{"movb",TERM,IR,0},	{"movb",TERM,IR,0},
		{"movb",TERM,IR,0},	{"movb",TERM,IR,0},
/* [B,8] */	{"mov",TERM,IR64,1},	{"mov",TERM,IR64,1},
		{"mov",TERM,IR64,1},	{"mov",TERM,IR64,1},
/* [B,C] */	{"mov",TERM,IR64,1},	{"mov",TERM,IR64,1},
		{"mov",TERM,IR64,1},	{"mov",TERM,IR64,1} },
/* [C,0] */  {  {"",opC0,TERM,0},	{"",opC1,TERM,0},
		{"ret",TERM,RET,1},	{"ret",TERM,GO_ON,0},
/* [C,4] */	{"les",TERM,MR,0,INVALID_64},
					{"lds",TERM,MR,0,INVALID_64},
		{"movb",TERM,IMw,0},	{"mov",TERM,IMw,1},
/* [C,8] */	{"enter",TERM,ENTER,0},	{"leave",TERM,GO_ON,0},
		{"lret",TERM,RET,1},	{"lret",TERM,GO_ON,0},
/* [C,C] */	{"int",TERM,INT3,0},	{"int",TERM,Ib,0},
		{"into",TERM,GO_ON,0,INVALID_64},
					{"iret",TERM,GO_ON,0} },
/* [D,0] */  {  {"",opD0,TERM,0},	{"",opD1,TERM,0},
		{"",opD2,TERM,0},	{"",opD3,TERM,0},
/* [D,4] */	{"aam",TERM,U,0,INVALID_64},
					{"aad",TERM,U,0,INVALID_64},
		{"falc",TERM,GO_ON,0},	{"xlat",TERM,GO_ON,0},
/* 287 instructions.  Note that although the indirect field		*/
/* indicates opFP1n2 for further decoding, this is not necessarily	*/
/* the case since the opFP arrays are not partitioned according to key1	*/
/* and key2.  opFP1n2 is given only to indicate that we haven't		*/
/* finished decoding the instruction.					*/
/* [D,8] */	{"",(const struct instable *)opFP1n2,TERM,0},
		{"",(const struct instable *)opFP1n2,TERM,0},
		{"",(const struct instable *)opFP1n2,TERM,0},
		{"",(const struct instable *)opFP1n2,TERM,0},
/* [D,C] */	{"",(const struct instable *)opFP1n2,TERM,0},
		{"",(const struct instable *)opFP1n2,TERM,0},
		{"",(const struct instable *)opFP1n2,TERM,0},
		{"",(const struct instable *)opFP1n2,TERM,0} },
/* [E,0] */  {  {"loopnz",TERM,BD,0},	{"loopz",TERM,BD,0},
		{"loop",TERM,BD,0},	{"jcxz",TERM,BD,0},
/* [E,4] */	{"inb",TERM,Pi,0},	{"in",TERM,Pi,1},
		{"outb",TERM,Po,0},	{"out",TERM,Po,1},
/* [E,8] */	{"call",TERM,D,0x03},	{"jmp",TERM,D,0x02},
		{"ljmp",TERM,SO,0},	{"jmp",TERM,BD,0},
/* [E,C] */	{"inb",TERM,Vi,0},	{"in",TERM,Vi,1},
		{"outb",TERM,Vo,0},	{"out",TERM,Vo,1} },
/* [F,0] */  {  {"lock/",TERM,PREFIX,0}, INVALID,
		{"repnz/",TERM,PREFIX,0}, {"repz/",TERM,PREFIX,0},
/* [F,4] */	{"hlt",TERM,GO_ON,0},	{"cmc",TERM,GO_ON,0},
		{"",opF6,TERM,0},	{"",opF7,TERM,0},
/* [F,8] */	{"clc",TERM,GO_ON,0},	{"stc",TERM,GO_ON,0},
		{"cli",TERM,GO_ON,0},	{"sti",TERM,GO_ON,0},
/* [F,C] */	{"cld",TERM,GO_ON,0},	{"std",TERM,GO_ON,0},
		{"",opFE,TERM,0},	{"",opFF,TERM,0} },
};

static const char *get_reg_name(int reg, int wbit, int data16, int rex)
{
	const char *reg_name;
	
	// A REX prefix takes precedent over a 66h prefix.
	if (rex != 0) {
		reg_name = REG32[reg + (REX_R(rex) << 3)][wbit + REX_W(rex)];
	} else if (data16) {
		reg_name = REG16[reg][wbit];
	} else {
		reg_name = REG32[reg][wbit];
	}
	
	return reg_name;
}

static const char *get_r_m_name(int r_m, int wbit, int data16, int rex)
{
	const char *reg_name;
	
	// A REX prefix takes precedent over a 66h prefix.
	if (rex != 0) {
		reg_name = REG32[r_m + (REX_B(rex) << 3)][wbit + REX_W(rex)];
	} else if (data16) {
		reg_name = REG16[r_m][wbit];
	} else {
		reg_name = REG32[r_m][wbit];
	}
	
	return reg_name;
}

// Returns the xmm register number referenced by reg and rex.
static unsigned int xmm_reg(int reg, int rex)
{
	return (reg + (REX_R(rex) << 3));
}

// Returns the xmm register number referenced by r_m and rex.
static unsigned int xmm_rm(int r_m, int rex)
{
	return (r_m + (REX_B(rex) << 3));
}

/*
 * This is passed to the llvm disassembler.
 */
static struct disassemble_info {
  enum bool verbose;
  /* Relocation information.  */
  struct relocation_info *sorted_relocs;
  uint32_t nsorted_relocs;
  struct relocation_info *ext_relocs;
  uint32_t next_relocs;
  struct relocation_info *loc_relocs;
  uint32_t nloc_relocs;
  struct dyld_bind_info *dbi;
  uint64_t ndbi;
  /* Symbol table.  */
  struct nlist *symbols;
  struct nlist_64 *symbols64;
  uint32_t nsymbols;
  /* Symbols sorted by address.  */
  struct symbol *sorted_symbols;
  uint32_t nsorted_symbols;
  /* String table.  */
  char *strings;
  uint32_t strings_size;
  /* Other useful info.  */
  uint32_t ncmds;
  uint32_t sizeofcmds;
  struct load_command *load_commands;
  enum byte_sex object_byte_sex;
  uint32_t *indirect_symbols;
  uint32_t nindirect_symbols;
  char *sect;
  uint32_t left;
  uint64_t addr;
  uint64_t sect_addr;
  cpu_type_t cputype;
  LLVMDisasmContextRef i386_dc;
  LLVMDisasmContextRef x86_64_dc;
  char *object_addr;
  uint64_t object_size;
  struct inst *inst;
  struct inst *insts;
  uint32_t ninsts;
  const char *class_name;
  const char *selector_name;
  char *method;
  char *demangled_name;
} dis_info;

/*
 * i386_disassemble()
 */
uint32_t
i386_disassemble(
char *sect,
uint32_t left,
uint64_t addr,
uint64_t sect_addr,
enum byte_sex object_byte_sex,
struct relocation_info *sorted_relocs,
uint32_t nsorted_relocs,
struct relocation_info *ext_relocs,
uint32_t next_relocs,
struct relocation_info *loc_relocs,
uint32_t nloc_relocs,
struct dyld_bind_info *dbi,
uint64_t ndbi,
struct nlist *symbols,
struct nlist_64 *symbols64,
uint32_t nsymbols,
struct symbol *sorted_symbols,
uint32_t nsorted_symbols,
char *strings,
uint32_t strings_size,
uint32_t *indirect_symbols,
uint32_t nindirect_symbols,
cpu_type_t cputype,
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum bool verbose,
enum bool llvm_mc,
LLVMDisasmContextRef i386_dc,
LLVMDisasmContextRef x86_64_dc,
char *object_addr,
uint64_t object_size,
struct inst *inst,
struct inst *insts,
uint32_t ninsts)
{
    char mnemonic[MAX_MNEMONIC+2]; /* one extra for suffix */
    const char *seg;
    const char *symbol0, *symbol1;
    const char *symadd0, *symsub0, *symadd1, *symsub1;
    uint32_t value0, value1;
    uint64_t imm0, imm1;
    uint32_t value0_size, value1_size;
    char result0[MAX_RESULT], result1[MAX_RESULT];
    const char *indirect_symbol_name;

    uint32_t i, length;
    unsigned char byte;
       unsigned char opcode_suffix;
    /* nibbles (4 bits) of the opcode */
    unsigned opcode1, opcode2, opcode3, opcode4, opcode5, prefix_byte;
    const struct instable *dp, *prefix_dp;
    uint32_t wbit, vbit;
    enum bool got_modrm_byte;
    uint32_t mode, reg, r_m;
    const char *reg_name;
    enum bool data16;		/* 16- or 32-bit data */
    enum bool addr16;		/* 16- or 32-bit addressing */
    enum bool sse2;		/* sse2 instruction using xmmreg's */
    enum bool mmx;		/* mmx instruction using mmreg's */
    unsigned char rex;		/* x86-64 REX prefix */

	if(left == 0){
	   printf("(end of section)\n");
	   return(0);
	}

	/* Use the llvm disassembler with the -q flag. */
	if(qflag || gflag){
	    LLVMDisasmContextRef dc;
	    char dst[8192];

	    dst[8191] = '\0';
	    dis_info.verbose = verbose;
	    dis_info.sorted_relocs = sorted_relocs;
	    dis_info.nsorted_relocs = nsorted_relocs;
	    dis_info.ext_relocs = ext_relocs;
	    dis_info.next_relocs = next_relocs;
	    dis_info.loc_relocs = loc_relocs;
	    dis_info.nloc_relocs = nloc_relocs;
	    dis_info.dbi = dbi;
	    dis_info.ndbi = ndbi;
	    dis_info.symbols = symbols;
	    dis_info.symbols64 = symbols64;
	    dis_info.nsymbols = nsymbols;
	    dis_info.sorted_symbols = sorted_symbols;
	    dis_info.nsorted_symbols = nsorted_symbols;
	    dis_info.strings = strings;
	    dis_info.strings_size = strings_size;
	    dis_info.load_commands = load_commands;
	    dis_info.object_byte_sex = object_byte_sex;
	    dis_info.indirect_symbols = indirect_symbols;
	    dis_info.nindirect_symbols = nindirect_symbols;
	    dis_info.ncmds = ncmds;
	    dis_info.sizeofcmds = sizeofcmds;
	    dis_info.sect = sect;
	    dis_info.left = left;
	    dis_info.addr = addr;
	    dis_info.sect_addr = sect_addr;
	    dis_info.cputype = cputype;
	    dis_info.object_addr = object_addr;
	    dis_info.object_size = object_size;
	    dis_info.inst = inst;
	    dis_info.insts = insts;
	    dis_info.ninsts = ninsts;
	    dis_info.demangled_name = NULL;
	    if(cputype == CPU_TYPE_I386)
		dc = i386_dc;
	    else
		dc = x86_64_dc;
	    length = (uint32_t)llvm_disasm_instruction(dc, (uint8_t *)sect,
						       left, addr, dst, 8191);
	    if(length != 0){
		if(inst == NULL || inst->print){
		    /* print the opcode bytes */
		    if(!Xflag && jflag){
			printf("\t");
			for(i = 0; i < length; i++)
			    printf("%02x", 0xff & sect[i]);
			for( ; i < 8; i++)
			    printf("  ");
		    }
		    /* print the disassembled instruction */
		    printf("%s\n", dst);
		}
	    }
	    else{
		if(inst == NULL || inst->print){
		    if(!Xflag && jflag)
			printf("\t%02x              ", 0xff & sect[0]);
		    printf("\t.byte 0x%02x #bad opcode\n", 0xff & sect[0]);
		}
		length = 1;
	    }
	    return(length);
	}

	memset(mnemonic, '\0', sizeof(mnemonic));
	seg = "";
	symbol0 = NULL;
	symbol1 = NULL;
	value0 = 0;
	value1 = 0;
	value0_size = 0;
	value1_size = 0;
	memset(result0, '\0', sizeof(result0));
	memset(result1, '\0', sizeof(result1));
	data16 = FALSE;
	addr16 = FALSE;
	sse2 = FALSE;
	mmx = FALSE;
	rex = 0;
	reg_name = NULL;
	wbit = 0;

	length = 0;
	byte = 0;
	opcode4 = 0; /* to remove a compiler warning only */
	opcode5 = 0; /* to remove a compiler warning only */
	r_m = 0;
	reg = 0;
	mode = 0;
	opcode3 = 0;

	/*
	 * As long as there is a prefix, the default segment register,
	 * addressing-mode, or data-mode in the instruction will be overridden.
	 * This may be more general than the chip actually is.
	 */
	prefix_dp = NULL;
	prefix_byte = 0;
	for(;;){
	    byte = get_value(sizeof(char), sect, &length, &left);
	    opcode1 = byte >> 4 & 0xf;
	    opcode2 = byte & 0xf;

	    dp = &distable[opcode1][opcode2];
	    if((cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64 &&
	       dp->arch64 != NULL)
		dp = dp->arch64;

	    if(dp->adr_mode == PREFIX){
		if(prefix_dp != NULL)
		    printf("%s", dp->name);
		else if(llvm_mc == TRUE && byte == 0x9b){
		    printf("wait\n");
		    return(length);
		}
		prefix_dp = dp;
		prefix_byte = byte;
	    }
	    else if(dp->adr_mode == AM){
		addr16 = !addr16;
		prefix_byte = byte;
	    }
	    else if(dp->adr_mode == DM){
		data16 = !data16;
		prefix_byte = byte;
	    }
	    else if(dp->adr_mode == OVERRIDE){
		seg = dp->name;
		prefix_byte = byte;
	    }
	    else if(dp->adr_mode == REX){
		rex = byte;
		/*
		 * REX is a prefix, but we don't set prefix_byte here because
		 * we use that to detect things related to the other prefixes
		 * and we don't want the existence of those bytes to be hidden
		 * by the presence of a REX prefix.
		 */
	    }
	    else
		break;
	}

	got_modrm_byte = FALSE;

	/*
	 * Some 386 instructions have 2 bytes of opcode before the mod_r/m
	 * byte so we need to perform a table indirection.
	 */
	if(dp->indirect == (const struct instable *)op0F){
	    byte = get_value(sizeof(char), sect, &length, &left);
	    opcode4 = byte >> 4 & 0xf;
	    opcode5 = byte & 0xf;
	    dp = &op0F[opcode4][opcode5];
	    if((cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64 &&
	       dp->arch64 != NULL)
		dp = dp->arch64;
	    if(dp->indirect == op0F38 || dp->indirect == op0F3A){
		/*
		 * MNI instructions are SSE2ish instructions with an
		 * extra byte.  Do the extra indirection here.
		 */
		byte = get_value(sizeof(char), sect, &length, &left);
		dp = &dp->indirect[byte];
	    }
	    /*
	     * SSE and SSE2 instructions have 3 bytes of opcode and the
	     * "third opcode byte" is before the other two (where the prefix
	     * byte would be).  This is why the prefix byte is saved above and
	     * the printing of the last prefix is delayed.
	     */
	    if(dp->adr_mode == SSE2 ||
	       dp->adr_mode == SSE2i ||
	       dp->adr_mode == SSE2i1 ||
	       dp->adr_mode == SSE2tm ||
	       dp->adr_mode == SSE2tfm ||
	       dp->adr_mode == SSE4 ||
	       dp->adr_mode == SSE4i ||
	       dp->adr_mode == SSE4MRw ||
	       dp->adr_mode == SSE4CRC ||
	       dp->adr_mode == SSE4CRCb ||
	       (byte == 0xc7 && prefix_byte == 0xf3)){ /* for vmxon */
		prefix_dp = NULL;
	    }
	    else{
		/*
		 * 3DNow! instructions have 2 bytes of opcode followed by their
		 * operands and then an instruction-specific suffix byte.
		 */
		if(dp->indirect == (const struct instable *)op0F0F){
		    data16 = FALSE;
		    mmx = TRUE;
		    if(got_modrm_byte == FALSE){
			got_modrm_byte = TRUE;
			byte = get_value(sizeof(char), sect, &length, &left);
			modrm_byte(&mode, &reg, &r_m, byte);
		    }
		    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size,
				result0);
		    opcode_suffix = get_value(sizeof(char), sect, &length,
					      &left);
		    dp = &op0F0F[opcode_suffix >> 4][opcode_suffix & 0x0F];
		}
		else if(dp->indirect == (const struct instable *)op0F01){
		    if(got_modrm_byte == FALSE){
			got_modrm_byte = TRUE;
			byte = get_value(sizeof(char), sect, &length, &left);
			modrm_byte(&mode, &reg, &r_m, byte);
			opcode3 = reg;
		    }
		    if(byte == 0xc8){
			data16 = FALSE;
			mmx = TRUE;
			dp = &op_monitor;
		    }
		    else if(byte == 0xc9){
			data16 = FALSE;
			mmx = TRUE;
			dp = &op_mwait;
		    }
		    else if(byte == 0xf9){
			data16 = FALSE;
			mmx = TRUE;
			dp = &op_rdtscp;
		    }
		    if((cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64){
			if(opcode3 == 0x7 && got_modrm_byte &&
			   mode == REG_ONLY && r_m == 0) {
			    dp = &op_swapgs;
			}
		    }
		    /*
		     * To get the 'q' suffix on all 0F 01 /0-3 opcodes in 64
		     * bit mode we set the REX_W here.
		     */
		    if((cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64 &&
		       (opcode3 == 0 || opcode3 == 1 || opcode3 == 2 ||
			opcode3 == 3))
			rex |= 0x8;
		}
		else{
		    /*
		     * Since the opcode is not an SSE or SSE2 instruction that
		     * uses the prefix byte as the "third opcode byte" print the
		     * delayed last prefix if any.
		     */
		    if(prefix_dp != NULL)
			printf("%s", prefix_dp->name);
		}
            }
	}
	else{
	    /*
	     * The "pause" Spin Loop Hint instruction is a "repz" prefix
	     * followed by a nop (0x90).
	     */
	    if(prefix_dp != NULL && prefix_byte == 0xf3 &&
	       opcode1 == 0x9 && opcode2 == 0x0){
		printf("pause\n");
		return(length);
	    }
	    /*
	     * Since the opcode is not an SSE or SSE2 instruction print the
	     * delayed last prefix if any.
	     */
	    if(prefix_dp != NULL){
		/*
		 * If the prefix is "repz" and the instruction is ins, outs,
		 * movs, lods, or stos then the name used is "rep".
		 */
		if(strcmp(prefix_dp->name, "repz/") == 0 &&
		   (byte == 0x6c || byte == 0x6d || /* ins */
		    byte == 0x6e || byte == 0x6f || /* outs */
		    byte == 0xa4 || byte == 0xa5 || /* movs */
		    byte == 0xac || byte == 0xad || /* lods */
		    byte == 0xaa || byte == 0xab))  /* stos */
		    printf("rep/");
		else
		    printf("%s", prefix_dp->name);
	    }
	}

	if(dp->indirect != TERM){
	    /*
	     * This must have been an opcode for which several instructions
	     * exist.  The opcode3 field further decodes the instruction.
	     */
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, (uint32_t *)&opcode3, &r_m, byte);
	    }
	    /*
	     * decode 287 instructions (D8-DF) from opcodeN
	     */
	    if(opcode1 == 0xD && opcode2 >= 0x8){
		/* instruction form 5 */
		if(opcode2 == 0xB && mode == 0x3 && opcode3 == 4)
		    dp = &opFP5[r_m];
		else if(opcode2 == 0xB && mode == 0x3 && opcode3 > 6){
		    printf(".byte 0x%01x%01x, 0x%01x%01x 0x%02x #bad opcode\n",
			   (unsigned int)opcode1, (unsigned int)opcode2,
			   (unsigned int)opcode4, (unsigned int)opcode5,
			   (unsigned int)byte);
		    return(length);
		}
		/* instruction form 4 */
		else if(opcode2 == 0x9 && mode == 0x3 && opcode3 >= 4)
		    dp = &opFP4[opcode3-4][r_m];
		/* instruction form 3 */
		else if(mode == 0x3)
		    dp = &opFP3[opcode2-8][opcode3];
		else /* instruction form 1 and 2 */
		    dp = &opFP1n2[opcode2-8][opcode3];
	    }
	    else
		dp = dp->indirect + opcode3;
		/* now dp points the proper subdecode table entry */
	}

	if(dp->indirect != TERM){
	    printf(".byte 0x%02x #bad opcode\n", (unsigned int)byte);
	    return(length);
	}
	
	/*
	 * Some addressing modes are implicitly 64-bit.  Set REX.W for those
	 * so we don't have to change the logic for them later.
	 */
	if((cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64){
	    if((dp->flags & IS_POINTER_SIZED) != 0){
		rex |= 0x8;	/* Set REX.W if it isn't already set */
	    }
	}

	/* setup the mnemonic with a possible suffix */
	if(dp->adr_mode != CBW && dp->adr_mode != CWD){
	    if((dp->flags & HAS_SUFFIX) != 0){
		if(data16 == TRUE)
		    sprintf(mnemonic, "%sw", dp->name);
		else{
		    if(dp->adr_mode == Mnol || dp->adr_mode == INM ||
		       dp->adr_mode == SM || dp->adr_mode == MS)
			sprintf(mnemonic, "%s", dp->name);
		    else if(REX_W(rex) != 0)
			sprintf(mnemonic, "%sq", dp->name);
		    else
			sprintf(mnemonic, "%sl", dp->name);
		}
	    }
	    else{
		sprintf(mnemonic, "%s", dp->name);
	    }
	    if(dp->adr_mode == BD){
		if(strcmp(seg, "%cs:") == 0){
		    sprintf(mnemonic, "%s,pn", mnemonic);
		    seg = "";
		}
		else if(strcmp(seg, "%ds:") == 0){
		    sprintf(mnemonic, "%s,pt", mnemonic);
		    seg = "";
		}
	    }
	}

	/*
	 * Each instruction has a particular instruction syntax format
	 * stored in the disassembly tables.  The assignment of formats
	 * to instructions was made by the author.  Individual formats
	 * are explained as they are encountered in the following
	 * switch construct.
	 */
	switch(dp -> adr_mode){

	case BSWAP:
	    reg = opcode5 & 0x7;
	    if(rex)
		reg_name = REG32[reg + (REX_B(rex) << 3)][1 + REX_W(rex)];
	    else
		reg_name = get_reg_name(reg, 1, data16, rex);
	    printf("%s\t%s\n", mnemonic, reg_name);
	    return(length);

	case XINST:
	    wbit = WBIT(opcode5);
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
		reg_name = get_reg_name(reg, wbit, data16, rex);
	    printf("%s\t%s,", mnemonic, reg_name);
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  "\n");
	    return(length);

	/* movsbl movsbw (0x0FBE) or movswl (0x0FBF) */
	/* movzbl movzbw (0x0FB6) or mobzwl (0x0FB7) */
	/* wbit lives in 2nd byte, note that operands are different sized */
	case MOVZ:
	    /* Get second operand first so data16 can be destroyed */
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    reg_name = get_reg_name(reg, LONGOPERAND, data16, rex);
	    wbit = WBIT(opcode5);
	    data16 = 1;
	    /* movslq (0x63) Move doubleword to quadword with sign-extension */
	    if(opcode1 != 0x6 && opcode2 != 0x3)
	        rex = 0;
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    printf("%s\t", mnemonic);
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  ",");
	    printf("%s\n", reg_name);
	    return(length);

	/* imul instruction, with either 8-bit or longer immediate */
	case IMUL:
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    wbit = LONGOPERAND;
	    GET_OPERAND(&symadd1, &symsub1, &value1, &value1_size, result1);
	    /* opcode 0x6B for byte, sign-extended displacement,
		0x69 for word(s) */
	    value0_size = OPSIZE(data16, opcode2 == 0x9, 0);
	    IMMEDIATE(&symadd0, &symsub0, &imm0, value0_size);
	    reg_name = get_reg_name(reg, wbit, data16, rex);
	    printf("%s\t$", mnemonic);
	    print_operand("", symadd0, symsub0, imm0, value0_size, "", ",");
	    print_operand(seg, symadd1, symsub1, value1, value1_size, result1,
			  ",");
	    printf("%s\n", reg_name);
	    return(length);

	/* memory or register operand to register, with 'w' bit	*/
	case MRw:
	case SSE4MRw:
	    /*
	     * If this is vmwrite in a 64-bit object the 0F 79
	     * opcode it results in a 64-bit operand.
	     * So to get the 64-bit register names in the disassembly we
	     * set the REX.W bit to indicate 64-bit operand size.
	     */
	    if((cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64 &&
	       opcode1 == 0x0 && opcode2 == 0xf &&
	       opcode4 == 0x7 && opcode5 == 0x9)
		rex |= 0x8;
	    wbit = WBIT(opcode2);
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    reg_name = get_reg_name(reg, wbit, data16, rex);
	    printf("%s\t", mnemonic);
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  ",");
	    printf("%s\n", reg_name);
	    return(length);

	/* register to memory or register operand, with 'w' bit	*/
	/* arpl happens to fit here also because it is odd */
	case RMw:
	    /*
	     * If this is vmread in a 64-bit object the 0F 78
	     * opcode it results in a 64-bit operand.
	     * So to get the 64-bit register names in the disassembly we
	     * set the REX.W bit to indicate 64-bit operand size.
	     */
	    if((cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64 &&
	       opcode1 == 0x0 && opcode2 == 0xf &&
	       opcode4 == 0x7 && opcode5 == 0x8)
		rex |= 0x8;
	    /* arpl, 0x63, always uses r16's */
	    if(opcode1 == 0x6 && opcode2 == 0x3)
		data16 = 1;
	    wbit = WBIT(opcode2);
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    reg_name = get_reg_name(reg, wbit, data16, rex);
	    printf("%s\t%s,", mnemonic, reg_name);
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  "\n");
	    return(length);

	/* SSE2 instructions with further prefix decoding dest to memory or
	   memory to dest depending on the opcode */
	case SSE2tfm:
	    data16 = FALSE;
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    switch(opcode4 << 4 | opcode5){
	    case 0x7e: /* movq & movd */
		if(prefix_byte == 0x66){
		    /* movd from xmm to r/m32 */
		    printf("%sd\t%%xmm%u,", mnemonic, xmm_reg(reg, rex));
		    wbit = LONGOPERAND;
		    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size,
				result0);
		    print_operand(seg, symadd0, symsub0, value0, value0_size,
				  result0, "\n");
		}
		else if(prefix_byte == 0xf0){
		    /* movq from mm to mm/m64 */
		    printf("%sd\t%%mm%u,", mnemonic, reg);
		    mmx = TRUE;
		    GET_OPERAND(&symadd1, &symsub1, &value1, &value1_size,
				result1);
		    print_operand(seg, symadd1, symsub1, value1, value1_size,
				  result1, "\n");
		}
		else if(prefix_byte == 0xf3){
		    /* movq from xmm2/mem64 to xmm1 */
		    printf("%sq\t", mnemonic);
		    sse2 = TRUE;
		    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size,
				result0);
		    print_operand(seg, symadd0, symsub0, value0, value0_size,
				  result0, ",");
		    printf("%%xmm%u\n", xmm_reg(reg, rex));
		}
		else{ /* no prefix_byte */
		    /* movd from mm to r/m32 */
		    printf("%sd\t%%mm%u,", mnemonic, reg);
		    wbit = LONGOPERAND;
		    GET_OPERAND(&symadd1, &symsub1, &value1, &value1_size,
				result1);
		    print_operand(seg, symadd1, symsub1, value1, value1_size,
				  result1, "\n");
		}
	    }
	    return(length);

	/* SSE2 instructions with further prefix decoding dest to memory */
	case SSE2tm:
	    data16 = FALSE;
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    sprintf(result0, "%%xmm%u", xmm_reg(reg, rex));
	    switch(opcode4 << 4 | opcode5){
	    case 0x11: /* movupd &         movups */
		       /*          movsd &        movss */
		sse2 = TRUE;
		if(prefix_byte == 0x66)
		    printf("%supd\t", mnemonic);
		else if(prefix_byte == 0xf2)
		    printf("%ssd\t", mnemonic);
		else if(prefix_byte == 0xf3)
		    printf("%sss\t", mnemonic);
		else /* no prefix_byte */
		    printf("%sups\t", mnemonic);
		break;
	    case 0x13: /*  movlpd &          movlps */
	    case 0x17: /*  movhpd &          movhps */
	    case 0x29: /*  movapd &  movasd */
	    case 0x2b: /* movntpd & movntsd */
		sse2 = TRUE;
		if(prefix_byte == 0x66)
		    printf("%spd\t", mnemonic);
		else if(prefix_byte == 0xf2)
		    printf("%ssd\t", mnemonic);
		else if(prefix_byte == 0xf3)
		    printf("%sss\t", mnemonic);
		else /* no prefix_byte */
		    printf("%sps\t", mnemonic);
		break;
	    case 0xd6: /* movq */
		if(prefix_byte == 0x66){
		    sse2 = TRUE;
		    printf("%sq\t", mnemonic);
		}
		else if(prefix_byte == 0xf2){
		    printf("%sdq2q\t", mnemonic);
		    sse2 = TRUE;
		    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size,
				result0);
		    print_operand(seg, symadd0, symsub0, value0, value0_size,
				  result0, ",");
		    sprintf(result1, "%%mm%u", reg);
		    printf("%s\n", result1);
		    return(length);
		}
		else if(prefix_byte == 0xf3){
		    printf("%sq2dq\t", mnemonic);
		    mmx = TRUE;
		    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size,
				result0);
		    print_operand(seg, symadd0, symsub0, value0, value0_size,
				  result0, ",");
		    sprintf(result1, "%%xmm%u", reg);
		    printf("%s\n", result1);
		    return(length);
		}
		break;
	    case 0x7f: /* movdqa, movdqu, movq */
		sse2 = TRUE;
		if(prefix_byte == 0x66)
		    printf("%sdqa\t", mnemonic);
		else if(prefix_byte == 0xf3)
		    printf("%sdqu\t", mnemonic);
		else{
		    sprintf(result0, "%%mm%u", reg);
		    printf("%sq\t", mnemonic);
		    mmx = TRUE;
		}
		break;
	    case 0xe7: /* movntdq & movntq */
		if(prefix_byte == 0x66){
		    printf("%stdq\t", mnemonic);
		}
		else{ /* no prefix_byte */
		    sprintf(result0, "%%mm%u", reg);
		    printf("%stq\t", mnemonic);
		    mmx = TRUE;
		}
		break;
	    }
	    printf("%s,", result0);
	    GET_OPERAND(&symadd1, &symsub1, &value1, &value1_size, result1);
	    print_operand(seg, symadd1, symsub1, value1, value1_size,
			  result1, "\n");
	    return(length);

	/* MNI instructions */
	case MNI:
	    data16 = FALSE;
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    if(prefix_byte == 0x66){
		sse2 = TRUE;
		sprintf(result1, "%%xmm%u", xmm_reg(reg, rex));
	    }
	    else{ /* no prefix byte */
		mmx = TRUE;
		sprintf(result1, "%%mm%u", reg);
	    }
	    printf("%s\t", mnemonic);
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    print_operand(seg, symadd0, symsub0, value0, value0_size,
			  result0, ",");
	    printf("%s\n", result1);
		return length;

	/* MNI instructions with 8-bit immediate */
	case MNIi:
	    data16 = FALSE;
	    if (got_modrm_byte == FALSE) {
			got_modrm_byte = TRUE;
			byte = get_value(sizeof(char), sect, &length, &left);
			modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    if(prefix_byte == 0x66){
		sse2 = TRUE;
		sprintf(result1, "%%xmm%u", xmm_reg(reg, rex));
	    }
	    else{ /* no prefix byte */
		mmx = TRUE;
		sprintf(result1, "%%mm%u", reg);
	    }
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    byte = get_value(sizeof(char), sect, &length, &left);
		printf("%s\t$0x%x,", mnemonic, byte);
		
	    print_operand(seg, symadd0, symsub0, value0, value0_size,
			  result0, ",");
	    printf("%s\n", result1);
		return length;

	/* SSE2 instructions with further prefix decoding */
	case SSE2:
	    data16 = FALSE;
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    sprintf(result1, "%%xmm%u", xmm_reg(reg, rex));
	    switch(opcode4 << 4 | opcode5){
	    case 0x14: /* unpcklpd &                 unpcklps */
	    case 0x15: /* unpckhpd &                 unpckhps */
	    case 0x28: /*   movapd & movasd */
	    case 0x51: /*   sqrtpd,  sqrtsd, sqrtss &  sqrtps */
	    case 0x52: /*                   rsqrtss & rsqrtps */
	    case 0x53: /*                     rcpss &   rcpps */
	    case 0x54: /*    andpd &  andsd */
	    case 0x55: /*   andnpd & andnsd */
	    case 0x56: /*     orpd &                    orps */
	    case 0x57: /*    xorpd &                   xorps */
	    case 0x58: /*    addpd &  addsd */
	    case 0x59: /*    mulpd,   mulsd,  mulss &   mulps */
	    case 0x5c: /*    subpd,   subsd,  subss &   subps */
	    case 0x5d: /*    minpd,   minsd,  minss &   minps */
	    case 0x5e: /*    divpd,   divsd,  divss &   divps */
	    case 0x5f: /*    maxpd,   maxsd,  maxss &   maxps */
		sse2 = TRUE;
		if(prefix_byte == 0x66)
		    printf("%spd\t", mnemonic);
		else if(prefix_byte == 0xf2)
		    printf("%ssd\t", mnemonic);
		else if(prefix_byte == 0xf3)
		    printf("%sss\t", mnemonic);
		else /* no prefix_byte */
		    printf("%sps\t", mnemonic);
		break;
	    case 0x12: /*   movlpd, movlps & movhlps */
		sse2 = TRUE;
		if(prefix_byte == 0x66)
		    printf("%slpd\t", mnemonic);
		else if(prefix_byte == 0xf2)
		    printf("movddup\t");
		else if(prefix_byte == 0xf3)
		    printf("movsldup\t");
		else{ /* no prefix_byte */
		    if(mode == REG_ONLY)
			printf("%shlps\t", mnemonic);
		    else
			printf("%slps\t", mnemonic);
		}
		break;
	    case 0x16: /*   movhpd, movhps & movlhps */
		sse2 = TRUE;
		if(prefix_byte == 0x66)
		    printf("%shpd\t", mnemonic);
		else if(prefix_byte == 0xf2)
		    printf("%shsd\t", mnemonic);
		else if(prefix_byte == 0xf3)
		    printf("movshdup\t");
		else{ /* no prefix_byte */
		    if(mode == REG_ONLY)
			printf("%slhps\t", mnemonic);
		    else
			printf("%shps\t", mnemonic);
		}
		break;
	    case 0x50: /* movmskpd &                 movmskps */
		sse2 = TRUE;
		reg_name = get_reg_name(reg, 1, data16, rex);
		strcpy(result1, reg_name);
		if(prefix_byte == 0x66)
		    printf("%spd\t", mnemonic);
		else /* no prefix_byte */
		    printf("%sps\t", mnemonic);
		break;
	    case 0x10: /*   movupd &                  movups */
		       /*             movsd & movss */
		sse2 = TRUE;
		if(prefix_byte == 0x66)
		    printf("%supd\t", mnemonic);
		else if(prefix_byte == 0xf2)
		    printf("%ssd\t", mnemonic);
		else if(prefix_byte == 0xf3)
		    printf("%sss\t", mnemonic);
		else /* no prefix_byte */
		    printf("%sups\t", mnemonic);
		break;
	    case 0x2a: /* cvtpi2pd, cvtsi2sd, cvtsi2ss & cvtpi2ps */
		if(prefix_byte == 0x66){
		    mmx = TRUE;
		    printf("%spi2pd\t", mnemonic);
		}
		else if(prefix_byte == 0xf2){
		    wbit = LONGOPERAND;
		    printf("%ssi2sd\t", mnemonic);
		}
		else if(prefix_byte == 0xf3){
		    wbit = LONGOPERAND;
		    printf("%ssi2ss\t", mnemonic);
		}
		else{ /* no prefix_byte */
		    mmx = TRUE;
		    printf("%spi2ps\t", mnemonic);
		}
		break;
	    case 0x2c: /* cvttpd2pi, cvttsd2si, cvttss2si & cvttps2pi */
		if(prefix_byte == 0x66){
		    sse2 = TRUE;
		    printf("%stpd2pi\t", mnemonic);
		    sprintf(result1, "%%mm%u", reg);
		}
		else if(prefix_byte == 0xf2){
		    sse2 = TRUE;
		    printf("%stsd2si\t", mnemonic);
		    reg_name = get_reg_name(reg, 1, data16, rex);
		    strcpy(result1, reg_name);
		}
		else if(prefix_byte == 0xf3){
		    sse2 = TRUE;
		    printf("%stss2si\t", mnemonic);
		    reg_name = get_reg_name(reg, 1, data16, rex);
		    strcpy(result1, reg_name);
		}
		else{ /* no prefix_byte */
		    sse2 = TRUE;
		    printf("%stps2pi\t", mnemonic);
		    sprintf(result1, "%%mm%u", reg);
		}
		break;
	    case 0x2d: /* cvtpd2pi, cvtsd2si, cvtss2si & cvtps2pi */
		if(prefix_byte == 0x66){
		    sse2 = TRUE;
		    printf("%spd2pi\t", mnemonic);
		    sprintf(result1, "%%mm%u", reg);
		}
		else if(prefix_byte == 0xf2){
		    sse2 = TRUE;
		    printf("%ssd2si\t", mnemonic);
		    reg_name = get_reg_name(reg, 1, data16, rex);
		    strcpy(result1, reg_name);
		}
		else if(prefix_byte == 0xf3){
		    sse2 = TRUE;
		    printf("%sss2si\t", mnemonic);
		    reg_name = get_reg_name(reg, 1, data16, rex);
		    strcpy(result1, reg_name);
		}
		else{ /* no prefix_byte */
		    sse2 = TRUE;
		    printf("%sps2pi\t", mnemonic);
		    sprintf(result1, "%%mm%u", reg);
		}
		break;
	    case 0x2e: /* ucomisd & ucomiss */
	    case 0x2f: /*  comisd &  comiss */
		sse2 = TRUE;
		if(prefix_byte == 0x66)
		    printf("%ssd\t", mnemonic);
		else /* no prefix_byte */
		    printf("%sss\t", mnemonic);
		break;
	    case 0xe0: /* pavgb */
	    case 0xe3: /* pavgw */
		if(prefix_byte == 0x66){
		    sse2 = TRUE;
		    printf("%s\t", mnemonic);
		}
		else{ /* no prefix_byte */
		    sprintf(result1, "%%mm%u", reg);
		    printf("%s\t", mnemonic);
		    mmx = TRUE;
		}
		break;
	    case 0xe6: /* cvttpd2dq, cvtdq2pd & cvtpd2dq */
		sse2 = TRUE;
		if(prefix_byte == 0x66)
		    printf("%stpd2dq\t", mnemonic);
		if(prefix_byte == 0xf3)
		    printf("%sdq2pd\t", mnemonic);
		else if(prefix_byte == 0xf2)
		    printf("%spd2dq\t", mnemonic);
		break;
	    case 0x5a: /* cvtpd2ps, cvtsd2ss, cvtss2sd & cvtps2pd */
		sse2 = TRUE;
		if(prefix_byte == 0x66)
		    printf("%spd2ps\t", mnemonic);
		else if(prefix_byte == 0xf2)
		    printf("%ssd2ss\t", mnemonic);
		else if(prefix_byte == 0xf3)
		    printf("%sss2sd\t", mnemonic);
		else /* no prefix_byte */
		    printf("%sps2pd\t", mnemonic);
		break;
	    case 0x5b: /* cvtdq2ps, cvttps2dq & cvtps2dq */
		sse2 = TRUE;
		if(prefix_byte == 0x66)
		    printf("%sps2dq\t", mnemonic);
		else if(prefix_byte == 0xf3)
		    printf("%stps2dq\t", mnemonic);
		else /* no prefix_byte */
		    printf("%sdq2ps\t", mnemonic);
		break;
	    case 0x60: /* punpcklbw */
	    case 0x61: /* punpcklwd */
	    case 0x62: /* punpckldq */
	    case 0x63: /* packsswb */
	    case 0x64: /* pcmpgtb */
	    case 0x65: /* pcmpgtw */
	    case 0x66: /* pcmpgtd */
	    case 0x67: /* packuswb */
	    case 0x68: /* punpckhbw */
	    case 0x69: /* punpckhwd */
	    case 0x6a: /* punpckhdq */
	    case 0x6b: /* packssdw */
	    case 0x74: /* pcmpeqb */
	    case 0x75: /* pcmpeqw */
	    case 0x76: /* pcmpeqd */
	    case 0xd1: /* psrlw */
	    case 0xd2: /* psrld */
	    case 0xd3: /* psrlq */
	    case 0xd4: /* paddq */
	    case 0xd5: /* pmullw */
	    case 0xd8: /* psubusb */
	    case 0xd9: /* psubusw */
	    case 0xdb: /* pand */
	    case 0xdc: /* paddusb */
	    case 0xdd: /* paddusw */
	    case 0xdf: /* pandn */
	    case 0xe1: /* psraw */
	    case 0xe2: /* psrad */
	    case 0xe5: /* pmulhw */
	    case 0xe8: /* psubsb */
	    case 0xe9: /* psubsw */
	    case 0xeb: /* por */
	    case 0xec: /* paddsb */
	    case 0xed: /* paddsw */
	    case 0xef: /* pxor */
	    case 0xf1: /* psllw */
	    case 0xf2: /* pslld */
	    case 0xf3: /* psllq */
	    case 0xf5: /* pmaddwd */
	    case 0xf8: /* psubb */
	    case 0xf9: /* psubw */
	    case 0xfa: /* psubd */
	    case 0xfb: /* psubq */
	    case 0xfc: /* paddb */
	    case 0xfd: /* paddw */
	    case 0xfe: /* paddd */
		if(prefix_byte == 0x66){
		    printf("%s\t", mnemonic);
		    sse2 = TRUE;
		}
		else{ /* no prefix_byte */
		    sprintf(result1, "%%mm%u", reg);
		    printf("%s\t", mnemonic);
		    mmx = TRUE;
		}
		break;
	    case 0x6c: /* punpcklqdq */
	    case 0x6d: /* punpckhqdq */
		sse2 = TRUE;
		if(prefix_byte == 0x66)
		    printf("%sqdq\t", mnemonic);
		break;
	    case 0x6f: /* movdqa, movdqu & movq */
		if(prefix_byte == 0x66){
		    sse2 = TRUE;
		    printf("%sdqa\t", mnemonic);
		}
		else if(prefix_byte == 0xf3){
		    sse2 = TRUE;
		    printf("%sdqu\t", mnemonic);
		}
		else{ /* no prefix_byte */
		    sprintf(result1, "%%mm%u", reg);
		    printf("%sq\t", mnemonic);
		    mmx = TRUE;
		}
		break;
	    case 0xd6: /* movdq2q & movq2dq */
		if(prefix_byte == 0xf2){
		    sprintf(result1, "%%mm%u", reg);
		    printf("%sdq2q\t", mnemonic);
		    sse2 = TRUE;
		}
		else if(prefix_byte == 0xf3){
		    printf("%sq2dq\t", mnemonic);
		    mmx = TRUE;
		}
		break;
	    case 0x6e: /* movd */
		if(prefix_byte == 0x66){
		    printf("%s\t", mnemonic);
		    wbit = LONGOPERAND;
		}
		else{ /* no prefix_byte */
		    sprintf(result1, "%%mm%u", reg);
		    printf("%s\t", mnemonic);
		    wbit = LONGOPERAND;
		}
		break;
	    case 0xd0: /* addsubpd */
	    case 0x7c: /* haddp */
	    case 0x7d: /* hsubp */
		if(prefix_byte == 0x66){
		    printf("%sd\t", mnemonic);
		    sse2 = TRUE;
		}
		else if(prefix_byte == 0xf2){
		    printf("%ss\t", mnemonic);
		    sse2 = TRUE;
		}
		else{ /* no prefix_byte */
		    sprintf(result1, "%%mm%u", reg);
		    printf("%s\t", mnemonic);
		    mmx = TRUE;
		}
		break;
	    case 0xd7: /* pmovmskb */
		if(prefix_byte == 0x66){
		    reg_name = get_reg_name(reg, 1, data16, rex);
		    printf("%s\t%%xmm%u,%s\n", mnemonic, xmm_rm(r_m, rex),
			   reg_name);
		    return(length);
		}
		else{ /* no prefix_byte */
		    reg_name = get_reg_name(reg, 1, data16, rex);
		    printf("%s\t%%mm%u,%s\n", mnemonic, r_m, reg_name);
		    return(length);
		}
		break;
	    case 0xda: /* pminub */
	    case 0xde: /* pmaxub */
	    case 0xe4: /* pmulhuw */
	    case 0xea: /* pminsw */
	    case 0xee: /* pmaxsw */
	    case 0xf4: /* pmuludq */
	    case 0xf6: /* psadbw */
		if(prefix_byte == 0x66){
		    sse2 = TRUE;
		    printf("%s\t", mnemonic);
		}
		else{ /* no prefix_byte */
		    sprintf(result1, "%%mm%u", reg);
		    printf("%s\t", mnemonic);
		    mmx = TRUE;
		}
		break;
	    case 0xf0: /* lddqu */
		printf("%s\t", mnemonic);
		sse2 = TRUE;
		break;
	    case 0xf7: /* maskmovdqu & maskmovq */
		sse2 = TRUE;
		if(prefix_byte == 0x66)
		    printf("%sdqu\t", mnemonic);
		else{ /* no prefix_byte */
		    printf("%sq\t%%mm%u,%%mm%u\n", mnemonic, r_m, reg);
		    return(length);
		}
		break;
	    }
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    print_operand(seg, symadd0, symsub0, value0, value0_size,
			  result0, ",");
	    printf("%s\n", result1);
	    return(length);

	/* SSE4 instructions */
	case SSE4:
	    sse2 = TRUE;
	    data16 = FALSE;
	    wbit = LONGOPERAND;
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    printf("%s\t", mnemonic);
	    sprintf(result1, "%%xmm%u", xmm_reg(reg, rex));
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    print_operand(seg, symadd0, symsub0, value0, value0_size,
			  result0, ",");
	    printf("%s\n", result1);
	    return(length);

	/* SSE4 instructions with 8 bit immediate */
	case SSE4i:
	    sse2 = TRUE;
	    data16 = FALSE;
	    wbit = LONGOPERAND;
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    byte = get_value(sizeof(char), sect, &length, &left);
	    printf("%s\t$0x%x,", mnemonic, byte);
	    print_operand(seg, symadd0, symsub0, value0, value0_size,
			  result0, ",");
	    printf("%%xmm%u\n", xmm_reg(reg, rex));
	    return(length);

	/* SSE4 instructions with dest to memory and 8-bit immediate */
	case SSE4itm:
	    sse2 = FALSE;
	    data16 = FALSE;
	    wbit = LONGOPERAND;
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    byte = get_value(sizeof(char), sect, &length, &left);
	    if(dp == &op0F3A[0x16]){
		if(rex != 0)
		    printf("%sq\t$0x%x,", mnemonic, byte);
		else
		    printf("%sd\t$0x%x,", mnemonic, byte);
	    }
	    else
		printf("%s\t$0x%x,", mnemonic, byte);
	    printf("%%xmm%u,", xmm_reg(reg, rex));
	    print_operand(seg, symadd0, symsub0, value0, value0_size,
			  result0, "\n");
	    return(length);

	/* SSE4 instructions with src from memory and 8-bit immediate */
	case SSE4ifm:
	    sse2 = FALSE;
	    data16 = FALSE;
	    wbit = LONGOPERAND;
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    byte = get_value(sizeof(char), sect, &length, &left);
	    if(dp == &op0F3A[0x22]){
		if(rex != 0)
		    printf("%sq\t$0x%x,", mnemonic, byte);
		else
		    printf("%sd\t$0x%x,", mnemonic, byte);
	    }
	    else
		printf("%s\t$0x%x,", mnemonic, byte);
	    print_operand(seg, symadd0, symsub0, value0, value0_size,
			  result0, ",");
	    printf("%%xmm%u\n", xmm_reg(reg, rex));
	    return(length);

	/* SSE4.2 instructions memory or register operand to register */
	case SSE4CRCb:
	    wbit = 0;
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    /*
	     * This is to get the byte register names for SSE4CRCb opcodes.
	     */
	    if(mode == REG_ONLY){
		strcpy(result0, REG64_BYTE[(REX_B(rex) << 3) | r_m]);
		symadd0 = NULL;
		symsub0 = NULL;
		value0 = 0;
		value0_size = 0;
	    }
	    else
		GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    reg_name = get_reg_name(reg, 1 /* wbit */, 0 /* data16 */, rex);
	    printf("%s\t", mnemonic);
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  ",");
	    printf("%s\n", reg_name);
	    return(length);

	case SSE4CRC:
	    wbit = 1;
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    reg_name = get_reg_name(reg, 1 /* wbit */, 0 /* data16 */, rex);
	    printf("%s\t", mnemonic);
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  ",");
	    printf("%s\n", reg_name);
	    return(length);

	/* SSE2 instructions with 8 bit immediate with further prefix decoding*/
	case SSE2i:
	    data16 = FALSE;
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    /* pshufw */
	    if((opcode4 << 4 | opcode5) == 0x70 && prefix_byte == 0)
		mmx = TRUE;
	    /* pinsrw */
	    else if((opcode4 << 4 | opcode5) == 0xc4)
		wbit = LONGOPERAND;
	    else
		sse2 = TRUE;
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    byte = get_value(sizeof(char), sect, &length, &left);

	    switch(opcode4 << 4 | opcode5){
	    case 0x70: /* pshufd, pshuflw, pshufhw & pshufw */
		if(prefix_byte == 0x66)
		    printf("%sfd\t$0x%x,", mnemonic, byte);
		else if(prefix_byte == 0xf2)
		    printf("%sflw\t$0x%x,", mnemonic, byte);
		else if(prefix_byte == 0xf3)
		    printf("%sfhw\t$0x%x,", mnemonic, byte);
		else{ /* no prefix_byte */
		    printf("%sfw\t$0x%x,", mnemonic, byte);
		    print_operand(seg, symadd0, symsub0, value0, value0_size,
				  result0, ",");
		    printf("%%mm%u\n", reg);
		    return(length);
		}
		break;
	    case 0xc4: /* pinsrw */
		if(prefix_byte == 0x66){
		    printf("%s\t$0x%x,", mnemonic, byte);
		}
		else{ /* no prefix_byte */
		    printf("%s\t$0x%x,", mnemonic, byte);
		    print_operand(seg, symadd0, symsub0, value0, value0_size,
				  result0, ",");
		    printf("%%mm%u\n", reg);
		    return(length);
		}
		break;
	    case 0xc5: /* pextrw */
		if(prefix_byte == 0x66){
		    reg_name = get_reg_name(reg, 1, data16, rex);
		    printf("%s\t$0x%x,%%xmm%u,%s\n", mnemonic, byte,
			   xmm_rm(r_m, rex), reg_name);
		    return(length);
		}
		else{ /* no prefix_byte */
		    reg_name = get_reg_name(reg, 1, data16, rex);
		    printf("%s\t$0x%x,%%mm%u,%s\n", mnemonic, byte, r_m,
			   reg_name);
		    return(length);
		}
		break;
	    default:
		if(prefix_byte == 0x66)
		    printf("%spd\t$0x%x,", mnemonic, byte);
		else if(prefix_byte == 0xf2)
		    printf("%ssd\t$0x%x,", mnemonic, byte);
		else if(prefix_byte == 0xf3)
		    printf("%sss\t$0x%x,", mnemonic, byte);
		else /* no prefix_byte */
		    printf("%sps\t$0x%x,", mnemonic, byte);
		break;
	    }
	    print_operand(seg, symadd0, symsub0, value0, value0_size,
			  result0, ",");
	    printf("%%xmm%u\n", xmm_reg(reg, rex));
	    return(length);

	/* SSE2 instructions with 8 bit immediate and only 1 reg */
	case SSE2i1:
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    byte = get_value(sizeof(char), sect, &length, &left);
	    switch(opcode4 << 4 | opcode5){
	    case 0x71: /* psrlw, psllw, psraw & psrld */
		if(prefix_byte == 0x66){
		    if(reg == 0x2)
			printf("%srlw\t$0x%x,", mnemonic, byte);
		    else if(reg == 0x4)
			printf("%sraw\t$0x%x,", mnemonic, byte);
		    else if(reg == 0x6)
			printf("%sllw\t$0x%x,", mnemonic, byte);
		}
		else{ /* no prefix_byte */
		    if(reg == 0x2)
			printf("%srlw\t$0x%x,", mnemonic, byte);
		    else if(reg == 0x4)
			printf("%sraw\t$0x%x,", mnemonic, byte);
		    else if(reg == 0x6)
			printf("%sllw\t$0x%x,", mnemonic, byte);
		    printf("%%mm%u\n", r_m);
		    return(length);
		}
		break;
	    case 0x72: /* psrld, pslld & psrad */
		if(prefix_byte == 0x66){
		    if(reg == 0x2)
			printf("%srld\t$0x%x,", mnemonic, byte);
		    else if(reg == 0x4)
			printf("%srad\t$0x%x,", mnemonic, byte);
		    else if(reg == 0x6)
			printf("%slld\t$0x%x,", mnemonic, byte);
		}
		else{ /* no prefix_byte */
		    if(reg == 0x2)
			printf("%srld\t$0x%x,", mnemonic, byte);
		    else if(reg == 0x4)
			printf("%srad\t$0x%x,", mnemonic, byte);
		    else if(reg == 0x6)
			printf("%slld\t$0x%x,", mnemonic, byte);
		    printf("%%mm%u\n", r_m);
		    return(length);
		}
		break;
	    case 0x73: /* pslldq & psrldq, psrlq & psllq */
		if(prefix_byte == 0x66){
		    if(reg == 0x7)
			printf("%slldq\t$0x%x,", mnemonic, byte);
		    else if(reg == 0x3)
			printf("%srldq\t$0x%x,", mnemonic, byte);
		    else if(reg == 0x2)
			printf("%srlq\t$0x%x,", mnemonic, byte);
		    else if(reg == 0x6)
			printf("%sllq\t$0x%x,", mnemonic, byte);
		}
		else{ /* no prefix_byte */
		    if(reg == 0x2)
			printf("%srlq\t$0x%x,", mnemonic, byte);
		    else if(reg == 0x6)
			printf("%sllq\t$0x%x,", mnemonic, byte);
		    printf("%%mm%u\n", r_m);
		    return(length);
		}
		break;
	    }
	    printf("%%xmm%u\n", xmm_rm(r_m, rex));
	    return(length);

       /* 3DNow instructions */
       case AMD3DNOW:
               printf("%s\t", mnemonic);
           sprintf(result1, "%%mm%u", reg);
           print_operand(seg, symadd0, symsub0, value0, value0_size,
                         result0, ",");
           printf("%s\n", result1);
           return(length);

	/* prefetch instructions */
	case PFCH:
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    switch(reg){
	    case 0:
		printf("%snta", dp->name);
		break;
	    case 1:
		printf("%st0", dp->name);
		break;
	    case 2:
		printf("%st1", dp->name);
		break;
	    case 3:
		printf("%st2", dp->name);
		break;
	    }
	    if(data16 == TRUE)
		printf("w");
	    printf("\t");
           GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
           print_operand(seg, symadd0, symsub0, value0, value0_size,
                         result0, "\n");
           return(length);

       /* 3DNow! prefetch instructions */
       case PFCH3DNOW:
           if(got_modrm_byte == FALSE){
               got_modrm_byte = TRUE;
               byte = get_value(sizeof(char), sect, &length, &left);
               modrm_byte(&mode, &reg, &r_m, byte);
           }
           switch(reg){
           case 0:
               printf("%s\t", dp->name);
               break;
           case 1:
               printf("%sw\t", dp->name);
               break;
           }
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    print_operand(seg, symadd0, symsub0, value0, value0_size,
			  result0, "\n");
	    return(length);

	/* sfence & clflush */
	case SFEN:
	    if(mode == REG_ONLY && r_m == 0){
		printf("sfence\n");
		return(length);
	    }
	    printf("%s\t", mnemonic);
	    reg = opcode3;
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    print_operand(seg, symadd0, symsub0, value0, value0_size,
			  result0, "\n");
	    return(length);

	/* Double shift. Has immediate operand specifying the shift. */
	case DSHIFT:
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    wbit = LONGOPERAND;
	    GET_OPERAND(&symadd1, &symsub1, &value1, &value1_size, result1);
	    value0_size = sizeof(char);
	    IMMEDIATE(&symadd0, &symsub0, &imm0, value0_size);
	    reg_name = get_reg_name(reg, wbit, data16, rex);
	    printf("%s\t$", mnemonic);
	    print_operand("", symadd0, symsub0, imm0, value0_size, "", ",");
	    printf("%s,", reg_name);
	    print_operand(seg, symadd1, symsub1, value1, value1_size, result1,
			  "\n");
	    return(length);

	/* Double shift. With no immediate operand, specifies using %cl. */
	case DSHIFTcl:
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    wbit = LONGOPERAND;
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    reg_name = get_reg_name(reg, wbit, data16, rex);
	    printf("%s\t%%cl,%s,", mnemonic, reg_name);
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  "\n");
	    return(length);

	/* immediate to memory or register operand */
	case IMlw:
	    wbit = WBIT(opcode2);
	    GET_OPERAND(&symadd1, &symsub1, &value1, &value1_size, result1);
	    /* A long immediate is expected for opcode 0x81, not 0x80 & 0x83 */
	    value0_size = OPSIZE(data16, opcode2 == 1, 0);
	    IMMEDIATE(&symadd0, &symsub0, &imm0, value0_size);
	    printf("%s\t$", mnemonic);
	    print_operand("", symadd0, symsub0, imm0, value0_size, "", ",");
	    print_operand(seg, symadd1, symsub1, value1, value1_size, result1,
			  "\n");
	    return(length);

	/* immediate to memory or register operand with the 'w' bit present */
	case IMw:
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    wbit = WBIT(opcode2);
	    GET_OPERAND(&symadd1, &symsub1, &value1, &value1_size, result1);
	    value0_size = OPSIZE(data16, wbit, 0);
	    IMMEDIATE(&symadd0, &symsub0, &imm0, value0_size);
	    printf("%s\t$", mnemonic);
	    print_operand("", symadd0, symsub0, imm0, value0_size, "", ",");
	    print_operand(seg, symadd1, symsub1, value1, value1_size, result1,
			  "\n");
	    return(length);

	/* immediate to register with register in low 3 bits of op code */
	case IR:
	    wbit = (opcode2 >> 3) & 0x1; /* w-bit here (with regs) is bit 3 */
	    reg = REGNO(opcode2);
	    value0_size = OPSIZE(data16, wbit, 0);
	    IMMEDIATE(&symadd0, &symsub0, &imm0, value0_size);
	    reg_name = get_r_m_name(reg, wbit, data16, rex);
	    printf("%s\t$", mnemonic);
	    print_operand("", symadd0, symsub0, imm0, value0_size, "", ",");
	    printf("%s\n", reg_name);
	    return(length);

	/* immediate to register with register in low 3 bits of op code,
	   possibly with a 64-bit immediate */
	case IR64:
	    wbit = (opcode2 >> 3) & 0x1; /* w-bit here (with regs) is bit 3 */
	    reg = REGNO(opcode2);
	    value0_size = OPSIZE(data16, wbit, REX_W(rex));
	    IMMEDIATE(&symadd0, &symsub0, &imm0, value0_size);
	    reg_name = get_r_m_name(reg, wbit, data16, rex);
	    printf("%s\t$", mnemonic);
	    print_operand("", symadd0, symsub0, imm0, value0_size, "", ",");
	    printf("%s\n", reg_name);
	    return(length);

	/* memory operand to accumulator */
	case OA:
	    if((cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64){
		value0_size = OPSIZE(addr16, LONGOPERAND, 1);
		if(opcode1 == 0xa && opcode2 == 0x0){
		    strcpy(mnemonic, "movabsb");
		    /*
		     * The REX 64-bit operand-size, REX.W aka 0x8, has no effect
		     * on byte size operations. Clear it to get the correct
		     * register from get_reg_name().
		     */
		    rex &= ~0x8;
		}
		else if(opcode1 == 0xa && opcode2 == 0x1){
		    if(rex != 0)
			strcpy(mnemonic, "movabsq");
		    else if(data16 == TRUE)
			strcpy(mnemonic, "movabsw");
		    else
			strcpy(mnemonic, "movabsl");
		}
	    }
	    else
		value0_size = OPSIZE(addr16, LONGOPERAND, 0);
	    IMMEDIATE(&symadd0, &symsub0, &imm0, value0_size);
	    printf("%s\t", mnemonic);
	    print_operand(seg, symadd0, symsub0, imm0, value0_size, "", ",");
	    wbit = WBIT(opcode2);
	    reg_name = get_reg_name(0, wbit, data16, rex);
	    printf("%s\n", reg_name);
	    return(length);

	/* accumulator to memory operand */
	case AO:
	    if((cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64){
		value0_size = OPSIZE(addr16, LONGOPERAND, 1);
		if(opcode1 == 0xa && opcode2 == 0x2){
		    strcpy(mnemonic, "movabsb");
		    /*
		     * The REX 64-bit operand-size, REX.W aka 0x8, has no effect
		     * on byte size operations. Clear it to get the correct
		     * register from get_reg_name().
		     */
		    rex &= ~0x8;
		}
		else if(opcode1 == 0xa && opcode2 == 0x3){
		    if(rex != 0)
			strcpy(mnemonic, "movabsq");
		    else if(data16 == TRUE)
			strcpy(mnemonic, "movabsw");
		    else
			strcpy(mnemonic, "movabsl");
		}
	    }
	    else
		value0_size = OPSIZE(addr16, LONGOPERAND, 0);
	    IMMEDIATE(&symadd0, &symsub0, &imm0, value0_size);
	    wbit = WBIT(opcode2);
	    reg_name = get_reg_name(0, wbit, data16, rex);
	    printf("%s\t%s,", mnemonic, reg_name);
	    print_operand(seg, symadd0, symsub0, imm0, value0_size, "", "\n");
	    return(length);

	/* memory or register operand to segment register */
	case MS:
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    wbit = LONGOPERAND;
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    printf("%s\t", mnemonic);
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  ",");
	    printf("%s\n", SEGREG[reg]);
	    return(length);

	/* segment register to memory or register operand	*/
	case SM:
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    wbit = LONGOPERAND;
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    printf("%s\t%s,", mnemonic, SEGREG[reg]);
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  "\n");
	    return(length);

	/* rotate or shift instrutions, which may shift by 1 or */
	/* consult the cl register, depending on the 'v' bit	*/
	case Mv:
	    vbit = VBIT(opcode2);
	    wbit = WBIT(opcode2);
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    /* When vbit is set, register is an operand, otherwise just $0x1 */
	    reg_name = vbit ? "%cl," : "" ;
	    printf("%s\t%s", mnemonic, reg_name);
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  "\n");
	    return(length);

	/* immediate rotate or shift instrutions, which may or */
	/* may not consult the cl register, depending on the 'v' bit */
	case MvI:
	    vbit = VBIT(opcode2);
	    wbit = WBIT(opcode2);
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    value1_size = sizeof(char);
	    IMMEDIATE(&symadd1, &symsub1, &imm0, value1_size);
	    /* When vbit is set, register is an operand, otherwise just $0x1 */
	    reg_name = vbit ? "%cl," : "" ;
	    printf("%s\t$", mnemonic);
	    print_operand("", symadd1, symsub1, imm0, value1_size, "", ",");
	    printf("%s", reg_name);
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  "\n");
	    return(length);

	case MIb:
	    wbit = LONGOPERAND;
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    value1_size = sizeof(char);
	    IMMEDIATE(&symadd1, &symsub1, &imm0, value1_size);
	    printf("%s\t$", mnemonic);
	    print_operand("", symadd1, symsub1, imm0, value1_size, "", ",");
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  "\n");
	    return(length);

	/* single memory or register operand with 'w' bit present */
	case Mw:
	    wbit = WBIT(opcode2);
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    printf("%s\t", mnemonic);
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  "\n");
	    return(length);

	/* single memory or register operand but don't use 'l' suffix */
	case Mnol:
	/* single memory or register operand */
	case M:
	    if(opcode1 == 0x0 && opcode2 == 0xf &&
	       opcode4 == 0x0 && opcode5 == 0x1){
		switch(byte){
		case 0xc1:
		    printf("vmcall\n");
		    return(length);
		case 0xc2:
		    printf("vmlaunch\n");
		    return(length);
		case 0xc3:
		    printf("vmresume\n");
		    return(length);
		case 0xc4:
		    printf("vmxoff\n");
		    return(length);
		}
	    }
	    if(opcode1 == 0x0 && opcode2 == 0xf && byte == 0xc7){
		if(prefix_byte == 0x66)
		    sprintf(mnemonic, "vmclear");
		else if(prefix_byte == 0xf3)
		    sprintf(mnemonic, "vmxon");
		else{
		    if(got_modrm_byte == FALSE){
			got_modrm_byte = TRUE;
			byte = get_value(sizeof(char), sect, &length, &left);
			modrm_byte(&mode, &reg, &r_m, byte);
		    }
		    if(reg == 6)
			sprintf(mnemonic, "vmptrld");
		    else if(reg == 7)
			sprintf(mnemonic, "vmptrst");
		    else if(reg == 1 && REX_W(rex))
			sprintf(mnemonic, "cmpxchg16b");
		}
	    }
	    /*
	     * Hacks for lldt, lmsw, ltr, verr and verw which take only a
	     * r/m16 operands.
	     */
	    if(opcode1 == 0 && opcode2 == 0xf && opcode4 == 0 && opcode5 == 1 &&
	       (opcode3 == 6))
		data16 = TRUE;
	    if(opcode1 == 0 && opcode2 == 0xf && opcode4 == 0 && opcode5 == 0 &&
	       (opcode3 == 2 || opcode3 == 3 || opcode3 == 4 || opcode3 == 5))
		data16 = TRUE;
	    /*
	     * Hacks for fnstsw which take only a r/m16 operand.
	     */
	    if((opcode1 == 0xd && opcode2 == 0xf && byte == 0xe0) ||
	       (opcode1 == 0xd && opcode2 == 0xd && opcode3 == 0x7))
		data16 = TRUE;
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    wbit = LONGOPERAND;
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    printf("%s\t", mnemonic);
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  "\n");
	    return(length);

	/* single memory or register operand */
	case Mb:
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    wbit = BYTEOPERAND;
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    printf("%s\t", mnemonic);
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  "\n");
	    return(length);

	case SREG: /* special register */
	    byte = get_value(sizeof(char), sect, &length, &left);
	    modrm_byte(&mode, &reg, &r_m, byte);
	    vbit = 0;
	    switch(opcode5){
	    case 2:
		vbit = 1;
		/* fall thru */
	    case 0: 
		if(llvm_mc == TRUE){
		    if((cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64)
		        reg_name = LLVM_MC_64_CONTROLREG[reg+(REX_R(rex) << 3)];
		    else
		        reg_name = LLVM_MC_32_CONTROLREG[reg+(REX_R(rex) << 3)];
		}
		else{
		    reg_name = CONTROLREG[reg + (REX_R(rex) << 3)];
		}
		break;
	    case 3:
		vbit = 1;
		/* fall thru */
	    case 1:
		if(llvm_mc == TRUE)
		    reg_name = LLVM_MC_DEBUGREG[reg + (REX_R(rex) << 3)];
		else
		    reg_name = DEBUGREG[reg + (REX_R(rex) << 3)];
		break;
	    case 6:
		vbit = 1;
		/* fall thru */
	    case 4:
		reg_name = TESTREG[reg];
		break;
	    }
	    if(vbit){
		printf("%s\t%s,%s\n", mnemonic, get_r_m_name(r_m, 1, data16,
		       rex), reg_name);
	    }
	    else{
		printf("%s\t%s,%s\n", mnemonic, reg_name, get_r_m_name(r_m, 1,
		       data16, rex));
	    }
	    return(length);

	/* single register operand with register in the low 3	*/
	/* bits of op code					*/
	case R:
	    reg = REGNO(opcode2);
	    reg_name = get_r_m_name(reg, LONGOPERAND, data16, rex);
	    printf("%s\t%s\n", mnemonic, reg_name);
	    return(length);

	/* register to accumulator with register in the low 3	*/
	/* bits of op code, xchg instructions                   */
	case RA:
	    reg = REGNO(opcode2);
	    if(rex)
		reg_name = REG32[reg + (REX_B(rex) << 3)]
				[LONGOPERAND + REX_W(rex)];
	    else
		reg_name = get_reg_name(reg, LONGOPERAND, data16, rex);
	    if(rex)
		printf("%s\t%s,%s\n", mnemonic, reg_name, "%rax");
	    else
		printf("%s\t%s,%s\n", mnemonic, reg_name, (data16 ?
							  "%ax" : "%eax"));
	    return(length);

	/* single segment register operand, with reg in bits 3-4 of op code */
	case SEG:
	    reg = byte >> 3 & 0x3; /* segment register */
	    printf("%s\t%s\n", mnemonic, SEGREG[reg]);
	    return(length);

	/* single segment register operand, with register in	*/
	/* bits 3-5 of op code					*/
	case LSEG:
	    reg = byte >> 3 & 0x7; /* long seg reg from opcode */
	    printf("%s\t%s\n", mnemonic, SEGREG[reg]);
	    return(length);

	/* memory or register operand to register */
	case MR:
	    /*
	     * invvpid and invept outside 64-bit mode the register operand is
	     * always 32 bits, since this is encoded with 0x66 (operand-size
	     * override) it would have set data16. So clear that to get the
	     * correct value from reg_name().
	     */
	    if((opcode1 == 0x0 && opcode2 == 0xf &&
	        opcode4 == 0x3 && opcode5 == 0x8 && prefix_byte == 0x66) &&
		(byte == 0x81 || byte == 0x80) &&
		(cputype & CPU_ARCH_ABI64) != CPU_ARCH_ABI64)
		data16 = FALSE;
	    if(got_modrm_byte == FALSE){
		got_modrm_byte = TRUE;
		byte = get_value(sizeof(char), sect, &length, &left);
		modrm_byte(&mode, &reg, &r_m, byte);
	    }
	    wbit = LONGOPERAND;
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    reg_name = get_reg_name(reg, wbit, data16, rex);
	    printf("%s\t", mnemonic);
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  ",");
	    printf("%s\n", reg_name);
	    return(length);

	/* immediate operand to accumulator */
	case IA:
	    value0_size = OPSIZE(data16, WBIT(opcode2), 0);
	    switch(value0_size) {
		case 1: reg_name = "%al"; break;
		case 2: reg_name = "%ax"; break;
		case 4: reg_name = "%eax"; break;
	    }
	    IMMEDIATE(&symadd0, &symsub0, &imm0, value0_size);
	    printf("%s\t$", mnemonic);
	    print_operand("", symadd0, symsub0, imm0, value0_size, "", ",");
	    printf("%s\n", reg_name);
	    return(length);

	/* memory or register operand to accumulator */
	case MA:
	    wbit = WBIT(opcode2);
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    printf("%s\t", mnemonic);
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  "\n");
	    return(length);

	/* si register to di register */
	case SD:
	    if(addr16 == TRUE)
		printf("%s\t%s(%%si),(%%di)\n", mnemonic, seg);
	    else
		printf("%s\t%s(%%esi),(%%edi)\n", mnemonic, seg);
	    return(length);

	/* accumulator to di register */
	case AD:
	    wbit = WBIT(opcode2);
	    reg_name = get_reg_name(0, wbit, data16, rex);
	    if(addr16 == TRUE)
		printf("%s\t%s,%s(%%di)\n", mnemonic, reg_name, seg);
	    else
		printf("%s\t%s,%s(%%edi)\n", mnemonic, reg_name, seg);
	    return(length);

	/* si register to accumulator */
	case SA:
	    wbit = WBIT(opcode2);
	    reg_name = get_reg_name(0, wbit, data16, rex);
	    if(addr16 == TRUE)
		printf("%s\t%s(%%si),%s\n", mnemonic, seg, reg_name);
	    else
		printf("%s\t%s(%%esi),%s\n", mnemonic, seg, reg_name);
	    return(length);

	/* single operand, a 16/32 bit displacement */
	case D:
	    value0_size = OPSIZE(data16, LONGOPERAND, 0);
	    DISPLACEMENT(&symadd0, &symsub0, &imm0, value0_size);
	    printf("%s\t", mnemonic);
	    print_operand(seg, symadd0, symsub0, imm0, value0_size, "", "");
	    if(verbose){
		indirect_symbol_name = guess_indirect_symbol(imm0,
		    ncmds, sizeofcmds, load_commands, object_byte_sex,
		    indirect_symbols, nindirect_symbols, symbols, symbols64,
		    nsymbols, strings,strings_size);
		if(indirect_symbol_name != NULL)
		    printf("\t; symbol stub for: %s", indirect_symbol_name);
	    }
	    printf("\n");
	    return(length);

	/* indirect to memory or register operand */
	case INM:
	    /*
	     * If this is call (near) in a 64-bit object the FF /2 opcode
	     * results in a 64-bit operand even without a rex prefix byte.
	     * So to get the 64-bit register names in the disassembly we
	     * set the REX.W bit to indicate 64-bit operand size.
	     */
	    if((cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64 &&
	       opcode1 == 0xf && opcode2 == 0xf &&
	       (opcode3 == 2 || opcode3 == 4))
		rex |= 0x8;
	    wbit = LONGOPERAND;
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    if((mode == 0 && (r_m == 5 || r_m == 4)) || mode == 1 ||
		mode == 2 || mode == 3)
		printf("%s\t*", mnemonic);
	    else
		printf("%s\t", mnemonic);
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  "\n");
	    return(length);

	/* indirect to memory or register operand (for lcall and ljmp) */
	case INMl:
	    wbit = LONGOPERAND;
	    GET_OPERAND(&symadd0, &symsub0, &value0, &value0_size, result0);
	    printf("%s\t*", mnemonic);
	    print_operand(seg, symadd0, symsub0, value0, value0_size, result0,
			  "\n");
	    return(length);

	/*
	 * For long jumps and long calls -- a new code segment
	 * register and an offset in IP -- stored in object
	 * code in reverse order
	 */
	case SO:
	    value1_size = OPSIZE(data16, LONGOPERAND, 0);
	    IMMEDIATE(&symadd1, &symsub1, &imm1, value1_size);
	    value0_size = sizeof(short);
	    IMMEDIATE(&symadd0, &symsub0, &imm0, value0_size);
	    printf("%s\t$", mnemonic);
	    print_operand("", symadd0, symsub0, imm0, value0_size, "", ",$");
	    print_operand(seg, symadd1, symsub1, imm1, value1_size, "", "\n");
	    return(length);

	/* jmp/call. single operand, 8 bit displacement */
	case BD:
	    /*
	     * The "Jump if rCX Zero" instruction is 0xe3 but is "jcxz" as in
	     * the table only in 32-bit mode with a Address-size override
	     * prefix.  Without a prefix it is "jecxz" in 32-bit mode.  In
	     * 64-bit mode with a prefix it is "jecxz" and without it is
	     * "jrcxz".
	     */
	    if(opcode1 == 0xe && opcode2 == 0x3){
		if((cputype & CPU_ARCH_ABI64) != CPU_ARCH_ABI64){
		   if(addr16 == FALSE)
			sprintf(mnemonic, "jecxz");
		}
		else if ((cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64){
		   if(addr16 == TRUE)
			sprintf(mnemonic, "jecxz");
		   else
			sprintf(mnemonic, "jrcxz");
		}
	    }
	    value0_size = sizeof(char);
	    DISPLACEMENT(&symadd0, &symsub0, &imm0, value0_size);
	    printf("%s\t", mnemonic);
	    print_operand(seg, symadd0, symsub0, imm0, sizeof(int32_t), "",
			  "\n");
	    return(length);

	/* single 32/16 bit immediate operand */
	case I:
	    value0_size = OPSIZE(data16, LONGOPERAND, 0);
	    IMMEDIATE(&symadd0, &symsub0, &imm0, value0_size);
	    printf("%s\t$", mnemonic);
	    print_operand("", symadd0, symsub0, imm0, value0_size, "", "\n");
	    return(length);

	/* single 8 bit immediate operand */
	case Ib:
	    value0_size = sizeof(char);
	    IMMEDIATE(&symadd0, &symsub0, &imm0, value0_size);
	    printf("%s\t$", mnemonic);
	    print_operand("", symadd0, symsub0, imm0, value0_size, "", "\n");
	    return(length);

	case ENTER:
	    value0_size = sizeof(short);
	    IMMEDIATE(&symadd0, &symsub0, &imm0, value0_size);
	    value1_size = sizeof(char);
	    IMMEDIATE(&symadd1, &symsub1, &imm1, value1_size);
	    printf("%s\t$", mnemonic);
	    print_operand("", symadd0, symsub0, imm0, value0_size, "", ",$");
	    print_operand("", symadd1, symsub1, imm1, value1_size, "", "\n");
	    return(length);

	/* 16-bit immediate operand */
	case RET:
	    value0_size = sizeof(short);
	    IMMEDIATE(&symadd0, &symsub0, &imm0, value0_size);
	    printf("%s\t$", mnemonic);
	    print_operand("", symadd0, symsub0, imm0, value0_size, "", "\n");
	    return(length);

	/* single 8 bit port operand */
	case P:
	    value0_size = sizeof(char);
	    IMMEDIATE(&symadd0, &symsub0, &imm0, value0_size);
	    printf("%s\t$", mnemonic);
	    print_operand(seg, symadd0, symsub0, imm0, value0_size, "", "\n");
	    return(length);

	/* single 8 bit (input) port operand				*/
	case Pi:
	    value0_size = sizeof(char);
	    IMMEDIATE(&symadd0, &symsub0, &imm0, value0_size);
	    printf("%s\t$", mnemonic);
	    if(opcode2 == 4)
		print_operand(seg, symadd0, symsub0, imm0, value0_size, "",
			      ",%al\n");
	    else if(data16)
		print_operand(seg, symadd0, symsub0, imm0, value0_size, "",
			      ",%ax\n");
	    else
		print_operand(seg, symadd0, symsub0, imm0, value0_size, "",
			      ",%eax\n");
	    return(length);

	/* single 8 bit (output) port operand				*/
	case Po:
	    value0_size = sizeof(char);
	    IMMEDIATE(&symadd0, &symsub0, &imm0, value0_size);
	    if(opcode2 == 0x6)
		printf("%s\t%%al,$", mnemonic);
	    else if(data16)
		printf("%s\t%%ax,$", mnemonic);
	    else
		printf("%s\t%%eax,$", mnemonic);
	    print_operand(seg, symadd0, symsub0, imm0, value0_size, "", "\n");
	    return(length);

	/* single operand, dx register (variable port instruction) */
	case V:
	    printf("%s\t%s(%%dx)\n", mnemonic, seg);
	    return(length);

	/* single operand, dx register (variable (input) port instruction) */
	case Vi:
	    if(opcode2 == 0xc)
		printf("%s\t%s%%dx,%%al\n", mnemonic, seg);
	    else if(data16)
		printf("%s\t%s%%dx,%%ax\n", mnemonic, seg);
	    else
		printf("%s\t%s%%dx,%%eax\n", mnemonic, seg);
	    return(length);

	/* single operand, dx register (variable (output) port instruction)*/
	case Vo:
	    if(opcode2 == 0xe)
		printf("%s\t%s%%al,%%dx\n", mnemonic, seg);
	    else if(data16)
		printf("%s\t%s%%ax,%%dx\n", mnemonic, seg);
	    else
		printf("%s\t%s%%eax,%%dx\n", mnemonic, seg);
	    return(length);

	/* The int instruction, which has two forms: int 3 (breakpoint) or  */
	/* int n, where n is indicated in the subsequent byte (format Ib).  */
	/* The int 3 instruction (opcode 0xCC), where, although the 3 looks */
	/* like an operand, it is implied by the opcode. It must be converted */
	/* to the correct base and output. */
	case INT3:
	    printf("%s\t$0x3\n", mnemonic);
	    return(length);

	/* just an opcode and an unused byte that must be discarded */
	case U:
	    byte = get_value(sizeof(char), sect, &length, &left);
	    if(opcode1 == 0xd && (opcode2 == 0x5 || opcode2 == 0x4) &&
	       byte != 0xa)
		printf("%s\t$0x%x\n", mnemonic, byte);
	    else
		printf("%s\n", mnemonic);
	    return(length);

	case CBW:
	    if(rex != 0)
		printf("cdqe\n");
	    else if(data16 == TRUE)
		printf("cbtw\n");
	    else
		printf("cwtl\n");
	    return(length);

	case CWD:
	    if(rex != 0)
		printf("cqto\n");
	    else if(data16 == TRUE)
		printf("cwtd\n");
	    else
		printf("cltd\n");
	    return(length);

	/* no disassembly, the mnemonic was all there was so go on */
	case GO_ON:
	    printf("%s\n", mnemonic);
	    return(length);

	/* float reg */
	case F:
	    printf("%s\t%%st(%1.1u)\n", mnemonic, r_m);
	    return(length);

	/* float reg to float reg, with ret bit present */
	case FF:
	    /* return result bit for 287 instructions */
	    if(((opcode2 >> 2) & 0x1) == 0x1 && opcode2 != 0xf)
		printf("%s\t%%st,%%st(%1.1u)\n", mnemonic, r_m);
	    else
		printf("%s\t%%st(%1.1u),%%st\n", mnemonic, r_m);
	    return(length);

	/* an invalid op code */
	case AM:
	case DM:
	case OVERRIDE:
	case PREFIX:
	case UNKNOWN:
	default:
	    printf(".byte 0x%02x", 0xff & sect[0]);
	    for(i = 1; i < length; i++)
		printf(", 0x%02x", 0xff & sect[i]);
	    printf(" #bad opcode\n");
	    return(length);
	} /* end switch */
}

/*
 * get_operand() is used to return the symbolic operand for an operand that is
 * encoded with a mod r/m byte.
 */
static
void
get_operand(
const char **symadd,
const char **symsub,
uint32_t *value,
uint32_t *value_size,
char *result,

const cpu_type_t cputype,
const uint32_t mode,
const uint32_t r_m,
const uint32_t wbit,
const enum bool data16,
const enum bool addr16,
const enum bool sse2,
const enum bool mmx,
const unsigned int rex,

const char *sect,
uint64_t sect_addr,
uint32_t *length,
uint32_t *left,

const uint64_t addr,
const struct relocation_info *sorted_relocs,
const uint32_t nsorted_relocs,
const struct relocation_info *ext_relocs,
const uint32_t next_relocs,
const struct nlist *symbols,
const struct nlist_64 *symbols64,
const uint32_t nsymbols,
const char *strings,
const uint32_t strings_size,

const struct symbol *sorted_symbols,
const uint32_t nsorted_symbols,
const enum bool verbose)
{
    enum bool s_i_b;		/* flag presence of scale-index-byte */
    unsigned char byte;		/* the scale-index-byte */
    uint32_t ss;		/* scale-factor from scale-index-byte */
    uint32_t index; 		/* index register number from scale-index-byte*/
    uint32_t base;  		/* base register number from scale-index-byte */
    uint64_t sect_offset, seg_offset;
    uint64_t offset;

	*symadd = NULL;
	*symsub = NULL;
	*value = 0;
	*result = '\0';
	base = 0;
	index = 0;
	ss = 0;

	/* check for the presence of the s-i-b byte */
	if(r_m == ESP && mode != REG_ONLY &&
	   (((cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64) || addr16 == FALSE)){
	    s_i_b = TRUE;
	    byte = get_value(sizeof(char), sect, length, left);
	    modrm_byte(&ss, &index, &base, byte);
	}
	else
	    s_i_b = FALSE;

	if(addr16 && (cputype & CPU_ARCH_ABI64) != CPU_ARCH_ABI64)
	    *value_size = dispsize16[r_m][mode];
	else
	    *value_size = dispsize32[r_m][mode];

	if(s_i_b == TRUE && mode == 0 && base == EBP)
	    *value_size = sizeof(int32_t);

	if(*value_size != 0){
	    seg_offset = addr + *length;
	    sect_offset = addr + *length - sect_addr;
	    *value = (uint32_t)get_value(*value_size, sect, length, left);
	    GET_SYMBOL(symadd, symsub, &offset, sect_offset, seg_offset,*value);
	    if(*symadd != NULL){
		*value = (uint32_t)offset;
	    }
	    else{
		*symadd = GUESS_SYMBOL(*value);
		if(*symadd != NULL)
		    *value = 0;
	    }
	}

	if(s_i_b == TRUE){
	    if(((cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64) && !addr16){
		/* If the scale factor is 1, don't display it. */
		if(ss == 0){
		    /*
		     * If mode is 0 and base is 5 (regardless of the rex bit)
		     * there is no base register, and if the index is
		     * also 4 then the operand is just a displacement.
		     */
		    if(mode == 0 && base == 5 && index == 4){
			result = "";
		    }
		    else{
			sprintf(result, "(%s%s)", regname64[mode][base +
				(REX_B(rex) << 3)], indexname64[index +
				(REX_X(rex) << 3)]);
		    }
		}
		else{
		    /*
		     * If mode is 0 and base is 5 (regardless of the rex bit)
		     * there is no base register.
		     */
		    if(mode == 0 && base == 5){
			sprintf(result, "(%s,%s)", indexname64[index +
				(REX_X(rex) << 3)], scale_factor[ss]);
		    }
		    else{
			sprintf(result, "(%s%s,%s)", regname64[mode][base +
				(REX_B(rex) << 3)], indexname64[index +
				(REX_X(rex) << 3)], scale_factor[ss]);
		    }
		}
	    }
	    else{
		/* If the scale factor is 1, don't display it. */
		if(ss == 0){
		    /*
		     * If mode is 0 and base is 5 it there is no base register,
		     * and if the index is also 4 then the operand is just a
		     * displacement.
		     */
		    if(mode == 0 && base == 5 && index == 4){
			result = "";
		    }
		    else{
			sprintf(result, "(%s%s)", regname32[mode][base],
				indexname[index]);
		    }
		}
		else{
		    sprintf(result, "(%s%s,%s)", regname32[mode][base],
			    indexname[index], scale_factor[ss]);
		}
	    }
	}
	else{ /* no s-i-b */
	    if(mode == REG_ONLY){
		if(sse2 == TRUE)
		    sprintf(result, "%%xmm%u", xmm_rm(r_m, rex));
		else if(mmx == TRUE)
		    sprintf(result, "%%mm%u", r_m);
		else if (data16 == FALSE || rex != 0)
		    /* The presence of a REX byte overrides 66h. */
		    strcpy(result, REG32[r_m + (REX_B(rex) << 3)][wbit +
			   REX_W(rex)]);
		else
		    strcpy(result, REG16[r_m][wbit]);
	    }
	    else{ /* Modes 00, 01, or 10 */
		if(r_m == EBP && mode == 0){ /* displacement only */
		    if((cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64)
			/*
			 * In 64-bit mode, mod=00 and r/m=101 defines
			 * RIP-relative addressing with a 32-bit displacement.
			 * In 32-bit mode, it's just a 32-bit displacement. See
			 * section 2.2.1.6 ("RIP-Relative Addressing") of Volume
			 * 2A of the Intel IA-32 manual.
			 */
			sprintf(result, "(%%rip)");
		    else
			*result = '\0';
		}
		else {
		    /* Modes 00, 01, or 10, not displacement only, no s-i-b */
		    if(addr16 == TRUE) {
			if((cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64)
			    /*
			     *  In 64-bit mode, the address size prefix drops us
			     * down to 32-bit, not 16-bit.
			     */
			    sprintf(result, "(%s)", regname32[mode][r_m]);
			else
			    sprintf(result, "(%s)", regname16[mode][r_m]);
		    }
		    else{
			if((cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64)
			    sprintf(result, "(%s)", regname64[mode][r_m +
				    (REX_B(rex) << 3)]);
			else
			    sprintf(result, "(%s)", regname32[mode][r_m]);
		    }
		}
	    }
	}
}

/*
 * immediate() is used to return the symbolic operand for an immediate operand.
 */
static
void
immediate(
const char **symadd,
const char **symsub,
uint64_t *value,
uint32_t value_size,

const char *sect,
uint64_t sect_addr,
uint32_t *length,
uint32_t *left,

const cpu_type_t cputype,
const uint64_t addr,
const struct relocation_info *sorted_relocs,
const uint32_t nsorted_relocs,
const struct relocation_info *ext_relocs,
const uint32_t next_relocs,
const struct nlist *symbols,
const struct nlist_64 *symbols64,
const uint32_t nsymbols,
const char *strings,
const uint32_t strings_size,

const struct symbol *sorted_symbols,
const uint32_t nsorted_symbols,
const enum bool verbose)
{
    uint64_t sect_offset, seg_offset;
	uint64_t offset;

	seg_offset = addr + *length;
	sect_offset = addr + *length - sect_addr;
	*value = get_value(value_size, sect, length, left);
	GET_SYMBOL(symadd, symsub, &offset, sect_offset, seg_offset, *value);
	if(*symadd == NULL){
	    *symadd = GUESS_SYMBOL(*value);
	    if(*symadd != NULL)
		*value = 0;
	}
	else if(*symsub != NULL){
	    *value = offset;
	}
}

/*
 * displacement() is used to return the symbolic operand for an operand that is
 * encoded as a displacement from the program counter.
 */
static
void
displacement(
const char **symadd,
const char **symsub,
uint64_t *value,
const uint32_t value_size,

const char *sect,
uint64_t sect_addr,
uint32_t *length,
uint32_t *left,

const cpu_type_t cputype,
const uint64_t addr,
const struct relocation_info *sorted_relocs,
const uint32_t nsorted_relocs,
const struct relocation_info *ext_relocs,
const uint32_t next_relocs,
const struct nlist *symbols,
const struct nlist_64 *symbols64,
const uint32_t nsymbols,
const char *strings,
const uint32_t strings_size,

const struct symbol *sorted_symbols,
const uint32_t nsorted_symbols,
const enum bool verbose)
{
    uint64_t sect_offset, seg_offset;
    uint64_t offset;
    uint64_t guess_addr;

	seg_offset = addr + *length;
	sect_offset = addr + *length - sect_addr;
	*value = get_value(value_size, sect, length, left);
	switch(value_size){
	case 1:
	    if((*value) & 0x80)
		*value = *value | 0xffffffffffffff00ULL;
	    break;
	case 2:
	    if((*value) & 0x8000)
		*value = *value | 0xffffffffffff0000ULL;
	    break;
	case 4:
	    if((*value) & 0x80000000)
		*value = *value | 0xffffffff00000000ULL;
	    break;
	}
	if((cputype & CPU_ARCH_ABI64) != CPU_ARCH_ABI64)
	    *value += addr + *length;

	GET_SYMBOL(symadd, symsub, &offset, sect_offset, seg_offset, *value);
	if(*symadd == NULL){
	    if((cputype & CPU_ARCH_ABI64) != CPU_ARCH_ABI64){
		*symadd = GUESS_SYMBOL(*value);
		if(*symadd != NULL)
		    *value = 0;
	    }
	    else{
		guess_addr = *value;
		if((*value) & 0x80000000)
		    guess_addr |= 0xffffffff00000000ULL;
		guess_addr += addr + *length;
		*symadd = GUESS_SYMBOL(guess_addr);
		if(*symadd != NULL)
		    *value = 0;
		else
		    *value += addr + *length;
	    }
	}
	else if(*symsub != NULL){
	    *value = offset;
	}
	if((cputype & CPU_ARCH_ABI64) != CPU_ARCH_ABI64)
	    *value = *value & 0x00000000ffffffffULL;
}

/*
 * get_symbol() returns the name of a symbol (or NULL) based on the relocation
 * information at the specified address.
 */
static
void
get_symbol(
const char **symadd,
const char **symsub,
uint64_t *offset,

const cpu_type_t cputype,
const uint64_t sect_offset,
const uint64_t seg_offset,
const uint64_t value,
const struct relocation_info *relocs,
const uint32_t nrelocs,
const struct relocation_info *ext_relocs,
const uint32_t next_relocs,
const struct nlist *symbols,
const struct nlist_64 *symbols64,
const uint32_t nsymbols,
const char *strings,
const uint32_t strings_size,
const struct symbol *sorted_symbols,
const uint32_t nsorted_symbols,
const enum bool verbose)
{
    uint32_t i;
    unsigned int r_symbolnum;
    uint32_t n_strx;
    struct scattered_relocation_info *sreloc, *pair;
    const char *name, *add, *sub;

    static char add_buffer[11]; /* max is "0x1234678\0" */
    static char sub_buffer[11];

	*symadd = NULL;
	*symsub = NULL;
	*offset = value;

	if(verbose == FALSE)
	    return;

	for(i = 0; i < nrelocs; i++){
	    if((cputype & CPU_ARCH_ABI64) != CPU_ARCH_ABI64 &&
	       ((relocs[i].r_address) & R_SCATTERED) != 0){
		sreloc = (struct scattered_relocation_info *)(relocs + i);
		if(sreloc->r_type == GENERIC_RELOC_PAIR){
		    fprintf(stderr, "Stray GENERIC_RELOC_PAIR relocation entry "
			    "%u\n", i);
		    continue;
		}
		if(sreloc->r_type == GENERIC_RELOC_VANILLA){
		    if(sreloc->r_address == sect_offset){
			name = guess_symbol(sreloc->r_value,
					    sorted_symbols,
					    nsorted_symbols,
					    verbose);
			if(name != NULL){
			    *symadd = name;
			    *offset = value - sreloc->r_value;
			    return;
			}
		    }
		    continue;
		}
		if(sreloc->r_type != GENERIC_RELOC_SECTDIFF &&
		   sreloc->r_type != GENERIC_RELOC_LOCAL_SECTDIFF){
		    fprintf(stderr, "Unknown relocation r_type for entry "
			    "%u\n", i);
		    continue;
		}
		if(i + 1 < nrelocs){
		    pair = (struct scattered_relocation_info *)(relocs + i + 1);
		    if(pair->r_scattered == 0 ||
		       pair->r_type != GENERIC_RELOC_PAIR){
			fprintf(stderr, "No GENERIC_RELOC_PAIR relocation "
				"entry after entry %u\n", i);
			continue;
		    }
		}
		else{
		    fprintf(stderr, "No GENERIC_RELOC_PAIR relocation entry "
			    "after entry %u\n", i);
		    continue;
		}
		i++; /* skip the pair reloc */

		if(sreloc->r_address == sect_offset){
		    add = guess_symbol(sreloc->r_value, sorted_symbols,
				       nsorted_symbols, verbose);
		    sub = guess_symbol(pair->r_value, sorted_symbols,
				       nsorted_symbols, verbose);
		    if(add == NULL){
			sprintf(add_buffer, "0x%x",
				(unsigned int)sreloc->r_value);
			add = add_buffer;
		    }
		    if(sub == NULL){
			sprintf(sub_buffer, "0x%x",
				(unsigned int)pair->r_value);
			sub = sub_buffer;
		    }
		    *symadd = add;
		    *symsub = sub;
		    *offset = value - (sreloc->r_value - pair->r_value);
		    return;
		}
	    }
	    else{
		if((uint32_t)relocs[i].r_address == sect_offset){
		    r_symbolnum = relocs[i].r_symbolnum;
		    if(relocs[i].r_extern){
		        if(r_symbolnum >= nsymbols)
			    return;
			if(symbols != NULL)
			    n_strx = symbols[r_symbolnum].n_un.n_strx;
			else
			    n_strx = symbols64[r_symbolnum].n_un.n_strx;
			if(n_strx <= 0 || n_strx >= strings_size)
			    return;
			if((cputype & CPU_ARCH_ABI64) == CPU_ARCH_ABI64 &&
			   relocs[i].r_type == X86_64_RELOC_SUBTRACTOR &&
			   i+1 < nrelocs &&
			   relocs[i+1].r_type == X86_64_RELOC_UNSIGNED &&
			   relocs[i+1].r_extern == 1 &&
			   relocs[i+1].r_symbolnum < nsymbols){
			    *symsub = strings + n_strx;
			    r_symbolnum = relocs[i+1].r_symbolnum;
			    if(symbols64 == NULL)
				return;
			    n_strx = symbols64[r_symbolnum].n_un.n_strx;
			    if(n_strx <= 0 || n_strx >= strings_size)
				return;
			}
			*symadd = strings + n_strx;
			return;
		    }
		    break;
		}
	    }
	}

	for(i = 0; i < next_relocs; i++){
	    if((uint32_t)ext_relocs[i].r_address == seg_offset){
		r_symbolnum = ext_relocs[i].r_symbolnum;
		if(ext_relocs[i].r_extern){
		    if(r_symbolnum >= nsymbols)
			return;
		    if(symbols != NULL)
			n_strx = symbols[r_symbolnum].n_un.n_strx;
		    else
			n_strx = symbols64[r_symbolnum].n_un.n_strx;
		    if(n_strx <= 0 || n_strx >= strings_size)
			return;
		    *symadd = strings + n_strx;
		    return;
		}
		break;
	    }
	}
}

/*
 * print_operand() prints an operand from it's broken out symbolic
 * representation.
 */
static
void
print_operand(
const char *seg,
const char *symadd,
const char *symsub,
uint64_t value,
unsigned int value_size,
const char *result,
const char *tail)
{
	if(symadd != NULL){
	    if(symsub != NULL){
		if(value_size != 0){
		    if(value != 0)
			printf("%s%s-%s+0x%0*llx%s%s", seg, symadd, symsub,
			       (int)value_size * 2, value, result, tail);
		    else
			printf("%s%s-%s%s%s",seg, symadd, symsub, result, tail);
		}
		else{
		    printf("%s%s%s%s", seg, symadd, result, tail);
		}
	    }
	    else{
		if(value_size != 0){
		    if(value != 0)
			printf("%s%s+0x%0*llx%s%s", seg, symadd,
			       (int)value_size * 2, value, result, tail);
		    else
			printf("%s%s%s%s", seg, symadd, result, tail);
		}
		else{
		    printf("%s%s%s%s", seg, symadd, result, tail);
		}
	    }
	}
	else{
	    if(value_size != 0){
		printf("%s0x%0*llx%s%s", seg, (int)value_size *2, value, result,
		       tail);
	    }
	    else{
		printf("%s%s%s", seg, result, tail);
	    }
	}
}

/*
 * get_value() gets a value of size from sect + length and decrease left by the
 * size and increase length by size.  The size of the value can be 1, 2, 4, or 8
 * bytes and the value is in little endian byte order.  The value is always
 * returned as a uint64_t and is not sign extended.
 */
static
uint64_t
get_value(
const uint32_t size,	/* size of the value to get as a number of bytes (in)*/
const char *sect,	/* pointer to the raw data of the section (in) */
uint32_t *length,	/* number of bytes taken from the sect (in/out) */
uint32_t *left)		/* number of bytes left in sect after length (in/out) */
{
    uint32_t i;
    uint64_t value;
    unsigned char byte;

	if(left == 0)
	    return(0);

	value = 0;
	for(i = 0; i < size; i++) {
	    byte = 0;
	    if(*left > 0){
		byte = sect[*length];
		(*length)++;
		(*left)--;
	    }
	    value |= (uint64_t)byte << (8*i);
	}
	return(value);
}

/*
 * modrm_byte() breaks a byte out into its mode, reg and r/m bits.
 */
static
void
modrm_byte(
uint32_t *mode,
uint32_t *reg,
uint32_t *r_m,
unsigned char byte)
{
	*r_m = byte & 0x7; /* r/m field from the byte */
	*reg = byte >> 3 & 0x7; /* register field from the byte */
	*mode = byte >> 6 & 0x3; /* mode field from the byte */
}

/*
 * i386GetOpInfo() is the operand information call back function for i386 code.
 * This is called to get the symbolic information for an operand of an i386
 * instruction.  This is done from the relocation information, symbol table,
 * etc.  That block of information is a pointer to the struct disassemble_info
 * that was passed when the disassembler context was created and passed to back
 * to i386GetOpInfo() when called back by LLVMDisasmInstruction().  The address
 * of the instruction containing operand is at the Pc parameter.  The immediate
 * for the operand is at Offset past the start of the instruction and has a byte
 * Width of 1, 2 or 4.  The information is returned in TagBuf is the
 * LLVMOpInfo1 struct defined in "llvm-c/Disassembler.h".  The value of TagType
 * is currently 1 (for the LLVMOpInfo1 struct). If symbolic information is
 * returned then this function returns 1 else it returns 0.
 */
static
int
i386GetOpInfo(
void *DisInfo,
uint64_t Pc,
uint64_t Offset,
uint64_t Width,
int TagType,     /* should currently always be passed as 1 */
void *TagBuf)
{
    struct disassemble_info *info;
    struct LLVMOpInfo1 *op_info;
    uint64_t value;
    uint64_t reloc_found, offset;
    uint64_t sect_offset, i, r_address, r_symbolnum, r_type, r_extern, r_length,
	     r_value, r_scattered, pair_r_type, pair_r_value, seg_offset;
    uint32_t other_half;
    const char *strings, *name, *add, *sub;
    struct relocation_info *relocs, *rp, *pairp;
    struct scattered_relocation_info *srp, *spairp;
    uint32_t nrelocs, strings_size, n_strx;
    struct nlist *symbols;

	info = (struct disassemble_info *)DisInfo;

	op_info = (struct LLVMOpInfo1 *)TagBuf;
	value = op_info->Value;
	/* make sure all feilds returned are zero if we don't set them */
	memset(op_info, '\0', sizeof(struct LLVMOpInfo1));
	op_info->Value = value;

	if((Width != 1 && Width != 2 && Width != 4) || TagType != 1 ||
	   info->verbose == FALSE)
	    return(0);

	seg_offset = (Pc + Offset);
	sect_offset = (Pc + Offset) - info->sect_addr;
	relocs = info->sorted_relocs;
	nrelocs = info->nsorted_relocs;
	symbols = info->symbols;
	strings = info->strings;
	strings_size = info->strings_size;

	r_symbolnum = 0;
	r_type = 0;
	r_extern = 0;
	r_value = 0;
	r_scattered = 0;
	other_half = 0;
	pair_r_value = 0;
	n_strx = 0;
	r_length = 0;

	/*
	 * When searching for a relocation entry for this sect_offset we simply
	 * return if we run into errors.
	 */
	reloc_found = 0;
	for(i = 0; i < nrelocs; i++){
	    rp = &relocs[i];
	    if(rp->r_address & R_SCATTERED){
		srp = (struct scattered_relocation_info *)rp;
		r_scattered = 1;
		r_address = srp->r_address;
		r_extern = 0;
		r_length = srp->r_length;
		r_type = srp->r_type;
		r_value = srp->r_value;
	    }
	    else{
		r_scattered = 0;
		r_address = rp->r_address;
		r_symbolnum = rp->r_symbolnum;
		r_extern = rp->r_extern;
		r_length = rp->r_length;
		r_type = rp->r_type;
	    }
	    if(r_type == GENERIC_RELOC_PAIR){
		/* Error stray GENERIC_RELOC_PAIR relocation entry. */
		return(0);
	    }
	    if(r_address == sect_offset){
		if(r_type == GENERIC_RELOC_SECTDIFF ||
		   r_type == GENERIC_RELOC_LOCAL_SECTDIFF){
		    if(i+1 < nrelocs){
			pairp = &rp[1];
			if(pairp->r_address & R_SCATTERED){
			    spairp = (struct scattered_relocation_info *)
				     pairp;
			    pair_r_type = spairp->r_type;
			    pair_r_value = spairp->r_value;
			}
			else{
			    pair_r_type = pairp->r_type;
			}
			if(pair_r_type != GENERIC_RELOC_PAIR){
			    /* Error missing GENERIC_RELOC_PAIR relocation. */
			    return(0);
			}
		    }
		}
		reloc_found = 1;
		break;
	    }
	    if(r_type == GENERIC_RELOC_SECTDIFF ||
	       r_type == GENERIC_RELOC_LOCAL_SECTDIFF){
		if(i+1 < nrelocs){
		    pairp = &rp[1];
		    if(pairp->r_address & R_SCATTERED){
			spairp = (struct scattered_relocation_info *)pairp;
			pair_r_type = spairp->r_type;
		    }
		    else{
			pair_r_type = pairp->r_type;
		    }
		    if(pair_r_type == GENERIC_RELOC_PAIR)
			i++;
		    else{
			/* Error missing GENERIC_RELOC_PAIR relocation. */
			return(0);
		    }
		}
	    }
	}

	if(reloc_found && r_extern == 1){
	    if(symbols != NULL)
		n_strx = symbols[r_symbolnum].n_un.n_strx;
	    if(n_strx >= strings_size){
		/* Error bad string offset. */
		return(0);
	    }
	    name = strings + n_strx;
	    op_info->AddSymbol.Present = 1;
	    op_info->AddSymbol.Name = name;
	    /*
	     * For i386 extern relocation entries the value in the instrucion is
	     * the offset from the symbol.
	     */
	    op_info->Value = value;
	    return(1);
	}

	if(reloc_found && 
	   (r_type == GENERIC_RELOC_SECTDIFF ||
	    r_type == GENERIC_RELOC_LOCAL_SECTDIFF)){
	    add = guess_symbol(r_value, info->sorted_symbols,
			       info->nsorted_symbols, info->verbose);
	    sub = guess_symbol(pair_r_value, info->sorted_symbols,
			       info->nsorted_symbols, info->verbose);
	    offset = value - (r_value - pair_r_value);
	    op_info->AddSymbol.Present = 1;
	    if(add != NULL)
		op_info->AddSymbol.Name = add;
	    else
		op_info->AddSymbol.Value = r_value;
	    op_info->SubtractSymbol.Present = 1;
	    if(sub != NULL)
		op_info->SubtractSymbol.Name = sub;
	    else
		op_info->SubtractSymbol.Value = pair_r_value;
	    op_info->Value = offset;
	    return(1);
	}

	relocs = info->ext_relocs;
	nrelocs = info->next_relocs;
	for(i = 0; i < nrelocs; i++){
	    if(relocs[i].r_address == seg_offset){
		if(symbols != NULL)
		    n_strx = symbols[relocs[i].r_symbolnum].n_un.n_strx;
		if(n_strx >= strings_size){
		    /* Error bad string offset. */
		    return(0);
		}
		name = strings + n_strx;
		op_info->AddSymbol.Present = 1;
		op_info->AddSymbol.Name = name;
		/*
		 * For i386 extern relocation entries the value in the
		 * instrucion is the offset from the symbol.
		 */
		op_info->Value = value;
		return(1);
	    }
	}

	/* We found no symbolic info so just return zero indicating that. */
	return(0);
}

/*
 * x86_64GetOpInfo() is the operand information call back function for x86_64
 * code.  This is called to get the symbolic information for an operand of an
 * x86_64 instruction.  This is done from the relocation information, symbol
 * table, etc.  That block of information is a pointer to the struct
 * disassemble_info that was passed when the disassembler context was created
 * and passed to back to x86_64GetOpInfo() when called back by
 * LLVMDisasmInstruction().  The address of the instruction containing operand
 * is at the Pc parameter.  The immediate for the operand is at Offset past the
 * start of the instruction and has a byte Width of 1, 2 or 4.  The information
 * is returned in TagBuf is the LLVMOpInfo1 struct defined in
 * "llvm-c/Disassembler.h".  The value of TagType is currently 1 (for the
 * LLVMOpInfo1 struct). If symbolic information is returned then this function
 * returns 1 else it returns 0.
 */
static
int
x86_64GetOpInfo(
void *DisInfo,
uint64_t Pc,
uint64_t Offset,
uint64_t Width,
int TagType,     /* should currently always be passed as 1 */
void *TagBuf)
{
    struct disassemble_info *info;
    struct LLVMOpInfo1 *op_info;
    uint64_t value;
    int32_t reloc_found;
    uint64_t sect_offset, seg_offset, i;
    const char *strings, *name;
    struct relocation_info *relocs;
    uint32_t nrelocs, strings_size, n_strx;
    struct nlist_64 *symbols;

	info = (struct disassemble_info *)DisInfo;

	op_info = (struct LLVMOpInfo1 *)TagBuf;
	value = op_info->Value;
	/* make sure all fields returned are zero if we don't set them */
	memset(op_info, '\0', sizeof(struct LLVMOpInfo1));
	op_info->Value = value;

	if((Width != 1 && Width != 2 && Width != 4 && Width != 0) ||
	   TagType != 1 ||
	   info->verbose == FALSE)
	    return(0);

	seg_offset = (Pc + Offset);
	sect_offset = (Pc + Offset) - info->sect_addr;
	relocs = info->sorted_relocs;
	nrelocs = info->nsorted_relocs;
	symbols = info->symbols64;
	strings = info->strings;
	strings_size = info->strings_size;

	reloc_found = 0;
	for(i = 0; i < nrelocs; i++){
	    /* We could also check the Width matches the r_length. */
	    if(relocs[i].r_address == sect_offset){
		reloc_found = 1;
		break;
	    }
	}

	if(reloc_found && relocs[i].r_extern == 1){
	    /*
	     * The Value passed in will be adjusted by the Pc if the instruction
	     * adds the Pc.  But for x86_64 external relocation entries the
	     * Value is the offset from the external symbol.
	     */
	    if(relocs[i].r_pcrel == 1)
		op_info->Value -= Pc + Offset + Width;
	    if(symbols != NULL && relocs[i].r_symbolnum < info->nsymbols)
		n_strx = symbols[relocs[i].r_symbolnum].n_un.n_strx;
	    else
		return(0);
	    if(n_strx >= strings_size)
		return(0);
	    name = strings + n_strx;
	    if(relocs[i].r_type == X86_64_RELOC_SUBTRACTOR &&
	       i+1 < nrelocs &&
	       relocs[i+1].r_type == X86_64_RELOC_UNSIGNED &&
	       relocs[i+1].r_extern == 1){
		op_info->SubtractSymbol.Present = 1;
		op_info->SubtractSymbol.Name = name;
		if(symbols != NULL)
		    n_strx = symbols[relocs[i+1].r_symbolnum].n_un.n_strx;
		if(n_strx >= strings_size)
		    return(0);
		name = strings + n_strx;
	    }
	    op_info->AddSymbol.Present = 1;
	    op_info->AddSymbol.Name = name;
	    return(1);
	}

	relocs = info->ext_relocs;
	nrelocs = info->next_relocs;
	for(i = 0; i < nrelocs; i++){
	    if(relocs[i].r_address == seg_offset){
		/*
		 * The Value passed in will be adjusted by the Pc if the
		 * instruction adds the Pc.  But for x86_64 external relocation
		 * entries the Value is the offset from the external symbol.
		 */
		if(relocs[i].r_pcrel == 1)
		    op_info->Value -= Pc + Offset + Width;
		if(symbols != NULL)
		    n_strx = symbols[relocs[i].r_symbolnum].n_un.n_strx;
		else
		    return(0);
		if(n_strx >= strings_size)
		    return(0);
		name = strings + n_strx;
		op_info->AddSymbol.Present = 1;
		op_info->AddSymbol.Name = name;
		return(1);
	    }
	}

	/* We found no symbolic info so just return zero indicating that. */
	return(0);
}

/*
 * guess_pointer_pointer() is passed the address of what might be a pointer to
 * a reference to an Objective-C class, selector, message ref or cfstring.
 */
static
uint64_t
guess_pointer_pointer(
const uint64_t value,
const uint32_t ncmds,
const uint32_t sizeofcmds,
const struct load_command *load_commands,
const enum byte_sex load_commands_byte_sex,
const char *object_addr,
const uint64_t object_size,
enum bool *classref,
enum bool *selref,
enum bool *msgref,
enum bool *cfstring)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    uint32_t i, j, section_type;
    uint64_t sect_offset, object_offset, pointer_value;
    const struct load_command *lc;
    struct load_command l;
    struct segment_command_64 sg64;
    struct section_64 s64;
    char *p;
    uint64_t big_load_end;

	*classref = FALSE;
	*selref = FALSE;
	*msgref = FALSE;
	*cfstring = FALSE;
	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != load_commands_byte_sex;

	lc = load_commands;
	big_load_end = 0;
	for(i = 0 ; i < ncmds; i++){
	    memcpy((char *)&l, (char *)lc, sizeof(struct load_command));
	    if(swapped)
		swap_load_command(&l, host_byte_sex);
	    if(l.cmdsize % sizeof(int32_t) != 0)
		return(0);
	    big_load_end += l.cmdsize;
	    if(big_load_end > sizeofcmds)
		return(0);
	    switch(l.cmd){
	    case LC_SEGMENT_64:
		memcpy((char *)&sg64, (char *)lc,
		       sizeof(struct segment_command_64));
		if(swapped)
		    swap_segment_command_64(&sg64, host_byte_sex);
		p = (char *)lc + sizeof(struct segment_command_64);
		for(j = 0 ; j < sg64.nsects &&
			    j * sizeof(struct section_64) +
			    sizeof(struct segment_command_64) < sizeofcmds ;
                    j++){
		    memcpy((char *)&s64, p, sizeof(struct section_64));
		    p += sizeof(struct section_64);
		    if(swapped)
			swap_section_64(&s64, 1, host_byte_sex);
		    section_type = s64.flags & SECTION_TYPE;
		    if((strncmp(s64.sectname, "__objc_selrefs", 16) == 0 ||
		        strncmp(s64.sectname, "__objc_classrefs", 16) == 0 ||
		        strncmp(s64.sectname, "__objc_superrefs", 16) == 0 ||
			strncmp(s64.sectname, "__objc_msgrefs", 16) == 0 ||
			strncmp(s64.sectname, "__cfstring", 16) == 0) &&
		       value >= s64.addr && value < s64.addr + s64.size){
			sect_offset = value - s64.addr;
			object_offset = s64.offset + sect_offset;
			if(object_offset < object_size){
			    memcpy(&pointer_value, object_addr + object_offset,
				   sizeof(uint64_t));
			    if(swapped)
				pointer_value = SWAP_LONG_LONG(pointer_value);
			    if(strncmp(s64.sectname,
				       "__objc_selrefs", 16) == 0)
				*selref = TRUE; 
			    else if(strncmp(s64.sectname,
				       "__objc_classrefs", 16) == 0 ||
			            strncmp(s64.sectname,
				       "__objc_superrefs", 16) == 0)
				*classref = TRUE; 
			    else if(strncmp(s64.sectname,
				       "__objc_msgrefs", 16) == 0 &&
			     value + 8 < s64.addr + s64.size){
				*msgref = TRUE; 
				memcpy(&pointer_value,
				       object_addr + object_offset + 8,
				       sizeof(uint64_t));
				if(swapped)
				    pointer_value =
					SWAP_LONG_LONG(pointer_value);
			    }
			    else if(strncmp(s64.sectname,
				       "__cfstring", 16) == 0)
				*cfstring = TRUE; 
			    return(pointer_value);
			}
			else
			    return(0);
		    }
		}
		break;
	    }
	    if(l.cmdsize == 0){
		return(0);
	    }
	    lc = (struct load_command *)((char *)lc + l.cmdsize);
	    if((char *)lc > (char *)load_commands + sizeofcmds)
		return(0);
	}
	return(0);
}

/*
 * guess_cstring_pointer() is passed the address of what might be a pointer to a
 * literal string in a cstring section.  If that address is in a cstring section
 * it returns a pointer to that string.  Else it returns NULL.
 */
static
const char *
guess_cstring_pointer(
const uint64_t value,
const uint32_t ncmds,
const uint32_t sizeofcmds,
const struct load_command *load_commands,
const enum byte_sex load_commands_byte_sex,
const char *object_addr,
const uint64_t object_size)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    uint32_t i, j, section_type;
    uint64_t sect_offset, object_offset;
    const struct load_command *lc;
    struct load_command l;
    struct segment_command_64 sg64;
    struct section_64 s64;
    char *p;
    uint64_t big_load_end;
    const char *name;

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != load_commands_byte_sex;

	lc = load_commands;
	big_load_end = 0;
	for(i = 0 ; i < ncmds; i++){
	    memcpy((char *)&l, (char *)lc, sizeof(struct load_command));
	    if(swapped)
		swap_load_command(&l, host_byte_sex);
	    if(l.cmdsize % sizeof(int32_t) != 0)
		return(NULL);
	    big_load_end += l.cmdsize;
	    if(big_load_end > sizeofcmds)
		return(NULL);
	    switch(l.cmd){
	    case LC_SEGMENT_64:
		memcpy((char *)&sg64, (char *)lc,
		       sizeof(struct segment_command_64));
		if(swapped)
		    swap_segment_command_64(&sg64, host_byte_sex);
		p = (char *)lc + sizeof(struct segment_command_64);
		for(j = 0 ; j < sg64.nsects &&
			    j * sizeof(struct section_64) +
			    sizeof(struct segment_command_64) < sizeofcmds ;
                    j++){
		    memcpy((char *)&s64, p, sizeof(struct section_64));
		    p += sizeof(struct section_64);
		    if(swapped)
			swap_section_64(&s64, 1, host_byte_sex);
		    section_type = s64.flags & SECTION_TYPE;
		    if(section_type == S_CSTRING_LITERALS &&
		       value >= s64.addr && value < s64.addr + s64.size){
			sect_offset = value - s64.addr;
			object_offset = s64.offset + sect_offset;
			if(object_offset < object_size){
			    name = object_addr + object_offset;
			    return(name);
			}
			else
			    return(NULL);
		    }
		}
		break;
	    }
	    if(l.cmdsize == 0){
		return(NULL);
	    }
	    lc = (struct load_command *)((char *)lc + l.cmdsize);
	    if((char *)lc > (char *)load_commands + sizeofcmds)
		return(NULL);
	}
	return(NULL);
}

/*
 * guess_literal_pointer() returns a pointer to a literal string if the value
 * passed in is the address of a literal pointer and the literal pointer's value
 * is and address of a cstring. Or returns a reference name for an Objective-C
 * item, or non-lazy pointer.  And sets the value of *reference_type to the
 * type of reference.
 */
static
const char *
guess_literal_pointer(
uint64_t value,	  	  /* the value of the reference */
const uint64_t pc,	  /* pc of the referencing instruction */
uint64_t *reference_type, /* type returned, symbol name or string literal */
struct disassemble_info *info)
{
    uint32_t reloc_found, i, nrelocs, ncmds, sizeofcmds;
    uint64_t sect_offset;
    struct relocation_info *relocs;
    struct nlist_64 *symbols;
    struct load_command *load_commands;
    enum byte_sex object_byte_sex;
    char *object_addr;
    uint64_t object_size, pointer_value;
    const char *name, *class_name;
    enum bool classref, selref, msgref, cfstring;

	/*
	 * First see if there is a relocation entry.
	 */
	sect_offset = pc - info->sect_addr;
	relocs = info->sorted_relocs;
	nrelocs = info->nsorted_relocs;
	symbols = info->symbols64;

	reloc_found = 0;
	for(i = 0; i < nrelocs; i++){
	    if(relocs[i].r_address == sect_offset){
		reloc_found = 1;
		break;
	    }
	}

	/*
	 * If there is an external relocation entry for a symbol in a section
	 * then used that symbol's value for the value of the reference.
	 */
	if(reloc_found && relocs[i].r_extern == 1){
	    if(relocs[i].r_pcrel == 1 &&
	       relocs[i].r_type == X86_64_RELOC_SIGNED &&
	       relocs[i].r_symbolnum < info->nsymbols &&
	       symbols != NULL &&
	       (symbols[relocs[i].r_symbolnum].n_type & N_TYPE) == N_SECT){
		value = symbols[relocs[i].r_symbolnum].n_value;
	    }
	}

	ncmds = info->ncmds;
	sizeofcmds = info->sizeofcmds;
	load_commands = info->load_commands;
	object_byte_sex = info->object_byte_sex;
	object_addr = info->object_addr;
	object_size = info->object_size;

	pointer_value = guess_pointer_pointer(value, ncmds, sizeofcmds,
			    load_commands, object_byte_sex, object_addr,
			    object_size, &classref, &selref, &msgref,
			    &cfstring);

	if(classref == TRUE && pointer_value == 0){
	    /*
	     * Note the value is a pointer into the __objc_classrefs section.
	     * And the pointer_value in that section is typically zero as it
	     * will be set by dyld as part of the "bind information".
	     */
	    name = get_dyld_bind_info_symbolname(value, info->dbi, info->ndbi,
						 CHAIN_FORMAT_NONE, NULL);
	    if(name != NULL){
		*reference_type =
		    LLVMDisassembler_ReferenceType_Out_Objc_Class_Ref;
		class_name = rindex(name, '$');
		if(class_name != NULL &&
		   class_name[1] == '_' && class_name[2] != '\0')
		    info->class_name = class_name + 2;
		return(name);
	    }
	}

	if(classref == TRUE){
	    *reference_type =
		LLVMDisassembler_ReferenceType_Out_Objc_Class_Ref;
	    name = get_objc2_64bit_class_name(pointer_value, value,
			info->load_commands, info->ncmds, info->sizeofcmds,
			info->object_byte_sex, info->object_addr,
			info->object_size, info->symbols64, info->nsymbols,
			info->strings, info->strings_size, info->cputype);
	    if(name != NULL)
		info->class_name = name;
	    else
	        name = "bad class ref";
	    return(name);
	}

	if(cfstring == TRUE){
	    *reference_type =
		LLVMDisassembler_ReferenceType_Out_Objc_CFString_Ref;
	    name = get_objc2_64bit_cfstring_name(value,
			info->load_commands, info->ncmds, info->sizeofcmds,
			info->object_byte_sex, info->object_addr,
			info->object_size, info->symbols64, info->nsymbols,
			info->strings, info->strings_size, info->cputype);
	    if(name == NULL)
	        name = "bad cfstring ref";
	    return(name);
	}

	if(selref == TRUE && pointer_value == 0){
	    pointer_value = get_objc2_64bit_selref(value,
			info->load_commands, info->ncmds, info->sizeofcmds,
			info->object_byte_sex, info->object_addr,
			info->object_size, info->symbols64, info->nsymbols,
			info->strings, info->strings_size, info->cputype);
	}

	if(pointer_value != 0)
	    value = pointer_value;

	/*
	 * See if the value is pointing to a cstring.
	 */
	name = guess_cstring_pointer(value, ncmds, sizeofcmds, load_commands,
				     object_byte_sex, object_addr, object_size);
	if(name != NULL){
	    if(pointer_value != 0 && selref == TRUE){
	        *reference_type =
		    LLVMDisassembler_ReferenceType_Out_Objc_Selector_Ref;
		info->selector_name = name;
	    }
	    else if(pointer_value != 0 && msgref == TRUE){
		info->class_name = NULL;
	        *reference_type =
		    LLVMDisassembler_ReferenceType_Out_Objc_Message_Ref;
		info->selector_name = name;
	    }
            else
		*reference_type =
		    LLVMDisassembler_ReferenceType_Out_LitPool_CstrAddr;
	    return(name);
	}

	name = guess_indirect_symbol(value, ncmds, sizeofcmds, load_commands,
		object_byte_sex, info->indirect_symbols,info->nindirect_symbols,
		info->symbols, info->symbols64, info->nsymbols, info->strings,
		info->strings_size);
	if(name != NULL){
	    *reference_type =
		    LLVMDisassembler_ReferenceType_Out_LitPool_SymAddr;
	    return(name);
	}

	return(NULL);
}

/*
 * method_reference() is called passing it the ReferenceName that might be
 * a reference it to an Objective-C method.  If so then it allocates and
 * assembles a method call string with the values last seen and saved in
 * the disassemble_info's class_name and selector_name fields.  This is saved
 * into the method field and any previous string is free'ed.  Then the
 * class_name field is NULL'ed out.
 */
static
void
method_reference(
struct disassemble_info *info,
uint64_t *ReferenceType,
const char **ReferenceName)
{
	if(*ReferenceName != NULL){
	    if(strcmp(*ReferenceName, "_objc_msgSend") == 0){
		*ReferenceType =
		    LLVMDisassembler_ReferenceType_Out_Objc_Message;
		if(info->selector_name != NULL){
		    if(info->method != NULL)
			free(info->method);
		    if(info->class_name != NULL){
			info->method =
			    allocate(5 + strlen(info->class_name) +
				    strlen(info->selector_name));
			strcpy(info->method, "+[");
			strcat(info->method, info->class_name);
			strcat(info->method, " ");
			strcat(info->method, info->selector_name);
			strcat(info->method, "]");
			*ReferenceName = info->method;
			info->class_name = NULL;
		    }
		    else{
			info->method =
			    allocate(9 + strlen(info->selector_name));
			strcpy(info->method, "-[%rdi ");
			strcat(info->method, info->selector_name);
			strcat(info->method, "]");
			*ReferenceName = info->method;
		    }
		}
	    }
	    else if(strcmp(*ReferenceName, "_objc_msgSendSuper2") == 0){
		*ReferenceType =
		    LLVMDisassembler_ReferenceType_Out_Objc_Message;
		if(info->selector_name != NULL){
		    if(info->method != NULL)
			free(info->method);
		    info->method =
			allocate(17 + strlen(info->selector_name));
		    strcpy(info->method, "-[[%rdi super] ");
		    strcat(info->method, info->selector_name);
		    strcat(info->method, "]");
		    *ReferenceName = info->method;
		    info->class_name = NULL;
		}
	    }
	}
}

/*
 * The symbol lookup function passed to LLVMCreateDisasm().  It looks up the
 * SymbolValue using the info passed vis the pointer to the struct
 * disassemble_info that was passed when disassembler context is created and
 * returns the symbol name that matches or NULL if none.
 *
 * When this is called to get a symbol name for a branch target then the
 * ReferenceType can be LLVMDisassembler_ReferenceType_In_Branch and then
 * SymbolValue will be looked for in the indirect symbol table to determine if
 * it is an address for a symbol stub.  If so then the symbol name for that
 * stub is returned indirectly through ReferenceName and then ReferenceType is
 * set to LLVMDisassembler_ReferenceType_Out_SymbolStub.
 */
static
const char *
SymbolLookUp(
void *DisInfo,
uint64_t SymbolValue,
uint64_t *ReferenceType,
uint64_t ReferencePC,
const char **ReferenceName)
{
    struct disassemble_info *info;
    const char *SymbolName;
    uint32_t i;

	info = (struct disassemble_info *)DisInfo;
	if(info->verbose == FALSE){
	    *ReferenceName = NULL;
	    *ReferenceType = LLVMDisassembler_ReferenceType_InOut_None;
	    return(NULL);
	}
	SymbolName = guess_symbol(SymbolValue, info->sorted_symbols,
				  info->nsorted_symbols, TRUE);
	if(SymbolName == NULL && info->insts != NULL && info->ninsts != 0){
	    for(i = 0; i < info->ninsts; i++){
		if(info->insts[i].address == SymbolValue){
		    SymbolName = info->insts[i].tmp_label;
		    break;
		}
	    }
	}

	if(*ReferenceType == LLVMDisassembler_ReferenceType_In_Branch){
	    *ReferenceName = guess_indirect_symbol(SymbolValue,
		    info->ncmds, info->sizeofcmds, info->load_commands,
		    info->object_byte_sex, info->indirect_symbols,
		    info->nindirect_symbols, info->symbols, info->symbols64,
		    info->nsymbols, info->strings, info->strings_size);
	    if(*ReferenceName != NULL){
		method_reference(info, ReferenceType, ReferenceName);
		if(*ReferenceType !=
			LLVMDisassembler_ReferenceType_Out_Objc_Message)
		    *ReferenceType =
			LLVMDisassembler_ReferenceType_Out_SymbolStub;
	    }
	    else if(SymbolName != NULL && strncmp(SymbolName, "__Z", 3) == 0){
		if(info->demangled_name != NULL)
		    free(info->demangled_name);
		info->demangled_name = __cxa_demangle(SymbolName + 1, 0, 0, 0);
		if(info->demangled_name != NULL){
		    *ReferenceName = info->demangled_name;
		    *ReferenceType =
			LLVMDisassembler_ReferenceType_DeMangled_Name;
		}
		else
		    *ReferenceType = LLVMDisassembler_ReferenceType_InOut_None;
	    }
	    else
		*ReferenceType = LLVMDisassembler_ReferenceType_InOut_None;
	    if(info->inst != NULL && SymbolName == NULL){
		info->inst->has_raw_target_address = TRUE;
		info->inst->raw_target_address = SymbolValue;
	    }
	}
	else if(*ReferenceType == LLVMDisassembler_ReferenceType_In_PCrel_Load){
	    *ReferenceName = guess_literal_pointer(SymbolValue, ReferencePC,
						   ReferenceType, info);
	    if(*ReferenceName != NULL)
		method_reference(info, ReferenceType, ReferenceName);
	    else
		*ReferenceType = LLVMDisassembler_ReferenceType_InOut_None;
	}
	else if(SymbolName != NULL && strncmp(SymbolName, "__Z", 3) == 0){
	    if(info->demangled_name != NULL)
		free(info->demangled_name);
	    info->demangled_name = __cxa_demangle(SymbolName + 1, 0, 0, 0);
	    if(info->demangled_name != NULL){
		*ReferenceName = info->demangled_name;
		*ReferenceType = LLVMDisassembler_ReferenceType_DeMangled_Name;
	    }
	}
	else{
	    *ReferenceName = NULL;
	    *ReferenceType = LLVMDisassembler_ReferenceType_InOut_None;
	}
	return(SymbolName);
}

LLVMDisasmContextRef
create_i386_llvm_disassembler(
void)
{
    LLVMDisasmContextRef dc;

	dc = llvm_create_disasm("i386-apple-darwin10", mcpu, &dis_info, 1,
				i386GetOpInfo, SymbolLookUp);
	return(dc);
}

void
delete_i386_llvm_disassembler(
LLVMDisasmContextRef dc)
{
	llvm_disasm_dispose(dc);
}

LLVMDisasmContextRef
create_x86_64_llvm_disassembler(
void)
{
    LLVMDisasmContextRef dc;

	dc = llvm_create_disasm("x86_64-apple-darwin10", mcpu, &dis_info, 1,
				x86_64GetOpInfo, SymbolLookUp);
	return(dc);
}

void
delete_x86_64_llvm_disassembler(
LLVMDisasmContextRef dc)
{
	llvm_disasm_dispose(dc);
}
