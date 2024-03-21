#objdump: -dr --prefix-addresses -mmips:4000
#name: MIPS bge
#as: -32
#source: bge.s

# Test the bge macro.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> slt	at,a0,a1
[0-9a-f]+ <[^>]*> beqz	at,0+0004 <.*>
[	]*4: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> bgez	a0,0+000c <.*>
[	]*c: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> blez	a1,0+0014 <.*>
[	]*14: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> bgez	a0,0+001c <.*>
[	]*1c: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> bgtz	a0,0+0024 <.*>
[	]*24: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> slti	at,a0,2
[0-9a-f]+ <[^>]*> beqz	at,0+0030 <.*>
[	]*30: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> li	at,0x8000
[0-9a-f]+ <[^>]*> slt	at,a0,at
[0-9a-f]+ <[^>]*> beqz	at,0+0040 <.*>
[	]*40: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> slti	at,a0,-32768
[0-9a-f]+ <[^>]*> beqz	at,0+004c <.*>
[	]*4c: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> lui	at,0x1
[0-9a-f]+ <[^>]*> slt	at,a0,at
[0-9a-f]+ <[^>]*> beqz	at,0+005c <.*>
[	]*5c: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> lui	at,0x1
[0-9a-f]+ <[^>]*> ori	at,at,0xa5a5
[0-9a-f]+ <[^>]*> slt	at,a0,at
[0-9a-f]+ <[^>]*> beqz	at,0+0070 <.*>
[	]*70: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> slt	at,a1,a0
[0-9a-f]+ <[^>]*> bnez	at,0+007c <.*>
[	]*7c: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> bgtz	a0,0+0084 <.*>
[	]*84: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> bltz	a1,0+008c <.*>
[	]*8c: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> bgtz	a0,0+0094 <.*>
[	]*94: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> slt	at,a0,a1
[0-9a-f]+ <[^>]*> beqz	at,0+00a0 <.*\+0xa0>
[ 	]*a0: R_MIPS_PC16	external_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> slt	at,a1,a0
[0-9a-f]+ <[^>]*> bnez	at,0+00ac <.*\+0xac>
[ 	]*ac: R_MIPS_PC16	external_label
[0-9a-f]+ <[^>]*> nop
	\.\.\.
