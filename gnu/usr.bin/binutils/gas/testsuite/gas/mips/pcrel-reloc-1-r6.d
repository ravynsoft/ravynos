#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS R6 local PC-relative relocations 1
#as: -32 -mips32r6
#source: pcrel-reloc-1.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 1000ffff 	b	00000000 <foo>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar
[0-9a-f]+ <[^>]*> 1000ffff 	b	00000004 <foo\+0x4>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar
[0-9a-f]+ <[^>]*> 1000ffff 	b	00000008 <foo\+0x8>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar
[0-9a-f]+ <[^>]*> 1000ffff 	b	0000000c <foo\+0xc>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 03e00009 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
