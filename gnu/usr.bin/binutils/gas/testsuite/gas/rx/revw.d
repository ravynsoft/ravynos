#source: ./revw.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 65 00                      	revw	r0, r0
   3:	fd 65 0f                      	revw	r0, r15
   6:	fd 65 f0                      	revw	r15, r0
   9:	fd 65 ff                      	revw	r15, r15
