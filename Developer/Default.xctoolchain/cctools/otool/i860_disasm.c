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
#include <mach-o/i860/reloc.h>
#include "stuff/symbol.h"
#include "stuff/bytesex.h"
#include "otool.h"
#include "../as/i860-opcode.h"

static void i860_dump_operands(
    uint32_t opcode,
    char *format,
    uint32_t addr,
    uint32_t sect_addr,
    struct relocation_info *relocs,
    uint32_t nrelocs,
    struct nlist *symbols,
    uint32_t nsymbols,
    struct symbol *sorted_symbols,
    uint32_t nsorted_symbols,
    char *strings,
    uint32_t strings_size,
    enum bool verbose);

static void i860_dump_addr(
    uint32_t addr_field,
    int format,
    int32_t addr,
    uint32_t sect_addr,
    struct relocation_info *relocs,
    uint32_t nrelocs,
    struct nlist *symbols,
    uint32_t nsymbols,
    struct symbol *sorted_symbols,
    uint32_t nsorted_symbols,
    char *strings,
    uint32_t strings_size,
    enum bool verbose);

static enum bool i860_print_symbol(
    uint32_t value,
    struct relocation_info *rp,
    struct nlist *symbols,
    uint32_t nsymbols,
    struct symbol *sorted_symbols,
    uint32_t nsorted_symbols,
    char *strings,
    uint32_t strings_size,
    enum bool verbose);

/*
 * Disassemble 1 instruction and return the length of the disassembled 
 * piece in bytes.
 */
uint32_t
i860_disassemble(
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
    int isdual;
    uint32_t i;
    struct i860_opcode *op;

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != object_byte_sex;

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

	/*
	 * The pad opcode, 0, is chosen as an illegal insn to fault if
	 * executed
	 */
	if(opcode == 0){
	    printf("| Padded to i860 section boundary\n");
	    return(4);
	}

	isdual = 0;
	/*
	 * See if this is a dual insn mode opcode.
	 * Turn off the dual mode bit and print a d. if appropriate.
	 */
	if((opcode & OP_PREFIX_MASK) == PREFIX_FPU || 
	   opcode == (OP_FNOP|DUAL_INSN_MODE_BIT)){
	    if(opcode & DUAL_INSN_MODE_BIT){
		opcode &= ~DUAL_INSN_MODE_BIT;
		isdual = 1;
	    }
	}
	/*
	 * Search the instruction table for a match for this opcode.
	 * We use a linear search because it's easy, uses the 
	 * assembler insn tables, and I'm so lazy....
	 * Feel free to recode with whizzy hashes and such.
	 */
	op = (struct i860_opcode *)i860_opcodes;
	for(i = 0; i < NUMOPCODES; i++, op++){
	    if((opcode & op->mask) == op->match){
		if(isdual)
		    printf("d.%-12s\t", op->name);
		else
		    printf("%-12s\t", op->name);
		i860_dump_operands(opcode, (char *)op->args, addr, sect_addr,
			relocs, nrelocs, symbols, nsymbols, sorted_symbols,
			nsorted_symbols, strings, strings_size, verbose);
		return(sizeof(uint32_t));
	    }
	}

	/* Didn't find the opcode.  Dump it as a .long directive. */
	/* Build it as a little-endian insn, in a format to match asm */
	printf(".long\t0x%08x\n", (unsigned int)opcode);
	return(sizeof(uint32_t));
}

/* 32 possible valuse, of which 6 are actually used. */
static char *i860_controlregs[] = {"fir", "psr", "dirbase", "db", "fsr", "epsr",
				   "?","?","?","?","?","?",
				   "?","?","?","?","?","?",
				   "?","?","?","?","?","?",
				   "?","?","?","?","?","?"};

static
void
i860_dump_operands(
uint32_t opcode,
char *format,
uint32_t addr,
uint32_t sect_addr,
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
    uint32_t addr_field;
	
	while(*format != '\0'){
	    switch(*format){
	    case '1': /* rs1 register, bits 11-15 of insn */
		printf("r%u", GET_RS1(opcode));
		break;
		    
	    case '2': /* rs2 register, bits 21-25 of insn */
		printf("r%u", GET_RS2(opcode));
		break;
		    
	    case 'd': /* rd register, bits 16-20 of insn */
		printf("r%u", GET_RD(opcode));
		break;
	    
	    case 'E':	
	    case 'e': /* frs1 floating point register, bits 11-15 of insn */
		printf("f%u", GET_RS1(opcode));
		break;
	    
	    case 'F':	
	    case 'f': /* frs2 floating point register, bits 21-25 of insn */
		printf("f%u", GET_RS2(opcode));
		break;
	    
	    case 'H':
	    case 'G':	
	    case 'g': /* frsd floating point register, bits 16-20 of insn */ 
		printf("f%u", GET_RD(opcode));
		break;
		    
	    case 'I': /* 16 bit High portion of address, I860_RELOC_HIGH */
	    case 'J': /* 16 bit High portion of addr requiring adjustment */
		addr_field = opcode & 0xFFFF;
		i860_dump_addr(addr_field, *format, addr, sect_addr, relocs,
			nrelocs, symbols, nsymbols, sorted_symbols,
			nsorted_symbols, strings, strings_size, verbose);
		break;

	    case 'i': /* 16 bit byte address low half */
		addr_field = (opcode & 0xFFFF);
		i860_dump_addr(addr_field, *format, addr, sect_addr, relocs,
			nrelocs, symbols, nsymbols, sorted_symbols,
			nsorted_symbols, strings, strings_size, verbose);
		break;

	    case 'j': /* 16 bit short address, I860_RELOC_LOW1 */
		addr_field = (opcode & 0xFFFE);
		i860_dump_addr(addr_field, *format, addr, sect_addr, relocs,
			nrelocs, symbols, nsymbols, sorted_symbols,
			nsorted_symbols, strings, strings_size, verbose);
		break;

	    case 'k': /* 16 bit word/int address low half, I860_RELOC_LOW2 */
		addr_field = (opcode & 0xFFFC);
		i860_dump_addr(addr_field, *format, addr, sect_addr, relocs,
			nrelocs, symbols, nsymbols, sorted_symbols,
			nsorted_symbols, strings, strings_size, verbose);
		break;

	    case 'l': /* 16 bit 8-byte address (double) low half */
		addr_field = (opcode & 0xFFF8);
		i860_dump_addr(addr_field, *format, addr, sect_addr, relocs,
			nrelocs, symbols, nsymbols, sorted_symbols,
			nsorted_symbols, strings, strings_size, verbose);
		break;

	    case 'm': /* 16 bit 16-byte address (quad) low half */
		addr_field = (opcode & 0xFFF0);
		i860_dump_addr(addr_field, *format, addr, sect_addr, relocs,
			nrelocs, symbols, nsymbols, sorted_symbols,
			nsorted_symbols, strings, strings_size, verbose);
		break;

	    case 'n': /* 16 bit byte aligned low half, split fields */
		addr_field = ((opcode >> 5) & 0xF800) | (opcode & 0x7FF);
		i860_dump_addr(addr_field, *format, addr, sect_addr, relocs,
			nrelocs, symbols, nsymbols, sorted_symbols,
			nsorted_symbols, strings, strings_size, verbose);
		break;

	    case 'o': /* 16 bit short aligned low half, split fields */
		addr_field = ((opcode >> 5) & 0xF800) | (opcode & 0x7FE);
		i860_dump_addr(addr_field, *format, addr, sect_addr, relocs,
			nrelocs, symbols, nsymbols, sorted_symbols,
			nsorted_symbols, strings, strings_size, verbose);
		break;

	    case 'p': /* 16 bit int/word aligned low half, split fields */
		addr_field = ((opcode >> 5) & 0xF800) | (opcode & 0x7FC);
		i860_dump_addr(addr_field, *format, addr, sect_addr, relocs,
			nrelocs, symbols, nsymbols, sorted_symbols,
			nsorted_symbols, strings, strings_size, verbose);
		break;

	    case 'K': /* 26 bit branch displacement */
		addr_field = opcode & 0x3FFFFFF;
		if(addr_field & 0x02000000)	/* MSB set? */
		    addr_field |= 0xFC000000; 	/* Sign extend */
		addr_field <<= 2;	/* Convert to byte addr */
		i860_dump_addr(addr_field, *format, addr, sect_addr, relocs,
			nrelocs, symbols, nsymbols, sorted_symbols,
			nsorted_symbols, strings, strings_size, verbose);
		break;
		    
	    case 'L': /* 16 bit split branch displacement */
		addr_field = ((opcode >> 5) & 0xF800) | (opcode & 0x7FF);
		if(addr_field & 0x8000)		/* MSB set? */
		    addr_field |= 0xFFFF0000; 	/* Sign extend */
		addr_field <<= 2;	/* Convert to byte addr */
		i860_dump_addr(addr_field, *format, addr, sect_addr, relocs,
			nrelocs, symbols, nsymbols, sorted_symbols,
			nsorted_symbols, strings, strings_size, verbose);
		break;

	    case 'D': /* constant for shift opcode */	
		printf("%u", opcode & 0xFFFF);
		break;
		    
	    case 'B': /* 5 bit immediate, for bte and btne insn */
		printf("%u", GET_RS1(opcode));
		break;
		    
	    case 'C': /* Control Register */
		printf("%s", i860_controlregs[GET_RS2(opcode)]);
		break;
		    
	    default:
		printf("%c", *format);
		break;
	    }
	    ++format;
	}
	printf("\n");
}

static
void
i860_dump_addr(
uint32_t addr_field,
int format,
int32_t addr,
uint32_t sect_addr,
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
    uint32_t i;
    struct relocation_info *rp, *pairp;
    struct scattered_relocation_info *sreloc;
    char *prefix;
	
	rp = NULL;
	pairp = NULL;
	if(nrelocs){
	    for(i = 0; i < nrelocs; i++){
		if(((relocs[i].r_address) & R_SCATTERED) != 0){
		    sreloc = (struct scattered_relocation_info *)(relocs + i);
		    if(sreloc->r_type == I860_RELOC_PAIR){
			fprintf(stderr, "Stray I860_RELOC_PAIR relocation "
				"entry %u\n", i);
			continue;
		    }
		    if(sreloc->r_type == I860_RELOC_HIGH ||
		       sreloc->r_type == I860_RELOC_HIGHADJ ||
		       sreloc->r_type == I860_RELOC_SECTDIFF){
			if(i+1 >= nrelocs ||
			   relocs[i+1].r_type != I860_RELOC_PAIR){
				fprintf(stderr, "No I860_RELOC_PAIR relocation "
					"entry after entry %u\n", i);
			}
			else{
			    if(((relocs[i+1].r_address) & R_SCATTERED) != 0){
				sreloc = (struct scattered_relocation_info *)
					 (relocs + i + 1);
				if(sreloc->r_type != I860_RELOC_PAIR)
				    fprintf(stderr, "No I860_RELOC_PAIR "
					    "relocation entry after entry "
					    "%u\n", i);
			    }
			    else if(relocs[i+1].r_type != I860_RELOC_PAIR){
				fprintf(stderr, "No I860_RELOC_PAIR relocation "
					"entry after entry %u\n", i);
			    }
			    i++;
			    continue;
			}
		    }
		}
		if(relocs[i].r_type == I860_RELOC_PAIR){
		    fprintf(stderr, "Stray I860_RELOC_PAIR relocation entry "
			    "%u\n", i);
		    continue;
		}
		if((uint32_t)relocs[i].r_address == addr - sect_addr){
		    rp = &relocs[i];
		    if(rp->r_type == I860_RELOC_HIGH ||
		       rp->r_type == I860_RELOC_HIGHADJ ||
		       rp->r_type == I860_RELOC_SECTDIFF){
			if(i+1 < nrelocs){
			    pairp = &rp[1];
			    if(pairp->r_type != I860_RELOC_PAIR){
				fprintf(stderr, "No I860_RELOC_PAIR relocation "
					"entry after entry %u\n", i);
				rp = NULL;
				pairp = NULL;
				continue;
			    }
			}
		    }
		    break;
		}
		if(relocs[i].r_type == I860_RELOC_HIGH ||
		   relocs[i].r_type == I860_RELOC_HIGHADJ ||
		   relocs[i].r_type == I860_RELOC_SECTDIFF){
		    if(i+1 >= nrelocs ||
		       relocs[i+1].r_type != I860_RELOC_PAIR){
			    fprintf(stderr, "No I860_RELOC_PAIR relocation "
				    "entry after entry %u\n", i);
		    }
		    else
			i++;
		}
	    }
	}

	/* Guess a prefix code for the immediate value */
	prefix = NULL;
	if((rp != NULL && rp->r_type == I860_RELOC_HIGH) || format == 'I')
	    prefix = "h%";
	else if((rp != NULL && rp->r_type == I860_RELOC_HIGHADJ) ||
		format == 'J' )
	    prefix = "ha%";
	else if(rp != NULL && rp->r_type >= I860_RELOC_LOW0 &&
			      rp->r_type <= I860_RELOC_SPLIT2){
	    if(rp->r_pcrel == 0)	/* Don't use for bte insns */
		prefix = "l%";
	}
	if(rp != NULL && (rp->r_type == I860_RELOC_HIGH ||
			  rp->r_type == I860_RELOC_HIGHADJ)){
	    if(pairp->r_type == I860_RELOC_PAIR){
		if(rp->r_type == I860_RELOC_HIGHADJ)

		    if(pairp->r_address & 0x8000)
			addr_field = (addr_field << 16) +
				     (0xffff0000 | (pairp->r_address & 0xffff));
		    else
			addr_field = (addr_field << 16) +
				     (pairp->r_address & 0xffff);
		else
		    addr_field = (addr_field << 16) |
				 (pairp->r_address & 0xffff);
	    }
	}	
	if(prefix != NULL)
	    printf("%s", prefix);

	if(format == 'K' || format == 'L'){ /* branch displacement */
	    if(i860_print_symbol(addr + 4 + ((int32_t)addr_field), rp,
		     symbols, nsymbols, sorted_symbols, nsorted_symbols,
		     strings, strings_size, verbose) == TRUE)
		return;
	    printf(".%+d", (int32_t)(addr_field + 4));
	    return;
	}
	if(i860_print_symbol(addr_field, rp, symbols, nsymbols,
			     sorted_symbols, nsorted_symbols, strings,
			     strings_size, verbose) == TRUE)	
	    return;

	/* we can't find anything else to do with it. */
	printf("0x%x", (unsigned int)addr_field);
}

/*
 * i860_print_symbol prints a symbol name for the addr and relocation entry
 * if a symbol exist with the same address.  Nothing else is printed, no
 * whitespace, no newline.  If it prints something then it returns TRUE, else
 * it returns FALSE.
 */
static
enum bool
i860_print_symbol(
uint32_t value,
struct relocation_info *rp,
struct nlist *symbols,
uint32_t nsymbols,
struct symbol *sorted_symbols,
uint32_t nsorted_symbols,
char *strings,
uint32_t strings_size,
enum bool verbose)
{
    int32_t high, low, mid;

	if(verbose == FALSE)
	    return(FALSE);

	if(rp != NULL){
	    if(rp->r_extern &&
	       rp->r_symbolnum < nsymbols){
		if(symbols[rp->r_symbolnum].n_un.n_strx > strings_size)
		    return(FALSE);
		if(value != 0)
		    printf("%s+0x%x", strings +
			   symbols[rp->r_symbolnum].n_un.n_strx,
			   (unsigned int)value);
		else
		    printf("%s",strings + symbols[rp->r_symbolnum].n_un.n_strx);
		return(TRUE);
	    }
	}

	low = 0;
	high = nsorted_symbols - 1;
	mid = (high - low) / 2;
	while(high >= low){
	    if(sorted_symbols[mid].n_value == value){
		printf("%s", sorted_symbols[mid].name);
		return(TRUE);
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
	return(FALSE);
}
