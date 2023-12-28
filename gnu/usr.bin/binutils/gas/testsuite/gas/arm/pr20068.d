# name: PR20068 - Misaligned constant pool when running GAS on a 32-bit host.
# as: -mfpu=vfpv3
# objdump: -S

.*:     file format .*


Disassembly of section .text:

00000000 <main>:
   0:	e59f0008 	ldr	r0, \[pc, #8\].*
   4:	ed9f9b03 	vldr	d9, \[pc, #12\].*
   8:	e1a0f00e 	mov	pc, lr
   c:	00000000 	.*
  10:	12345678 	.*
  14:	00000000 	.*
  18:	(00000fff|0000fff0).*
  1c:	(0000fff0|00000fff).*
