#objdump: -dr --prefix-addresses --show-raw-insn -M no-aliases
#name: MIPS32r2 sync instructions 1
#as: -32
#source: mips32r2-sync.s

# Check MIPS32r2 sync instructions assembly and disassembly

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 0000000f 	sync
[0-9a-f]+ <[^>]*> 0000008f 	sync	0x2
[0-9a-f]+ <[^>]*> 0000010f 	sync	0x4
[0-9a-f]+ <[^>]*> 0000020f 	sync	0x8
[0-9a-f]+ <[^>]*> 0000040f 	sync	0x10
[0-9a-f]+ <[^>]*> 0000044f 	sync	0x11
[0-9a-f]+ <[^>]*> 0000048f 	sync	0x12
[0-9a-f]+ <[^>]*> 000004cf 	sync	0x13
[0-9a-f]+ <[^>]*> 0000060f 	sync	0x18
[0-9a-f]+ <[^>]*> 0000000f 	sync
[0-9a-f]+ <[^>]*> 0000008f 	sync	0x2
[0-9a-f]+ <[^>]*> 0000010f 	sync	0x4
[0-9a-f]+ <[^>]*> 0000020f 	sync	0x8
[0-9a-f]+ <[^>]*> 0000040f 	sync	0x10
[0-9a-f]+ <[^>]*> 0000044f 	sync	0x11
[0-9a-f]+ <[^>]*> 0000048f 	sync	0x12
[0-9a-f]+ <[^>]*> 000004cf 	sync	0x13
[0-9a-f]+ <[^>]*> 0000060f 	sync	0x18
	\.\.\.
