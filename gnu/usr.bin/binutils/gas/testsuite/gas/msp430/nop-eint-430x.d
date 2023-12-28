#name: EINT NOP Insertions (MSP430X CPU)
#source: nop-eint.s
#as: -my -mn -mcpu=430x
#warning_output: nop-eint-430x.l
#objdump: -d --prefix-addresses --show-raw-insn

.*: +file format .*msp.*


Disassembly of section .text:
0x0+0000 03 43[ 	]+nop[ 	]+
0x0+0002 32 d2[ 	]+eint[ 	]+
0x0+0004 03 43[ 	]+nop[ 	]+
0x0+0006 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+000a 03 43[ 	]+nop[ 	]+
0x0+000c 32 d2[ 	]+eint[ 	]+
0x0+000e 03 43[ 	]+nop[ 	]+
0x0+0010 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+0014 03 43[ 	]+nop[ 	]+
0x0+0016 32 d2[ 	]+eint[ 	]+
0x0+0018 03 43[ 	]+nop[ 	]+
0x0+001a 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+001e 03 43[ 	]+nop[ 	]+
0x0+0020 32 42[ 	]+mov	#8,	r2	;r2 As==11
0x0+0022 03 43[ 	]+nop[ 	]+
0x0+0024 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+0028 03 43[ 	]+nop[ 	]+
0x0+002a 32 40 0f 00[ 	]+mov	#15,	r2	;#0x000f
0x0+002e 03 43[ 	]+nop[ 	]+
0x0+0030 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+0034 03 43[ 	]+nop[ 	]+
0x0+0036 32 43[ 	]+mov	#-1,	r2	;r3 As==11
0x0+0038 03 43[ 	]+nop[ 	]+
0x0+003a 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+003e 03 43[ 	]+nop[ 	]+
0x0+0040 32 d2[ 	]+eint[ 	]+
0x0+0042 03 43[ 	]+nop[ 	]+
0x0+0044 32 c2[ 	]+dint[ 	]+
0x0+0046 03 43[ 	]+nop[ 	]+
0x0+0048 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+004c 32 c2[ 	]+dint[ 	]+
0x0+004e 03 43[ 	]+nop[ 	]+
0x0+0050 32 d2[ 	]+eint[ 	]+
0x0+0052 03 43[ 	]+nop[ 	]+
0x0+0054 1a 42 00 00[ 	]+mov	&0x0000,r10	;0x0000
0x0+0058 03 43[ 	]+nop[ 	]+
0x0+005a 32 d2[ 	]+eint[ 	]+
0x0+005c 03 43[ 	]+nop[ 	]+
