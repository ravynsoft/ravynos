#source: ssbs.s
#objdump: -dr
#as: -march=armv8-a+ssbs --defsym SUCCESS=1

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
.*:	d503413f 	msr	ssbs, #0x1
.*:	d503403f 	msr	ssbs, #0x0
.*:	d51b42c0 	msr	ssbs, x0
.*:	d53b42c0 	mrs	x0, ssbs
.*:	d51b42c1 	msr	ssbs, x1
.*:	d53b42c1 	mrs	x1, ssbs
.*:	d51b42c2 	msr	ssbs, x2
.*:	d53b42c2 	mrs	x2, ssbs
.*:	d51b42c3 	msr	ssbs, x3
.*:	d53b42c3 	mrs	x3, ssbs
.*:	d51b42c4 	msr	ssbs, x4
.*:	d53b42c4 	mrs	x4, ssbs
.*:	d51b42c5 	msr	ssbs, x5
.*:	d53b42c5 	mrs	x5, ssbs
.*:	d51b42c6 	msr	ssbs, x6
.*:	d53b42c6 	mrs	x6, ssbs
.*:	d51b42c7 	msr	ssbs, x7
.*:	d53b42c7 	mrs	x7, ssbs
.*:	d51b42c8 	msr	ssbs, x8
.*:	d53b42c8 	mrs	x8, ssbs
.*:	d51b42c9 	msr	ssbs, x9
.*:	d53b42c9 	mrs	x9, ssbs
.*:	d51b42ca 	msr	ssbs, x10
.*:	d53b42ca 	mrs	x10, ssbs
.*:	d51b42cb 	msr	ssbs, x11
.*:	d53b42cb 	mrs	x11, ssbs
.*:	d51b42cc 	msr	ssbs, x12
.*:	d53b42cc 	mrs	x12, ssbs
.*:	d51b42cd 	msr	ssbs, x13
.*:	d53b42cd 	mrs	x13, ssbs
.*:	d51b42ce 	msr	ssbs, x14
.*:	d53b42ce 	mrs	x14, ssbs
.*:	d51b42cf 	msr	ssbs, x15
.*:	d53b42cf 	mrs	x15, ssbs
.*:	d51b42d0 	msr	ssbs, x16
.*:	d53b42d0 	mrs	x16, ssbs
.*:	d51b42d1 	msr	ssbs, x17
.*:	d53b42d1 	mrs	x17, ssbs
.*:	d51b42d2 	msr	ssbs, x18
.*:	d53b42d2 	mrs	x18, ssbs
.*:	d51b42d3 	msr	ssbs, x19
.*:	d53b42d3 	mrs	x19, ssbs
.*:	d51b42d4 	msr	ssbs, x20
.*:	d53b42d4 	mrs	x20, ssbs
.*:	d51b42d5 	msr	ssbs, x21
.*:	d53b42d5 	mrs	x21, ssbs
.*:	d51b42d6 	msr	ssbs, x22
.*:	d53b42d6 	mrs	x22, ssbs
.*:	d51b42d7 	msr	ssbs, x23
.*:	d53b42d7 	mrs	x23, ssbs
.*:	d51b42d8 	msr	ssbs, x24
.*:	d53b42d8 	mrs	x24, ssbs
.*:	d51b42d9 	msr	ssbs, x25
.*:	d53b42d9 	mrs	x25, ssbs
.*:	d51b42da 	msr	ssbs, x26
.*:	d53b42da 	mrs	x26, ssbs
.*:	d51b42db 	msr	ssbs, x27
.*:	d53b42db 	mrs	x27, ssbs
.*:	d51b42dc 	msr	ssbs, x28
.*:	d53b42dc 	mrs	x28, ssbs
.*:	d51b42dd 	msr	ssbs, x29
.*:	d53b42dd 	mrs	x29, ssbs
.*:	d51b42de 	msr	ssbs, x30
.*:	d53b42de 	mrs	x30, ssbs
