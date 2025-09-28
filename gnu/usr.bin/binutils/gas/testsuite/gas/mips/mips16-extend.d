#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS16 explicit EXTEND encoding
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> f000 0000 	addiu	s0,sp,0
[0-9a-f]+ <[^>]*> f001 0000 	addiu	s0,sp,2048
[0-9a-f]+ <[^>]*> f5a5 0000 	addiu	s0,sp,11680
[0-9a-f]+ <[^>]*> f7ff 0000 	addiu	s0,sp,-32
[0-9a-f]+ <[^>]*> f123 0000 	addiu	s0,sp,6432
[0-9a-f]+ <[^>]*> f432 0000 	addiu	s0,sp,-27616
[0-9a-f]+ <[^>]*> f789 0000 	addiu	s0,sp,20352
[0-9a-f]+ <[^>]*> f7ff 0000 	addiu	s0,sp,-32
	\.\.\.
