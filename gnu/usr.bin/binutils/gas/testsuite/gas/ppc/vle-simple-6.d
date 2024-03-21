#as: -a32 -mbig -mvle
#objdump: -dr -Mvle
#name: VLE Simplified mnemonics 6

.*: +file format elf.*-powerpc.*

Disassembly of section .text:

0+0 <.text>:
   0:	7c b1 9b a6 	mtmas1  r5
   4:	7c 3a 0b a6 	mtcsrr0 r1
   8:	7c 5b 0b a6 	mtcsrr1 r2
   c:	7c b0 62 a6 	mfivor0 r5
  10:	7c b1 62 a6 	mfivor1 r5
  14:	7c b2 62 a6 	mfivor2 r5
  18:	7c b3 62 a6 	mfivor3 r5
  1c:	7c b4 62 a6 	mfivor4 r5
  20:	7c b5 62 a6 	mfivor5 r5
  24:	7c b6 62 a6 	mfivor6 r5
  28:	7c b7 62 a6 	mfivor7 r5
  2c:	7c b8 62 a6 	mfivor8 r5
  30:	7c b9 62 a6 	mfivor9 r5
  34:	7c ba 62 a6 	mfivor10 r5
  38:	7c bb 62 a6 	mfivor11 r5
  3c:	7c bc 62 a6 	mfivor12 r5
  40:	7c bd 62 a6 	mfivor13 r5
  44:	7c be 62 a6 	mfivor14 r5
  48:	7c bf 62 a6 	mfivor15 r5
  4c:	7d 50 43 a6 	mtsprg  0,r10
  50:	7d 51 43 a6 	mtsprg  1,r10
  54:	7d 52 43 a6 	mtsprg  2,r10
  58:	7d 53 43 a6 	mtsprg  3,r10
  5c:	7d 54 43 a6 	mtsprg  4,r10
  60:	7d 55 43 a6 	mtsprg  5,r10
  64:	7d 56 43 a6 	mtsprg  6,r10
  68:	7d 57 43 a6 	mtsprg  7,r10
  6c:	7d 50 43 a6 	mtsprg  0,r10
  70:	7d 51 43 a6 	mtsprg  1,r10
  74:	7d 52 43 a6 	mtsprg  2,r10
  78:	7d 53 43 a6 	mtsprg  3,r10
  7c:	7d 54 43 a6 	mtsprg  4,r10
  80:	7d 55 43 a6 	mtsprg  5,r10
  84:	7d 56 43 a6 	mtsprg  6,r10
  88:	7d 57 43 a6 	mtsprg  7,r10
  8c:	7d 30 42 a6 	mfsprg  r9,0
  90:	7d 31 42 a6 	mfsprg  r9,1
  94:	7d 32 42 a6 	mfsprg  r9,2
  98:	7d 33 42 a6 	mfsprg  r9,3
  9c:	7d 24 42 a6 	mfsprg  r9,4
  a0:	7d 25 42 a6 	mfsprg  r9,5
  a4:	7d 26 42 a6 	mfsprg  r9,6
  a8:	7d 27 42 a6 	mfsprg  r9,7
  ac:	7d 30 42 a6 	mfsprg  r9,0
  b0:	7d 31 42 a6 	mfsprg  r9,1
  b4:	7d 32 42 a6 	mfsprg  r9,2
  b8:	7d 33 42 a6 	mfsprg  r9,3
  bc:	7d 24 42 a6 	mfsprg  r9,4
  c0:	7d 25 42 a6 	mfsprg  r9,5
  c4:	7d 26 42 a6 	mfsprg  r9,6
  c8:	7d 27 42 a6 	mfsprg  r9,7
