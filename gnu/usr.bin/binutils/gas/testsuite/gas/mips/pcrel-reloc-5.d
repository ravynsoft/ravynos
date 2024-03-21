#objdump: -dr --prefix-addresses --show-raw-insn -mmips:isa64r6
#name: MIPS local PC-relative relocations 5
#as: -32 --defsym reverse=1
#source: pcrel-reloc-4.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 03e00009 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 03e00009 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 1000fff7 	b	00010010 <bar>
[0-9a-f]+ <[^>]*> 1000fff6 	b	00010010 <bar>
[0-9a-f]+ <[^>]*> 1000fff5 	b	00010010 <bar>
[0-9a-f]+ <[^>]*> 1000fff4 	b	00010010 <bar>
[0-9a-f]+ <[^>]*> cbfffff3 	bc	00010010 <bar>
[0-9a-f]+ <[^>]*> cbfffff2 	bc	00010010 <bar>
[0-9a-f]+ <[^>]*> cbfffff1 	bc	00010010 <bar>
[0-9a-f]+ <[^>]*> cbfffff0 	bc	00010010 <bar>
[0-9a-f]+ <[^>]*> d85fffef 	beqzc	v0,00010010 <bar>
[0-9a-f]+ <[^>]*> d85fffee 	beqzc	v0,00010010 <bar>
[0-9a-f]+ <[^>]*> d85fffed 	beqzc	v0,00010010 <bar>
[0-9a-f]+ <[^>]*> d85fffec 	beqzc	v0,00010010 <bar>
[0-9a-f]+ <[^>]*> ec4fffec 	lwpc	v0,00010010 <bar>
[0-9a-f]+ <[^>]*> ec4fffeb 	lwpc	v0,00010010 <bar>
[0-9a-f]+ <[^>]*> ec4fffea 	lwpc	v0,00010010 <bar>
[0-9a-f]+ <[^>]*> ec4fffe9 	lwpc	v0,00010010 <bar>
[0-9a-f]+ <[^>]*> ec5bfff4 	ldpc	v0,00010010 <bar>
[0-9a-f]+ <[^>]*> ec5bfff4 	ldpc	v0,00010010 <bar>
[0-9a-f]+ <[^>]*> ec5bfff3 	ldpc	v0,00010010 <bar>
[0-9a-f]+ <[^>]*> ec5bfff3 	ldpc	v0,00010010 <bar>
[0-9a-f]+ <[^>]*> ec5effff 	auipc	v0,0xffff
[0-9a-f]+ <[^>]*> 2442ff80 	addiu	v0,v0,-128
[0-9a-f]+ <[^>]*> ec5effff 	auipc	v0,0xffff
[0-9a-f]+ <[^>]*> 2442ff78 	addiu	v0,v0,-136
[0-9a-f]+ <[^>]*> ec5effff 	auipc	v0,0xffff
[0-9a-f]+ <[^>]*> 2442ff70 	addiu	v0,v0,-144
[0-9a-f]+ <[^>]*> ec5effff 	auipc	v0,0xffff
[0-9a-f]+ <[^>]*> 2442ff68 	addiu	v0,v0,-152
	\.\.\.
