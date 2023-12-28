#as: -Av9m8
#objdump: -dr
#name: sparc LDM/STM/LDMA/STMA

.*: +file format .*

Disassembly of section .text:

0+ <.text>:
   0:	c5 88 00 01 	ldmsh  \[ %g0 \+ %g1 \], %g2
   4:	c7 88 40 00 	ldmsh  \[ %g1 \], %g3
   8:	c7 88 60 66 	ldmsh  \[ %g1 \+ 0x66 \], %g3
   c:	c7 88 60 66 	ldmsh  \[ %g1 \+ 0x66 \], %g3
  10:	c7 88 20 66 	ldmsh  \[ 0x66 \], %g3
  14:	c5 88 04 01 	ldmuh  \[ %g0 \+ %g1 \], %g2
  18:	c7 88 44 00 	ldmuh  \[ %g1 \], %g3
  1c:	c7 88 64 66 	ldmuh  \[ %g1 \+ 0x66 \], %g3
  20:	c7 88 64 66 	ldmuh  \[ %g1 \+ 0x66 \], %g3
  24:	c7 88 24 66 	ldmuh  \[ 0x66 \], %g3
  28:	c5 88 08 01 	ldmsw  \[ %g0 \+ %g1 \], %g2
  2c:	c7 88 48 00 	ldmsw  \[ %g1 \], %g3
  30:	c7 88 68 66 	ldmsw  \[ %g1 \+ 0x66 \], %g3
  34:	c7 88 68 66 	ldmsw  \[ %g1 \+ 0x66 \], %g3
  38:	c7 88 28 66 	ldmsw  \[ 0x66 \], %g3
  3c:	c5 88 0c 01 	ldmuw  \[ %g0 \+ %g1 \], %g2
  40:	c7 88 4c 00 	ldmuw  \[ %g1 \], %g3
  44:	c7 88 6c 66 	ldmuw  \[ %g1 \+ 0x66 \], %g3
  48:	c7 88 6c 66 	ldmuw  \[ %g1 \+ 0x66 \], %g3
  4c:	c7 88 2c 66 	ldmuw  \[ 0x66 \], %g3
  50:	c5 88 14 01 	ldmx  \[ %g0 \+ %g1 \], %g2
  54:	c7 88 54 00 	ldmx  \[ %g1 \], %g3
  58:	c7 88 74 66 	ldmx  \[ %g1 \+ 0x66 \], %g3
  5c:	c7 88 74 66 	ldmx  \[ %g1 \+ 0x66 \], %g3
  60:	c7 88 34 66 	ldmx  \[ 0x66 \], %g3
  64:	c5 88 14 01 	ldmx  \[ %g0 \+ %g1 \], %g2
  68:	c7 88 54 00 	ldmx  \[ %g1 \], %g3
  6c:	c7 88 74 66 	ldmx  \[ %g1 \+ 0x66 \], %g3
  70:	c7 88 74 66 	ldmx  \[ %g1 \+ 0x66 \], %g3
  74:	c7 88 34 66 	ldmx  \[ 0x66 \], %g3
  78:	c7 88 42 02 	ldmsha  \[ %g1 \+ %g2 \] %asi, %g3
  7c:	c5 88 42 00 	ldmsha  \[ %g1 \] %asi, %g2
  80:	c7 88 46 02 	ldmuha  \[ %g1 \+ %g2 \] %asi, %g3
  84:	c5 88 46 00 	ldmuha  \[ %g1 \] %asi, %g2
  88:	c7 88 4a 02 	ldmswa  \[ %g1 \+ %g2 \] %asi, %g3
  8c:	c5 88 4a 00 	ldmswa  \[ %g1 \] %asi, %g2
  90:	c7 88 4e 02 	ldmuwa  \[ %g1 \+ %g2 \] %asi, %g3
  94:	c5 88 4e 00 	ldmuwa  \[ %g1 \] %asi, %g2
  98:	c7 88 56 02 	ldmxa  \[ %g1 \+ %g2 \] %asi, %g3
  9c:	c5 88 56 00 	ldmxa  \[ %g1 \] %asi, %g2
  a0:	c5 a8 04 01 	stmh  %g2, \[ %g0 \+ %g1 \]
  a4:	c7 a8 44 00 	stmh  %g3, \[ %g1 \]
  a8:	c7 a8 64 66 	stmh  %g3, \[ %g1 \+ 0x66 \]
  ac:	c7 a8 64 66 	stmh  %g3, \[ %g1 \+ 0x66 \]
  b0:	c7 a8 24 66 	stmh  %g3, \[ 0x66 \]
  b4:	c5 a8 0c 01 	stmw  %g2, \[ %g0 \+ %g1 \]
  b8:	c7 a8 4c 00 	stmw  %g3, \[ %g1 \]
  bc:	c7 a8 6c 66 	stmw  %g3, \[ %g1 \+ 0x66 \]
  c0:	c7 a8 6c 66 	stmw  %g3, \[ %g1 \+ 0x66 \]
  c4:	c7 a8 2c 66 	stmw  %g3, \[ 0x66 \]
  c8:	c5 a8 14 01 	stmx  %g2, \[ %g0 \+ %g1 \]
  cc:	c7 a8 54 00 	stmx  %g3, \[ %g1 \]
  d0:	c7 a8 74 66 	stmx  %g3, \[ %g1 \+ 0x66 \]
  d4:	c7 a8 74 66 	stmx  %g3, \[ %g1 \+ 0x66 \]
  d8:	c7 a8 34 66 	stmx  %g3, \[ 0x66 \]
  dc:	c5 a8 06 01 	stmha  %g2, \[ %g0 \+ %g1 \] %asi
  e0:	c7 a8 46 00 	stmha  %g3, \[ %g1 \] %asi
  e4:	c5 a8 0e 01 	stmwa  %g2, \[ %g0 \+ %g1 \] %asi
  e8:	c7 a8 4e 00 	stmwa  %g3, \[ %g1 \] %asi
  ec:	c5 a8 16 01 	stmxa  %g2, \[ %g0 \+ %g1 \] %asi
  f0:	c7 a8 56 00 	stmxa  %g3, \[ %g1 \] %asi
