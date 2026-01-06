#include "string.h"
#include "as.h"
#include "flonum.h"
#include "expr.h"
#include "fixes.h"
#include "relax.h"
#include "i386.h"

#include "i386-opcode.h"
/* these are to get rid of the compiler "defined but not used" messages */
const seg_entry *use_it2 = &cs;
const seg_entry *use_it3 = &es;
const seg_entry *use_it4 = &fs;
const seg_entry *use_it5 = &gs;

static
char *
get_suffix(
uint32_t type,
uint32_t opcode_modifier)
{
#define NoSuf (No_bSuf|No_wSuf|No_lSuf|No_sSuf|No_xSuf|No_qSuf)
	if((opcode_modifier & NoSuf) == NoSuf)
	    return("");

	switch(type){
	case Disp8:
	   if(opcode_modifier & No_bSuf)
		return("");
	   return("b");

	case Imm8:
	case Imm8S:
	   return("b");

	case Imm16:
	case Disp16:
	case InOutPortReg:
	    if(opcode_modifier & No_wSuf)
		return("");
	    return("w");

	case Disp32:
	case Disp32S:
	case BaseIndex:
	   if(opcode_modifier & No_lSuf)
		return("");
	   return("l");

	case Imm32:
	case Imm32S:	
	case Imm1:
	   return("l");
/*
	case Mem8:	return("b");
	case Mem16:	return("w");
	case Mem32:	return("l");
*/
	default:	return("");
	}
}

static char *Reg8_table[] = { "%bl", NULL };
static char *Reg16_table[] = { "%bx", NULL };
static char *Reg32_table[] = { "%ecx", NULL };
static char *Reg64_table[] = { "%r13", NULL };
static char *Imm8_table[] = { "$0x7f", NULL };
static char *Imm8_aad_aam_table[] = { "$0x8", NULL };
static char *Imm8S_table[] = { "$0xfe", NULL };
static char *Imm16_table[] = { "$0x7ace", NULL };
static char *Imm16_8_table[] = { "$0x6e", NULL };
static char *Imm32_table[] = { "$0x7afebabe", NULL };
static char *Imm32_8table[] = { "$0xbe", NULL };
static char *Imm32_16table[] = { "$0xbabe", NULL };
static char *Imm32S_table[] = { "$0x13572468", NULL };
static char *Imm32S16_table[] = { "$0x2468", NULL };
static char *Imm32S8_table[] = { "$0x68", NULL };
static char *Imm64_table[] = { "$0xfeedfacecafebabe", NULL };
static char *Imm1_table[] = { "$0", "$1", NULL };
static char *Disp8_table[] = { "0x45", NULL };
static char *Disp16_table[] = { "0x7eed", NULL };
static char *Disp32_table[] = { "0xbabecafe", NULL };
static char *Disp32S_table[] = { "0x12345678", NULL };
static char *Disp64_table[] = { "0xfeedfacebabecafe", NULL };
/*
static char *Mem8_table[] = { "0x88888888", NULL };
static char *Mem16_table[] = { "0x1616", NULL };
static char *Mem32_table[] = { "0x32323232", NULL };
*/
#ifdef ARCH64
static char *BaseIndex_table[] = { "0xdeadbeef(%rbx,%rcx,8)", NULL };
#else
static char *BaseIndex_table[] = { "0xdeadbeef(%ebx,%ecx,8)", NULL };
#endif
static char *InOutPortReg_table[] = { "%dx", NULL };
static char *ShiftCount_table[] = { "%cl", NULL };
static char *Control_table[] = { "%cr0", NULL };
static char *Debug_table[] = { "%db0", NULL };
static char *Test_table[] = { "%tr3", NULL };
static char *FloatReg_table[] = { "%st(2)", NULL };
static char *FloatAcc_table[] = { "%st", NULL };
static char *SReg2_table[] = { "%ds", NULL };
static char *SReg3_table[] = { "%fs", NULL };
static char *Acc_table[] = { "%eax", NULL };
static char *Acc8_table[] = { "%al", NULL };
static char *Acc16_table[] = { "%ax", NULL };
static char *Acc64_table[] = { "%rax", NULL };
static char *JumpAbsolute_table[] = { "*0xbadeface", NULL };
static char *RegMMX_table[] = { "%mm3", NULL };
static char *RegXMM_table[] = { "%xmm5", NULL };
/*
static char *Abs8_table[] = { "0xab", NULL };
static char *Abs16_table[] = { "0xabcd", NULL };
static char *Abs32_table[] = { "0xabcdef01", NULL };
*/
static char *hosed_table[] = { "hosed", NULL };

static
char **
get_operand(
uint32_t type0,
uint32_t type1)
{
	switch(type0){
/* These constants come from i386.h line 188 after the following comment: */
/* operand_types[i] bits */
	case Reg8:	return(Reg8_table);
	case Reg16:	return(Reg16_table);
	case Reg32:	return(Reg32_table);
	case Reg64:	return(Reg64_table);
	case Imm8:	return(Imm8_table);
	case Imm8S:	return(Imm8S_table);
	case Imm16:
	    switch(type1){
	    case Reg8:
		return(Imm16_8_table);
	    default:
		return(Imm16_table);
	    }
	case Imm32:
	    switch(type1){
	    case Reg8:
		return(Imm32_8table);
	    case Reg16:
		return(Imm32_16table);
	    default:
		return(Imm32_table);
	    }
	case Imm32S:
	    switch(type1){
	    case Reg8:
		return(Imm32S8_table);
	    case Reg16:
		return(Imm32S16_table);
	    default:
		return(Imm32S_table);
	    }
	case Imm64:	return(Imm64_table);
	case Imm1:	return(Imm1_table);
	case Disp8:	return(Disp8_table);
	case Disp16:	return(Disp16_table);
	case Disp32:	return(Disp32_table);
	case Disp32S:	return(Disp32S_table);
	case Disp64:	return(Disp64_table);
/*
	case Mem8:	return(Mem8_table);
	case Mem16:	return(Mem16_table);
	case Mem32:	return(Mem32_table);
*/
	case BaseIndex:	return(BaseIndex_table);
	case InOutPortReg:	return(InOutPortReg_table);
	case ShiftCount:	return(ShiftCount_table);
	case Control:	return(Control_table);
	case Debug:	return(Debug_table);
	case Test:	return(Test_table);
	case FloatReg:	return(FloatReg_table);
	case FloatAcc:	return(FloatAcc_table);
	case SReg2:	return(SReg2_table);
	case SReg3:	return(SReg3_table);
	case Acc:
	    switch(type1){
	    case Reg8:
		return(Acc8_table);
	    case Reg16:
		return(Acc16_table);
	    case Reg64:
		return(Acc64_table);
	    default:
		return(Acc_table);
	    }
	case JumpAbsolute:	return(JumpAbsolute_table);
	case RegMMX:	return(RegMMX_table);
	case RegXMM:	return(RegXMM_table);
/*
	case Abs8:	return(Abs8_table);
	case Abs16:	return(Abs16_table);
	case Abs32:	return(Abs32_table);
*/
	default:	return(hosed_table);
	}
}

int
main(
int argc,
char **argv)
{
    const template *t;

    uint32_t i, j, type0, type1, llvm_mc, bad_reg;
    char **op0, **op1;
    char *suffix;

	llvm_mc = 0;
	if(argc != 1){
	    if(argc == 2 && strcmp(argv[1], "-llvm-mc") == 0)
		llvm_mc = 1;
	}
	for(t = i386_optab; t->name != NULL ; t++){
	    /*
	     * If producing tests for llvm-mc don't use test registers.
	     * 
	     * These are "Test Registers" and these only show up in the i486
	     * book (see page 4-80) and the Move to/from Special Register page
	     * 26-213.  In i386-opcode.h they are not correct as they use the
	     * opcodes 0f 24 where the book also uses 0f 26.  And they are only
	     * TR3-TR7 not TR0-TR7.
	     */
	    if(llvm_mc){
		bad_reg = 0;
		if(t->operands >= 1)
		    bad_reg |= t->operand_types[0] & Test;
		if(t->operands >= 2)
		    bad_reg |= t->operand_types[1] & Test;

		/* Hack to pull out segment register operands for now */
		if(t->operands >= 1){
		    bad_reg |= t->operand_types[0] & SReg2;
		    bad_reg |= t->operand_types[0] & SReg3;
		}
		if(t->operands >= 2){
		    bad_reg |= t->operand_types[1] & SReg2;
		    bad_reg |= t->operand_types[1] & SReg3;
		}

		/* Hack to pull out control register operands for now */
		if(t->operands >= 1)
		    bad_reg |= t->operand_types[0] & Control;
		if(t->operands >= 2)
		    bad_reg |= t->operand_types[1] & Control;

		if(bad_reg)
		    continue;
	    }
	    /*
	     * Don't use the table entries that are prefixes and not
	     * instructions.
	     */
	    if(t->opcode_modifier & IsPrefix)
		continue;
	    /*
	     * The string instructions with operands take only specific
	     * operands and are not checked here.
	     */
	    if(t->opcode_modifier & IsString)
		continue;

#ifdef ARCH64
	    if(t->cpu_flags & CpuNo64)
		continue;
#else
	    if(t->cpu_flags & Cpu64)
		continue;
	    if(t->cpu_flags & CpuK6)
		continue;
	    if(t->cpu_flags & CpuSledgehammer)
		continue;
#endif
	   
	    if(t->operands == 0){
		if((t->opcode_modifier & W) == 0) {
		    printf("\t%s\n", t->name);
		}
		else{
		    printf("\t%sb\n", t->name);
		    printf("\t%sw\n", t->name);
		    printf("\t%sl\n", t->name);
		}
	    }

	    if(t->operands == 1){
		for(i = 0; i < 32; i++){
		    type0 = 1 << i;
		    if((type0 & t->operand_types[0]) == 0)
			continue;

#ifndef ARCH64
		    if(type0 == Reg64)
			continue;
#endif
/* for now do not deal with operands of this and up */
if(type0 >= EsSeg)
    continue;

		    /* These only take byte displacement */
		    if((t->opcode_modifier & JumpByte) &&
		       (type0 == Disp16 || type0 == Disp32))
			continue;

		    if(type0 == Disp8 &&
		       ((t->operand_types[0] & (Disp16 | Disp32)) != 0))
			continue;

		    suffix = "";
		    if((type0 & AnyMem) != 0 || (type0 & EncImm) != 0 ||
		       type0 == InOutPortReg)
			suffix = get_suffix(type0, t->opcode_modifier);

#ifdef ARCH64
		    /*
		     * For 64-bit push & pop defaults to 64-bits and takes no
		     * suffix.
		     */
		    if(strcmp(t->name, "push") == 0 ||
		       strcmp(t->name, "pop") == 0)
			suffix = "";
#endif

		    /* more opcodes that don't want suffixes */
		    if((strcmp(t->name, "call") == 0 ||
		        strcmp(t->name, "lcall") == 0) &&
			strcmp(suffix, "l") == 0)
			suffix = "";
		    if(strcmp(t->name, "jmp") == 0 ||
		       strcmp(t->name, "jecxz") == 0 ||
		       strcmp(t->name, "jrcxz") == 0 ||
		       t->opcode_modifier & Jump)
			suffix = "";
		    if(strncmp(t->name, "set", 3) == 0)
			suffix = "";

		    /*
		     * This is to avoid the problem with the
		     * fildll opcode which is a fildq and
		     * fistpll opcode which is a fistpq
		     */
		    if((strcmp(t->name, "fildl") == 0 ||
			strcmp(t->name, "fistpl") == 0) &&
			strcmp(suffix, "l") == 0)
			suffix = "";

		    /*
		     * This is to avoid the problems with the
		     * fisttpl opcode and the fisttpll opcodes.
		     */
		    if((strcmp(t->name, "fisttpl") == 0 ||
			strcmp(t->name, "fisttpll") == 0))
			continue;
		
		    /* fwait prefixed instructions */
		    if((t->base_opcode & 0xff00) == 0x9b00 &&
		       strcmp(suffix, "w") == 0)
			continue;


		    op0 = get_operand(type0, 0);
		
		    /* aad & aam can only take 0x8, 0xa or 0xc */
		    if(type0 == Imm8S &&
		       (strcmp(t->name, "aad") == 0 ||
		        strcmp(t->name, "aam") == 0))
			op0 = Imm8_aad_aam_table;

		    /*
		     * TODO the jecxz, jrcxz and loop instructions only have
		     * a byte displacement. Can't do them with really without
		     * a label or address that is close.
		     */
		    if(t->opcode_modifier & JumpByte)
			continue;
			
		    for( ; *op0; op0++){
			if(((strcmp(t->name, "call") == 0 ||
			     strcmp(t->name, "jmp") == 0) &&
			   (type0 == BaseIndex || type0 == Reg16 ||
			    type0 == Reg32 || type0 == Reg64)) ||
			    ((strcmp(t->name, "lcall") == 0 ||
			      strcmp(t->name, "ljmp") == 0) &&
			     type0 != JumpAbsolute))
			    printf("\t%s%s\t*%s\n", t->name, suffix, *op0);
			else
			    printf("\t%s%s\t%s\n", t->name, suffix, *op0);
		    }
		}
	    }

	    if(t->operands == 2){
		for(i = 0; i < 32; i++){
		    type0 = 1 << i;
		    if((type0 & t->operand_types[0]) == 0)
			continue;
#ifndef ARCH64
		    if(type0 == Reg64)
			continue;
#endif
/* for now do not deal with operands of this and up */
if(type0 >= EsSeg)
    continue;
		    for(j = 0; j < 32; j++){
			type1 = 1 << j;
			if((type1 & t->operand_types[1]) == 0)
			    continue;
#ifndef ARCH64
			if(type1 == Reg64)
			    continue;
#endif
/* for now do not deal with operands of this and up */
if(type1 >= EsSeg)
    continue;
			if((type0 & Reg) != 0 && (type1 & Reg) != 0)
			    if(type0 != type1)
				continue;

			suffix = "";
			if((type0 & (Imm|Imm1)) != 0 && (type1 & AnyMem) != 0)
			    suffix = get_suffix(type0, t->opcode_modifier);
			if((type0 & AnyMem) != 0 && (type1 & (Imm|Imm1)) != 0)
			    suffix = get_suffix(type1, t->opcode_modifier);

			if(strncmp(t->name, "bt", 2) == 0)
			    suffix = "";

			op0 = get_operand(type0, type1);
			op1 = get_operand(type1, type0);

			/* hack since only "mwait %eax,%ecx" is accepted */
			if(strcmp(t->name, "mwait") == 0){
			    op0 = Acc_table;
			    op1 = Reg32_table;
			}
			for( ; *op0; op0++){
			    for( ; *op1; op1++){
				printf("\t%s%s\t%s,%s\n", t->name, suffix,
				       *op0, *op1);
				if(t->opcode_modifier & D){
				    printf("\t%s%s\t%s,%s\n", t->name, suffix,
					   *op1, *op0);
				}
			    }
			}
		    }
		}
	    }
	}
	return(0);
}
