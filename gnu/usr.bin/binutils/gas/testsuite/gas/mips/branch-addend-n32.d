#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS BAL addend encoding (n32)
#as: -n32 -march=from-abi
#source: branch-addend.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 001f 0f3c 	jr	ra
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 04110000 	bal	00001018 <bar\+0x8>
[ 	]*[0-9a-f]+: R_MIPS_PC16	foo\+0x1fffc
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 04110000 	bal	00001020 <bar\+0x10>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar\+0x1fffc
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 03e00009 	jalr	zero,ra
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
	\.\.\.
