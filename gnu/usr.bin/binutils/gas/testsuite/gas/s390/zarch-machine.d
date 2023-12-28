#name: s390x machine
#objdump: -dr

.*: +file format .*

Disassembly of section .text:

.* <foo>:
.*:	e3 95 af ff 00 08 [ 	]*ag	%r9,4095\(%r5,%r10\)
.*:	eb d6 65 b3 01 6a [ 	]*asi	5555\(%r6\),-42
.*:	e3 95 af ff 00 18 [ 	]*agf	%r9,4095\(%r5,%r10\)
.*:	07 07 [ 	]*nopr	%r7
