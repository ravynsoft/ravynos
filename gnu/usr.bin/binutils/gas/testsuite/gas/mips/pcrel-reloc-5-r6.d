#objdump: -dr --prefix-addresses --show-raw-insn -mmips:isa64r6
#name: MIPS local PC-relative relocations 5
#as: -32 -mips32r6 --defsym reverse=1
#source: pcrel-reloc-4.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 03e00009 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 03e00009 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 1000ffff 	b	00010030 <foo>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar
[0-9a-f]+ <[^>]*> 1000ffff 	b	00010034 <foo\+0x4>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar
[0-9a-f]+ <[^>]*> 1000ffff 	b	00010038 <foo\+0x8>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar
[0-9a-f]+ <[^>]*> 1000ffff 	b	0001003c <foo\+0xc>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar
[0-9a-f]+ <[^>]*> cbffffff 	bc	00010040 <foo\+0x10>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar
[0-9a-f]+ <[^>]*> cbffffff 	bc	00010044 <foo\+0x14>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar
[0-9a-f]+ <[^>]*> cbffffff 	bc	00010048 <foo\+0x18>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar
[0-9a-f]+ <[^>]*> cbffffff 	bc	0001004c <foo\+0x1c>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar
[0-9a-f]+ <[^>]*> d85fffff 	beqzc	v0,00010050 <foo\+0x20>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar
[0-9a-f]+ <[^>]*> d85fffff 	beqzc	v0,00010054 <foo\+0x24>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar
[0-9a-f]+ <[^>]*> d85fffff 	beqzc	v0,00010058 <foo\+0x28>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar
[0-9a-f]+ <[^>]*> d85fffff 	beqzc	v0,0001005c <foo\+0x2c>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar
[0-9a-f]+ <[^>]*> ec480000 	lwpc	v0,00010060 <foo\+0x30>
[ 	]*[0-9a-f]+: R_MIPS_PC19_S2	bar
[0-9a-f]+ <[^>]*> ec480000 	lwpc	v0,00010064 <foo\+0x34>
[ 	]*[0-9a-f]+: R_MIPS_PC19_S2	bar
[0-9a-f]+ <[^>]*> ec480000 	lwpc	v0,00010068 <foo\+0x38>
[ 	]*[0-9a-f]+: R_MIPS_PC19_S2	bar
[0-9a-f]+ <[^>]*> ec480000 	lwpc	v0,0001006c <foo\+0x3c>
[ 	]*[0-9a-f]+: R_MIPS_PC19_S2	bar
[0-9a-f]+ <[^>]*> ec580000 	ldpc	v0,00010070 <foo\+0x40>
[ 	]*[0-9a-f]+: R_MIPS_PC18_S3	bar
[0-9a-f]+ <[^>]*> ec580000 	ldpc	v0,00010070 <foo\+0x40>
[ 	]*[0-9a-f]+: R_MIPS_PC18_S3	bar
[0-9a-f]+ <[^>]*> ec580000 	ldpc	v0,00010078 <foo\+0x48>
[ 	]*[0-9a-f]+: R_MIPS_PC18_S3	bar
[0-9a-f]+ <[^>]*> ec580000 	ldpc	v0,00010078 <foo\+0x48>
[ 	]*[0-9a-f]+: R_MIPS_PC18_S3	bar
[0-9a-f]+ <[^>]*> ec5e0000 	auipc	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS_PCHI16	\.text
[0-9a-f]+ <[^>]*> 24420004 	addiu	v0,v0,4
[ 	]*[0-9a-f]+: R_MIPS_PCLO16	\.text
[0-9a-f]+ <[^>]*> ec5e0000 	auipc	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS_PCHI16	\.text
[0-9a-f]+ <[^>]*> 24420004 	addiu	v0,v0,4
[ 	]*[0-9a-f]+: R_MIPS_PCLO16	\.text
[0-9a-f]+ <[^>]*> ec5e0000 	auipc	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS_PCHI16	\.text
[0-9a-f]+ <[^>]*> 24420004 	addiu	v0,v0,4
[ 	]*[0-9a-f]+: R_MIPS_PCLO16	\.text
[0-9a-f]+ <[^>]*> ec5e0000 	auipc	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS_PCHI16	\.text
[0-9a-f]+ <[^>]*> 24420004 	addiu	v0,v0,4
[ 	]*[0-9a-f]+: R_MIPS_PCLO16	\.text
	\.\.\.
