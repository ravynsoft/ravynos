#objdump: -dr --prefix-addresses --show-raw-insn -mmips:isa32r6
#name: MIPS branch local symbol relocation 3 (ignore branch ISA)
#as: -32 -mignore-branch-isa
#source: branch-local-3.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 001f 0f3c 	jr	ra
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> cbffffff 	bc	00001014 <bar\+0x4>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	foo
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> d85fffff 	beqzc	v0,0000101c <bar\+0xc>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	foo
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 03e00009 	jr	ra
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
	\.\.\.
