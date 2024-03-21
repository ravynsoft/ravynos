#objdump: -dr --prefix-addresses --show-raw-insn
#name: MRS/MSR test, architecture v6t2, Thumb mode
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince

.*:     file format .*


Disassembly of section .text:
0+00 <[^>]*> f3ef 8400 	mrs	r4, CPSR
0+04 <[^>]*> f3ef 8500 	mrs	r5, CPSR
0+08 <[^>]*> f3ff 8600 	mrs	r6, SPSR
0+0c <[^>]*> f384 8c00 	msr	CPSR_fs, r4
0+10 <[^>]*> f385 8800 	msr	CPSR_f, r5
0+14 <[^>]*> f396 8900 	msr	SPSR_fc, r6
