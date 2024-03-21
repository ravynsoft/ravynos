#source: ./movco.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 27 00                      	movco	r0, \[r0\]
   3:	fd 27 f0                      	movco	r0, \[r15\]
   6:	fd 27 0f                      	movco	r15, \[r0\]
   9:	fd 27 ff                      	movco	r15, \[r15\]
