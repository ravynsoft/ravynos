#include <stdio.h>
#include "m88k-opcode.h"

static int reg(void);
static int creg(void);
static int cnst(void);
static void print_cond(void);
static void print_cmp(void);

void
main(
int argc,
char *argv[],
char *envp[])
{
    int32_t i, j;

	for(i = 0; i < NUMOPCODES - 1; i++){
	    printf("\t%s\t", m88k_opcodes[i].name);
	    if(m88k_opcodes[i].op[0].type == NIL)
		printf("\n");
	    for(j = 0; j < 3 && m88k_opcodes[i].op[j].type != NIL; j++){
		switch(m88k_opcodes[i].op[j].type){
		case CNST:
#ifdef SIGNED_IMMEDIATES
		case SCNST:
#endif
		    printf("0x%04x", cnst() );
		    break;
		case REG:
		    printf("r%d", reg() );
		    break;
		case BF:
		    printf("%d<%d>", reg(), reg() );
		    break;
		case REGSC:
		    printf("[r%d]", reg() );
		    break;
		case CRREG:
		    printf("cr%d", creg() );
		    break;
		case FCRREG:
		    printf("fcr%d", creg() );
		    break;
		case PCREL:
		    printf("undef");
		    break;
		case CONDMASK:
		    print_cond();
		    break;
		case CMPRSLT:
		    print_cmp();
		    break;
		case ROT:
		    printf("<%d>", reg() );
		    break;
		case E4ROT:
		    printf("<%d>", creg() & ~0x3);
		    break;
		case EREG:
		    printf("r%d", reg() & ~0x1);
		    break;
		case XREG:
		    printf("x%d", reg());
		    break;
		}
		if(j == 2 || m88k_opcodes[i].op[j+1].type == NIL)
		    printf("\n");
		else if(m88k_opcodes[i].op[j+1].type != REGSC)
		    printf(",");
	    }
	}
}

static
int
reg(void)
{
     static int x = 1;

	x = (x + 1) & 0x1f;
	return(x);
}

static
int
creg(void)
{
     static int x = 1;

	x = (x + 1) & 0x3f;
	return(x);
}

static
int
cnst(void)
{
     static int x = 1;

	x = (x + 1) & 0xffff;
	return(x);
}

static
void
print_cond(void)
{
     static int x = 1;

	x = (x + 1) & 0x1f;
	switch(x){
	case 0x02:
	    printf("eq0");
	    break;
	case 0x0d:
	    printf("ne0");
	    break;
	case 0x01:
	    printf("gt0");
	    break;
	case 0x0c:
	    printf("lt0");
	    break;
	case 0x03:
	    printf("ge0");
	    break;
	case 0x0e:
	    printf("le0");
	    break;
	default:
	    printf("%d", x);
	}
}

static
char *cmpslot[] = { "**", "**", "eq", "ne", "gt", "le", "lt", "ge",
		    "hi", "ls", "lo", "hs", "be", "nb", "he", "nh" };
static
void
print_cmp(void)
{
     static int x = 1;

	x = (x + 1) & 0x1f;
	if(x < 2 || x > 15)
	    printf("%d", x);
	else
	    printf("%s", cmpslot[x]);
}
