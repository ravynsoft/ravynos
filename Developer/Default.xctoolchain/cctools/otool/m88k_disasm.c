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
#include <stdio.h>
#include <string.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/reloc.h>
#include <mach-o/m88k/reloc.h>
#include "stuff/bytesex.h"
#include "stuff/symbol.h"
#include "otool.h"

#define	D(x)		(((x) >> 21) & 0x1f)
#define	S1(x)		(((x) >> 16) & 0x1f)
#define	S2(x)		((x) & 0x1f)
#define	CR(x)		(((x) >> 5) & 0x3f)
#define	T1(x)		(((x) >> 9) & 0x3)
#define	T2(x)		(((x) >> 7) & 0x3)
#define	TD(x)		(((x) >> 5) & 0x3)
#define	W(x)		(((x) >> 10) & 0x1)
#define	B(x)		(((x) >> 10) & 0x1)
#define	USR(x)		(((x) >> 8) & 0x1)
#define TY(x)		(((x) >> 10) & 0x3)
#define THRU(x)		(((x) >> 7) & 0x1)
#define IO(x)		(((x) >> 8) & 0x3)
#define SAT(x)		(((x) >> 7) & 0x3)
#define PT(x)		(((x) >> 5) & 0x3)
#define	B5(x)		(((x) >> 21) & 0x1f)
#define	M5(x)		(((x) >> 21) & 0x1f)
#define	W5(x)		(((x) >> 5) & 0x1f)
#define	OP5(x)		((x) & 0x1f)
#define	VEC9(x)		((x) & 0x1ff)

static char ty[] = { 's', 'd', 'x', '?' };
static char *ty_star[] = { ".s", ".d", ".?", ".?" };
static char *w[] = { ".bu", "" };
static char *b[] = { ".hu", ".bu" };
static char *usr[] = { "", ".usr" };
static char *tyr0[] = { ".d", "", ".x", ".?" };
static char *tyr1[] = { ".d", "", ".h", ".b" };
static char *tyf1[] = { ".d", "", ".h", ".x" };
static char *thru[] = { "", ".wt" };
static char *io[] = { "", ".co", ".ci", ".cio" };
static char *sat[] = { "", "s.u", "s.us", "s.s" };
static char *pt[] = { ".n", ".b", ".h", "" };
static char *b5[] = { "??", "??", "eq", "ne", "gt", "le", "lt", "ge", "hi",
		      "ls", "lo", "hs", "be", "nb", "he", "nh" };

static void print_b5(
    uint32_t value);
static void print_m5(
    uint32_t value);
static void print_immediate(
    uint32_t value, 
    uint32_t sect_offset,
    struct relocation_info *sorted_relocs,
    uint32_t nsorted_relocs,
    struct nlist *symbols,
    uint32_t nsymbols,
    struct symbol *sorted_symbols,
    uint32_t nsorted_symbols,
    char *strings,
    uint32_t strings_size,
    enum bool verbose);

uint32_t
m88k_disassemble(
char *sect,
uint32_t left,
uint32_t addr,
uint32_t sect_addr,
enum byte_sex object_byte_sex,
struct relocation_info *relocs,
uint32_t nrelocs,
struct nlist *symbols,
uint32_t nsymbols,
struct symbol *sorted_symbols,
uint32_t nsorted_symbols,
char *strings,
uint32_t strings_size,
enum bool verbose)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    uint32_t opcode;
    uint32_t sect_offset;

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != object_byte_sex;
	sect_offset = addr - sect_addr;

	if(left < sizeof(uint32_t)){
	   if(left != 0){
		memcpy(&opcode, sect, left);
		if(swapped)
		    opcode = SWAP_INT(opcode);
		printf(".long\t0x%08x\n", (unsigned int)opcode);
	   }
	   printf("(end of section)\n");
	   return(left);
	}

	memcpy(&opcode, sect, sizeof(uint32_t));
	if(swapped)
	    opcode = SWAP_INT(opcode);

	switch(opcode & 0xfc000000){
	case 0x00000000:
	    printf("ld.d\tx%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x04000000:
	    printf("ld\tx%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x08000000:
	    printf("ld.hu\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x0c000000:
	    printf("ld.bu\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x10000000:
	    printf("ld.d\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x14000000:
	    printf("ld\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x18000000:
	    printf("ld.h\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x1c000000:
	    printf("ld.b\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x20000000:
	    printf("st.d\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x24000000:
	    printf("st\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x28000000:
	    printf("st.h\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x2c000000:
	    printf("st.b\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x30000000:
	    printf("st.d\tx%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x34000000:
	    printf("st\tx%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x38000000:
	    printf("st.x\tx%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x3c000000:
	    printf("ld.x\tx%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x40000000:
	    printf("and\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x44000000:
	    printf("and.u\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x48000000:
	    printf("mask\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x4c000000:
	    printf("mask.u\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x50000000:
	    printf("xor\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x54000000:
	    printf("xor.u\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x58000000:
	    printf("or\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x5c000000:
	    printf("or.u\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x60000000:
	    printf("addu\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x64000000:
	    printf("subu\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x68000000:
	    printf("divu\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x6c000000:
	    printf("mulu\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x70000000:
	    printf("add\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x74000000:
	    printf("sub\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x78000000:
	    printf("divs\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x7C000000:
	    printf("cmp\tr%u,r%u,",D(opcode), S1(opcode));
	    print_immediate(opcode & 0xffff, sect_offset, relocs, nrelocs,
			    symbols, nsymbols, sorted_symbols, nsorted_symbols,
			    strings, strings_size, verbose);
	    break;
	case 0x80000000:
	    switch(opcode & 0x0000f800){
	    case 0x00004000:
		if((opcode & 0x001f001f) == 0)
		    printf("ldcr\tr%u,cr%u\n", D(opcode), CR(opcode));
		else
		    printf("0x%08x\n", (unsigned int)opcode);
		break;
	    case 0x00008000:
		if((opcode & 0x03e00000) == 0)
		    if(S1(opcode) == S2(opcode))
			printf("stcr\tr%u,cr%u\n", S1(opcode), CR(opcode));
		    else
			printf("stcr\tr%u,cr%u\t| error: fields S1 != S2\n",
				S1(opcode), CR(opcode));
		else
		    printf("0x%08x\n", (unsigned int)opcode);
		break;
	    case 0x0000c000:
		if(S1(opcode) == S2(opcode))
		    printf("xcr\tr%u,r%u,cr%u\n", D(opcode), S1(opcode),
			    CR(opcode));
		else
		    printf("xcr\tr%u,r%u,cr%u\t| error: fields S1 != S2\n",
			    D(opcode), S1(opcode), CR(opcode));
		break;
	    case 0x00004800:
		if((opcode & 0x001f001f) == 0)
		    printf("fldcr\tr%u,fcr%u\n", D(opcode), CR(opcode));
		else
		    printf("0x%08x\n", (unsigned int)opcode);
		break;
	    case 0x00008800:
		if((opcode & 0x03e00000) == 0)
		    if(S1(opcode) == S2(opcode))
			printf("fstcr\tr%u,fcr%u\n", S1(opcode), CR(opcode));
		    else
			printf("fstcr\tr%u,fcr%u\t| error: fields S1 != S2\n",
				S1(opcode), CR(opcode));
		else
		    printf("0x%08x\n", (unsigned int)opcode);
		break;
	    case 0x0000c800:
		if(S1(opcode) == S2(opcode))
		    printf("fxcr\tr%u,r%u,fcr%u\n", D(opcode), S1(opcode),
			    CR(opcode));
		else
		    printf("fxcr\tr%u,r%u,fcr%u\t| error: fields S1 != S2\n",
			    D(opcode), S1(opcode), CR(opcode));
		break;
	    default:
		printf("0x%08x\n", (unsigned int)opcode);
		break;
	    }
	    break;

	/* all sfu1 opcodes */
	case 0x84000000:
	    switch(opcode & 0x00007800){
	    case 0x00000000:
		if(opcode & 0x00008000)
		    printf("fmul.%c%c%c\tx%u,x%u,x%u\n", ty[TD(opcode)],
			    ty[T1(opcode)], ty[T2(opcode)], D(opcode),
			    S1(opcode), S2(opcode));
		else
		    printf("fmul.%c%c%c\tr%u,r%u,r%u\n", ty[TD(opcode)],
			    ty[T1(opcode)], ty[T2(opcode)], D(opcode),
			    S1(opcode), S2(opcode));
		break;
	    case 0x00000800:
		if(opcode & 0x00008000)
		    printf("fcvt.%c%c\tx%u,x%u\n", ty[TD(opcode)],
			    ty[T2(opcode)], D(opcode), S2(opcode));
		else
		    printf("fcvt.%c%c\tr%u,r%u\n", ty[TD(opcode)],
			    ty[T2(opcode)], D(opcode), S2(opcode));
		break;
	    case 0x00002000:
		if(opcode & 0x00000200)
		    printf("flt.%cs\tx%u,r%u\n", ty[TD(opcode)], D(opcode),
			   S2(opcode));
		else
		    printf("flt.%cs\tr%u,r%u\n", ty[TD(opcode)], D(opcode),
			   S2(opcode));
		break;
	    case 0x00002800:
		if(opcode & 0x00008000)
		    printf("fadd.%c%c%c\tx%u,x%u,x%u\n", ty[TD(opcode)],
			    ty[T1(opcode)], ty[T2(opcode)], D(opcode),
			    S1(opcode), S2(opcode));
		else
		    printf("fadd.%c%c%c\tr%u,r%u,r%u\n", ty[TD(opcode)],
			    ty[T1(opcode)], ty[T2(opcode)], D(opcode),
			    S1(opcode), S2(opcode));
		break;
	    case 0x00003000:
		if(opcode & 0x00008000)
		    printf("fsub.%c%c%c\tx%u,x%u,x%u\n", ty[TD(opcode)],
			    ty[T1(opcode)], ty[T2(opcode)], D(opcode),
			    S1(opcode), S2(opcode));
		else
		    printf("fsub.%c%c%c\tr%u,r%u,r%u\n", ty[TD(opcode)],
			    ty[T1(opcode)], ty[T2(opcode)], D(opcode),
			    S1(opcode), S2(opcode));
		break;
	    case 0x00003800:
		if((opcode & 0x00000060) == 0x00000000){
		    if(opcode & 0x00008000)
			printf("fcmp.s%c%c\tr%u,x%u,x%u\n", ty[T1(opcode)],
				ty[T2(opcode)], D(opcode), S1(opcode),
				S2(opcode));
		    else
			printf("fcmp.s%c%c\tr%u,r%u,r%u\n", ty[T1(opcode)],
				ty[T2(opcode)], D(opcode), S1(opcode),
				S2(opcode));
		}
		else if((opcode & 0x00000060) == 0x00000020){
		    if(opcode & 0x00008000)
			printf("fcmpu.s%c%c\tr%u,x%u,x%u\n", ty[T1(opcode)],
				ty[T2(opcode)], D(opcode), S1(opcode),
				S2(opcode));
		    else
			printf("fcmpu.s%c%c\tr%u,r%u,r%u\n", ty[T1(opcode)],
				ty[T2(opcode)], D(opcode), S1(opcode),
				S2(opcode));
		}
		else
		    printf("0x%08x\n", (unsigned int)opcode);
		break;
	    case 0x00004000:
		if((opcode & 0x001f8660) == 0x00008000){
		    /* mov to g (from x) */
		    printf("mov%s\tr%u,x%u\n", ty_star[T2(opcode)],
			    D(opcode), S2(opcode));
		}
		else if((opcode & 0x001f8660) == 0x00000200){
		    /* mov to x (from g) */
		    printf("mov%s\tx%u,r%u\n", ty_star[T2(opcode)],
			    D(opcode), S2(opcode));
		}
		else if((opcode & 0x001f87e0) == 0x00008300){
		    /* mov to x (from x) */
		    printf("mov\tx%u,x%u\n", D(opcode), S2(opcode));
		}
		else
		    printf("0x%08x\n", (unsigned int)opcode);
		break;
	    case 0x00004800:
		if((opcode & 0x001f8660) == 0x00008000){
		    printf("int.s%c\tr%u,x%u\n", ty[T2(opcode)], D(opcode),
			   S2(opcode));
		}
		else if((opcode & 0x001f8660) == 0x00000000){
		    printf("int.s%c\tr%u,r%u\n", ty[T2(opcode)], D(opcode),
			   S2(opcode));
		}
		else
		    printf("0x%08x\n", (unsigned int)opcode);
		break;
	    case 0x00005000:
		if((opcode & 0x001f8660) == 0x00008000){
		    printf("nint.s%c\tr%u,x%u\n", ty[T2(opcode)], D(opcode),
			   S2(opcode));
		}
		else if((opcode & 0x001f8660) == 0x00000000){
		    printf("nint.s%c\tr%u,r%u\n", ty[T2(opcode)], D(opcode),
			   S2(opcode));
		}
		else
		    printf("0x%08x\n", (unsigned int)opcode);
		break;
	    case 0x00005800:
		if((opcode & 0x001f8660) == 0x00008000){
		    printf("trnc.s%c\tr%u,x%u\n", ty[T2(opcode)], D(opcode),
			   S2(opcode));
		}
		else if((opcode & 0x001f8660) == 0x00000000){
		    printf("trnc.s%c\tr%u,r%u\n", ty[T2(opcode)], D(opcode),
			   S2(opcode));
		}
		else
		    printf("0x%08x\n", (unsigned int)opcode);
		break;
	    case 0x00007000:
		if(opcode & 0x00008000)
		    printf("fdiv.%c%c%c\tx%u,x%u,x%u\n", ty[TD(opcode)],
			    ty[T1(opcode)], ty[T2(opcode)], D(opcode),
			    S1(opcode), S2(opcode));
		else
		    printf("fdiv.%c%c%c\tr%u,r%u,r%u\n", ty[TD(opcode)],
			    ty[T1(opcode)], ty[T2(opcode)], D(opcode),
			    S1(opcode), S2(opcode));
		break;
	    case 0x00007800:
		if(opcode & 0x00008000)
		    printf("fsqrt.%c%c\tx%u,x%u\n", ty[TD(opcode)],
			    ty[T2(opcode)], D(opcode), S2(opcode));
		else
		    printf("fsqrt.%c%c\tr%u,r%u\n", ty[TD(opcode)],
			    ty[T2(opcode)], D(opcode), S2(opcode));
		break;
	    default:
		printf("0x%08x\n", (unsigned int)opcode);
		break;
	    }
	    break;

	/* all sfu2 opcodes */
	case 0x88000000:
	    switch(opcode & 0x0000f800){
	    case 0x00000000:
		printf("pmul\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
		       S2(opcode) );
		break;
	    case 0x00002000:
		printf("padd%s%s\tr%u,r%u,r%u\n", sat[SAT(opcode)],
		       pt[PT(opcode)], D(opcode), S1(opcode), S2(opcode) );
		break;
	    case 0x00003000:
		printf("psub%s%s\tr%u,r%u,r%u\n", sat[SAT(opcode)],
		       pt[PT(opcode)], D(opcode), S1(opcode), S2(opcode) );
		break;
	    case 0x00003800:
		printf("pcmp\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
		       S2(opcode) );
		break;
	    case 0x00006000:
		switch(opcode & 0x00000780){
		case 0x00000100:
		    printf("ppack.8%s\tr%u,r%u,r%u\n", pt[PT(opcode)],
			   D(opcode), S1(opcode), S2(opcode) );
		    break;
		case 0x00000200:
		    printf("ppack.16%s\tr%u,r%u,r%u\n", pt[PT(opcode)],
			   D(opcode), S1(opcode), S2(opcode) );
		    break;
		case 0x00000400:
		    printf("ppack.32%s\tr%u,r%u,r%u\n", pt[PT(opcode)],
			   D(opcode), S1(opcode), S2(opcode) );
		    break;
		default:
		    printf("0x%08x\n", (unsigned int)opcode);
		    break;
		}
		break;
	    case 0x00006800:
		printf("punpk%s\tr%u,r%u\n", pt[PT(opcode)], D(opcode),
		       S1(opcode) );
		break;
	    case 0x00007000:
		printf("prot\tr%u,r%u,<%u>\n", D(opcode), S1(opcode),
		       ((opcode >> 7) & 0xf) << 2 );
		break;
	    case 0x00007800:
		printf("prot\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
		       S2(opcode) );
		break;
	    default:
		printf("0x%08x\n", (unsigned int)opcode);
		break;
	    }
	    break;

	case 0xc0000000:
	case 0xc4000000:
	    if((opcode & 0x04000000) == 0)
		printf("br\t");
	    else
		printf("br.n\t");
	    if((opcode & 0x02000000) == 0)
		print_immediate(addr + ((opcode & 0x03ffffff) << 2),sect_offset,
		    relocs, nrelocs, symbols, nsymbols, sorted_symbols,
		    nsorted_symbols, strings, strings_size, verbose);
	    else
		print_immediate(addr +
		    (((opcode & 0x03ffffff) << 2) | 0xf0000000), sect_offset,
		    relocs, nrelocs, symbols, nsymbols, sorted_symbols,
		    nsorted_symbols, strings, strings_size, verbose);
	    break;
	case 0xc8000000:
	case 0xcc000000:
	    if((opcode & 0x04000000) == 0)
		printf("bsr\t");
	    else
		printf("bsr.n\t");
	    if((opcode & 0x02000000) == 0)
		print_immediate(addr + ((opcode & 0x03ffffff) << 2),sect_offset,
		    relocs, nrelocs, symbols, nsymbols, sorted_symbols,
		    nsorted_symbols, strings, strings_size, verbose);
	    else
		print_immediate(addr +
		    (((opcode & 0x03ffffff) << 2) | 0xf0000000), sect_offset,
		    relocs, nrelocs, symbols, nsymbols, sorted_symbols,
		    nsorted_symbols, strings, strings_size, verbose);
	    break;
	case 0xd0000000:
	case 0xd4000000:
	    if((opcode & 0x04000000) == 0)
		printf("bb0\t");
	    else
		printf("bb0.n\t");
	    print_b5(B5(opcode));
	    printf(",r%u,", S1(opcode) );
	    if((opcode & 0x00008000) == 0)
		print_immediate(addr + ((opcode & 0x0000ffff) << 2),sect_offset,
		    relocs, nrelocs, symbols, nsymbols, sorted_symbols,
		    nsorted_symbols, strings, strings_size, verbose);
	    else
		print_immediate(addr +
		    (((opcode & 0x0000ffff) << 2) | 0xfffc0000), sect_offset,
		    relocs, nrelocs, symbols, nsymbols, sorted_symbols,
		    nsorted_symbols, strings, strings_size, verbose);
	    break;
	case 0xd8000000:
	case 0xdc000000:
	    if((opcode & 0x04000000) == 0)
		printf("bb1\t");
	    else
		printf("bb1.n\t");
	    print_b5(B5(opcode));
	    printf(",r%u,", S1(opcode) );
	    if((opcode & 0x00008000) == 0)
		print_immediate(addr + ((opcode & 0x0000ffff) << 2),sect_offset,
		    relocs, nrelocs, symbols, nsymbols, sorted_symbols,
		    nsorted_symbols, strings, strings_size, verbose);
	    else
		print_immediate(addr +
		    (((opcode & 0x0000ffff) << 2) | 0xfffc0000), sect_offset,
		    relocs, nrelocs, symbols, nsymbols, sorted_symbols,
		    nsorted_symbols, strings, strings_size, verbose);
	    break;
	case 0xe8000000:
	case 0xec000000:
	    if((opcode & 0x04000000) == 0)
		printf("bcnd\t");
	    else
		printf("bcnd.n\t");
	    print_m5(M5(opcode));
	    printf(",r%u,", S1(opcode) );
	    if((opcode & 0x00008000) == 0)
		print_immediate(addr + ((opcode & 0x0000ffff) << 2),sect_offset,
		    relocs, nrelocs, symbols, nsymbols, sorted_symbols,
		    nsorted_symbols, strings, strings_size, verbose);
	    else
		print_immediate(addr +
		    (((opcode & 0x0000ffff) << 2) | 0xfffc0000), sect_offset,
		    relocs, nrelocs, symbols, nsymbols, sorted_symbols,
		    nsorted_symbols, strings, strings_size, verbose);
	    break;

	case 0xf0000000:
	    switch(opcode & 0x0000c000){
	    /* ld, st, lda[] */
	    case 0x00000000:
		switch(opcode & 0x0000f000){
		case 0x00000000:
		    printf("0x%08x\n", (unsigned int)opcode);
		    break;
		case 0x00001000:
		    /* ld R = 0, XRF dest */
		    if((opcode & 0x00000200) == 0)
			printf("ld%s%s\tx%u,r%u,r%u\n", tyr0[TY(opcode)],
			       usr[USR(opcode)], D(opcode), S1(opcode),
			       S2(opcode) );
		    else
			printf("ld%s%s\tx%u,r%u[r%u]\n", tyr0[TY(opcode)],
			       usr[USR(opcode)], D(opcode), S1(opcode),
			       S2(opcode) );
		    break;
		case 0x00002000:
		    /* st R = 0, XRF dest */
		    if((opcode & 0x00000200) == 0)
			printf("st%s%s%s\tx%u,r%u,r%u\n", tyr0[TY(opcode)],
			       usr[USR(opcode)], thru[THRU(opcode)], D(opcode),
			       S1(opcode), S2(opcode) );
		    else
			printf("st%s%s%s\tx%u,r%u[r%u]\n", tyr0[TY(opcode)],
			       usr[USR(opcode)], thru[THRU(opcode)], D(opcode),
			       S1(opcode), S2(opcode) );
		    break;
		default:
		    printf("0x%08x\n", (unsigned int)opcode);
		    break;
		}
		break;
	    case 0x00004000:
		printf("0x%08x\n", (unsigned int)opcode);
		break;
	    /* clr, set, ext, extu, mak, rot */
	    case 0x00008000:
		switch(opcode & 0x0000fc00){
		case 0x00008000:
		    printf("clr\tr%u,r%u,%u<%u>\n", D(opcode), S1(opcode),
			   W5(opcode), OP5(opcode) );
		    break;
		case 0x00008800:
		    printf("set\tr%u,r%u,%u<%u>\n", D(opcode), S1(opcode),
			   W5(opcode), OP5(opcode) );
		    break;
		case 0x00009000:
		    printf("ext\tr%u,r%u,%u<%u>\n", D(opcode), S1(opcode),
			   W5(opcode), OP5(opcode) );
		    break;
		case 0x00009800:
		    printf("extu\tr%u,r%u,%u<%u>\n", D(opcode), S1(opcode),
			   W5(opcode), OP5(opcode) );
		    break;
		case 0x0000a000:
		    printf("mak\tr%u,r%u,%u<%u>\n", D(opcode), S1(opcode),
			   W5(opcode), OP5(opcode) );
		    break;
		case 0x0000a800:
		    printf("rot\tr%u,r%u,<%u>\n", D(opcode), S1(opcode),
			   OP5(opcode) );
		    break;
		default:
		    printf("0x%08x\n", (unsigned int)opcode);
		    break;
		}
		break;
	    /* tb0, tb1, tcnd */
	    case 0x0000c000:
		switch(opcode & 0x0000fe00){
		case 0x0000d000:
		    printf("tb0\t");
		    print_b5(B5(opcode));
	    	    printf(",r%u,%u\n", S1(opcode), VEC9(opcode) );
		    break;
		case 0x0000d800:
		    printf("tb1\t");
		    print_b5(B5(opcode));
	    	    printf(",r%u,%u\n", S1(opcode), VEC9(opcode) );
		    break;
		case 0x0000e800:
		    printf("tcnd\t");
		    print_m5(M5(opcode));
	    	    printf(",r%u,%u\n", S1(opcode), VEC9(opcode) );
		    break;
		default:
		    printf("0x%08x\n", (unsigned int)opcode);
		    break;
		}
		break;
	    }
	    break;

	case 0xf4000000:
	    switch(opcode & 0x0000e000){
	    /* xmem, ld.u, ld R = 1 GRF dest */
	    case 0x00000000:
		if((opcode & 0x00001000) == 0)
		    if((opcode & 0x00000800) == 0)
			/* xmem */
			if((opcode & 0x00000200) == 0)
			    printf("xmem%s%s\tr%u,r%u,r%u\n", w[W(opcode)],
				   usr[USR(opcode)], D(opcode), S1(opcode),
				   S2(opcode) );
			else
			    printf("xmem%s%s\tr%u,r%u[r%u]\n", w[W(opcode)],
				   usr[USR(opcode)], D(opcode), S1(opcode),
				   S2(opcode) );
		    else
			/* ld.u */
			if((opcode & 0x00000200) == 0)
			    printf("ld%s%s\tr%u,r%u,r%u\n", b[B(opcode)],
				   usr[USR(opcode)], D(opcode), S1(opcode),
				   S2(opcode) );
			else
			    printf("ld%s%s\tr%u,r%u[r%u]\n", b[B(opcode)],
				   usr[USR(opcode)], D(opcode), S1(opcode),
				   S2(opcode) );
		else
		    /* ld R = 1, GRF dest */
		    if((opcode & 0x00000200) == 0)
			printf("ld%s%s\tr%u,r%u,r%u\n", tyr1[TY(opcode)],
			       usr[USR(opcode)], D(opcode), S1(opcode),
			       S2(opcode) );
		    else
			printf("ld%s%s\tr%u,r%u[r%u]\n", tyr1[TY(opcode)],
			       usr[USR(opcode)], D(opcode), S1(opcode),
			       S2(opcode) );
		break;
	    /* st R = 0 GRF dest, lda[] F = 1, 8x, 4x, 2x, 16x scale factors */
	    case 0x00002000:
		if((opcode & 0x00001000) == 0)
		    /* st R = 0, GRF dest */
		    if((opcode & 0x00000200) == 0)
			printf("st%s%s%s\tr%u,r%u,r%u\n", tyr1[TY(opcode)],
			       usr[USR(opcode)], thru[THRU(opcode)], D(opcode),
			       S1(opcode), S2(opcode) );
		    else
			printf("st%s%s%s\tr%u,r%u[r%u]\n", tyr1[TY(opcode)],
			       usr[USR(opcode)], thru[THRU(opcode)], D(opcode),
			       S1(opcode), S2(opcode) );
		else
		    /* lda[] F = 1, 8x, 4x, 2x, 16x scale factors */
		    printf("lda%s\tr%u,r%u[r%u]\n", tyf1[TY(opcode)],
			   D(opcode), S1(opcode), S2(opcode) );
		break;
	    /* and, or, xor */
	    case 0x00004000:
		switch(opcode & 0x00001800){
		case 0x00000000:
		    if((opcode & 0x00000400) == 0)
			printf("and\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
			       S2(opcode) );
		    else
			printf("and.c\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
			       S2(opcode) );
		    break;
		case 0x00000800:
		    printf("0x%08x\n", (unsigned int)opcode);
		    break;
		case 0x00001000:
		    if((opcode & 0x00000400) == 0)
			printf("xor\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
			       S2(opcode) );
		    else
			printf("xor.c\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
			       S2(opcode) );
		    break;
		case 0x00001800:
		    if((opcode & 0x00000400) == 0)
			printf("or\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
			       S2(opcode) );
		    else
			printf("or.c\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
			       S2(opcode) );
		    break;
		}
		break;
	    /* andu, subu, divu, mulu, muls, add, sub, divs, cmp */
	    case 0x00006000:
		switch(opcode & 0x00001c00){
		case 0x00000000:
		    printf("addu%s\tr%u,r%u,r%u\n", io[IO(opcode)], D(opcode),
			   S1(opcode), S2(opcode) );
		    break;
		case 0x00000400:
		    printf("subu%s\tr%u,r%u,r%u\n", io[IO(opcode)], D(opcode),
			   S1(opcode), S2(opcode) );
		    break;
		case 0x00000800:
		    if((opcode & 0x00000100) == 0)
			printf("divu\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
			       S2(opcode) );
		    else
			printf("divu.d\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
			       S2(opcode) );
		    break;
		case 0x00000c00:
		    if((opcode & 0x00000200) == 0)
			if((opcode & 0x00000100) == 0)
			    printf("mulu\tr%u,r%u,r%u\n", D(opcode),
				   S1(opcode), S2(opcode) );
			else
			    printf("mulu.d\tr%u,r%u,r%u\n", D(opcode),
				   S1(opcode), S2(opcode) );
		    else
			printf("muls\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
			       S2(opcode) );
		    break;
		case 0x00001000:
		    printf("add%s\tr%u,r%u,r%u\n", io[IO(opcode)], D(opcode),
			   S1(opcode), S2(opcode) );
		    break;
		case 0x00001400:
		    printf("sub%s\tr%u,r%u,r%u\n", io[IO(opcode)], D(opcode),
			   S1(opcode), S2(opcode) );
		    break;
		case 0x00001800:
		    printf("divs\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
			   S2(opcode) );
		    break;
		case 0x00001c00:
		    printf("cmp\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
			   S2(opcode) );
		    break;
		}
		break;
	    /* clr, set, ext, extu, mak, rot */
	    case 0x00008000:
	    case 0x0000a000:
		switch(opcode & 0x0000ffe0){
		case 0x00008000:
		    printf("clr\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
			   S2(opcode) );
		    break;
		case 0x00008800:
		    printf("set\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
			   S2(opcode) );
		    break;
		case 0x00009000:
		    printf("ext\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
			   S2(opcode) );
		    break;
		case 0x00009800:
		    printf("extu\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
			   S2(opcode) );
		    break;
		case 0x0000a000:
		    printf("mak\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
			   S2(opcode) );
		    break;
		case 0x0000a800:
		    printf("rot\tr%u,r%u,r%u\n", D(opcode), S1(opcode),
			   S2(opcode) );
		    break;
		default:
		    printf("0x%08x\n", (unsigned int)opcode);
		    break;
		}
		break;
	    /* jmp, jsr */
	    case 0x0000c000:
		switch(opcode & 0x03ffffe0){
		case 0x0000c000:
		    printf("jmp\tr%u\n", S2(opcode) );
		    break;
		case 0x0000c400:
		    printf("jmp.n\tr%u\n", S2(opcode) );
		    break;
		case 0x0000c800:
		    printf("jsr\tr%u\n", S2(opcode) );
		    break;
		case 0x0000cc00:
		    printf("jsr.n\tr%u\n", S2(opcode) );
		    break;
		default:
		    printf("0x%08x\n", (unsigned int)opcode);
		    break;
		}
		break;
	    /* ff1, ff0, tbnd, rte */
	    case 0x0000e000:
		switch(opcode & 0x0000ffe0){
		case 0x0000e800:
		    printf("ff1\tr%u,r%u\n", D(opcode), S2(opcode) );
		    break;
		case 0x0000ec00:
		    printf("ff0\tr%u,r%u\n", D(opcode), S2(opcode) );
		    break;
		case 0x0000f800:
		    printf("tbnd\tr%u,r%u\n", S1(opcode), S2(opcode) );
		    break;
		case 0x0000fc00:
		    if((opcode & 0x3) == 0)
			printf("rte\n");
		    else
			printf("illop%u\n", opcode & 0x3);
		    break;
		default:
		    printf("0x%08x\n", (unsigned int)opcode);
		    break;
		}
		break;
	    }
	    break;

	case 0xf8000000:
	    printf("tbnd\tr%u,%u\n", S1(opcode), 0xffff & opcode);
	    break;

	default:
	    printf(".long 0x%08x\n", (unsigned int)opcode);
	    break;
	}
	return(4);
}

static
void
print_b5(
uint32_t value)
{
	if(value < 2 || value > 15)
	     printf("%u", value);
	else
	     printf("%s", b5[value]);
}

static
void
print_m5(
uint32_t value)
{
	switch(value){
	case 0x01:
	    printf("gt0");
	    break;
	case 0x02:
	    printf("eq0");
	    break;
	case 0x03:
	    printf("ge0");
	    break;
	case 0x0c:
	    printf("lt0");
	    break;
	case 0x0d:
	    printf("ne0");
	    break;
	case 0x0e:
	    printf("le0");
	    break;
	default:
	    printf("%u", value);
	}
}

static
void
print_immediate(
uint32_t value, 
uint32_t sect_offset,
struct relocation_info *relocs,
uint32_t nrelocs,
struct nlist *symbols,
uint32_t nsymbols,
struct symbol *sorted_symbols,
uint32_t nsorted_symbols,
char *strings,
uint32_t strings_size,
enum bool verbose)
{
    int32_t low, high, mid, reloc_found, offset;
    uint32_t i, r_address, r_symbolnum, r_type, r_extern,
	     r_value, r_scattered, pair_r_type;
    unsigned short other_half;
    char *name;
    struct relocation_info *rp, *pairp;
    struct scattered_relocation_info *srp, *spairp;

	r_symbolnum = 0;
	r_type = 0;
	r_extern = 0;
	r_value = 0;
	r_scattered = 0;
	other_half = 0;

	if(verbose == FALSE){
	    printf("0x%x\n", (unsigned int)value);
	    return;
	}
	reloc_found = 0;
	if(nrelocs != 0){
	    for(i = 0; i < nrelocs; i++){
		rp = &relocs[i];
		if(rp->r_address & R_SCATTERED){
		    srp = (struct scattered_relocation_info *)rp;
		    r_scattered = 1;
		    r_address = srp->r_address;
		    r_extern = 0;
		    r_type = srp->r_type;
		    r_value = srp->r_value;
		}
		else{
		    r_scattered = 0;
		    r_address = rp->r_address;
		    r_symbolnum = rp->r_symbolnum;
		    r_extern = rp->r_extern;
		    r_type = rp->r_type;
		}
		if(r_type == M88K_RELOC_PAIR){
		    fprintf(stderr, "Stray M88K_RELOC_PAIR relocation entry "
			    "%u\n", i);
		    continue;
		}
		if(r_address == sect_offset){
		    if(r_type == M88K_RELOC_HI16 || r_type == M88K_RELOC_LO16){
			if(i+1 < nrelocs){
			    pairp = &rp[1];
			    if(pairp->r_address & R_SCATTERED){
				spairp = (struct scattered_relocation_info *)
					 pairp;
				other_half = spairp->r_address & 0xfff;
				pair_r_type = spairp->r_type;
			    }
			    else{
				other_half = pairp->r_address & 0xffff;
				pair_r_type = pairp->r_type;
			    }
			    if(pair_r_type != M88K_RELOC_PAIR){
				fprintf(stderr, "No M88K_RELOC_PAIR relocation "
					"entry after entry %u\n", i);
				continue;
			    }
			}
		    }
		    reloc_found = 1;
		    break;
		}
		if(r_type == M88K_RELOC_HI16 || r_type == M88K_RELOC_LO16){
		    if(i+1 < nrelocs){
			pairp = &rp[1];
			if(pairp->r_address & R_SCATTERED){
			    spairp = (struct scattered_relocation_info *)pairp;
			    pair_r_type = spairp->r_type;
			}
			else{
			    pair_r_type = pairp->r_type;
			}
			if(pair_r_type == M88K_RELOC_PAIR)
			    i++;
			else
			    fprintf(stderr, "No M88K_RELOC_PAIR relocation "
				    "entry after entry %u\n", i);
		    }
		}
	    }
	}

	if(reloc_found && r_extern == 1){
	    if(r_symbolnum > nsymbols)
		name = "bad r_symbolnum offset";
	    else if((uint32_t)symbols[r_symbolnum].n_un.n_strx >= strings_size)
		name = "bad string offset";
	    else
		name = strings + symbols[r_symbolnum].n_un.n_strx;
	    if(value != 0){
		switch(r_type){
		case M88K_RELOC_HI16:
		    value = value << 16 | other_half;
		    printf("hi16(%s+0x%x)\n", name, (unsigned int)value);
		    break;
		case M88K_RELOC_LO16:
		    value = other_half << 16 | value;
		    printf("lo16(%s+0x%x)\n", name, (unsigned int)value);
		    break;
		default:
		    printf("%s+0x%x\n", name, (unsigned int)value);
		}
	    }
	    else{
		switch(r_type){
		case M88K_RELOC_HI16:
		    value = value << 16 | other_half;
		    if(value == 0)
			printf("hi16(%s)\n", name);
		    else
			printf("hi16(%s+0x%x)\n", name, (unsigned int)value);
		    break;
		case M88K_RELOC_LO16:
		    value = other_half << 16 | value;
		    if(value == 0)
			printf("lo16(%s)\n", name);
		    else
			printf("lo16(%s+0x%x)\n", name, (unsigned int)value);
		    break;
		default:
		    if(value == 0)
			printf("%s\n", name);
		    else
			printf("%s+0x%x\n", name, (unsigned int)value);
		}
	    }
	    return;
	}

	offset = 0;
	if(reloc_found){
	    if(r_type == M88K_RELOC_HI16)
		value = value << 16 | other_half;
	    else if(r_type == M88K_RELOC_LO16)
		value = other_half << 16 | value;
	    if(r_scattered){
		offset = value - r_value;
		value = r_value;
	    }
	}

	low = 0;
	high = nsorted_symbols - 1;
	mid = (high - low) / 2;
	while(high >= low){
	    if(sorted_symbols[mid].n_value == value){
		if(reloc_found){
		    switch(r_type){
		    case M88K_RELOC_HI16:
			if(offset == 0)
			    printf("hi16(%s)\n",
				   sorted_symbols[mid].name);
			else
			    printf("hi16(%s+0x%x)\n",
				    sorted_symbols[mid].name,
				    (unsigned int)offset);
			break;
		    case M88K_RELOC_LO16:
			if(offset == 0)
			    printf("lo16(%s)\n",
				   sorted_symbols[mid].name);
			else
			    printf("lo16(%s+0x%x)\n",
				   sorted_symbols[mid].name,
				   (unsigned int)offset);
			break;
		    default:
			if(offset == 0)
			    printf("%s\n",sorted_symbols[mid].name);
			else
			    printf("%s+0x%x\n",
				   sorted_symbols[mid].name,
				   (unsigned int)offset);
			break;
		    }
		}
		else{
		    if(offset == 0)
			printf("%s\n",sorted_symbols[mid].name);
		    else
			printf("%s+0x%x\n",
			       sorted_symbols[mid].name,
			       (unsigned int)offset);
		}
		return;
	    }
	    if(sorted_symbols[mid].n_value > value){
		high = mid - 1;
		mid = (high + low) / 2;
	    }
	    else{
		low = mid + 1;
		mid = (high + low) / 2;
	    }
	}
	if(offset == 0){
	    if(reloc_found){
		if(r_type == M88K_RELOC_HI16)
		    printf("hi16(0x%x)\n", (unsigned int)value);
		else if(r_type == M88K_RELOC_LO16)
		    printf("lo16(0x%x)\n", (unsigned int)value);
	    }
	    else
		printf("0x%04x\n", (unsigned int)value);
	}
	else{
	    if(reloc_found){
		if(r_type == M88K_RELOC_HI16)
		    printf("hi16(0x%x+0x%x)\n",
			    (unsigned int)value, (unsigned int)offset);
		else if(r_type == M88K_RELOC_LO16)
		    printf("lo16(0x%x+0x%x)\n",
			    (unsigned int)value, (unsigned int)offset);
	    }
	    else
		printf("0x%x+0x%x\n",
			(unsigned int)value, (unsigned int)offset);
	}
	return;
}
