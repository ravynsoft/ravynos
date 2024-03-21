#as: --EL -mxbpf
#objdump: -dr -mxbpf
#name: xBPF ALU64 insns

.*: +file format .*bpf.*

Disassembly of section \.text:

0+ <\.text>:
   0:	e7 02 00 00 02 00 00 00 	sdiv %r2,2
   8:	e7 03 00 00 fd ff ff ff 	sdiv %r3,-3
  10:	e7 04 00 00 ef be ad 7e 	sdiv %r4,0x7eadbeef
  18:	ef 25 00 00 00 00 00 00 	sdiv %r5,%r2
  20:	f7 02 00 00 03 00 00 00 	smod %r2,3
  28:	f7 03 00 00 fc ff ff ff 	smod %r3,-4
  30:	f7 04 00 00 ef be ad 7e 	smod %r4,0x7eadbeef
  38:	ff 25 00 00 00 00 00 00 	smod %r5,%r2
