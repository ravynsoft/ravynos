#source: ext.s
#as: -a64
#objdump: -dr
#target: powerpc64*-*-*
# Just assembles an object for notoc.d

.*

Disassembly of section \.text:

0+ <ext>:
   0:	(00 00 4c 3c|3c 4c 00 00) 	addis   r2,r12,0
			(0|2): R_PPC64_REL16_HA	\.TOC\.(|\+0x2)
   4:	(00 00 42 38|38 42 00 00) 	addi    r2,r2,0
			(4|6): R_PPC64_REL16_LO	\.TOC\.\+0x(4|6)
   8:	(00 00 00 60|60 00 00 00) 	nop
   c:	(20 00 80 4e|4e 80 00 20) 	blr
