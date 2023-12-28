#objdump: -dr --prefix-addresses --show-raw-insn
#name: microMIPS relaxed branch to a different section
#as: -32 -mmicromips --relax-branch
#source: branch-section.s
#warning_output: branch-section.l

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.

Disassembly of section \.init:
[0-9a-f]+ <[^>]*> 45bf      	jrc	ra
	\.\.\.
