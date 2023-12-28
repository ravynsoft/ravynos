#objdump: -dr --show-raw-insn
#name: microMIPS b16, bnez16, beqz16
#as: -32 -mmicromips
#source: micromips-b16.s

.*: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <test1>:
	\.\.\.

[0-9a-f]+ <test2>:
[ 0-9a-f]+:	cfff      	b	[0-9a-f]+ <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	.*
[ 0-9a-f]+:	0c00      	nop

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	ad7f      	bnez	v0,[0-9a-f]+ <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	.*
[ 0-9a-f]+:	0c00      	nop

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	8d7f      	beqz	v0,[0-9a-f]+ <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	.*
[ 0-9a-f]+:	0c00      	nop

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	cfff      	b	[0-9a-f]+ <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	.*
[ 0-9a-f]+:	0c00      	nop

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	ad7f      	bnez	v0,[0-9a-f]+ <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	.*
[ 0-9a-f]+:	0c00      	nop

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	8d7f      	beqz	v0,[0-9a-f]+ <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	.*
[ 0-9a-f]+:	0c00      	nop

[0-9a-f]+ <.*>:
[ 0-9a-f]+:	0c00      	nop
#pass
