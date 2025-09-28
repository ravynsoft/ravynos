#objdump: -dr --prefix-addresses --show-raw-insn
#name: microMIPS relaxed branch to an external symbol
#as: -32 -KPIC -mmicromips --relax-branch
#source: branch-extern.s
#warning_output: branch-extern.l

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	bar
[0-9a-f]+ <[^>]*> 3021 0000 	addiu	at,at,0
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	bar
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
	\.\.\.
