#objdump: -d
#name:    The TFR (incl ZEX) instruction
#source:  tfr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <L1>:
   0:	9e 45       	tfr d0, d1
   2:	9e 58       	tfr d1, x
   4:	9e 0a       	tfr d2, s
   6:	9e 0d       	tfr d2, ccl
   8:	9e 6e       	tfr d6, ccw
   a:	9e 29       	tfr d4, y
   c:	9e 37       	tfr d5, d7
   e:	9e 5e       	tfr d1, ccw
  10:	9e 40       	tfr d0, d2
