#as: -little
#objdump: -drj.text
#name: Sign-extended immediate

.*:     file format .*sh.*

Disassembly of section \.text:

00000000 <foo>:
   0:	f0 e0       	mov	#-16,r0
   2:	09 00       	nop	
