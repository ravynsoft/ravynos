#PROG: objcopy
#source: out-of-order.s
#ld: -e v1 -Ttext-segment=0x400000
#objdump: -d
#skip: *-*-pe *-wince-* *-*-coff
#name: Check if disassembler can handle sections in default order

.*: +file format .*arm.*

Disassembly of section \.func1:

.+ <v1>:
[^:]+:	e0800001 	add	r0, r0, r1
[^:]+:	00000000 	\.word	0x00000000

Disassembly of section \.func2:

.+ <\.func2>:
[^:]+:	e0800001 	add	r0, r0, r1

Disassembly of section \.func3:

.+ <\.func3>:
[^:]+:	e0800001 	add	r0, r0, r1
[^:]+:	e0800001 	add	r0, r0, r1
[^:]+:	e0800001 	add	r0, r0, r1
[^:]+:	e0800001 	add	r0, r0, r1
[^:]+:	e0800001 	add	r0, r0, r1
[^:]+:	00000000 	\.word	0x00000000
