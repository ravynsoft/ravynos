#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS JALR reloc unaligned/cross-mode (o32)
#as: -32
#source: jalr4.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 0320f809 	jalr	t9
[ 	]*[0-9a-f]+: R_MIPS_JALR	bar0
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 03200009 	jalr	zero,t9
[ 	]*[0-9a-f]+: R_MIPS_JALR	bar0
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 03200008 	jr	t9
[ 	]*[0-9a-f]+: R_MIPS_JALR	bar0
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 0320f809 	jalr	t9
[ 	]*[0-9a-f]+: R_MIPS_JALR	bar1
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 03200009 	jalr	zero,t9
[ 	]*[0-9a-f]+: R_MIPS_JALR	bar1
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 03200008 	jr	t9
[ 	]*[0-9a-f]+: R_MIPS_JALR	bar1
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 0320f809 	jalr	t9
[ 	]*[0-9a-f]+: R_MIPS_JALR	bar2
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 03200009 	jalr	zero,t9
[ 	]*[0-9a-f]+: R_MIPS_JALR	bar2
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 03200008 	jr	t9
[ 	]*[0-9a-f]+: R_MIPS_JALR	bar2
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
