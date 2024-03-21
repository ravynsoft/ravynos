#source: b-twoinsn.s
#source: b-offlocmis.s
#source: b-post1.s
#source: b-goodmain.s
#ld: --oformat binary
#objdump: -sh

# Check that a LOP_LOC at a misaligned location followed by a
# LOP_QUOTE hits the corresponding aligned address.  This is a
# variant of sec-5.d with the lop_loc having a misalignment, followed
# by another misaligned lop_loc with a lop_quot.

.*:     file format mmo

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 \.text         0+8  0+  0+  0+  2\*\*2
                  CONTENTS, ALLOC, LOAD, CODE
  1 \.MMIX\.sec\.0   0+24  789abcdef0123458  789abcdef0123458  0+  2\*\*2
                  CONTENTS, ALLOC, LOAD
Contents of section \.text:
 0+ e3fd0001 e3fd0004                  .*
Contents of section \.MMIX\.sec\.0:
 789abcdef0123458 b045197d 2c1b03b2 e4dbf877 0fc766fb  .*
 789abcdef0123468 00000000 00000000 00000000 00000000  .*
 789abcdef0123478 12345677  .*
