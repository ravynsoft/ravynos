# name: MVE vcvt instructions, part 3
# as: -march=armv8.1-m.main+mve.fp
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
[^>]*> ee3f 1e01 	vcvtt.f16.f32	q0, q0
[^>]*> ee3f 0e01 	vcvtb.f16.f32	q0, q0
[^>]*> ee3f 1e05 	vcvtt.f16.f32	q0, q2
[^>]*> ee3f 0e05 	vcvtb.f16.f32	q0, q2
[^>]*> ee3f 1e09 	vcvtt.f16.f32	q0, q4
[^>]*> ee3f 0e09 	vcvtb.f16.f32	q0, q4
[^>]*> ee3f 1e11 			@ <UNDEFINED> instruction: 0xee3f1e11
[^>]*> ee3f 0e11 			@ <UNDEFINED> instruction: 0xee3f0e11
[^>]*> ee3f 1e1d 			@ <UNDEFINED> instruction: 0xee3f1e1d
[^>]*> ee3f 0e1d 			@ <UNDEFINED> instruction: 0xee3f0e1d
[^>]*> ee3f 5e01 	vcvtt.f16.f32	q2, q0
[^>]*> ee3f 4e01 	vcvtb.f16.f32	q2, q0
[^>]*> ee3f 5e05 	vcvtt.f16.f32	q2, q2
[^>]*> ee3f 4e05 	vcvtb.f16.f32	q2, q2
[^>]*> ee3f 5e09 	vcvtt.f16.f32	q2, q4
[^>]*> ee3f 4e09 	vcvtb.f16.f32	q2, q4
[^>]*> ee3f 5e11 			@ <UNDEFINED> instruction: 0xee3f5e11
[^>]*> ee3f 4e11 			@ <UNDEFINED> instruction: 0xee3f4e11
[^>]*> ee3f 5e1d 			@ <UNDEFINED> instruction: 0xee3f5e1d
[^>]*> ee3f 4e1d 			@ <UNDEFINED> instruction: 0xee3f4e1d
[^>]*> ee3f 9e01 	vcvtt.f16.f32	q4, q0
[^>]*> ee3f 8e01 	vcvtb.f16.f32	q4, q0
[^>]*> ee3f 9e05 	vcvtt.f16.f32	q4, q2
[^>]*> ee3f 8e05 	vcvtb.f16.f32	q4, q2
[^>]*> ee3f 9e09 	vcvtt.f16.f32	q4, q4
[^>]*> ee3f 8e09 	vcvtb.f16.f32	q4, q4
[^>]*> ee3f 9e11 			@ <UNDEFINED> instruction: 0xee3f9e11
[^>]*> ee3f 8e11 			@ <UNDEFINED> instruction: 0xee3f8e11
[^>]*> ee3f 9e1d 			@ <UNDEFINED> instruction: 0xee3f9e1d
[^>]*> ee3f 8e1d 			@ <UNDEFINED> instruction: 0xee3f8e1d
[^>]*> ee3f 1e01 	vcvtt.f16.f32	q0, q0
[^>]*> ee3f 0e01 	vcvtb.f16.f32	q0, q0
[^>]*> ee3f 1e05 	vcvtt.f16.f32	q0, q2
[^>]*> ee3f 0e05 	vcvtb.f16.f32	q0, q2
[^>]*> ee3f 1e09 	vcvtt.f16.f32	q0, q4
[^>]*> ee3f 0e09 	vcvtb.f16.f32	q0, q4
[^>]*> ee3f 1e11 			@ <UNDEFINED> instruction: 0xee3f1e11
[^>]*> ee3f 0e11 			@ <UNDEFINED> instruction: 0xee3f0e11
[^>]*> ee3f 1e1d 			@ <UNDEFINED> instruction: 0xee3f1e1d
[^>]*> ee3f 0e1d 			@ <UNDEFINED> instruction: 0xee3f0e1d
[^>]*> ee3f de01 	vcvtt.f16.f32	q6, q0
[^>]*> ee3f ce01 	vcvtb.f16.f32	q6, q0
[^>]*> ee3f de05 	vcvtt.f16.f32	q6, q2
[^>]*> ee3f ce05 	vcvtb.f16.f32	q6, q2
[^>]*> ee3f de09 	vcvtt.f16.f32	q6, q4
[^>]*> ee3f ce09 	vcvtb.f16.f32	q6, q4
[^>]*> ee3f de11 			@ <UNDEFINED> instruction: 0xee3fde11
[^>]*> ee3f ce11 			@ <UNDEFINED> instruction: 0xee3fce11
[^>]*> ee3f de1d 			@ <UNDEFINED> instruction: 0xee3fde1d
[^>]*> ee3f ce1d 			@ <UNDEFINED> instruction: 0xee3fce1d
[^>]*> fe3f 1e01 	vcvtt.f32.f16	q0, q0
[^>]*> fe3f 0e01 	vcvtb.f32.f16	q0, q0
[^>]*> fe3f 1e05 	vcvtt.f32.f16	q0, q2
[^>]*> fe3f 0e05 	vcvtb.f32.f16	q0, q2
[^>]*> fe3f 1e09 	vcvtt.f32.f16	q0, q4
[^>]*> fe3f 0e09 	vcvtb.f32.f16	q0, q4
[^>]*> fe3f 1e11 			@ <UNDEFINED> instruction: 0xfe3f1e11
[^>]*> fe3f 0e11 			@ <UNDEFINED> instruction: 0xfe3f0e11
[^>]*> fe3f 1e1d 			@ <UNDEFINED> instruction: 0xfe3f1e1d
[^>]*> fe3f 0e1d 			@ <UNDEFINED> instruction: 0xfe3f0e1d
[^>]*> fe3f 5e01 	vcvtt.f32.f16	q2, q0
[^>]*> fe3f 4e01 	vcvtb.f32.f16	q2, q0
[^>]*> fe3f 5e05 	vcvtt.f32.f16	q2, q2
[^>]*> fe3f 4e05 	vcvtb.f32.f16	q2, q2
[^>]*> fe3f 5e09 	vcvtt.f32.f16	q2, q4
[^>]*> fe3f 4e09 	vcvtb.f32.f16	q2, q4
[^>]*> fe3f 5e11 			@ <UNDEFINED> instruction: 0xfe3f5e11
[^>]*> fe3f 4e11 			@ <UNDEFINED> instruction: 0xfe3f4e11
[^>]*> fe3f 5e1d 			@ <UNDEFINED> instruction: 0xfe3f5e1d
[^>]*> fe3f 4e1d 			@ <UNDEFINED> instruction: 0xfe3f4e1d
[^>]*> fe3f 9e01 	vcvtt.f32.f16	q4, q0
[^>]*> fe3f 8e01 	vcvtb.f32.f16	q4, q0
[^>]*> fe3f 9e05 	vcvtt.f32.f16	q4, q2
[^>]*> fe3f 8e05 	vcvtb.f32.f16	q4, q2
[^>]*> fe3f 9e09 	vcvtt.f32.f16	q4, q4
[^>]*> fe3f 8e09 	vcvtb.f32.f16	q4, q4
[^>]*> fe3f 9e11 			@ <UNDEFINED> instruction: 0xfe3f9e11
[^>]*> fe3f 8e11 			@ <UNDEFINED> instruction: 0xfe3f8e11
[^>]*> fe3f 9e1d 			@ <UNDEFINED> instruction: 0xfe3f9e1d
[^>]*> fe3f 8e1d 			@ <UNDEFINED> instruction: 0xfe3f8e1d
[^>]*> fe3f 1e01 	vcvtt.f32.f16	q0, q0
[^>]*> fe3f 0e01 	vcvtb.f32.f16	q0, q0
[^>]*> fe3f 1e05 	vcvtt.f32.f16	q0, q2
[^>]*> fe3f 0e05 	vcvtb.f32.f16	q0, q2
[^>]*> fe3f 1e09 	vcvtt.f32.f16	q0, q4
[^>]*> fe3f 0e09 	vcvtb.f32.f16	q0, q4
[^>]*> fe3f 1e11 			@ <UNDEFINED> instruction: 0xfe3f1e11
[^>]*> fe3f 0e11 			@ <UNDEFINED> instruction: 0xfe3f0e11
[^>]*> fe3f 1e1d 			@ <UNDEFINED> instruction: 0xfe3f1e1d
[^>]*> fe3f 0e1d 			@ <UNDEFINED> instruction: 0xfe3f0e1d
[^>]*> fe3f de01 	vcvtt.f32.f16	q6, q0
[^>]*> fe3f ce01 	vcvtb.f32.f16	q6, q0
[^>]*> fe3f de05 	vcvtt.f32.f16	q6, q2
[^>]*> fe3f ce05 	vcvtb.f32.f16	q6, q2
[^>]*> fe3f de09 	vcvtt.f32.f16	q6, q4
[^>]*> fe3f ce09 	vcvtb.f32.f16	q6, q4
[^>]*> fe3f de11 			@ <UNDEFINED> instruction: 0xfe3fde11
[^>]*> fe3f ce11 			@ <UNDEFINED> instruction: 0xfe3fce11
[^>]*> fe3f de1d 			@ <UNDEFINED> instruction: 0xfe3fde1d
[^>]*> fe3f ce1d 			@ <UNDEFINED> instruction: 0xfe3fce1d
[^>]*> fe31 af4d 	vpsttee
[^>]*> ee3f 1e05 	vcvttt.f16.f32	q0, q2
[^>]*> ee3f 0e05 	vcvtbt.f16.f32	q0, q2
[^>]*> fe3f 5e09 	vcvtte.f32.f16	q2, q4
[^>]*> fe3f 4e09 	vcvtbe.f32.f16	q2, q4
