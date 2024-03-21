#objdump: -dr --prefix-addresses --show-raw-insn
#name: MRS/MSR test, architecture v6, ARM mode

.*:     file format .*


Disassembly of section .text:
0+00 <[^>]*> e10f4000 	mrs	r4, CPSR
0+04 <[^>]*> e10f5000 	mrs	r5, CPSR
0+08 <[^>]*> e14f6000 	mrs	r6, SPSR
0+0c <[^>]*> e328f101 	msr	CPSR_f, #1073741824	@ 0x40000000
0+10 <[^>]*> e328f202 	msr	CPSR_f, #536870912	@ 0x20000000
0+14 <[^>]*> e369f201 	msr	SPSR_fc, #268435456	@ 0x10000000
0+18 <[^>]*> e128f004 	msr	CPSR_f, r4
0+1c <[^>]*> e128f005 	msr	CPSR_f, r5
0+20 <[^>]*> e169f006 	msr	SPSR_fc, r6
