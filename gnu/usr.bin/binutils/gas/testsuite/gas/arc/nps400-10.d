#as: -mcpu=arc700 -mnps400
#objdump: -dr

.*: +file format .*arc.*

Disassembly of section \.text:

[0-9a-f]+ <.*>:
   0:	4846 0c21           	exc	r0,r0,\[xa:r2\]
   4:	4926 0c61           	exc	r1,r1,\[sd:r1\]
   8:	4a66 0c81           	exc	r2,r2,\[xd:r3\]
   c:	4b26 0c01           	exc	r3,r3,\[r1\]
  10:	4c96 4c21           	exc\.di\.f	r12,r12,\[xa:r12\]
  14:	4eb6 4c61           	exc\.di\.f	r14,r14,\[sd:r13\]
  18:	4dd6 4c81           	exc\.di\.f	r13,r13,\[xd:r14\]
  1c:	4ff6 4c01           	exc\.di\.f	r15,r15,\[r15\]
  20:	4c16 0c21           	exc\.f	r12,r12,\[xa:r0\]
  24:	4e36 0c61           	exc\.f	r14,r14,\[sd:r1\]
  28:	4d16 0c81           	exc\.f	r13,r13,\[xd:r0\]
  2c:	4f56 0c01           	exc\.f	r15,r15,\[r2\]
  30:	4c86 4c21           	exc\.di	r12,r12,\[xa:r12\]
  34:	4ec6 4c61           	exc\.di	r14,r14,\[sd:r14\]
  38:	4da6 4c81           	exc\.di	r13,r13,\[xd:r13\]
  3c:	4fe6 4c01           	exc\.di	r15,r15,\[r15\]
