#PROG: objcopy
#objdump: -dr
#name: Disassembler detects unallocated instruction encodings.

.*: +file format .*aarch64.*

Disassembly of section \.text:

0000000000000000 <.*>:
   0:	0d0047de 	.inst	0x0d0047de ; undefined
   4:	0d2047dd 	.inst	0x0d2047dd ; undefined
   8:	0d0067dc 	.inst	0x0d0067dc ; undefined
   c:	0d2067db 	.inst	0x0d2067db ; undefined
  10:	0d008bde 	.inst	0x0d008bde ; undefined
  14:	0d208bdd 	.inst	0x0d208bdd ; undefined
  18:	0d00abdc 	.inst	0x0d00abdc ; undefined
  1c:	0d20abdb 	.inst	0x0d20abdb ; undefined
  20:	0d008fde 	.inst	0x0d008fde ; undefined
  24:	0d208fdd 	.inst	0x0d208fdd ; undefined
  28:	0d00afdc 	.inst	0x0d00afdc ; undefined
  2c:	0d20afdb 	.inst	0x0d20afdb ; undefined
  30:	0d0097de 	.inst	0x0d0097de ; undefined
  34:	0d2097dd 	.inst	0x0d2097dd ; undefined
  38:	0d00b7dc 	.inst	0x0d00b7dc ; undefined
  3c:	0d20b7db 	.inst	0x0d20b7db ; undefined
  40:	0d009fde 	.inst	0x0d009fde ; undefined
  44:	0d209fdd 	.inst	0x0d209fdd ; undefined
  48:	0d00bfdc 	.inst	0x0d00bfdc ; undefined
  4c:	0d20bfdb 	.inst	0x0d20bfdb ; undefined
