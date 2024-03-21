#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 registers
#as: -march=r2
#source: registers.s

# Test the register names

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 007b0004 	movi	zero,123
0+0004 <[^>]*> 007b0804 	movi	at,123
0+0008 <[^>]*> 007b1004 	movi	r2,123
0+000c <[^>]*> 007b1804 	movi	r3,123
0+0010 <[^>]*> 007b2004 	movi	r4,123
0+0014 <[^>]*> 007b2804 	movi	r5,123
0+0018 <[^>]*> 007b3004 	movi	r6,123
0+001c <[^>]*> 007b3804 	movi	r7,123
0+0020 <[^>]*> 007b4004 	movi	r8,123
0+0024 <[^>]*> 007b4804 	movi	r9,123
0+0028 <[^>]*> 007b5004 	movi	r10,123
0+002c <[^>]*> 007b5804 	movi	r11,123
0+0030 <[^>]*> 007b6004 	movi	r12,123
0+0034 <[^>]*> 007b6804 	movi	r13,123
0+0038 <[^>]*> 007b7004 	movi	r14,123
0+003c <[^>]*> 007b7804 	movi	r15,123
0+0040 <[^>]*> 007b8004 	movi	r16,123
0+0044 <[^>]*> 007b8804 	movi	r17,123
0+0048 <[^>]*> 007b9004 	movi	r18,123
0+004c <[^>]*> 007b9804 	movi	r19,123
0+0050 <[^>]*> 007ba004 	movi	r20,123
0+0054 <[^>]*> 007ba804 	movi	r21,123
0+0058 <[^>]*> 007bb004 	movi	r22,123
0+005c <[^>]*> 007bb804 	movi	r23,123
0+0060 <[^>]*> 007bc004 	movi	et,123
0+0064 <[^>]*> 007bc804 	movi	bt,123
0+0068 <[^>]*> 007bd004 	movi	gp,123
0+006c <[^>]*> 007bd804 	movi	sp,123
0+0070 <[^>]*> 007be004 	movi	fp,123
0+0074 <[^>]*> 007be804 	movi	ea,123
0+0078 <[^>]*> 007bf004 	movi	sstatus,123
0+007c <[^>]*> 007bf804 	movi	ra,123
0+0080 <[^>]*> 007b0004 	movi	zero,123
0+0084 <[^>]*> 007b0804 	movi	at,123
0+0088 <[^>]*> 007bc004 	movi	et,123
0+008c <[^>]*> 007bc804 	movi	bt,123
0+0090 <[^>]*> 007bd004 	movi	gp,123
0+0094 <[^>]*> 007bd804 	movi	sp,123
0+0098 <[^>]*> 007be004 	movi	fp,123
0+009c <[^>]*> 007be804 	movi	ea,123
0+00a0 <[^>]*> 007bf004 	movi	sstatus,123
0+00a4 <[^>]*> 007bf004 	movi	sstatus,123
0+00a8 <[^>]*> 007bf804 	movi	ra,123
