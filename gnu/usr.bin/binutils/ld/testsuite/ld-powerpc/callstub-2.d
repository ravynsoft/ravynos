#as: -a64
#ld: -melf64ppc -shared --plt-align=0 --hash-style=gnu
#objdump: -dr

.*

Disassembly of section \.text:

.*\.plt_call\.f1>:
.*:	(f8 41 00 18|18 00 41 f8) 	std     r2,24\(r1\)
.*:	(7d 88 02 a6|a6 02 88 7d) 	mflr    r12
.*:	(42 9f 00 05|05 00 9f 42) 	bcl     .*
.*:	(7d 68 02 a6|a6 02 68 7d) 	mflr    r11
.*:	(7d 88 03 a6|a6 03 88 7d) 	mtlr    r12
.*:	(3d 8b 00 01|01 00 8b 3d) 	addis   r12,r11,1
.*:	(e9 8c .. ..|.. .. 8c e9) 	ld      r12,.*\(r12\)
.*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*:	(4e 80 04 20|20 04 80 4e) 	bctr

.*\.plt_call\.f2>:
.*:	(7d 88 02 a6|a6 02 88 7d) 	mflr    r12
.*:	(42 9f 00 05|05 00 9f 42) 	bcl     .*
.*:	(7d 68 02 a6|a6 02 68 7d) 	mflr    r11
.*:	(7d 88 03 a6|a6 03 88 7d) 	mtlr    r12
.*:	(3d 8b 00 01|01 00 8b 3d) 	addis   r12,r11,1
.*:	(e9 8c .. ..|.. .. 8c e9) 	ld      r12,.*\(r12\)
.*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*:	(4e 80 04 20|20 04 80 4e) 	bctr

.*<_start>:
.*:	(4b ff .. ..|.. .. ff 4b) 	bl      .*\.plt_call\.f1>
.*:	(e8 41 00 18|18 00 41 e8) 	ld      r2,24\(r1\)
.*:	(4b ff .. ..|.. .. ff 4b) 	bl      .*\.plt_call\.f1\+0x4>
.*:	(4b ff .. ..|.. .. ff 4b) 	bl      .*\.plt_call\.f2>
#pass
