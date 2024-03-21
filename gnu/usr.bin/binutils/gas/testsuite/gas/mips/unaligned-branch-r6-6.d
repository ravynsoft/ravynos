#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPSr6 branch to unaligned symbol 6
#as: -n32 -mips64r6
#source: unaligned-branch-r6-4.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> c8000000 	bc	00001008 <foo\+0x8>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar0-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> d8400000 	beqzc	v0,00001010 <foo\+0x10>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar0-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 10400000 	beqz	v0,00001018 <foo\+0x18>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar0-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> c8000000 	bc	00001020 <foo\+0x20>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar1-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> d8400000 	beqzc	v0,00001028 <foo\+0x28>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar1-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 10400000 	beqz	v0,00001030 <foo\+0x30>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar1-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> c8000000 	bc	00001038 <foo\+0x38>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar2-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> d8400000 	beqzc	v0,00001040 <foo\+0x40>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar2-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 10400000 	beqz	v0,00001048 <foo\+0x48>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar2-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> c8000000 	bc	00001050 <foo\+0x50>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar3-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> d8400000 	beqzc	v0,00001058 <foo\+0x58>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar3-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 10400000 	beqz	v0,00001060 <foo\+0x60>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar3-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> c8000000 	bc	00001068 <foo\+0x68>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar4-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> d8400000 	beqzc	v0,00001070 <foo\+0x70>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar4-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 10400000 	beqz	v0,00001078 <foo\+0x78>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar4-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> c8000000 	bc	00001080 <foo\+0x80>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar4-0x3
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> d8400000 	beqzc	v0,00001088 <foo\+0x88>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar4-0x3
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 10400000 	beqz	v0,00001090 <foo\+0x90>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar4-0x3
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> c8000000 	bc	00001098 <foo\+0x98>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar4-0x2
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> d8400000 	beqzc	v0,000010a0 <foo\+0xa0>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar4-0x2
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 10400000 	beqz	v0,000010a8 <foo\+0xa8>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar4-0x2
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> c8000000 	bc	000010b0 <foo\+0xb0>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar4-0x1
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> d8400000 	beqzc	v0,000010b8 <foo\+0xb8>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar4-0x1
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 10400000 	beqz	v0,000010c0 <foo\+0xc0>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar4-0x1
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> c8000000 	bc	000010c8 <foo\+0xc8>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> d8400000 	beqzc	v0,000010d0 <foo\+0xd0>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 10400000 	beqz	v0,000010d8 <foo\+0xd8>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> c8000000 	bc	000010e0 <foo\+0xe0>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar16-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> d8400000 	beqzc	v0,000010e8 <foo\+0xe8>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar16-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 10400000 	beqz	v0,000010f0 <foo\+0xf0>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar16-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> c8000000 	bc	000010f8 <foo\+0xf8>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar17-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> d8400000 	beqzc	v0,00001100 <foo\+0x100>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar17-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 10400000 	beqz	v0,00001108 <foo\+0x108>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar17-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> c8000000 	bc	00001110 <foo\+0x110>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar18-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> d8400000 	beqzc	v0,00001118 <foo\+0x118>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar18-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 10400000 	beqz	v0,00001120 <foo\+0x120>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar18-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> c8000000 	bc	00001128 <foo\+0x128>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar18-0x3
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> d8400000 	beqzc	v0,00001130 <foo\+0x130>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar18-0x3
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 10400000 	beqz	v0,00001138 <foo\+0x138>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar18-0x3
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> c8000000 	bc	00001140 <foo\+0x140>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar18-0x2
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> d8400000 	beqzc	v0,00001148 <foo\+0x148>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar18-0x2
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 10400000 	beqz	v0,00001150 <foo\+0x150>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar18-0x2
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> c8000000 	bc	00001158 <foo\+0x158>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar18-0x1
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> d8400000 	beqzc	v0,00001160 <foo\+0x160>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar18-0x1
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 10400000 	beqz	v0,00001168 <foo\+0x168>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar18-0x1
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> c8000000 	bc	00001170 <foo\+0x170>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	bar18
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> d8400000 	beqzc	v0,00001178 <foo\+0x178>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	bar18
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 10400000 	beqz	v0,00001180 <foo\+0x180>
[ 	]*[0-9a-f]+: R_MIPS_PC16	bar18
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 03e00009 	jr	ra
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
