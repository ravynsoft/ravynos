#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative reference to absolute expression 6
#as: -32
#source: mips16-pcrel-absolute-6.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f222 6a34 	lui	v0,0x1234
[0-9a-f]+ <[^>]*> f66a fd58 	daddiu	v0,22136
[0-9a-f]+ <[^>]*> f222 6a34 	lui	v0,0x1234
[0-9a-f]+ <[^>]*> f66a 3a58 	ld	v0,22136\(v0\)
[0-9a-f]+ <[^>]*> f222 6a34 	lui	v0,0x1234
[0-9a-f]+ <[^>]*> f66a fd58 	daddiu	v0,22136
[0-9a-f]+ <[^>]*> f222 6a34 	lui	v0,0x1234
[0-9a-f]+ <[^>]*> f66a 3a58 	ld	v0,22136\(v0\)
[0-9a-f]+ <[^>]*> f222 6a34 	lui	v0,0x1234
[0-9a-f]+ <[^>]*> f2ef fd40 	daddiu	v0,31456
[0-9a-f]+ <[^>]*> f222 6a34 	lui	v0,0x1234
[0-9a-f]+ <[^>]*> f2ef 3a40 	ld	v0,31456\(v0\)
[0-9a-f]+ <[^>]*> f464 6a29 	lui	v0,0x2469
[0-9a-f]+ <[^>]*> f4f5 fd40 	daddiu	v0,-21280
[0-9a-f]+ <[^>]*> f464 6a29 	lui	v0,0x2469
[0-9a-f]+ <[^>]*> f4f5 3a40 	ld	v0,-21280\(v0\)
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
