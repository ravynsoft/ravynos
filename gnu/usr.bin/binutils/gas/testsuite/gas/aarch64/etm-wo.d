#name: ETM write-only system registers
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
[^:]+:	d5117cc0 	msr	trclar, x0
[^:]+:	d5111080 	msr	trcoslar, x0
