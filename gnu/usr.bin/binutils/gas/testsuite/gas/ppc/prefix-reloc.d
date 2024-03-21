#as: -a64 -mpower10
#objdump: -dr -Mpower10
#name: Prefix insn relocations

.*

Disassembly of section \.text:

0+ <\.text>:
   0:	(00 00 00 06|06 00 00 00) 	pli     r9,0
   4:	(00 00 20 39|39 20 00 00) 
			0: R_PPC64_D34_HA30	ext
   8:	(46 17 29 79|79 29 17 46) 	sldi    r9,r9,34
   c:	(00 00 00 06|06 00 00 00) 	paddi   r9,r9,0
  10:	(00 00 29 39|39 29 00 00) 
			c: R_PPC64_D34_LO	ext
  14:	(00 00 10 04|04 10 00 00) 	pld     r3,0	# 14
  18:	(00 00 60 e4|e4 60 00 00) 
			14: R_PPC64_PCREL34	ext
  1c:	(00 00 10 04|04 10 00 00) 	pld     r4,0	# 1c
  20:	(00 00 80 e4|e4 80 00 00) 
			1c: R_PPC64_GOT_PCREL34	ext
  24:	(00 00 10 04|04 10 00 00) 	pld     r5,0	# 24
  28:	(00 00 a0 e4|e4 a0 00 00) 
			24: R_PPC64_PLT_PCREL34	ext
  2c:	(00 00 10 04|04 10 00 00) 	pld     r6,0	# 2c
  30:	(00 00 c0 e4|e4 c0 00 00) 
			2c: R_PPC64_PCREL34	ext
  34:	(00 00 00 04|04 00 00 00) 	pld     r7,0\(0\)
  38:	(00 00 e0 e4|e4 e0 00 00) 
			34: R_PPC64_D34	ext
  3c:	(00 00 00 60|60 00 00 00) 	nop
  40:	(00 00 10 04|04 10 00 00) 	pld     r8,0	# 40
  44:	(00 00 00 e5|e5 00 00 00) 
			40: R_PPC64_PCREL34	ext
