#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 ldw

# Test the ld instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 01000017 	ldw	r4,0\(zero\)
0+0004 <[^>]*> 01000117 	ldw	r4,4\(zero\)
0+0008 <[^>]*> 011fff17 	ldw	r4,32764\(zero\)
0+000c <[^>]*> 01200017 	ldw	r4,-32768\(zero\)
0+0010 <[^>]*> 29000017 	ldw	r4,0\(r5\)
0+0014 <[^>]*> 29000117 	ldw	r4,4\(r5\)
0+0018 <[^>]*> 291fff17 	ldw	r4,32764\(r5\)
0+001c <[^>]*> 29200017 	ldw	r4,-32768\(r5\)
0+0020 <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*20: R_NIOS2_S16	.data
0+0024 <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*24: R_NIOS2_S16	big_external_data_label
0+0028 <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*28: R_NIOS2_S16	small_external_data_label
0+002c <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*2c: R_NIOS2_S16	big_external_common
0+0030 <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*30: R_NIOS2_S16	small_external_common
0+0034 <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*34: R_NIOS2_S16	.bss
0+0038 <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*38: R_NIOS2_S16	.bss\+0x4000
0+003c <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*3c: R_NIOS2_S16	.data\+0x4
0+0040 <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*40: R_NIOS2_S16	big_external_data_label\+0x4
0+0044 <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*44: R_NIOS2_S16	small_external_data_label\+0x4
0+0048 <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*48: R_NIOS2_S16	big_external_common\+0x4
0+004c <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*4c: R_NIOS2_S16	small_external_common\+0x4
0+0050 <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*50: R_NIOS2_S16	.bss\+0x4
0+0054 <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*54: R_NIOS2_S16	.bss\+0x4004
0+0058 <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*58: R_NIOS2_S16	.data-0x8000
0+005c <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*5c: R_NIOS2_S16	big_external_data_label-0x8000
0+0060 <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*60: R_NIOS2_S16	small_external_data_label-0x8000
0+0064 <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*64: R_NIOS2_S16	big_external_common-0x8000
0+0068 <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*68: R_NIOS2_S16	small_external_common-0x8000
0+006c <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*6c: R_NIOS2_S16	.bss-0x8000
0+0070 <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*70: R_NIOS2_S16	.bss-0x4000
0+0074 <[^>]*> 01000017 	ldw	r4,0\(zero\)
[	]*74: R_NIOS2_S16	.data\+0x10000
0+0078 <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*78: R_NIOS2_S16	.data
0+007c <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*7c: R_NIOS2_S16	big_external_data_label
0+0080 <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*80: R_NIOS2_S16	small_external_data_label
0+0084 <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*84: R_NIOS2_S16	big_external_common
0+0088 <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*88: R_NIOS2_S16	small_external_common
0+008c <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*8c: R_NIOS2_S16	.bss
0+0090 <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*90: R_NIOS2_S16	.bss\+0x4000
0+0094 <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*94: R_NIOS2_S16	.data\+0x4
0+0098 <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*98: R_NIOS2_S16	big_external_data_label\+0x4
0+009c <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*9c: R_NIOS2_S16	small_external_data_label\+0x4
0+00a0 <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*a0: R_NIOS2_S16	big_external_common\+0x4
0+00a4 <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*a4: R_NIOS2_S16	small_external_common\+0x4
0+00a8 <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*a8: R_NIOS2_S16	.bss\+0x4
0+00ac <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*ac: R_NIOS2_S16	.bss\+0x4004
0+00b0 <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*b0: R_NIOS2_S16	.data-0x8000
0+00b4 <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*b4: R_NIOS2_S16	big_external_data_label-0x8000
0+00b8 <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*b8: R_NIOS2_S16	small_external_data_label-0x8000
0+00bc <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*bc: R_NIOS2_S16	big_external_common-0x8000
0+00c0 <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*c0: R_NIOS2_S16	small_external_common-0x8000
0+00c4 <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*c4: R_NIOS2_S16	.bss-0x8000
0+00c8 <[^>]*> 29000017 	ldw	r4,0\(r5\)
[	]*c8: R_NIOS2_S16	.bss-0x4000
0+00cc <[^>]*> 01000037 	ldwio	r4,0\(zero\)
0+00d0 <[^>]*> 01000137 	ldwio	r4,4\(zero\)
0+00d4 <[^>]*> 011fff37 	ldwio	r4,32764\(zero\)
0+00d8 <[^>]*> 01200037 	ldwio	r4,-32768\(zero\)
0+00dc <[^>]*> 29000037 	ldwio	r4,0\(r5\)
0+00e0 <[^>]*> 29000137 	ldwio	r4,4\(r5\)
0+00e4 <[^>]*> 291fff37 	ldwio	r4,32764\(r5\)
0+00e8 <[^>]*> 29200037 	ldwio	r4,-32768\(r5\)
0+00ec <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*ec: R_NIOS2_S16	.data
0+00f0 <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*f0: R_NIOS2_S16	big_external_data_label
0+00f4 <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*f4: R_NIOS2_S16	small_external_data_label
0+00f8 <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*f8: R_NIOS2_S16	big_external_common
0+00fc <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*fc: R_NIOS2_S16	small_external_common
0+0100 <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*100: R_NIOS2_S16	.bss
0+0104 <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*104: R_NIOS2_S16	.bss\+0x4000
0+0108 <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*108: R_NIOS2_S16	.data\+0x4
0+010c <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*10c: R_NIOS2_S16	big_external_data_label\+0x4
0+0110 <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*110: R_NIOS2_S16	small_external_data_label\+0x4
0+0114 <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*114: R_NIOS2_S16	big_external_common\+0x4
0+0118 <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*118: R_NIOS2_S16	small_external_common\+0x4
0+011c <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*11c: R_NIOS2_S16	.bss\+0x4
0+0120 <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*120: R_NIOS2_S16	.bss\+0x4004
0+0124 <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*124: R_NIOS2_S16	.data-0x8000
0+0128 <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*128: R_NIOS2_S16	big_external_data_label-0x8000
0+012c <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*12c: R_NIOS2_S16	small_external_data_label-0x8000
0+0130 <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*130: R_NIOS2_S16	big_external_common-0x8000
0+0134 <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*134: R_NIOS2_S16	small_external_common-0x8000
0+0138 <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*138: R_NIOS2_S16	.bss-0x8000
0+013c <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*13c: R_NIOS2_S16	.bss-0x4000
0+0140 <[^>]*> 01000037 	ldwio	r4,0\(zero\)
[	]*140: R_NIOS2_S16	.data\+0x10000
0+0144 <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*144: R_NIOS2_S16	.data
0+0148 <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*148: R_NIOS2_S16	big_external_data_label
0+014c <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*14c: R_NIOS2_S16	small_external_data_label
0+0150 <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*150: R_NIOS2_S16	big_external_common
0+0154 <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*154: R_NIOS2_S16	small_external_common
0+0158 <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*158: R_NIOS2_S16	.bss
0+015c <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*15c: R_NIOS2_S16	.bss\+0x4000
0+0160 <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*160: R_NIOS2_S16	.data\+0x4
0+0164 <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*164: R_NIOS2_S16	big_external_data_label\+0x4
0+0168 <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*168: R_NIOS2_S16	small_external_data_label\+0x4
0+016c <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*16c: R_NIOS2_S16	big_external_common\+0x4
0+0170 <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*170: R_NIOS2_S16	small_external_common\+0x4
0+0174 <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*174: R_NIOS2_S16	.bss\+0x4
0+0178 <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*178: R_NIOS2_S16	.bss\+0x4004
0+017c <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*17c: R_NIOS2_S16	.data-0x8000
0+0180 <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*180: R_NIOS2_S16	big_external_data_label-0x8000
0+0184 <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*184: R_NIOS2_S16	small_external_data_label-0x8000
0+0188 <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*188: R_NIOS2_S16	big_external_common-0x8000
0+018c <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*18c: R_NIOS2_S16	small_external_common-0x8000
0+0190 <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*190: R_NIOS2_S16	.bss-0x8000
0+0194 <[^>]*> 29000037 	ldwio	r4,0\(r5\)
[	]*194: R_NIOS2_S16	.bss-0x4000
