#name: Bfloat 16 extension
#source: bfloat16.s
#as: -mno-warn-deprecated -march=armv8.6-a+simd -I$srcdir/$subdir
#objdump: -dr --show-raw-insn

.*:     file format .*

Disassembly of section \.text:

00000000 <.text>:
 *[0-9a-f]+:	fc040d8b 	vdot\.bf16	d0, d20, d11
 *[0-9a-f]+:	fc00bd24 	vdot\.bf16	d11, d0, d20
 *[0-9a-f]+:	eeb309c0 	vcvtt\.bf16\.f32	s0, s0
 *[0-9a-f]+:	eeb309c0 	vcvtt\.bf16\.f32	s0, s0
 *[0-9a-f]+:	eeb309c0 	vcvtt\.bf16\.f32	s0, s0
 *[0-9a-f]+:	eeb30940 	vcvtb\.bf16\.f32	s0, s0
 *[0-9a-f]+:	eeb30940 	vcvtb\.bf16\.f32	s0, s0
 *[0-9a-f]+:	eeb30940 	vcvtb\.bf16\.f32	s0, s0
 *[0-9a-f]+:	f3b60640 	vcvt\.bf16\.f32	d0, q0
 *[0-9a-f]+:	f3b60640 	vcvt\.bf16\.f32	d0, q0
 *[0-9a-f]+:	f3b60640 	vcvt\.bf16\.f32	d0, q0
 *[0-9a-f]+:	fe00bd24 	vdot\.bf16	d11, d0, d4\[1\]
 *[0-9a-f]+:	fe040d8b 	vdot\.bf16	d0, d20, d11\[0\]
 *[0-9a-f]+:	fc4a4c40 	vmmla\.bf16	q10, q5, q0
 *[0-9a-f]+:	fc00ac64 	vmmla\.bf16	q5, q0, q10
 *[0-9a-f]*:	fc7648d0 	vfmat\.bf16	q10, q11, q0
 *[0-9a-f]*:	fe7648f8 	vfmat\.bf16	q10, q11, d0\[3\]
 *[0-9a-f]*:	fe7648d0 	vfmat\.bf16	q10, q11, d0\[0\]
 *[0-9a-f]*:	fc764890 	vfmab\.bf16	q10, q11, q0
 *[0-9a-f]*:	fe7648b8 	vfmab\.bf16	q10, q11, d0\[3\]
 *[0-9a-f]*:	fe764890 	vfmab\.bf16	q10, q11, d0\[0\]
 *[0-9a-f]+:	f3f6464a 	vcvt\.bf16\.f32	d20, q5
 *[0-9a-f]+:	f3b6b664 	vcvt\.bf16\.f32	d11, q10
 *[0-9a-f]+:	eeb3a965 	vcvtb\.bf16\.f32	s20, s11
 *[0-9a-f]+:	1ef3594a 	vcvtbne\.bf16\.f32	s11, s20
 *[0-9a-f]+:	eeb30940 	vcvtb\.bf16\.f32	s0, s0
 *[0-9a-f]+:	eeb3a9e5 	vcvtt\.bf16\.f32	s20, s11
 *[0-9a-f]+:	1ef359ca 	vcvttne\.bf16\.f32	s11, s20
 *[0-9a-f]+:	eeb309c0 	vcvtt\.bf16\.f32	s0, s0
