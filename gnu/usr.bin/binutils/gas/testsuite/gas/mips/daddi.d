#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS DADDI instruction
#as: -32

# Check MIPS DADDI instruction assembly.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 604301ff 	daddi	v1,v0,511
[0-9a-f]+ <[^>]*> 6085fe00 	daddi	a1,a0,-512
[0-9a-f]+ <[^>]*> 60c70200 	daddi	a3,a2,512
[0-9a-f]+ <[^>]*> 6109fdff 	daddi	t1,t0,-513
[0-9a-f]+ <[^>]*> 614b7fff 	daddi	t3,t2,32767
[0-9a-f]+ <[^>]*> 618d8000 	daddi	t5,t4,-32768
[0-9a-f]+ <[^>]*> 34018000 	li	at,0x8000
[0-9a-f]+ <[^>]*> 01c1782c 	dadd	t7,t6,at
[0-9a-f]+ <[^>]*> 3c01ffff 	lui	at,0xffff
[0-9a-f]+ <[^>]*> 34217fff 	ori	at,at,0x7fff
[0-9a-f]+ <[^>]*> 0201882c 	dadd	s1,s0,at
[0-9a-f]+ <[^>]*> 34018200 	li	at,0x8200
[0-9a-f]+ <[^>]*> 0241982c 	dadd	s3,s2,at
[0-9a-f]+ <[^>]*> 3c01ffff 	lui	at,0xffff
[0-9a-f]+ <[^>]*> 34217dff 	ori	at,at,0x7dff
[0-9a-f]+ <[^>]*> 0281a82c 	dadd	s5,s4,at
	\.\.\.
