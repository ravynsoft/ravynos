#PROG: objcopy
#source: out-of-order.s
#ld: -T out-of-order.T
#objdump: -D
#name: Check if disassembler can handle all sections in different order than header

.*: +file format .*aarch64.*

Disassembly of section \.global:

.+ <\.global>:
	...

Disassembly of section \.func2:

.+ <\.func2>:
[^:]+:	8b010000 	add	x0, x0, x1

Disassembly of section \.func1:

.+ <v1>:
[^:]+:	8b010000 	add	x0, x0, x1
[^:]+:	00000000 	udf	#0

Disassembly of section \.func3:

.+ <\.func3>:
[^:]+:	8b010000 	add	x0, x0, x1
[^:]+:	8b010000 	add	x0, x0, x1
[^:]+:	8b010000 	add	x0, x0, x1
[^:]+:	8b010000 	add	x0, x0, x1
[^:]+:	8b010000 	add	x0, x0, x1
[^:]+:	00000000 	udf	#0

Disassembly of section \.rodata:

.+ <\.rodata>:
[^:]+:	00000000 	udf	#0
