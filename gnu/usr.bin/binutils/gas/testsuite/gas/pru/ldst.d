#objdump: -dr --prefix-addresses --show-raw-insn
#name: PRU load-store

# Test the load/store operations

.*: +file format elf32-pru

Disassembly of section .text:
0+0000 <[^>]*> 240000f0 	ldi	r16, 0
0+0004 <[^>]*> 24fffff0 	ldi	r16, 65535
0+0008 <[^>]*> 2401fff0 	ldi	r16, 511
0+000c <[^>]*> f0611e20 	lbbo	r0.b1, r30, r1.b3, 1
0+0010 <[^>]*> fe41bec0 	lbbo	r0.b2, r30, r1.b2, 124
0+0014 <[^>]*> f1ff1e60 	lbbo	r0.b3, r30, 255, 1
0+0018 <[^>]*> f1011e80 	lbbo	r0.b0, r30, 1, 2
0+001c <[^>]*> fb005e00 	lbbo	r0.b0, r30, 0, 85
0+0020 <[^>]*> fea1d912 	lbbo	r18.b0, r25, r1.w1, r0.b0
0+0024 <[^>]*> ff65d992 	lbbo	r18.b0, r25, 101, r0.b1
0+0028 <[^>]*> fee1f992 	lbbo	r18.b0, r25, r1, r0.b3
0+002c <[^>]*> e0611e20 	sbbo	r0.b1, r30, r1.b3, 1
0+0030 <[^>]*> ee41bec0 	sbbo	r0.b2, r30, r1.b2, 124
0+0034 <[^>]*> e1ff1e60 	sbbo	r0.b3, r30, 255, 1
0+0038 <[^>]*> e1011e80 	sbbo	r0.b0, r30, 1, 2
0+003c <[^>]*> eb005e00 	sbbo	r0.b0, r30, 0, 85
0+0040 <[^>]*> eee1d912 	sbbo	r18.b0, r25, r1, r0.b0
0+0044 <[^>]*> ef65d992 	sbbo	r18.b0, r25, 101, r0.b1
0+0048 <[^>]*> eee1f992 	sbbo	r18.b0, r25, r1, r0.b3
0+004c <[^>]*> 9105608a 	lbco	r10.b0, 0, 5, 8
0+0050 <[^>]*> 90ab618a 	lbco	r10.b0, 1, r11.w1, 8
0+0054 <[^>]*> 91057f8a 	lbco	r10.b0, 31, 5, 8
0+0058 <[^>]*> 8105608a 	sbco	r10.b0, 0, 5, 8
0+005c <[^>]*> 80ab618a 	sbco	r10.b0, 1, r11.w1, 8
0+0060 <[^>]*> 81057f8a 	sbco	r10.b0, 31, 5, 8
