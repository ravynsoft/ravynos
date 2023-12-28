#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS and
#source: and.s
#as: -32

# Test the and macro (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> d084 0000 	andi	a0,a0,0x0
[0-9a-f]+ <[^>]*> d084 0001 	andi	a0,a0,0x1
[0-9a-f]+ <[^>]*> d084 8000 	andi	a0,a0,0x8000
[0-9a-f]+ <[^>]*> 3020 8000 	li	at,-32768
[0-9a-f]+ <[^>]*> 0024 2250 	and	a0,a0,at
[0-9a-f]+ <[^>]*> 41a1 0001 	lui	at,0x1
[0-9a-f]+ <[^>]*> 0024 2250 	and	a0,a0,at
[0-9a-f]+ <[^>]*> 41a1 0001 	lui	at,0x1
[0-9a-f]+ <[^>]*> 5021 a5a5 	ori	at,at,0xa5a5
[0-9a-f]+ <[^>]*> 0024 2250 	and	a0,a0,at
[0-9a-f]+ <[^>]*> 5085 0000 	ori	a0,a1,0x0
[0-9a-f]+ <[^>]*> 0004 22d0 	not	a0,a0
[0-9a-f]+ <[^>]*> 5085 0001 	ori	a0,a1,0x1
[0-9a-f]+ <[^>]*> 0004 22d0 	not	a0,a0
[0-9a-f]+ <[^>]*> 5085 8000 	ori	a0,a1,0x8000
[0-9a-f]+ <[^>]*> 0004 22d0 	not	a0,a0
[0-9a-f]+ <[^>]*> 3020 8000 	li	at,-32768
[0-9a-f]+ <[^>]*> 0025 22d0 	nor	a0,a1,at
[0-9a-f]+ <[^>]*> 41a1 0001 	lui	at,0x1
[0-9a-f]+ <[^>]*> 0025 22d0 	nor	a0,a1,at
[0-9a-f]+ <[^>]*> 41a1 0001 	lui	at,0x1
[0-9a-f]+ <[^>]*> 5021 a5a5 	ori	at,at,0xa5a5
[0-9a-f]+ <[^>]*> 0025 22d0 	nor	a0,a1,at
[0-9a-f]+ <[^>]*> 5085 0000 	ori	a0,a1,0x0
[0-9a-f]+ <[^>]*> 7085 0000 	xori	a0,a1,0x0
	\.\.\.
