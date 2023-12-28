#name: DINT NOP Insertions (MSP430 CPU)
#source: nop-dint.s
#as: -my -mn -mcpu=430
#warning_output: nop-dint-430.l
#objdump: -d --prefix-addresses --show-raw-insn

.*: +file format .*msp.*

Disassembly of section .text:
0x0+000 32 c2[ 	]+dint[ 	]+
0x0+002 03 43[ 	]+nop[ 	]+
0x0+004 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+008 32 c2[ 	]+dint[ 	]+
0x0+00a 03 43[ 	]+nop[ 	]+
0x0+00c 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+010 32 c2[ 	]+dint[ 	]+
0x0+012 03 43[ 	]+nop[ 	]+
0x0+014 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+018 02 43[ 	]+clr	r2		;
0x0+01a 03 43[ 	]+nop[ 	]+
0x0+01c 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+020 32 40 07 00[ 	]+mov	#7,	r2	;
0x0+024 03 43[ 	]+nop[ 	]+
0x0+026 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+02a 32 40 07 f0[ 	]+mov	#-4089,	r2	;#0xf007
0x0+02e 03 43[ 	]+nop[ 	]+
0x0+030 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+034 02 43[ 	]+clr	r2		;
0x0+036 03 43[ 	]+nop[ 	]+
0x0+038 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+03c 32 c2[ 	]+dint[ 	]+
0x0+03e 03 43[ 	]+nop[ 	]+
