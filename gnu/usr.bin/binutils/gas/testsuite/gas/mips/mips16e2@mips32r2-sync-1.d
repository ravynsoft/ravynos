#objdump: -dr --prefix-addresses --show-raw-insn -M no-aliases
#name: MIPS32r2 sync instructions 1
#as: -32
#source: mips32r2-sync.s

# Check MIPS32r2 sync instructions assembly and disassembly (MIPS16e2).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> f000 3014 	sync
[0-9a-f]+ <[^>]*> f080 3014 	sync	0x2
[0-9a-f]+ <[^>]*> f100 3014 	sync	0x4
[0-9a-f]+ <[^>]*> f200 3014 	sync	0x8
[0-9a-f]+ <[^>]*> f400 3014 	sync	0x10
[0-9a-f]+ <[^>]*> f440 3014 	sync	0x11
[0-9a-f]+ <[^>]*> f480 3014 	sync	0x12
[0-9a-f]+ <[^>]*> f4c0 3014 	sync	0x13
[0-9a-f]+ <[^>]*> f600 3014 	sync	0x18
[0-9a-f]+ <[^>]*> f000 3014 	sync
[0-9a-f]+ <[^>]*> f080 3014 	sync	0x2
[0-9a-f]+ <[^>]*> f100 3014 	sync	0x4
[0-9a-f]+ <[^>]*> f200 3014 	sync	0x8
[0-9a-f]+ <[^>]*> f400 3014 	sync	0x10
[0-9a-f]+ <[^>]*> f440 3014 	sync	0x11
[0-9a-f]+ <[^>]*> f480 3014 	sync	0x12
[0-9a-f]+ <[^>]*> f4c0 3014 	sync	0x13
[0-9a-f]+ <[^>]*> f600 3014 	sync	0x18
	\.\.\.
