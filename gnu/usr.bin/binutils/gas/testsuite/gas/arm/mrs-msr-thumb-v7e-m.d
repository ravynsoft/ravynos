#objdump: -dr --prefix-addresses --show-raw-insn
#name: MRS/MSR test, architecture v7e-M, Thumb mode
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince

.*:     file format .*


Disassembly of section .text:
0+00 <[^>]*> f3ef 8400 	mrs	r4, CPSR
0+04 <[^>]*> f3ef 8502 	mrs	r5, EAPSR
0+08 <[^>]*> f3ef 8610 	mrs	r6, PRIMASK
0+0c <[^>]*> f384 8c00 	msr	CPSR_fs, r4
0+10 <[^>]*> f385 8401 	msr	IAPSR, r5
0+14 <[^>]*> f386 8812 	msr	BASEPRI_MAX, r6
