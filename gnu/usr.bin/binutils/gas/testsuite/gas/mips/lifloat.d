#objdump: -drz --prefix-addresses -mmips:3000
#name: MIPS lifloat
#as: -32 -mips1

# Test the li.d and li.s macros.

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> lui	at,0x0
[ 	]*0: R_MIPS_HI16	\.rodata
0+0004 <[^>]*> lw	a0,[-0-9]+\(at\)
[ 	]*4: R_MIPS_LO16	\.rodata
0+0008 <[^>]*> lw	a1,[-0-9]+\(at\)
[ 	]*8: R_MIPS_LO16	\.rodata
0+000c <[^>]*> lwc1	\$f[45],[-0-9]+\(gp\)
[ 	]*c: R_MIPS_LITERAL	\.lit8
0+0010 <[^>]*> lwc1	\$f[45],[-0-9]+\(gp\)
[ 	]*10: R_MIPS_LITERAL	\.lit8
0+0014 <[^>]*> lui	a0,0x3f8f
0+0018 <[^>]*> ori	a0,a0,0xcd36
0+001c <[^>]*> lwc1	\$f4,[-0-9]+\(gp\)
[ 	]*1c: R_MIPS_LITERAL	\.lit4
0+0020 <[^>]*> nop
#pass
