# name: FMA instructions, ARM mode
# as: -mfpu=vfpv4 -I$srcdir/$subdir
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> eea00a81 	vfma\.f32	s0, s1, s2
0[0-9a-f]+ <[^>]+> eea10b02 	vfma\.f64	d0, d1, d2
0[0-9a-f]+ <[^>]+> 0ea00a81 	vfmaeq\.f32	s0, s1, s2
0[0-9a-f]+ <[^>]+> 0ea10b02 	vfmaeq\.f64	d0, d1, d2
0[0-9a-f]+ <[^>]+> eea00ac1 	vfms\.f32	s0, s1, s2
0[0-9a-f]+ <[^>]+> eea10b42 	vfms\.f64	d0, d1, d2
0[0-9a-f]+ <[^>]+> 0ea00ac1 	vfmseq\.f32	s0, s1, s2
0[0-9a-f]+ <[^>]+> 0ea10b42 	vfmseq\.f64	d0, d1, d2
0[0-9a-f]+ <[^>]+> ee900ac1 	vfnma\.f32	s0, s1, s2
0[0-9a-f]+ <[^>]+> ee910b42 	vfnma\.f64	d0, d1, d2
0[0-9a-f]+ <[^>]+> 0e900ac1 	vfnmaeq\.f32	s0, s1, s2
0[0-9a-f]+ <[^>]+> 0e910b42 	vfnmaeq\.f64	d0, d1, d2
0[0-9a-f]+ <[^>]+> ee900a81 	vfnms\.f32	s0, s1, s2
0[0-9a-f]+ <[^>]+> ee910b02 	vfnms\.f64	d0, d1, d2
0[0-9a-f]+ <[^>]+> 0e900a81 	vfnmseq\.f32	s0, s1, s2
0[0-9a-f]+ <[^>]+> 0e910b02 	vfnmseq\.f64	d0, d1, d2
