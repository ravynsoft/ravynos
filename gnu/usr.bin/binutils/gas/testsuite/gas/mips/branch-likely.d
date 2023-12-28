#objdump: -dr --prefix-addresses -m mips:4000
#name: MIPS branch-likely instructions
#as: -32

# Check branch-likely instructions

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> beqzl	a0,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> bnezl	a0,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> beqzl	a0,0+0010 <.*\+0x10>
[ 	]*10: R_MIPS_PC16	external_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> bnezl	a0,0+0018 <.*\+0x18>
[ 	]*18: R_MIPS_PC16	external_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> slt	at,a0,a1
[0-9a-f]+ <[^>]*> beqzl	at,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> slt	at,a1,a0
[0-9a-f]+ <[^>]*> bnezl	at,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> slt	at,a0,a1
[0-9a-f]+ <[^>]*> beqzl	at,0+003c <.*\+0x3c>
[ 	]*3c: R_MIPS_PC16	external_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> slt	at,a1,a0
[0-9a-f]+ <[^>]*> bnezl	at,0+0048 <.*\+0x48>
[ 	]*48: R_MIPS_PC16	external_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> sltu	at,a0,a1
[0-9a-f]+ <[^>]*> beqzl	at,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> sltu	at,a1,a0
[0-9a-f]+ <[^>]*> bnezl	at,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> sltu	at,a0,a1
[0-9a-f]+ <[^>]*> beqzl	at,0+006c <.*\+0x6c>
[ 	]*6c: R_MIPS_PC16	external_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> sltu	at,a1,a0
[0-9a-f]+ <[^>]*> bnezl	at,0+0078 <.*\+0x78>
[ 	]*78: R_MIPS_PC16	external_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> slt	at,a0,a1
[0-9a-f]+ <[^>]*> bnezl	at,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> slt	at,a1,a0
[0-9a-f]+ <[^>]*> beqzl	at,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> slt	at,a0,a1
[0-9a-f]+ <[^>]*> bnezl	at,0+009c <.*\+0x9c>
[ 	]*9c: R_MIPS_PC16	external_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> slt	at,a1,a0
[0-9a-f]+ <[^>]*> beqzl	at,0+00a8 <.*\+0xa8>
[ 	]*a8: R_MIPS_PC16	external_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> sltu	at,a0,a1
[0-9a-f]+ <[^>]*> bnezl	at,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> sltu	at,a1,a0
[0-9a-f]+ <[^>]*> beqzl	at,0+0000 <.*>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> sltu	at,a0,a1
[0-9a-f]+ <[^>]*> bnezl	at,0+00cc <.*\+0xcc>
[ 	]*cc: R_MIPS_PC16	external_label
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> sltu	at,a1,a0
[0-9a-f]+ <[^>]*> beqzl	at,0+00d8 <.*\+0xd8>
[ 	]*d8: R_MIPS_PC16	external_label
[0-9a-f]+ <[^>]*> nop
	\.\.\.
