#as: -march=armv8.3-a+fp16+simd
#objdump: -dr
#skip: *-*-pe *-wince-*

.*: +file format .*arm.*

Disassembly of section .text:

[0-9a-f]+ <.*>:
 +[0-9a-f]+:	fc942846 	vcadd.f32	q1, q2, q3, #90
 +[0-9a-f]+:	fd942846 	vcadd.f32	q1, q2, q3, #270
 +[0-9a-f]+:	fcc658a7 	vcadd.f16	d21, d22, d23, #90
 +[0-9a-f]+:	fc842846 	vcadd.f16	q1, q2, q3, #90
 +[0-9a-f]+:	fcd658a7 	vcadd.f32	d21, d22, d23, #90
 +[0-9a-f]+:	fc342846 	vcmla.f32	q1, q2, q3, #0
 +[0-9a-f]+:	fcb42846 	vcmla.f32	q1, q2, q3, #90
 +[0-9a-f]+:	fd342846 	vcmla.f32	q1, q2, q3, #180
 +[0-9a-f]+:	fdb42846 	vcmla.f32	q1, q2, q3, #270
 +[0-9a-f]+:	fce658a7 	vcmla.f16	d21, d22, d23, #90
 +[0-9a-f]+:	fca42846 	vcmla.f16	q1, q2, q3, #90
 +[0-9a-f]+:	fcf658a7 	vcmla.f32	d21, d22, d23, #90
 +[0-9a-f]+:	fe565883 	vcmla.f16	d21, d22, d3\[0\], #90
 +[0-9a-f]+:	fe5658a3 	vcmla.f16	d21, d22, d3\[1\], #90
 +[0-9a-f]+:	fe142843 	vcmla.f16	q1, q2, d3\[0\], #90
 +[0-9a-f]+:	fe142863 	vcmla.f16	q1, q2, d3\[1\], #90
 +[0-9a-f]+:	fed658a7 	vcmla.f32	d21, d22, d23\[0\], #90
 +[0-9a-f]+:	fe942867 	vcmla.f32	q1, q2, d23\[0\], #90
 +[0-9a-f]+:	fe042863 	vcmla.f16	q1, q2, d3\[1\], #0
 +[0-9a-f]+:	fe242863 	vcmla.f16	q1, q2, d3\[1\], #180
 +[0-9a-f]+:	fe342863 	vcmla.f16	q1, q2, d3\[1\], #270
 +[0-9a-f]+:	fe842843 	vcmla.f32	q1, q2, d3\[0\], #0
 +[0-9a-f]+:	fea42843 	vcmla.f32	q1, q2, d3\[0\], #180
 +[0-9a-f]+:	feb42843 	vcmla.f32	q1, q2, d3\[0\], #270

[0-9a-f]+ <.*>:
 +[0-9a-f]+:	fc94 2846 	vcadd.f32	q1, q2, q3, #90
 +[0-9a-f]+:	fd94 2846 	vcadd.f32	q1, q2, q3, #270
 +[0-9a-f]+:	fcc6 58a7 	vcadd.f16	d21, d22, d23, #90
 +[0-9a-f]+:	fc84 2846 	vcadd.f16	q1, q2, q3, #90
 +[0-9a-f]+:	fcd6 58a7 	vcadd.f32	d21, d22, d23, #90
 +[0-9a-f]+:	fc34 2846 	vcmla.f32	q1, q2, q3, #0
 +[0-9a-f]+:	fcb4 2846 	vcmla.f32	q1, q2, q3, #90
 +[0-9a-f]+:	fd34 2846 	vcmla.f32	q1, q2, q3, #180
 +[0-9a-f]+:	fdb4 2846 	vcmla.f32	q1, q2, q3, #270
 +[0-9a-f]+:	fce6 58a7 	vcmla.f16	d21, d22, d23, #90
 +[0-9a-f]+:	fca4 2846 	vcmla.f16	q1, q2, q3, #90
 +[0-9a-f]+:	fcf6 58a7 	vcmla.f32	d21, d22, d23, #90
 +[0-9a-f]+:	fe56 5883 	vcmla.f16	d21, d22, d3\[0\], #90
 +[0-9a-f]+:	fe56 58a3 	vcmla.f16	d21, d22, d3\[1\], #90
 +[0-9a-f]+:	fe14 2843 	vcmla.f16	q1, q2, d3\[0\], #90
 +[0-9a-f]+:	fe14 2863 	vcmla.f16	q1, q2, d3\[1\], #90
 +[0-9a-f]+:	fed6 58a7 	vcmla.f32	d21, d22, d23\[0\], #90
 +[0-9a-f]+:	fe94 2867 	vcmla.f32	q1, q2, d23\[0\], #90
 +[0-9a-f]+:	fe04 2863 	vcmla.f16	q1, q2, d3\[1\], #0
 +[0-9a-f]+:	fe24 2863 	vcmla.f16	q1, q2, d3\[1\], #180
 +[0-9a-f]+:	fe34 2863 	vcmla.f16	q1, q2, d3\[1\], #270
 +[0-9a-f]+:	fe84 2843 	vcmla.f32	q1, q2, d3\[0\], #0
 +[0-9a-f]+:	fea4 2843 	vcmla.f32	q1, q2, d3\[0\], #180
 +[0-9a-f]+:	feb4 2843 	vcmla.f32	q1, q2, d3\[0\], #270
