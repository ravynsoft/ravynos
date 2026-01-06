#include "i860-opcode.h"
#include <stdio.h>

extern int32_t random();

char *controlregs[] = { "fir", "psr", "epsr", "dirbase", "db", "fsr" };
#define NCREGS	(sizeof controlregs / sizeof controlregs[0])

char *textlabels[] = { "foo", "bar", "baz", "xork" };
#define NTLABELS	(sizeof textlabels / sizeof textlabels[0])

char *datalabels[] = { "data1", "data2", "data3", "data4" };
#define NDLABELS	(sizeof datalabels / sizeof datalabels[0])

/*
 * Traverse the opcode table, dumping out sample instructions.
 */
void
main(
int argc,
char *argv[],
char *envp[])
{
	int i;
	const char *arg;
	int r1, r2, rd;
	
	printf( "\t.text\n%s:", textlabels[0] );
	for ( i = 0; i < NUMOPCODES; ++i )
	{
		if ( i == (NUMOPCODES/3) )
			printf( "%s:", textlabels[1] );
		if ( i == (NUMOPCODES/2) )
			printf( "%s:", textlabels[2] );

		if ( (random() & 0x30) == 0 &&
		     (i860_opcodes[i].match & OP_PREFIX_MASK) == PREFIX_FPU )
		{
			printf( "\td.%s\t", i860_opcodes[i].name );
		}
		else
			printf( "\t%s\t", i860_opcodes[i].name );
		r1 = random() & 0x1F;
		do
			r2 = random() & 0x1F;
		while( (r2 & 0x1E) == (r1 & 0x1E) );
		do
			rd = random() & 0x1F;
		while( (rd & 0x1E) == (r1 & 0x1E) || (rd & 0x1E) == (r2 & 0x1E) );
		
		for ( arg = i860_opcodes[i].args; *arg != '\0'; ++arg )
		{
		    switch( *arg )
		    {
			case '1': /*    rs1 register, bits 11-15 of insn. */
   			case '2': /*    rs2 register, bits 21-25 of insn. */
   			case 'd': /*    rd register, bits 16-20 of insn.  */
				printf( "r%d", random() % 32);
				break;
				
   			case 'e': /*    frs1 floating point register, bits 11-15 of insn*/
				printf( "f%d", r1 );
				break;
				  
   			case 'f': /*    frs2 floating point register, bits 21-25 of insn*/
				printf( "f%d", r2 );
				break;
				  
   			case 'g': /*    frsd floating point register, bits 16-20 of insn*/ 
				printf( "f%d", rd );
				break;
				
   			case 'E': /*    frs1 floating point register, bits 11-15 of insn*/
				printf( "f%d", r1 & 0x1E );
				break;
				  
   			case 'F': /*    frs2 floating point register, bits 21-25 of insn*/
				printf( "f%d", r2 & 0x1E );
				break;
				  
   			case 'G': /*    frsd floating point register, bits 16-20 of insn*/ 
				printf( "f%d", rd & 0x1E );
				break;
				  
   			case 'H': /*    frsd floating point register, bits 16-20 of insn*/ 
				printf( "f%d", rd & 0x1C );
				break;

			case 'I': /*	16 bit High portion of address, I860_RELOC_HIGH. */
				printf( "h%%%s", datalabels[random() % NDLABELS] );
				break;
				
			case 'i': /*	16 bit byte address low half, */
   			case 'j': /*	16 bit short address, I860_RELOC_LOW1 */
   			case 'k': /*	16 bit word/int	address low half, I860_RELOC_LOW2 */
   			case 'l': /*	16 bit 8-byte address (double) low half */
   			case 'm': /*	16 bit 16-byte address (quad) low half */
   			case 'n': /*	16 bit byte aligned low half, split fields */
   			case 'o': /*	16 bit short aligned low half, split fields */
   			case 'p': /*	16 bit int/word aligned low half, split fields */
				printf( "l%%%s", datalabels[random() % NDLABELS] );
				break;

   			case 'J': /*	16 bit High portion of addr requiring adjustment*/
				printf( "ha%%%s", datalabels[random() % NDLABELS] );
				break;
							
			case 'K': /*	26 bit branch displacement */
   			case 'L': /*	16 bit split branch displacement */
				printf( textlabels[random() % NTLABELS] );
				break;
			
			case 'D': /* constant for shift opcode */	
   			case 'B': /*	5 bit immediate, for bte and btne insn */
				printf( "%d", random() % 32 );
				break;
				
   			case 'C': /*    Control Register */
				printf( controlregs[random() % NCREGS] );
				break;
				
			default:
				putchar( *arg );
				break;
		    }
		}
		putchar( '\n' );
	}
	printf( "%s:\n", textlabels[3] );
	printf( "nop\n" );
	printf( "\t.text\n" );
	printf( "data1:	nop\n" );
	printf( "data2:	nop\n" );
	printf( "data3:	nop\n" );
	printf( "data4:	nop\n" );

}
