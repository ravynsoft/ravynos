#objdump: -dr --prefix-addresses --show-raw-insn
#name: microMIPS branch to a different section
#as: -32 -mmicromips
#source: branch-section.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 40e0 fffe 	bc	00000000 <foo>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar
	\.\.\.

Disassembly of section \.init:
[0-9a-f]+ <[^>]*> 45bf      	jrc	ra
	\.\.\.
