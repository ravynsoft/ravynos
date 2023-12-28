#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch-misc-4
#as: -32
#source: branch-misc-4.s

# Verify PC-relative relocations do not overflow (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
([0-9a-f]+) <[^>]*> 40e0 fffe 	bc	\1 <foo>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar
([0-9a-f]+) <[^>]*> 40e0 fffe 	bc	\1 <\.Lfoo>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\.Lbar
	\.\.\.

Disassembly of section \.init:
([0-9a-f]+) <[^>]*> 40e0 fffe 	bc	\1 <bar>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	foo
([0-9a-f]+) <[^>]*> 40e0 fffe 	bc	\1 <\.Lbar>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\.Lfoo
	\.\.\.
