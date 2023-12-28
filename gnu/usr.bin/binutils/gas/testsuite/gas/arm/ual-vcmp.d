#objdump: -dr --prefix-addresses --show-raw-insn
#name: UAL vcmp with 0
#as: -mfpu=vfpv3

.*: +file format .*arm.*


Disassembly of section .text:
0+000 <[^>]*> eeb50a40 	vcmp.f32	s0, #0.0
0+004 <[^>]*> eef50a40 	vcmp.f32	s1, #0.0
0+008 <[^>]*> eef51ac0 	vcmpe.f32	s3, #0.0
0+00c <[^>]*> eeb52ac0 	vcmpe.f32	s4, #0.0
0+010 <[^>]*> eef52a40 	vcmp.f32	s5, #0.0
0+014 <[^>]*> eeb53a40 	vcmp.f32	s6, #0.0
0+018 <[^>]*> eef53ac0 	vcmpe.f32	s7, #0.0
0+01c <[^>]*> eeb54ac0 	vcmpe.f32	s8, #0.0
0+020 <[^>]*> eef54a40 	vcmp.f32	s9, #0.0
0+024 <[^>]*> eeb55ac0 	vcmpe.f32	s10, #0.0
0+028 <[^>]*> eeb50b40 	vcmp.f64	d0, #0.0
0+02c <[^>]*> eeb51b40 	vcmp.f64	d1, #0.0
0+030 <[^>]*> eeb52bc0 	vcmpe.f64	d2, #0.0
0+034 <[^>]*> eeb53bc0 	vcmpe.f64	d3, #0.0
0+038 <[^>]*> eeb54b40 	vcmp.f64	d4, #0.0
0+03c <[^>]*> eeb55b40 	vcmp.f64	d5, #0.0
0+040 <[^>]*> eeb56bc0 	vcmpe.f64	d6, #0.0
0+044 <[^>]*> eeb57bc0 	vcmpe.f64	d7, #0.0
0+048 <[^>]*> eeb58b40 	vcmp.f64	d8, #0.0
0+04c <[^>]*> eeb59bc0 	vcmpe.f64	d9, #0.0