# as: -march=armv6t2 -EL
# objdump: -dr --prefix-addresses --show-raw-insn
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince
#name: Thumb2 vldr with immediate constant

.*: +file format .*arm.*

Disassembly of section .text:
00000000 <thumb2_ldr> ed9f 0a0f 	vldr	s0, \[pc, #60\]	@ 00000040 <thumb2_ldr\+0x40>
00000004 <thumb2_ldr\+0x4> ed9f 7a0e 	vldr	s14, \[pc, #56\]	@ 00000040 <thumb2_ldr\+0x40>
00000008 <thumb2_ldr\+0x8> ed9f ea0d 	vldr	s28, \[pc, #52\]	@ 00000040 <thumb2_ldr\+0x40>
0000000c <thumb2_ldr\+0xc> eddf fa0c 	vldr	s31, \[pc, #48\]	@ 00000040 <thumb2_ldr\+0x40>
00000010 <thumb2_ldr\+0x10> ed9f 0a0c 	vldr	s0, \[pc, #48\]	@ 00000044 <thumb2_ldr\+0x44>
00000014 <thumb2_ldr\+0x14> ed9f 7a0b 	vldr	s14, \[pc, #44\]	@ 00000044 <thumb2_ldr\+0x44>
00000018 <thumb2_ldr\+0x18> ed9f ea0a 	vldr	s28, \[pc, #40\]	@ 00000044 <thumb2_ldr\+0x44>
0000001c <thumb2_ldr\+0x1c> eddf fa09 	vldr	s31, \[pc, #36\]	@ 00000044 <thumb2_ldr\+0x44>
00000020 <thumb2_ldr\+0x20> ed9f 0a09 	vldr	s0, \[pc, #36\]	@ 00000048 <thumb2_ldr\+0x48>
00000024 <thumb2_ldr\+0x24> ed9f 7a08 	vldr	s14, \[pc, #32\]	@ 00000048 <thumb2_ldr\+0x48>
00000028 <thumb2_ldr\+0x28> ed9f ea07 	vldr	s28, \[pc, #28\]	@ 00000048 <thumb2_ldr\+0x48>
0000002c <thumb2_ldr\+0x2c> eddf fa06 	vldr	s31, \[pc, #24\]	@ 00000048 <thumb2_ldr\+0x48>
00000030 <thumb2_ldr\+0x30> ed9f 0a06 	vldr	s0, \[pc, #24\]	@ 0000004c <thumb2_ldr\+0x4c>
00000034 <thumb2_ldr\+0x34> ed9f 7a05 	vldr	s14, \[pc, #20\]	@ 0000004c <thumb2_ldr\+0x4c>
00000038 <thumb2_ldr\+0x38> ed9f ea04 	vldr	s28, \[pc, #16\]	@ 0000004c <thumb2_ldr\+0x4c>
0000003c <thumb2_ldr\+0x3c> eddf fa03 	vldr	s31, \[pc, #12\]	@ 0000004c <thumb2_ldr\+0x4c>
00000040 <thumb2_ldr\+0x40> 00000000 	.word	0x00000000
00000044 <thumb2_ldr\+0x44> ff000000 	.word	0xff000000
00000048 <thumb2_ldr\+0x48> ffffffff 	.word	0xffffffff
0000004c <thumb2_ldr\+0x4c> 0fff0000 	.word	0x0fff0000
00000050 <thumb2_ldr\+0x50> ed9f 0a0f 	vldr	s0, \[pc, #60\]	@ 00000090 <thumb2_ldr\+0x90>
00000054 <thumb2_ldr\+0x54> ed9f 7a0e 	vldr	s14, \[pc, #56\]	@ 00000090 <thumb2_ldr\+0x90>
00000058 <thumb2_ldr\+0x58> ed9f ea0d 	vldr	s28, \[pc, #52\]	@ 00000090 <thumb2_ldr\+0x90>
0000005c <thumb2_ldr\+0x5c> eddf fa0c 	vldr	s31, \[pc, #48\]	@ 00000090 <thumb2_ldr\+0x90>
00000060 <thumb2_ldr\+0x60> ed9f 0a0c 	vldr	s0, \[pc, #48\]	@ 00000094 <thumb2_ldr\+0x94>
00000064 <thumb2_ldr\+0x64> ed9f 7a0b 	vldr	s14, \[pc, #44\]	@ 00000094 <thumb2_ldr\+0x94>
00000068 <thumb2_ldr\+0x68> ed9f ea0a 	vldr	s28, \[pc, #40\]	@ 00000094 <thumb2_ldr\+0x94>
0000006c <thumb2_ldr\+0x6c> eddf fa09 	vldr	s31, \[pc, #36\]	@ 00000094 <thumb2_ldr\+0x94>
00000070 <thumb2_ldr\+0x70> ed9f 0a09 	vldr	s0, \[pc, #36\]	@ 00000098 <thumb2_ldr\+0x98>
00000074 <thumb2_ldr\+0x74> ed9f 7a08 	vldr	s14, \[pc, #32\]	@ 00000098 <thumb2_ldr\+0x98>
00000078 <thumb2_ldr\+0x78> ed9f ea07 	vldr	s28, \[pc, #28\]	@ 00000098 <thumb2_ldr\+0x98>
0000007c <thumb2_ldr\+0x7c> eddf fa06 	vldr	s31, \[pc, #24\]	@ 00000098 <thumb2_ldr\+0x98>
00000080 <thumb2_ldr\+0x80> ed9f 0a06 	vldr	s0, \[pc, #24\]	@ 0000009c <thumb2_ldr\+0x9c>
00000084 <thumb2_ldr\+0x84> ed9f 7a05 	vldr	s14, \[pc, #20\]	@ 0000009c <thumb2_ldr\+0x9c>
00000088 <thumb2_ldr\+0x88> ed9f ea04 	vldr	s28, \[pc, #16\]	@ 0000009c <thumb2_ldr\+0x9c>
0000008c <thumb2_ldr\+0x8c> eddf fa03 	vldr	s31, \[pc, #12\]	@ 0000009c <thumb2_ldr\+0x9c>
00000090 <thumb2_ldr\+0x90> 00000000 	.word	0x00000000
00000094 <thumb2_ldr\+0x94> 00ff0000 	.word	0x00ff0000
00000098 <thumb2_ldr\+0x98> ff00ffff 	.word	0xff00ffff
0000009c <thumb2_ldr\+0x9c> 00fff000 	.word	0x00fff000
000000a0 <thumb2_ldr\+0xa0> ef80 0e30 	vmov.i64	d0, #0x0000000000000000
000000a4 <thumb2_ldr\+0xa4> ef80 ee30 	vmov.i64	d14, #0x0000000000000000
000000a8 <thumb2_ldr\+0xa8> efc0 ce30 	vmov.i64	d28, #0x0000000000000000
000000ac <thumb2_ldr\+0xac> efc0 fe30 	vmov.i64	d31, #0x0000000000000000
000000b0 <thumb2_ldr\+0xb0> ed9f 0b0b 	vldr	d0, \[pc, #44\]	@ 000000e0 <thumb2_ldr\+0xe0>
000000b4 <thumb2_ldr\+0xb4> ed9f eb0a 	vldr	d14, \[pc, #40\]	@ 000000e0 <thumb2_ldr\+0xe0>
000000b8 <thumb2_ldr\+0xb8> eddf cb09 	vldr	d28, \[pc, #36\]	@ 000000e0 <thumb2_ldr\+0xe0>
000000bc <thumb2_ldr\+0xbc> eddf fb08 	vldr	d31, \[pc, #32\]	@ 000000e0 <thumb2_ldr\+0xe0>
000000c0 <thumb2_ldr\+0xc0> ff87 0e3f 	vmov.i64	d0, #0xffffffffffffffff
000000c4 <thumb2_ldr\+0xc4> ff87 ee3f 	vmov.i64	d14, #0xffffffffffffffff
000000c8 <thumb2_ldr\+0xc8> ffc7 ce3f 	vmov.i64	d28, #0xffffffffffffffff
000000cc <thumb2_ldr\+0xcc> ffc7 fe3f 	vmov.i64	d31, #0xffffffffffffffff
000000d0 <thumb2_ldr\+0xd0> ed9f 0b05 	vldr	d0, \[pc, #20\]	@ 000000e8 <thumb2_ldr\+0xe8>
000000d4 <thumb2_ldr\+0xd4> ed9f eb04 	vldr	d14, \[pc, #16\]	@ 000000e8 <thumb2_ldr\+0xe8>
000000d8 <thumb2_ldr\+0xd8> eddf cb03 	vldr	d28, \[pc, #12\]	@ 000000e8 <thumb2_ldr\+0xe8>
000000dc <thumb2_ldr\+0xdc> eddf fb02 	vldr	d31, \[pc, #8\]	@ 000000e8 <thumb2_ldr\+0xe8>
000000e0 <thumb2_ldr\+0xe0> ca000000 	.word	0xca000000
000000e4 <thumb2_ldr\+0xe4> 00000000 	.word	0x00000000
000000e8 <thumb2_ldr\+0xe8> 0fff0000 	.word	0x0fff0000
000000ec <thumb2_ldr\+0xec> 00000000 	.word	0x00000000
000000f0 <thumb2_ldr\+0xf0> ef80 0e30 	vmov.i64	d0, #0x0000000000000000
000000f4 <thumb2_ldr\+0xf4> ef80 ee30 	vmov.i64	d14, #0x0000000000000000
000000f8 <thumb2_ldr\+0xf8> efc0 ce30 	vmov.i64	d28, #0x0000000000000000
000000fc <thumb2_ldr\+0xfc> efc0 fe30 	vmov.i64	d31, #0x0000000000000000
00000100 <thumb2_ldr\+0x100> ef80 0e34 	vmov.i64	d0, #0x0000000000ff0000
00000104 <thumb2_ldr\+0x104> ef80 ee34 	vmov.i64	d14, #0x0000000000ff0000
00000108 <thumb2_ldr\+0x108> efc0 ce34 	vmov.i64	d28, #0x0000000000ff0000
0000010c <thumb2_ldr\+0x10c> efc0 fe34 	vmov.i64	d31, #0x0000000000ff0000
00000110 <thumb2_ldr\+0x110> ef80 0e39 	vmov.i64	d0, #0x00000000ff0000ff
00000114 <thumb2_ldr\+0x114> ef80 ee39 	vmov.i64	d14, #0x00000000ff0000ff
00000118 <thumb2_ldr\+0x118> efc0 ce39 	vmov.i64	d28, #0x00000000ff0000ff
0000011c <thumb2_ldr\+0x11c> efc0 fe39 	vmov.i64	d31, #0x00000000ff0000ff
00000120 <thumb2_ldr\+0x120> ed9f 0b03 	vldr	d0, \[pc, #12\]	@ 00000130 <thumb2_ldr\+0x130>
00000124 <thumb2_ldr\+0x124> ed9f eb02 	vldr	d14, \[pc, #8\]	@ 00000130 <thumb2_ldr\+0x130>
00000128 <thumb2_ldr\+0x128> eddf cb01 	vldr	d28, \[pc, #4\]	@ 00000130 <thumb2_ldr\+0x130>
0000012c <thumb2_ldr\+0x12c> eddf fb00 	vldr	d31, \[pc\]	@ 00000130 <thumb2_ldr\+0x130>
00000130 <thumb2_ldr\+0x130> 00fff000 	.word	0x00fff000
00000134 <thumb2_ldr\+0x134> 00000000 	.word	0x00000000
00000138 <thumb2_ldr\+0x138> ef80 0e30 	vmov.i64	d0, #0x0000000000000000
0000013c <thumb2_ldr\+0x13c> ef80 ee30 	vmov.i64	d14, #0x0000000000000000
00000140 <thumb2_ldr\+0x140> efc0 ce30 	vmov.i64	d28, #0x0000000000000000
00000144 <thumb2_ldr\+0x144> efc0 fe30 	vmov.i64	d31, #0x0000000000000000
00000148 <thumb2_ldr\+0x148> ff80 0e30 	vmov.i64	d0, #0xff00000000000000
0000014c <thumb2_ldr\+0x14c> ff80 ee30 	vmov.i64	d14, #0xff00000000000000
00000150 <thumb2_ldr\+0x150> ffc0 ce30 	vmov.i64	d28, #0xff00000000000000
00000154 <thumb2_ldr\+0x154> ffc0 fe30 	vmov.i64	d31, #0xff00000000000000
00000158 <thumb2_ldr\+0x158> ff87 0e3f 	vmov.i64	d0, #0xffffffffffffffff
0000015c <thumb2_ldr\+0x15c> ff87 ee3f 	vmov.i64	d14, #0xffffffffffffffff
00000160 <thumb2_ldr\+0x160> ffc7 ce3f 	vmov.i64	d28, #0xffffffffffffffff
00000164 <thumb2_ldr\+0x164> ffc7 fe3f 	vmov.i64	d31, #0xffffffffffffffff
00000168 <thumb2_ldr\+0x168> ed9f 0b03 	vldr	d0, \[pc, #12\]	@ 00000178 <thumb2_ldr\+0x178>
0000016c <thumb2_ldr\+0x16c> ed9f eb02 	vldr	d14, \[pc, #8\]	@ 00000178 <thumb2_ldr\+0x178>
00000170 <thumb2_ldr\+0x170> eddf cb01 	vldr	d28, \[pc, #4\]	@ 00000178 <thumb2_ldr\+0x178>
00000174 <thumb2_ldr\+0x174> eddf fb00 	vldr	d31, \[pc\]	@ 00000178 <thumb2_ldr\+0x178>
00000178 <thumb2_ldr\+0x178> 00000000 	.word	0x00000000
0000017c <thumb2_ldr\+0x17c> 0fff0000 	.word	0x0fff0000
00000180 <thumb2_ldr\+0x180> ef80 0e30 	vmov.i64	d0, #0x0000000000000000
00000184 <thumb2_ldr\+0x184> ef80 ee30 	vmov.i64	d14, #0x0000000000000000
00000188 <thumb2_ldr\+0x188> efc0 ce30 	vmov.i64	d28, #0x0000000000000000
0000018c <thumb2_ldr\+0x18c> efc0 fe30 	vmov.i64	d31, #0x0000000000000000
00000190 <thumb2_ldr\+0x190> ed9f 0b0b 	vldr	d0, \[pc, #44\]	@ 000001c0 <thumb2_ldr\+0x1c0>
00000194 <thumb2_ldr\+0x194> ed9f eb0a 	vldr	d14, \[pc, #40\]	@ 000001c0 <thumb2_ldr\+0x1c0>
00000198 <thumb2_ldr\+0x198> eddf cb09 	vldr	d28, \[pc, #36\]	@ 000001c0 <thumb2_ldr\+0x1c0>
0000019c <thumb2_ldr\+0x19c> eddf fb08 	vldr	d31, \[pc, #32\]	@ 000001c0 <thumb2_ldr\+0x1c0>
000001a0 <thumb2_ldr\+0x1a0> ed9f 0b09 	vldr	d0, \[pc, #36\]	@ 000001c8 <thumb2_ldr\+0x1c8>
000001a4 <thumb2_ldr\+0x1a4> ed9f eb08 	vldr	d14, \[pc, #32\]	@ 000001c8 <thumb2_ldr\+0x1c8>
000001a8 <thumb2_ldr\+0x1a8> eddf cb07 	vldr	d28, \[pc, #28\]	@ 000001c8 <thumb2_ldr\+0x1c8>
000001ac <thumb2_ldr\+0x1ac> eddf fb06 	vldr	d31, \[pc, #24\]	@ 000001c8 <thumb2_ldr\+0x1c8>
000001b0 <thumb2_ldr\+0x1b0> ed9f 0b05 	vldr	d0, \[pc, #20\]	@ 000001c8 <thumb2_ldr\+0x1c8>
000001b4 <thumb2_ldr\+0x1b4> ed9f eb04 	vldr	d14, \[pc, #16\]	@ 000001c8 <thumb2_ldr\+0x1c8>
000001b8 <thumb2_ldr\+0x1b8> eddf cb03 	vldr	d28, \[pc, #12\]	@ 000001c8 <thumb2_ldr\+0x1c8>
000001bc <thumb2_ldr\+0x1bc> eddf fb02 	vldr	d31, \[pc, #8\]	@ 000001c8 <thumb2_ldr\+0x1c8>
000001c0 <thumb2_ldr\+0x1c0> 00000000 	.word	0x00000000
000001c4 <thumb2_ldr\+0x1c4> 000ff000 	.word	0x000ff000
000001c8 <thumb2_ldr\+0x1c8> f0000000 	.word	0xf0000000
000001cc <thumb2_ldr\+0x1cc> 0ff00fff 	.word	0x0ff00fff
000001d0 <thumb2_ldr\+0x1d0> ed9f 1b01 	vldr	d1, \[pc, #4\]	@ 000001d8 <thumb2_ldr\+0x1d8>
	\.\.\.
000001dc <thumb2_ldr\+0x1dc> 0000fff0 	.word	0x0000fff0
000001e0 <thumb2_ldr\+0x1e0> f101 0000 	add.w	r0, r1, #0
000001e4 <thumb2_ldr\+0x1e4> ed9f 1b00 	vldr	d1, \[pc\]	@ 000001e8 <thumb2_ldr\+0x1e8>
000001e8 <thumb2_ldr\+0x1e8> 00000000 	.word	0x00000000
000001ec <thumb2_ldr\+0x1ec> 0000fff0 	.word	0x0000fff0
000001f0 <thumb2_ldr\+0x1f0> ed9f 1b11 	vldr	d1, \[pc, #68\]	@ 00000238 <thumb2_ldr\+0x238>
000001f4 <thumb2_ldr\+0x1f4> ed9f 1a12 	vldr	s2, \[pc, #72\]	@ 00000240 <thumb2_ldr\+0x240>
000001f8 <thumb2_ldr\+0x1f8> ed9f 3b13 	vldr	d3, \[pc, #76\]	@ 00000248 <thumb2_ldr\+0x248>
000001fc <thumb2_ldr\+0x1fc> ed9f 2a11 	vldr	s4, \[pc, #68\]	@ 00000244 <thumb2_ldr\+0x244>
00000200 <thumb2_ldr\+0x200> ed9f 5b11 	vldr	d5, \[pc, #68\]	@ 00000248 <thumb2_ldr\+0x248>
00000204 <thumb2_ldr\+0x204> ed9f 6b12 	vldr	d6, \[pc, #72\]	@ 00000250 <thumb2_ldr\+0x250>
00000208 <thumb2_ldr\+0x208> ed9f 7b13 	vldr	d7, \[pc, #76\]	@ 00000258 <thumb2_ldr\+0x258>
0000020c <thumb2_ldr\+0x20c> ed9f 4a14 	vldr	s8, \[pc, #80\]	@ 00000260 <thumb2_ldr\+0x260>
00000210 <thumb2_ldr\+0x210> ed9f 9b15 	vldr	d9, \[pc, #84\]	@ 00000268 <thumb2_ldr\+0x268>
00000214 <thumb2_ldr\+0x214> ed9f 5a13 	vldr	s10, \[pc, #76\]	@ 00000264 <thumb2_ldr\+0x264>
00000218 <thumb2_ldr\+0x218> ed9f bb15 	vldr	d11, \[pc, #84\]	@ 00000270 <thumb2_ldr\+0x270>
0000021c <thumb2_ldr\+0x21c> ed9f 6a16 	vldr	s12, \[pc, #88\]	@ 00000278 <thumb2_ldr\+0x278>
00000220 <thumb2_ldr\+0x220> eddf 6a16 	vldr	s13, \[pc, #88\]	@ 0000027c <thumb2_ldr\+0x27c>
00000224 <thumb2_ldr\+0x224> ed9f 7a07 	vldr	s14, \[pc, #28\]	@ 00000244 <thumb2_ldr\+0x244>
00000228 <thumb2_ldr\+0x228> eddf 7a04 	vldr	s15, \[pc, #16\]	@ 0000023c <thumb2_ldr\+0x23c>
0000022c <thumb2_ldr\+0x22c> eddf 0b12 	vldr	d16, \[pc, #72\]	@ 00000278 <thumb2_ldr\+0x278>
00000230 <thumb2_ldr\+0x230> eddf 1b13 	vldr	d17, \[pc, #76\]	@ 00000280 <thumb2_ldr\+0x280>
	\.\.\.
0000023c <thumb2_ldr\+0x23c> 0000fff0 	.word	0x0000fff0
00000240 <thumb2_ldr\+0x240> ff000000 	.word	0xff000000
00000244 <thumb2_ldr\+0x244> ff000001 	.word	0xff000001
00000248 <thumb2_ldr\+0x248> 00000001 	.word	0x00000001
0000024c <thumb2_ldr\+0x24c> 0000fff0 	.word	0x0000fff0
00000250 <thumb2_ldr\+0x250> 00000002 	.word	0x00000002
00000254 <thumb2_ldr\+0x254> 0000fff0 	.word	0x0000fff0
00000258 <thumb2_ldr\+0x258> 00000003 	.word	0x00000003
0000025c <thumb2_ldr\+0x25c> 0000fff0 	.word	0x0000fff0
00000260 <thumb2_ldr\+0x260> ff000002 	.word	0xff000002
00000264 <thumb2_ldr\+0x264> ff000003 	.word	0xff000003
00000268 <thumb2_ldr\+0x268> 00000004 	.word	0x00000004
0000026c <thumb2_ldr\+0x26c> 0000fff0 	.word	0x0000fff0
00000270 <thumb2_ldr\+0x270> 00000005 	.word	0x00000005
00000274 <thumb2_ldr\+0x274> 0000fff0 	.word	0x0000fff0
00000278 <thumb2_ldr\+0x278> ff000004 	.word	0xff000004
0000027c <thumb2_ldr\+0x27c> ff000005 	.word	0xff000005
00000280 <thumb2_ldr\+0x280> 0000fff0 	.word	0x0000fff0
00000284 <thumb2_ldr\+0x284> ff000004 	.word	0xff000004
