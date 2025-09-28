#name: Bfloat 16 extension Thumb
#source: bfloat16.s
#as: -mno-warn-deprecated --defsym COMPILING_FOR_THUMB=1 -mthumb -march=armv8.6-a+simd -I$srcdir/$subdir
#objdump: -dr --show-raw-insn
#skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:

00000000 <\.text>:
 *[0-9a-f]+:	fc04 0d8b 	vdot\.bf16	d0, d20, d11
 *[0-9a-f]+:	fc00 bd24 	vdot\.bf16	d11, d0, d20
 *[0-9a-f]+:	eeb3 09c0 	vcvtt\.bf16\.f32	s0, s0
 *[0-9a-f]+:	eeb3 09c0 	vcvtt\.bf16\.f32	s0, s0
 *[0-9a-f]+:	eeb3 09c0 	vcvtt\.bf16\.f32	s0, s0
 *[0-9a-f]+:	eeb3 0940 	vcvtb\.bf16\.f32	s0, s0
 *[0-9a-f]+:	eeb3 0940 	vcvtb\.bf16\.f32	s0, s0
 *[0-9a-f]+:	eeb3 0940 	vcvtb\.bf16\.f32	s0, s0
 *[0-9a-f]+:	ffb6 0640 	vcvt\.bf16\.f32	d0, q0
 *[0-9a-f]+:	ffb6 0640 	vcvt\.bf16\.f32	d0, q0
 *[0-9a-f]+:	ffb6 0640 	vcvt\.bf16\.f32	d0, q0
 *[0-9a-f]+:	fe00 bd24 	vdot\.bf16	d11, d0, d4\[1\]
 *[0-9a-f]+:	fe04 0d8b 	vdot\.bf16	d0, d20, d11\[0\]
 *[0-9a-f]+:	fc4a 4c40 	vmmla\.bf16	q10, q5, q0
 *[0-9a-f]+:	fc00 ac64 	vmmla\.bf16	q5, q0, q10
 *[0-9a-f]+:	fc76 48d0 	vfmat\.bf16	q10, q11, q0
 *[0-9a-f]+:	fe76 48f8 	vfmat\.bf16	q10, q11, d0\[3\]
 *[0-9a-f]+:	fe76 48d0 	vfmat\.bf16	q10, q11, d0\[0\]
 *[0-9a-f]+:	fc76 4890 	vfmab\.bf16	q10, q11, q0
 *[0-9a-f]+:	fe76 48b8 	vfmab\.bf16	q10, q11, d0\[3\]
 *[0-9a-f]+:	fe76 4890 	vfmab\.bf16	q10, q11, d0\[0\]
 *[0-9a-f]+:	fff6 464a 	vcvt\.bf16\.f32	d20, q5
 *[0-9a-f]+:	ffb6 b664 	vcvt\.bf16\.f32	d11, q10
 *[0-9a-f]+:	bf18      	it	ne
 *[0-9a-f]+:	ffb6 0640 	vcvtne\.bf16\.f32	d0, q0
 *[0-9a-f]+:	eeb3 a965 	vcvtb\.bf16\.f32	s20, s11
 *[0-9a-f]+:	bf18      	it	ne
 *[0-9a-f]+:	eef3 594a 	vcvtbne\.bf16\.f32	s11, s20
 *[0-9a-f]+:	eeb3 0940 	vcvtb\.bf16\.f32	s0, s0
 *[0-9a-f]+:	eeb3 a9e5 	vcvtt\.bf16\.f32	s20, s11
 *[0-9a-f]+:	bf18      	it	ne
 *[0-9a-f]+:	eef3 59ca 	vcvttne\.bf16\.f32	s11, s20
 *[0-9a-f]+:	eeb3 09c0 	vcvtt\.bf16\.f32	s0, s0
