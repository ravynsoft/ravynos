#objdump: -dr --prefix-addresses
#name: MIPS .aent directive
#as: -32

# Test the .aent directive retains function symbol type annotation.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <foo[^>]*> sllv	v0,a0,a2
[0-9a-f]+ <foo[^>]*> srav	t0,t2,t4
[0-9a-f]+ <bar[^>]*> sllv	v0,a0,a2
[0-9a-f]+ <bar[^>]*> srav	t0,t2,t4
	\.\.\.
