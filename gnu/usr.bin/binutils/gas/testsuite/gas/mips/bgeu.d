#objdump: -dr --prefix-addresses -mmips:4000
#name: MIPS bgeu
#as: -32

# Test the bgeu macro.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> sltu	at,a0,a1
[0-9a-f]+ <[^>]*> beqz	at,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> beq	zero,a1,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> bnez	a0,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> sltiu	at,a0,2
[0-9a-f]+ <[^>]*> beqz	at,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> li	at,0x8000
[0-9a-f]+ <[^>]*> sltu	at,a0,at
[0-9a-f]+ <[^>]*> beqz	at,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> sltiu	at,a0,-32768
[0-9a-f]+ <[^>]*> beqz	at,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> lui	at,0x1
[0-9a-f]+ <[^>]*> sltu	at,a0,at
[0-9a-f]+ <[^>]*> beqz	at,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> lui	at,0x1
[0-9a-f]+ <[^>]*> ori	at,at,0xa5a5
[0-9a-f]+ <[^>]*> sltu	at,a0,at
[0-9a-f]+ <[^>]*> beqz	at,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> sltu	at,a1,a0
[0-9a-f]+ <[^>]*> bnez	at,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> bnez	a0,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> bnez	a0,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> sltu	at,a0,a1
[0-9a-f]+ <[^>]*> beqz	at,0+0088 <.*\+0x88>
[ 	]*88: R_MIPS_PC16	external_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> sltu	at,a1,a0
[0-9a-f]+ <[^>]*> bnez	at,0+0094 <.*\+0x94>
[ 	]*94: R_MIPS_PC16	external_label
[0-9a-f]+ <[^>]*> nop
	\.\.\.
