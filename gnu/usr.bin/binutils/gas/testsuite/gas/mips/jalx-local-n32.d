#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS JALX local symbol relocation (n32)
#as: -n32 -march=from-abi
#source: jalx-local.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> f400 0000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	\.text\+0x1020
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> f000 0000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	\.text\+0x1020
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 001f 0f3c 	jr	ra
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c000000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	foo
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 74000000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	foo
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 03e00009 	jalr	zero,ra
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
	\.\.\.
