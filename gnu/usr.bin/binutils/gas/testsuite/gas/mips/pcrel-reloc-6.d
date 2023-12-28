#objdump: -dr --prefix-addresses --show-raw-insn -mmips:isa64r6
#name: MIPS local PC-relative relocations 6a
#as: -32 --defsym offset=0
#source: pcrel-reloc-6.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 10008000 	b	fffe0004 <foo\+0xfffe0004>
[0-9a-f]+ <[^>]*> 10007fff 	b	00020004 <foo\+0x20004>
[0-9a-f]+ <[^>]*> ca000000 	bc	f800000c <foo\+0xf800000c>
[0-9a-f]+ <[^>]*> c9ffffff 	bc	0800000c <foo\+0x800000c>
[0-9a-f]+ <[^>]*> d8500000 	beqzc	v0,ffc00014 <foo\+0xffc00014>
[0-9a-f]+ <[^>]*> d84fffff 	beqzc	v0,00400014 <foo\+0x400014>
[0-9a-f]+ <[^>]*> ec4c0000 	lwpc	v0,fff00018 <foo\+0xfff00018>
[0-9a-f]+ <[^>]*> ec4bffff 	lwpc	v0,00100018 <foo\+0x100018>
[0-9a-f]+ <[^>]*> ec5a0000 	ldpc	v0,fff00020 <foo\+0xfff00020>
[0-9a-f]+ <[^>]*> ec59ffff 	ldpc	v0,00100018 <foo\+0x100018>
	\.\.\.
