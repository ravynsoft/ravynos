#objdump: -dr --prefix-addresses -mmips:4000
#name: MIPS beq
#as: -32
#source: beq.s

# Test the beq macro.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> beq	a0,a1,0+0000 <.*>
[	]*0: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> beqz	a0,0+0008 <.*>
[	]*8: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> li	at,1
[0-9a-f]+ <[^>]*> beq	a0,at,0+0014 <.*>
[	]*14: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> li	at,0x8000
[0-9a-f]+ <[^>]*> beq	a0,at,0+0020 <.*>
[	]*20: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> li	at,-32768
[0-9a-f]+ <[^>]*> beq	a0,at,0+002c <.*>
[	]*2c: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> lui	at,0x1
[0-9a-f]+ <[^>]*> beq	a0,at,0+0038 <.*>
[	]*38: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> lui	at,0x1
[0-9a-f]+ <[^>]*> ori	at,at,0xa5a5
[0-9a-f]+ <[^>]*> beq	a0,at,0+0048 <.*>
[	]*48: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> bnez	a0,0+0050 <.*>
[	]*50: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> b	0+0058 <.*>
[ 	]*58: R_MIPS_PC16	external_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> bal	0+0060 <.*>
[ 	]*60: R_MIPS_PC16	external_label
[0-9a-f]+ <[^>]*> nop
	\.\.\.
