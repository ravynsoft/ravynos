#objdump: -dr --prefix-addresses --show-raw-insn
#name: microMIPS relaxed branch to a weak symbol
#as: -32 -mmicromips --relax-branch --defsym align=12
#source: branch-weak.s
#warning_output: branch-weak.l

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 459f      	jr	ra
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
