#name: Int8 Matrix Multiply extension
#source: i8mm.s
#as: -mno-warn-deprecated -march=armv8.6-a+i8mm+simd -I$srcdir/$subdir
#objdump: -dr --show-raw-insn

.*: +file format .*arm.*

Disassembly of section \.text:

00000000 <\.text>:
 *[0-9a-f]+:	fcea4c40 	vusmmla\.s8	q10, q5, q0
 *[0-9a-f]+:	fc6a4c50 	vummla\.u8	q10, q5, q0
 *[0-9a-f]+:	fc6a4c40 	vsmmla\.s8	q10, q5, q0
 *[0-9a-f]+:	fcea4d40 	vusdot\.s8	q10, q5, q0
 *[0-9a-f]+:	feca4d50 	vsudot\.u8	q10, q5, d0\[0\]
 *[0-9a-f]+:	feca4d70 	vsudot\.u8	q10, q5, d0\[1\]
 *[0-9a-f]+:	feca4d40 	vusdot\.s8	q10, q5, d0\[0\]
 *[0-9a-f]+:	feca4d60 	vusdot\.s8	q10, q5, d0\[1\]
 *[0-9a-f]+:	fca5ad00 	vusdot\.s8	d10, d5, d0
 *[0-9a-f]+:	fe85ad00 	vusdot\.s8	d10, d5, d0\[0\]
 *[0-9a-f]+:	fe85ad20 	vusdot\.s8	d10, d5, d0\[1\]
 *[0-9a-f]+:	fe85ad10 	vsudot\.u8	d10, d5, d0\[0\]
 *[0-9a-f]+:	fe85ad30 	vsudot\.u8	d10, d5, d0\[1\]
 *[0-9a-f]+:	fcea4c40 	vusmmla\.s8	q10, q5, q0
 *[0-9a-f]+:	fc6a4c50 	vummla\.u8	q10, q5, q0
 *[0-9a-f]+:	fc6a4c40 	vsmmla\.s8	q10, q5, q0
 *[0-9a-f]+:	fcea4d40 	vusdot\.s8	q10, q5, q0
 *[0-9a-f]+:	feca4d50 	vsudot\.u8	q10, q5, d0\[0\]
 *[0-9a-f]+:	feca4d70 	vsudot\.u8	q10, q5, d0\[1\]
 *[0-9a-f]+:	feca4d40 	vusdot\.s8	q10, q5, d0\[0\]
 *[0-9a-f]+:	feca4d60 	vusdot\.s8	q10, q5, d0\[1\]
 *[0-9a-f]+:	fca5ad00 	vusdot\.s8	d10, d5, d0
 *[0-9a-f]+:	fe85ad00 	vusdot\.s8	d10, d5, d0\[0\]
 *[0-9a-f]+:	fe85ad20 	vusdot\.s8	d10, d5, d0\[1\]
 *[0-9a-f]+:	fe85ad10 	vsudot\.u8	d10, d5, d0\[0\]
 *[0-9a-f]+:	fe85ad30 	vsudot\.u8	d10, d5, d0\[1\]
