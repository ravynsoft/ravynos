#as: -march=armv8.3-a+simd
#source: armv8_3-a-fp.s
#objdump: -dr
#skip: *-*-pe *-wince-*

.*: +file format .*arm.*

Disassembly of section .text:

[0-9a-f]+ <.*>:
   [0-9a-f]+:	eef90bc7 	vjcvt.s32.f64	s1, d7
   [0-9a-f]+:	eef90bc7 	vjcvt.s32.f64	s1, d7

[0-9a-f]+ <.*>:
   [0-9a-f]+:	eef9 0bc7 	vjcvt.s32.f64	s1, d7

