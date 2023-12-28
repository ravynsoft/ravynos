#objdump: -dr --prefix-addresses --show-raw-insn -mmips:isa64r6
#name: MIPS local PC-relative relocations 4
#as: -32 -mips32r6
#source: pcrel-reloc-4.s

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
[0-9a-f]+ <[^>]*> cbffffff 	bc	00000010 <foo\+0x10>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar
[0-9a-f]+ <[^>]*> cbffffff 	bc	00000014 <foo\+0x14>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar
[0-9a-f]+ <[^>]*> cbffffff 	bc	00000018 <foo\+0x18>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar
[0-9a-f]+ <[^>]*> cbffffff 	bc	0000001c <foo\+0x1c>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar
[0-9a-f]+ <[^>]*> d85fffff 	beqzc	v0,00000020 <foo\+0x20>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar
[0-9a-f]+ <[^>]*> d85fffff 	beqzc	v0,00000024 <foo\+0x24>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar
[0-9a-f]+ <[^>]*> d85fffff 	beqzc	v0,00000028 <foo\+0x28>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar
[0-9a-f]+ <[^>]*> d85fffff 	beqzc	v0,0000002c <foo\+0x2c>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar
[0-9a-f]+ <[^>]*> ec480000 	lwpc	v0,00000030 <foo\+0x30>
[ 	]*[0-9a-f]+: R_MIPS_PC19_S2	bar
[0-9a-f]+ <[^>]*> ec480000 	lwpc	v0,00000034 <foo\+0x34>
[ 	]*[0-9a-f]+: R_MIPS_PC19_S2	bar
[0-9a-f]+ <[^>]*> ec480000 	lwpc	v0,00000038 <foo\+0x38>
[ 	]*[0-9a-f]+: R_MIPS_PC19_S2	bar
[0-9a-f]+ <[^>]*> ec480000 	lwpc	v0,0000003c <foo\+0x3c>
[ 	]*[0-9a-f]+: R_MIPS_PC19_S2	bar
[0-9a-f]+ <[^>]*> ec580000 	ldpc	v0,00000040 <foo\+0x40>
[ 	]*[0-9a-f]+: R_MIPS_PC18_S3	bar
[0-9a-f]+ <[^>]*> ec580000 	ldpc	v0,00000040 <foo\+0x40>
[ 	]*[0-9a-f]+: R_MIPS_PC18_S3	bar
[0-9a-f]+ <[^>]*> ec580000 	ldpc	v0,00000048 <foo\+0x48>
[ 	]*[0-9a-f]+: R_MIPS_PC18_S3	bar
[0-9a-f]+ <[^>]*> ec580000 	ldpc	v0,00000048 <foo\+0x48>
[ 	]*[0-9a-f]+: R_MIPS_PC18_S3	bar
[0-9a-f]+ <[^>]*> ec5e0001 	auipc	v0,0x1
[ 	]*[0-9a-f]+: R_MIPS_PCHI16	\.text
[0-9a-f]+ <[^>]*> 24420014 	addiu	v0,v0,20
[ 	]*[0-9a-f]+: R_MIPS_PCLO16	\.text
[0-9a-f]+ <[^>]*> ec5e0001 	auipc	v0,0x1
[ 	]*[0-9a-f]+: R_MIPS_PCHI16	\.text
[0-9a-f]+ <[^>]*> 24420014 	addiu	v0,v0,20
[ 	]*[0-9a-f]+: R_MIPS_PCLO16	\.text
[0-9a-f]+ <[^>]*> ec5e0001 	auipc	v0,0x1
[ 	]*[0-9a-f]+: R_MIPS_PCHI16	\.text
[0-9a-f]+ <[^>]*> 24420014 	addiu	v0,v0,20
[ 	]*[0-9a-f]+: R_MIPS_PCLO16	\.text
[0-9a-f]+ <[^>]*> ec5e0001 	auipc	v0,0x1
[ 	]*[0-9a-f]+: R_MIPS_PCHI16	\.text
[0-9a-f]+ <[^>]*> 24420014 	addiu	v0,v0,20
[ 	]*[0-9a-f]+: R_MIPS_PCLO16	\.text
	\.\.\.
[0-9a-f]+ <[^>]*> 03e00009 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 03e00009 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
