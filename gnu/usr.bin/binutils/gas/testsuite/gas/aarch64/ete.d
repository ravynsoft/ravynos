#name: ETE System registers
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
[^:]+:	d5310880 	mrs	x0, trcextinselr0
[^:]+:	d5310980 	mrs	x0, trcextinselr1
[^:]+:	d5310a80 	mrs	x0, trcextinselr2
[^:]+:	d5310b80 	mrs	x0, trcextinselr3
[^:]+:	d5310a00 	mrs	x0, trcrsr
[^:]+:	d5110880 	msr	trcextinselr0, x0
[^:]+:	d5110980 	msr	trcextinselr1, x0
[^:]+:	d5110a80 	msr	trcextinselr2, x0
[^:]+:	d5110b80 	msr	trcextinselr3, x0
[^:]+:	d5110a00 	msr	trcrsr, x0
