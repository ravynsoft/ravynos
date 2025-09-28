#as: --EB
#source: jump.s
#source: jump-pseudoc.s
#objdump: -dr
#name: eBPF JUMP instructions, big endian

.*: +file format .*bpf.*

Disassembly of section .text:

0+ <.text>:
   0:	05 00 00 03 00 00 00 00 	ja 3
   8:	0f 11 00 00 00 00 00 00 	add %r1,%r1
  10:	15 30 00 01 00 00 00 03 	jeq %r3,3,1
  18:	1d 34 00 00 00 00 00 00 	jeq %r3,%r4,0
  20:	35 30 ff fd 00 00 00 03 	jge %r3,3,-3
  28:	3d 34 ff fc 00 00 00 00 	jge %r3,%r4,-4
  30:	a5 30 00 01 00 00 00 03 	jlt %r3,3,1
  38:	ad 34 00 00 00 00 00 00 	jlt %r3,%r4,0
  40:	b5 30 00 01 00 00 00 03 	jle %r3,3,1
  48:	bd 34 00 00 00 00 00 00 	jle %r3,%r4,0
  50:	45 30 00 01 00 00 00 03 	jset %r3,3,1
  58:	4d 34 00 00 00 00 00 00 	jset %r3,%r4,0
  60:	55 30 00 01 00 00 00 03 	jne %r3,3,1
  68:	5d 34 00 00 00 00 00 00 	jne %r3,%r4,0
  70:	65 30 00 01 00 00 00 03 	jsgt %r3,3,1
  78:	6d 34 00 00 00 00 00 00 	jsgt %r3,%r4,0
  80:	75 30 00 01 00 00 00 03 	jsge %r3,3,1
  88:	7d 34 00 00 00 00 00 00 	jsge %r3,%r4,0
  90:	c5 30 00 01 00 00 00 03 	jslt %r3,3,1
  98:	cd 34 00 00 00 00 00 00 	jslt %r3,%r4,0
  a0:	d5 30 00 01 00 00 00 03 	jsle %r3,3,1
  a8:	dd 34 00 00 00 00 00 00 	jsle %r3,%r4,0
