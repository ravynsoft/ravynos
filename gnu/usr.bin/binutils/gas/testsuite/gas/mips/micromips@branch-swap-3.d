#objdump: -dr -M reg-names=numeric
#as: -32 -O2 -aln=branch-swap-lst.lst
#name: MIPS branch swapping with assembler listing
#source: branch-swap-3.s

# Check delay slot filling with a listing file works (microMIPS)

.*: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <test>:
[ 0-9a-f]+:	0e02      	move	\$16,\$2
[ 0-9a-f]+:	f400 0000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	func
[ 0-9a-f]+:	0000 0000 	nop
[ 0-9a-f]+:	6c10      	addiu	\$16,\$17,1
[ 0-9a-f]+:	f400 0000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	func
[ 0-9a-f]+:	0000 0000 	nop
[ 0-9a-f]+:	f400 0000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	func
[ 0-9a-f]+:	3211 0001 	addiu	\$16,\$17,1
[ 0-9a-f]+:	f400 0000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	func
[ 0-9a-f]+:	3211 3fff 	addiu	\$16,\$17,16383
[ 0-9a-f]+:	f400 0000 	jal	0 <test>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	func
[ 0-9a-f]+:	3211 3fff 	addiu	\$16,\$17,16383
[ 0-9a-f]+:	459f      	jr	\$31
[ 0-9a-f]+:	0e02      	move	\$16,\$2
[ 0-9a-f]+:	459f      	jr	\$31
[ 0-9a-f]+:	6c10      	addiu	\$16,\$17,1
[ 0-9a-f]+:	459f      	jr	\$31
[ 0-9a-f]+:	3211 0001 	addiu	\$16,\$17,1
[ 0-9a-f]+:	459f      	jr	\$31
[ 0-9a-f]+:	3211 3fff 	addiu	\$16,\$17,16383
[ 0-9a-f]+:	459f      	jr	\$31
[ 0-9a-f]+:	3211 3fff 	addiu	\$16,\$17,16383
	\.\.\.
