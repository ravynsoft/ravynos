#as: -mpower5
#objdump: -dr -Mpower5
#name: POWER5 tests

.*

Disassembly of section \.text:

0+00 <power5>:
.*:	(7d 40 e2 a6|a6 e2 40 7d) 	mfppr   r10
.*:	(7d 62 e2 a6|a6 e2 62 7d) 	mfppr32 r11
.*:	(7d 80 e3 a6|a6 e3 80 7d) 	mtppr   r12
.*:	(7d a2 e3 a6|a6 e3 a2 7d) 	mtppr32 r13
#pass
