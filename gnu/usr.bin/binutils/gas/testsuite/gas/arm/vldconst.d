#objdump: -dr --prefix-addresses --show-raw-insn
#name: ARM vldr with immediate constant
#skip: *-*-pe *-*-wince
#as: -mcpu=arm7m -EL

.*: +file format .*arm.*

Disassembly of section .text:
00000000 <foo> ed9f0a0e 	vldr	s0, \[pc, #56\]	@ 00000040 <foo\+0x40>
00000004 <foo\+0x4> ed9f7a0d 	vldr	s14, \[pc, #52\]	@ 00000040 <foo\+0x40>
00000008 <foo\+0x8> ed9fea0c 	vldr	s28, \[pc, #48\]	@ 00000040 <foo\+0x40>
0000000c <foo\+0xc> eddffa0b 	vldr	s31, \[pc, #44\]	@ 00000040 <foo\+0x40>
00000010 <foo\+0x10> ed9f0a0b 	vldr	s0, \[pc, #44\]	@ 00000044 <foo\+0x44>
00000014 <foo\+0x14> ed9f7a0a 	vldr	s14, \[pc, #40\]	@ 00000044 <foo\+0x44>
00000018 <foo\+0x18> ed9fea09 	vldr	s28, \[pc, #36\]	@ 00000044 <foo\+0x44>
0000001c <foo\+0x1c> eddffa08 	vldr	s31, \[pc, #32\]	@ 00000044 <foo\+0x44>
00000020 <foo\+0x20> ed9f0a08 	vldr	s0, \[pc, #32\]	@ 00000048 <foo\+0x48>
00000024 <foo\+0x24> ed9f7a07 	vldr	s14, \[pc, #28\]	@ 00000048 <foo\+0x48>
00000028 <foo\+0x28> ed9fea06 	vldr	s28, \[pc, #24\]	@ 00000048 <foo\+0x48>
0000002c <foo\+0x2c> eddffa05 	vldr	s31, \[pc, #20\]	@ 00000048 <foo\+0x48>
00000030 <foo\+0x30> ed9f0a05 	vldr	s0, \[pc, #20\]	@ 0000004c <foo\+0x4c>
00000034 <foo\+0x34> ed9f7a04 	vldr	s14, \[pc, #16\]	@ 0000004c <foo\+0x4c>
00000038 <foo\+0x38> ed9fea03 	vldr	s28, \[pc, #12\]	@ 0000004c <foo\+0x4c>
0000003c <foo\+0x3c> eddffa02 	vldr	s31, \[pc, #8\]	@ 0000004c <foo\+0x4c>
00000040 <foo\+0x40> 00000000 	.word	0x00000000
00000044 <foo\+0x44> ff000000 	.word	0xff000000
00000048 <foo\+0x48> ffffffff 	.word	0xffffffff
0000004c <foo\+0x4c> 0fff0000 	.word	0x0fff0000
00000050 <foo\+0x50> ed9f0a0e 	vldr	s0, \[pc, #56\]	@ 00000090 <foo\+0x90>
00000054 <foo\+0x54> ed9f7a0d 	vldr	s14, \[pc, #52\]	@ 00000090 <foo\+0x90>
00000058 <foo\+0x58> ed9fea0c 	vldr	s28, \[pc, #48\]	@ 00000090 <foo\+0x90>
0000005c <foo\+0x5c> eddffa0b 	vldr	s31, \[pc, #44\]	@ 00000090 <foo\+0x90>
00000060 <foo\+0x60> ed9f0a0b 	vldr	s0, \[pc, #44\]	@ 00000094 <foo\+0x94>
00000064 <foo\+0x64> ed9f7a0a 	vldr	s14, \[pc, #40\]	@ 00000094 <foo\+0x94>
00000068 <foo\+0x68> ed9fea09 	vldr	s28, \[pc, #36\]	@ 00000094 <foo\+0x94>
0000006c <foo\+0x6c> eddffa08 	vldr	s31, \[pc, #32\]	@ 00000094 <foo\+0x94>
00000070 <foo\+0x70> ed9f0a08 	vldr	s0, \[pc, #32\]	@ 00000098 <foo\+0x98>
00000074 <foo\+0x74> ed9f7a07 	vldr	s14, \[pc, #28\]	@ 00000098 <foo\+0x98>
00000078 <foo\+0x78> ed9fea06 	vldr	s28, \[pc, #24\]	@ 00000098 <foo\+0x98>
0000007c <foo\+0x7c> eddffa05 	vldr	s31, \[pc, #20\]	@ 00000098 <foo\+0x98>
00000080 <foo\+0x80> ed9f0a05 	vldr	s0, \[pc, #20\]	@ 0000009c <foo\+0x9c>
00000084 <foo\+0x84> ed9f7a04 	vldr	s14, \[pc, #16\]	@ 0000009c <foo\+0x9c>
00000088 <foo\+0x88> ed9fea03 	vldr	s28, \[pc, #12\]	@ 0000009c <foo\+0x9c>
0000008c <foo\+0x8c> eddffa02 	vldr	s31, \[pc, #8\]	@ 0000009c <foo\+0x9c>
00000090 <foo\+0x90> 00000000 	.word	0x00000000
00000094 <foo\+0x94> 00ff0000 	.word	0x00ff0000
00000098 <foo\+0x98> ff00ffff 	.word	0xff00ffff
0000009c <foo\+0x9c> 00fff000 	.word	0x00fff000
000000a0 <foo\+0xa0> 0d9f0a0e 	vldreq	s0, \[pc, #56\]	@ 000000e0 <foo\+0xe0>
000000a4 <foo\+0xa4> 0d9f7a0d 	vldreq	s14, \[pc, #52\]	@ 000000e0 <foo\+0xe0>
000000a8 <foo\+0xa8> 0d9fea0c 	vldreq	s28, \[pc, #48\]	@ 000000e0 <foo\+0xe0>
000000ac <foo\+0xac> 0ddffa0b 	vldreq	s31, \[pc, #44\]	@ 000000e0 <foo\+0xe0>
000000b0 <foo\+0xb0> 0d9f0a0b 	vldreq	s0, \[pc, #44\]	@ 000000e4 <foo\+0xe4>
000000b4 <foo\+0xb4> 0d9f7a0a 	vldreq	s14, \[pc, #40\]	@ 000000e4 <foo\+0xe4>
000000b8 <foo\+0xb8> 0d9fea09 	vldreq	s28, \[pc, #36\]	@ 000000e4 <foo\+0xe4>
000000bc <foo\+0xbc> 0ddffa08 	vldreq	s31, \[pc, #32\]	@ 000000e4 <foo\+0xe4>
000000c0 <foo\+0xc0> 0d9f0a08 	vldreq	s0, \[pc, #32\]	@ 000000e8 <foo\+0xe8>
000000c4 <foo\+0xc4> 0d9f7a07 	vldreq	s14, \[pc, #28\]	@ 000000e8 <foo\+0xe8>
000000c8 <foo\+0xc8> 0d9fea06 	vldreq	s28, \[pc, #24\]	@ 000000e8 <foo\+0xe8>
000000cc <foo\+0xcc> 0ddffa05 	vldreq	s31, \[pc, #20\]	@ 000000e8 <foo\+0xe8>
000000d0 <foo\+0xd0> 0d9f0a05 	vldreq	s0, \[pc, #20\]	@ 000000ec <foo\+0xec>
000000d4 <foo\+0xd4> 0d9f7a04 	vldreq	s14, \[pc, #16\]	@ 000000ec <foo\+0xec>
000000d8 <foo\+0xd8> 0d9fea03 	vldreq	s28, \[pc, #12\]	@ 000000ec <foo\+0xec>
000000dc <foo\+0xdc> 0ddffa02 	vldreq	s31, \[pc, #8\]	@ 000000ec <foo\+0xec>
000000e0 <foo\+0xe0> 00000000 	.word	0x00000000
000000e4 <foo\+0xe4> 0000ff00 	.word	0x0000ff00
000000e8 <foo\+0xe8> ffff00ff 	.word	0xffff00ff
000000ec <foo\+0xec> 000fff00 	.word	0x000fff00
000000f0 <foo\+0xf0> 4d9f0a0e 	vldrmi	s0, \[pc, #56\]	@ 00000130 <foo\+0x130>
000000f4 <foo\+0xf4> 4d9f7a0d 	vldrmi	s14, \[pc, #52\]	@ 00000130 <foo\+0x130>
000000f8 <foo\+0xf8> 4d9fea0c 	vldrmi	s28, \[pc, #48\]	@ 00000130 <foo\+0x130>
000000fc <foo\+0xfc> 4ddffa0b 	vldrmi	s31, \[pc, #44\]	@ 00000130 <foo\+0x130>
00000100 <foo\+0x100> 4d9f0a0b 	vldrmi	s0, \[pc, #44\]	@ 00000134 <foo\+0x134>
00000104 <foo\+0x104> 4d9f7a0a 	vldrmi	s14, \[pc, #40\]	@ 00000134 <foo\+0x134>
00000108 <foo\+0x108> 4d9fea09 	vldrmi	s28, \[pc, #36\]	@ 00000134 <foo\+0x134>
0000010c <foo\+0x10c> 4ddffa08 	vldrmi	s31, \[pc, #32\]	@ 00000134 <foo\+0x134>
00000110 <foo\+0x110> 4d9f0a08 	vldrmi	s0, \[pc, #32\]	@ 00000138 <foo\+0x138>
00000114 <foo\+0x114> 4d9f7a07 	vldrmi	s14, \[pc, #28\]	@ 00000138 <foo\+0x138>
00000118 <foo\+0x118> 4d9fea06 	vldrmi	s28, \[pc, #24\]	@ 00000138 <foo\+0x138>
0000011c <foo\+0x11c> 4ddffa05 	vldrmi	s31, \[pc, #20\]	@ 00000138 <foo\+0x138>
00000120 <foo\+0x120> 4d9f0a05 	vldrmi	s0, \[pc, #20\]	@ 0000013c <foo\+0x13c>
00000124 <foo\+0x124> 4d9f7a04 	vldrmi	s14, \[pc, #16\]	@ 0000013c <foo\+0x13c>
00000128 <foo\+0x128> 4d9fea03 	vldrmi	s28, \[pc, #12\]	@ 0000013c <foo\+0x13c>
0000012c <foo\+0x12c> 4ddffa02 	vldrmi	s31, \[pc, #8\]	@ 0000013c <foo\+0x13c>
00000130 <foo\+0x130> 00000000 	.word	0x00000000
00000134 <foo\+0x134> 000000ff 	.word	0x000000ff
00000138 <foo\+0x138> ffffff00 	.word	0xffffff00
0000013c <foo\+0x13c> 0000fff0 	.word	0x0000fff0
00000140 <foo\+0x140> f2800e30 	vmov.i64	d0, #0x0000000000000000
00000144 <foo\+0x144> f280ee30 	vmov.i64	d14, #0x0000000000000000
00000148 <foo\+0x148> f2c0ce30 	vmov.i64	d28, #0x0000000000000000
0000014c <foo\+0x14c> f2c0fe30 	vmov.i64	d31, #0x0000000000000000
00000150 <foo\+0x150> ed9f0b0a 	vldr	d0, \[pc, #40\]	@ 00000180 <foo\+0x180>
00000154 <foo\+0x154> ed9feb09 	vldr	d14, \[pc, #36\]	@ 00000180 <foo\+0x180>
00000158 <foo\+0x158> eddfcb08 	vldr	d28, \[pc, #32\]	@ 00000180 <foo\+0x180>
0000015c <foo\+0x15c> eddffb07 	vldr	d31, \[pc, #28\]	@ 00000180 <foo\+0x180>
00000160 <foo\+0x160> f3870e3f 	vmov.i64	d0, #0xffffffffffffffff
00000164 <foo\+0x164> f387ee3f 	vmov.i64	d14, #0xffffffffffffffff
00000168 <foo\+0x168> f3c7ce3f 	vmov.i64	d28, #0xffffffffffffffff
0000016c <foo\+0x16c> f3c7fe3f 	vmov.i64	d31, #0xffffffffffffffff
00000170 <foo\+0x170> ed9f0b04 	vldr	d0, \[pc, #16\]	@ 00000188 <foo\+0x188>
00000174 <foo\+0x174> ed9feb03 	vldr	d14, \[pc, #12\]	@ 00000188 <foo\+0x188>
00000178 <foo\+0x178> eddfcb02 	vldr	d28, \[pc, #8\]	@ 00000188 <foo\+0x188>
0000017c <foo\+0x17c> eddffb01 	vldr	d31, \[pc, #4\]	@ 00000188 <foo\+0x188>
00000180 <foo\+0x180> ca000000 	.word	0xca000000
00000184 <foo\+0x184> 00000000 	.word	0x00000000
00000188 <foo\+0x188> 0fff0000 	.word	0x0fff0000
0000018c <foo\+0x18c> 00000000 	.word	0x00000000
00000190 <foo\+0x190> f2800e30 	vmov.i64	d0, #0x0000000000000000
00000194 <foo\+0x194> f280ee30 	vmov.i64	d14, #0x0000000000000000
00000198 <foo\+0x198> f2c0ce30 	vmov.i64	d28, #0x0000000000000000
0000019c <foo\+0x19c> f2c0fe30 	vmov.i64	d31, #0x0000000000000000
000001a0 <foo\+0x1a0> f2800e34 	vmov.i64	d0, #0x0000000000ff0000
000001a4 <foo\+0x1a4> f280ee34 	vmov.i64	d14, #0x0000000000ff0000
000001a8 <foo\+0x1a8> f2c0ce34 	vmov.i64	d28, #0x0000000000ff0000
000001ac <foo\+0x1ac> f2c0fe34 	vmov.i64	d31, #0x0000000000ff0000
000001b0 <foo\+0x1b0> f2800e39 	vmov.i64	d0, #0x00000000ff0000ff
000001b4 <foo\+0x1b4> f280ee39 	vmov.i64	d14, #0x00000000ff0000ff
000001b8 <foo\+0x1b8> f2c0ce39 	vmov.i64	d28, #0x00000000ff0000ff
000001bc <foo\+0x1bc> f2c0fe39 	vmov.i64	d31, #0x00000000ff0000ff
000001c0 <foo\+0x1c0> ed9f0b02 	vldr	d0, \[pc, #8\]	@ 000001d0 <foo\+0x1d0>
000001c4 <foo\+0x1c4> ed9feb01 	vldr	d14, \[pc, #4\]	@ 000001d0 <foo\+0x1d0>
000001c8 <foo\+0x1c8> eddfcb00 	vldr	d28, \[pc\]	@ 000001d0 <foo\+0x1d0>
000001cc <foo\+0x1cc> ed5ffb01 	vldr	d31, \[pc, #-4\]	@ 000001d0 <foo\+0x1d0>
000001d0 <foo\+0x1d0> 00fff000 	.word	0x00fff000
000001d4 <foo\+0x1d4> 00000000 	.word	0x00000000
000001d8 <foo\+0x1d8> f2800e30 	vmov.i64	d0, #0x0000000000000000
000001dc <foo\+0x1dc> f280ee30 	vmov.i64	d14, #0x0000000000000000
000001e0 <foo\+0x1e0> f2c0ce30 	vmov.i64	d28, #0x0000000000000000
000001e4 <foo\+0x1e4> f2c0fe30 	vmov.i64	d31, #0x0000000000000000
000001e8 <foo\+0x1e8> f2800e32 	vmov.i64	d0, #0x000000000000ff00
000001ec <foo\+0x1ec> f280ee32 	vmov.i64	d14, #0x000000000000ff00
000001f0 <foo\+0x1f0> f2c0ce32 	vmov.i64	d28, #0x000000000000ff00
000001f4 <foo\+0x1f4> f2c0fe32 	vmov.i64	d31, #0x000000000000ff00
000001f8 <foo\+0x1f8> f2800e3d 	vmov.i64	d0, #0x00000000ffff00ff
000001fc <foo\+0x1fc> f280ee3d 	vmov.i64	d14, #0x00000000ffff00ff
00000200 <foo\+0x200> f2c0ce3d 	vmov.i64	d28, #0x00000000ffff00ff
00000204 <foo\+0x204> f2c0fe3d 	vmov.i64	d31, #0x00000000ffff00ff
00000208 <foo\+0x208> 0d9f0b02 	vldreq	d0, \[pc, #8\]	@ 00000218 <foo\+0x218>
0000020c <foo\+0x20c> 0d9feb01 	vldreq	d14, \[pc, #4\]	@ 00000218 <foo\+0x218>
00000210 <foo\+0x210> 0ddfcb00 	vldreq	d28, \[pc\]	@ 00000218 <foo\+0x218>
00000214 <foo\+0x214> 0d5ffb01 	vldreq	d31, \[pc, #-4\]	@ 00000218 <foo\+0x218>
00000218 <foo\+0x218> 000fff00 	.word	0x000fff00
0000021c <foo\+0x21c> 00000000 	.word	0x00000000
00000220 <foo\+0x220> f2800e30 	vmov.i64	d0, #0x0000000000000000
00000224 <foo\+0x224> f280ee30 	vmov.i64	d14, #0x0000000000000000
00000228 <foo\+0x228> f2c0ce30 	vmov.i64	d28, #0x0000000000000000
0000022c <foo\+0x22c> f2c0fe30 	vmov.i64	d31, #0x0000000000000000
00000230 <foo\+0x230> f2800e31 	vmov.i64	d0, #0x00000000000000ff
00000234 <foo\+0x234> f280ee31 	vmov.i64	d14, #0x00000000000000ff
00000238 <foo\+0x238> f2c0ce31 	vmov.i64	d28, #0x00000000000000ff
0000023c <foo\+0x23c> f2c0fe31 	vmov.i64	d31, #0x00000000000000ff
00000240 <foo\+0x240> f2800e3e 	vmov.i64	d0, #0x00000000ffffff00
00000244 <foo\+0x244> f280ee3e 	vmov.i64	d14, #0x00000000ffffff00
00000248 <foo\+0x248> f2c0ce3e 	vmov.i64	d28, #0x00000000ffffff00
0000024c <foo\+0x24c> f2c0fe3e 	vmov.i64	d31, #0x00000000ffffff00
00000250 <foo\+0x250> f2800e33 	vmov.i64	d0, #0x000000000000ffff
00000254 <foo\+0x254> f280ee33 	vmov.i64	d14, #0x000000000000ffff
00000258 <foo\+0x258> f2c0ce33 	vmov.i64	d28, #0x000000000000ffff
0000025c <foo\+0x25c> f2c0fe33 	vmov.i64	d31, #0x000000000000ffff
00000260 <foo\+0x260> f2800e30 	vmov.i64	d0, #0x0000000000000000
00000264 <foo\+0x264> f280ee30 	vmov.i64	d14, #0x0000000000000000
00000268 <foo\+0x268> f2c0ce30 	vmov.i64	d28, #0x0000000000000000
0000026c <foo\+0x26c> f2c0fe30 	vmov.i64	d31, #0x0000000000000000
00000270 <foo\+0x270> f3800e30 	vmov.i64	d0, #0xff00000000000000
00000274 <foo\+0x274> f380ee30 	vmov.i64	d14, #0xff00000000000000
00000278 <foo\+0x278> f3c0ce30 	vmov.i64	d28, #0xff00000000000000
0000027c <foo\+0x27c> f3c0fe30 	vmov.i64	d31, #0xff00000000000000
00000280 <foo\+0x280> f3870e3f 	vmov.i64	d0, #0xffffffffffffffff
00000284 <foo\+0x284> f387ee3f 	vmov.i64	d14, #0xffffffffffffffff
00000288 <foo\+0x288> f3c7ce3f 	vmov.i64	d28, #0xffffffffffffffff
0000028c <foo\+0x28c> f3c7fe3f 	vmov.i64	d31, #0xffffffffffffffff
00000290 <foo\+0x290> ed9f0b02 	vldr	d0, \[pc, #8\]	@ 000002a0 <foo\+0x2a0>
00000294 <foo\+0x294> ed9feb01 	vldr	d14, \[pc, #4\]	@ 000002a0 <foo\+0x2a0>
00000298 <foo\+0x298> eddfcb00 	vldr	d28, \[pc\]	@ 000002a0 <foo\+0x2a0>
0000029c <foo\+0x29c> ed5ffb01 	vldr	d31, \[pc, #-4\]	@ 000002a0 <foo\+0x2a0>
000002a0 <foo\+0x2a0> 00000000 	.word	0x00000000
000002a4 <foo\+0x2a4> 0fff0000 	.word	0x0fff0000
000002a8 <foo\+0x2a8> f2800e30 	vmov.i64	d0, #0x0000000000000000
000002ac <foo\+0x2ac> f280ee30 	vmov.i64	d14, #0x0000000000000000
000002b0 <foo\+0x2b0> f2c0ce30 	vmov.i64	d28, #0x0000000000000000
000002b4 <foo\+0x2b4> f2c0fe30 	vmov.i64	d31, #0x0000000000000000
000002b8 <foo\+0x2b8> ed9f0b0a 	vldr	d0, \[pc, #40\]	@ 000002e8 <foo\+0x2e8>
000002bc <foo\+0x2bc> ed9feb09 	vldr	d14, \[pc, #36\]	@ 000002e8 <foo\+0x2e8>
000002c0 <foo\+0x2c0> eddfcb08 	vldr	d28, \[pc, #32\]	@ 000002e8 <foo\+0x2e8>
000002c4 <foo\+0x2c4> eddffb07 	vldr	d31, \[pc, #28\]	@ 000002e8 <foo\+0x2e8>
000002c8 <foo\+0x2c8> ed9f0b08 	vldr	d0, \[pc, #32\]	@ 000002f0 <foo\+0x2f0>
000002cc <foo\+0x2cc> ed9feb07 	vldr	d14, \[pc, #28\]	@ 000002f0 <foo\+0x2f0>
000002d0 <foo\+0x2d0> eddfcb06 	vldr	d28, \[pc, #24\]	@ 000002f0 <foo\+0x2f0>
000002d4 <foo\+0x2d4> eddffb05 	vldr	d31, \[pc, #20\]	@ 000002f0 <foo\+0x2f0>
000002d8 <foo\+0x2d8> ed9f0b06 	vldr	d0, \[pc, #24\]	@ 000002f8 <foo\+0x2f8>
000002dc <foo\+0x2dc> ed9feb05 	vldr	d14, \[pc, #20\]	@ 000002f8 <foo\+0x2f8>
000002e0 <foo\+0x2e0> eddfcb04 	vldr	d28, \[pc, #16\]	@ 000002f8 <foo\+0x2f8>
000002e4 <foo\+0x2e4> eddffb03 	vldr	d31, \[pc, #12\]	@ 000002f8 <foo\+0x2f8>
000002e8 <foo\+0x2e8> 00000000 	.word	0x00000000
000002ec <foo\+0x2ec> 000ff000 	.word	0x000ff000
000002f0 <foo\+0x2f0> f0000000 	.word	0xf0000000
000002f4 <foo\+0x2f4> 0ff00fff 	.word	0x0ff00fff
000002f8 <foo\+0x2f8> 00000000 	.word	0x00000000
000002fc <foo\+0x2fc> 000fff00 	.word	0x000fff00
00000300 <foo\+0x300> f2800e30 	vmov.i64	d0, #0x0000000000000000
00000304 <foo\+0x304> f280ee30 	vmov.i64	d14, #0x0000000000000000
00000308 <foo\+0x308> f2c0ce30 	vmov.i64	d28, #0x0000000000000000
0000030c <foo\+0x30c> f2c0fe30 	vmov.i64	d31, #0x0000000000000000
00000310 <foo\+0x310> f2820e30 	vmov.i64	d0, #0x0000ff0000000000
00000314 <foo\+0x314> f282ee30 	vmov.i64	d14, #0x0000ff0000000000
00000318 <foo\+0x318> f2c2ce30 	vmov.i64	d28, #0x0000ff0000000000
0000031c <foo\+0x31c> f2c2fe30 	vmov.i64	d31, #0x0000ff0000000000
00000320 <foo\+0x320> f3850e30 	vmov.i64	d0, #0xffff00ff00000000
00000324 <foo\+0x324> f385ee30 	vmov.i64	d14, #0xffff00ff00000000
00000328 <foo\+0x328> f3c5ce30 	vmov.i64	d28, #0xffff00ff00000000
0000032c <foo\+0x32c> f3c5fe30 	vmov.i64	d31, #0xffff00ff00000000
00000330 <foo\+0x330> 0d9f0b02 	vldreq	d0, \[pc, #8\]	@ 00000340 <foo\+0x340>
00000334 <foo\+0x334> 0d9feb01 	vldreq	d14, \[pc, #4\]	@ 00000340 <foo\+0x340>
00000338 <foo\+0x338> 0ddfcb00 	vldreq	d28, \[pc\]	@ 00000340 <foo\+0x340>
0000033c <foo\+0x33c> 0d5ffb01 	vldreq	d31, \[pc, #-4\]	@ 00000340 <foo\+0x340>
00000340 <foo\+0x340> 00000000 	.word	0x00000000
00000344 <foo\+0x344> 000fff00 	.word	0x000fff00
00000348 <foo\+0x348> f2800e30 	vmov.i64	d0, #0x0000000000000000
0000034c <foo\+0x34c> f280ee30 	vmov.i64	d14, #0x0000000000000000
00000350 <foo\+0x350> f2c0ce30 	vmov.i64	d28, #0x0000000000000000
00000354 <foo\+0x354> f2c0fe30 	vmov.i64	d31, #0x0000000000000000
00000358 <foo\+0x358> f2810e30 	vmov.i64	d0, #0x000000ff00000000
0000035c <foo\+0x35c> f281ee30 	vmov.i64	d14, #0x000000ff00000000
00000360 <foo\+0x360> f2c1ce30 	vmov.i64	d28, #0x000000ff00000000
00000364 <foo\+0x364> f2c1fe30 	vmov.i64	d31, #0x000000ff00000000
00000368 <foo\+0x368> f3860e30 	vmov.i64	d0, #0xffffff0000000000
0000036c <foo\+0x36c> f386ee30 	vmov.i64	d14, #0xffffff0000000000
00000370 <foo\+0x370> f3c6ce30 	vmov.i64	d28, #0xffffff0000000000
00000374 <foo\+0x374> f3c6fe30 	vmov.i64	d31, #0xffffff0000000000
00000378 <foo\+0x378> 4d9f0b02 	vldrmi	d0, \[pc, #8\]	@ 00000388 <foo\+0x388>
0000037c <foo\+0x37c> 4d9feb01 	vldrmi	d14, \[pc, #4\]	@ 00000388 <foo\+0x388>
00000380 <foo\+0x380> 4ddfcb00 	vldrmi	d28, \[pc\]	@ 00000388 <foo\+0x388>
00000384 <foo\+0x384> 4d5ffb01 	vldrmi	d31, \[pc, #-4\]	@ 00000388 <foo\+0x388>
00000388 <foo\+0x388> 00000000 	.word	0x00000000
0000038c <foo\+0x38c> 0000fff0 	.word	0x0000fff0
00000390 <foo\+0x390> ed9f1b00 	vldr	d1, \[pc\]	@ 00000398 <foo\+0x398>
	\.\.\.
0000039c <foo\+0x39c> 0000fff0 	.word	0x0000fff0
000003a0 <foo\+0x3a0> e2810000 	add	r0, r1, #0
000003a4 <foo\+0x3a4> ed1f1b01 	vldr	d1, \[pc, #-4\]	@ 000003a8 <foo\+0x3a8>
000003a8 <foo\+0x3a8> 00000000 	.word	0x00000000
000003ac <foo\+0x3ac> 0000fff0 	.word	0x0000fff0
000003b0 <foo\+0x3b0> ed9f1b10 	vldr	d1, \[pc, #64\]	@ 000003f8 <foo\+0x3f8>
000003b4 <foo\+0x3b4> ed9f1a11 	vldr	s2, \[pc, #68\]	@ 00000400 <foo\+0x400>
000003b8 <foo\+0x3b8> ed9f3b12 	vldr	d3, \[pc, #72\]	@ 00000408 <foo\+0x408>
000003bc <foo\+0x3bc> ed9f2a10 	vldr	s4, \[pc, #64\]	@ 00000404 <foo\+0x404>
000003c0 <foo\+0x3c0> ed9f5b10 	vldr	d5, \[pc, #64\]	@ 00000408 <foo\+0x408>
000003c4 <foo\+0x3c4> ed9f6b11 	vldr	d6, \[pc, #68\]	@ 00000410 <foo\+0x410>
000003c8 <foo\+0x3c8> ed9f7b12 	vldr	d7, \[pc, #72\]	@ 00000418 <foo\+0x418>
000003cc <foo\+0x3cc> ed9f4a13 	vldr	s8, \[pc, #76\]	@ 00000420 <foo\+0x420>
000003d0 <foo\+0x3d0> ed9f9b14 	vldr	d9, \[pc, #80\]	@ 00000428 <foo\+0x428>
000003d4 <foo\+0x3d4> ed9f5a12 	vldr	s10, \[pc, #72\]	@ 00000424 <foo\+0x424>
000003d8 <foo\+0x3d8> ed9fbb14 	vldr	d11, \[pc, #80\]	@ 00000430 <foo\+0x430>
000003dc <foo\+0x3dc> ed9f6a15 	vldr	s12, \[pc, #84\]	@ 00000438 <foo\+0x438>
000003e0 <foo\+0x3e0> eddf6a15 	vldr	s13, \[pc, #84\]	@ 0000043c <foo\+0x43c>
000003e4 <foo\+0x3e4> ed9f7a06 	vldr	s14, \[pc, #24\]	@ 00000404 <foo\+0x404>
000003e8 <foo\+0x3e8> eddf7a03 	vldr	s15, \[pc, #12\]	@ 000003fc <foo\+0x3fc>
000003ec <foo\+0x3ec> eddf0b11 	vldr	d16, \[pc, #68\]	@ 00000438 <foo\+0x438>
000003f0 <foo\+0x3f0> eddf1b12 	vldr	d17, \[pc, #72\]	@ 00000440 <foo\+0x440>
	\.\.\.
000003fc <foo\+0x3fc> 0000fff0 	.word	0x0000fff0
00000400 <foo\+0x400> ff000000 	.word	0xff000000
00000404 <foo\+0x404> ff000001 	.word	0xff000001
00000408 <foo\+0x408> 00000001 	.word	0x00000001
0000040c <foo\+0x40c> 0000fff0 	.word	0x0000fff0
00000410 <foo\+0x410> 00000002 	.word	0x00000002
00000414 <foo\+0x414> 0000fff0 	.word	0x0000fff0
00000418 <foo\+0x418> 00000003 	.word	0x00000003
0000041c <foo\+0x41c> 0000fff0 	.word	0x0000fff0
00000420 <foo\+0x420> ff000002 	.word	0xff000002
00000424 <foo\+0x424> ff000003 	.word	0xff000003
00000428 <foo\+0x428> 00000004 	.word	0x00000004
0000042c <foo\+0x42c> 0000fff0 	.word	0x0000fff0
00000430 <foo\+0x430> 00000005 	.word	0x00000005
00000434 <foo\+0x434> 0000fff0 	.word	0x0000fff0
00000438 <foo\+0x438> ff000004 	.word	0xff000004
0000043c <foo\+0x43c> ff000005 	.word	0xff000005
00000440 <foo\+0x440> 0000fff0 	.word	0x0000fff0
00000444 <foo\+0x444> ff000004 	.word	0xff000004
