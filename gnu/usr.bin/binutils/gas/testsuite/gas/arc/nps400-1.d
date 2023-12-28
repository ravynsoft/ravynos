#as: -mcpu=arc700 -mnps400
#objdump: -dr

.*: +file format .*arc.*

Disassembly of section .text:

[0-9a-f]+ <.*>:
   0:	4821 1485           	movb	r0,r0,r1,0x4,0x5,0x6
   4:	4881 1485           	movb	r0,r0,r12,0x4,0x5,0x6
   8:	4f81 1485           	movb	r15,r15,r12,0x4,0x5,0x6
   c:	4821 9485           	movb\.cl	r0,r1,0x4,0x5,0x6
  10:	48c1 9485           	movb\.cl	r0,r14,0x4,0x5,0x6
  14:	4d21 9485           	movb\.cl	r13,r1,0x4,0x5,0x6
  18:	4808 04d2           	movh	r0,r0,0x4d2
  1c:	4868 ffff           	movh	r3,r3,0xffff
  20:	4818 04d2           	movh\.cl	r0,0x4d2
  24:	4878 ffff           	movh\.cl	r3,0xffff
  28:	49cf 0906           	movbi	r14,r14,0x6,0x8,0x4
  2c:	4aff 0174           	movbi\.f	r23,r23,0x14,0xb,0x1
  30:	4bcf 864a           	movbi\.cl	r30,0xa,0x12,0x2
  34:	48df 8c09           	movbi\.f\.cl	r6,0x9,0,0x8
  38:	4843 a845           	decode1	r0,r0,r2,0x5,0xb
  3c:	4853 a845           	decode1\.f	r0,r0,r2,0x5,0xb
  40:	4843 d06b           	decode1\.cl	r0,r2,0xb
  44:	4853 b472           	decode1\.cl\.f	r0,r2,0x12
  48:	4963 b803           	fbset	r1,r1,r3,0x3,0xf
  4c:	4973 b803           	fbset\.f	r1,r1,r3,0x3,0xf
  50:	4a83 3803           	fbclr	r2,r2,r12,0x3,0xf
  54:	4b93 3803           	fbclr\.f	r3,r3,r12,0x3,0xf
  58:	4a24 0012           	encode0	r2,r1,0x12,0x1
  5c:	4814 7c00           	encode0\.f	r0,r0,0,0x20
  60:	4a24 801f           	encode1	r2,r1,0x1f,0x1
  64:	4814 fc00           	encode1\.f	r0,r0,0,0x20
  68:	3c2e 150a           	rflt	r10,r12,r20
  6c:	3e2e 7500 1234 5678 	rflt	r0,0x12345678,r20
  74:	3f2e 0f86 ffff ffff 	rflt	r6,r7,0xffffffff
  7c:	3e2e 7f88 ffff ffff 	rflt	r8,0xffffffff,0xffffffff
  84:	3e2e 137e           	rflt	0,r14,r13
  88:	3e2e 72be ffff ffff 	rflt	0,0xffffffff,r10
  90:	3c2e 1fbe ffff ffff 	rflt	0,r12,0xffffffff
  98:	3d6e 0044           	rflt	r4,r5,0x1
  9c:	3e6e 7083 1234 5678 	rflt	r3,0x12345678,0x2
  a4:	396e 013e           	rflt	0,r1,0x4
  a8:	3e6e 707e ffff ffff 	rflt	0,0xffffffff,0x1
  b0:	3a33 00c1           	crc16	r1,r2,r3
  b4:	3e33 7144 ffff ffff 	crc16	r4,0xffffffff,r5
  bc:	3f33 0f86 ffff ffff 	crc16	r6,r7,0xffffffff
  c4:	3e33 7f88 ffff ffff 	crc16	r8,0xffffffff,0xffffffff
  cc:	3933 12be           	crc16	0,r9,r10
  d0:	3e33 72fe ffff ffff 	crc16	0,0xffffffff,r11
  d8:	3c33 1fbe ffff ffff 	crc16	0,r12,0xffffffff
  e0:	3e73 1fcd           	crc16	r13,r14,0x3f
  e4:	3e73 7fcf ffff ffff 	crc16	r15,0xffffffff,0x3f
  ec:	3873 2ffe           	crc16	0,r16,0x3f
  f0:	3e73 7ffe ffff ffff 	crc16	0,0xffffffff,0x3f
  f8:	3a33 80c1           	crc16.r	r1,r2,r3
  fc:	3e33 f144 ffff ffff 	crc16.r	r4,0xffffffff,r5
 104:	3f33 8f86 ffff ffff 	crc16.r	r6,r7,0xffffffff
 10c:	3e33 ff88 ffff ffff 	crc16.r	r8,0xffffffff,0xffffffff
 114:	3933 92be           	crc16.r	0,r9,r10
 118:	3e33 f2fe ffff ffff 	crc16.r	0,0xffffffff,r11
 120:	3c33 9fbe ffff ffff 	crc16.r	0,r12,0xffffffff
 128:	3e73 9fcd           	crc16.r	r13,r14,0x3f
 12c:	3e73 ffcf ffff ffff 	crc16.r	r15,0xffffffff,0x3f
 134:	3873 affe           	crc16.r	0,r16,0x3f
 138:	3e73 fffe ffff ffff 	crc16.r	0,0xffffffff,0x3f
 140:	3a34 00c1           	crc32	r1,r2,r3
 144:	3e34 7144 ffff ffff 	crc32	r4,0xffffffff,r5
 14c:	3f34 0f86 ffff ffff 	crc32	r6,r7,0xffffffff
 154:	3e34 7f88 ffff ffff 	crc32	r8,0xffffffff,0xffffffff
 15c:	3934 12be           	crc32	0,r9,r10
 160:	3e34 72fe ffff ffff 	crc32	0,0xffffffff,r11
 168:	3c34 1fbe ffff ffff 	crc32	0,r12,0xffffffff
 170:	3e74 1fcd           	crc32	r13,r14,0x3f
 174:	3e74 7fcf ffff ffff 	crc32	r15,0xffffffff,0x3f
 17c:	3874 2ffe           	crc32	0,r16,0x3f
 180:	3e74 7ffe ffff ffff 	crc32	0,0xffffffff,0x3f
 188:	3a34 80c1           	crc32.r	r1,r2,r3
 18c:	3e34 f144 ffff ffff 	crc32.r	r4,0xffffffff,r5
 194:	3f34 8f86 ffff ffff 	crc32.r	r6,r7,0xffffffff
 19c:	3e34 ff88 ffff ffff 	crc32.r	r8,0xffffffff,0xffffffff
 1a4:	3934 92be           	crc32.r	0,r9,r10
 1a8:	3e34 f2fe ffff ffff 	crc32.r	0,0xffffffff,r11
 1b0:	3c34 9fbe ffff ffff 	crc32.r	0,r12,0xffffffff
 1b8:	3e74 9fcd           	crc32.r	r13,r14,0x3f
 1bc:	3e74 ffcf ffff ffff 	crc32.r	r15,0xffffffff,0x3f
 1c4:	3874 affe           	crc32.r	0,r16,0x3f
 1c8:	3e74 fffe ffff ffff 	crc32.r	0,0xffffffff,0x3f
