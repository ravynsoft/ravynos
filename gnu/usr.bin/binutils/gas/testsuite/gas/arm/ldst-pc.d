#objdump: -dr --prefix-addresses --show-raw-insn
#name: ARM load/store with pc base register
#as: -mno-warn-deprecated

# Test the standard ARM instructions:

.*: +file format .*arm.*

Disassembly of section .text:
(0[0-9a-f]+) <[^>]+> e51f1008 	ldr	r1, \[pc, #-8\]	@ \1 <[^>]*>
0[0-9a-f]+ <[^>]+> e79f1002 	ldr	r1, \[pc, r2\]
0[0-9a-f]+ <[^>]+> e7df1002 	ldrb	r1, \[pc, r2\]
0[0-9a-f]+ <[^>]+> e18f00d2 	ldrd	r0, \[pc, r2\]
0[0-9a-f]+ <[^>]+> e19f10b2 	ldrh	r1, \[pc, r2\]
0[0-9a-f]+ <[^>]+> e19f10d2 	ldrsb	r1, \[pc, r2\]
0[0-9a-f]+ <[^>]+> e19f10f2 	ldrsh	r1, \[pc, r2\]
(0[0-9a-f]+) <[^>]+> f55ff008 	pld	\[pc, #-8\]	@ \1 <[^>]*>
0[0-9a-f]+ <[^>]+> f7dff001 	pld	\[pc, r1\]
(0[0-9a-f]+) <[^>]+> f45ff008 	pli	\[pc, #-8\]	@ \1 <[^>]*>
0[0-9a-f]+ <[^>]+> f6dff001 	pli	\[pc, r1\]
0[0-9a-f]+ <[^>]+> e58f1004 	str	r1, \[pc, #4\]	@ 0+038 <[^>]*>
0[0-9a-f]+ <[^>]+> e78f1002 	str	r1, \[pc, r2\]
0[0-9a-f]+ <[^>]+> e7cf1002 	strb	r1, \[pc, r2\]
0[0-9a-f]+ <[^>]+> e18f00f2 	strd	r0, \[pc, r2\]
0[0-9a-f]+ <[^>]+> e18f10b2 	strh	r1, \[pc, r2\]
