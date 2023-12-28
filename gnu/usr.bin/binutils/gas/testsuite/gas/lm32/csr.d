#as:
#objdump: -d
#name: csr

.*:     file format elf32-lm32.*

Disassembly of section .text:

00000000 <.text>:
   0:	d0 00 00 00 	wcsr IE,r0
   4:	d0 1f 00 00 	wcsr IE,ba
   8:	d0 20 00 00 	wcsr IM,r0
   c:	d0 3f 00 00 	wcsr IM,ba
  10:	d0 60 00 00 	wcsr ICC,r0
  14:	d0 7f 00 00 	wcsr ICC,ba
  18:	d0 80 00 00 	wcsr DCC,r0
  1c:	d0 9f 00 00 	wcsr DCC,ba
  20:	90 00 00 00 	rcsr r0,IE
  24:	90 00 f8 00 	rcsr ba,IE
  28:	90 20 00 00 	rcsr r0,IM
  2c:	90 20 f8 00 	rcsr ba,IM
  30:	90 40 00 00 	rcsr r0,IP
  34:	90 40 f8 00 	rcsr ba,IP
  38:	90 a0 00 00 	rcsr r0,CC
  3c:	90 a0 f8 00 	rcsr ba,CC
  40:	90 c0 00 00 	rcsr r0,CFG
  44:	90 c0 f8 00 	rcsr ba,CFG
