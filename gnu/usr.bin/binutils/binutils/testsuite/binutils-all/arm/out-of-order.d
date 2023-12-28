#PROG: objcopy
#ld: -T out-of-order.T
#objdump: -d
#skip: *-*-pe *-wince-* *-*-coff
#name: Check if disassembler can handle sections in different order than header

.*: +file format .*arm.*

Disassembly of section \.func2:

.+ <\.func2>:
[^:]+:	e0800001 	add	r0, r0, r1

Disassembly of section \.func1:

.+ <v1>:
[^:]+:	e0800001 	add	r0, r0, r1
[^:]+:	00000000 	\.word	0x00000000

Disassembly of section \.func3:

.+ <\.func3>:
[^:]+:	e0800001 	add	r0, r0, r1
[^:]+:	e0800001 	add	r0, r0, r1
[^:]+:	e0800001 	add	r0, r0, r1
[^:]+:	e0800001 	add	r0, r0, r1
[^:]+:	e0800001 	add	r0, r0, r1
[^:]+:	00000000 	\.word	0x00000000
