#include "hppa-opcode.h"
#include <stdio.h>

extern uint32_t random();

#define RANGE(number)	(random() % (number))
#define COIN			(RANGE(2))
#define IMM_NUM(range)	{ 	int tmp000111000usv = RANGE(range); \
							if (tmp000111000usv == 0) \
								printf(" %d", tmp000111000usv); \
							else \
								printf(" %c%d", (COIN) ? '-' : ' ',  \
									tmp000111000usv); \
						}


/* The 'cond' decode table */

typedef  struct  condition_decode {
	char condition[4];
}  condT;

char *get_cond_str(const condT table[8][2], int c, int f );

/* condition decode tables
 * Refer to PA-RISC 1.1 Architecture and Instruction Set 
 * Reference Manual, Second edition, Sep. 1992, pp. 5-2 - 5-8.
 */

/* Note : In all the following tables
 *        ""   ... never
 *        "???"   ... Invalid combination 
 */

/* compare/subtraclt instruction conditions */
const condT c_comp_sub[8][2]  = { /* Table 5-3 */
	{ "", "TR" },
	{ "=", "<>" },
	{ "<", ">=" },
	{ "<=", ">" },
	{ "<<", ">>=" },
	{ "<<=", ">>" },
	{ "SV", "NSV" },
	{ "OD", "EV" }
};

/* add instruction conditions */
const condT c_add[8][2]  = { /* Table 5-4 */
	{ "", "TR" },
	{ "=", "<>" },
	{ "<", ">=" },
	{ "<=", ">" },
	{ "NUV", "UV"  },
	{ "ZNV", "VNZ" },
	{ "SV", "NSV" },
	{ "OD", "EV" }
};

/* logical instruction conditions */
const condT c_logical[8][2]  = { /* Table 5-5 */
	{ "", "TR" },
	{ "=", "<>" },
	{ "<", ">=" },
	{ "<=", ">" },
	{ "???", "???" },
	{ "???", "???" },
	{ "???", "???" },
	{ "OD", "EV" }
};

/* unit instruction conditions */
const condT c_unit[8][2]  = { /* Table 5-6 */
	{ "", "TR" },
	{ "???", "???" },
	{ "SBZ", "NBZ" },
	{ "SHZ", "NHZ" },
	{ "SDC", "NDC" },
	{ "???", "???" },
	{ "SBC", "NBC" },
	{ "SHC", "NHC" }
};

/* shift/extract/deposit instruction conditions */
const condT c_shift_extract_deposit[8][2]  = { /* Table 5-7 */
	{ "", "???" },
	{ "=", "???" },
	{ "<", "???" },
	{ "OD", "???" },
	{ "TR", "???" },
	{ "<>", "???" },
	{ ">=", "???" },
	{ "EV", "???" }
};



char *controlregs[] = { "fir", "psr", "epsr", "dirbase", "db", "fsr" };
#define NCREGS	(sizeof controlregs / sizeof controlregs[0])

char *textlabels[] = { "foo", "bar", "baz", "xork" };
#define NTLABELS	(sizeof textlabels / sizeof textlabels[0])

char *datalabels[] = { "data1", "data2", "data3", "data4" };
#define NDLABELS	(sizeof datalabels / sizeof datalabels[0])

char *fp_cmp_cond[] =	{
	"false?","false", "true?", "true", "!<=>", "!?>=", "!?<=",
	"!<>", "!>=", "!?>", "?<=", "!<=", "!?<", "?>=", "!?=",
	"!=t", "<=>", "=t", "?=", "?<", "<=", "!>", "?>", ">=",
	"!<", "<>", "!=", "!?", "?", "=", "<", ">"
};
#define NFPCOND	(sizeof fp_cmp_cond / sizeof fp_cmp_cond[0])

char *fp_format_str[] = { "sgl", "dbl", "quad" };
#define NFPFMT (sizeof fp_format_str / sizeof fp_format_str[0])

/*
 * Traverse the opcode table, dumping out sample instructions.
 */
void
main()
{
	int i;
	const char *arg;
	int do_not_nullify = 0;
	
	printf( "\t.text\n%s:", textlabels[0] );
	/* a label at the begining of the file */
	printf("label1:\n");
	
	for ( i = 0; i < NUMOPCODES; ++i )
	{
		if ( i == (NUMOPCODES/3) )
			printf( "%s:", textlabels[1] );
		if ( i == (NUMOPCODES/2) )
			printf( "%s:", textlabels[2] );

		printf( "\t%s", pa_opcodes[i].name );
		
		for ( arg = pa_opcodes[i].args; *arg != '\0'; ++arg )
		{
		    switch( *arg ) {

			case '\0':  /* end of args */
  				break;

			case '(':   /* these must match exactly */
				putchar(' '); /* and FALLTHRU  */
			case ')':
			case ',':
			case ' ':
				putchar(*arg);
  				break;

			case 'b':   /* 5 bit register field at 10 */
			case 'x':   /* 5 bit register field at 15 */
 			case 't':   /* 5 bit register field at 31 */
			case 'v':   /* a 't' type extended to handle L/R register halves. */
			case 'E':   /* a 'b' type extended to handle L/R register halves. */
			case 'X':   /* an 'x' type extended to handle L/R register halves. */
			case '4':   /* 5 bit register field at 10 (used in 'fmpyadd' and 'fmpysub') */
			case '6':   /* 5 bit register field at 15 (used in 'fmpyadd' and 'fmpysub') */
			case '7':   /* 5 bit register field at 31 (used in 'fmpyadd' and 'fmpysub') */
			case '8':   /* 5 bit register field at 20 (used in 'fmpyadd' and 'fmpysub') */
			case '9':   /* 5 bit register field at 25 (used in 'fmpyadd' and 'fmpysub') */
				printf(" %%r%d", RANGE(32));
 				break;

			case 'r':   /* 5  bit immediate at 31 */
			case 'R':   /* 5  bit immediate at 15 */
				printf(" %d", RANGE(32));
 				break;
			case 'T':   /* 5 bit field length at 31 (encoded as 32-T) */
				printf(" %d", RANGE(31) + 1);
 				break;

			case '5':   /* 5 bit immediate at 15 */
			case 'V':   /* 5  bit immediate at 31 */
			case 'p':   /* 5 bit shift count at 26 (to support SHD instr.) */
						/* value is encoded in instr. as 31-p where p is   */
						/* the value scanned here */
			case 'P':   /* 5-bit bit position at 26 */
			case 'Q':   /* 5  bit immediate at 10 (unsigned bit position */
						/* value for the bb instruction) */
				IMM_NUM(15);
				continue;

			case 's':   /* 2 bit space identifier at 17 */
				printf(" %d", RANGE(4));
				break;

			case 'S':   /* 3 bit space identifier at 18 */
				printf(" %%sr%d", RANGE(8));
				break;

			case 'c':   /* indexed load completer. */
			{
				int m, u, i;
				
				m = COIN;
				u = COIN;
				i = 0;
				if (COIN)
					while (i < 2) {
						if (m==1 && u==1) {
							printf(",sm");
							i++;
						}
						else if (m==1) 
							printf(",m");
							else if (u==1)
								printf(",s");
								else /* m==0 && u==0 */ {
									printf(",sm");
									i++;
								} /* probability distribution */
						i++;
  					}
  				continue;
			}
			case 'C':   /* short load and store completer */
				if (COIN) 
					if (COIN)
						printf(",mb");
					else
						printf(",ma");
				continue;
			case 'Y':   /* Store Bytes Short completer */
			{
				int i = 0, m, a;
				while ( i < 2 ) {
					m = COIN;
					a = COIN;
				
					if (m==1) /* && (a==0 || a==1) */
						printf(",m");
					else if (a==0)  /* && m==0 */
						printf(",b");
						else if (a==1) /* && m==0 */
							printf(",e");
					i++;
				}
				continue;
			}
			case '<':   /* non-negated compare/subtract conditions. */
			{
				int cmpltr;
				
				do {
					cmpltr = RANGE(4);
				} while (cmpltr == 0);
				
				printf(",%s", get_cond_str(c_comp_sub, cmpltr, 0));
			}
				continue;
			case '?':   /* negated or non-negated cmp/sub conditions. */
					/* used only by ``comb'' and ``comib'' pseudo-ops */
			case '-':   /* compare/subtract conditions */
			{
				int flag, cmpltr;
				char *tmp;
				
				do {
					flag = COIN;
					cmpltr = RANGE(8);
				} while ((flag & cmpltr) == 0 || (cmpltr == 0));
				
				tmp = get_cond_str(c_comp_sub, cmpltr, flag);				

				if (*tmp != '\0')
					printf(",%s", tmp);
			}
				continue;
			case '+':   /* non-negated add conditions */
			case '!':   /* negated or non-negated add conditions. */
			{
				int flag, cmpltr;
				char *tmp;
				
				do {
					flag = COIN;
					cmpltr = RANGE(8);
				} while ((flag & cmpltr) == 0 || (cmpltr == 0));
				
				tmp = get_cond_str(c_add, cmpltr, flag);
				
				if (COIN && (*tmp != '\0')) /* condition */ {
						printf(",%s", tmp);
						if (COIN) { /* nullify */
							printf(",n ");
							do_not_nullify = 1;
						}
				}
			}
				continue;		
			case '&':   /* logical instruction conditions */
			{
				int flag, cmpltr;
				char *tmp;
				
				flag = COIN;
				do {
					cmpltr = RANGE(8);
				} while (cmpltr == 4 || cmpltr == 5 || cmpltr == 6);
				
				tmp = get_cond_str(c_logical, cmpltr, flag);				

				if (COIN && (*tmp != '\0')) /* condition */
					printf(",%s", tmp);
			}
				continue;
			case 'U':   /* unit instruction conditions */
			{
				int flag, cmpltr;
				char *tmp;

				flag = COIN;
				do {
					cmpltr = RANGE(8);
				} while (cmpltr == 1 || cmpltr == 5);
				
				tmp = get_cond_str(c_unit, cmpltr, flag);				
				if (COIN && (*tmp != '\0')) /* condition */
					printf(",%s", tmp);
			}
				continue;
			case '>':   /* shift/extract/deposit conditions. */
			{
				int  cmpltr;
				char *tmp;
				
				cmpltr = RANGE(8);
				
				tmp = get_cond_str(c_shift_extract_deposit, cmpltr, 0);				

				if (COIN && (*tmp != '\0')) /* condition */
					printf(",%s", tmp);
			}
				continue;
			case '~':   /* bvb,bb conditions */
				if (COIN)
					printf(",<");
				else
					printf(",>=");
				continue;
				
			case 'i':   /* 11 bit immediate at 31 */
				IMM_NUM(1024);
				continue;
				
			case 'j':   /* 14 bit immediate at 31 --- LO14 */
			case 'a':	/* for be, ble --- BR17*/
			{
				int field_selector = RANGE(3);
				switch (field_selector) {
				case 2:	/* field selector R`*/
					printf(" R`");
					break;
				case 1:	/* field selector L`*/
					printf(" L`");
					break;
				default:
					break;
				}
				IMM_NUM(8192);
				continue;
			}  
			case 'k':   /* 21 bit immediate at 31 --- HI21 */
			{
				int field_selector = RANGE(3);
				switch (field_selector) {
				case 2:	/* field selector R`*/
					printf(" R`");
					break;
				case 1:	/* field selector L`*/
					printf(" L`");
					break;
				default:
					break;
				}
				IMM_NUM(1048576);
				continue;
			}			
			case 'n':   /* nullification for branch instructions */
				if (!do_not_nullify)
					if (COIN)
						printf(",n");
				else
					do_not_nullify = 0;
				continue;		
			case 'w':   /* 12 bit branch displacement */
				IMM_NUM(2048);
				continue;
			case 'W':   /* 17 bit branch displacement --- BL17 */
			case '@':   /* 17 bit branch displacement --- JBSR */
			case 'z':   /* 17 bit branch displacement (non-pc-relative) */
				IMM_NUM(65536);
				continue;
			case 'B':   /* either "s,b" or "b" where b & s are defined above */
				if (COIN)
					printf(" %d,", RANGE(4));
				printf(" %%r%d", RANGE(32));
				break;
			case 'A':   /* 13 bit immediate at 18 (to support BREAK instr.) */
				printf(" %d", RANGE(4096));
				continue;
			case 'Z':   /* System Control Completer(for LDA, LHA, etc.) */
				if (COIN)
					printf(",M");
  				continue;
			case 'D':   /* 26 bit immediate at 31 (to support DIAG instr.) */
  						/* the action (and interpretation of this operand is
		 					implementation dependent) */
				IMM_NUM(33554432);
  				continue;
			case 'f':   /* 3 bit Special Function Unit (SFU) identifier at 25 */
			case 'u':   /* 3 bit coprocessor unit identifier at 25 */
				printf("%d", RANGE(8));
  				continue;
			case 'O':   /* 20 bit SFU op. split between 15 bits at 20 and 5 bits at 31 */
				printf("%d", RANGE(1048576));
  				continue;
			case 'o':   /* 15 bit Special Function Unit operation at 20 */
			case '1':   /* 15 bit SFU op. split between 10 bits at 20
			   				and 5 bits at 31 */
				printf("%d", RANGE(32768));
  				continue;
			case '2':   /* 22 bit SFU op. split between 17 bits at 20
			   				and 5 bits at 31 */
				printf("%d", RANGE(4194304));
  				continue;
			case '0':   /* 10 bit SFU op. split between 5 bits at 20
			   				and 5 bits at 31 */
				printf("%d", RANGE(1024));
  				continue;
			case 'G':   /* Destination FP Operand Format Completer (2 bits at 18) */
			case 'F':   /* Source FP Operand Format Completer (2 bits at 20) */
				printf(",%s", fp_format_str[RANGE(NFPFMT)]);
  				continue;
			case 'M':   /* FP Compare Conditions (encoded as 5 bits at 31) */
				printf(",%s", fp_cmp_cond[RANGE(NFPCOND)]);
  				continue;

#if 0
			case 'H':  /* Floating Point Operand Format at 26 for       */
						/* 'fmpyadd' and 'fmpysub' (very similar to 'F') */
						/* bits are switched from other FP Operand       */
						/* formats. 1=SGL, 1=<none>, 0=DBL               */
				f = pa_parse_fp_format(&s);
				switch (f) {
				case SGL:
					opcode |= 0x20;
				case DBL:
					the_insn.fpof1 = f;
					continue;

				case QUAD:
				case ILLEGAL_FMT:
				default:
					as_bad("Illegal Floating Point Operand Format for"
						"this instruction: '%s'",*s);
				}
				break;
			default:
				abort();
#endif    /* 0 */

			}
		}
		putchar( '\n' );
	}
	/* a label at the end of the file */
	printf("label2:\n");
	

	printf( "%s:\n", textlabels[3] );
	printf( "\t.data\n" );
	printf( "data1:	.space 1024\n" );
	printf( "data2:	.space 1024\n" );
	printf( "data3:	.space 1024\n" );
	printf( "data4:	.space 1024\n" );

}

/* The function to search the condition decode table 
 * and return the 'cond'
 * The way tables are initialised ... NULL termination 
 * of 'cond' is assured.
 */ 
char *get_cond_str(const condT table[8][2], int c, int f )
{

/* do range check and return NULL if out of bound */
/*	char *str;
	if ( c < 0  || c > 7 )
		*str = (char *) NULL;
	else	if ( f < 0  || f > 1 )
		*str = (char *) NULL;
	else 
		str = table[c][f].condition;

	return str;
*/

/* here goes the one liner */

	return (c<0 || c>7 || f<0 || f>1)
		? (char *)NULL : table[c][f].condition	;
}    /* end get_cond_str() */

