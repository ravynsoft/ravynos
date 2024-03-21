#as: -march=armv8-a
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
.*:	d503221f 	esb
.*:	d503221f 	esb
.*:	d503223f 	psb	csync
.*:	d503223f 	psb	csync
.*:	d503223f 	psb	csync
.*:	d503225f 	tsb	csync
.*:	d503225f 	tsb	csync
.*:	d503225f 	tsb	csync
