#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS MIPS32 cop2 instructions
#as: -32

# Check MIPS32 cop2 instruction assembly

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> 4900ffff 	bc2f	0+0000 <text_label>
0+0004 <[^>]*> 00000000 	nop
0+0008 <[^>]*> 4902fffd 	bc2fl	0+0000 <text_label>
0+000c <[^>]*> 00000000 	nop
0+0010 <[^>]*> 4901fffb 	bc2t	0+0000 <text_label>
0+0014 <[^>]*> 00000000 	nop
0+0018 <[^>]*> 4903fff9 	bc2tl	0+0000 <text_label>
0+001c <[^>]*> 00000000 	nop
0+0020 <[^>]*> 48411000 	cfc2	at,\$2
0+0024 <[^>]*> 4b234567 	c2	0x1234567
0+0028 <[^>]*> 48c21800 	ctc2	v0,\$3
0+002c <[^>]*> 48032000 	mfc2	v1,\$4
0+0030 <[^>]*> 48042800 	mfc2	a0,\$5
0+0034 <[^>]*> 48053007 	mfc2	a1,\$6,7
0+0038 <[^>]*> 48863800 	mtc2	a2,\$7
0+003c <[^>]*> 48874000 	mtc2	a3,\$8
0+0040 <[^>]*> 48884807 	mtc2	t0,\$9,7
0+0044 <[^>]*> 4900ffee 	bc2f	0+0000 <text_label>
0+0048 <[^>]*> 00000000 	nop
0+004c <[^>]*> 4906ffec 	bc2fl	\$cc1,0+0000 <text_label>
0+0050 <[^>]*> 00000000 	nop
0+0054 <[^>]*> 4919ffea 	bc2t	\$cc6,0+0000 <text_label>
0+0058 <[^>]*> 00000000 	nop
0+005c <[^>]*> 491fffe8 	bc2tl	\$cc7,0+0000 <text_label>
0+0060 <[^>]*> 00000000 	nop
#pass
