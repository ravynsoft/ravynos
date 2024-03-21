#source: callstub-1.s
#as: -a64 -mpower10
#ld: -melf64ppc -shared --plt-align=0 --hash-style=gnu
#objdump: -dr -Mpower10

.*

Disassembly of section \.text:

.*\.plt_call\.f1>:
.*:	(04 10 00 01|01 00 10 04) 	pld     r12,.*
.*:	(e5 80 .. ..|.. .. 80 e5) 
.*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*:	(4e 80 04 20|20 04 80 4e) 	bctr
.*:	(f8 41 00 18|18 00 41 f8) 	std     r2,24\(r1\)
.*:	(e9 82 80 28|28 80 82 e9) 	ld      r12,-32728\(r2\)
.*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*:	(4e 80 04 20|20 04 80 4e) 	bctr

.*\.plt_call\.f2>:
.*:	(04 10 00 01|01 00 10 04) 	pld     r12,.*
.*:	(e5 80 .. ..|.. .. 80 e5) 
.*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*:	(4e 80 04 20|20 04 80 4e) 	bctr
#...

.*<_start>:
.*:	(4b ff .. ..|.. .. ff 4b) 	bl      .*\.plt_call\.f1\+0x10>
.*:	(e8 41 00 18|18 00 41 e8) 	ld      r2,24\(r1\)
.*:	(4b ff .. ..|.. .. ff 4b) 	bl      .*\.plt_call\.f1>
.*:	(4b ff .. ..|.. .. ff 4b) 	bl      .*\.plt_call\.f2>
#pass
