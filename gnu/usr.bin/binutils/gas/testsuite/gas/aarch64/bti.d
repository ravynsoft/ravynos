#as: -march=armv8-a
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
.*:	d503241f 	bti
.*:	d503245f 	bti	c
.*:	d503249f 	bti	j
.*:	d50324df 	bti	jc
.*:	d503245f 	bti	c
.*:	d503249f 	bti	j
.*:	d50324df 	bti	jc
