#name: Fixes for Silicon Errata
#source: errata_fixes.s
#as: -msilicon-errata=cpu4,cpu12,cpu19
#objdump: -d --prefix-addresses --show-raw-insn

.*: +file format .*msp.*

Disassembly of section .text:
0+0000 <[^>]*> 30 12 04 00[ 	]+push	#4		;
0+0004 <[^>]*> 30 12 08 00[ 	]+push	#8		;
0+0008 <[^>]*> 10 c3[ 	]+bic	#1,	r0	;r3 As==01$
0+000a <[^>]*> 10 d3[ 	]+bis	#1,	r0	;r3 As==01$
0+000c <[^>]*> 10 43[ 	]+br	#1		;r3 As==01$
0+000e <[^>]*> 10 92 c8 00[ 	]+cmp	&0x00c8,r0	;0x00c8
0+0012 <[^>]*> 03 43[ 	]+nop			
0+0014 <[^>]*> 00 b1[ 	]+bit	r1,	r0	;
0+0016 <[^>]*> 03 43[ 	]+nop			
0+0018 <[^>]*> 32 d0 10 00[ 	]+bis	#16,	r2	;#0x0010
0+001c <[^>]*> 03 43[ 	]+nop			
0+001e <[^>]*> 32 40 10 00[ 	]+mov	#16,	r2	;#0x0010
0+0022 <[^>]*> 03 43[ 	]+nop			
0+0024 <[^>]*> 32 e0 10 00[ 	]+xor	#16,	r2	;#0x0010
0+0028 <[^>]*> 03 43[ 	]+nop			
