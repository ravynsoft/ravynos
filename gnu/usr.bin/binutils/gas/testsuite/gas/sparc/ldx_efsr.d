#as: -Av9d
#objdump: -dr -m sparc:v9d
#name: sparc LDXEFSR

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	c7 08 c0 00 	ldx  \[ %g3 \], %efsr
