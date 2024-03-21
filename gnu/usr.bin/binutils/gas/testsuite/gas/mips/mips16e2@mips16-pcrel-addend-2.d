#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative relocation with addend 2
#as: -32
#source: mips16-pcrel-addend-2.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f000 6a20 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar
[0-9a-f]+ <[^>]*> f222 4a14 	addiu	v0,4660
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar
[0-9a-f]+ <[^>]*> f000 6a20 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	bar
[0-9a-f]+ <[^>]*> f222 9a54 	lw	v0,4660\(v0\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	bar
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
	\.\.\.
