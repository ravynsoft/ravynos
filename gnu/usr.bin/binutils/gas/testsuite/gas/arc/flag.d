#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 2029 0000           	flag	r0
0x[0-9a-f]+ 2069 0040           	flag	0x1
0x[0-9a-f]+ 2069 0080           	flag	0x2
0x[0-9a-f]+ 2069 0100           	flag	0x4
0x[0-9a-f]+ 2069 0200           	flag	0x8
0x[0-9a-f]+ 2069 0400           	flag	0x10
0x[0-9a-f]+ 2069 0800           	flag	0x20
0x[0-9a-f]+ 20a9 0001           	flag	64
0x[0-9a-f]+ 20a9 0002           	flag	128
0x[0-9a-f]+ 2029 0f80 8000 0001 	flag	0x80000001
0x[0-9a-f]+ 20e9 000b           	flag.lt	r0
0x[0-9a-f]+ 20e9 0069           	flag.gt	0x1
0x[0-9a-f]+ 20e9 00a9           	flag.gt	0x2
0x[0-9a-f]+ 20e9 0129           	flag.gt	0x4
0x[0-9a-f]+ 20e9 0229           	flag.gt	0x8
0x[0-9a-f]+ 20e9 0429           	flag.gt	0x10
0x[0-9a-f]+ 20e9 0829           	flag.gt	0x20
0x[0-9a-f]+ 20e9 0f89 0000 0040 	flag.gt	0x40
0x[0-9a-f]+ 20e9 0f89 0000 0080 	flag.gt	0x80
0x[0-9a-f]+ 20e9 0f8a 8000 0001 	flag.ge	0x80000001
