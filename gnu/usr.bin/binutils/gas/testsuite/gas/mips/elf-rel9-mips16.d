#as: -march=mips2 -mabi=32
#objdump: -M gpr-names=numeric -dr
#name: MIPS ELF reloc 9 (MIPS16 version)

.*:     file format .*

Disassembly of section \.text:

0+00 <foo>:
   0:	675c      	move	\$2,\$28
   2:	f000 9a80 	lw	\$4,0\(\$2\)
			2: R_MIPS16_GOT16	\.data
   6:	f000 4c10 	addiu	\$4,16
			6: R_MIPS16_LO16	\.data
   a:	f000 9a80 	lw	\$4,0\(\$2\)
			a: R_MIPS16_GOT16	\.data
   e:	f020 4c00 	addiu	\$4,32
			e: R_MIPS16_LO16	\.data
  12:	f000 9a80 	lw	\$4,0\(\$2\)
			12: R_MIPS16_GOT16	\.data
  16:	f7ef 4c1c 	addiu	\$4,32764
			16: R_MIPS16_LO16	\.data
  1a:	f000 9a81 	lw	\$4,1\(\$2\)
			1a: R_MIPS16_GOT16	\.data
  1e:	f010 4c00 	addiu	\$4,-32768
			1e: R_MIPS16_LO16	\.data
  22:	f000 9a81 	lw	\$4,1\(\$2\)
			22: R_MIPS16_GOT16	\.data
  26:	f7ff 4c1c 	addiu	\$4,-4
			26: R_MIPS16_LO16	\.data
  2a:	f000 9a81 	lw	\$4,1\(\$2\)
			2a: R_MIPS16_GOT16	\.data
  2e:	f000 4c00 	addiu	\$4,0
			2e: R_MIPS16_LO16	\.data
  32:	f000 9a82 	lw	\$4,2\(\$2\)
			32: R_MIPS16_GOT16	\.data
  36:	f010 4c10 	addiu	\$4,-32752
			36: R_MIPS16_LO16	\.data
  3a:	f000 9a82 	lw	\$4,2\(\$2\)
			3a: R_MIPS16_GOT16	\.data
  3e:	f01e 4c00 	addiu	\$4,-4096
			3e: R_MIPS16_LO16	\.data
  42:	f000 9a82 	lw	\$4,2\(\$2\)
			42: R_MIPS16_GOT16	\.data
  46:	f7ff 4c1f 	addiu	\$4,-1
			46: R_MIPS16_LO16	\.data
  4a:	f000 9a82 	lw	\$4,2\(\$2\)
			4a: R_MIPS16_GOT16	\.data
  4e:	f000 4c00 	addiu	\$4,0
			4e: R_MIPS16_LO16	\.data
  52:	f000 9a83 	lw	\$4,3\(\$2\)
			52: R_MIPS16_GOT16	\.data
  56:	f342 4c05 	addiu	\$4,4933
			56: R_MIPS16_LO16	\.data
  5a:	f000 9a80 	lw	\$4,0\(\$2\)
			5a: R_MIPS16_GPREL	\.sdata
  5e:	f000 9a84 	lw	\$4,4\(\$2\)
			5e: R_MIPS16_GPREL	\.sdata
  62:	f000 9a84 	lw	\$4,4\(\$2\)
			62: R_MIPS16_GPREL	\.sdata
  66:	f000 9a88 	lw	\$4,8\(\$2\)
			66: R_MIPS16_GPREL	\.sdata
  6a:	f000 9a8c 	lw	\$4,12\(\$2\)
			6a: R_MIPS16_GPREL	\.sdata
  6e:	f000 9a94 	lw	\$4,20\(\$2\)
			6e: R_MIPS16_GPREL	\.sdata
  72:	f000 9a98 	lw	\$4,24\(\$2\)
			72: R_MIPS16_GPREL	\.sdata
  76:	6500      	nop
#pass
