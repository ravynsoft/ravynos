#objdump: -dr -M reg-names=numeric
#as: -32 -O2 -aln=branch-swap-lst.lst
#name: MIPS branch swapping with assembler listing
#source: branch-swap-3.s

# Check delay slot filling with a listing file works

.*: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <test>:
[ 0-9a-f]+:	0c000000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MIPS_26	func
[ 0-9a-f]+:	00408025 	move	\$16,\$2
[ 0-9a-f]+:	0c000000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MIPS_26	func
[ 0-9a-f]+:	26300001 	addiu	\$16,\$17,1
[ 0-9a-f]+:	0c000000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MIPS_26	func
[ 0-9a-f]+:	26300001 	addiu	\$16,\$17,1
[ 0-9a-f]+:	0c000000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MIPS_26	func
[ 0-9a-f]+:	26303fff 	addiu	\$16,\$17,16383
[ 0-9a-f]+:	0c000000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MIPS_26	func
[ 0-9a-f]+:	26303fff 	addiu	\$16,\$17,16383
[ 0-9a-f]+:	03e0000[89] 	jr	\$31
[ 0-9a-f]+:	00408025 	move	\$16,\$2
[ 0-9a-f]+:	03e0000[89] 	jr	\$31
[ 0-9a-f]+:	26300001 	addiu	\$16,\$17,1
[ 0-9a-f]+:	03e0000[89] 	jr	\$31
[ 0-9a-f]+:	26300001 	addiu	\$16,\$17,1
[ 0-9a-f]+:	03e0000[89] 	jr	\$31
[ 0-9a-f]+:	26303fff 	addiu	\$16,\$17,16383
[ 0-9a-f]+:	03e0000[89] 	jr	\$31
[ 0-9a-f]+:	26303fff 	addiu	\$16,\$17,16383
	\.\.\.
