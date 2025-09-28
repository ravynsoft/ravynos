#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS MIPS32 instructions
#as: -32
#source: mips32.s

# Check MIPS32 instruction assembly

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> 00400851 	clo	at,v0
0+0004 <[^>]*> 00801850 	clz	v1,a0
0+0008 <[^>]*> 01cf6898 	mul	t5,t6,t7
0+000c <[^>]*> 7e040035 	pref	0x4,0\(s0\)
0+0010 <[^>]*> 00000040 	ssnop
0+0014 <[^>]*> 7c250025 	cache	0x5,0\(at\)
0+0018 <[^>]*> 42000018 	eret
0+001c <[^>]*> 42000008 	tlbp
0+0020 <[^>]*> 42000001 	tlbr
0+0024 <[^>]*> 42000002 	tlbwi
0+0028 <[^>]*> 42000006 	tlbwr
0+002c <[^>]*> 42000020 	wait
0+0030 <[^>]*> 42000020 	wait
0+0034 <[^>]*> 4200d160 	wait	0x345
0+0038 <[^>]*> 0000000d 	break
0+003c <[^>]*> 0000000d 	break
0+0040 <[^>]*> 0345000d 	break	0x345
0+0044 <[^>]*> 0048d14d 	break	0x48,0x345
0+0048 <[^>]*> 0000000e 	sdbbp
0+004c <[^>]*> 0000000e 	sdbbp
0+0050 <[^>]*> 0000d14e 	sdbbp	0x345
	\.\.\.
