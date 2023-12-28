#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch local symbol relocation 1
#as: -32
#source: branch-local-1.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 001f 0f3c 	jr	ra
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 40e2 fffe 	beqzc	v0,00001014 <bar\+0x4>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	foo
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> cfff      	b	0000101c <bar\+0xc>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	foo
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 8d7f      	beqz	v0,00001022 <bar\+0x12>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	foo
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 001f 0f3c 	jr	ra
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
	\.\.\.
