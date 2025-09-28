#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch local symbol relocation 6 (ignore branch ISA, n32)
#as: -n32 -march=from-abi -mignore-branch-isa
#source: branch-local-6.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 03e00009 	jalr	zero,ra
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 40e2 0000 	beqzc	v0,00001018 <bar\+0x8>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\.text\+0xffc
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> cc00      	b	0000101e <bar\+0xe>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	\.text\+0xffe
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 8d00      	beqz	v0,00001024 <bar\+0x14>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	\.text\+0xffe
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 4062 0000 	bgezal	v0,0000102c <bar\+0x1c>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\.text\+0xffc
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 4262 0000 	bgezals	v0,00001034 <bar\+0x24>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\.text\+0xffc
[0-9a-f]+ <[^>]*> 4400      	not	s0,s0
[0-9a-f]+ <[^>]*> 4022 0000 	bltzal	v0,0000103a <bar\+0x2a>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\.text\+0xffc
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 4222 0000 	bltzals	v0,00001042 <bar\+0x32>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\.text\+0xffc
[0-9a-f]+ <[^>]*> 4400      	not	s0,s0
[0-9a-f]+ <[^>]*> 4260 0000 	bals	00001048 <bar\+0x38>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\.text\+0xffc
[0-9a-f]+ <[^>]*> 4400      	not	s0,s0
[0-9a-f]+ <[^>]*> 001f 0f3c 	jr	ra
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
	\.\.\.
