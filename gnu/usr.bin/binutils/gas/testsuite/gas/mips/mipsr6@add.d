#objdump: -dr --prefix-addresses
#name: MIPS add
#source: add.s
#as: -32

# Test the add macro.

.*: +file format .*mips.*

Disassembly of section .text:
[0-9a-f]+ <[^>]*> li	at,0
[0-9a-f]+ <[^>]*> add	a0,a0,at
[0-9a-f]+ <[^>]*> li	at,1
[0-9a-f]+ <[^>]*> add	a0,a0,at
[0-9a-f]+ <[^>]*> li	at,0x8000
[0-9a-f]+ <[^>]*> add	a0,a0,at
[0-9a-f]+ <[^>]*> li	at,-32768
[0-9a-f]+ <[^>]*> add	a0,a0,at
[0-9a-f]+ <[^>]*> lui	at,0x1
[0-9a-f]+ <[^>]*> add	a0,a0,at
[0-9a-f]+ <[^>]*> lui	at,0x1
[0-9a-f]+ <[^>]*> ori	at,at,0xa5a5
[0-9a-f]+ <[^>]*> add	a0,a0,at
[0-9a-f]+ <[^>]*> addiu	a0,a0,1
	\.\.\.
