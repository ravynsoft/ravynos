#as: -mcpu=arc700 -mnps400
#objdump: -dr

.*: +file format .*arc.*

Disassembly of section .text:

[0-9a-f]+ <.*>:
   0:	5000 07c0 2400 0000 	dcmac	r0,\[cm:r0\],\[cm:r0\],r0
   8:	5044 3fc0 2400 0000 	dcmac	r2,\[cm:r4\],\[cm:r7\],r7
  10:	53ff ffc0 2400 0000 	dcmac	blink,\[cm:blink\],\[cm:blink\],blink
  18:	5000 07c0 2600 0000 	dcmac	r0,\[cm:r0\],\[cm:0\],r0
  20:	5044 3fc0 2600 1234 	dcmac	r2,\[cm:r4\],\[cm:0x1234\],r7
  28:	53ff ffc0 2600 ffff 	dcmac	blink,\[cm:blink\],\[cm:0xffff\],blink
  30:	5000 07c0 2700 0000 	dcmac	r0,\[cm:0\],\[cm:r0\],r0
  38:	5044 3fc0 2700 4321 	dcmac	r2,\[cm:0x4321\],\[cm:r4\],r7
  40:	53ff ffc0 2700 ffff 	dcmac	blink,\[cm:0xffff\],\[cm:blink\],blink
  48:	57c0 07c0 2400 0000 	dcmac	0,\[cm:r0\],\[cm:r0\],r0
  50:	57c4 3fc0 2400 0000 	dcmac	0,\[cm:r4\],\[cm:r7\],r7
  58:	57df ffc0 2400 0000 	dcmac	0,\[cm:blink\],\[cm:blink\],blink
  60:	57c0 07c0 2600 0000 	dcmac	0,\[cm:r0\],\[cm:0\],r0
  68:	57c4 3fc0 2600 1234 	dcmac	0,\[cm:r4\],\[cm:0x1234\],r7
  70:	57df ffc0 2600 ffff 	dcmac	0,\[cm:blink\],\[cm:0xffff\],blink
  78:	57c0 07c0 2700 0000 	dcmac	0,\[cm:0\],\[cm:r0\],r0
  80:	57c4 3fc0 2700 4321 	dcmac	0,\[cm:0x4321\],\[cm:r4\],r7
  88:	57df ffc0 2700 ffff 	dcmac	0,\[cm:0xffff\],\[cm:blink\],blink
  90:	5000 07c0 2001 0000 	dcmac	r0,\[cm:r0\],\[cm:r0\],0x1
  98:	5044 27c0 200f 0000 	dcmac	r2,\[cm:r4\],\[cm:r4\],0xf
  a0:	53ff ffc0 203f 0000 	dcmac	blink,\[cm:blink\],\[cm:blink\],0x3f
  a8:	5000 07c0 2201 0000 	dcmac	r0,\[cm:r0\],\[cm:0\],0x1
  b0:	5044 27c0 220f 1234 	dcmac	r2,\[cm:r4\],\[cm:0x1234\],0xf
  b8:	53ff ffc0 223f ffff 	dcmac	blink,\[cm:blink\],\[cm:0xffff\],0x3f
  c0:	5000 07c0 2301 0000 	dcmac	r0,\[cm:0\],\[cm:r0\],0x1
  c8:	5044 27c0 230f 4321 	dcmac	r2,\[cm:0x4321\],\[cm:r4\],0xf
  d0:	53ff ffc0 233f ffff 	dcmac	blink,\[cm:0xffff\],\[cm:blink\],0x3f
  d8:	57c0 07c0 2001 0000 	dcmac	0,\[cm:r0\],\[cm:r0\],0x1
  e0:	57c4 27c0 200f 0000 	dcmac	0,\[cm:r4\],\[cm:r4\],0xf
  e8:	57df ffc0 2000 0000 	dcmac	0,\[cm:blink\],\[cm:blink\],0x40
  f0:	57c0 07c0 2201 0000 	dcmac	0,\[cm:r0\],\[cm:0\],0x1
  f8:	57c4 27c0 220f 1234 	dcmac	0,\[cm:r4\],\[cm:0x1234\],0xf
 100:	57df ffc0 2200 ffff 	dcmac	0,\[cm:blink\],\[cm:0xffff\],0x40
 108:	57c0 07c0 2301 0000 	dcmac	0,\[cm:0\],\[cm:r0\],0x1
 110:	57c4 27c0 230f 4321 	dcmac	0,\[cm:0x4321\],\[cm:r4\],0xf
 118:	57df ffc0 2300 ffff 	dcmac	0,\[cm:0xffff\],\[cm:blink\],0x40
