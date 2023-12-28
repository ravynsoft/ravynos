#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 stb
#as: -march=r2

# Test the ld instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 00002027 	stb	r4,0\(zero\)
0+0004 <[^>]*> 00042027 	stb	r4,4\(zero\)
0+0008 <[^>]*> 07fc2027 	stb	r4,2044\(zero\)
0+000c <[^>]*> f8002027 	stb	r4,-2048\(zero\)
0+0010 <[^>]*> 00002167 	stb	r4,0\(r5\)
0+0014 <[^>]*> 00042167 	stb	r4,4\(r5\)
0+0018 <[^>]*> 07fc2167 	stb	r4,2044\(r5\)
0+001c <[^>]*> f8002167 	stb	r4,-2048\(r5\)
0+0020 <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*20: R_NIOS2_S16	.data
0+0024 <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*24: R_NIOS2_S16	big_external_data_label
0+0028 <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*28: R_NIOS2_S16	small_external_data_label
0+002c <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*2c: R_NIOS2_S16	big_external_common
0+0030 <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*30: R_NIOS2_S16	small_external_common
0+0034 <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*34: R_NIOS2_S16	.bss
0+0038 <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*38: R_NIOS2_S16	.bss\+0x400
0+003c <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*3c: R_NIOS2_S16	.data\+0x4
0+0040 <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*40: R_NIOS2_S16	big_external_data_label\+0x4
0+0044 <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*44: R_NIOS2_S16	small_external_data_label\+0x4
0+0048 <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*48: R_NIOS2_S16	big_external_common\+0x4
0+004c <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*4c: R_NIOS2_S16	small_external_common\+0x4
0+0050 <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*50: R_NIOS2_S16	.bss\+0x4
0+0054 <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*54: R_NIOS2_S16	.bss\+0x404
0+0058 <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*58: R_NIOS2_S16	.data-0x800
0+005c <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*5c: R_NIOS2_S16	big_external_data_label-0x800
0+0060 <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*60: R_NIOS2_S16	small_external_data_label-0x800
0+0064 <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*64: R_NIOS2_S16	big_external_common-0x800
0+0068 <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*68: R_NIOS2_S16	small_external_common-0x800
0+006c <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*6c: R_NIOS2_S16	.bss-0x800
0+0070 <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*70: R_NIOS2_S16	.bss-0x400
0+0074 <[^>]*> 00002027 	stb	r4,0\(zero\)
[	]*74: R_NIOS2_S16	.data\+0x10000
0+0078 <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*78: R_NIOS2_S16	.data
0+007c <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*7c: R_NIOS2_S16	big_external_data_label
0+0080 <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*80: R_NIOS2_S16	small_external_data_label
0+0084 <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*84: R_NIOS2_S16	big_external_common
0+0088 <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*88: R_NIOS2_S16	small_external_common
0+008c <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*8c: R_NIOS2_S16	.bss
0+0090 <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*90: R_NIOS2_S16	.bss\+0x400
0+0094 <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*94: R_NIOS2_S16	.data\+0x4
0+0098 <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*98: R_NIOS2_S16	big_external_data_label\+0x4
0+009c <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*9c: R_NIOS2_S16	small_external_data_label\+0x4
0+00a0 <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*a0: R_NIOS2_S16	big_external_common\+0x4
0+00a4 <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*a4: R_NIOS2_S16	small_external_common\+0x4
0+00a8 <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*a8: R_NIOS2_S16	.bss\+0x4
0+00ac <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*ac: R_NIOS2_S16	.bss\+0x404
0+00b0 <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*b0: R_NIOS2_S16	.data-0x800
0+00b4 <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*b4: R_NIOS2_S16	big_external_data_label-0x800
0+00b8 <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*b8: R_NIOS2_S16	small_external_data_label-0x800
0+00bc <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*bc: R_NIOS2_S16	big_external_common-0x800
0+00c0 <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*c0: R_NIOS2_S16	small_external_common-0x800
0+00c4 <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*c4: R_NIOS2_S16	.bss-0x800
0+00c8 <[^>]*> 00002167 	stb	r4,0\(r5\)
[	]*c8: R_NIOS2_S16	.bss-0x400
0+00cc <[^>]*> 10002028 	stbio	r4,0\(zero\)
0+00d0 <[^>]*> 10042028 	stbio	r4,4\(zero\)
0+00d4 <[^>]*> 17fc2028 	stbio	r4,2044\(zero\)
0+00d8 <[^>]*> 18002028 	stbio	r4,-2048\(zero\)
0+00dc <[^>]*> 10002168 	stbio	r4,0\(r5\)
0+00e0 <[^>]*> 10042168 	stbio	r4,4\(r5\)
0+00e4 <[^>]*> 17fc2168 	stbio	r4,2044\(r5\)
0+00e8 <[^>]*> 18002168 	stbio	r4,-2048\(r5\)
0+00ec <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*ec: R_NIOS2_R2_S12	.data
0+00f0 <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*f0: R_NIOS2_R2_S12	big_external_data_label
0+00f4 <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*f4: R_NIOS2_R2_S12	small_external_data_label
0+00f8 <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*f8: R_NIOS2_R2_S12	big_external_common
0+00fc <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*fc: R_NIOS2_R2_S12	small_external_common
0+0100 <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*100: R_NIOS2_R2_S12	.bss
0+0104 <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*104: R_NIOS2_R2_S12	.bss\+0x400
0+0108 <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*108: R_NIOS2_R2_S12	.data\+0x4
0+010c <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*10c: R_NIOS2_R2_S12	big_external_data_label\+0x4
0+0110 <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*110: R_NIOS2_R2_S12	small_external_data_label\+0x4
0+0114 <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*114: R_NIOS2_R2_S12	big_external_common\+0x4
0+0118 <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*118: R_NIOS2_R2_S12	small_external_common\+0x4
0+011c <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*11c: R_NIOS2_R2_S12	.bss\+0x4
0+0120 <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*120: R_NIOS2_R2_S12	.bss\+0x404
0+0124 <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*124: R_NIOS2_R2_S12	.data-0x800
0+0128 <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*128: R_NIOS2_R2_S12	big_external_data_label-0x800
0+012c <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*12c: R_NIOS2_R2_S12	small_external_data_label-0x800
0+0130 <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*130: R_NIOS2_R2_S12	big_external_common-0x800
0+0134 <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*134: R_NIOS2_R2_S12	small_external_common-0x800
0+0138 <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*138: R_NIOS2_R2_S12	.bss-0x800
0+013c <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*13c: R_NIOS2_R2_S12	.bss-0x400
0+0140 <[^>]*> 10002028 	stbio	r4,0\(zero\)
[	]*140: R_NIOS2_R2_S12	.data\+0x10000
0+0144 <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*144: R_NIOS2_R2_S12	.data
0+0148 <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*148: R_NIOS2_R2_S12	big_external_data_label
0+014c <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*14c: R_NIOS2_R2_S12	small_external_data_label
0+0150 <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*150: R_NIOS2_R2_S12	big_external_common
0+0154 <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*154: R_NIOS2_R2_S12	small_external_common
0+0158 <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*158: R_NIOS2_R2_S12	.bss
0+015c <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*15c: R_NIOS2_R2_S12	.bss\+0x400
0+0160 <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*160: R_NIOS2_R2_S12	.data\+0x4
0+0164 <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*164: R_NIOS2_R2_S12	big_external_data_label\+0x4
0+0168 <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*168: R_NIOS2_R2_S12	small_external_data_label\+0x4
0+016c <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*16c: R_NIOS2_R2_S12	big_external_common\+0x4
0+0170 <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*170: R_NIOS2_R2_S12	small_external_common\+0x4
0+0174 <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*174: R_NIOS2_R2_S12	.bss\+0x4
0+0178 <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*178: R_NIOS2_R2_S12	.bss\+0x404
0+017c <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*17c: R_NIOS2_R2_S12	.data-0x800
0+0180 <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*180: R_NIOS2_R2_S12	big_external_data_label-0x800
0+0184 <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*184: R_NIOS2_R2_S12	small_external_data_label-0x800
0+0188 <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*188: R_NIOS2_R2_S12	big_external_common-0x800
0+018c <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*18c: R_NIOS2_R2_S12	small_external_common-0x800
0+0190 <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*190: R_NIOS2_R2_S12	.bss-0x800
0+0194 <[^>]*> 10002168 	stbio	r4,0\(r5\)
[	]*194: R_NIOS2_R2_S12	.bss-0x400
