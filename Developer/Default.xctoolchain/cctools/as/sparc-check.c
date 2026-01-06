#include <stdio.h>
#include "sparc-opcode.h"

static void print_rs_reg(char);
static void print_cp_reg(void);
static void print_f_reg(int);
static void print_imm_13(void);
static void print_imm_22(void);
static void print_asi(void);


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

void
main(
     int argc,
     char *argv[],
     char *envp[])
{
  int32_t i;
  const char *arg;
  
  /* output each instruction */
  
  for(i = 0; i < NUMOPCODES - 1; i++){
    printf("\t%s", sparc_opcodes[i].name);
    arg = sparc_opcodes[i].args;

    if (*arg != ',' && *(arg+1) != 'a')  /* handle annul case */
      printf("\t");
    
    /* and every possible combination */
    for (arg = sparc_opcodes[i].args; *arg != '\0'; arg++) {
      switch (*arg) {
      case '\0':
	break;		/* done */
      case '1':
      case '2':
      case 'r':
      case 'd':
	print_rs_reg(*arg);	/* output in a random register */
	break;
      case 'i':
	print_imm_13();	/* output a random immediate value */
	break;
      case 'n':
	print_imm_22();	/* output a random immediate value */
	break;
      case 'L':
      case 'l':
	printf("undef");
	break;
      case 'D':
	print_cp_reg();
	break;
      case 'F':
	printf("%%fsr");
	break;
      case 'p':
	printf("%%psr");
	break;
      case 'C':
	printf("%%csr");
	break;
      case 'A':
	print_asi();
	break;
      case 'q':
	printf("%%fq");
	break;
      case 'Q':
	printf("%%cq");
	break;
      case 'y':
	printf("%%y");
	break;
      case 'w':
	printf("%%wim");
	break;
      case 't':
	printf("%%tbr");
	break;
      case 'h':
	printf("%%hi(0xaaaaa)");
	break;
      case 'e':
      case 'f':
      case 'g':
	print_f_reg(0);
	break;
      case 'v':
      case 'B':
      case 'H':
	print_f_reg(1);
	break;
      case 'R':
      case 'V':
      case 'J':
	print_f_reg(3);
	break;
      case 'm':
      case 'M':
	printf("%%asr16");
	break;
      case 'S':
	/* special case set insn */
	break;
      case '+':
	putchar('+');
	break;
      case ']':
      case '[':
      case ',':
      case ' ':
	putchar(*arg);
	break;
      case '#':
	printf("0");
	break;
      case 'a':
	printf("a\t");
	break;
      default:
	printf("*** what's this garbage %c 0x%x?\n", *arg, (int) *arg);
      }
    }
    printf("\n");
  }
  printf("\n");
exit(0);
}


#define MAX_RS_REG 32

static
void
print_rs_reg(char type)
{
  static int i=0;

  printf("%%%s", reg_names[i++]);
  if (i >= MAX_RS_REG)
    i = 0;
}

#define MAX_FP_REG 64

static
void
print_f_reg(int align)
{
  static int i=32;

  printf("%%%s", reg_names[(i++) & ~align]);
  if (i >= MAX_FP_REG)
    i = 32;
}

static
void
print_imm_13(void)
{
  static int val = 0;

  val = (val+4) & 0x3ff;
  printf("0x%x", val);
}

static
void
print_imm_22(void)
{
  static int val = 0;

  val = (val + 4) & 0x3fffff;
  printf("0x%x", val);
}


#define MAX_ASI 255

static
void print_asi(void)
{
  static int i=0;

  printf("(%d)", i++);
  if (i >= MAX_ASI)
    i = 0;
}

#define MAX_CP_REG 32

static
void print_cp_reg(void)
{
  static int i=0;

  printf("%%c%d", i++);
  if (i >= MAX_CP_REG)
    i = 0;
}
