#name: MIPS r6 PC-relative hi+lo relocations
#ld: -Treloc-pcrel-r6.ld -e0
#as: -mips32r6
#objdump: -dr --prefix-addresses --show-raw-insn
#dump: reloc-pcrel-r6.d

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <test> ec9ed000 	auipc	a0,0xd000
[0-9a-f]+ <[^>]*> 24840001 	addiu	a0,a0,1
[0-9a-f]+ <[^>]*> ec9e0000 	auipc	a0,0x0
[0-9a-f]+ <[^>]*> 2484eff8 	addiu	a0,a0,-4104
[0-9a-f]+ <[^>]*> ec9e0001 	auipc	a0,0x1
[0-9a-f]+ <[^>]*> 2484eff0 	addiu	a0,a0,-4112
[0-9a-f]+ <[^>]*> ec9e4000 	auipc	a0,0x4000
[0-9a-f]+ <[^>]*> 2484ffe9 	addiu	a0,a0,-23
	\.\.\.
