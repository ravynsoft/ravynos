#name: MIPS JAL/JALX addend calculation
#source: ../../../gas/testsuite/gas/mips/jalx-addend.s
#ld: -Ttext 0x1c000000 -e 0x1c000000
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> f400 0801 	jal	0*18001002 <.*>
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> f100 0409 	jalx	0*14001024 <.*>
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 001f 0f3c 	jr	ra
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 75000401 	jalx	0*14001004 <.*>
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0d000409 	jal	0*14001024 <.*>
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 03e00009 	jalr	zero,ra
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
	\.\.\.
