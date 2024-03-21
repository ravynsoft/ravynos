#PROG: objcopy
#source vdup-cond.s
#as: -mfpu=neon
#objdump: -d
#skip: *-*-pe *-wince-* *-*-coff
#name: Check if disassembler can handle vdup instructions in thumb

.*: +file format .*arm.*

Disassembly of section \.vdups:

.+ <\.vdups>:
[^:]+:	ee80 0b10 	vdup.32	d0, r0
