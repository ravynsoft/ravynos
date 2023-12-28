#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS link branch to absolute expression with addend
#source: ../../../gas/testsuite/gas/mips/branch-absolute-addend.s
#ld: -Ttext 0x12340000 -e foo

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 1000162a 	b	0*123468ac <bar\+0x1234>
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 04111628 	bal	0*123468ac <bar\+0x1234>
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 04101626 	bltzal	zero,0*123468ac <bar\+0x1234>
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 10401624 	beqz	v0,0*123468ac <bar\+0x1234>
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 14401622 	bnez	v0,0*123468ac <bar\+0x1234>
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
