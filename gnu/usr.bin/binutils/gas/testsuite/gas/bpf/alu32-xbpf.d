#as: --EL -mxbpf
#objdump: -dr -mxbpf
#name: xBPF ALU32 insns

.*: +file format .*bpf.*

Disassembly of section \.text:

0+ <\.text>:
   0:	e4 02 00 00 02 00 00 00 	sdiv32 %r2,2
   8:	e4 03 00 00 fd ff ff ff 	sdiv32 %r3,-3
  10:	e4 04 00 00 ef be ad 7e 	sdiv32 %r4,0x7eadbeef
  18:	ec 25 00 00 00 00 00 00 	sdiv32 %r5,%r2
  20:	f4 02 00 00 03 00 00 00 	smod32 %r2,3
  28:	f4 03 00 00 fc ff ff ff 	smod32 %r3,-4
  30:	f4 04 00 00 ef be ad 7e 	smod32 %r4,0x7eadbeef
  38:	fc 25 00 00 00 00 00 00 	smod32 %r5,%r2
