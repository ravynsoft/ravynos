#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
[^:]+:	d51c1100 	msr	hcr_el2, x0
[^:]+:	d53c1100 	mrs	x0, hcr_el2
