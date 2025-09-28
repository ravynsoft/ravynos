#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch local symbol relocation 2 (ignore branch ISA)
#as: -32 -mignore-branch-isa
#source: branch-local-2.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 001f 0f3c 	jr	ra
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 1000ffff 	b	00001014 <bar\+0x4>
[ 	]*[0-9a-f]+: R_MIPS_PC16	foo
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 1443ffff 	bne	v0,v1,0000101c <bar\+0xc>
[ 	]*[0-9a-f]+: R_MIPS_PC16	foo
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0451ffff 	bgezal	v0,00001024 <bar\+0x14>
[ 	]*[0-9a-f]+: R_MIPS_PC16	foo
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0450ffff 	bltzal	v0,0000102c <bar\+0x1c>
[ 	]*[0-9a-f]+: R_MIPS_PC16	foo
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 03e00009 	jalr	zero,ra
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
	\.\.\.
