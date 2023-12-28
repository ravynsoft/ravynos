#objdump: -dr --prefix-addresses --show-raw-insn
#name: MRS/MSR test, architecture v7-M, Thumb mode
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince

.*:     file format .*


Disassembly of section .text:
0+00 <[^>]*> f3ef 8400 	mrs	r4, CPSR
0+04 <[^>]*> f3ef 8502 	mrs	r5, EAPSR
0+08 <[^>]*> f3ef 8610 	mrs	r6, PRIMASK
0+0c <[^>]*> f383 8803 	msr	PSR, r3
0+10 <[^>]*> f384 8800 	msr	CPSR_f, r4
0+14 <[^>]*> f385 8801 	msr	IAPSR, r5
0+18 <[^>]*> f386 8810 	msr	PRIMASK, r6
