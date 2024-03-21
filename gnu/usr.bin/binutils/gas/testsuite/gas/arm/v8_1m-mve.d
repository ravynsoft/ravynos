# name: V8.1-m FP register instructions enabled by +mve
# as: -march=armv8.1-m.main+mve
# objdump: -dr --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*


Disassembly of section .text:

00000000 <\.text>:
 *[0-9a-f]+:	ec80 0b08 	vstmia	r0, {d0-d3}
 *[0-9a-f]+:	ecb7 3b04 	vldmia	r7!, {d3-d4}
 *[0-9a-f]+:	ecbd 0b06 	vpop	{d0-d2}
 *[0-9a-f]+:	ed2d 0b06 	vpush	{d0-d2}
 *[0-9a-f]+:	ecbd 2b08 	vpop	{d2-d5}
 *[0-9a-f]+:	ed2d 1b0c 	vpush	{d1-d6}
 *[0-9a-f]+:	fe71 0f4d 	vpst
 *[0-9a-f]+:	fd00 3e01 	vstrwt\.32	q1, \[q0, #-4\]
 *[0-9a-f]+:	ed82 2f80 	vstr	FPSCR, \[r2\]
 *[0-9a-f]+:	ed80 0b00 	vstr	d0, \[r0\]
 *[0-9a-f]+:	ed90 0b00 	vldr	d0, \[r0\]
 *[0-9a-f]+:	ed80 0a00 	vstr	s0, \[r0\]
 *[0-9a-f]+:	ed90 0a00 	vldr	s0, \[r0\]
 *[0-9a-f]+:	ed81 fb00 	vstr	d15, \[r1\]
 *[0-9a-f]+:	ed91 fb00 	vldr	d15, \[r1\]
 *[0-9a-f]+:	edc1 fa00 	vstr	s31, \[r1\]
 *[0-9a-f]+:	edd1 fa00 	vldr	s31, \[r1\]
 *[0-9a-f]+:	ed2d 0a20 	vpush	{s0-s31}
 *[0-9a-f]+:	ed2d 0a10 	vpush	{s0-s15}
 *[0-9a-f]+:	ecbd 0a10 	vpop	{s0-s15}
 *[0-9a-f]+:	ecbd 0a20 	vpop	{s0-s31}
