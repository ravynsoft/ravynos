#objdump: -dr -M reg-names=numeric
#as: -32 -O2 -aln=branch-swap-lst.lst
#name: MIPS branch swapping with assembler listing
#source: branch-swap-3.s

# Check delay slot filling with a listing file works (MIPS16)

.*: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <test>:
[ 0-9a-f]+:	1800 0000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MIPS16_26	func
[ 0-9a-f]+:	6702      	move	\$16,\$2
[ 0-9a-f]+:	1800 0000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MIPS16_26	func
[ 0-9a-f]+:	4101      	addiu	\$16,\$17,1
[ 0-9a-f]+:	4101      	addiu	\$16,\$17,1
[ 0-9a-f]+:	1800 0000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MIPS16_26	func
[ 0-9a-f]+:	6500      	nop
[ 0-9a-f]+:	f7f7 410f 	addiu	\$16,\$17,16383
[ 0-9a-f]+:	1800 0000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MIPS16_26	func
[ 0-9a-f]+:	6500      	nop
[ 0-9a-f]+:	f7f7 410f 	addiu	\$16,\$17,16383
[ 0-9a-f]+:	1800 0000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MIPS16_26	func
[ 0-9a-f]+:	6500      	nop
[ 0-9a-f]+:	e820      	jr	\$31
[ 0-9a-f]+:	6702      	move	\$16,\$2
[ 0-9a-f]+:	e820      	jr	\$31
[ 0-9a-f]+:	4101      	addiu	\$16,\$17,1
[ 0-9a-f]+:	4101      	addiu	\$16,\$17,1
[ 0-9a-f]+:	e820      	jr	\$31
[ 0-9a-f]+:	6500      	nop
[ 0-9a-f]+:	f7f7 410f 	addiu	\$16,\$17,16383
[ 0-9a-f]+:	e820      	jr	\$31
[ 0-9a-f]+:	6500      	nop
[ 0-9a-f]+:	f7f7 410f 	addiu	\$16,\$17,16383
[ 0-9a-f]+:	e820      	jr	\$31
[ 0-9a-f]+:	6500      	nop
	\.\.\.
