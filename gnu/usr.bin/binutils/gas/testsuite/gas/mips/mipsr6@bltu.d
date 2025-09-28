#objdump: -dr --prefix-addresses -mmips:4000
#name: MIPS bltu
#as: -32
#source: bltu.s

# Test the bltu macro.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> sltu	at,a0,a1
[0-9a-f]+ <[^>]*> bnez	at,0+0004 <.*>
[	]*4: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> bne	zero,a1,0+000c <.*>
[	]*c: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> beqz	a0,0+0014 <.*>
[	]*14: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> sltiu	at,a0,2
[0-9a-f]+ <[^>]*> bnez	at,0+0020 <.*>
[	]*20: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> li	at,0x8000
[0-9a-f]+ <[^>]*> sltu	at,a0,at
[0-9a-f]+ <[^>]*> bnez	at,0+0030 <.*>
[	]*30: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> sltiu	at,a0,-32768
[0-9a-f]+ <[^>]*> bnez	at,0+003c <.*>
[	]*3c: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> lui	at,0x1
[0-9a-f]+ <[^>]*> sltu	at,a0,at
[0-9a-f]+ <[^>]*> bnez	at,0+004c <.*>
[	]*4c: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> lui	at,0x1
[0-9a-f]+ <[^>]*> ori	at,at,0xa5a5
[0-9a-f]+ <[^>]*> sltu	at,a0,at
[0-9a-f]+ <[^>]*> bnez	at,0+0060 <.*>
[	]*60: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> sltu	at,a1,a0
[0-9a-f]+ <[^>]*> beqz	at,0+006c <.*>
[	]*6c: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> beqz	a0,0+0074 <.*>
[	]*74: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> beqz	a0,0+007c <.*>
[	]*7c: R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> sltu	at,a0,a1
[0-9a-f]+ <[^>]*> bnez	at,0+0088 <.*\+0x88>
[ 	]*88: R_MIPS_PC16	external_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> sltu	at,a1,a0
[0-9a-f]+ <[^>]*> beqz	at,0+0094 <.*\+0x94>
[ 	]*94: R_MIPS_PC16	external_label
[0-9a-f]+ <[^>]*> nop
	\.\.\.
