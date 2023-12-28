#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS MIPS32 cop2 instructions
#source: mips32-cp2.s
#as: -32

# Check MIPS32 cop2 instruction assembly

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> 48411000 	cfc2	at,\$2
0+0004 <[^>]*> 4b234567 	c2	0x1234567
0+0008 <[^>]*> 48c21800 	ctc2	v0,\$3
0+000c <[^>]*> 48032000 	mfc2	v1,\$4
0+0010 <[^>]*> 48042800 	mfc2	a0,\$5
0+0014 <[^>]*> 48053007 	mfc2	a1,\$6,7
0+0018 <[^>]*> 48863800 	mtc2	a2,\$7
0+001c <[^>]*> 48874000 	mtc2	a3,\$8
0+0020 <[^>]*> 48884807 	mtc2	t0,\$9,7
#pass
