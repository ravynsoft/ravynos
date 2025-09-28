#objdump: -dr --prefix-addresses --show-raw-insn
#name: microMIPS link branch to absolute expression with addend
#source: ../../../gas/testsuite/gas/mips/micromips-branch-absolute-addend.s
#ld: -Ttext 0x12340000 -e foo

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 40e0 2c54 	bc	0*123468ac <bar\+0x1233>
[0-9a-f]+ <[^>]*> 4060 2c52 	bal	0*123468ac <bar\+0x1233>
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 4020 2c4e 	bltzal	zero,0*123468ac <bar\+0x1233>
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 40e2 2c4a 	beqzc	v0,0*123468ac <bar\+0x1233>
[0-9a-f]+ <[^>]*> 40a2 2c48 	bnezc	v0,0*123468ac <bar\+0x1233>
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
