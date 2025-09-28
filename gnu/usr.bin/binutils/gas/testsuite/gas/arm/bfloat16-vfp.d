#name: Bfloat 16 VFP
#source: bfloat16-non-neon.s
#as: -mno-warn-deprecated -mfpu=vfpxd -march=armv8.6-a -I$srcdir/$subdir
#objdump: -dr --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:

00000000 <.text>:
 *[0-9a-f]*:	eeb3a965 	vcvtb.bf16.f32	s20, s11
 *[0-9a-f]*:	1ef3594a 	vcvtbne.bf16.f32	s11, s20
 *[0-9a-f]*:	eeb30940 	vcvtb.bf16.f32	s0, s0
 *[0-9a-f]*:	eeb3a9e5 	vcvtt.bf16.f32	s20, s11
 *[0-9a-f]*:	1ef359ca 	vcvttne.bf16.f32	s11, s20
 *[0-9a-f]*:	eeb309c0 	vcvtt.bf16.f32	s0, s0
