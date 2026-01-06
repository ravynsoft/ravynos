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
#include <stdlib.h>
#include <string.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/reloc.h>
#include <mach-o/sparc/reloc.h>
#include "stuff/bytesex.h"
#include "stuff/symbol.h"
#include "otool.h"
#include "dyld_bind_info.h"
#include "ofile_print.h"
#include "../as/sparc-opcode.h"

/* Sign-extend a value which is N bits long.  */
#define	SEX(value, bits) \
	((((int)(value)) << ((8 * sizeof (int)) - bits))	\
			 >> ((8 * sizeof (int)) - bits) )

static  char *reg_names[] =
{ "g0", "g1", "g2", "g3", "g4", "g5", "g6", "g7",	
  "o0", "o1", "o2", "o3", "o4", "o5", "sp", "o7",	
  "l0", "l1", "l2", "l3", "l4", "l5", "l6", "l7",	
  "i0", "i1", "i2", "i3", "i4", "i5", "fp", "i7",	
  "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7",	
  "f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",	
  "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",
  "f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31",
  "y", "psr", "wim", "tbr", "pc", "npc", "fpsr", "cpsr"
};

#define	freg_names	(&reg_names[4 * 8])

union sparc_insn
  {
    uint32_t code;
    struct
      {
#ifdef __BIG_ENDIAN__
	unsigned int anop:2;
	unsigned int anrd:5;
	unsigned int op3:6;
	unsigned int anrs1:5;
	unsigned int i:1;
	unsigned int anasi:8;
	unsigned int anrs2:5;
#endif
#ifdef __LITTLE_ENDIAN__
	unsigned int anrs2:5;
	unsigned int anasi:8;
	unsigned int i:1;
	unsigned int anrs1:5;
	unsigned int op3:6;
	unsigned int anrd:5;
	unsigned int anop:2;
#endif
      } ldst;
    struct
      {
#ifdef __BIG_ENDIAN__
	unsigned int anop:2; 
        unsigned int anrd:5;
        unsigned int op3:6;
        unsigned int anrs1:5;
        unsigned int i:1;
	unsigned int IMM13:13;
#endif
#ifdef __LITTLE_ENDIAN__
	unsigned int IMM13:13;
        unsigned int i:1;
        unsigned int anrs1:5;
        unsigned int op3:6;
        unsigned int anrd:5;
	unsigned int anop:2; 
#endif
      } IMM13;
    struct
      {
#ifdef __BIG_ENDIAN__
	unsigned int anop:2;
	unsigned int a:1;
	unsigned int cond:4;
	unsigned int op2:3;
	unsigned int DISP22:22;
#endif
#ifdef __LITTLE_ENDIAN__
	unsigned int DISP22:22;
	unsigned int op2:3;
	unsigned int cond:4;
	unsigned int a:1;
	unsigned int anop:2;
#endif
      } branch;
    struct
      {
#ifdef __BIG_ENDIAN__
	unsigned int anop:2;
	unsigned int adisp30:30;
#endif
#ifdef __LITTLE_ENDIAN__
	unsigned int adisp30:30;
	unsigned int anop:2;
#endif
      } call;
  };

#define	op	ldst.anop
#define	rd	ldst.anrd
#define	rs1	ldst.anrs1
#define	asi	ldst.anasi
#define	rs2	ldst.anrs2
#define	shcnt	rs2
#define	imm13	IMM13.IMM13
#define	disp22	branch.DISP22
#define	imm22	disp22
#define	disp30	call.adisp30

#define SYM_PRINT(a,b,c) print_symbolic((a),(b),(c),relocs, nrelocs, symbols, \
				      nsymbols, sorted_symbols, \
				      nsorted_symbols, strings, \
				      strings_size, verbose)

static int opcodes_sorted = 0;
static int compare_opcodes (const void* a, const void* b);

#ifdef NOT_USED
/* Nonzero if INSN is the opcode for a delayed branch.  */
static int
is_delayed_branch (insn)
	union sparc_insn insn;
{
	unsigned int i;

	for (i = 0; i < NUMOPCODES; ++i) {
		struct sparc_opcode *opcode = &sparc_opcodes[i];

		if ((opcode->match & insn.code) == opcode->match
				&& (opcode->lose & insn.code) == 0)
			return (opcode->flags & F_DELAYED);
	}
	return 0;
}

/* find_lo_value attempts to look ahead in the code and find the %lo
   value corresponding to the current %hi and return its value.
   The %lo may have been optimized away by the assembler and we don't
   know how many intermediate instructions there will be between the hi/lo
   combination. To limit the search, we only look a maximum of 5 insn
   ahead */

#define MAX_LO_LOOKAHEAD 5

int
find_lo_value(
	      char *section, 
	      uint32_t addr, 
	      union sparc_insn insn)
{
  int index, next_insn;

  index = 0;
  while (++index < MAX_LO_LOOKAHEAD) {
    memcpy(&next_insn, section + 4*index, sizeof (unsigned int));
  }
}
#endif /* NOT_USED */

void
print_address_func(unsigned int addr)
{
	printf("0x%x", addr);
}

/* print a label symbol. Modified from print_label in ofile_print.c
 * Prints the label it found and returns TRUE or prints nothing
 * and return FALSE
 */

enum bool
label_symbol(
uint32_t addr,
enum bool colon_and_newline,
struct symbol *sorted_symbols,
uint32_t nsorted_symbols)
{
    int32_t high, low, mid;

	low = 0;
	high = nsorted_symbols - 1;
	mid = (high - low) / 2;
	while(high >= low){
	    if(sorted_symbols[mid].n_value == addr){
		printf("%s", sorted_symbols[mid].name);
		if(colon_and_newline == TRUE)
		    printf(":\n");
		return TRUE;
	    }
	    if(sorted_symbols[mid].n_value > addr){
		high = mid - 1;
		mid = (high + low) / 2;
	    }
	    else{
		low = mid + 1;
		mid = (high + low) / 2;
	    }
	}
    return FALSE;
}

/* print symbol name for an address and reloc entry. 
   print either symbol found or print the value of the address but print
   something
*/
static void
print_symbolic(
	       char operand,
	       uint32_t value,
	       unsigned int pc,
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
  struct relocation_info *rp, *rp_pair;
  uint32_t i, r_address, r_symbolnum, r_type, r_extern,
	   r_value, r_scattered, pair_r_type, pair_r_value;
  int32_t reloc_found, offset;
  uint32_t other_half;
  const char *name, *add, *sub;
  struct scattered_relocation_info *srp;

  r_symbolnum = 0;
  r_type = 0;
  r_extern = 0;
  r_value = 0;
  r_scattered = 0;
  other_half = 0;
  reloc_found = 0;
  pair_r_value = 0;

  /* if verbose output is not requested, don't even attempt lookup */

  if (!verbose) {
    printf("0x%x", (unsigned int) value);
    return;
  }

  /* Check all reloc entries */
  if (nrelocs) {
    for (i=0, rp = relocs; i<nrelocs; i++, rp++) {
      if (rp->r_address & R_SCATTERED) {
	srp = (struct scattered_relocation_info *) rp;
	r_scattered = 1;
	r_address = srp->r_address;
	r_extern = 0;
	r_type = srp->r_type;
	r_value = srp->r_value;
      }	else {
	r_scattered = 0;
	r_address = rp->r_address;
	r_symbolnum = rp->r_symbolnum;
	r_extern = rp->r_extern;
	r_type = rp->r_type;
      }
      if (r_type == SPARC_RELOC_PAIR) {
	fprintf(stderr, "Stray SPARC_RELOC_PAIR entry ");
	continue;
      }
      if(r_address == pc){
	if(r_type == SPARC_RELOC_HI22 || 
	   r_type == SPARC_RELOC_LO10 ||
	   r_type == SPARC_RELOC_HI22_SECTDIFF ||
	   r_type == SPARC_RELOC_LO10_SECTDIFF ||
	   r_type == SPARC_RELOC_SECTDIFF){
	  if(i+1 < nrelocs){
	    rp_pair = rp+1;
	    if(rp_pair->r_address & R_SCATTERED){
	      srp = (struct scattered_relocation_info *) rp_pair;
	      other_half = srp->r_address;
	      pair_r_type = srp->r_type;
	      pair_r_value = srp->r_value;
	    } else {
	      other_half = rp_pair->r_address;
	      pair_r_type = rp_pair->r_type;
	    }
	    if(pair_r_type != SPARC_RELOC_PAIR){
	      fprintf(stderr, "No SPARC_RELOC_PAIR relocation "
		      "entry after entry %u\n", i);
	      continue;
	    }
	  }
	}
	reloc_found = 1;
	break;
      }

      if (r_type == SPARC_RELOC_HI22 ||
	  r_type == SPARC_RELOC_LO10 ||
	  r_type == SPARC_RELOC_SECTDIFF ||
	  r_type == SPARC_RELOC_HI22_SECTDIFF ||
	  r_type == SPARC_RELOC_LO10_SECTDIFF)
	{
	  if (i+1 < nrelocs) {
	    rp_pair = (rp + 1);
	    if (rp_pair->r_address & R_SCATTERED) {
	      srp = (struct scattered_relocation_info *) rp_pair;
	      pair_r_type = srp->r_type;
	    } else {
	      pair_r_type = rp_pair->r_type;
	    }
	    if (pair_r_type == SPARC_RELOC_PAIR) {
	      i++;
	      rp++;
	    } else {
	      fprintf(stderr, 
		      "no SPARC_RELOC_PAIR relocation entry after %u\n", i);
	    }
	  }
	}
      }
    }

    /* Now get the reloc information if we located one */

    if (reloc_found && (r_extern == 1)) {
      if ((uint32_t)symbols[r_symbolnum].n_un.n_strx >= strings_size)
	name = "bad string offset";
      else
	name = strings + symbols[r_symbolnum].n_un.n_strx;

      switch (r_type) {
      case SPARC_RELOC_HI22:
	value += other_half;
	printf("%%hi(%s", name);
	if (value)
	  printf("+0x%x)", (unsigned int) value);
	else
	  printf(")");
	break;
      case SPARC_RELOC_LO10:
	value |= other_half << 10;
	printf("%%lo(%s", name);
	if (value)
	  printf("+0x%x)", (unsigned int) value);
	else
	  printf(")");
	break;
      case SPARC_RELOC_WDISP30:
	printf("%s", name);
	if (value)
	  printf("+0x%x", (unsigned int) value);
	break;
      case SPARC_RELOC_WDISP22:
	printf("%s", name);
	if (value)
	  printf("+0x%x", (unsigned int) value);
	break;
      default:
	printf("%s", name);
      }
      return;
    }

    offset = 0;
    if(reloc_found){
      if(r_type == SPARC_RELOC_HI22 ||
	 r_type == SPARC_RELOC_HI22_SECTDIFF)
	value |= other_half;
      else if(r_type == SPARC_RELOC_LO10 ||
	      r_type == SPARC_RELOC_LO10_SECTDIFF)
	value |= other_half << 10;
      if(r_scattered &&
	 (r_type != SPARC_RELOC_HI22_SECTDIFF &&
	  r_type != SPARC_RELOC_LO10_SECTDIFF)){
	offset = value - r_value;
	value = r_value;
      }
    }
    
    if (reloc_found &&
	(r_type == SPARC_RELOC_HI22_SECTDIFF ||
	 r_type == SPARC_RELOC_LO10_SECTDIFF)) {
      if (r_type == SPARC_RELOC_HI22_SECTDIFF)
	printf("%%hi(");
      else
	printf("%%lo(");
      add = guess_symbol (r_value, sorted_symbols,
			  nsorted_symbols, verbose);
      sub = guess_symbol (pair_r_value, sorted_symbols,
			  nsorted_symbols, verbose);
      if (add)
	printf("%s", add);
      else
	printf("0x%x", (unsigned int) r_value);

      if (sub)
	printf("-%s", sub);
      else
	printf("-0x%x", (unsigned int) pair_r_value);

      if (offset)
	printf("+0x%x)", (unsigned int) offset);
      else
	printf(")");
      return;
    }

    /* no reloc entry, so it's either a symbol or a label */

    if (operand == 'l' || operand == 'L') {
/*
The pc has already been added in the 'l' and 'L' cases by the caller.
      if ((name = guess_symbol(pc + value, sorted_symbols, 
			       nsorted_symbols, verbose)) != NULL)
*/
      if ((name = guess_symbol(value, sorted_symbols, 
			       nsorted_symbols, verbose)) != NULL)
	printf("%s", name);
      else
	printf("0x%x", (unsigned int) value);
    } else {
      /* possible symbol reference */
      name = guess_symbol(value, sorted_symbols, nsorted_symbols, TRUE);

      switch (r_type) {
      case SPARC_RELOC_HI22:
	
	/* to print the correct value, we really should look ahead for
	   %lo portion of the immediate value and reconstruct it instead
	   of simply printing the already truncated %hi value only */
	if (name)
	  printf("%%hi(%s)", name);
	else
	  printf("%%hi(0x%x)", (unsigned int) value);
	break;
      case SPARC_RELOC_LO10:
	
	/* same here. Look at the last remembered %hi value and add it in */
	if (name)
	  printf("%%lo(%s)", name);
	else
	  printf("%%lo(0x%x)", (unsigned int) value);
	break;
      default:
	if (name)
	  printf("%s", name);
	else
	  printf("0x%x", (unsigned int) value);
      }
    }
}
       
uint32_t
sparc_disassemble(
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
uint32_t *indirect_symbols,
uint32_t nindirect_symbols,
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum bool verbose)
{
  enum byte_sex host_byte_sex;
  enum bool swapped;
  int i;
  uint32_t sect_offset;
  union sparc_insn	insn;
  struct sparc_opcode	*opcode;
  int imm_added_to_rs1 = 0;	/* adding or or'ing imm13 into rs1? */
  int found_plus = 0;	/* Found plus sign in args? */
  int is_annulled = 0;	/* We have an annulled branch? */
  const char *s;
  static union sparc_insn sethi_insn;
  const char *indirect_symbol_name;
  
  
  opcode = NULL;
  if (!opcodes_sorted) {
    qsort ((char *) sparc_opcodes, NUMOPCODES,
	   sizeof (sparc_opcodes[0]), compare_opcodes);
    opcodes_sorted = 1;
  }
  
  sect_offset = addr - sect_addr;
  host_byte_sex = get_host_byte_sex();
  swapped = host_byte_sex != object_byte_sex;
  
  if (left < sizeof(uint32_t)) {
    if(left != 0) {
      memcpy(&insn, sect, left);
      if(swapped)
	insn.code = SWAP_INT(insn.code);
      printf(".long\t0x%08x\n", (unsigned int)insn.code);
    }
    printf("(end of section)\n");
    return(left);
  }
  
  memcpy(&insn, sect, sizeof(uint32_t));
  if (swapped)
    insn.code = SWAP_INT(insn.code);
  
  /* search through the opcode table */
  for (i=0; i<NUMOPCODES; ++i) {
    opcode = &sparc_opcodes[i];
    if ((opcode->match & insn.code) == opcode->match
	&& (opcode->lose & insn.code) == 0) {
      /* Can't do simple format if source and dest are different.  */
      /* This also takes care of the synthetic 'neg' instruction */
      if ((insn.rs1 != insn.rd
	  && strchr (opcode->args, 'r') != 0) ||
	  (insn.rs2 != insn.rd && 
	   strchr (opcode->args, 'u'))) {
	continue;
      }
      else {
	break;
      }
    }
  }
  
  if (i >= NUMOPCODES) {
    printf(".long 0x%08x\n", (unsigned int)insn.code);
    return(4);
  }
  
  printf("%s", opcode->name);

  /* Do we have an `add' or `or' instruction where rs1 is the same
     as rsd, and which has the i bit set?  */
  /* Check for st instruction as well, as it's commonly used with %lo */
  if ((opcode->match == 0x80102000 || /* or */
       opcode->match == 0x80002000 || /* add */
       opcode->match == 0xd0202000 ) /* st */
      /* && insn.rs1 == insn.rd */)
    imm_added_to_rs1 = 1;
  
  if (opcode->args[0] != ',')
    printf("\t");
  for (s = opcode->args; *s != '\0'; ++s) {
    while (*s == ',') {
      printf(",");
      ++s;
      switch (*s) {
      case 'a':
	printf("a\t");
	is_annulled = 1;
	++s;
	continue;
      default:
	break;
      }	/* switch on arg */
    } /* while there are comma started args */
	
    switch (*s) {
    case '+':
      found_plus = 1;
      /* fall through */
    default:
      printf("%c", *s);
      break;
    case '#':
      printf("0");
      break;

#define	reg(n)	printf("%%%s", reg_names[n])
    case '1':
    case 'r':
      reg (insn.rs1);
      break;

    case '2':
      reg (insn.rs2);
      break;

    case 'u':
      reg (insn.rs2);
      break;

    case 'd':
      reg (insn.rd);
      break;
#undef	reg

#define	freg(n)		printf("%%%s", freg_names[n])
#define	fregx(n)	printf("%%%s", reg_names[((n) & ~1) | (((n) & 1) << 5)])
    case 'e':
      freg (insn.rs1);
      break;
    case 'v':			/* double/even */
    case 'V':			/* quad/multiple of 4 */
      fregx (insn.rs1);
      break;

    case 'f':
      freg (insn.rs2);
      break;
    case 'B':			/* double/even */
    case 'R':			/* quad/multiple of 4 */
      fregx (insn.rs2);
      break;

    case 'g':
      freg (insn.rd);
      break;
    case 'H':			/* double/even */
    case 'J':			/* quad/multiple of 4 */
      fregx (insn.rd);
      break;
#undef	freg
#undef	fregx

#define	creg(n)	printf("%%c%u", (unsigned int) (n))
    case 'b':
      creg (insn.rs1);
      break;

    case 'c':
      creg (insn.rs2);
      break;

    case 'D':
      creg (insn.rd);
      break;
#undef	creg
    case 'h':
      sethi_insn.code = insn.code; /* for later use */
      SYM_PRINT(*s ,((int) insn.imm22<<10), sect_offset);
      break;
    case 'i':
      {
	/* We cannot trust the compiler to sign-extend
	   when extracting the bitfield, hence the shifts.  */
	int imm = SEX (insn.imm13, 13);

	/* Check to see whether we have a 1+i, and take
	   note of that fact.
	   
	   Note: because of the way we sort the table,
	   we will be matching 1+i rather than i+1,
	   so it is OK to assume that i is after +,
	   not before it.  */
	if (found_plus)
	  imm_added_to_rs1 = 1;

	printf("0x%x", imm);
      }
      break;
    case 'M':
      printf("%%asr%d", insn.rs1);
      break;
    case 'm':
      printf("%%asr%d", insn.rd);
      break;
    case 'L':
      SYM_PRINT(*s,addr + ((insn.disp30 & 0x3fffffff) << 2), sect_offset);
      if(verbose){
	indirect_symbol_name = guess_indirect_symbol(
	    addr + ((insn.disp30 & 0x3fffffff) << 2),
	    ncmds, sizeofcmds, load_commands, object_byte_sex, indirect_symbols,
	    nindirect_symbols, symbols, NULL, nsymbols, strings, strings_size);
	if(indirect_symbol_name != NULL)
	    printf("\t; symbol stub for: %s", indirect_symbol_name);
      }
      break;
    case 'n':
      printf("%#x", (SEX (insn.disp22, 22)));
      break;
    case 'l':
      SYM_PRINT(*s,addr + (SEX(insn.disp22, 22) << 2), sect_offset);
      break;
    case 'A':
      printf("(%d)", (int) insn.asi);
      break;
    case 'C':
      printf("%%csr");
      break;
    case 'F':
      printf("%%fsr");
      break;
    case 'p':
      printf("%%psr");
      break;
    case 'q':
      printf("%%fq");
      break;
    case 'Q':
      printf("%%cq");
      break;
    case 't':
      printf("%%tbr");
      break;
    case 'w':
      printf("%%wim");
      break;
    case 'y':
      printf("%%y");
      break;
    }
  }
  
  /* If we are adding or or'ing something to rs1, then
     check to see whether the previous instruction was
     a sethi to the same register as in the sethi.
     If so, attempt to print the result of the add or
     or (in this context add and or do the same thing)
     and its symbolic value.  */
  if (imm_added_to_rs1) {
    union sparc_insn prev_insn;

    /* Check if there is a pending sethi instruction. If so, check if
       the insn uses the same r1 register - that's a good indication
       this is the %lo we're waiting for. The prolem is that the 
       instruction containg the %lo may have been optimized away by
       the assembler. There is also no telling how many intermediate
       instructions the compiler (or human) has put between the
       sethi and %lo. */

    if (sethi_insn.code) 	/* got a pending one */
      if (insn.rs1 == sethi_insn.rs1) { 	/* getting closer */
/*	SYM_PRINT(*s ,((int) insn.imm13), sect_offset); */
	sethi_insn.code = 0;
      }


    if ((sect_addr - addr) >= 8) // space before us this?
      memcpy(&prev_insn, sect - 4, sizeof(uint32_t));
    else
      prev_insn.code = 0;

    /* If it is a delayed branch, we need to look at the
       instruction before the delayed branch.  This handles
       sequences such as
       
       sethi %o1, %hi(_foo), %o1
       call _printf
       or %o1, %lo(_foo), %o1
       */

    if (prev_insn.code		/* && is_delayed_branch (prev_insn) */)
      memcpy(&prev_insn, sect - 8, sizeof(uint32_t));
    else
      prev_insn.code = 0;

    /* Is it sethi to the same register?  */
    if ((prev_insn.code & 0xc1c00000) == 0x01000000
	&& prev_insn.rd == insn.rs1) {
      printf("\t! ");
      print_address_func(
			 (0xFFFFFFFF & (int) prev_insn.imm22 << 10)
			 | SEX (insn.imm13, 13));
    }
  }
  printf("\n");
  return (4);
}

/* Compare opcodes A and B.  */

static int
compare_opcodes (a, b)
     const void *a, *b;
{
  struct sparc_opcode *op0 = (struct sparc_opcode *) a;
  struct sparc_opcode *op1 = (struct sparc_opcode *) b;
  uint32_t match0 = op0->match, match1 = op1->match;
  uint32_t lose0 = op0->lose, lose1 = op1->lose;
  register unsigned int i;

  /* If a bit is set in both match and lose, there is something
     wrong with the opcode table.  */
  if (match0 & lose0)
    {
      fprintf (stderr, "Internal error:  bad sparc-opcode.h: \"%s\", %#.8x, %#.8x\n",
	       op0->name, match0, lose0);
      op0->lose &= ~op0->match;
      lose0 = op0->lose;
    }

  if (match1 & lose1)
    {
      fprintf (stderr, "Internal error: bad sparc-opcode.h: \"%s\", %#.8x, %#.8x\n",
	       op1->name, match1, lose1);
      op1->lose &= ~op1->match;
      lose1 = op1->lose;
    }

  /* Because the bits that are variable in one opcode are constant in
     another, it is important to order the opcodes in the right order.  */
  for (i = 0; i < 32; ++i)
    {
      uint32_t x = 1 << i;
      int x0 = (match0 & x) != 0;
      int x1 = (match1 & x) != 0;

      if (x0 != x1)
	return x1 - x0;
    }

  for (i = 0; i < 32; ++i)
    {
      uint32_t x = 1 << i;
      int x0 = (lose0 & x) != 0;
      int x1 = (lose1 & x) != 0;

      if (x0 != x1)
	return x1 - x0;
    }

  /* They are functionally equal.  So as long as the opcode table is
     valid, we can put whichever one first we want, on aesthetic grounds.  */

  /* Our first aesthetic ground is that aliases defer to real insns.  */
  {
    int alias_diff = (op0->flags & F_ALIAS) - (op1->flags & F_ALIAS);
    if (alias_diff != 0)
      /* Put the one that isn't an alias first.  */
      return alias_diff;
  }

  /* Except for aliases, two "identical" instructions had
     better have the same opcode.  This is a sanity check on the table.  */
  i = strcmp (op0->name, op1->name);
  if (i){
      if (op0->flags & F_ALIAS) /* If they're both aliases, be arbitrary. */
	  return i;
      else
	  fprintf (stderr,
		   "Internal error: bad sparc-opcode.h: \"%s\" == \"%s\"\n",
		   op0->name, op1->name);
  }

  /* Fewer arguments are preferred.  */
  {
    int length_diff = (int)(strlen(op0->args)) - ((int)strlen(op1->args));
    if (length_diff != 0)
      /* Put the one with fewer arguments first.  */
      return length_diff;
  }

  /* Put 1+i before i+1.  */
  {
    char *p0 = (char *) strchr(op0->args, '+');
    char *p1 = (char *) strchr(op1->args, '+');

    if (p0 && p1)
      {
	/* There is a plus in both operands.  Note that a plus
	   sign cannot be the first character in args,
	   so the following [-1]'s are valid.  */
	if (p0[-1] == 'i' && p1[1] == 'i')
	  /* op0 is i+1 and op1 is 1+i, so op1 goes first.  */
	  return 1;
	if (p0[1] == 'i' && p1[-1] == 'i')
	  /* op0 is 1+i and op1 is i+1, so op0 goes first.  */
	  return -1;
      }
  }

  /* They are, as far as we can tell, identical.
     Since qsort may have rearranged the table partially, there is
     no way to tell which one was first in the opcode table as
     written, so just say there are equal.  */
  return 0;
}
