#PROG: objcopy
#ld: -T out-of-order.T
#objdump: -d
#name: Check if disassembler can handle sections in different order than header

.*: +file format .*aarch64.*

Disassembly of section \.func2:

.+ <\.func2>:
[^:]+:	8b010000 	add	x0, x0, x1

Disassembly of section \.func1:

.+ <v1>:
[^:]+:	8b010000 	add	x0, x0, x1
[^:]+:	00000000 	\.word	0x00000000

Disassembly of section \.func3:

.+ <\.func3>:
[^:]+:	8b010000 	add	x0, x0, x1
[^:]+:	8b010000 	add	x0, x0, x1
[^:]+:	8b010000 	add	x0, x0, x1
[^:]+:	8b010000 	add	x0, x0, x1
[^:]+:	8b010000 	add	x0, x0, x1
[^:]+:	00000000 	\.word	0x00000000
