#objdump: -dr --prefix-addresses --show-raw-insn
#name: PRU xfr

# Test the XFR class of instruction

.*: +file format elf32-pru

Disassembly of section .text:
0+0000 <[^>]*> 2eff8002 	zero	sp.b0, 1
0+0004 <[^>]*> 2eff81d7 	zero	r23.b2, 4
0+0008 <[^>]*> 2effbd80 	zero	r0.b0, 124
0+000c <[^>]*> 2eff0002 	fill	sp.b0, 1
0+0010 <[^>]*> 2eff01b7 	fill	r23.b1, 4
0+0014 <[^>]*> 2eff3d80 	fill	r0.b0, 124
0+0018 <[^>]*> 2e80000a 	xin	0, r10.b0, 1
0+001c <[^>]*> 2e803daa 	xin	0, r10.b1, 124
0+0020 <[^>]*> 2efe806a 	xin	253, r10.b3, 1
0+0024 <[^>]*> 2efebdca 	xin	253, r10.b2, 124
0+0028 <[^>]*> 2eaaaa0c 	xin	85, r12.b0, 85
0+002c <[^>]*> 2f00000a 	xout	0, r10.b0, 1
0+0030 <[^>]*> 2f003daa 	xout	0, r10.b1, 124
0+0034 <[^>]*> 2f7e806a 	xout	253, r10.b3, 1
0+0038 <[^>]*> 2f7ebdca 	xout	253, r10.b2, 124
0+003c <[^>]*> 2f2aaa0c 	xout	85, r12.b0, 85
0+0040 <[^>]*> 2f80000a 	xchg	0, r10.b0, 1
0+0044 <[^>]*> 2f803daa 	xchg	0, r10.b1, 124
0+0048 <[^>]*> 2ffe806a 	xchg	253, r10.b3, 1
0+004c <[^>]*> 2ffebdca 	xchg	253, r10.b2, 124
0+0050 <[^>]*> 2faaaa0c 	xchg	85, r12.b0, 85
0+0054 <[^>]*> 2e80400a 	sxin	0, r10.b0, 1
0+0058 <[^>]*> 2e807daa 	sxin	0, r10.b1, 124
0+005c <[^>]*> 2efec06a 	sxin	253, r10.b3, 1
0+0060 <[^>]*> 2efefdca 	sxin	253, r10.b2, 124
0+0064 <[^>]*> 2eaaea0c 	sxin	85, r12.b0, 85
0+0068 <[^>]*> 2f00400a 	sxout	0, r10.b0, 1
0+006c <[^>]*> 2f007daa 	sxout	0, r10.b1, 124
0+0070 <[^>]*> 2f7ec06a 	sxout	253, r10.b3, 1
0+0074 <[^>]*> 2f7efdca 	sxout	253, r10.b2, 124
0+0078 <[^>]*> 2f2aea0c 	sxout	85, r12.b0, 85
0+007c <[^>]*> 2f80400a 	sxchg	0, r10.b0, 1
0+0080 <[^>]*> 2f807daa 	sxchg	0, r10.b1, 124
0+0084 <[^>]*> 2ffec06a 	sxchg	253, r10.b3, 1
0+0088 <[^>]*> 2ffefdca 	sxchg	253, r10.b2, 124
0+008c <[^>]*> 2faaea0c 	sxchg	85, r12.b0, 85
