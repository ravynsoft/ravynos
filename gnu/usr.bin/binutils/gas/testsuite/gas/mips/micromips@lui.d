#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS lui
#as: -32
#source: lui.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 41a2 0000 	lui	v0,0x0
[0-9a-f]+ <[^>]*> 41a2 ffff 	lui	v0,0xffff
[0-9a-f]+ <[^>]*> 41a2 0008 	lui	v0,0x8
[0-9a-f]+ <[^>]*> 41a2 0008 	lui	v0,0x8
[0-9a-f]+ <[^>]*> 41a2 000c 	lui	v0,0xc
[0-9a-f]+ <[^>]*> 41a2 000c 	lui	v0,0xc
[0-9a-f]+ <[^>]*> 41a2 000d 	lui	v0,0xd
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 41a2 0000 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	ext
[0-9a-f]+ <[^>]*> 41a2 0021 	lui	v0,0x21
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 41a2 0029 	lui	v0,0x29
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 41a2 0000 	lui	v0,0x0
[0-9a-f]+ <[^>]*> 41a2 ffff 	lui	v0,0xffff
	\.\.\.
