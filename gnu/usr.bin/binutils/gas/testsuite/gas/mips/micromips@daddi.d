#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS DADDI instruction
#as: -32 --defsym micromips=1
#source: daddi.s

# Check MIPS DADDI instruction assembly (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 5862 7fdc 	daddi	v1,v0,511
[0-9a-f]+ <[^>]*> 58a4 801c 	daddi	a1,a0,-512
[0-9a-f]+ <[^>]*> 3020 0200 	li	at,512
[0-9a-f]+ <[^>]*> 5826 3910 	dadd	a3,a2,at
[0-9a-f]+ <[^>]*> 3020 fdff 	li	at,-513
[0-9a-f]+ <[^>]*> 5828 4910 	dadd	t1,t0,at
[0-9a-f]+ <[^>]*> 3020 7fff 	li	at,32767
[0-9a-f]+ <[^>]*> 582a 5910 	dadd	t3,t2,at
[0-9a-f]+ <[^>]*> 3020 8000 	li	at,-32768
[0-9a-f]+ <[^>]*> 582c 6910 	dadd	t5,t4,at
[0-9a-f]+ <[^>]*> 5020 8000 	li	at,0x8000
[0-9a-f]+ <[^>]*> 582e 7910 	dadd	t7,t6,at
[0-9a-f]+ <[^>]*> 41a1 ffff 	lui	at,0xffff
[0-9a-f]+ <[^>]*> 5021 7fff 	ori	at,at,0x7fff
[0-9a-f]+ <[^>]*> 5830 8910 	dadd	s1,s0,at
[0-9a-f]+ <[^>]*> 5020 8200 	li	at,0x8200
[0-9a-f]+ <[^>]*> 5832 9910 	dadd	s3,s2,at
[0-9a-f]+ <[^>]*> 41a1 ffff 	lui	at,0xffff
[0-9a-f]+ <[^>]*> 5021 7dff 	ori	at,at,0x7dff
[0-9a-f]+ <[^>]*> 5834 a910 	dadd	s5,s4,at
	\.\.\.
