#as: -march=mips2 -mabi=32
#objdump: -M gpr-names=numeric -dr
#name: MIPS ELF reloc 9

.*:     file format .*

Disassembly of section \.text:

0+00 <foo>:
   0:	8f840000 	lw	\$4,0\(\$28\)
			0: R_MIPS_GOT16	\.data
   4:	24840010 	addiu	\$4,\$4,16
			4: R_MIPS_LO16	\.data
   8:	8f840000 	lw	\$4,0\(\$28\)
			8: R_MIPS_GOT16	\.data
   c:	24840020 	addiu	\$4,\$4,32
			c: R_MIPS_LO16	\.data
  10:	8f840000 	lw	\$4,0\(\$28\)
			10: R_MIPS_GOT16	\.data
  14:	24847ffc 	addiu	\$4,\$4,32764
			14: R_MIPS_LO16	\.data
  18:	8f840001 	lw	\$4,1\(\$28\)
			18: R_MIPS_GOT16	\.data
  1c:	24848000 	addiu	\$4,\$4,-32768
			1c: R_MIPS_LO16	\.data
  20:	8f840001 	lw	\$4,1\(\$28\)
			20: R_MIPS_GOT16	\.data
  24:	2484fffc 	addiu	\$4,\$4,-4
			24: R_MIPS_LO16	\.data
  28:	8f840001 	lw	\$4,1\(\$28\)
			28: R_MIPS_GOT16	\.data
  2c:	24840000 	addiu	\$4,\$4,0
			2c: R_MIPS_LO16	\.data
  30:	8f840002 	lw	\$4,2\(\$28\)
			30: R_MIPS_GOT16	\.data
  34:	24848010 	addiu	\$4,\$4,-32752
			34: R_MIPS_LO16	\.data
  38:	8f840002 	lw	\$4,2\(\$28\)
			38: R_MIPS_GOT16	\.data
  3c:	2484f000 	addiu	\$4,\$4,-4096
			3c: R_MIPS_LO16	\.data
  40:	8f840002 	lw	\$4,2\(\$28\)
			40: R_MIPS_GOT16	\.data
  44:	2484ffff 	addiu	\$4,\$4,-1
			44: R_MIPS_LO16	\.data
  48:	8f840002 	lw	\$4,2\(\$28\)
			48: R_MIPS_GOT16	\.data
  4c:	24840000 	addiu	\$4,\$4,0
			4c: R_MIPS_LO16	\.data
  50:	8f840003 	lw	\$4,3\(\$28\)
			50: R_MIPS_GOT16	\.data
  54:	24841345 	addiu	\$4,\$4,4933
			54: R_MIPS_LO16	\.data
  58:	8f840000 	lw	\$4,0\(\$28\)
			58: R_MIPS_GPREL16	\.sdata
  5c:	8f840004 	lw	\$4,4\(\$28\)
			5c: R_MIPS_GPREL16	\.sdata
  60:	8f840004 	lw	\$4,4\(\$28\)
			60: R_MIPS_GPREL16	\.sdata
  64:	8f840008 	lw	\$4,8\(\$28\)
			64: R_MIPS_GPREL16	\.sdata
  68:	8f84000c 	lw	\$4,12\(\$28\)
			68: R_MIPS_GPREL16	\.sdata
  6c:	8f840014 	lw	\$4,20\(\$28\)
			6c: R_MIPS_GPREL16	\.sdata
  70:	8f840018 	lw	\$4,24\(\$28\)
			70: R_MIPS_GPREL16	\.sdata
	\.\.\.
