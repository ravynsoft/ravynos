#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 registers

# Test the register names

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 00001ec4 	movi	zero,123
0+0004 <[^>]*> 00401ec4 	movi	at,123
0+0008 <[^>]*> 00801ec4 	movi	r2,123
0+000c <[^>]*> 00c01ec4 	movi	r3,123
0+0010 <[^>]*> 01001ec4 	movi	r4,123
0+0014 <[^>]*> 01401ec4 	movi	r5,123
0+0018 <[^>]*> 01801ec4 	movi	r6,123
0+001c <[^>]*> 01c01ec4 	movi	r7,123
0+0020 <[^>]*> 02001ec4 	movi	r8,123
0+0024 <[^>]*> 02401ec4 	movi	r9,123
0+0028 <[^>]*> 02801ec4 	movi	r10,123
0+002c <[^>]*> 02c01ec4 	movi	r11,123
0+0030 <[^>]*> 03001ec4 	movi	r12,123
0+0034 <[^>]*> 03401ec4 	movi	r13,123
0+0038 <[^>]*> 03801ec4 	movi	r14,123
0+003c <[^>]*> 03c01ec4 	movi	r15,123
0+0040 <[^>]*> 04001ec4 	movi	r16,123
0+0044 <[^>]*> 04401ec4 	movi	r17,123
0+0048 <[^>]*> 04801ec4 	movi	r18,123
0+004c <[^>]*> 04c01ec4 	movi	r19,123
0+0050 <[^>]*> 05001ec4 	movi	r20,123
0+0054 <[^>]*> 05401ec4 	movi	r21,123
0+0058 <[^>]*> 05801ec4 	movi	r22,123
0+005c <[^>]*> 05c01ec4 	movi	r23,123
0+0060 <[^>]*> 06001ec4 	movi	et,123
0+0064 <[^>]*> 06401ec4 	movi	bt,123
0+0068 <[^>]*> 06801ec4 	movi	gp,123
0+006c <[^>]*> 06c01ec4 	movi	sp,123
0+0070 <[^>]*> 07001ec4 	movi	fp,123
0+0074 <[^>]*> 07401ec4 	movi	ea,123
0+0078 <[^>]*> 07801ec4 	movi	sstatus,123
0+007c <[^>]*> 07c01ec4 	movi	ra,123
0+0080 <[^>]*> 00001ec4 	movi	zero,123
0+0084 <[^>]*> 00401ec4 	movi	at,123
0+0088 <[^>]*> 06001ec4 	movi	et,123
0+008c <[^>]*> 06401ec4 	movi	bt,123
0+0090 <[^>]*> 06801ec4 	movi	gp,123
0+0094 <[^>]*> 06c01ec4 	movi	sp,123
0+0098 <[^>]*> 07001ec4 	movi	fp,123
0+009c <[^>]*> 07401ec4 	movi	ea,123
0+00a0 <[^>]*> 07801ec4 	movi	sstatus,123
0+00a4 <[^>]*> 07801ec4 	movi	sstatus,123
0+00a8 <[^>]*> 07c01ec4 	movi	ra,123
