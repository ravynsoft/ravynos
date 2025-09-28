#objdump: -dr --prefix-addresses --show-raw-insn
#name: Thumb-2 VFP Additional instructions
#as: -mfpu=vfp

# Test the ARM VFP Double Precision instructions

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <[^>]*> ec4a 5b10 	vmov	d0, r5, sl
0+004 <[^>]*> ec5a 5b10 	vmov	r5, sl, d0
0+008 <[^>]*> ec4a 5a37 	(vmov	s15, s16, r5, sl|fmsrr	{s15, s16}, r5, sl)
0+00c <[^>]*> ec5a 5a37 	(vmov	r5, sl, s15, s16|fmrrs	r5, sl, {s15, s16})
0+010 <[^>]*> ec45 ab1f 	vmov	d15, sl, r5
0+014 <[^>]*> ec55 ab1f 	vmov	sl, r5, d15
0+018 <[^>]*> ec45 aa38 	(vmov	s17, s18, sl, r5|fmsrr	{s17, s18}, sl, r5)
0+01c <[^>]*> ec55 aa38 	(vmov	sl, r5, s17, s18|fmrrs	sl, r5, {s17, s18})
