#name: DSB memory nXS barrier variant
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
.*:	d503323f 	dsb	oshnxs
.*:	d503363f 	dsb	nshnxs
.*:	d5033a3f 	dsb	ishnxs
.*:	d5033e3f 	dsb	synxs
.*:	d503323f 	dsb	oshnxs
.*:	d503363f 	dsb	nshnxs
.*:	d5033a3f 	dsb	ishnxs
.*:	d5033e3f 	dsb	synxs
