#name: s390x optargs
#objdump: -dr

.*: +file format .*

Disassembly of section .text:

.* <foo>:
.*:	e7 00 00 10 00 0e [	 ]*vst	%v0,16
.*:	e7 00 00 10 30 0e [	 ]*vst	%v0,16,3
.*:	e7 00 20 10 00 0e [	 ]*vst	%v0,16\(%r2\)
.*:	e7 00 20 10 30 0e [	 ]*vst	%v0,16\(%r2\),3
