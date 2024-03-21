#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16e2 link PC-relative relocation with addend 2
#source: ../../../gas/testsuite/gas/mips/mips16-pcrel-addend-2.s
#as: -mips32r2 -mmips16e2
#ld: -Ttext 0x43210000 -e 0

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f328 6a21 	lui	v0,0x4321
[0-9a-f]+ <[^>]*> f264 4a04 	addiu	v0,8804
[0-9a-f]+ <[^>]*> f328 6a21 	lui	v0,0x4321
[0-9a-f]+ <[^>]*> f264 9a44 	lw	v0,8804\(v0\)
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
	\.\.\.
