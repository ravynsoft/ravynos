#source: relbrlt.s
#as: -a64
#ld: -melf64ppc --no-plt-align --no-ld-generated-unwind-info --emit-relocs
#objdump: -Dr

.*

Disassembly of section \.text:

0*100000c0 <_start>:
[0-9a-f	 ]*:	(49 bf 00 39|39 00 bf 49) 	bl      .*
[0-9a-f	 ]*: R_PPC64_REL24	far
[0-9a-f	 ]*:	(60 00 00 00|00 00 00 60) 	nop
[0-9a-f	 ]*:	(49 bf 00 25|25 00 bf 49) 	bl      .*
[0-9a-f	 ]*: R_PPC64_REL24	far2far
[0-9a-f	 ]*:	(60 00 00 00|00 00 00 60) 	nop
[0-9a-f	 ]*:	(49 bf 00 11|11 00 bf 49) 	bl      .*
[0-9a-f	 ]*: R_PPC64_REL24	huge
[0-9a-f	 ]*:	(60 00 00 00|00 00 00 60) 	nop
[0-9a-f	 ]*:	00 00 00 00 	\.long 0x0
[0-9a-f	 ]*:	(4b ff ff e4|e4 ff ff 4b) 	b       .* <_start>
	\.\.\.

[0-9a-f	 ]*<.*plt_branch.*>:
[0-9a-f	 ]*:	(e9 82 80 f8|f8 80 82 e9) 	ld      r12,-32520\(r2\)
[0-9a-f	 ]*: R_PPC64_TOC16_DS	\*ABS\*\+0x157f00f8
[0-9a-f	 ]*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
[0-9a-f	 ]*:	(4e 80 04 20|20 04 80 4e) 	bctr

[0-9a-f	 ]*<.*plt_branch.*>:
[0-9a-f	 ]*:	(e9 82 81 00|00 81 82 e9) 	ld      r12,-32512\(r2\)
[0-9a-f	 ]*: R_PPC64_TOC16_DS	\*ABS\*\+0x157f0100
[0-9a-f	 ]*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
[0-9a-f	 ]*:	(4e 80 04 20|20 04 80 4e) 	bctr

[0-9a-f	 ]*<.*long_branch.*>:
[0-9a-f	 ]*:	(49 bf 00 04|04 00 bf 49) 	b       .* <far>
[0-9a-f	 ]*: R_PPC64_REL24	far
	\.\.\.

0*137e00fc <far>:
[0-9a-f	 ]*:	(4e 80 00 20|20 00 80 4e) 	blr
	\.\.\.

0*13bf00f0 <far2far>:
[0-9a-f	 ]*:	(4e 80 00 20|20 00 80 4e) 	blr
	\.\.\.

0*157e00f4 <huge>:
[0-9a-f	 ]*:	(4e 80 00 20|20 00 80 4e) 	blr

Disassembly of section \.branch_lt:

0*157f00f8 .*:
[0-9a-f	 ]*:	(00 00 00 00|f4 00 7e 15) .*
[0-9a-f	 ]*: R_PPC64_RELATIVE	\*ABS\*\+0x157e00f4
[0-9a-f	 ]*:	(15 7e 00 f4|00 00 00 00) .*
[0-9a-f	 ]*:	(00 00 00 00|f0 00 bf 13) .*
[0-9a-f	 ]*: R_PPC64_RELATIVE	\*ABS\*\+0x13bf00f0
[0-9a-f	 ]*:	(13 bf 00 f0|00 00 00 00) .*
