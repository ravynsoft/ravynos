#objdump: -dr --prefix-addresses --show-raw-insn
#name: microMIPS branch to an external symbol
#as: -32 -KPIC -mmicromips
#source: branch-extern.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 40e0 fffe 	bc	00000000 <foo>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar
	\.\.\.
