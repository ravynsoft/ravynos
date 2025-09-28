#PROG: objcopy
#source: out-of-order.s
#ld: -T out-of-order.T
#objdump: -D
#skip: *-*-pe *-wince-* *-*-coff
#name: Check if disassembler can handle all sections in different order than header

.*: +file format .*arm.*

Disassembly of section \.global:

.+ <\.global>:
	...

Disassembly of section \.func2:

.+ <\.func2>:
[^:]+:	e0800001 	add	r0, r0, r1

Disassembly of section \.func1:

.+ <v1>:
[^:]+:	e0800001 	add	r0, r0, r1
[^:]+:	00000000 	andeq	r0, r0, r0

Disassembly of section \.func3:

.+ <\.func3>:
[^:]+:	e0800001 	add	r0, r0, r1
[^:]+:	e0800001 	add	r0, r0, r1
[^:]+:	e0800001 	add	r0, r0, r1
[^:]+:	e0800001 	add	r0, r0, r1
[^:]+:	e0800001 	add	r0, r0, r1
[^:]+:	00000000 	andeq	r0, r0, r0

Disassembly of section \.rodata:

.+ <\.rodata>:
[^:]+:	00000000 	andeq	r0, r0, r0

Disassembly of section \.ARM\.attributes:

.+ <\.ARM\.attributes>:
[^:]+:	.+
[^:]+:	.+
[^:]+:	.+
[^:]+:	.+
[^:]+:	.+

