#objdump: -dr --prefix-addresses --show-raw-insn
#name: PRU ALU

# Test the ALU instructions

.*: +file format elf32-pru

Disassembly of section .text:
0+0000 <[^>]*> 00e4e4e4 	add	fp, fp, fp
0+0004 <[^>]*> 01ffe4e4 	add	fp, fp, 255
0+0008 <[^>]*> 0100e4e4 	add	fp, fp, 0
0+000c <[^>]*> 0100e4e4 	add	fp, fp, 0
0+0010 <[^>]*> 0100a424 	add	fp.b1, fp.w1, 0
0+0014 <[^>]*> 00634221 	add	r1.b1, sp.b2, ra.b3
0+0018 <[^>]*> 02634221 	adc	r1.b1, sp.b2, ra.b3
0+001c <[^>]*> 03634221 	adc	r1.b1, sp.b2, 99
0+0020 <[^>]*> 00e0e0e0 	add	r0, r0, r0
0+0024 <[^>]*> 02e0e0e0 	adc	r0, r0, r0
0+0028 <[^>]*> 050affe1 	sub	r1, r31, 10
0+002c <[^>]*> 070affe1 	suc	r1, r31, 10
0+0030 <[^>]*> 090affff 	lsl	r31, r31, 10
0+0034 <[^>]*> 0b0affff 	lsr	r31, r31, 10
0+0038 <[^>]*> 0d0a70f0 	rsb	r16, r16.b3, 10
0+003c <[^>]*> 0f0a70f0 	rsc	r16, r16.b3, 10
0+0040 <[^>]*> 11aa61a1 	and	r1.w1, r1.b3, 170
0+0044 <[^>]*> 13aa61a1 	or	r1.w1, r1.b3, 170
0+0048 <[^>]*> 15aa61a1 	xor	r1.w1, r1.b3, 170
0+004c <[^>]*> 1700e1e2 	not	sp, r1
0+0050 <[^>]*> 18e2e1e1 	min	r1, r1, sp
0+0054 <[^>]*> 1ac3e2e1 	max	r1, sp, ra.w2
0+0058 <[^>]*> 1cc3e2e1 	clr	r1, sp, ra.w2
0+005c <[^>]*> 1f0ce2e1 	set	r1, sp, 12
