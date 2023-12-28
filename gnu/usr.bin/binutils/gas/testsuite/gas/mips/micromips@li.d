#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS li
#source: li.s
#as: -32

# Test the li macro (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> ee00      	li	a0,0
[0-9a-f]+ <[^>]*> ee01      	li	a0,1
[0-9a-f]+ <[^>]*> 5080 8000 	li	a0,0x8000
[0-9a-f]+ <[^>]*> 3080 8000 	li	a0,-32768
[0-9a-f]+ <[^>]*> 41a4 0001 	lui	a0,0x1
[0-9a-f]+ <[^>]*> 41a4 0001 	lui	a0,0x1
[0-9a-f]+ <[^>]*> 5084 a5a5 	ori	a0,a0,0xa5a5
	\.\.\.
