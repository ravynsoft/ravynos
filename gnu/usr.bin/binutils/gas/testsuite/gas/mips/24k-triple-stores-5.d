#objdump: -dr
#as: -mfix-24k -32
#name: 24K: Triple Store (Mix byte/half/word size check)

.*: +file format .*mips.*

Disassembly of section .text:
0+ <.*>:
   0:	a5020007 	sh	v0,7\(t0\)
   4:	a1030000 	sb	v1,0\(t0\)
   8:	ad040001 	sw	a0,1\(t0\)
   c:	0000000d 	break
  10:	a5020016 	sh	v0,22\(t0\)
  14:	a103000f 	sb	v1,15\(t0\)
  18:	00000000 	nop
  1c:	ad040018 	sw	a0,24\(t0\)
  20:	0000000d 	break
  24:	a5020000 	sh	v0,0\(t0\)
  28:	a1030009 	sb	v1,9\(t0\)
  2c:	ad040002 	sw	a0,2\(t0\)
  30:	0000000d 	break
  34:	a5020006 	sh	v0,6\(t0\)
  38:	a1030010 	sb	v1,16\(t0\)
  3c:	00000000 	nop
  40:	ad04000c 	sw	a0,12\(t0\)
  44:	0000000d 	break
  48:	a502000a 	sh	v0,10\(t0\)
  4c:	a103000f 	sb	v1,15\(t0\)
  50:	ad040004 	sw	a0,4\(t0\)
  54:	0000000d 	break
  58:	a502000a 	sh	v0,10\(t0\)
  5c:	a1030010 	sb	v1,16\(t0\)
  60:	00000000 	nop
  64:	ad040004 	sw	a0,4\(t0\)
  68:	0000000d 	break
	\.\.\.
