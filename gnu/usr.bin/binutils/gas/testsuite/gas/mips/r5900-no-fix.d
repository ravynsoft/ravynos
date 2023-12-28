#objdump: -dr --prefix-addresses --show-raw-insn -M gpr-names=numeric -mmips:5900
#name: MIPS R5900 workarounds disabled (-mno-fix-r5900)
#as: -march=r5900 -mtune=r5900 -mno-fix-r5900

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 2403012c 	li	\$3,300
[0-9a-f]+ <[^>]*> 2063ffff 	addi	\$3,\$3,-1
[0-9a-f]+ <[^>]*> 1460fffe 	bnez	\$3,[0-9a-f]+ <short_loop_no_mfix_r5900>
[0-9a-f]+ <[^>]*> 2084ffff 	addi	\$4,\$4,-1
[0-9a-f]+ <[^>]*> 24040003 	li	\$4,3
	\.\.\.
