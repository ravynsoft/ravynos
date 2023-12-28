#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS hardware rol/ror
#source: rol.s
#as: -32

# Test the rol and ror macros (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 00a0 09d0 	negu	at,a1
[0-9a-f]+ <[^>]*> 0081 20d0 	rorv	a0,a0,at
[0-9a-f]+ <[^>]*> 00c0 21d0 	negu	a0,a2
[0-9a-f]+ <[^>]*> 00a4 20d0 	rorv	a0,a1,a0
[0-9a-f]+ <[^>]*> 0084 f8c0 	ror	a0,a0,0x1f
[0-9a-f]+ <[^>]*> 0085 f8c0 	ror	a0,a1,0x1f
[0-9a-f]+ <[^>]*> 0085 00c0 	ror	a0,a1,0x0
[0-9a-f]+ <[^>]*> 0085 20d0 	rorv	a0,a0,a1
[0-9a-f]+ <[^>]*> 00a6 20d0 	rorv	a0,a1,a2
[0-9a-f]+ <[^>]*> 0084 08c0 	ror	a0,a0,0x1
[0-9a-f]+ <[^>]*> 0085 08c0 	ror	a0,a1,0x1
[0-9a-f]+ <[^>]*> 0085 00c0 	ror	a0,a1,0x0
[0-9a-f]+ <[^>]*> 0085 00c0 	ror	a0,a1,0x0
[0-9a-f]+ <[^>]*> 0085 f8c0 	ror	a0,a1,0x1f
[0-9a-f]+ <[^>]*> 0085 08c0 	ror	a0,a1,0x1
[0-9a-f]+ <[^>]*> 0085 00c0 	ror	a0,a1,0x0
[0-9a-f]+ <[^>]*> 0085 08c0 	ror	a0,a1,0x1
[0-9a-f]+ <[^>]*> 0085 f8c0 	ror	a0,a1,0x1f
	\.\.\.
