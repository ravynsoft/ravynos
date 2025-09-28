# name: ARMv6-M
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> f386 8800 	msr	(APSR|CPSR_f), r6
0[0-9a-f]+ <[^>]+> f389 8806 	msr	EPSR, r9
0[0-9a-f]+ <[^>]+> f3ef 8201 	mrs	r2, IAPSR
0[0-9a-f]+ <[^>]+> bf10      	yield
0[0-9a-f]+ <[^>]+> bf20      	wfe
0[0-9a-f]+ <[^>]+> bf30      	wfi
0[0-9a-f]+ <[^>]+> bf40      	sev
0[0-9a-f]+ <[^>]+> 4408      	add	r0, r1
0[0-9a-f]+ <[^>]+> 46c0      	nop.*
0[0-9a-f]+ <[^>]+> f3bf 8f5f 	dmb	sy
0[0-9a-f]+ <[^>]+> f3bf 8f4f 	dsb	sy
0[0-9a-f]+ <[^>]+> f3bf 8f6f 	isb	sy
