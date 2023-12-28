# name: Two register form of data processing instruction with register shifted register operand
# as:
# objdump: -dr

.*: +file format .*arm.*

Disassembly of section .text:

00000000 <.text>:
   0:	e0855014 	add	r5, r5, r4, lsl r0
   4:	e0855014 	add	r5, r5, r4, lsl r0
