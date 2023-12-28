#objdump: -dr --prefix-addresses --show-raw-insn -M gpr-names=numeric -mmips:5900
#name: MIPS R5900 workarounds (-mips3 -mfix-r5900)
#as: -mips3 -mfix-r5900

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 2403012c 	li	\$3,300
[0-9a-f]+ <[^>]*> 2063ffff 	addi	\$3,\$3,-1
[0-9a-f]+ <[^>]*> 2084ffff 	addi	\$4,\$4,-1
[0-9a-f]+ <[^>]*> 1460fffd 	bnez	\$3,[0-9a-f]+ <short_loop3>
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 2403012c 	li	\$3,300
[0-9a-f]+ <[^>]*> 2063ffff 	addi	\$3,\$3,-1
[0-9a-f]+ <[^>]*> 2084ffff 	addi	\$4,\$4,-1
[0-9a-f]+ <[^>]*> 20a5ffff 	addi	\$5,\$5,-1
[0-9a-f]+ <[^>]*> 20c6ffff 	addi	\$6,\$6,-1
[0-9a-f]+ <[^>]*> 20e7ffff 	addi	\$7,\$7,-1
[0-9a-f]+ <[^>]*> 1460fffa 	bnez	\$3,[0-9a-f]+ <short_loop6>
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 2403012c 	li	\$3,300
[0-9a-f]+ <[^>]*> 2063ffff 	addi	\$3,\$3,-1
[0-9a-f]+ <[^>]*> 2084ffff 	addi	\$4,\$4,-1
[0-9a-f]+ <[^>]*> 20a5ffff 	addi	\$5,\$5,-1
[0-9a-f]+ <[^>]*> 20c6ffff 	addi	\$6,\$6,-1
[0-9a-f]+ <[^>]*> 20e7ffff 	addi	\$7,\$7,-1
[0-9a-f]+ <[^>]*> 1460fffa 	bnez	\$3,[0-9a-f]+ <short_loop7>
[0-9a-f]+ <[^>]*> 2108ffff 	addi	\$8,\$8,-1
[0-9a-f]+ <[^>]*> 24040003 	li	\$4,3
	\.\.\.
