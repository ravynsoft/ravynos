#name: EINT NOP Insertions (MSP430 CPU)
#source: nop-eint.s
#as: -my -mn -mcpu=430
#warning_output: nop-eint-430.l
#objdump: -d --prefix-addresses --show-raw-insn

.*: +file format .*msp.*


Disassembly of section .text:
0x0+0000 32 d2[ 	]+eint[ 	]+
0x0+0002 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+0006 32 d2[ 	]+eint[ 	]+
0x0+0008 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+000c 32 d2[ 	]+eint[ 	]+
0x0+000e 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+0012 32 42[ 	]+mov	#8,	r2	;r2 As==11
0x0+0014 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+0018 32 40 0f 00[ 	]+mov	#15,	r2	;#0x000f
0x0+001c 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+0020 32 43[ 	]+mov	#-1,	r2	;r3 As==11
0x0+0022 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+0026 32 d2[ 	]+eint[ 	]+
0x0+0028 32 c2[ 	]+dint[ 	]+
0x0+002a 03 43[ 	]+nop[ 	]+
0x0+002c 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+0030 32 c2[ 	]+dint[ 	]+
0x0+0032 03 43[ 	]+nop[ 	]+
0x0+0034 32 d2[ 	]+eint[ 	]+
0x0+0036 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+003a 32 d2[ 	]+eint[ 	]+
