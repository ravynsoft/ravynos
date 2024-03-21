#as: -march=mips2 -mabi=32
#objdump: -M gpr-names=numeric -dr
#name: MIPS ELF reloc 8 (MIPS16 version)

.*:     file format .*

Disassembly of section \.text:

0+00 <foo>:
   0:	675c      	move	\$2,\$28
   2:	f000 6c00 	li	\$4,0
			2: R_MIPS16_HI16	gvar
   6:	f400 3480 	sll	\$4,16
   a:	f000 4c00 	addiu	\$4,0
			a: R_MIPS16_LO16	gvar
   e:	f000 9d80 	lw	\$4,0\(\$5\)
			e: R_MIPS16_LO16	gvar
  12:	f000 9982 	lw	\$4,2\(\$17\)
  16:	f000 9a80 	lw	\$4,0\(\$2\)
			16: R_MIPS16_GOT16	\.data
  1a:	f000 c4a0 	sb	\$5,0\(\$4\)
			1a: R_MIPS16_LO16	\.data
  1e:	f000 9a80 	lw	\$4,0\(\$2\)
			1e: R_MIPS16_GOT16	\.data
  22:	f000 4c00 	addiu	\$4,0
			22: R_MIPS16_LO16	\.data
  26:	f000 9a60 	lw	\$3,0\(\$2\)
			26: R_MIPS16_CALL16	gfunc
  2a:	f000 4c00 	addiu	\$4,0
			2a: R_MIPS16_CALL16	gfunc
  2e:	f000 9a80 	lw	\$4,0\(\$2\)
			2e: R_MIPS16_GPREL	gvar
  32:	f000 da80 	sw	\$4,0\(\$2\)
			32: R_MIPS16_GPREL	gvar
  36:	f000 4c00 	addiu	\$4,0
			36: R_MIPS16_GPREL	gvar
  3a:	f000 9a80 	lw	\$4,0\(\$2\)
			3a: R_MIPS16_GPREL	gvar
  3e:	f000 da80 	sw	\$4,0\(\$2\)
			3e: R_MIPS16_GPREL	gvar
  42:	f000 4c00 	addiu	\$4,0
			42: R_MIPS16_GPREL	gvar
  46:	6500      	nop
#pass
