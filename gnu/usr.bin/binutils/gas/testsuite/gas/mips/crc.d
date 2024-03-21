#objdump: -pdr --prefix-addresses --show-raw-insn
#name: MIPS CRC
#as: -mcrc -32

# Test the CRC instructions

.*: +file format .*mips.*
#...
ASEs:
#...
	CRC ASE
#...

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 7ce4000f 	crc32b	a0,a3,a0
[0-9a-f]+ <[^>]*> 7ce4004f 	crc32h	a0,a3,a0
[0-9a-f]+ <[^>]*> 7ce4008f 	crc32w	a0,a3,a0
[0-9a-f]+ <[^>]*> 7ce4010f 	crc32cb	a0,a3,a0
[0-9a-f]+ <[^>]*> 7ce4014f 	crc32ch	a0,a3,a0
[0-9a-f]+ <[^>]*> 7ce4018f 	crc32cw	a0,a3,a0
	\.\.\.
