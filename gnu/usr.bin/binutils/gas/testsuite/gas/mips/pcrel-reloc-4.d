#objdump: -dr --prefix-addresses --show-raw-insn -mmips:isa64r6
#name: MIPS local PC-relative relocations 4
#as: -32
#source: pcrel-reloc-4.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 1000001f 	b	00000080 <bar>
[0-9a-f]+ <[^>]*> 1000001e 	b	00000080 <bar>
[0-9a-f]+ <[^>]*> 1000001d 	b	00000080 <bar>
[0-9a-f]+ <[^>]*> 1000001c 	b	00000080 <bar>
[0-9a-f]+ <[^>]*> c800001b 	bc	00000080 <bar>
[0-9a-f]+ <[^>]*> c800001a 	bc	00000080 <bar>
[0-9a-f]+ <[^>]*> c8000019 	bc	00000080 <bar>
[0-9a-f]+ <[^>]*> c8000018 	bc	00000080 <bar>
[0-9a-f]+ <[^>]*> d8400017 	beqzc	v0,00000080 <bar>
[0-9a-f]+ <[^>]*> d8400016 	beqzc	v0,00000080 <bar>
[0-9a-f]+ <[^>]*> d8400015 	beqzc	v0,00000080 <bar>
[0-9a-f]+ <[^>]*> d8400014 	beqzc	v0,00000080 <bar>
[0-9a-f]+ <[^>]*> ec480014 	lwpc	v0,00000080 <bar>
[0-9a-f]+ <[^>]*> ec480013 	lwpc	v0,00000080 <bar>
[0-9a-f]+ <[^>]*> ec480012 	lwpc	v0,00000080 <bar>
[0-9a-f]+ <[^>]*> ec480011 	lwpc	v0,00000080 <bar>
[0-9a-f]+ <[^>]*> ec580008 	ldpc	v0,00000080 <bar>
[0-9a-f]+ <[^>]*> ec580008 	ldpc	v0,00000080 <bar>
[0-9a-f]+ <[^>]*> ec580007 	ldpc	v0,00000080 <bar>
[0-9a-f]+ <[^>]*> ec580007 	ldpc	v0,00000080 <bar>
[0-9a-f]+ <[^>]*> ec5e0001 	auipc	v0,0x1
[0-9a-f]+ <[^>]*> 2442ffc0 	addiu	v0,v0,-64
[0-9a-f]+ <[^>]*> ec5e0001 	auipc	v0,0x1
[0-9a-f]+ <[^>]*> 2442ffb8 	addiu	v0,v0,-72
[0-9a-f]+ <[^>]*> ec5e0001 	auipc	v0,0x1
[0-9a-f]+ <[^>]*> 2442ffb0 	addiu	v0,v0,-80
[0-9a-f]+ <[^>]*> ec5e0001 	auipc	v0,0x1
[0-9a-f]+ <[^>]*> 2442ffa8 	addiu	v0,v0,-88
	\.\.\.
[0-9a-f]+ <[^>]*> 03e00009 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 03e00009 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
