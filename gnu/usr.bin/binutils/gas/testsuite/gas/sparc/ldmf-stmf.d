#as: -Av9m8
#objdump: -dr
#name: sparc LDMF/STMF/LDMFA/STMFA

.*: +file format .*

Disassembly of section .text:

0+ <.text>:
   0:	c3 88 18 01 	ldmfs  \[ %g0 \+ %g1 \], %f1
   4:	c3 88 58 00 	ldmfs  \[ %g1 \], %f1
   8:	c3 88 78 66 	ldmfs  \[ %g1 \+ 0x66 \], %f1
   c:	c3 88 78 66 	ldmfs  \[ %g1 \+ 0x66 \], %f1
  10:	c3 88 38 66 	ldmfs  \[ 0x66 \], %f1
  14:	c3 88 1c 01 	ldmfd  \[ %g0 \+ %g1 \], %f32
  18:	c3 88 5c 00 	ldmfd  \[ %g1 \], %f32
  1c:	c3 88 7c 66 	ldmfd  \[ %g1 \+ 0x66 \], %f32
  20:	c3 88 7c 66 	ldmfd  \[ %g1 \+ 0x66 \], %f32
  24:	c3 88 3c 66 	ldmfd  \[ 0x66 \], %f32
  28:	c3 88 1a 01 	ldmfsa  \[ %g0 \+ %g1 \] %asi, %f1
  2c:	c3 88 5a 00 	ldmfsa  \[ %g1 \] %asi, %f1
  30:	c3 88 1e 01 	ldmfda  \[ %g0 \+ %g1 \] %asi, %f32
  34:	c3 88 5e 00 	ldmfda  \[ %g1 \] %asi, %f32
  38:	c3 a8 18 01 	stmfs  %f1, \[ %g0 \+ %g1 \]
  3c:	c3 a8 58 00 	stmfs  %f1, \[ %g1 \]
  40:	c3 a8 78 66 	stmfs  %f1, \[ %g1 \+ 0x66 \]
  44:	c3 a8 78 66 	stmfs  %f1, \[ %g1 \+ 0x66 \]
  48:	c3 a8 38 66 	stmfs  %f1, \[ 0x66 \]
  4c:	c3 a8 1c 01 	stmfd  %f32, \[ %g0 \+ %g1 \]
  50:	c3 a8 5c 00 	stmfd  %f32, \[ %g1 \]
  54:	c3 a8 7c 66 	stmfd  %f32, \[ %g1 \+ 0x66 \]
  58:	c3 a8 7c 66 	stmfd  %f32, \[ %g1 \+ 0x66 \]
  5c:	c3 a8 3c 66 	stmfd  %f32, \[ 0x66 \]
  60:	c3 a8 1a 01 	stmfsa  %f1, \[ %g0 \+ %g1 \] %asi
  64:	c3 a8 5a 00 	stmfsa  %f1, \[ %g1 \] %asi
  68:	c3 a8 1e 01 	stmfda  %f32, \[ %g0 \+ %g1 \] %asi
  6c:	c3 a8 5e 00 	stmfda  %f32, \[ %g1 \] %asi
