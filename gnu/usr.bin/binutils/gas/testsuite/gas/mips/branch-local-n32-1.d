#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch local symbol relocation 1 (n32)
#as: -n32 -march=from-abi
#source: branch-local-1.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 001f 0f3c 	jr	ra
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 40e2 0000 	beqzc	v0,00001018 <bar\+0x8>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	foo-0x4
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> cc00      	b	0000101e <bar\+0xe>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	foo-0x2
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 8d00      	beqz	v0,00001024 <bar\+0x14>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	foo-0x2
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 001f 0f3c 	jr	ra
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
	\.\.\.
