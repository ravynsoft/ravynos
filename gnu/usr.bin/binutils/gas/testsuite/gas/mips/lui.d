#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS lui
#as: -32
#source: lui.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 3c020000 	lui	v0,0x0
[0-9a-f]+ <[^>]*> 3c02ffff 	lui	v0,0xffff
[0-9a-f]+ <[^>]*> 3c020008 	lui	v0,0x8
[0-9a-f]+ <[^>]*> 3c020008 	lui	v0,0x8
[0-9a-f]+ <[^>]*> 3c02000c 	lui	v0,0xc
[0-9a-f]+ <[^>]*> 3c02000c 	lui	v0,0xc
[0-9a-f]+ <[^>]*> 3c02000c 	lui	v0,0xc
[ 	]*[0-9a-f]+: R_MIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 3c020000 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS_LO16	ext
[0-9a-f]+ <[^>]*> 3c020020 	lui	v0,0x20
[ 	]*[0-9a-f]+: R_MIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 3c020028 	lui	v0,0x28
[ 	]*[0-9a-f]+: R_MIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 3c020000 	lui	v0,0x0
[0-9a-f]+ <[^>]*> 3c02ffff 	lui	v0,0xffff
	\.\.\.
