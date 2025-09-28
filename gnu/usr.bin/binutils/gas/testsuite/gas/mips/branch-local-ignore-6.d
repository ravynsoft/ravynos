#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch local symbol relocation 6 (ignore branch ISA)
#as: -32 -mignore-branch-isa
#source: branch-local-6.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 03e00009 	jalr	zero,ra
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 00000000 	nop
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
[0-9a-f]+ <[^>]*> 4062 fffe 	bgezal	v0,00001028 <bar\+0x18>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	foo
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 4262 fffe 	bgezals	v0,00001030 <bar\+0x20>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	foo
[0-9a-f]+ <[^>]*> 4400      	not	s0,s0
[0-9a-f]+ <[^>]*> 4022 fffe 	bltzal	v0,00001036 <bar\+0x26>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	foo
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 4222 fffe 	bltzals	v0,0000103e <bar\+0x2e>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	foo
[0-9a-f]+ <[^>]*> 4400      	not	s0,s0
[0-9a-f]+ <[^>]*> 4260 fffe 	bals	00001044 <bar\+0x34>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	foo
[0-9a-f]+ <[^>]*> 4400      	not	s0,s0
[0-9a-f]+ <[^>]*> 001f 0f3c 	jr	ra
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
	\.\.\.
