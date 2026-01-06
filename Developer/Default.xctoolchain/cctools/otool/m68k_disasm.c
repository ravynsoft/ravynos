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
#include "stuff/bytesex.h"
#include "dyld_bind_info.h"
#include "ofile_print.h"
#include "stuff/symbol.h"
#include "otool.h"

#define GET_DOUBLE(sect, left, length, swapped, d) \
if((left) < sizeof(double)){ \
    (d) = 0.0; \
    memcpy((char *)&(d), (sect) + (length), (left)); \
    (length) += (left); \
    (left) = 0; \
} \
else{ \
    memcpy((char *)&(d), (sect) + (length), sizeof(double)); \
    (length) += sizeof(double); \
    (left) -= sizeof(double); \
} \
if(swapped) \
    (d) = SWAP_DOUBLE((d));

#define GET_FLOAT(sect, left, length, swapped, f) \
if((left) < sizeof(float)){ \
    (f) = 0.0; \
    memcpy((char *)&(f), (sect) + (length), (left)); \
    (length) += (left); \
    (left) = 0; \
} \
else{ \
    memcpy((char *)&(f), (sect) + (length), sizeof(float)); \
    (length) += sizeof(float); \
    (left) -= sizeof(float); \
} \
if(swapped) \
    (f) = SWAP_FLOAT((f));

#define GET_LONG(sect, left, length, swapped, l) \
if((left) < sizeof(uint32_t)){ \
    (l) = 0; \
    memcpy((char *)&(l), (sect) + (length), (left)); \
    (length) += (left); \
    (left) = 0; \
} \
else{ \
    memcpy((char *)&(l), (sect) + (length), sizeof(uint32_t)); \
    (length) += sizeof(uint32_t); \
    (left) -= sizeof(uint32_t); \
} \
if(swapped) \
    (l) = SWAP_INT((l));

#define GET_WORD(sect, left, length, swapped, w) \
if((left) < sizeof(unsigned short)){ \
    (w) = 0; \
    memcpy((char *)&(w), (sect) + (length), (left)); \
    (length) += (left); \
    (left) = 0; \
} \
else{ \
    memcpy((char *)&(w), (sect) + (length), sizeof(unsigned short)); \
    (length) += sizeof(unsigned short); \
    (left) -= sizeof(unsigned short); \
} \
if(swapped) \
    (w) = SWAP_SHORT((w));

#define	MODE(x)		(((x) >> 3) & 7)
#define	REG(x)		((x) & 7)
#define	DEST_MODE(x)	(((x) >> 6) & 7)
#define	DEST_REG(x)	(((x) >> 9) & 7)
#define B_SIZE	0
#define W_SIZE	1
#define L_SIZE	2
#define	S_SIZE	3
#define	D_SIZE	4
#define	X_SIZE	5
#define	P_SIZE	6

static char wl[] = "wl";
static char size[] = "bwl?";
static char *scales[] = { ":1", ":2", ":4", ":8" };
static char *aregs[] = { "a0", "a1", "a2", "a3", "a4", "a5", "a6", "sp" };
static char *dregs[] = { "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7" };
static char *fpregs[] = { "fp0", "fp1", "fp2", "fp3", "fp4", "fp5", "fp6",
			  "fp7" };
static char *fpcregs[] = { "fpc?", "fpi", "fps", "fps/fpi", "fpc",
			   "fpc/fpi", "fpc/fps", "fpc/fps/fpi" }; 
static char *branches[] = { "bra", "bsr", "bhi", "bls", "bcc", "bcs", "bne",
			    "beq", "bvc", "bvs", "bpl", "bmi", "bge", "blt",
			    "bgt", "ble" };
static char *conditions[] = { "t", "f", "hi", "ls", "cc", "cs", "ne",
			    "eq", "vc", "vs", "pl", "mi", "ge", "lt",
			    "gt", "le" };
static char *fpops[] = { "fmove", "fint", "fsinh", "fintrz", "fsqrt",
	"f0x05", "flognp1", "f0x07", "fetoxm1", "ftanh", "fatan",
	"f0x0b", "fasin", "fatanh", "fsin", "ftan", "fetox", "ftwotox",
	"ftentox", "f0x13", "flogn", "flog10", "flog2", "f0x17", "fabs",
	"fcosh", "fneg", "f0x1b", "facos", "fcos", "fgetexp", "fgetman",
	"fdiv", "fmod", "fadd", "fmul", "fsgldiv", "frem", "fscale", "fsglmul",
	"fsub", "f0x29", "f0x2a", "f0x2b", "f0x2c", "f0x2d",
	"f0x2e", "f0x2f", "fsincos", "fsincos", "fsincos", "fsincos",
	"fsincos", "fsincos", "fsincos", "fsincos", "fcmp", "f0x39", "ftst",
	"f0x3b", "f0x3c", "f0x3d", "f0x3e", "f0x3f",
	"fsmove","fssqrt","f0x42", "f0x43", "fdmove","fdsqrt","f0x46", "f0x47",
	"f0x48", "f0x49", "f0x4a", "f0x4b", "f0x4c", "f0x4d", "f0x4e", "f0x4f",
	"f0x50", "f0x51", "f0x52", "f0x53", "f0x54", "f0x55", "f0x56", "f0x57",
	"fsabs", "f0x59", "fsneg", "f0x5b", "fdabs", "f0x5d", "fdneg", "f0x5f",
	"fsdiv", "f0x61", "fsadd", "fsmul", "fddiv", "f0x65", "fdadd", "fdmul",
	"fssub", "f0x69", "f0x6a", "f0x6b", "fdsub", "f0x6d", "f0x6e", "f0x6f",
	"f0x70", "f0x71", "f0x72", "f0x73", "f0x74", "f0x75", "f0x76", "f0x77",
	"f0x78", "f0x79", "f0x7a", "f0x7b", "f0x7c", "f0x7d", "f0x7e", "f0x7f",
};

static char fpformat[] = "lsxpwdbp";
static uint32_t fpsize[] = { L_SIZE, S_SIZE, X_SIZE, P_SIZE, W_SIZE,
			     D_SIZE, B_SIZE, P_SIZE };
static char *fpcond[] = { "f", "eq", "ogt", "oge", "olt", "ole", "ogl", "or",
	"un", "ueq", "ugt", "uge", "ult", "ule", "ne", "t", "sf", "seq", "gt",
	"ge", "lt", "le", "gl", "gle", "ngle", "ngl", "nle", "nlt", "nge",
	"ngt", "sne", "st" };
static char scope[] = "lpa";
static char *cache[] = { "dc", "ic", "bc" };

#define PRINT_SYMBOL(sect, addr) \
	print_symbol((sect), (addr) - sect_addr, 0, sorted_relocs, \
		nsorted_relocs, symbols, NULL, nsymbols, sorted_symbols, \
		nsorted_symbols, strings, strings_size, verbose)

#define PRINT_SYMBOL_DOT(sect, addr, dot) \
	print_symbol((sect), (addr) - sect_addr, dot, sorted_relocs, \
		nsorted_relocs, symbols, NULL, nsymbols, sorted_symbols, \
		nsorted_symbols, strings, strings_size, verbose)

#define PRINT_EF(mode, reg, sect, size) \
	print_ef((mode), (reg), (sect), addr + length, sect_addr, &left, \
		(size), sorted_relocs, nsorted_relocs, symbols, nsymbols, \
		sorted_symbols, nsorted_symbols, strings, strings_size, \
		verbose, swapped)

static uint32_t print_ef(
    uint32_t mode,
    uint32_t reg,
    char *sect,
    uint32_t addr,
    uint32_t sect_addr,
    uint32_t *left,
    uint32_t size,
    struct relocation_info *sorted_relocs,
    uint32_t nsorted_relocs,
    struct nlist *symbols,
    uint32_t nsymbols,
    struct symbol *sorted_symbols,
    uint32_t nsorted_symbols,
    char *strings,
    uint32_t strings_size,
    enum bool verbose,
    enum bool swapped);


uint32_t
m68k_disassemble(
char *sect,
uint32_t left,
uint32_t addr,
uint32_t sect_addr,
enum byte_sex object_byte_sex,
struct relocation_info *sorted_relocs,
uint32_t nsorted_relocs,
struct nlist *symbols,
uint32_t nsymbols,
struct symbol *sorted_symbols,
uint32_t nsorted_symbols,
char *strings,
uint32_t strings_size,
uint32_t *indirect_symbols,
uint32_t nindirect_symbols,
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum bool verbose)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    uint32_t length, i, l;
    unsigned short opword, specop1, specop2, w;
    char *reg1, *reg2;
    const char *indirect_symbol_name;

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != object_byte_sex;

	if(left < sizeof(unsigned short)){
	   if(left != 0){
		memcpy(&opword, sect, left);
		if(swapped)
		    opword = SWAP_SHORT(opword);
		printf(".short\t0x%04x\n", (unsigned int)opword);
	   }
	   printf("(end of section)\n");
	   return(left);
	}

	memcpy(&opword, sect, sizeof(unsigned short));
	if(swapped)
	    opword = SWAP_SHORT(opword);
	length = sizeof(unsigned short);

	switch((opword & 0xf000) >> 12){
	case 0x0:
	    if(opword == 0x003c){
		GET_WORD(sect, left, length, swapped, w);
		printf("orb\t#0x%x,cc\n", (unsigned int)(w & 0xff));
		return(length);
	    }
	    if(opword == 0x007c){
		GET_WORD(sect, left, length, swapped, w);
		printf("orw\t#0x%x,sr\n", (unsigned int)w);
		return(length);
	    }
	    if((opword & 0xfff0) == 0x06c0){
		if(opword & 0x0008)
		    printf("rtm\t%s\n", aregs[opword & 0x7]);
		else
		    printf("rtm\t%s\n", dregs[opword & 0x7]);
		return(length);
	    }
	    if((opword & 0xffc0) == 0x06c0){
		GET_WORD(sect, left, length, swapped, specop1);
		printf("callm\t#0x%x,", (unsigned int)(specop1 & 0x00ff));
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, W_SIZE);
		printf("\n");
		return(length);
	    }
	    if((opword & 0xf900) == 0){
		if((opword & 0x00c0) == 0x00c0){
		    GET_WORD(sect, left, length, swapped, specop1);
		    if((specop1 & 0x0800) == 0x0800)
			printf("chk2%c\t", size[((opword & 0x0600) >> 9)]);
		    else
			printf("cmp2%c\t", size[((opword & 0x0600) >> 9)]);
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, (opword & 0x0600) >> 9);
		    if(((specop1 & 0x8000) >> 15) == 0)
			reg1 = dregs[(specop1 & 0x7000) >> 12];
		    else
			reg1 = aregs[(specop1 & 0x7000) >> 12];
		    printf(",%s\n", reg1);
		    return(length);
		}
	    }
	    if((opword & 0xff00) == 0){
		switch((opword & 0x00c0) >> 6){
		case 0:
		    GET_WORD(sect, left, length, swapped, w);
		    printf("orb\t#0x%x,", (unsigned int)(w & 0xff));
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, B_SIZE);
		    printf("\n");
		    return(length);
		case 1:
		    GET_WORD(sect, left, length, swapped, w);
		    printf("orw\t#0x%x,", (unsigned int)w);
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, W_SIZE);
		    printf("\n");
		    return(length);
		case 2:
		    GET_LONG(sect, left, length, swapped, l);
		    printf("orl\t#0x%x,", (unsigned int)l);
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, L_SIZE);
		    printf("\n");
		    return(length);
		}
	    }
	    if((opword & 0xf100) == 0x0100){
		if((opword & 0xf038) == 0x0008){
		    switch((opword & 0x00c0) >> 6){
		    case 0:
			GET_WORD(sect, left, length, swapped, w);
			printf("movepw\t(0x%x,a%d),d%d\n", (unsigned int)w,
			       REG(opword), (opword & 0x0e00) >> 9);
			break;
		    case 1:
			GET_WORD(sect, left, length, swapped, w);
			printf("movepl\t(0x%x,a%d),d%d\n", (unsigned int)w,
			       REG(opword), (opword & 0x0e00) >> 9);
			break;
		    case 2:
			GET_WORD(sect, left, length, swapped, w);
			printf("movepw\td%d,(0x%x,a%d)\n",
			       (opword & 0x0e00) >> 9, (unsigned int)w,
			       REG(opword));
			break;
		    case 3:
			GET_WORD(sect, left, length, swapped, w);
			printf("movepl\td%d,(0x%x,a%d)\n",
			       (opword & 0x0e00) >> 9, (unsigned int)w,
			       REG(opword));
			break;
		    }
		    return(length);
		}
		else{
		    switch((opword & 0x00c0) >> 6){
		    case 0:
			printf("btst\td%d,", (opword & 0x0e00) >> 9);
			break;
		    case 1:
			printf("bchg\td%d,", (opword & 0x0e00) >> 9);
			break;
		    case 2:
			printf("bclr\td%d,", (opword & 0x0e00) >> 9);
			break;
		    case 3:
			printf("bset\td%d,", (opword & 0x0e00) >> 9);
			break;
		    }
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, L_SIZE);
		    printf("\n");
		    return(length);
		}
	    }
	    if((opword & 0xff00) == 0x0200){
		if(opword == 0x023c){
		    GET_WORD(sect, left, length, swapped, w);
		    printf("andb\t#0x%x,cc\n", (unsigned int)(w & 0xff));
		    return(length);
		}
		if(opword == 0x027c){
		    GET_WORD(sect, left, length, swapped, w);
		    printf("andw\t#0x%x,sr\n", (unsigned int)w);
		    return(length);
		}
		switch((opword & 0x00c0) >> 6){
		case 0:
		    GET_WORD(sect, left, length, swapped, w);
		    printf("andb\t#0x%x,", (unsigned int)(w & 0xff));
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, B_SIZE);
		    break;
		case 1:
		    GET_WORD(sect, left, length, swapped, w);
		    printf("andw\t#0x%x,", (unsigned int)w);
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, W_SIZE);
		    break;
		case 2:
		    GET_LONG(sect, left, length, swapped, l);
		    printf("andl\t#0x%x,", (unsigned int)l);
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, L_SIZE);
		    break;
		default:
		    goto bad;
		}
		printf("\n");
		return(length);
	    }
	    if((opword & 0xff00) == 0x0400){
		switch((opword & 0x00c0) >> 6){
		case 0:
		    GET_WORD(sect, left, length, swapped, w);
		    printf("subb\t#0x%x,", (unsigned int)(w & 0xff));
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, B_SIZE);
		    break;
		case 1:
		    GET_WORD(sect, left, length, swapped, w);
		    printf("subw\t#0x%x,", (unsigned int)w);
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, W_SIZE);
		    break;
		case 2:
		    GET_LONG(sect, left, length, swapped, l);
		    printf("subl\t");
		    if(PRINT_SYMBOL(l, addr + length - sizeof(uint32_t)))
			printf(",");
		    else
			printf("#0x%x,", (unsigned int)l);
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, L_SIZE);
		    break;
		default:
		    goto bad;
		}
		printf("\n");
		return(length);
	    }
	    if((opword & 0xff00) == 0x0600){
		switch((opword & 0x00c0) >> 6){
		case 0:
		    GET_WORD(sect, left, length, swapped, w);
		    printf("addb\t#0x%x,", (unsigned int)(w &0xff));
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, B_SIZE);
		    break;
		case 1:
		    GET_WORD(sect, left, length, swapped, w);
		    printf("addw\t#0x%x,",  (unsigned int)w);
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, W_SIZE);
		    break;
		case 2:
		    GET_LONG(sect, left, length, swapped, l);
		    printf("addl\t#0x%x,", (unsigned int)l);
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, L_SIZE);
		    break;
		default:
		    goto bad;
		}
		printf("\n");
		return(length);
	    }
	    if(opword == 0x0a3c){
		GET_WORD(sect, left, length, swapped, w);
		printf("eorb\t#0x%x,cc\n", (unsigned int)(w & 0xff));
		return(length);
	    }
	    if(opword == 0x0a7c){
		GET_WORD(sect, left, length, swapped, w);
		printf("eorw\t#0x%x,sr\n", (unsigned int)w);
		return(length);
	    }
	    if((opword & 0xff00) == 0x0800){
		switch((opword & 0x00c0) >> 6){
		case 0:
		    printf("btst\t");
		    break;
		case 1:
		    printf("bchg\t");
		    break;
		case 2:
		    printf("bclr\t");
		    break;
		case 3:
		    printf("bset\t");
		    break;
		}
		GET_WORD(sect, left, length, swapped, w);
		printf("#%d,", w & 0xff);
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, L_SIZE);
		printf("\n");
		return(length);
	    }
	    if((opword & 0xf9ff) == 0x08fc){
		GET_WORD(sect, left, length, swapped, specop1);
		if((specop1 & 0x8000) == 0)
		    reg1 = dregs[(specop1 & 0x7000) >> 12];
		else
		    reg1 = aregs[(specop1 & 0x7000) >> 12];

		GET_WORD(sect, left, length, swapped, specop2);
		if((specop2 & 0x8000) == 0)
		    reg2 = dregs[(specop2 & 0x7000) >> 12];
		else
		    reg2 = aregs[(specop2 & 0x7000) >> 12];
#ifdef MOTO_SYNTAX
		printf("cas2%c\td%d:d%d,d%d:d%d,(%s):(%s)\n",
#else
		printf("cas2%c\td%d,d%d,d%d,d%d,%s,%s\n",
#endif
			size[((opword & 0x0600) >> 9) - 1],
			specop1 & 7, specop2 & 7,
			(specop1 & 0x01c0) >> 6, (specop2 & 0x01c0) >> 6,
			reg1, reg2);
		return(length);
	    }
	    if((opword & 0xf9c0) == 0x08c0){
		GET_WORD(sect, left, length, swapped, specop1);
		printf("cas%c\td%d,d%d,", size[((opword & 0x0600) >> 9) - 1],
			specop1 & 7, (specop1 & 0x01c0) >> 6);
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, (opword & 0x0600) >> 9);
		printf("\n");
		return(length);
	    }
	    if((opword & 0xff00) == 0x0a00){
		switch((opword & 0x00c0) >> 6){
		case 0:
		    GET_WORD(sect, left, length, swapped, w);
		    printf("eorb\t#0x%x,", (unsigned int)(w & 0xff));
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, B_SIZE);
		    break;
		case 1:
		    GET_WORD(sect, left, length, swapped, w);
		    printf("eorw\t#0x%x,", (unsigned int)w);
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, W_SIZE);
		    break;
		case 2:
		    GET_LONG(sect, left, length, swapped, l);
		    printf("eorl\t#0x%x,", (unsigned int)l);
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, L_SIZE);
		    break;
		default:
		    goto bad;
		}
		printf("\n");
		return(length);
	    }
	    if((opword & 0xff00) == 0x0c00){
		switch((opword & 0x00c0) >> 6){
		case 0:
		    GET_WORD(sect, left, length, swapped, w);
		    printf("cmpb\t#0x%x,", (unsigned int)(w & 0xff));
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, B_SIZE);
		    break;
		case 1:
		    GET_WORD(sect, left, length, swapped, w);
		    printf("cmpw\t#0x%x,", (unsigned int)w);
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, W_SIZE);
		    break;
		case 2:
		    printf("cmpl\t#");
		    GET_LONG(sect, left, length, swapped, l);
		    if(PRINT_SYMBOL(l, addr + length - sizeof(uint32_t)))
			printf(",");
		    else
			printf("0x%x,", (unsigned int)l);
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, L_SIZE);
		    break;
		default:
		    goto bad;
		}
		printf("\n");
		return(length);
	    }
	    if((opword & 0xff00) == 0x0e00){
		printf("moves%c\t", size[(opword & 0x00c0) >> 6] );
		GET_WORD(sect, left, length, swapped, specop1);
		if((specop1 & 0x8000) == 0)
		    reg1 = dregs[(specop1 & 0x7000) >> 12];
		else
		    reg1 = aregs[(specop1 & 0x7000) >> 12];
		if((specop1 & 0x0800) == 0){
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, (opword & 0x00c0) >> 6);
		    printf(",%s\n", reg1);
		}
		else{
		    printf("%s,", reg1);
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, (opword & 0x00c0) >> 6);
		    printf("\n");
		}
		return(length);
	    }
	    break;
	case 0x1:
	    printf("moveb\t");
	    length += PRINT_EF(MODE(opword), REG(opword),
			       sect + length, B_SIZE);
	    printf(",");
	    length += PRINT_EF(DEST_MODE(opword), DEST_REG(opword),
			       sect + length, B_SIZE);
	    printf("\n");
	    return(length);
	case 0x2:
	    if((opword & 0x01c0) == 0x0040){
		printf("movel\t");
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, L_SIZE);
		printf(",%s\n", aregs[(opword >> 9) & 0x7]);
		return(length);
	    }
	    printf("movel\t");
	    length += PRINT_EF(MODE(opword), REG(opword),
			       sect + length, L_SIZE);
	    printf(",");
	    length += PRINT_EF(DEST_MODE(opword), DEST_REG(opword),
			       sect + length, L_SIZE);
	    printf("\n");
	    return(length);
	case 0x3:
	    if((opword & 0x01c0) == 0x0040){
		printf("movew\t");
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, W_SIZE);
		printf(",%s\n", aregs[(opword >> 9) & 0x7]);
		return(length);
	    }
	    printf("movew\t");
	    length += PRINT_EF(MODE(opword), REG(opword),
			       sect + length, W_SIZE);
	    printf(",");
	    length += PRINT_EF(DEST_MODE(opword), DEST_REG(opword),
			       sect + length, W_SIZE);
	    printf("\n");
	    return(length);
	case 0x4:
	    if((opword & 0xff00) == 0x4000){
		if((opword & 0xffc0) == 0x40c0){
		    printf("movew\tsr,");
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, W_SIZE);
		    printf("\n");
		    return(length);
		}
		printf("negx%c\t", size[(opword >> 6) & 0x3]);
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, W_SIZE);
		printf("\n");
		return(length);
	    }
	    if((opword & 0xf140) == 0x4100){
		if(opword & 0x0080){
		    printf("chkw\t");
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, W_SIZE);
		}
		else{
		    printf("chkl\t");
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, L_SIZE);
		}
		printf(",%s\n", dregs[(opword >> 9) & 0x7]);
		return(length);
	    }
	    if((opword & 0xfe38) == 0x4800){
		switch((opword >> 6) & 0x7){
		case 0x2:
		    printf("extw\t%s\n", dregs[REG(opword)]);
		    return(length);
		case 0x3:
		    printf("extl\t%s\n", dregs[REG(opword)]);
		    return(length);
		case 0x7:
		    printf("extbl\t%s\n", dregs[REG(opword)]);
		    return(length);
		}
	    }
	    if((opword & 0xf1c0) == 0x41c0){
		printf("lea\t");
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, W_SIZE);
		printf(",a%d\n", (opword & 0x0e00)>>9);
		return(length);
	    }
	    if((opword & 0xff00) == 0x4200){
		if((opword & 0xffc0) == 0x42c0){
		    printf("movew\tcc,");
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, W_SIZE);
		    printf("\n");
		    return(length);
		}
		printf("clr%c\t", size[(opword >> 6) & 0x3]);
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, W_SIZE);
		printf("\n");
		return(length);
	    }
	    if((opword & 0xff00) == 0x4400){
		if((opword & 0xffc0) == 0x44c0){
		    printf("movew\t");
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, W_SIZE);
		    printf(",cc\n");
		    return(length);
		}
		printf("neg%c\t", size[(opword >> 6) & 0x3]);
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, W_SIZE);
		printf("\n");
		return(length);
	    }
	    if((opword & 0xff00) == 0x4600){
		if((opword & 0xffc0) == 0x46c0){
		    printf("movew\t");
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, W_SIZE);
		    printf(",sr\n");
		    return(length);
		}
		printf("not%c\t", size[(opword >> 6) & 0x3]);
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, W_SIZE);
		printf("\n");
		return(length);
	    }
	    if((opword & 0xfff8) == 0x4808){
		GET_LONG(sect, left, length, swapped, l);
		printf("linkl\t%s,#0x%x\n", aregs[REG(opword)],(unsigned int)l);
		return(length);
	    }
	    if((opword & 0xffc0) == 0x4800){
		printf("nbcd\t");
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, W_SIZE);
		printf("\n");
		return(length);
	    }
	    if((opword & 0xfff8) == 0x4840){
		printf("swap\t%s\n", dregs[REG(opword)]);
		return(length);
	    }
	    if((opword & 0xfff8) == 0x4848){
		printf("bkpt\t#%d\n", opword & 0x7);
		return(length);
	    }
	    if((opword & 0xffc0) == 0x4840){
		printf("pea\t");
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, W_SIZE);
		printf("\n");
		return(length);
	    }
	    if((opword & 0xfb80) == 0x4880){
		printf("movem%c\t", size[((opword & 0x0040) >> 6) + 1]);
		GET_WORD(sect, left, length, swapped, specop1);
		if((opword & 0x0400) == 0x0000){
		    if(specop1 == 0){
			printf("#0x0");
		    }
		    else{
			if(MODE(opword) == 4){
			    for(i = 0; i < 8; i++){
				if((specop1 & 0x8000) != 0){
				    printf("%s", dregs[i]);
				    if(((specop1 << 1) & 0xffff) != 0)
					printf("/");
				}
				specop1 <<= 1;
			    }
			    for(i = 0; i < 8; i++){
				if((specop1 & 0x8000) != 0){
				    printf("%s", aregs[i]);
				    if(((specop1 << 1) & 0xffff) != 0)
					printf("/");
				}
				specop1 <<= 1;
			    }
		        }
		        else{
			    for(i = 0; i < 8; i++){
				if((specop1 & 1) != 0){
				    printf("%s", dregs[i]);
				    if((specop1 >> 1) != 0)
					printf("/");
				}
				specop1 >>= 1;
			    }
			    for(i = 0; i < 8; i++){
				if((specop1 & 1) != 0){
				    printf("%s", aregs[i]);
				    if((specop1 >> 1) != 0)
					printf("/");
				}
				specop1 >>= 1;
			    }
		        }
		    }
		    printf(",");
		    length += PRINT_EF(MODE(opword), REG(opword), sect + length,
				       ((opword & 0x0040) >> 6) + 1);
		}
		else{
		    length += PRINT_EF(MODE(opword), REG(opword), sect + length,
				       ((opword & 0x0040) >> 6) + 1);
		    printf(",");
		    if(specop1 == 0){
			printf("#0x0");
		    }
		    else{
			if(MODE(opword) == 4){
			    for(i = 0; i < 8; i++){
				if((specop1 & 0x8000) != 0){
				    printf("%s", dregs[i]);
				    if(((specop1 << 1) & 0xffff) != 0)
					printf("/");
				}
				specop1 <<= 1;
			    }
			    for(i = 0; i < 8; i++){
				if((specop1 & 0x8000) != 0){
				    printf("%s", aregs[i]);
				    if(((specop1 << 1) & 0xffff) != 0)
					printf("/");
				}
				specop1 <<= 1;
			    }
		        }
		        else{
			    for(i = 0; i < 8; i++){
				if((specop1 & 1) != 0){
				    printf("%s", dregs[i]);
				    if((specop1 >> 1) != 0)
					printf("/");
				}
				specop1 >>= 1;
			    }
			    for(i = 0; i < 8; i++){
				if((specop1 & 1) != 0){
				    printf("%s", aregs[i]);
				    if((specop1 >> 1) != 0)
					printf("/");
				}
				specop1 >>= 1;
			    }
		        }
		    }
		}
		printf("\n");
		return(length);
	    }
	    if(opword == 0x4afc){
		printf("illegal\n");
		return(length);
	    }
	    if((opword & 0xff00) == 0x4a00){
		if((opword & 0xffc0) == 0x4ac0){
		    printf("tas\t");
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, W_SIZE);
		    printf("\n");
		    return(length);
		}
		printf("tst%c\t", size[((opword >> 6) & 0x3)]);
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, (opword >> 6) & 0x3);
		printf("\n");
		return(length);
	    }
	    if((opword & 0xffc0) == 0x4c00){
		GET_WORD(sect, left, length, swapped, specop1);
		if(specop1 & 0x0800)
		    printf("mulsl\t");
		else
		    printf("mulul\t");
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, L_SIZE);
		if(specop1 & 0x0400)
		    printf(",%s:%s\n", dregs[specop1 & 0x7],
			   dregs[(specop1 >> 12) & 0x7]);
		else
		    printf(",%s\n", dregs[(specop1 >> 12) & 0x7]);
		return(length);
	    }
	    if((opword & 0xffc0) == 0x4c40){
		GET_WORD(sect, left, length, swapped, specop1);
		if(specop1 & 0x0800)
		    printf("divs");
		else
		    printf("divu");
		if((specop1 & 0x0400) == 0 &&
		   (specop1 & 0x7) == ((specop1 >> 12) & 0x7) ){
		    printf("l\t");
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, L_SIZE);
		    printf(",%s\n", dregs[specop1 & 0x7]);
		}
		else{
		    if(specop1 & 0x0400)
			printf("l\t");
		    else
			printf("ll\t");
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, L_SIZE);
#ifdef MOTO_SYNTAX
		    printf(",%s:%s\n", dregs[specop1 & 0x7],
#else
		    printf(",%s,%s\n", dregs[specop1 & 0x7],
#endif
			   dregs[(specop1 >> 12) & 0x7]);
		}
		return(length);
	    }
	    if((opword & 0xfff0) == 0x4e40){
		printf("trap\t#%d\n", opword &0xf);
		return(length);
	    }
	    if((opword & 0xfff8) == 0x4e50){
		GET_WORD(sect, left, length, swapped, w);
		printf("linkw\t%s,#0x%x\n", aregs[REG(opword)],(unsigned int)w);
		return(length);
	    }
	    if((opword & 0xfff8) == 0x4e58){
		printf("unlk\t%s\n", aregs[REG(opword)]);
		return(length);
	    }
	    if((opword & 0xfff8) == 0x4e60){
		printf("movel\t%s,usp\n", aregs[REG(opword)]);
		return(length);
	    }
	    if((opword & 0xfff8) == 0x4e68){
		printf("movel\tusp,%s\n", aregs[REG(opword)]);
		return(length);
	    }
	    if(opword == 0x4e70){
		printf("reset\n");
		return(length);
	    }
	    if(opword == 0x4e71){
		printf("nop\n");
		return(length);
	    }
	    if(opword == 0x4e72){
		GET_WORD(sect, left, length, swapped, w);
		printf("stop\t#0x%x\n", (unsigned int)w);
		return(length);
	    }
	    if(opword == 0x4e73){
		printf("rte\n");
		return(length);
	    }
	    if(opword == 0x4e74){
		GET_WORD(sect, left, length, swapped, w);
		printf("rtd\t#0x%x\n", (unsigned int)w);
		return(length);
	    }
	    if(opword == 0x4e75){
		printf("rts\n");
		return(length);
	    }
	    if(opword == 0x4e76){
		printf("trapv\n");
		return(length);
	    }
	    if(opword == 0x4e77){
		printf("rtr\n");
		return(length);
	    }
	    if((opword & 0xfffe) == 0x4e7a){
		GET_WORD(sect, left, length, swapped, specop1);
		printf("movec\t");
		if(opword & 0x1){
		    if(specop1 & 0x8000)
			printf("%s,", aregs[(specop1 >> 12) & 0x7]);
		    else
			printf("%s,", dregs[(specop1 >> 12) & 0x7]);
		    switch(specop1 & 0x0fff){
		    case 0x000:
			printf("sfc\n");
			break;
		    case 0x001:
			printf("dfc\n");
			break;
		    case 0x002:
			printf("cacr\n");
			break;
		    case 0x800:
			printf("usp\n");
			break;
		    case 0x801:
			printf("vbr\n");
			break;
		    case 0x802:
			printf("caar\n");
			break;
		    case 0x803:
			printf("msp\n");
			break;
		    case 0x804:
			printf("isp\n");
			break;
		    case 0x003:
			printf("tc\n");
			break;
		    case 0x004:
			printf("itt0\n");
			break;
		    case 0x005:
			printf("itt1\n");
			break;
		    case 0x006:
			printf("dtt0\n");
			break;
		    case 0x007:
			printf("dtt1\n");
			break;
		    case 0x805:
			printf("mmusr\n");
			break;
		    case 0x806:
			printf("urp\n");
			break;
		    case 0x807:
			printf("srp\n");
			break;
		    default:
			printf("???\n");
			break;
		    }
		}
		else{
		    switch(specop1 & 0x0fff){
		    case 0x000:
			printf("sfc,");
			break;
		    case 0x001:
			printf("dfc,");
			break;
		    case 0x002:
			printf("cacr,");
			break;
		    case 0x800:
			printf("usp,");
			break;
		    case 0x801:
			printf("vbr,");
			break;
		    case 0x802:
			printf("caar,");
			break;
		    case 0x803:
			printf("msp,");
			break;
		    case 0x804:
			printf("isp,");
			break;
		    case 0x003:
			printf("tc,");
			break;
		    case 0x004:
			printf("itt0,");
			break;
		    case 0x005:
			printf("itt1,");
			break;
		    case 0x006:
			printf("dtt0,");
			break;
		    case 0x007:
			printf("dtt1,");
			break;
		    case 0x805:
			printf("mmusr,");
			break;
		    case 0x806:
			printf("urp,");
			break;
		    case 0x807:
			printf("srp,");
			break;
		    default:
			printf("???,");
			break;
		    }
		    if(specop1 & 0x8000)
			printf("%s\n", aregs[(specop1 >> 12) & 0x7]);
		    else
			printf("%s\n", dregs[(specop1 >> 12) & 0x7]);
		}
		return(length);
	    }
	    if((opword & 0xffc0) == 0x4e80){
		printf("jsr\t");
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, L_SIZE);
		printf("\n");
		return(length);
	    }
	    if((opword & 0xffc0) == 0x4ec0){
		printf("jmp\t");
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, W_SIZE);
		printf("\n");
		return(length);
	    }
	    goto bad;
	case 0x5:
	    if((opword & 0xf0c0) == 0x50c0){
		if((opword & 0x00f8) == 0x00f8){
		    switch(opword & 0x0007){
		    case 2:
			GET_WORD(sect, left, length, swapped, w);
			printf("trap%s.w\t#0x%x\n",
			       conditions[(opword & 0x0f00) >> 8],
			       (unsigned int)w);
			return(length);
		    case 3:
			GET_LONG(sect, left, length, swapped, l);
			printf("trap%s.l\t#0x%x\n",
			       conditions[(opword & 0x0f00) >> 8],
			       (unsigned int)l);
			return(length);
		    case 4:
			printf("trap%s\n", conditions[(opword & 0x0f00) >> 8]);
			return(length);
		    }
		}
		if((opword & 0x00f8) == 0x00c8){
		    printf("db%s\t%s,", conditions[(opword & 0x0f00) >> 8],
			   dregs[REG(opword)]);
		    GET_WORD(sect, left, length, swapped, w);
		    if(w & 0x8000)
			l = 0xffff0000 | w;
		    else
			l = w;
		    if(PRINT_SYMBOL(addr + length - sizeof(unsigned short) + l,
				    addr + length - sizeof(unsigned short)))
			printf("\n");
		    else
			printf("0x%x\n", (unsigned int)
			       (addr + length - sizeof(unsigned short) + l));
		    return(length);
		}
		printf("s%s\t", conditions[(opword & 0x0f00) >> 8]);
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, W_SIZE);
		printf("\n");
		return(length);
	    }
	    if(opword & 0x0100)
		printf("subq%c\t#0x%x,", size[(opword >> 6) & 0x3],
		       ((opword >> 9) & 0x7) == 0 ?
		       (unsigned int)8 : (unsigned int)((opword >> 9) & 0x7));
	    else
		printf("addq%c\t#0x%x,", size[(opword >> 6) & 0x3],
		       ((opword >> 9) & 0x7) == 0 ?
		       (unsigned int)8 : (unsigned int)((opword >> 9) & 0x7));
	    length += PRINT_EF(MODE(opword), REG(opword),
			       sect + length, W_SIZE);
	    printf("\n");
	    return(length);
	case 0x6:
	    printf("%s\t", branches[(opword & 0x0f00) >> 8]);
	    if((opword & 0x00ff) == 0x00ff){
		GET_LONG(sect, left, length, swapped, l);
		if(PRINT_SYMBOL(addr + length - sizeof(uint32_t) + l,
				addr + length - sizeof(uint32_t)))
		    printf(":l\n");
		else{
		    printf("0x%x:l", (unsigned int)
			   (addr + length - sizeof(uint32_t) + l));
		    if(verbose){
			indirect_symbol_name = guess_indirect_symbol(
			   (addr + length - sizeof(uint32_t) + l),
			    ncmds, sizeofcmds, load_commands, object_byte_sex,
			    indirect_symbols, nindirect_symbols, symbols, NULL,
			    nsymbols, strings, strings_size);
			if(indirect_symbol_name != NULL)
			    printf("\t; symbol stub for: %s",
				indirect_symbol_name);
		    }
		    printf("\n");
		}
	    }
	    else if((opword & 0x00ff) == 0){
		GET_WORD(sect, left, length, swapped, w);
		if(w & 0x8000)
		    l = 0xffff0000 | w;
		else
		    l = w;
		if(PRINT_SYMBOL(addr + length - sizeof(unsigned short) + l,
				addr + length - sizeof(unsigned short)))
		    printf(":w\n");
		else
		    printf("0x%x:w\n", (unsigned int)
			   (addr + length - sizeof(unsigned short) + l));
	    }
	    else{
		l = (char)opword;
		if(PRINT_SYMBOL(addr + length + l, addr + 1))
		    printf(":b\n");
		else
		    printf("0x%x:b\n", (unsigned int)
			   (addr + length + (int32_t)((char)(opword))));
	    }
	    return(length);
	case 0x7:
	    printf("moveq\t#%u,%s\n", (int32_t)((char)(opword)),
		   dregs[(opword >> 9) & 0x7]);
	    return(length);
	case 0x8:
	    if((opword & 0xf1f0) == 0x8100){
		if(opword & 0x0008)
		    printf("sbcd\t%s@-,%s@-\n", aregs[opword & 0x7],
			   aregs[(opword >> 9) & 0x7]);
		else
		    printf("sbcd\t%s,%s\n", dregs[opword & 0x7],
			   dregs[(opword >> 9) & 0x7]);
		return(length);
	    }
	    if((opword & 0xf1f0) == 0x8140){
		GET_WORD(sect, left, length, swapped, w);
		if(opword & 0x0008){
		    printf("pack\t%s@-,%s@-,#0x%x\n", aregs[opword & 0x7],
			   aregs[(opword >> 9) & 0x7], (unsigned int)w);
		}
		else{
		    printf("pack\t%s,%s,#0x%x\n", dregs[opword & 0x7],
			   dregs[(opword >> 9) & 0x7], (unsigned int)w);
		}
		return(length);
	    }
	    if((opword & 0xf1f0) == 0x8180){
		GET_WORD(sect, left, length, swapped, w);
		if(opword & 0x0008){
		    printf("unpk\t%s@-,%s@-,#0x%x\n", aregs[opword & 0x7],
			   aregs[(opword >> 9) & 0x7], (unsigned int)w);
		}
		else{
		    printf("unpk\t%s,%s,#0x%x\n", dregs[opword & 0x7],
			   dregs[(opword >> 9) & 0x7], (unsigned int)w);
		}
		return(length);
	    }
	    if((opword & 0xf0c0) == 0x80c0){
		if(opword & 0x0100)
		    printf("divs\t");
		else
		    printf("divu\t");
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, W_SIZE);
		printf(",%s\n", dregs[(opword >> 9) & 0x7]);
		return(length);
	    }
	    if(opword & 0x0100){
		printf("or%c\t%s,", size[(opword >> 6) & 0x3],
		       dregs[(opword >> 9) & 0x7]);
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, L_SIZE);
		printf("\n");
		return(length);
	    }
	    else{
		printf("or%c\t", size[(opword >> 6) & 0x3]);
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, L_SIZE);
		printf(",%s\n", dregs[(opword >> 9) & 0x7]);
		return(length);
	    }
	case 0xa:
	    break;
	case 0xb:
	    if((opword & 0xf138) == 0xb108 && ((opword >> 6) & 0x7) != 0x7){
		printf("cmpm%c\t%s@+,%s@+\n", size[(opword >> 6) & 0x3],
		       aregs[opword & 0x7], aregs[(opword >> 9) & 0x7]);
		return(length);
	    }
	    switch((opword >> 6) & 0x7){
	    case 0:
	    case 1:
	    case 2:
		printf("cmp%c\t", size[(opword >> 6) & 0x3]);
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, (opword >> 6) & 0x3);
		printf(",%s\n", dregs[(opword >> 9) & 0x7]);
		return(length);

	    case 3:
		printf("cmpw\t");
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, W_SIZE);
		printf(",%s\n", aregs[(opword >> 9) & 0x7]);
		return(length);
	    case 7:
		printf("cmpl\t");
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, L_SIZE);
		printf(",%s\n", aregs[(opword >> 9) & 0x7]);
		return(length);

	    case 4:
	    case 5:
	    case 6:
		printf("eor%c\t%s,", size[(opword >> 6) & 0x3],
		       dregs[(opword >> 9) & 0x7]);
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, (opword >> 6) & 0x3);
		printf("\n");
		return(length);
	    }
	    break;
	case 0xc:
	    if((opword & 0xf1f8) == 0xc140){
		printf("exg\t%s,%s\n", dregs[(opword >> 9) & 0x7],
		       dregs[opword & 0x7]);
		return(length);
	    }
	    if((opword & 0xf1f8) == 0xc148){
		printf("exg\t%s,%s\n", aregs[(opword >> 9) & 0x7],
		       aregs[opword & 0x7]);
		return(length);
	    }
	    if((opword & 0xf1f8) == 0xc188){
		printf("exg\t%s,%s\n", dregs[(opword >> 9) & 0x7],
		       aregs[opword & 0x7]);
		return(length);
	    }
	    if((opword & 0xf1f0) == 0xc100){
		if(opword & 0x0008){
		    printf("abcd\t%s@-,%s@-\n", aregs[opword & 0x7],
			   aregs[(opword >> 9) & 0x7]);
		}
		else{
		    printf("abcd\t%s,%s\n", dregs[opword & 0x7],
			   dregs[(opword >> 9) & 0x7]);
		}
		return(length);
	    }
	    if((opword & 0xf0c0) == 0xc0c0){
		if(opword & 0x0100)
		    printf("muls\t");
		else
		    printf("mulu\t");
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, W_SIZE);
		printf(",%s\n", dregs[(opword >> 9) & 0x7]);
		return(length);
	    }
	    switch((opword >> 6) & 0x7){
	    case 0:
	    case 1:
	    case 2:
		printf("and%c\t", size[(opword >> 6) & 0x3]);
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, (opword >> 6) & 0x3);
		printf(",%s\n", dregs[(opword >> 9) & 0x7]);
		return(length);
	    case 4:
	    case 5:
	    case 6:
		printf("and%c\t%s,", size[(opword >> 6) & 0x3],
		       dregs[(opword >> 9) & 0x7]);
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, (opword >> 6) & 0x3);
		printf("\n");
		return(length);
	    }
	    break;
	case 0x9:
	case 0xd:
	    if(opword & 0x4000)
		printf("add");
	    else
		printf("sub");
	    switch((opword >> 6) & 0x7){
	    case 0:
	    case 1:
	    case 2:
		printf("%c\t", size[(opword >> 6) & 0x3]);
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, (opword >> 6) & 0x3);
		printf(",%s\n", dregs[(opword >> 9) & 0x7]);
		return(length);
	    case 4:
	    case 5:
	    case 6:
		if((opword & 0x0030) == 0x0000){
		    if((opword & 0x0008) == 0){
			printf("x%c\t%s,%s\n", size[(opword >> 6) & 0x3],
			       dregs[opword & 0x7], dregs[(opword >> 9) & 0x7]);
		    }
		    else{
			printf("x%c\t%s@-,%s@-\n", size[(opword >> 6) & 0x3],
			       aregs[opword & 0x7], aregs[(opword >> 9) & 0x7]);
		    }
		    return(length);
		}
		printf("%c\t%s,", size[(opword >> 6) & 0x3],
		       dregs[(opword >> 9) & 0x7]);
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, (opword >> 6) & 0x3);
		printf("\n");
		return(length);
	    case 3:
	    case 7:
		printf("a%c\t", size[((opword >> 8) & 0x1) + 1]);
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, ((opword >> 8) & 0x1) + 1);
		printf(",%s\n", aregs[(opword >> 9) & 0x7]);
		return(length);
	    }
	case 0xe:
	    if((opword & 0xf8c0) == 0xe8c0){
		GET_WORD(sect, left, length, swapped, specop1);
		switch((opword >> 8) & 0x7){
		case 0:
		    printf("bftst\t");
		    break;
		case 1:
		    printf("bfextu\t");
		    break;
		case 2:
		    printf("bfchg\t");
		    break;
		case 3:
		    printf("bfexts\t");
		    break;
		case 4:
		    printf("bfclr\t");
		    break;
		case 5:
		    printf("bfffo\t");
		    break;
		case 6:
		    printf("bfset\t");
		    break;
		case 7:
		    printf("bfins\t%s,", dregs[(specop1 >> 12) & 0x7]);
		    break;
		}
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, ((opword & 0x0100) >> 8) + 1);
		if(specop1 & 0x0800)
		    printf("{%s,", dregs[(specop1 >> 6) & 0x7]);
		else
		    printf("{#%d,", (specop1 >> 6) & 0x1f);
		if(specop1 & 0x0020)
		    printf("%s}", dregs[specop1 & 0x7]);
		else
		    printf("#%d}", specop1 & 0x1f);
		if((opword & 0x0100) && (opword & 0x0700) != 0x0700)
		    printf(",%s\n", dregs[(specop1 >> 12) & 0x7]);
		else
		    printf("\n");
		return(length);
	    }
	    if((opword & 0xf8c0) == 0xe0c0){
		switch((opword >> 9) & 0x3){
		case 0:
		    printf("as");
		    break;
		case 1:
		    printf("ls");
		    break;
		case 2:
		    printf("rox");
		    break;
		case 3:
		    printf("ro");
		    break;
		}
		if(opword & 0x0100)
		    printf("lw\t");
		else
		    printf("rw\t");
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, ((opword & 0x0100) >> 8) + 1);
		printf("\n");
		return(length);
	    }
	    switch((opword >> 3) & 0x3){
	    case 0:
		printf("as");
		break;
	    case 1:
		printf("ls");
		break;
	    case 2:
		printf("rox");
		break;
	    case 3:
		printf("ro");
		break;
	    }
	    if(opword & 0x0100)
		printf("l%c\t", size[(opword >> 6) & 0x3]);
	    else
		printf("r%c\t", size[(opword >> 6) & 0x3]);
	    if(opword & 0x0020)
		printf("%s,%s\n", dregs[(opword >> 9) & 0x7],
		       dregs[opword & 0x7]);
	    else
		printf("#%d,%s\n",
		       ((opword >> 9) & 0x7) == 0 ? 8 : (opword >> 9) & 0x7,
		       dregs[opword & 0x7]);
	    return(length);
	case 0xf:
	    if((opword & 0x0e00) == 0x0000){
		GET_WORD(sect, left, length, swapped, specop1);
		switch((specop1 >> 13) & 0x7){
		case 0:
		    if(specop1 & 0x0200){
			if(((specop1 >> 10) & 0x7) == 0x2){
			    if(specop1 & 0x0100){
				printf("pmovefd\ttt0,");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf("\n");
				return(length);
			    }
			    else{
				printf("pmove\ttt0,");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf("\n");
				return(length);
			    }
			}
			else if(((specop1 >> 10) & 0x7) == 0x3){
			    if(specop1 & 0x0100){
				printf("pmovefd\ttt1,");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf("\n");
				return(length);
			    }
			    else{
				printf("pmove\ttt1,");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf("\n");
				return(length);
			    }
			} else
			    goto bad;
		     }
		     else{
			if(((specop1 >> 10) & 0x7) == 0x2){
			    if(specop1 & 0x0100){
				printf("pmovefd\t");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf(",tt0\n");
				return(length);
			    }
			    else{
				printf("pmove\t");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf(",tt0\n");
				return(length);
			    }
			}
			else if(((specop1 >> 10) & 0x7) == 0x3){
			    if(specop1 & 0x0100){
				printf("pmovefd\t");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf(",tt1\n");
				return(length);
			    }
			    else{
				printf("pmove\t");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf(",tt1\n");
				return(length);
			    }
			} else
			    goto bad;
		     }
		case 1:
		    if((specop1 & 0xfde0) == 0x2000){
			if(specop1 & 0x0200)
			    printf("ploadr\t");
			else
			    printf("ploadw\t");
			if((specop1 & 0x18) == 0x10)
			    printf("#0x%x,", (unsigned int)(specop1 & 0x7));
			else if((specop1 & 0x18) == 0x08)
			    printf("%s,", dregs[specop1 & 0x7]);
			else if((specop1 & 0x1f) == 0x00)
			    printf("sfc,");
			else if((specop1 & 0x1f) == 0x01)
			    printf("dfc,");
			else
			    goto bad;
			length += PRINT_EF(MODE(opword), REG(opword),
					   sect + length, L_SIZE);
			printf("\n");
			return(length);
		    }
		    else if((specop1 & 0xe300) == 0x2000){
			if(((specop1 >> 10) & 0x7) == 0x1){
			    printf("pflusha030\n");
			    return(length);
			}
			else if(((specop1 >> 10) & 0x7) == 0x4){
			    if((specop1 & 0x18) == 0x10)
				printf("pflush\t#0x%x,#0x%x\n",
				       (unsigned int)(specop1 & 0x7), 
				       (unsigned int)((specop1 >> 5) & 0x7));
			    else if((specop1 & 0x18) == 0x08)
				printf("pflush\t%s,#0x%x\n",
				       dregs[specop1 & 0x7], 
				       (unsigned int)((specop1 >> 5) & 0x7));
			    else if((specop1 & 0x1f) == 0x00)
				printf("pflush\tsfc,#0x%x\n",
				       (unsigned int)((specop1 >> 5) & 0x7));
			    else if((specop1 & 0x1f) == 0x01)
				printf("pflush\tdfc,#0x%x\n",
				       (unsigned int)((specop1 >> 5) & 0x7));
			    else
				goto bad;
			    return(length);
			}
			else if(((specop1 >> 10) & 0x7) == 0x6){
			    if((specop1 & 0x18) == 0x10)
				printf("pflush\t#0x%x,#0x%x,",
				       (unsigned int)(specop1 & 0x7),
				       (unsigned int)((specop1 >> 5) & 0x7));
			    else if((specop1 & 0x18) == 0x08)
				printf("pflush\t%s,#0x%x,",
				       dregs[specop1 & 0x7],
				       (unsigned int)((specop1 >> 5) & 0x7));
			    else if((specop1 & 0x1f) == 0x00)
				printf("pflush\tsfc,#0x%x,",
				       (unsigned int)((specop1 >> 5) & 0x7));
			    else if((specop1 & 0x1f) == 0x01)
				printf("pflush\tdfc,#0x%x,",
				       (unsigned int)((specop1 >> 5) & 0x7));
			    else
				goto bad;
			    length += PRINT_EF(MODE(opword), REG(opword),
					       sect + length, L_SIZE);
			    printf("\n");
			    return(length);
			}
			else
			    goto bad;
		    }
		    else
			goto bad;
		case 2:
		    if(specop1 & 0x0200){
			if(((specop1 >> 10) & 0x7) == 0x0){
			    if(specop1 & 0x0100){
				printf("pmovefd\ttc,");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf("\n");
				return(length);
			    }
			    else{
				printf("pmove\ttc,");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf("\n");
				return(length);
			    }
			}
			else if(((specop1 >> 10) & 0x7) == 0x2){
			    if(specop1 & 0x0100){
				printf("pmovefd\tsrp,");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf("\n");
				return(length);
			    }
			    else{
				printf("pmove\tsrp,");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf("\n");
				return(length);
			    }
			}
			else if(((specop1 >> 10) & 0x7) == 0x3){
			    if(specop1 & 0x0100){
				printf("pmovefd\tcrp,");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf("\n");
				return(length);
			    }
			    else{
				printf("pmove\tcrp,");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf("\n");
				return(length);
			    }
			}
			else
			    goto bad;
		     }
		     else{
			if(((specop1 >> 10) & 0x7) == 0x0){
			    if(specop1 & 0x0100){
				printf("pmovefd\t");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf(",tc\n");
				return(length);
			    }
			    else{
				printf("pmove\t");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf(",tc\n");
				return(length);
			    }
			}
			else if(((specop1 >> 10) & 0x7) == 0x2){
			    if(specop1 & 0x0100){
				printf("pmovefd\t");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf(",srp\n");
				return(length);
			    }
			    else{
				printf("pmove\t");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf(",srp\n");
				return(length);
			    }
			}
			else if(((specop1 >> 10) & 0x7) == 0x3){
			    if(specop1 & 0x0100){
				printf("pmovefd\t");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf(",crp\n");
				return(length);
			    }
			    else{
				printf("pmove\t");
				length += PRINT_EF(MODE(opword), REG(opword),
						   sect + length, L_SIZE);
				printf(",crp\n");
				return(length);
			    }
			}
			else
			    goto bad;
		     }
		case 3:
		    if(specop1 & 0x0200){
			printf("pmove\tmmusr,");
			length += PRINT_EF(MODE(opword), REG(opword),
					   sect + length, L_SIZE);
			printf("\n");
			return(length);
		    }
		    else{
			printf("pmove\t");
			length += PRINT_EF(MODE(opword), REG(opword),
					   sect + length, L_SIZE);
			printf(",mmusr\n");
			return(length);
		    }
		case 4:
		    if(specop1 & 0x0200)
			printf("ptestr\t");
		    else
			printf("ptestw\t");
		    if((specop1 & 0x18) == 0x10)
			printf("#0x%x,", (unsigned int)(specop1 & 0x7));
		    else if((specop1 & 0x18) == 0x08)
			printf("%s,", dregs[specop1 & 0x7]);
		    else if((specop1 & 0x1f) == 0x00)
			printf("sfc,");
		    else if((specop1 & 0x1f) == 0x01)
			printf("dfc,");
		    else
			goto bad;
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, L_SIZE);
		    if(specop1 & 0x0100)
			printf(",#0x%x,%s\n",
			       (unsigned int)((specop1 >> 10) & 0x7),
			       aregs[(specop1 >> 5) & 0x7]);
		    else
			printf(",#0x%x\n",
			       (unsigned int)((specop1 >> 10) & 0x7));
		    return(length);
		default:
		    goto bad;
		}
	    }
	    if((opword & 0x0f20) == 0x0400){
		if(((opword >> 6) & 0x3) == 0 ||
		   ((opword >> 3) & 0x3) == 0)
		    goto bad;
		printf("cinv%c\t%s", scope[((opword >> 3) & 0x3) - 1],
		       cache[((opword >> 6) & 0x3) - 1]);
		if(((opword >> 3) & 0x3) != 0x3)
		    printf(",%s@\n", aregs[opword & 0x7]);
		else
		    printf("\n");
		return(length);
	    }
	    if((opword & 0x0f20) == 0x0420){
		if(((opword >> 6) & 0x3) == 0 ||
		   ((opword >> 3) & 0x3) == 0)
		    goto bad;
		printf("cpush%c\t%s", scope[((opword >> 3) & 0x3) - 1],
		       cache[((opword >> 6) & 0x3) - 1]);
		if(((opword >> 3) & 0x3) != 0x3)
		    printf(",%s@\n", aregs[opword & 0x7]);
		else
		    printf("\n");
		return(length);
	    }
	    if((opword & 0x0fe0) == 0x0500){
		switch((opword >> 3) & 0x3){
		case 0:
		    printf("pflushn\t%s@\n", aregs[opword & 0x7]);
		    return(length);
		case 1:
		    printf("pflush\t%s@\n", aregs[opword & 0x7]);
		    return(length);
		case 2:
		    printf("pflushan\n");
		    return(length);
		case 3:
		    printf("pflusha040\n");
		    return(length);
		}
	    }
	    if((opword & 0x0fd8) == 0x0548){
		if(opword & 0x0020){
		    printf("ptestr\t%s@\n", aregs[opword & 0x7]);
		    return(length);
		}
		else{
		    printf("ptestw\t%s@\n", aregs[opword & 0x7]);
		    return(length);
		}
	    }
	    if((opword & 0x0ff8) == 0x0620){
		GET_WORD(sect, left, length, swapped, specop1);
		printf("move16\t%s@+,%s@+\n", aregs[opword & 0x7],
		       aregs[(specop1 >> 12) & 0x7]);
		return(length);
	    }
	    if((opword & 0x0fe0) == 0x0600){
		GET_LONG(sect, left, length, swapped, l);
		switch((opword >> 3) & 0x3){
		case 0:
		    printf("move16\t%s@+,#0x%x\n", aregs[opword & 0x7],
		           (unsigned int)l);
		    break;
		case 1:
		    printf("move16\t#0x%x,%s@+\n",
		           (unsigned int)l,
			   aregs[opword & 0x7]);
		    break;
		case 2:
		    printf("move16\t%s@,#0x%x\n", aregs[opword & 0x7],
		           (unsigned int)l);
		    break;
		case 3:
		    printf("move16\t#0x%x,%s@\n",
		           (unsigned int)l,
			   aregs[opword & 0x7]);
		    break;
		}
		return(length);
	    }
	    if((opword & 0x0e00) != 0x0200)
		goto bad;

	    switch((opword >> 6) & 0x7){
	    case 0:
		GET_WORD(sect, left, length, swapped, specop1);
		switch((specop1 >> 13) & 0x7){
		case 0:
		    if((specop1 & 0x0078) == 0x0030)
#ifdef MOTO_SYNTAX
			printf("fsincosx\t%s,%s:%s\n",
#else
			printf("fsincosx\t%s,%s,%s\n",
#endif
			       fpregs[(specop1 >> 10) & 0x7],
			       fpregs[specop1 & 0x7],
			       fpregs[(specop1 >> 7) & 0x7]);
		    else{
			if((((specop1 >> 10) & 0x7) == ((specop1 >> 7) & 0x7))||
			   (specop1 & 0x007f) == 0x003a){ /* ftst */
			    if((specop1 & 0x7b) == 0x41) /* f?sqrt */
				printf("%s\t%s\n", fpops[specop1 & 0x7f],
				       fpregs[(specop1 >> 10) & 0x7]);
			    else
				printf("%sx\t%s\n", fpops[specop1 & 0x7f],
				       fpregs[(specop1 >> 10) & 0x7]);
			}
			else{
			    if((specop1 & 0x7b) == 0x41) /* f?sqrt */
				printf("%s\t%s,%s\n", fpops[specop1 & 0x7f],
				       fpregs[(specop1 >> 10) & 0x7],
				       fpregs[(specop1 >> 7) & 0x7]);
			    else
				printf("%sx\t%s,%s\n", fpops[specop1 & 0x7f],
				       fpregs[(specop1 >> 10) & 0x7],
				       fpregs[(specop1 >> 7) & 0x7]);
			}
		    }
		    return(length);
		case 1:
		    goto bad;
		case 2:
		    if((specop1 & 0x1c00) == 0x1c00){
			printf("fmovecrx\t#0x%x,%s\n",
			       (unsigned int)(specop1 & 0x7f),
			       fpregs[(specop1 >> 7) & 0x7]);
			return(length);
		    }
		    printf("%s%c\t", fpops[specop1 & 0x7f],
			   fpformat[(specop1 >> 10) & 0x7]);
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length,
				       fpsize[(specop1 >> 10) & 0x7]);
		    if((specop1 & 0x0078) == 0x0030)
#ifdef MOTO_SYNTAX
			printf(",%s:%s\n",
#else
			printf(",%s,%s\n",
#endif
			       fpregs[specop1 & 0x7],
			       fpregs[(specop1 >> 7) & 0x7]);
		    else{
		        if((specop1 & 0x007f) == 0x003a) /* ftst */
			    printf("\n");
			else
			    printf(",%s\n", fpregs[(specop1 >> 7) & 0x7]);
		    }
		    return(length);
		case 3:
		    printf("fmove%c\t%s,", fpformat[(specop1 >> 10) & 0x7],
		    	   fpregs[(specop1 >> 7) & 0x7]);
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length,
				       fpsize[(specop1 >> 10) & 0x7]);
		    if(((specop1 >> 10) & 0x7) == 0x3){
			printf("{#%u}\n", (unsigned int)(specop1 & 0x0040) ?
			       (unsigned int)(specop1 & 0x7f) |
				(unsigned int)0xffffff80 :
				(unsigned int)(specop1 & 0x7f));
		    } else if(((specop1 >> 10) & 0x7) == 0x7){
			printf("{%s}\n", dregs[(specop1 >> 4) & 0x7]);
		    } else
			printf("\n");
		    return(length);
		case 4:
		    printf("fmoveml\t");
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, L_SIZE);
		    printf(",%s\n", fpcregs[(specop1 >> 10) & 0x7]);
		    return(length);
		case 5:
		    printf("fmoveml\t%s,", fpcregs[(specop1 >> 10) & 0x7]);
		    length += PRINT_EF(MODE(opword), REG(opword),
				       sect + length, L_SIZE);
		    printf("\n");
		    return(length);
		case 6:
		case 7:
		    printf("fmovemx\t");
		    if((specop1 & 0x2000) == 0){
			length += PRINT_EF(MODE(opword), REG(opword),
					   sect + length, X_SIZE);
			printf(",");
			if((specop1 & 0x0800) == 0x0800){
			    printf("%s\n", dregs[((specop1 & 0x0070) >> 4)] );
			}
			else{
			    if((specop1 & 0x00ff) == 0){
				printf("#0x0\n");
			    }
			    else{
				if((specop1 & 0x1000) == 0x1000){
				    for(i = 0; i < 8; i++){
					if((specop1 & 0x0080) != 0){
					    printf("fp%u", i);
					    if(((specop1 << 1) & 0x00ff) != 0)
						printf("/");
					}
					specop1 <<= 1;
				    }
				}
				else{
				    specop1 &= 0x00ff;
				    for(i = 0; i < 8; i++){
					if((specop1 & 1) != 0){
					    printf("fp%u", i);
					    if((specop1 >> 1) != 0)
						printf("/");
					}
					specop1 >>= 1;
				    }
				}
				printf("\n");
			    }
			}
		    }
		    else{
			if((specop1 & 0x0800) == 0x0800){
			    printf("%s,", dregs[((specop1 & 0x0070) >> 4)] );
			}
			else{
			    if((specop1 & 0x00ff) == 0){
				printf("#0x0,");
			    }
			    else{
				if((specop1 & 0x1000) == 0x1000){
				    for(i = 0; i < 8; i++){
					if((specop1 & 0x0080) != 0){
					    printf("fp%u", i);
					    if(((specop1 << 1) & 0x00ff) != 0)
						printf("/");
					}
					specop1 <<= 1;
				    }
				}
				else{
				    specop1 &= 0x00ff;
				    for(i = 0; i < 8; i++){
					if((specop1 & 1) != 0){
					    printf("fp%u", i);
					    if((specop1 >> 1) != 0)
						printf("/");
					}
					specop1 >>= 1;
				    }
				}
				printf(",");
			    }
			}
			length += PRINT_EF(MODE(opword), REG(opword),
					   sect + length, X_SIZE);
			printf("\n");
		    }
		    return(length);
		}
	    case 1:
		GET_WORD(sect, left, length, swapped, specop1);
		if((opword & 0x003f) == 0x003a){
		    GET_WORD(sect, left, length, swapped, w);
		    printf("ftrap%sw\t#0x%x\n", fpcond[specop1 & 0x3f],
			   (unsigned int)w);
		    return(length);
		}
		if((opword & 0x003f) == 0x003b){
		    GET_LONG(sect, left, length, swapped, l);
		    printf("ftrap%sl\t#0x%x\n", fpcond[specop1 & 0x3f],
			   (unsigned int)l);
		    return(length);
		}
		if((opword & 0x003f) == 0x003c){
		    printf("ftrap%s\n", fpcond[specop1 & 0x3f]);
		    return(length);
		}
		if((opword & 0x0038) == 0x0008){
		    printf("fdb%s\t%s,", fpcond[specop1 & 0x3f],
			   dregs[REG(opword)]);
		    GET_WORD(sect, left, length, swapped, w);
		    if(w & 0x8000)
			l = 0xffff0000 | w;
		    else
			l = w;
		    if(PRINT_SYMBOL(addr + length - sizeof(unsigned short) + l,
				    addr + length - sizeof(unsigned short)))
			printf("\n");
		    else
			printf("0x%x\n", (unsigned int)
			       (addr + length - sizeof(unsigned short) + l));
		    return(length);
		}
		printf("fs%s\t", fpcond[specop1 & 0x3f]);
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, B_SIZE);
		printf("\n");
		return(length);
		
	    case 2:
		GET_WORD(sect, left, length, swapped, w);
		if(opword == 0xf280 && w == 0){
		    printf("fnop\n");
		    return(length);
		}
		if(opword & 0x20)
		    goto bad;
		printf("fb%s\t", fpcond[opword & 0x3f]);
		if(w & 0x8000)
		    l = 0xffff0000 | w;
		else
		    l = w;
		if(PRINT_SYMBOL(addr + length - sizeof(unsigned short) + l,
				addr + length - sizeof(unsigned short)))
		    printf(":w\n");
		else
		    printf("0x%x:w\n", (unsigned int)
			   (addr + length - sizeof(unsigned short) + l));
		return(length);

	    case 3:
		if(opword & 0x20)
		    goto bad;
		printf("fb%s\t", fpcond[opword & 0x3f]);
		GET_LONG(sect, left, length, swapped, l);
		if(PRINT_SYMBOL(addr + length - sizeof(uint32_t) + l,
				addr + length - sizeof(uint32_t)))
		    printf(":l\n");
		else
		    printf("0x%x:l\n", (unsigned int)
			   (addr + length - sizeof(uint32_t) + l));
		return(length);
	    case 4:
		printf("fsave\t");
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, B_SIZE);
		printf("\n");
		return(length);
	    case 5:
		printf("frestore\t");
		length += PRINT_EF(MODE(opword), REG(opword),
				   sect + length, B_SIZE);
		printf("\n");
		return(length);
	    case 6:
	    case 7:
		goto bad;
	    }
	}

bad:
	printf(".word\t0x%04x  | invalid instruction\n",
	       (unsigned int)opword);
	return(length);
}

union extension {
    short word;
    struct {
#ifdef __BIG_ENDIAN__
	unsigned da : 1;
	unsigned reg : 3;
	unsigned wl : 1;
	unsigned scale : 2;
	unsigned fb : 1;
	int      disp : 8;
#endif /*  __BIG_ENDIAN__ */
#ifdef __LITTLE_ENDIAN__
	int      disp : 8;
	unsigned fb : 1;
	unsigned scale : 2;
	unsigned wl : 1;
	unsigned reg : 3;
	unsigned da : 1;
#endif /*  __LITTLE_ENDIAN__ */
    } brief;
    struct {
#ifdef __BIG_ENDIAN__
	unsigned da : 1;
	unsigned reg : 3;
	unsigned wl : 1;
	unsigned scale : 2;
	unsigned fb : 1;
	unsigned bs : 1;
	unsigned is : 1;
	unsigned bdsize : 2;
	unsigned : 1;
	unsigned iis : 3;
#endif /*  __BIG_ENDIAN__ */
#ifdef __LITTLE_ENDIAN__
	unsigned iis : 3;
	unsigned : 1;
	unsigned bdsize : 2;
	unsigned is : 1;
	unsigned bs : 1;
	unsigned fb : 1;
	unsigned scale : 2;
	unsigned wl : 1;
	unsigned reg : 3;
	unsigned da : 1;
#endif /*  __LITTLE_ENDIAN__ */
    } full;
};

/*
 * Print the effective address mode for the mode and register and the
 * extension word(s) if needed.  The length in bytes of the extension
 * words this effective address used is returned.  Text points to the
 * extension word for this effective address.  Size is the size of the
 * immediate data for the #<data> addressing mode (B_SIZE == byte, etc).
 */
static
uint32_t
print_ef(
uint32_t mode,
uint32_t reg,
char *sect,
uint32_t addr,
uint32_t sect_addr,
uint32_t *left,
uint32_t size,
struct relocation_info *sorted_relocs,
uint32_t nsorted_relocs,
struct nlist *symbols,
uint32_t nsymbols,
struct symbol *sorted_symbols,
uint32_t nsorted_symbols,
char *strings,
uint32_t strings_size,
enum bool verbose,
enum bool swapped)
{
    uint32_t length, bd, od, l, x0, x1, x2, bd_addr, od_addr;
    unsigned short w;
    union extension ext;
    char *base, *index, *scale, bd_size, od_size;
    float f;
    double d;

	bd_size = '\0';
	od_size = '\0';
	length = 0;
	bd_addr = 0;
	od_addr = 0;
	od = bd = 0;
	switch(mode){
	case 0:
	    printf("%s", dregs[reg]);
	    return(length);
	case 1:
	    printf("%s", aregs[reg]);
	    return(length);
	case 2:
	    printf("%s@", aregs[reg]);
	    return(length);
	case 3:
	    printf("%s@+", aregs[reg]);
	    return(length);
	case 4:
	    printf("%s@-", aregs[reg]);
	    return(length);
	case 5:
	    GET_WORD(sect, *left, length, swapped, w);
	    printf("%s@(0x%x:w)", aregs[reg], (unsigned int)w);
	    return(length);
	case 7:
	    switch(reg){
	    case 0:
		GET_WORD(sect, *left, length, swapped, w);
		l = (uint32_t)(w & 0xffff);
		if(PRINT_SYMBOL(l, addr) == TRUE)
		    printf(":w");
		else
		    printf("0x%x:w", (unsigned int)w);
		return(length);
	    case 1:
		GET_LONG(sect, *left, length, swapped, l);
		if(PRINT_SYMBOL(l, addr) == TRUE)
		    printf(":l");
		else
		    printf("0x%x:l", (unsigned int)l);
		return(length);
	    case 2:
		GET_WORD(sect, *left, length, swapped, w);
		printf("pc@(0x%x:w)", (unsigned int)w);
		return(length);
	    case 3:
		break;
	    case 4:
		if(size == B_SIZE){
		    printf("#");
		    GET_WORD(sect, *left, length, swapped, w);
		    l = (uint32_t)(w & 0xff);
		    if(PRINT_SYMBOL(l, addr) == TRUE)
			printf(":b");
		    else
			printf("0x%x:b", (unsigned int)(w & 0xff));
		    return(length);
		}
		else if(size == W_SIZE){
		    printf("#");
		    GET_WORD(sect, *left, length, swapped, w);
		    l = (uint32_t)(w & 0xffff);
		    if(PRINT_SYMBOL(l, addr) == TRUE)
			printf(":w");
		    else
			printf("0x%x:w", (unsigned int)w);
		    return(length);
		}
		else if(size == L_SIZE){
		    printf("#");
		    GET_LONG(sect, *left, length, swapped, l);
		    if(PRINT_SYMBOL(l, addr) == TRUE)
			printf(":l");
		    else
			printf("0x%x:l", (unsigned int)l);
		    return(length);
		}
		else if(size == S_SIZE){
		    GET_FLOAT(sect, *left, length, swapped, f);
		    printf("#0s%g", f);
		    return(length);
		}
		else if(size == D_SIZE){
		    GET_DOUBLE(sect, *left, length, swapped, d);
		    printf("#0d%g", d);
		    return(length);
		}
		else if(size == X_SIZE){
		    if(*left < sizeof(uint32_t) * 3){
			x0 = 0;
			x1 = 0;
			x2 = 0;
			if(*left < sizeof(uint32_t)){
			    memcpy((char *)&x0, sect + length, *left);
			}
			else if(*left < sizeof(uint32_t) * 2){
			    memcpy((char *)&x0, sect + length,
				   sizeof(uint32_t));
			    memcpy((char *)&x1, sect + length + 4, *left);
			}
			else{
			    memcpy((char *)&x0, sect + length,
				   sizeof(uint32_t));
			    memcpy((char *)&x1, sect + length + 4,
				   sizeof(uint32_t));
			    memcpy((char *)&x2, sect + length + 8, *left);
			}
			length += *left;
			*left = 0;
		    }
		    else{
			memcpy((char *)&x0, sect + length,
			       sizeof(uint32_t));
			memcpy((char *)&x1, sect + length + 4,
			       sizeof(uint32_t));
			memcpy((char *)&x2, sect + length + 8,
			       sizeof(uint32_t));
			length += sizeof(uint32_t) * 3;
			*left -= sizeof(uint32_t) * 3;
		    }
		    if(swapped){
			x0 = SWAP_INT(x0);
			x1 = SWAP_INT(x1);
			x2 = SWAP_INT(x2);
		    }
		    printf("#0b%08x%08x%08x", (unsigned int)x0,
			   (unsigned int)x1, (unsigned int)x2);
		    return(length);
		}
		else if(size == P_SIZE){
		    if(*left < sizeof(uint32_t) * 3){
			x0 = 0;
			x1 = 0;
			x2 = 0;
			if(*left < sizeof(uint32_t)){
			    memcpy((char *)&x0, sect + length, *left);
			}
			else if(*left < sizeof(uint32_t) * 2){
			    memcpy((char *)&x0, sect + length,
				   sizeof(uint32_t));
			    memcpy((char *)&x1, sect + length + 4, *left);
			}
			else{
			    memcpy((char *)&x0, sect + length,
				   sizeof(uint32_t));
			    memcpy((char *)&x1, sect + length + 4,
				   sizeof(uint32_t));
			    memcpy((char *)&x2, sect + length + 8, *left);
			}
			length += *left;
			*left = 0;
		    }
		    else{
			memcpy((char *)&x0, sect + length,
			       sizeof(uint32_t));
			memcpy((char *)&x1, sect + length + 4,
			       sizeof(uint32_t));
			memcpy((char *)&x2, sect + length + 8,
			       sizeof(uint32_t));
			length += sizeof(uint32_t) * 3;
			*left -= sizeof(uint32_t) * 3;
		    }
		    if(swapped){
			x0 = SWAP_INT(x0);
			x1 = SWAP_INT(x1);
			x2 = SWAP_INT(x2);
		    }
		    if(x0 & 0x80000000)
			printf("#-%c.", (char)((x0 & 0xf) + '0'));
		    else
			printf("#%c.", (char)((x0 & 0xf) + '0'));
		    printf("%08x%08x", (unsigned int)x1, (unsigned int)x2);
		    if(x0 & 0x40000000)
			printf("e-%03x", (unsigned int)(x0 & 0xfff)); 
		    else
			printf("e%03x", (unsigned int)(x0 & 0xfff)); 
		    return(length);
		}
	    default:
		printf("<bad ef>");
		return(length);
	    }
	}

	/*
	 * To get here we know that the mode is 6 (110) or the mode is 7 (111)
	 * and the register is (011).  So that this uses either a brief or
	 * full extension word.
	 */
	GET_WORD(sect, *left, length, swapped, ext.word);
	if(mode == 6)
	    base = aregs[reg];
	else
	    base = "pc";
	if(ext.brief.da == 0)
	    index = dregs[ext.brief.reg];
	else
	    index = aregs[ext.brief.reg];
	scale = scales[ext.brief.scale];
	/* check for a brief format extension word */
	if(ext.brief.fb == 0){
	    printf("%s@(0x%x:b,%s:%c%s)", base, (unsigned int)ext.brief.disp,
		   index, wl[ext.brief.wl], scale);
	}
	/* extension word is a full format extension word */
	else{
	    switch(ext.full.bdsize){
	    case 0:
		printf("<bad ef>");
		return(length);
	    case 1:
		break;
	    case 2:
		bd_addr = addr + length;
		GET_WORD(sect, *left, length, swapped, w);
		bd = w;
		bd_size = 'w';
		break;
	    case 3:
		bd_addr = addr + length;
		GET_LONG(sect, *left, length, swapped, bd);
		bd_size = 'l';
		break;
	    }
	    switch(ext.full.iis & 0x3){
	    case 0:
	    case 1:
		break;
	    case 2:
		od_addr = addr + length;
		GET_WORD(sect, *left, length, swapped, w);
		od = w;
		od_size = 'w';
		break;
	    case 3:
		od_addr = addr + length;
		GET_LONG(sect, *left, length, swapped, od);
		od_size = 'l';
		break;
	    }
	    /* check if base (address) register is not used */
	    if(ext.full.bs == 1){
		if(mode == 7)
		    base = "zpc";
		else
		    base = "";
	    }
	    /* check if base displacement is used */
	    if(ext.full.bdsize != 1){
		/* check if index register is used */
		if(ext.full.is == 0){
		    switch(ext.full.iis){
		    case 0:
/*
			printf("%s@(0x%x:%c,%s:%c%s)", base, (unsigned int)bd,
				bd_size, index, wl[ext.full.wl], scale);
*/
			printf("%s@(", base);
			if(PRINT_SYMBOL(bd, bd_addr) == TRUE)
			    printf(":%c,", bd_size);
			else
			    printf("0x%x:%c,", (unsigned int)bd, bd_size);
			printf("%s:%c%s)", index, wl[ext.full.wl], scale);
			break;
		    case 1:
/*
			printf("%s@(0x%x:%c,%s:%c%s)@(0)",base,(unsigned int)bd,
				bd_size, index, wl[ext.full.wl], scale);
*/
			printf("%s@(", base);
			if(PRINT_SYMBOL(bd, bd_addr) == TRUE)
			    printf(":%c,", bd_size);
			else
			    printf("0x%x:%c,", (unsigned int)bd, bd_size);
			printf("%s:%c%s)@(0)", index, wl[ext.full.wl], scale);
			break;
		    case 2:
		    case 3:
/*
			printf("%s@(0x%x:%c,%s:%c%s)@(0x%x:%c)", base,
				(unsigned int)bd, bd_size, index,
				wl[ext.full.wl], scale, (unsigned int)od,
				od_size);
*/
			printf("%s@(", base);
			if(PRINT_SYMBOL(bd, bd_addr) == TRUE)
			    printf(":%c,", bd_size);
			else
			    printf("0x%x:%c,", (unsigned int)bd, bd_size);
			printf("%s:%c%s)@(", index, wl[ext.full.wl], scale);
			if(PRINT_SYMBOL(od, od_addr) == TRUE)
			    printf(":%c)", od_size);
			else
			    printf("0x%x:%c)", (unsigned int)od, od_size);
			break;
		    case 5:
/*
			printf("%s@(0x%x:%c)@(0,%s:%c%s)",base,(unsigned int)bd,
				bd_size, index, wl[ext.full.wl], scale);
*/
			printf("%s@(", base);
			if(PRINT_SYMBOL(bd, bd_addr) == TRUE)
			    printf(":%c)", bd_size);
			else
			    printf("0x%x:%c)", (unsigned int)bd, bd_size);
			printf("@(0,%s:%c%s)", index, wl[ext.full.wl], scale);
			break;
		    case 6:
		    case 7:
/*
			printf("%s@(0x%x:%c)@(0x%x:%c,%s:%c%s)", base,
				(unsigned int)bd, bd_size, (unsigned int)od,
				od_size, index, wl[ext.full.wl], scale);
*/
			printf("%s@(", base);
			if(PRINT_SYMBOL(bd, bd_addr) == TRUE)
			    printf(":%c)@(", bd_size);
			else
			    printf("0x%x:%c)@(", (unsigned int)bd, bd_size);
			if(PRINT_SYMBOL(od, od_addr) == TRUE)
			    printf(":%c,", od_size);
			else
			    printf("0x%x:%c,", (unsigned int)od, od_size);
			printf("%s:%c%s)", index, wl[ext.full.wl], scale);
			break;
		    case 4:
		    default:
			printf("<bad ef>");
			break;
		    }
		}
		/* index register is suppressed */
		else{
		    switch(ext.full.iis){
		    case 0:
/*
			printf("%s@(0x%x:%c)", base, (unsigned int)bd, bd_size);
*/
			printf("%s@(", base);
			if(PRINT_SYMBOL_DOT(bd, bd_addr, addr) == TRUE)
			    printf(":%c)", bd_size);
			else
			    printf("0x%x:%c)", (unsigned int)bd, bd_size);
			break;
		    case 1:
/*
			printf("%s@(0x%x:%c)@(0)", base, (unsigned int)bd,
				bd_size);
*/
			printf("%s@(", base);
			if(PRINT_SYMBOL(bd, bd_addr) == TRUE)
			    printf(":%c)@(0)", bd_size);
			else
			    printf("0x%x:%c)@(0)", (unsigned int)bd, bd_size);
			break;
		    case 2:
		    case 3:
/*
			printf("%s@(0x%x:%c)@(0x%x:%c)", base, (unsigned int)bd,
				 bd_size, (unsigned int)od, od_size);
*/
			printf("%s@(", base);
			if(PRINT_SYMBOL(bd, bd_addr) == TRUE)
			    printf(":%c)@(", bd_size);
			else
			    printf("0x%x:%c)@(", (unsigned int)bd, bd_size);
			if(PRINT_SYMBOL(od, od_addr) == TRUE)
			    printf(":%c)", od_size);
			else
			    printf("0x%x:%c)", (unsigned int)od, od_size);
			break;
		    default:
			printf("<bad ef>");
			break;
		    }
		}
	    }
	    /* base displacement is not used */
	    else{
		/* check if index register is used */
		if(ext.full.is == 0){
		    switch(ext.full.iis){
		    case 0:
			printf("%s@(0,%s:%c%s)", base, index, wl[ext.full.wl],
				scale);
			break;
		    case 1:
			printf("%s@(0,%s:%c%s)@(0)", base, index,
				wl[ext.full.wl], scale);
			break;
		    case 2:
		    case 3:
/*
			printf("%s@(0,%s:%c%s)@(0x%x:%c)", base, index,
				wl[ext.full.wl], scale, (unsigned int)od,
				od_size);
*/
			printf("%s@(0,%s:%c%s)@(", base, index, wl[ext.full.wl],
			       scale);
			if(PRINT_SYMBOL(od, od_addr) == TRUE)
			    printf(":%c)", od_size);
			else
			    printf("0x%x:%c)", (unsigned int)od, od_size);
			break;
		    case 5:
			printf("%s@(0)@(0,%s:%c%s)", base, index,
				wl[ext.full.wl], scale);
			break;
		    case 6:
		    case 7:
/*
			printf("%s@(0)@(0x%x:%c,%s:%c%s)",base,(unsigned int)od,
				od_size, index, wl[ext.full.wl], scale);
*/
			printf("%s@(0)@(", base);
			if(PRINT_SYMBOL(od, od_addr) == TRUE)
			    printf(":%c)", od_size);
			else
			    printf("0x%x:%c)", (unsigned int)od, od_size);
			printf(",%s:%c%s)@(", index, wl[ext.full.wl], scale);
			break;
		    case 4:
		    default:
			printf("<bad ef>");
			break;
		    }
		}
		/* index register is suppressed */
		else{
		    switch(ext.full.iis){
		    case 0:
			printf("%s@(0)", base);
			break;
		    case 1:
			printf("%s@(0)@(0)", base);
			break;
		    case 2:
		    case 3:
/*
			printf("%s@(0)@(0x%x:%c)", base, (unsigned int)od,
				od_size);
*/
			printf("%s@(0)@(", base);
			if(PRINT_SYMBOL(od, od_addr) == TRUE)
			    printf(":%c)", od_size);
			else
			    printf("0x%x:%c)", (unsigned int)od, od_size);
			break;
		    default:
			printf("<bad ef>");
			break;
		    }
		}
	    }
	}
	return(length);
}
