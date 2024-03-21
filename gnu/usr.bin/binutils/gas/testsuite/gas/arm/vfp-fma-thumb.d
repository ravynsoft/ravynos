# name: FMA instructions, Thumb mode
# as: -mfpu=vfpv4 -I$srcdir/$subdir
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> eea0 0a81 	vfma\.f32	s0, s1, s2
0[0-9a-f]+ <[^>]+> eea1 0b02 	vfma\.f64	d0, d1, d2
0[0-9a-f]+ <[^>]+> bf04      	itt	eq
0[0-9a-f]+ <[^>]+> eea0 0a81 	vfmaeq\.f32	s0, s1, s2
0[0-9a-f]+ <[^>]+> eea1 0b02 	vfmaeq\.f64	d0, d1, d2
0[0-9a-f]+ <[^>]+> eea0 0ac1 	vfms\.f32	s0, s1, s2
0[0-9a-f]+ <[^>]+> eea1 0b42 	vfms\.f64	d0, d1, d2
0[0-9a-f]+ <[^>]+> bf04      	itt	eq
0[0-9a-f]+ <[^>]+> eea0 0ac1 	vfmseq\.f32	s0, s1, s2
0[0-9a-f]+ <[^>]+> eea1 0b42 	vfmseq\.f64	d0, d1, d2
0[0-9a-f]+ <[^>]+> ee90 0ac1 	vfnma\.f32	s0, s1, s2
0[0-9a-f]+ <[^>]+> ee91 0b42 	vfnma\.f64	d0, d1, d2
0[0-9a-f]+ <[^>]+> bf04      	itt	eq
0[0-9a-f]+ <[^>]+> ee90 0ac1 	vfnmaeq\.f32	s0, s1, s2
0[0-9a-f]+ <[^>]+> ee91 0b42 	vfnmaeq\.f64	d0, d1, d2
0[0-9a-f]+ <[^>]+> ee90 0a81 	vfnms\.f32	s0, s1, s2
0[0-9a-f]+ <[^>]+> ee91 0b02 	vfnms\.f64	d0, d1, d2
0[0-9a-f]+ <[^>]+> bf04      	itt	eq
0[0-9a-f]+ <[^>]+> ee90 0a81 	vfnmseq\.f32	s0, s1, s2
0[0-9a-f]+ <[^>]+> ee91 0b02 	vfnmseq\.f64	d0, d1, d2
