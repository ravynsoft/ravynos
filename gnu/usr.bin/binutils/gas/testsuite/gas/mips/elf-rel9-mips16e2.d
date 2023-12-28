#as: -march=mips32r2 -mmips16e2 -mabi=32
#objdump: -M gpr-names=numeric -dr
#name: MIPS ELF reloc 9 (MIPS16e2 version)

.*:     file format .*

Disassembly of section \.text:

0+00 <foo>:
[ 	]*[0-9a-f]+:	659a      	move	\$28,\$2
[ 	]*[0-9a-f]+:	f000 9420 	lw	\$4,0\(\$28\)
[ 	]*[0-9a-f]+: R_MIPS16_GOT16	\.data
[ 	]*[0-9a-f]+:	f000 4c10 	addiu	\$4,16
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 9420 	lw	\$4,0\(\$28\)
[ 	]*[0-9a-f]+: R_MIPS16_GOT16	\.data
[ 	]*[0-9a-f]+:	f020 4c00 	addiu	\$4,32
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 9420 	lw	\$4,0\(\$28\)
[ 	]*[0-9a-f]+: R_MIPS16_GOT16	\.data
[ 	]*[0-9a-f]+:	f7ef 4c1c 	addiu	\$4,32764
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 9421 	lw	\$4,1\(\$28\)
[ 	]*[0-9a-f]+: R_MIPS16_GOT16	\.data
[ 	]*[0-9a-f]+:	f010 4c00 	addiu	\$4,-32768
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 9421 	lw	\$4,1\(\$28\)
[ 	]*[0-9a-f]+: R_MIPS16_GOT16	\.data
[ 	]*[0-9a-f]+:	f7ff 4c1c 	addiu	\$4,-4
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 9421 	lw	\$4,1\(\$28\)
[ 	]*[0-9a-f]+: R_MIPS16_GOT16	\.data
[ 	]*[0-9a-f]+:	f000 4c00 	addiu	\$4,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 9422 	lw	\$4,2\(\$28\)
[ 	]*[0-9a-f]+: R_MIPS16_GOT16	\.data
[ 	]*[0-9a-f]+:	f010 4c10 	addiu	\$4,-32752
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 9422 	lw	\$4,2\(\$28\)
[ 	]*[0-9a-f]+: R_MIPS16_GOT16	\.data
[ 	]*[0-9a-f]+:	f01e 4c00 	addiu	\$4,-4096
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 9422 	lw	\$4,2\(\$28\)
[ 	]*[0-9a-f]+: R_MIPS16_GOT16	\.data
[ 	]*[0-9a-f]+:	f7ff 4c1f 	addiu	\$4,-1
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 9422 	lw	\$4,2\(\$28\)
[ 	]*[0-9a-f]+: R_MIPS16_GOT16	\.data
[ 	]*[0-9a-f]+:	f000 4c00 	addiu	\$4,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 9423 	lw	\$4,3\(\$28\)
[ 	]*[0-9a-f]+: R_MIPS16_GOT16	\.data
[ 	]*[0-9a-f]+:	f342 4c05 	addiu	\$4,4933
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 9420 	lw	\$4,0\(\$28\)
[ 	]*[0-9a-f]+: R_MIPS16_GPREL	\.sdata
[ 	]*[0-9a-f]+:	f000 9424 	lw	\$4,4\(\$28\)
[ 	]*[0-9a-f]+: R_MIPS16_GPREL	\.sdata
[ 	]*[0-9a-f]+:	f000 9424 	lw	\$4,4\(\$28\)
[ 	]*[0-9a-f]+: R_MIPS16_GPREL	\.sdata
[ 	]*[0-9a-f]+:	f000 9428 	lw	\$4,8\(\$28\)
[ 	]*[0-9a-f]+: R_MIPS16_GPREL	\.sdata
[ 	]*[0-9a-f]+:	f000 942c 	lw	\$4,12\(\$28\)
[ 	]*[0-9a-f]+: R_MIPS16_GPREL	\.sdata
[ 	]*[0-9a-f]+:	f000 9434 	lw	\$4,20\(\$28\)
[ 	]*[0-9a-f]+: R_MIPS16_GPREL	\.sdata
[ 	]*[0-9a-f]+:	f000 9438 	lw	\$4,24\(\$28\)
[ 	]*[0-9a-f]+: R_MIPS16_GPREL	\.sdata
	\.\.\.
