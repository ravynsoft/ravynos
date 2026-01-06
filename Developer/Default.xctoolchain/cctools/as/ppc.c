#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <mach-o/ppc/reloc.h>
#include "ppc-opcode.h"
#include "as.h"
#include "flonum.h"
#include "expr.h"
#include "hash.h"
#include "read.h"
#include "md.h"
#include "obstack.h"
#include "symbols.h"
#include "messages.h"
#include "atof-ieee.h"
#include "input-scrub.h"
#include "sections.h"
#include "dwarf2dbg.h"

/*
 * The assembler can assemble the trailing +/- by setting either the Y-bit or
 * the AT-bits.  The default is setting the Y-bit and is the same as specifying:
 *  -static_branch_prediction_Y_bit
 *	Treat a single trailing '+' or '-' after a conditional PowerPC branch
 *	instruction as a static branch prediction that sets the Y-bit in the
 *	opcode.  Pairs of trailing "++" or "--" always set the AT-bits. This is
 *	the default for Mac OS X.
 * This can be changed by specifying:
 * -static_branch_prediction_AT_bits
 *	Treat a single trailing '+' or '-' after a conditional PowerPC branch
 *	instruction as a static branch prediction sets the AT-bits in the
 *	opcode. Pairs of trailing "++" or "--" always set the AT-bits but with
 *	this option a warning is issued if this syntax is used.  With this flag 
 *	the assembler behaves like the IBM tools.
 */
enum static_branch_prediction {
    STATIC_BRANCH_PREDICTION_Y_BIT,
    STATIC_BRANCH_PREDICTION_AT_BITS
};
static int static_branch_prediction_specified = 0;
static enum static_branch_prediction static_branch_prediction =
    STATIC_BRANCH_PREDICTION_Y_BIT;

/* relocation type for internal assembler use only for LIKELY_{,NOT_}TAKEN */
#define PPC_RELOC_BR14_predicted (0x10 | PPC_RELOC_BR14)
enum branch_prediction {
    BRANCH_PREDICTION_NONE,
    BRANCH_PREDICTION_LIKELY_TAKEN,
    BRANCH_PREDICTION_LIKELY_NOT_TAKEN,
    BRANCH_PREDICTION_VERY_LIKELY_TAKEN,
    BRANCH_PREDICTION_VERY_LIKELY_NOT_TAKEN
};

/*
 * Set if -no_ppc601 is specified or .no_pcc601 is seen.  It flags all 601
 * uses as errors.
 */
static int no_ppc601 = 0;

/*
 * The directive .flag_reg and .noflag_reg use these to flag register usage. 
 */
int flag_registers = 0;
int flag_gregs[32] = { 0 };

/*
 * These are the default cputype and cpusubtype for the ppc architecture.
 */
#ifdef ARCH64
const cpu_type_t md_cputype = CPU_TYPE_POWERPC64;
cpu_subtype_t md_cpusubtype = CPU_SUBTYPE_POWERPC_ALL;
#else
const cpu_type_t md_cputype = CPU_TYPE_POWERPC;
cpu_subtype_t md_cpusubtype = CPU_SUBTYPE_POWERPC_ALL;
#endif

/* This is the byte sex for the ppc architecture */
const enum byte_sex md_target_byte_sex = BIG_ENDIAN_BYTE_SEX;

/* These characters start a comment anywhere on the line */
const char md_comment_chars[] = ";";

/* These characters only start a comment at the beginning of a line */
const char md_line_comment_chars[] = "#";

/*
 * These characters can be used to separate mantissa decimal digits from 
 * exponent decimal digits in floating point numbers.
 */
const char md_EXP_CHARS[] = "eE";

/*
 * The characters after a leading 0 that means this number is a floating point
 * constant as in 0f123.456 or 0d1.234E-12 (see md_EXP_CHARS above).
 */
const char md_FLT_CHARS[] = "dDfF";

/*
 * This is the machine dependent pseudo opcode table for this target machine.
 */
static void s_reg(
    uintptr_t reg);
static void s_no_ppc601(
    uintptr_t ignore);
static void s_flag_reg(
    uintptr_t ignore);
static void s_noflag_reg(
    uintptr_t ignore);
const pseudo_typeS md_pseudo_table[] =
{
    {"greg",		s_reg,		'r' },
    {"no_ppc601",	s_no_ppc601,	0 },
    {"flag_reg",	s_flag_reg,	0 },
    {"noflag_reg",	s_noflag_reg,	0 },
    {"file", (void (*) (uintptr_t)) dwarf2_directive_file, 0},
    {"loc", dwarf2_directive_loc, 0},
    {0} /* end of table marker */
};

#define RT(x)           (((x) >> 21) & 0x1f)
#define RA(x)           (((x) >> 16) & 0x1f)
#define RB(x)           (((x) >> 11) & 0x1f)

struct ppc_insn {
    uint32_t opcode;
    expressionS exp;
    expressionS jbsr_exp;
    int reloc;
    int32_t pcrel;
    int32_t pcrel_reloc;
};

/*
 * The pointer to the opcode hash table built by md_begin() and used by
 * md_assemble() to look up opcodes.
 */
static struct hash_control *op_hash = NULL;

/*
 * These aid in the printing of better error messages for parameter syntax
 * errors when there is only one mnemonic in the tables.
 */
static uint32_t error_param_count = 0;
static char *error_param_message = NULL;

/*
 * These are name names of the known special registers and the numbers assigned
 * to them.
 */
struct special_register {
    uint32_t number;
    char *name;
};
static const struct special_register special_registers[] = {
    { 0,   "mq" },  /* 601 only */
    { 1,   "xer" }, /* user access */
    { 4,   "rtcu" },/* real time counter high (601 only, not in PowerPC arch)*/
    { 5,   "rtcl" },/* real time counter low (601 only, not in PowerPC arch) */
    { 8,   "lr" },  /* user access */
    { 9,   "ctr" }, /* user access */
    { 18,  "dsisr" },
    { 19,  "dar" },
    { 22,  "dec" },
    { 25,  "sdr1" },
    { 26,  "srr0" },
    { 27,  "srr1" },
    { 256, "VRsave" }, /* user access, VMX register save */
    { 272, "sprg0" },
    { 273, "sprg1" },
    { 274, "sprg2" },
    { 275, "sprg3" },
    { 280, "asr" },	/* 64-bit implementaions only */
    { 282, "ear" },	/* optional in the PowerPC architecure */
    { 284, "tbl" },
    { 285, "tbu" },
    { 287, "pvr" },
    { 528, "ibat0u" },
    { 529, "ibat0l" },
    { 530, "ibat1u" },
    { 531, "ibat1l" },
    { 532, "ibat2u" },
    { 533, "ibat2l" },
    { 534, "ibat3u" },
    { 535, "ibat3l" },
    { 536, "dbat0u" },
    { 537, "dbat0l" },
    { 538, "dbat1u" },
    { 539, "dbat1l" },
    { 540, "dbat2u" },
    { 541, "dbat2l" },
    { 542, "dbat3u" },
    { 543, "dbat3l" },
    { 936, "ummcr0" },/* 750 only */
    { 937, "upmc1" }, /* 750 only */
    { 938, "upmc2" }, /* 750 only */
    { 939, "usia" },  /* 750 only */
    { 940, "ummcr1" },/* 750 only */
    { 941, "upmc3" }, /* 750 only */
    { 942, "upmc4" }, /* 750 only */
    { 952, "mmcr0" }, /* 604, 604e, 750 only */
    { 953, "pmc1" },  /* 604, 604e & 750 only */
    { 954, "pmc2" },  /* 604, 604e & 750 only */
    { 955, "sia" },   /* 604, 604e & 750 only */
    { 956, "mmcr1" }, /* 604e & 750 only */
    { 957, "pmc3" },  /* 604e & 750 only */
    { 958, "pmc4" },  /* 604e & 750 only */
    { 959, "sda" },   /* 604 & 604e only */
    { 976, "dmiss" }, /* 603 only */
    { 977, "dcmp" },  /* 603 only */
    { 978, "hash1" }, /* 603 only */
    { 979, "hash2" }, /* 603 only */
    { 980, "imiss" }, /* 603 only */
    { 981, "icmp" },  /* 603 only */
    { 982, "rpa" },   /* 603 only */
    { 1008,"hid0" }, /* 601, 603, 603e, 604, 604e, 750 only */
    { 1009,"hid1" }, /* 601, 603e, 604e 750 only */
    { 1010,"hid2" }, /* 601 */
    { 1010,"iabr" }, /* 601, 603, 603e, 604, 604e & 750 only */
    { 1013,"hid5" }, /* 601 only */
    { 1013,"dabr" }, /* optional in the PowerPC architecure (604, 604e & 750) */
    { 1017,"l2cr" }, /* 750 only */
    { 1019,"ictc" }, /* 750 only */
    { 1020,"thrm1" },/* 750 only */
    { 1021,"thrm2" },/* 750 only */
    { 1022,"thrm3" },/* 750 only */
    { 1023,"hid15" }, /* 601 only */
    { 1023,"pir" }, /* 601, 604 & 604e only */
    { 0, "" } /* end of table marker */
};

/*
 * These are name names of the condition field special registers and the
 * numbers assigned to them.
 */
struct condition_symbol {
    uint32_t value;
    char *name;
};
static const struct condition_symbol condition_symbols[] = {
    { 0, "lt" }, /* less than */
    { 1, "gt" }, /* greater than */
    { 2, "eq" }, /* equal */
    { 3, "so" }, /* summary overflow */
    { 3, "un" }, /* unordered */
    { 0, "" } /* end of table marker */
};

struct CR_field {
    uint32_t value;
    char *name;
};
static const struct CR_field CR_fields[] = {
    { 0,  "cr0" }, /* CR field 0 */
    { 4,  "cr1" }, /* CR field 1 */
    { 8,  "cr2" }, /* CR field 2 */
    { 12, "cr3" }, /* CR field 3 */
    { 16, "cr4" }, /* CR field 4 */
    { 20, "cr5" }, /* CR field 5 */
    { 24, "cr6" }, /* CR field 6 */
    { 28, "cr7" }, /* CR field 7 */
    { 0, "" } /* end of table marker */
};

/*
 * These are built in macros because they are trivial to implement as macros
 * which otherwise be less obvious to do special entries for them.
 */
struct macros {
    char *name;
    char *body;
};
static const struct macros ppc_macros[] = {
    { "mr\n",      "or $0,$1,$1\n" },
    { "mr.\n",     "or. $0,$1,$1\n" },
    { "not\n",     "nor $0,$1,$1\n" },
    { "not.\n",    "nor. $0,$1,$1\n" },
    { "extldi\n",  "rldicr $0,$1,$3,($2)-1\n" },
    { "extldi.\n", "rldicr. $0,$1,$3,($2)-1\n" },
    { "extrdi\n",  "rldicl $0,$1,($2)+($3),64-($2)\n" },
    { "extrdi.\n", "rldicl. $0,$1,($2)+($3),64-($2)\n" },
    { "insrdi\n",  "rldimi $0,$1,64-(($3)+($2)),$3\n" },
    { "insrdi.\n", "rldimi. $0,$1,64-(($3)+($2)),$3\n" },
    { "rotldi\n",  "rldicl $0,$1,$2,0\n" },
    { "rotldi.\n", "rldicl. $0,$1,$2,0\n" },
    { "rotrdi\n",  "rldicl $0,$1,64-($2),0\n" },
    { "rotrdi.\n", "rldicl. $0,$1,64-($2),0\n" },
    { "rotld\n",   "rldcl $0,$1,$2,0\n" },
    { "rotld.\n",  "rldcl. $0,$1,$2,0\n" },
    { "sldi\n",    "rldicr $0,$1,$2,63-($2)\n" },
    { "sldi.\n",   "rldicr. $0,$1,$2,63-($2)\n" },
    { "srdi\n",    "rldicl $0,$1,64-($2),$2\n" },
    { "srdi.\n",   "rldicl. $0,$1,64-($2),$2\n" },
    { "clrldi\n",  "rldicl $0,$1,0,$2\n" },
    { "clrldi.\n", "rldicl. $0,$1,0,$2\n" },
    { "clrrdi\n",  "rldicr $0,$1,0,63-($2)\n" },
    { "clrrdi.\n", "rldicr. $0,$1,0,63-($2)\n" },
    { "clrlsldi\n","rldic $0,$1,$3,($2)-($3)\n" },
    { "clrlsldi.\n","rldic. $0,$1,$3,($2)-($3)\n" },

    { "extlwi\n",  "rlwinm $0,$1,$3,0,($2)-1\n" },
    { "extlwi.\n", "rlwinm. $0,$1,$3,0,($2)-1\n" },
    { "extrwi\n",  "rlwinm $0,$1,($2)+($3),32-($2),31\n" },
    { "extrwi.\n", "rlwinm. $0,$1,($2)+($3),32-($2),31\n" },
    { "inslwi\n",  "rlwimi $0,$1,32-($3),$3,(($3)+($2))-1\n" },
    { "inslwi.\n", "rlwimi. $0,$1,32-($3),$3,(($3)+($2))-1\n" },
    { "insrwi\n",  "rlwimi $0,$1,32-(($3)+($2)),$3,(($3)+($2))-1\n" },
    { "insrwi.\n", "rlwimi. $0,$1,32-(($3)+($2)),$3,(($3)+($2))-1\n" },
    { "rotlwi\n",  "rlwinm $0,$1,$2,0,31\n" },
    { "rotlwi.\n", "rlwinm. $0,$1,$2,0,31\n" },
    { "rotrwi\n",  "rlwinm $0,$1,32-($2),0,31\n" },
    { "rotrwi.\n", "rlwinm. $0,$1,32-($2),0,31\n" },
    { "rotlw\n",   "rlwnm $0,$1,$2,0,31\n" },
    { "rotlw.\n",  "rlwnm. $0,$1,$2,0,31\n" },
    { "slwi\n",    "rlwinm $0,$1,$2,0,31-($2)\n" },
    { "slwi.\n",   "rlwinm. $0,$1,$2,0,31-($2)\n" },
    { "srwi\n",    "rlwinm $0,$1,32-($2),$2,31\n" },
    { "srwi.\n",   "rlwinm. $0,$1,32-($2),$2,31\n" },
    { "clrlwi\n",  "rlwinm $0,$1,0,$2,31\n" },
    { "clrlwi.\n", "rlwinm. $0,$1,0,$2,31\n" },
    { "clrrwi\n",  "rlwinm $0,$1,0,0,31-($2)\n" },
    { "clrrwi.\n", "rlwinm. $0,$1,0,0,31-($2)\n" },
    { "clrlslwi\n","rlwinm $0,$1,$3,($2)-($3),31-($3)\n" },
    { "clrlslwi.\n","rlwinm. $0,$1,$3,($2)-($3),31-($3)\n" },

    { "mtxer\n",   "mtspr 1,$0\n"},
    { "mfxer\n",   "mfspr $0,1\n"},
    { "mtlr\n",    "mtspr 8,$0\n"},
    { "mflr\n",    "mfspr $0,8\n"},
    { "mtctr\n",   "mtspr 9,$0\n"},
    { "mfctr\n",   "mfspr $0,9\n"},
    { "mtdsisr\n", "mtspr 18,$0\n"},
    { "mfdsisr\n", "mfspr $0,18\n"},
    { "mtdar\n",   "mtspr 19,$0\n"},
    { "mfdar\n",   "mfspr $0,19\n"},
    { "mtdec\n",   "mtspr 22,$0\n"},
    { "mfdec\n",   "mfspr $0,22\n"},
    { "mtsdr1\n",  "mtspr 25,$0\n"},
    { "mfsdr1\n",  "mfspr $0,25\n"},
    { "mtsrr0\n",  "mtspr 26,$0\n"},
    { "mfsrr0\n",  "mfspr $0,26\n"},
    { "mtsrr1\n",  "mtspr 27,$0\n"},
    { "mfsrr1\n",  "mfspr $0,27\n"},
    { "mtsprg\n",  "mtspr 272+($0),$1\n"},
    { "mfsprg\n",  "mfspr $0,272+($1)\n"},
    { "mtasr\n",   "mtspr 280,$0\n"},
    { "mfasr\n",   "mfspr $0,280\n"},
    { "mfear\n",   "mfspr $0,282\n"},
    { "mtear\n",   "mtspr 282,$0\n"},
    { "mfpvr\n",   "mfspr $0,287\n"},
    { "mtvrsave\n","mtspr 256,$0\n"},
    { "mtibatu\n", "mtspr 528+2*($0),$1\n"},
    { "mfibatu\n", "mfspr $0,528+2*($1)\n"},
    { "mtibatl\n", "mtspr 529+2*($0),$1\n"},
    { "mfibatl\n", "mfspr $0,529+2*($1)\n"},
    { "mtdbatu\n", "mtspr 536+2*($0),$1\n"},
    { "mfdbatu\n", "mfspr $0,536+2*($1)\n"},
    { "mtdbatl\n", "mtspr 537+2*($0),$1\n"},
    { "mfdbatl\n", "mfspr $0,537+2*($1)\n"},

    { "subi\n",    "addi $0,$1,-($2)\n"},
    { "subis\n",   "addis $0,$1,-($2)\n"},
    { "subic\n",   "addic $0,$1,-($2)\n"},
    { "subic.\n",  "addic. $0,$1,-($2)\n"},

    { "crclr\n",   "crxor $0,$0,$0\n"},
    { "crmove\n",  "cror $0,$1,$1\n"},
    { "crnot\n",   "crnor $0,$1,$1\n"},
    { "crset\n",   "creqv $0,$0,$0\n"},
    { "mtcr\n",    "mtcrf 0xff,$0\n"},
    { "mtfs\n",    "mtfsf 0xff,$0\n"},
    { "mtfs.\n",   "mtfsf. 0xff,$0\n"},

    { "vmr\n",  "vor $0,$1,$1\n"},
    { "vnot\n",   "vnor $0,$1,$1\n"},

    {  "", "" } /* end of table marker */
};

static int calcop(
    struct ppc_opcode *format,
    char *param,
    struct ppc_insn *insn,
    char *op,
    enum branch_prediction prediction);
static char *parse_jbsr(
    char *param,
    struct ppc_insn *insn,
    struct ppc_opcode *format,
    int parcnt);
static char *parse_branch(
    char *param,
    struct ppc_insn *insn,
    struct ppc_opcode *format,
    int parcnt);
static char *parse_displacement(
    char *param,
    struct ppc_insn *insn,
    struct ppc_opcode *format,
    int parcnt);
static char *parse_immediate(
    char *param,
    struct ppc_insn *insn,
    struct ppc_opcode *format,
    int parcnt);
static char *parse_reg(
    char *reg_name,
    char *param,
    struct ppc_insn *insn,
    struct ppc_opcode *format,
    uint32_t parcnt);
static char *parse_spreg(
    char *param,
    struct ppc_insn *insn,
    struct ppc_opcode *format,
    uint32_t parcnt);
static char *parse_bcnd(
    char *param,
    struct ppc_insn *insn,
    struct ppc_opcode *format,
    uint32_t parcnt);
static char *parse_crf(
    char *param,
    struct ppc_insn *insn,
    struct ppc_opcode *format,
    uint32_t parcnt);
static char *parse_num(
    char *param,
    struct ppc_insn *insn,
    struct ppc_opcode *format,
    uint32_t parcnt,
    int32_t max_width_zero,
    int32_t zero_only,
    int32_t signed_num,
    int32_t bit_mask_with_1_bit_set);
static char *parse_mbe(
    char *param,
    struct ppc_insn *insn,
    struct ppc_opcode *format,
    uint32_t parcnt);
static char *parse_sh(
    char *param,
    struct ppc_insn *insn,
    struct ppc_opcode *format,
    uint32_t parcnt);
static char *parse_mb(
    char *param,
    struct ppc_insn *insn,
    struct ppc_opcode *format,
    uint32_t parcnt);

/*
 * md_begin() is called from main() in as.c before assembly begins.  It is used
 * to allow target machine dependent initialization.
 */
void
md_begin(void)
{
    uint32_t i;
    char *name;
    const char *retval;

	/* initialize the opcode hash table */
	op_hash = hash_new();
	if(op_hash == NULL)
	    as_fatal("Could not initialize the opcode hash table");

	/* loop until you see the end of the list */
	i = 0;
	while(*ppc_opcodes[i].name){
	    name = ppc_opcodes[i].name;

	    /* hash each mnemonic and record its position */
	    retval = hash_insert(op_hash, name, (char *)&ppc_opcodes[i]);
	    if(retval != NULL && *retval != '\0')
		as_fatal("Can't hash instruction '%s':%s",
			 ppc_opcodes[i].name, retval);

	    /* skip to next unique mnemonic or end of list */
	    for(i++; strcmp(ppc_opcodes[i].name, name) == 0; i++)
		;
	}

	/*
	 * Load the builtin macros for extended mnemonics for rotate and
	 * shift mnemonics.
	 */
	for(i = 0; *ppc_macros[i].name != '\0'; i++){
	    input_line_pointer = ppc_macros[i].name;
	    s_macro(0);
	    add_to_macro_definition(ppc_macros[i].body);
	    s_endmacro(0);
	}
}

/*
 * md_end() is called from main() in as.c after assembly ends.  It is used
 * to allow target machine dependent clean up.
 */
void
md_end(void)
{
}

/*
 * md_parse_option() is called from main() in as.c to parse target machine
 * dependent command line options.  This routine returns 0 if it is passed an
 * option that is not recognized non-zero otherwise.
 */
int
md_parse_option(
char **argP,
int *cntP,
char ***vecP)
{
	switch(**argP) {
	case 'n':
	    if(strcmp(*argP, "no_ppc601") == 0){
		no_ppc601 = 1;
		*argP = "";
		return(1);
	    }
	    break;
	case 'p':
	    if(strcmp(*argP, "ppcasm") == 0){
		*argP = "";
		return(1);
	    }
	    break;
	case 's':
	    if(strcmp(*argP, "static_branch_prediction_Y_bit") == 0){
		if(static_branch_prediction_specified &&
		   static_branch_prediction != STATIC_BRANCH_PREDICTION_Y_BIT)
		    as_bad("Can't specify both -static_branch_prediction_Y_bit"
			    " and -static_branch_prediction_AT_bits");
		static_branch_prediction_specified = 1;
		static_branch_prediction = STATIC_BRANCH_PREDICTION_Y_BIT;
		*argP = "";
		return(1);
	    }
	    else if(strcmp(*argP, "static_branch_prediction_AT_bits") == 0){
		if(static_branch_prediction_specified &&
		   static_branch_prediction != STATIC_BRANCH_PREDICTION_AT_BITS)
		    as_bad("Can't specify both -static_branch_prediction_Y_bit"
			    " and -static_branch_prediction_AT_bits");
		static_branch_prediction_specified = 1;
		static_branch_prediction = STATIC_BRANCH_PREDICTION_AT_BITS;
		*argP = "";
		return(1);
	    }
	    break;
	}
	return(0);
}

/*
 * s_reg() is used to implement ".greg symbol,exp" which sets symbol to 1 or 0
 * depending on if the expression is a general register.  This is intended for
 * use in macros.
 */
static
void
s_reg(
uintptr_t reg)
{
	char *name, *end_name, delim;
	symbolS *symbolP;
	uint32_t n_value, val;

	if( * input_line_pointer == '"')
	  name = input_line_pointer + 1;
	else
	  name = input_line_pointer;
	delim = get_symbol_end();
	end_name = input_line_pointer;
	*end_name = delim;
	SKIP_WHITESPACE();
	if ( * input_line_pointer != ',' ) {
		*end_name = 0;
		as_bad("Expected comma after name \"%s\"", name);
		*end_name = delim;
		ignore_rest_of_line();
		return;
	}
	input_line_pointer ++;
	*end_name = 0;

	SKIP_WHITESPACE();
	n_value = 0;
	if (*input_line_pointer == reg || *input_line_pointer == toupper(reg)){
	    input_line_pointer++;
	    if(isdigit(*input_line_pointer)){
		val = 0;
		while (isdigit(*input_line_pointer)){
		    if ((val = val * 10 + *input_line_pointer++ - '0') > 31)
			break;
		}
		SKIP_WHITESPACE();
		if(val <= 31 &&
		   (*input_line_pointer == '\n' || *input_line_pointer == '@'))
		    n_value = 1;
	    }
	}

	symbolP = symbol_find_or_make (name);
	symbolP -> sy_type = N_ABS;
	symbolP -> sy_other = 0; /* NO_SECT */
	symbolP -> sy_value = n_value;
	symbolP -> sy_frag = & zero_address_frag;

	*end_name = delim;
	totally_ignore_line();
}

/*
 * s_no_ppc601() inplements .no_ppc601 which causes 601 instructions to be
 * flagged as errors.  This is the same as if -no_ppc601 is specified.
 */
static
void
s_no_ppc601(
uintptr_t ignore)
{
	no_ppc601 = 1;
	totally_ignore_line();
}

/*
 * s_flag_reg() implements .flag_reg <reg_number> so that uses of that register
 * get flagged as warnings.
 */
static
void
s_flag_reg(
uintptr_t ignore)
{
   int reg;

	reg = get_absolute_expression();
	if(reg < 0 || reg >= 32)
	    as_bad("register number (%d) out of range (0-31) for .flag_reg",
		    reg);
	demand_empty_rest_of_line();
	flag_registers = 1;
	flag_gregs[reg] = 1;
}

/*
 * s_noflag_reg() implements .noflag_reg <reg_number> so that uses of that
 * register no longer get flagged as warnings.
 */
static
void
s_noflag_reg(
uintptr_t ignore)
{
   int i, reg;

	reg = get_absolute_expression();
	if(reg < 0 || reg >= 32)
	    as_bad("register number (%d) out of range (0-31) for .noflag_reg",
		    reg);
	demand_empty_rest_of_line();
	flag_gregs[reg] = 0;
	flag_registers = 0;
	for(i = 0; i < 32; i++){
	    if(flag_gregs[i]){
		flag_registers = 1;
		return;
	    }
	}
}

/*
 * md_assemble() is passed a pointer to a string that should be a assembly
 * statement for the target machine.
 */
void
md_assemble(
char *op)
{
    char *param, *thisfrag, *start_op, *end_op;
    enum branch_prediction prediction;
    struct ppc_opcode *format;
    struct ppc_insn insn;
    uint32_t i, val, retry;

    static char *file_spec;
    static uint32_t line_spec;
    static int syntax_warning_issued_for_AT_bits = 0;

	/*
	 * Pick up the instruction and any trailing branch prediction character
	 * (a trailing '+', '-' on the instruction).
  	 */
	prediction = BRANCH_PREDICTION_NONE;
	start_op = op;
	end_op = op;
	for(param = op; !isspace(*param) && *param != '\0' ; param++)
	    end_op = param;
	if(*end_op == '+'){
	    if(end_op != start_op && end_op[-1] == '+'){
		if(static_branch_prediction ==
		   STATIC_BRANCH_PREDICTION_AT_BITS &&
		   syntax_warning_issued_for_AT_bits == 0){
		    as_warn("branch prediction ++/-- syntax always sets the "
			       "AT-bits");
		    syntax_warning_issued_for_AT_bits = 1;
		}
		prediction = BRANCH_PREDICTION_VERY_LIKELY_TAKEN;
		end_op[-1] = '\0';
	    }
	    else{
		if(static_branch_prediction == STATIC_BRANCH_PREDICTION_AT_BITS)
		    prediction = BRANCH_PREDICTION_VERY_LIKELY_TAKEN;
		else
		    prediction = BRANCH_PREDICTION_LIKELY_TAKEN;
		*end_op = '\0';
	    }
	}
	else if(*end_op == '-'){
	    if(end_op != start_op && end_op[-1] == '-'){
		if(static_branch_prediction ==
		   STATIC_BRANCH_PREDICTION_AT_BITS &&
		   syntax_warning_issued_for_AT_bits == 0){
		    as_warn("branch prediction ++/-- syntax always sets the "
			       "AT-bits");
		    syntax_warning_issued_for_AT_bits = 1;
		}
		prediction = BRANCH_PREDICTION_VERY_LIKELY_NOT_TAKEN;
		end_op[-1] = '\0';
	    }
	    else{
		if(static_branch_prediction == STATIC_BRANCH_PREDICTION_AT_BITS)
		    prediction = BRANCH_PREDICTION_VERY_LIKELY_NOT_TAKEN;
		else
		    prediction = BRANCH_PREDICTION_LIKELY_NOT_TAKEN;
		*end_op = '\0';
	    }
	}
	if(*param != '\0')
	    *param++ = '\0';

	/* try to find the instruction in the hash table */
	if((format = (struct ppc_opcode *)hash_find(op_hash, op)) == NULL){
	    as_bad("Invalid mnemonic '%s'", op);
	    return;
	}

	/* try parsing this instruction into insn */
	retry = 0;
	error_param_count = 0;
	error_param_message = NULL;
	while(calcop(format, param, &insn, op, prediction) == 0){
	    /* if it doesn't parse try the next instruction */
	    if(strcmp(format->name, format[1].name) == 0){
		format++;
		retry = 1;
	    }
	    else{
		if(retry == 0){
		    if(error_param_message != NULL)
			as_bad("%s (parameter %u)", error_param_message,
			       error_param_count + 1);
		    else
			as_bad("Parameter syntax error (parameter %u)",
				error_param_count + 1);
		}
		else
		    as_bad("Parameter syntax error");
		return;
	    }
	}

#ifndef ALLOW_INVALID_FORMS
	/*
	 * Check for invalid forms of instructions.  For the following
	 * instructions: lbzu, lbzux, lhzu, lhzux, lhau, lhaux, lwzu, lwzux,
	 * lwaux, ldu, ldux
	 * if RA == 0 or RA == RT the instruction form is invalid.
	 */
	if((insn.opcode & 0xfc000000) == 0x8c000000 || /* lbzu */
	   (insn.opcode & 0xfc0007fe) == 0x7c0000ee || /* lbzux */
	   (insn.opcode & 0xfc000000) == 0xa4000000 || /* lhzu */
	   (insn.opcode & 0xfc0007fe) == 0x7c00026e || /* lbzux */
	   (insn.opcode & 0xfc000000) == 0xac000000 || /* lhau */
	   (insn.opcode & 0xfc0007fe) == 0x7c0002ee || /* lhaux */
	   (insn.opcode & 0xfc000000) == 0x84000000 || /* lwzu */
	   (insn.opcode & 0xfc0007fe) == 0x7c00006e || /* lwzux */
	   (insn.opcode & 0xfc0007fe) == 0x7c0002ea || /* lwaux */
	   (insn.opcode & 0xfc000003) == 0xe8000001 || /* ldu */
	   (insn.opcode & 0xfc0007fe) == 0x7c00006a){  /* ldux */
	    if(RA(insn.opcode) == 0)
		as_bad("Invalid form of the instruction (RA must not be 0)");
	    if(RA(insn.opcode) == RT(insn.opcode))
		as_bad("Invalid form of the instruction (RA must not the same "
			"as RT)");
	}
	/*
	 * For the following instructions: stbu, stbux, sthu, sthux, stwu,
	 * stwux, stdu, stdux, lfsu, lfsux, lfdu, lfdux, stfsu, stfsux, stfdu,
	 * stfdux
	 * if RA == 0 the instruction form is invalid.
	 */
	if((insn.opcode & 0xfc000000) == 0x9c000000 || /* stbu */
	   (insn.opcode & 0xfc0007fe) == 0x7c0001ee || /* stbux */
	   (insn.opcode & 0xfc000000) == 0xb4000000 || /* sthu */
	   (insn.opcode & 0xfc0007fe) == 0x7c00036e || /* sthux */
	   (insn.opcode & 0xfc000000) == 0x94000000 || /* stwu */
	   (insn.opcode & 0xfc0007fe) == 0x7c00016e || /* stwux */
	   (insn.opcode & 0xfc000003) == 0xf8000001 || /* stdu */
	   (insn.opcode & 0xfc0007fe) == 0x7c00016a || /* stdux */
	   (insn.opcode & 0xfc000000) == 0xc4000000 || /* lfsu */
	   (insn.opcode & 0xfc0007fe) == 0x7c00046e || /* lfsux */
	   (insn.opcode & 0xfc000000) == 0xcc000000 || /* lfdu */
	   (insn.opcode & 0xfc0007fe) == 0x7c0004ee || /* lfdux */
	   (insn.opcode & 0xfc000000) == 0xd4000000 || /* stfsu */
	   (insn.opcode & 0xfc0007fe) == 0x7c00056e || /* stfsux */
	   (insn.opcode & 0xfc000000) == 0xdc000000 || /* stfdu */
	   (insn.opcode & 0xfc0007fe) == 0x7c0005ee || /* stfdux */
	   (insn.opcode & 0xfc0007fe) == 0x7c0002ac || /* dst, dstt */
	   (insn.opcode & 0xfc0007fe) == 0x7c0002ec){  /* dstst, dststt */
	    if(RA(insn.opcode) == 0)
		as_bad("Invalid form of the instruction (RA must not be 0)");
	}
	/*
	 * For the following instruction lmw if RA is in the range of
	 * registers to be loaded or RT == RA == 0 the instruction form is
	 * invalid.
	 */
	if((insn.opcode & 0xfc000000) == 0xb8000000){ /* lmw */
	    if(RT(insn.opcode) <= RA(insn.opcode))
		as_bad("Invalid form of the instruction (RA is in the range "
			"of registers to be loaded)");
	}
	/*
 	 * For the lswi instruction if RA is in the range of registers to be
	 * loaded the instruction form is invalid.
	 */
	if((insn.opcode & 0xfc0007fe) == 0x7c0004aa){ /* lswi */
	    uint32_t nb, nr;

		nb = (insn.opcode & 0x0000f800) >> 11;
		if(nb == 0)
		    nb = 32;
		nr = (nb + 3) / 4;
		if(RA(insn.opcode) >= RT(insn.opcode) &&
		   RA(insn.opcode) <= RT(insn.opcode) + nr - 1)
		    as_bad("Invalid form of the instruction (RA is in the "
			    "range of registers to be loaded)");
		if(RT(insn.opcode) + nr - 1 > 31 &&
		   RA(insn.opcode) < (RT(insn.opcode) + nr - 1) - 31)
		    as_bad("Invalid form of the instruction (RA is in the "
			    "range of registers to be loaded)");
	}
	/*
	 * For the lswx instruction if RT == RA or RT == RB the instruction
	 * form is invalid.  Of if RT and RA both specifiy r0 the form is
	 * invalid (covered by the RT == RA case).
	 */
	if((insn.opcode & 0xfc0007fe) == 0x7c00042a){  /* lswx */
	    if(RT(insn.opcode) == RA(insn.opcode))
		as_bad("Invalid form of the instruction (RT must not the same "
			"as RA)");
	    if(RT(insn.opcode) == RB(insn.opcode))
		as_bad("Invalid form of the instruction (RT must not the same "
			"as RB)");
	}
#if !defined(ARCH64)
	/*
	 * The 64-bit compares are invalid on 32-bit implementations.  Since
	 * we don't expect to ever use the 620 all 64-bit instructions require
	 * the -force_cpusubtype_ALL option to not be flagged as invalid.
	 */
	if(((insn.opcode & 0xfc4007ff) == 0x7c000000 ||  /* cmp */
	    (insn.opcode & 0xfc4007ff) == 0x7c000040 ||  /* cmpl */
	    (insn.opcode & 0xfc400000) == 0x2c000000 ||  /* cmpi */
	    (insn.opcode & 0xfc400000) == 0x28000000) && /* cmpli */
	   (insn.opcode & 0x00200000) == 0x00200000 &&   /* the L bit */
	   !force_cpusubtype_ALL &&
	   archflag_cpusubtype != CPU_SUBTYPE_POWERPC_970){
	    as_bad("Invalid form of the instruction (64-bit compares not "
		    "allowed without -force_cpusubtype_ALL option)");
	}
#endif /* !defined(ARCH64) */
	/*
	 * For branch conditional instructions certian BO fields are reserved.
	 * These are flagged as invalid forms unless the -force_cpusubtype_ALL
	 * option is specified.
	 */
	if(((insn.opcode & 0xfc000000) == 0x40000000 ||   /* bc */
	    (insn.opcode & 0xfc00fffe) == 0x4c000420 ||   /* bcctr */
	    (insn.opcode & 0xfc00fffe) == 0x4c000020) &&  /* bclr */
	    !force_cpusubtype_ALL){
	    /*
	     * We have a branch conditional instruction and force_cpusubtype_ALL
	     * is not specified.  So check for reserved BO fields where the z
	     * bits should be zero.
	     */
	    if((insn.opcode & 0x02800000) == 0x02800000 && /* 1z1zz */
	       (insn.opcode & 0x01600000) != 0x00000000){
		as_bad("Invalid form of the instruction (reserved bits in the "
			"BO field must be zero without -force_cpusubtype_ALL "
			"option)");
	    }
	}
#endif /* ALLOW_INVALID_FORMS */

	/*
	 * If the -g flag is present generate a line number stab for the
	 * instruction.
	 * 
	 * See the detailed comments about stabs in read_a_source_file() for a
	 * description of what is going on here.
	 */
	if(flagseen['g'] && frchain_now->frch_nsect == text_nsect){
	    (void)symbol_new(
		  "",
		  68 /* N_SLINE */,
		  text_nsect,
		  logical_input_line /* n_desc, line number */,
		  obstack_next_free(&frags) - frag_now->fr_literal,
		  frag_now);
	}

	/* grow the current frag and plop in the opcode */
	thisfrag = frag_more(4);
	md_number_to_chars(thisfrag, insn.opcode, 4);
	dwarf2_emit_insn(4);

	/*
	 * If we are to flag registers not to be used then check the instruction
	 * we just assembled for registers to be flagged.
	 */
	if(flag_registers){
	    for(i = 0; i < 5; i++){
		if(format->ops[i].type == GREG ||
		   format->ops[i].type == G0REG){
		    val = (insn.opcode & (0x1f << format->ops[i].offset)) >>
			  format->ops[i].offset;
		    if(flag_gregs[val])
			as_bad("flagged register r%u used", val);
		}
	    }
	}

	/*
	 * Deal with the instructions that are for specific cpusubtypes.
	 */
	if(format->cpus != 0 && !force_cpusubtype_ALL){
	    if(no_ppc601 == 1 && format->cpus == CPU601)
		as_bad("not allowed 601 instruction \"%s\"", format->name);
#if !defined(ARCH64)
	    if((format->cpus & IMPL64) == IMPL64
		&& archflag_cpusubtype != CPU_SUBTYPE_POWERPC_970
		){
		as_bad("%s instruction is only for 64-bit implementations (not "
		       "allowed without -force_cpusubtype_ALL option)",
		       format->name);
	    }
	    if((format->cpus & OPTIONAL) == OPTIONAL){
		if((format->cpus & CPU970) == CPU970 &&
		   archflag_cpusubtype != CPU_SUBTYPE_POWERPC_970)
		    as_bad("%s instruction is optional for the PowerPC (not "
			   "allowed without -force_cpusubtype_ALL option)",
			   format->name);
	    }
	    if(format->cpus == VMX &&
	       (archflag_cpusubtype != CPU_SUBTYPE_POWERPC_7400 &&
	        archflag_cpusubtype != CPU_SUBTYPE_POWERPC_7450 &&
	        archflag_cpusubtype != CPU_SUBTYPE_POWERPC_970)){
		as_bad("%s vector instruction is optional for the PowerPC (not "
		       "allowed without -force_cpusubtype_ALL option)",
		       format->name);
	    }
	    else
#endif /* !defined(ARCH64) */
	    if(md_cpusubtype == CPU_SUBTYPE_POWERPC_ALL){
		switch(format->cpus){
		case CPU601:
		    if(archflag_cpusubtype != -1 &&
		       archflag_cpusubtype != CPU_SUBTYPE_POWERPC_601)
			as_bad("%s 601 instruction not allowed with -arch %s",
			       format->name, specific_archflag);
		    else{
			file_spec = logical_input_file ?
				    logical_input_file : physical_input_file;
			line_spec = logical_input_line ?
				    logical_input_line : physical_input_line;
			md_cpusubtype = CPU_SUBTYPE_POWERPC_601;
		    }
		    break;
		}
	    } 
	    else{
		switch(format->cpus){
		case CPU601:
		    if(archflag_cpusubtype != -1 &&
		       archflag_cpusubtype != CPU_SUBTYPE_POWERPC_601)
			as_bad("%s 601 instruction not allowed with -arch %s",
			       format->name, specific_archflag);
		    else{
			if(md_cpusubtype != CPU_SUBTYPE_POWERPC_601)
			    as_bad("more than one implementation specific "
				   "instruction seen and -force_cpusubtype_ALL "
				   "not specified (first implementation "
				   "specific instruction in: %s at line %u)",
				   file_spec, line_spec);
		    }
		    break;
		}
	    }
	}

	/*
	 * We are putting a machine instruction in this section so mark it as
	 * containg some machine instructions.
	 */
	frchain_now->frch_section.flags |= S_ATTR_SOME_INSTRUCTIONS;

	/* if this instruction requires labels mark it for later */
	switch(insn.reloc){
	case NO_RELOC:
	    break;
	case PPC_RELOC_HI16:
	case PPC_RELOC_LO16:
	case PPC_RELOC_HA16:
	case PPC_RELOC_LO14:
	    fix_new(frag_now,
		    thisfrag - frag_now->fr_literal,
		    4,
		    insn.exp.X_add_symbol,
		    insn.exp.X_subtract_symbol,
		    insn.exp.X_add_number,
		    0, 0,
		    insn.reloc);
	    break;
	case PPC_RELOC_BR14:
	case PPC_RELOC_BR14_predicted:
	    fix_new(frag_now,
		    thisfrag - frag_now->fr_literal,
		    4,
		    insn.exp.X_add_symbol,
		    insn.exp.X_subtract_symbol,
		    insn.exp.X_add_number,
		    insn.pcrel,
		    insn.pcrel_reloc,
		    insn.reloc);
	    break;

	case PPC_RELOC_BR24:
	    fix_new(frag_now,
		    thisfrag - frag_now->fr_literal,
		    4,
		    insn.exp.X_add_symbol,
		    insn.exp.X_subtract_symbol,
		    insn.exp.X_add_number,
		    insn.pcrel,
		    insn.pcrel_reloc,
		    insn.reloc);
	    break;
	default:
	    as_bad("Unknown relocation type");
	    break;
	}
	if(insn.jbsr_exp.X_add_symbol != NULL){
	    fix_new(frag_now,
		    thisfrag - frag_now->fr_literal,
		    4,
		    insn.jbsr_exp.X_add_symbol,
		    insn.jbsr_exp.X_subtract_symbol,
		    insn.jbsr_exp.X_add_number,
		    0, /* pcrel */
		    1, /* pcrel_reloc */
		    PPC_RELOC_JBSR);
	}
}

static
int
calcop(
struct ppc_opcode *format,
char *param,
struct ppc_insn *insn,
char *op,
enum branch_prediction prediction)
{
    uint32_t parcnt, bo;

	/* initial the passed structure */
	memset(insn, '\0', sizeof(struct ppc_insn));
	insn->opcode = format->opcode;
	insn->reloc = NO_RELOC;

	/* parse all parameters */
	for(parcnt = 0; parcnt < 5 &&
			format->ops[parcnt].type != NONE; parcnt++){
	    error_param_count = parcnt;

	    switch(format->ops[parcnt].type){
	    case JBSR:
		param = parse_jbsr(param, insn, format, parcnt);
		break;
	    case PCREL:
	    case BADDR:
		param = parse_branch(param, insn, format, parcnt);
		break;
	    case D:
	    case DS:
		param = parse_displacement(param, insn, format, parcnt);
		break;
	    case SI:
	    case UI:
	    case HI:
		param = parse_immediate(param, insn, format, parcnt);
		break;
	    case GREG:
	    case G0REG:
		param = parse_reg("r", param, insn, format, parcnt);
		break;
	    case FREG:
		param = parse_reg("f", param, insn, format, parcnt);
		break;
	    case VREG:
		param = parse_reg("v", param, insn, format, parcnt);
		break;
	    case SGREG:
		param = parse_reg("sr", param, insn, format, parcnt);
		break;
	    case SPREG:
		param = parse_spreg(param, insn, format, parcnt);
		break;
	    case BCND:
		param = parse_bcnd(param, insn, format, parcnt);
		break;
	    case CRF:
	    case CRFONLY:
		param = parse_crf(param, insn, format, parcnt);
		break;
	    case SNUM:
		param = parse_num(param, insn, format, parcnt, 0, 0, 1, 0);
		break;
	    case FXM:
		param = parse_num(param, insn, format, parcnt, 0, 0, 0, 1);
		break;
	    case NUM:
		param = parse_num(param, insn, format, parcnt, 0, 0, 0, 0);
		break;
	    case NUM0:
		param = parse_num(param, insn, format, parcnt, 1, 0, 0, 0);
		break;
	    case MBE:
		param = parse_mbe(param, insn, format, parcnt);
		break;
	    case ZERO:
		param = parse_num(param, insn, format, parcnt, 0, 1, 0, 0);
		break;
	    case sh:
		param = parse_sh(param, insn, format, parcnt);
		break;
	    case mb:
		param = parse_mb(param, insn, format, parcnt);
		break;
	    default:
		as_fatal("Unknown parameter type");
	    }

	    /* see if parser failed or not */
	    if (param == NULL)
		return(0);
	}
	if((parcnt == 5 && *param != '\0') ||
	   (format->ops[0].type == NONE && *param != '\0')){
	    error_param_message = "too many parameters";
	    return(0);
	}

	if(IS_BRANCH_CONDITIONAL(insn->opcode)){
	    if(prediction != BRANCH_PREDICTION_NONE){
		if(prediction == BRANCH_PREDICTION_LIKELY_TAKEN ||
		   prediction == BRANCH_PREDICTION_LIKELY_NOT_TAKEN){
		    /*
		     * Set the Y_BIT assuming the displacement is non-negitive.
		     * If the displacement is negitive then the Y_BIT is flipped
		     * in md_number_to_imm() if the reloc is
		     * PPC_RELOC_BR14_predicted.
		     */
		    if(insn->reloc == PPC_RELOC_BR14)
			insn->reloc = PPC_RELOC_BR14_predicted;
		    if(prediction == BRANCH_PREDICTION_LIKELY_TAKEN)
			insn->opcode |= Y_BIT;
		    else{ /* prediction == BRANCH_PREDICTION_LIKELY_NOT_TAKEN */
			if((insn->opcode & Y_BIT) != 0)
			    as_warn("branch prediction ('-') ignored (specified"
				    " operand has prediction bit set)");
			else
			    insn->opcode &= ~(Y_BIT);
		    }
		}
		if(prediction == BRANCH_PREDICTION_VERY_LIKELY_TAKEN ||
		   prediction == BRANCH_PREDICTION_VERY_LIKELY_NOT_TAKEN){
		    bo = (insn->opcode >> 21) & 0x1f;
		    /*
		     * For 'branch if the condition is FALSE or TRUE' the AT
		     * bits are the lower 2 bits of the BO field (xxxAT).
		     */
		    if(bo == 0x04 || bo == 0x0c){
			if(prediction == BRANCH_PREDICTION_VERY_LIKELY_TAKEN)
			    insn->opcode |= 0x00600000; /* AT == 11 */
			else
			    insn->opcode |= 0x00400000; /* AT == 10 */
		    }
		    else if(bo == 0x10 || bo == 0x12){
			/*
			 * For 'decrement the CTR, then branch if the
			 * decremented CTR is non-zero or zero' the AT bits are
			 * the xAxxT bits of the BO field.
			 */
			if(prediction == BRANCH_PREDICTION_VERY_LIKELY_TAKEN)
			    insn->opcode |= 0x01200000;	/* AT == 11 */
			else
			    insn->opcode |= 0x01000000;	/* AT == 10 */
		    }
		    else{
			if(prediction == BRANCH_PREDICTION_VERY_LIKELY_TAKEN)
			    as_warn("branch prediction ('++') ignored "
				    "(specified operand has does not allow this"
				    " prediction)");
			else
			    as_warn("branch prediction ('--') ignored "
				    "(specified operand has does not allow this"
				    " prediction)");
		    }
		}
	    }
	}
	else{
	    if(prediction != '\0')
		as_warn("branch prediction ignored (instruction is not a "
			"conditional branch)");
	}
	return(1);
}

static
char *
parse_displacement(
char *param,
struct ppc_insn *insn,
struct ppc_opcode *format,
int parcnt)
{
    signed_target_addr_t val;
    char *end, *saveptr, *saveparam;
    segT seg;


	if(parcnt != 1 ||
	   (format->ops[2].type != G0REG && format->ops[2].type != GREG))
	     as_fatal("internal error, bad table entry for instruction %s "
		      "(displacement operand not second operand or general "
		      "register not third operand)", format->name);

	/*
	 * There must be "(rX)" (where X is a number between 0-31) or "(0)"
	 * at the end of the parameter string.  To know out where the
	 * displacement expression ends determine the begining the "(rX)"
	 * by looking for the last '(' in the string.  The parsing of this
	 * trailing string will be done in another routine.
	 */
	end = strrchr(param, '(');
	if(end == NULL)
	    return(NULL);
	*end = '\0';

	/*
	 * The expression may have one of the following: hi16(exp), ha16(exp),
	 * or lo16(exp) around the expression which determines the relocation
	 * type.
	 */
	if(strncmp(param,"hi16(",5) == 0){
	    insn->reloc = PPC_RELOC_HI16;
	    param += 5;
	}
	else if(strncmp(param,"ha16(",5) == 0){
	    insn->reloc = PPC_RELOC_HA16;
	    param += 5;
	}
	else if(strncmp(param,"lo16(",5) == 0){
	    if(format->ops[parcnt].type == DS)
		insn->reloc = PPC_RELOC_LO14;
	    else
		insn->reloc = PPC_RELOC_LO16;
	    param += 5;
	}

	saveptr = input_line_pointer;
	input_line_pointer = param;

	seg = expression(&insn->exp);
	try_to_make_absolute(&insn->exp);
	seg = insn->exp.X_seg;

	saveparam = input_line_pointer;
	input_line_pointer = saveptr;
	*end = '(';

	if(insn->reloc != NO_RELOC){
	    if(*saveparam != ')' || ++saveparam != end)
		return(NULL);
	}
	else{
	    if(saveparam != end)
		return(NULL);
	    val = insn->exp.X_add_number;
	    if(seg != SEG_ABSOLUTE){
		error_param_message = "Parameter error: expression must be "
				      "absolute";
		return(NULL);
	    }
	    if(val & 0x8000){
		if((val & 0xffff0000) != 0xffff0000){
		    error_param_message = "Parameter error: expression out of "
					  "range";
		    return(NULL);
		}
		val = val & 0xffff;
	    }
	    else{
		if((val & 0xffff0000) != 0){
		    error_param_message = "Parameter error: expression out of "
					  "range";
		    return(NULL);
		}
	    }
	    if(format->ops[parcnt].type == DS){
		if((val & 0x3) != 0){
		    error_param_message = "Parameter error: expression must be "
					  "a multiple of 4";
		    return(NULL);
		}
		val >>= 2;
	    }
	    insn->opcode |= val << format->ops[parcnt].offset;
	}
	return(saveparam);
}

static
char *
parse_immediate(
char *param,
struct ppc_insn *insn,
struct ppc_opcode *format,
int parcnt)
{
    uint32_t val;
    char *saveptr, *saveparam;
    segT seg;

	/*
	 * The expression may have one of the following: hi16(exp), ha16(exp),
	 * or lo16(exp) around the expression which determines the relocation
	 * type.
	 */
	if(strncmp(param,"hi16(",5) == 0){
	    insn->reloc = PPC_RELOC_HI16;
	    param += 5;
	}
	else if(strncmp(param,"ha16(",5) == 0){
	    insn->reloc = PPC_RELOC_HA16;
	    param += 5;
	}
	else if(strncmp(param,"lo16(",5) == 0){
	    if(format->ops[parcnt].type == DS)
		insn->reloc = PPC_RELOC_LO14;
	    else
		insn->reloc = PPC_RELOC_LO16;
	    param += 5;
	}

	saveptr = input_line_pointer;
	input_line_pointer = param;

	seg = expression(&insn->exp);
	try_to_make_absolute(&insn->exp);
	seg = insn->exp.X_seg;

	saveparam = input_line_pointer;
	input_line_pointer = saveptr;

	if(insn->reloc != NO_RELOC){
	    if(*saveparam != ')')
		return(NULL);
	    saveparam++;
	    if(*saveparam == '\0'){
		if(parcnt == 4 || format->ops[parcnt+1].type == NONE)
		    return(saveparam);
		else
		    return(NULL);
	    }
	    else if(*saveparam == ','){
		if(parcnt != 4 && format->ops[parcnt+1].type != NONE)
		    return(saveparam+1);
		else
		    return(NULL);
	    }
	    else
		return(NULL);
	}
	else{
	    val = insn->exp.X_add_number;
	    if(seg != SEG_ABSOLUTE){
		error_param_message = "Parameter error: expression must be "
				      "absolute";
		return(NULL);
	    }
	    if(format->ops[parcnt].type == SI){
		if(val & 0x8000){
		    if((val & 0xffff0000) != 0xffff0000){
			error_param_message = "Parameter error: expression out "
					      "of range";
			return(NULL);
		    }
		    val = val & 0xffff;
		}
		else{
		    if((val & 0xffff0000) != 0){
			error_param_message = "Parameter error: expression out "
					      "of range";
			return(NULL);
		    }
		}
	    }
	    else if(format->ops[parcnt].type == UI){
		if((val & 0xffff0000) != 0){
		    error_param_message = "Parameter error: expression out "
					  "of range";
		    return(NULL);
		}
	    }
	    else if(format->ops[parcnt].type == HI){
		if((val & 0xffff0000) != 0 &&
		   (val & 0xffff0000) != 0xffff0000){
		    error_param_message = "Parameter error: expression out "
					  "of range";
		    return(NULL);
		}
		val = val & 0xffff;
	    }
	    if(*saveparam == '\0'){
		if(parcnt == 4 || format->ops[parcnt+1].type == NONE){
		    insn->opcode |= val << format->ops[parcnt].offset;
		    return(saveparam);
		}
		else
		    return(NULL);
	    }
	    else if(*saveparam == ','){
		if(parcnt != 4 && format->ops[parcnt+1].type != NONE){
		    insn->opcode |= val << format->ops[parcnt].offset;
		    return(saveparam+1);
		}
		else
		    return(NULL);
	    }
	    else
		return(NULL);
	}
	return(saveparam);
}

static
char *
parse_jbsr(
char *param,
struct ppc_insn *insn,
struct ppc_opcode *format,
int parcnt)
{
    char *saveptr, *saveparam, *p;
    segT seg;
    short reference;

	reference = 0;
	saveptr = input_line_pointer;
	input_line_pointer = param;

	/*
	 * If we are assembling -dynamic then if the symbol name before the ','
	 * has not yet been seen it will be marked as a non-lazy reference.
	 */
	if(flagseen[(int)'k'] == TRUE){
	    p = strchr(param, ',');
	    if(p != NULL)
		*p = '\0';
	    if(symbol_find(param) == NULL)
		reference = REFERENCE_FLAG_UNDEFINED_LAZY;
	    else
		reference = REFERENCE_FLAG_UNDEFINED_NON_LAZY;
	    if(p != NULL)
		*p = ',';
	}

	seg = expression(&insn->jbsr_exp);
	try_to_make_absolute(&insn->jbsr_exp);
	seg = insn->jbsr_exp.X_seg;

	if(flagseen[(int)'k'] == TRUE)
	    insn->jbsr_exp.X_add_symbol->sy_desc |= reference;

	saveparam = input_line_pointer;
	input_line_pointer = saveptr;

	if(*saveparam == ',')
	    return(saveparam+1);
	else
	    return(NULL);
}

static
char *
parse_branch(
char *param,
struct ppc_insn *insn,
struct ppc_opcode *format,
int parcnt)
{
    char *saveptr, *saveparam;
    segT seg;

	saveptr = input_line_pointer;
	input_line_pointer = param;

	seg = expression(&insn->exp);
	try_to_make_absolute(&insn->exp);
	seg = insn->exp.X_seg;

	saveparam = input_line_pointer;
	input_line_pointer = saveptr;

	insn->pcrel = 0;
	insn->pcrel_reloc = 0;
	if(format->ops[parcnt].type == PCREL){
	    /*
	     * The NeXT linker has the ability to scatter blocks of
	     * sections between labels.  This requires that brances to
	     * labels that survive to the link phase must be able to
	     * be relocated.
	     */
	    if(insn->exp.X_add_symbol != NULL &&
	       (insn->exp.X_add_symbol->sy_name[0] != 'L' || flagseen ['L'])){
		if(insn->jbsr_exp.X_add_symbol != NULL)
		    as_fatal("Stub label used in a JBSR must be "
			     "non-relocatable");
		insn->pcrel_reloc = 1;
	    }
	    else{
		insn->pcrel_reloc = 0;
	    }
	    insn->pcrel = 1;
	}
	switch(format->ops[parcnt].width){
	case 14:
	    insn->reloc = PPC_RELOC_BR14;
	    break;
	case 24:
	    insn->reloc = PPC_RELOC_BR24;
	    break;
	default:
	    as_fatal("Unknown branch instruction width %d",
		    format->ops[parcnt].width);
	    break;
	}
	return(saveparam);
}

static
char *
parse_reg(
char *reg_name,
char *param,
struct ppc_insn *insn,
struct ppc_opcode *format,
uint32_t parcnt)
{
    uint32_t val, d;

	d = 0;
	if(*param == '(' && parcnt == 2 &&
	   (format->ops[1].type == D || format->ops[1].type == DS)){
	    d = 1;
	    param++;
	}

	if(format->ops[parcnt].type == G0REG && *param == '0'){
	    val = 0;
	    param++;
	}
	else{
	    val = 0;
	    while(*reg_name){
		if(*param++ != *reg_name++)
		    return(NULL);
	    }
	    if(!isdigit(*param))
		return(NULL);

	    while(isdigit(*param))
		if((val = val * 10 + *param++ - '0') >=
		   (uint32_t)(1 << format->ops[parcnt].width))
		return(NULL);

	    if(format->ops[parcnt].type == G0REG && val == 0){
		error_param_message = "Parameter error: r0 not allowed "
				      "for parameter %lu (code as 0 not r0)";
		return(NULL);
	    }
	}

	if(*param == '\0'){
	    if(parcnt == 4 || format->ops[parcnt+1].type == NONE){
		insn->opcode |= val << format->ops[parcnt].offset;
		return(param);
	    }
	    else
		return(NULL);
	}
	else if(*param == ','){
	    if(parcnt != 4 && format->ops[parcnt+1].type != NONE){
		insn->opcode |= val << format->ops[parcnt].offset;
		return(param+1);
	    }
	    else
		return(NULL);
	}
	else if(d == 1 && *param == ')' && param[1] == '\0'){
	    if(parcnt == 4 || format->ops[parcnt+1].type == NONE){
		insn->opcode |= val << format->ops[parcnt].offset;
		return(++param);
	    }
	    else
		return(NULL);
	}
	return(NULL);
}

static
char *
parse_spreg(
char *param,
struct ppc_insn *insn,
struct ppc_opcode *format,
uint32_t parcnt)
{
    signed_target_addr_t val;
    uint32_t i;
    char *saveptr, save_c;
    expressionS exp;
    segT seg;

	saveptr = input_line_pointer;
	input_line_pointer = param;
	while(*param != ',' && *param != '\0')
		param++;
	save_c = *param;
	*param = '\0';

	seg = SEG_ABSOLUTE;
	val = 0;
	for(i = 0; *special_registers[i].name != '\0'; i++){
	    if(strcmp(input_line_pointer, special_registers[i].name) == 0){
		val = special_registers[i].number;
		break;
	    }
	}
	if(*special_registers[i].name == '\0'){
	    seg = expression(&exp);
	    try_to_make_absolute(&exp);
	    seg = exp.X_seg;
	    val = exp.X_add_number;
	}
	*param = save_c;
	input_line_pointer = saveptr;

	if(seg != SEG_ABSOLUTE){
	    error_param_message = "Parameter error: expression must be "
				  "absolute";
	    return(NULL);
	}
	if(val > 1024 || val < 0){
	    error_param_message = "Parameter error: expression out "
				  "of range";
	    return(NULL);
	}

	val = ((val & 0x1f) << 5) | ((val >> 5) & 0x1f);

	if(*param == '\0'){
	    if(parcnt == 4 || format->ops[parcnt+1].type == NONE){
		insn->opcode |= val << format->ops[parcnt].offset;
		return(param);
	    }
	    else
		return(NULL);
	}
	else if(*param == ','){
	    if(parcnt != 4 && format->ops[parcnt+1].type != NONE){
		insn->opcode |= val << format->ops[parcnt].offset;
		return(param+1);
	    }
	    else
		return(NULL);
	}
	return(NULL);
}

static
char *
parse_bcnd(
char *param,
struct ppc_insn *insn,
struct ppc_opcode *format,
uint32_t parcnt)
{
    signed_target_addr_t val;
    uint32_t i, j;
    char *saveptr, save_c, *plus, save_plus;
    expressionS exp;
    segT seg;

	saveptr = input_line_pointer;
	input_line_pointer = param;
	while(*param != ',' && *param != '\0')
	    param++;
	save_c = *param;
	*param = '\0';

	/*
	 * look for "[CR_field+]condition_symbol".
	 */
	val = -1;
	for(plus = input_line_pointer; *plus != '+' && *plus != '\0'; plus++)
	    ;
	if(*plus == '+'){
	    save_plus = *plus;
	    *plus = '\0';
	    for(i = 0; *CR_fields[i].name != '\0'; i++)
		if(strcmp(input_line_pointer, CR_fields[i].name) == 0)
		    break;
	    *plus = save_plus;
	    if(*CR_fields[i].name != '\0'){
		for(j = 0; *condition_symbols[j].name != '\0'; j++)
		    if(strcmp(plus+1, condition_symbols[j].name) == 0)
			break;
		if(*condition_symbols[j].name != '\0'){
		    val = CR_fields[i].value + condition_symbols[j].value;
		}
	    }
	}
	else{
	    for(i = 0; *condition_symbols[i].name != '\0'; i++)
		if(strcmp(input_line_pointer, condition_symbols[i].name) == 0)
		    break;
	    if(*condition_symbols[i].name != '\0')
		val = condition_symbols[i].value;
	}
	if(val == -1){
	    seg = expression(&exp);
	    try_to_make_absolute(&exp);
	    seg = exp.X_seg;
	    val = exp.X_add_number;
	    if(seg != SEG_ABSOLUTE){
		error_param_message = "Parameter error: expression must be "
				      "absolute";
		*param = save_c;
		input_line_pointer = saveptr;
		return(NULL);
	    }
	    if(val >= (1 << format->ops[parcnt].width) || val < 0){
		error_param_message = "Parameter error: expression out "
				      "of range";
		*param = save_c;
		input_line_pointer = saveptr;
		return(NULL);
	    }
	}

	*param = save_c;
	input_line_pointer = saveptr;


	if(*param == '\0'){
	    if(parcnt == 4 || format->ops[parcnt+1].type == NONE){
		insn->opcode |= val << format->ops[parcnt].offset;
		return(param);
	    }
	    else
		return(NULL);
	}
	else if(*param == ','){
	    if(parcnt != 4 && format->ops[parcnt+1].type != NONE){
		insn->opcode |= val << format->ops[parcnt].offset;
		return(param+1);
	    }
	    else
		return(NULL);
	}
	return(NULL);

}

static
char *
parse_crf(
char *param,
struct ppc_insn *insn,
struct ppc_opcode *format,
uint32_t parcnt)
{
    signed_target_addr_t val;
    uint32_t i;
    char *saveptr, save_c;
    expressionS exp;
    segT seg;

	saveptr = input_line_pointer;
	input_line_pointer = param;
	while(*param != ',' && *param != '\0')
	    param++;
	save_c = *param;
	*param = '\0';
	val = -1;
	for(i = 0; *CR_fields[i].name != '\0'; i++){
	    if(strcmp(input_line_pointer, CR_fields[i].name) == 0){
		val = CR_fields[i].value;
		break;
	    }
	}
	if(val == -1){
	    if(format->ops[parcnt].type == CRFONLY){
		*param = save_c;
		input_line_pointer = saveptr;
		return(NULL);
	    }
	    seg = expression(&exp);
	    try_to_make_absolute(&exp);
	    seg = exp.X_seg;
	    val = exp.X_add_number;
	    if(seg != SEG_ABSOLUTE){
		error_param_message = "Parameter error: expression must be "
				      "absolute";
		*param = save_c;
		input_line_pointer = saveptr;
		return(NULL);
	    }
	    if(val >= (1 << format->ops[parcnt].width) || val < 0){
		error_param_message = "Parameter error: expression out "
				      "of range";
		*param = save_c;
		input_line_pointer = saveptr;
		return(NULL);
	    }
	}
	*param = save_c;
	input_line_pointer = saveptr;

	if(*param == '\0'){
	    if(parcnt == 4 || format->ops[parcnt+1].type == NONE){
		insn->opcode |= val << format->ops[parcnt].offset;
		return(param);
	    }
	    else
		return(NULL);
	}
	else if(*param == ','){
	    if(parcnt != 4 && format->ops[parcnt+1].type != NONE){
		insn->opcode |= val << format->ops[parcnt].offset;
		return(param+1);
	    }
	    else
		return(NULL);
	}
	return(NULL);
}

static
char *
parse_num(
char *param,
struct ppc_insn *insn,
struct ppc_opcode *format,
uint32_t parcnt,
int32_t max_width_zero,
int32_t zero_only,
int32_t signed_num,
int32_t bit_mask_with_1_bit_set)
{
    signed_target_addr_t val;
    int i, max, min, mask, temp;
    char *saveptr, save_c;
    expressionS exp;
    segT seg;

	saveptr = input_line_pointer;
	input_line_pointer = param;
	while(*param != ',' && *param != '\0')
		param++;
	save_c = *param;
	*param = '\0';
	seg = expression(&exp);
	try_to_make_absolute(&exp);
	seg = exp.X_seg;
	*param = save_c;
	input_line_pointer = saveptr;

	val = exp.X_add_number;
	if(seg != SEG_ABSOLUTE){
	    error_param_message = "Parameter error: expression must be "
				  "absolute";
	    return(NULL);
	}
	if(max_width_zero){
	    if(val == (1 << format->ops[parcnt].width))
		val = 0;
	}
	if(signed_num){
	    max = (1 << (format->ops[parcnt].width - 1)) - 1;
	    min = (0xffffffff << (format->ops[parcnt].width - 1));
	    temp = val;
	    if(temp > max || temp < min){
		error_param_message = "Parameter error: expression out "
				      "of range";
		return(NULL);
	    }
	}
	else{
	    max = (1 << (format->ops[parcnt].width)) - 1;
	    if(val > max){
		error_param_message = "Parameter error: expression out "
				      "of range";
		return(NULL);
	    }
	}
	if(bit_mask_with_1_bit_set){
	    mask = 1;
	    for(i = 0; i < format->ops[parcnt].width; i++){
		if(mask & val)
		    break;
		mask = mask << 1;
	    }
	    /*
	     * If this is the mtcrf opcode (0x7c000120) and val is not zero and
	     * has exactly one bit set then use the new form of the mtcrf
	     * opcode.  This has bit 0x00100000 set and the FXM field is a bit
	     * mask. Else use the old form without bit 0x00100000 set.
	     */ 
	    if(insn->opcode == 0x7c000120){
		if(val != 0 && val == mask)
		    insn->opcode |= 0x00100000;
	    }
	    else{
		/*
		 * For instructions other than mtcrf if exactly one bit in val
		 * is not set it is an error.
		 */
		if(val == 0 || val != mask){
		    error_param_message = "Parameter error: expression must "
				  "have exactly one bit set";
		    return(NULL);
		}
	    }
	}
	if(zero_only == 1 && val != 0){
	    error_param_message = "Parameter error: expression must have a "
				  "value of zero";
	    return(NULL);
	}
	if(*param == '\0'){
	    if(parcnt == 4 || format->ops[parcnt+1].type == NONE){
		insn->opcode |= (val & ((1 << format->ops[parcnt].width)-1)) <<
				format->ops[parcnt].offset;
		return(param);
	    }
	    else
		return(NULL);
	}
	else if(*param == ','){
	    if(parcnt != 4 && format->ops[parcnt+1].type != NONE){
		insn->opcode |= (val & ((1 << format->ops[parcnt].width)-1)) <<
				format->ops[parcnt].offset;
		return(param+1);
	    }
	    else
		return(NULL);
	}
	return(NULL);
}

static
char *
parse_mbe(
char *param,
struct ppc_insn *insn,
struct ppc_opcode *format,
uint32_t parcnt)
{
    signed_target_addr_t val;
    char *saveptr, save_c;
    expressionS exp;
    segT seg;

	if (parcnt == 4 && *param == '\0')
	  return param;

	saveptr = input_line_pointer;
	input_line_pointer = param;
	while(*param != ',' && *param != '\0')
		param++;
	save_c = *param;
	*param = '\0';
	seg = expression(&exp);
	try_to_make_absolute(&exp);
	seg = exp.X_seg;
	*param = save_c;
	input_line_pointer = saveptr;

	val = exp.X_add_number;
	if(seg != SEG_ABSOLUTE){
	    error_param_message = "Parameter error: expression must be "
				  "absolute";
	    return(NULL);
	}
	/* Note that we need to allow all 32-bit values for val. */

	/* Look for the special case. */

	if (parcnt == 3 && *param == '\0')
	  {
	    uint32_t uval, mask;
	    int mb, me, mx, count, last;

	    uval = val;

	    mb = 0;
	    me = 32;
	    if ((uval & 1) != 0)
	      last = 1;
	    else
	      last = 0;
	    count = 0;

	    /* mb: location of last 0->1 transition */
	    /* me: location of last 1->0 transition */
	    /* count: # transitions */

	    for (mx = 0, mask = 1 << 31; mx < 32; ++mx, mask >>= 1)
	      {
		if ((uval & mask) && !last)
		  {
		    ++count;
		    mb = mx;
		    last = 1;
		  }
		else if (!(uval & mask) && last)
		  {
		    ++count;
		    me = mx;
		    last = 0;
		  }
	      }
	    if (me == 0)
	      me = 32;
	  
	    if (count != 2 && (count != 0 || ! last))
	      {
		return NULL;
	      }

	    insn->opcode |= (mb & ((1 << format->ops[parcnt].width)-1)) <<
	      format->ops[parcnt].offset;
	    insn->opcode |= ((me - 1) & ((1 << format->ops[parcnt+1].width)-1)) <<
	      format->ops[parcnt+1].offset;

	    return param;
	  }

	    if((parcnt == 3 || parcnt == 4)){
		insn->opcode |= (val & ((1 << format->ops[parcnt].width)-1)) <<
				format->ops[parcnt].offset;
		return((parcnt == 3 ? param+1 : param));
	    }

	return(NULL);

}

static
char *
parse_sh(
char *param,
struct ppc_insn *insn,
struct ppc_opcode *format,
uint32_t parcnt)
{
    signed_target_addr_t val;
    char *saveptr, save_c;
    expressionS exp;
    segT seg;

	saveptr = input_line_pointer;
	input_line_pointer = param;
	while(*param != ',' && *param != '\0')
		param++;
	save_c = *param;
	*param = '\0';
	seg = expression(&exp);
	try_to_make_absolute(&exp);
	seg = exp.X_seg;
	*param = save_c;
	input_line_pointer = saveptr;

	val = exp.X_add_number;
	if(seg != SEG_ABSOLUTE){
	    error_param_message = "Parameter error: expression must be "
				  "absolute";
	    return(NULL);
	}
	if(val == 64)
	    val = 0;
	if(val >= 64 || val < 0){
	    error_param_message = "Parameter error: expression out "
				  "of range";
	    return(NULL);
	}

	if(*param == '\0'){
	    if(parcnt == 4 || format->ops[parcnt+1].type == NONE){
		insn->opcode |= (val & 0x1f) << 11;
		insn->opcode |= ((val >> 5) & 0x1) << 1;
		return(param);
	    }
	    else
		return(NULL);
	}
	else if(*param == ','){
	    if(parcnt != 4 && format->ops[parcnt+1].type != NONE){
		insn->opcode |= (val & 0x1f) << 11;
		insn->opcode |= ((val >> 5) & 0x1) << 1;
		return(param+1);
	    }
	    else
		return(NULL);
	}
	return(NULL);

}

static
char *
parse_mb(
char *param,
struct ppc_insn *insn,
struct ppc_opcode *format,
uint32_t parcnt)
{
    signed_target_addr_t val;
    char *saveptr, save_c;
    expressionS exp;
    segT seg;

	saveptr = input_line_pointer;
	input_line_pointer = param;
	while(*param != ',' && *param != '\0')
		param++;
	save_c = *param;
	*param = '\0';
	seg = expression(&exp);
	try_to_make_absolute(&exp);
	seg = exp.X_seg;
	*param = save_c;
	input_line_pointer = saveptr;

	val = exp.X_add_number;
	if(seg != SEG_ABSOLUTE){
	    error_param_message = "Parameter error: expression must be "
				  "absolute";
	    return(NULL);
	}
	if(val > 64 || val < 0){
	    error_param_message = "Parameter error: expression out "
				  "of range";
	    return(NULL);
	}

	if(*param == '\0'){
	    if(parcnt == 4 || format->ops[parcnt+1].type == NONE){
		insn->opcode |= (val & 0x1f) << 6;
		insn->opcode |= ((val >> 5) & 0x1) << 5;
		return(param);
	    }
	    else
		return(NULL);
	}
	else if(*param == ','){
	    if(parcnt != 4 && format->ops[parcnt+1].type != NONE){
		insn->opcode |= (val & 0x1f) << 6;
		insn->opcode |= ((val >> 5) & 0x1) << 5;
		return(param+1);
	    }
	    else
		return(NULL);
	}
	return(NULL);

}

/*
 * md_number_to_chars() is the target machine dependent routine that puts out
 * a binary value of size 8, 4, 2, or 1 bytes into the specified buffer.  This
 * is done in the target machine's byte sex.  In this case the byte order is
 * big endian.
 */
void
md_number_to_chars(
char *buf,
signed_expr_t val,
int nbytes)
{
	switch(nbytes){
	case 8:
	    *buf++ = val >> 56;
	    *buf++ = val >> 48;
	    *buf++ = val >> 40;
	    *buf++ = val >> 32;
	case 4:
	    *buf++ = val >> 24;
	    *buf++ = val >> 16;
	case 2:
	    *buf++ = val >> 8;
	case 1:
	    *buf = val;
	    break;

	default:
	    abort();
	}
}

/*
 * md_number_to_imm() is the target machine dependent routine that puts out
 * a binary value of size 4, 2, or 1 bytes into the specified buffer with
 * reguard to a possible relocation entry (the fixP->fx_r_type field in the fixS
 * structure pointed to by fixP) for the section with the ordinal nsect.  This
 * is done in the target machine's byte sex using it's relocation types.
 * In this case the byte order is big endian.
 */
void
md_number_to_imm(
unsigned char *buf,
signed_expr_t val,
int nbytes,
fixS *fixP,
int nsect)
{
    uint32_t opcode;

	if(fixP->fx_r_type == NO_RELOC ||
	   fixP->fx_r_type == PPC_RELOC_VANILLA){
	    switch(nbytes){
            case 8:
                *buf++ = val >> 56;
                *buf++ = val >> 48;
                *buf++ = val >> 40;
                *buf++ = val >> 32;
	    case 4:
		*buf++ = val >> 24;
		*buf++ = val >> 16;
	    case 2:
		*buf++ = val >> 8;
	    case 1:
		*buf = val;
		break;

	    default:
		abort();
	    }
	    return;
	}
	switch(fixP->fx_r_type){
	case PPC_RELOC_HI16:
	    buf[2] = val >> 24;
	    buf[3] = val >> 16;
	    break;

	case PPC_RELOC_LO16:
	    buf[2] = val >> 8;
	    buf[3] = val;
	    break;

	case PPC_RELOC_HA16:
	    val += 0x00008000;
	    buf[2] = val >> 24;
	    buf[3] = val >> 16;
	    break;

	case PPC_RELOC_LO14:
	    buf[2] = val >> 8;
	    buf[3] |= val & 0xfc;
	    break;

	case PPC_RELOC_BR14:
	case PPC_RELOC_BR14_predicted:
	    if(fixP->fx_pcrel)
		val += 4;
	    if((val & 0xffff8000) && ((val & 0xffff8000) != 0xffff8000)){
		layout_file = fixP->file;
		layout_line = fixP->line;
		as_bad("Fixup of %lld too large for field width of 16 "
			"bits", val);
	    }
	    if((val & 0x3) != 0){
		layout_file = fixP->file;
		layout_line = fixP->line;
		as_bad("Fixup of %lld is not to a 4 byte address", val);
	    }
	    /*
	     * Note PPC_RELOC_BR14 are only used with bc, "branch conditional"
	     * instructions.  The Y_BIT was previously set assuming the
	     * displacement is non-negitive. If the displacement is negitive
	     * then the Y_BIT is flipped if the prediction was specified (the
	     * reloc type is PPC_RELOC_BR14_predicted).  If the prediction was
	     * not specified (the reloc type is PPC_RELOC_BR14) then the bit
	     * must remain cleared as per the PowerPC book.
	     */
	    if((val & 0x00008000) != 0){
		opcode = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
		/*
		 * The 5-bit BO encoding of branch always is 1z1zz. Where z is
		 * ignored but should be set to zero.  If the user as specified
		 * the BO "branch always" value with some of the z bits on then
		 * treat it like a branch always and don't change the Y bit
		 * based on the sign of the displacement.
		 */
		if(((opcode) & 0x02800000) != 0x02800000){
		    if(fixP->fx_r_type == PPC_RELOC_BR14_predicted)
			opcode ^= Y_BIT;
		    buf[0] = opcode >> 24;
		    buf[1] = opcode >> 16;
		    buf[2] = opcode >> 8;
		    buf[3] = opcode;
		}
	    }
	    buf[2] = val >> 8;
	    buf[3] |= val & 0xfc;
	    break;

	case PPC_RELOC_BR24:
	    if(fixP->fx_pcrel)
		val += 4;
	    if((val & 0xfc000000) && ((val & 0xfc000000) != 0xfc000000)){
		layout_file = fixP->file;
		layout_line = fixP->line;
		as_bad("Fixup of %lld too large for field width of 26 "
			"bits", val);
	    }
	    if((val & 0x3) != 0){
		layout_file = fixP->file;
		layout_line = fixP->line;
		as_bad("Fixup of %lld is not to a 4 byte address", val);
	    }
	    buf[0] |= (val >> 24) & 0x03;
	    buf[1] = val >> 16;
	    buf[2] = val >> 8;
	    buf[3] |= val & 0xfc;
	    break;

	case PPC_RELOC_JBSR:
	    /* no bytes are written for JBSR, only a relocation entry */
	    break;

	default:
	    layout_file = fixP->file;
	    layout_line = fixP->line;
	    as_bad("Bad relocation type");
	    break;
	}
}

/*
 * md_atof() turns a string pointed to by input_line_pointer into a floating
 * point constant of type type, and store the appropriate bytes in *litP.
 * The number of LITTLENUMS emitted is stored indirectly through *sizeP.
 * An error message is returned, or a string containg only a '\0' for OK.
 * For this machine only IEEE single and IEEE double floating-point formats
 * are allowed.
 */
char *
md_atof(
int type,
char *litP,
int *sizeP)
{
    int	prec;
    LITTLENUM_TYPE words[6];
    LITTLENUM_TYPE *wordP;
    char *t;

	switch(type){
	case 'f':
	case 'F':
	case 's':
	case 'S':
	    prec = 2;
	    break;

	case 'd':
	case 'D':
	case 'r':
	case 'R':
	    prec = 4;
	    break;

	default:
	    *sizeP = 0;
	    return("Bad call to MD_ATOF()");
	}
	t = atof_ieee(input_line_pointer, type, words);
	if(t != NULL)
	    input_line_pointer = t;

	*sizeP = prec * sizeof(LITTLENUM_TYPE);
	for(wordP = words; prec--; ){
	    md_number_to_chars(litP, (int32_t)(*wordP++), sizeof(LITTLENUM_TYPE));
	    litP += sizeof(LITTLENUM_TYPE);
	}
	return ""; /* OK */
}

int
md_estimate_size_before_relax(
fragS *fragP,
int segment_type)
{
	as_bad("Relaxation should never occur");
	return(sizeof(int32_t));
}

const relax_typeS md_relax_table[] = { {0} };

void
md_convert_frag(
fragS *fragP)
{
	as_bad("Relaxation should never occur");
}
