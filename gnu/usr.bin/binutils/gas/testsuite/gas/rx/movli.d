#source: ./movli.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 2f 00                      	movli	\[r0\], r0
   3:	fd 2f 0f                      	movli	\[r0\], r15
   6:	fd 2f f0                      	movli	\[r15\], r0
   9:	fd 2f ff                      	movli	\[r15\], r15
