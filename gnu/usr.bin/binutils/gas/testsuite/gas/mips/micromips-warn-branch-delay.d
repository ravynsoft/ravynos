#objdump: -dr --show-raw-insn -M gpr-names=numeric
#name: microMIPS fixed-size branch delay slots
#as: -mmicromips
#source: micromips-warn-branch-delay.s
#warning_output: micromips-warn-branch-delay.l

.*: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <foo>:
[ 0-9a-f]+:	45e2      	jalrs	\$2
[ 0-9a-f]+:	0083 1250 	and	\$2,\$3,\$4
[ 0-9a-f]+:	45e2      	jalrs	\$2
[ 0-9a-f]+:	6043 9000 	swr	\$2,0\(\$3\)
[ 0-9a-f]+:	45e2      	jalrs	\$2
[ 0-9a-f]+:	6043 8000 	swl	\$2,0\(\$3\)
[ 0-9a-f]+:	45e2      	jalrs	\$2
[ 0-9a-f]+:	0272 8210 	mul	\$16,\$18,\$19
[ 0-9a-f]+:	45e2      	jalrs	\$2
[ 0-9a-f]+:	001f 8b90 	sltu	\$17,\$31,\$0
[ 0-9a-f]+:	45e2      	jalrs	\$2
[ 0-9a-f]+:	0220 8910 	add	\$17,\$0,\$17
[ 0-9a-f]+:	45e2      	jalrs	\$2
[ 0-9a-f]+:	01b1 8990 	sub	\$17,\$17,\$13
#pass
