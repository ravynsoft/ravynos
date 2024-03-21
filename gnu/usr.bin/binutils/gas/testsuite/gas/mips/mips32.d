#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS MIPS32 instructions
#as: -32

# Check MIPS32 instruction assembly

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> 70410821 	clo	at,v0
0+0004 <[^>]*> 70831820 	clz	v1,a0
0+0008 <[^>]*> 70a60000 	madd	a1,a2
0+000c <[^>]*> 70e80001 	maddu	a3,t0
0+0010 <[^>]*> 712a0004 	msub	t1,t2
0+0014 <[^>]*> 716c0005 	msubu	t3,t4
0+0018 <[^>]*> 71cf6802 	mul	t5,t6,t7
0+001c <[^>]*> ce040000 	pref	0x4,0\(s0\)
0+0020 <[^>]*> ce2407ff 	pref	0x4,2047\(s1\)
0+0024 <[^>]*> ce44f800 	pref	0x4,-2048\(s2\)
0+0028 <[^>]*> 00000040 	ssnop
0+002c <[^>]*> bc250000 	cache	0x5,0\(at\)
0+0030 <[^>]*> bc4507ff 	cache	0x5,2047\(v0\)
0+0034 <[^>]*> bc65f800 	cache	0x5,-2048\(v1\)
0+0038 <[^>]*> 3c010001 	lui	at,0x1
0+003c <[^>]*> 00240821 	addu	at,at,a0
0+0040 <[^>]*> bc258000 	cache	0x5,-32768\(at\)
0+0044 <[^>]*> 3c01ffff 	lui	at,0xffff
0+0048 <[^>]*> 00250821 	addu	at,at,a1
0+004c <[^>]*> bc257fff 	cache	0x5,32767\(at\)
0+0050 <[^>]*> 3c010001 	lui	at,0x1
0+0054 <[^>]*> bc258000 	cache	0x5,-32768\(at\)
0+0058 <[^>]*> 3c01ffff 	lui	at,0xffff
0+005c <[^>]*> bc257fff 	cache	0x5,32767\(at\)
0+0060 <[^>]*> 42000018 	eret
0+0064 <[^>]*> 42000008 	tlbp
0+0068 <[^>]*> 42000001 	tlbr
0+006c <[^>]*> 42000002 	tlbwi
0+0070 <[^>]*> 42000006 	tlbwr
0+0074 <[^>]*> 42000020 	wait
0+0078 <[^>]*> 42000020 	wait
0+007c <[^>]*> 4200d160 	wait	0x345
0+0080 <[^>]*> 0000000d 	break
0+0084 <[^>]*> 0000000d 	break
0+0088 <[^>]*> 0345000d 	break	0x345
0+008c <[^>]*> 0048d14d 	break	0x48,0x345
0+0090 <[^>]*> 7000003f 	sdbbp
0+0094 <[^>]*> 7000003f 	sdbbp
0+0098 <[^>]*> 7000d17f 	sdbbp	0x345
	\.\.\.
