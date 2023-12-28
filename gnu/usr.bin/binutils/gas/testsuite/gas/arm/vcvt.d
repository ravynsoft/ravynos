#objdump: -dr --prefix-addresses --show-raw-insn
#name: VCVT
#as: -mcpu=cortex-a8 -mfpu=vfpv3

# Test the `VCVT' op

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <[^>]*> eebb1b48 	vcvt.f64.u16	d1, d1, #0
0+004 <[^>]*> eebb1b40 	vcvt.f64.u16	d1, d1, #16
0+008 <[^>]*> eebb1b44 	vcvt.f64.u16	d1, d1, #8
0+00c <[^>]*> eebb1bef 	vcvt.f64.u32	d1, d1, #1
0+010 <[^>]*> eebb1bc0 	vcvt.f64.u32	d1, d1, #32
0+014 <[^>]*> eebb1be7 	vcvt.f64.u32	d1, d1, #17
0+018 <[^>]*> eefb0a48 	vcvt.f32.u16	s1, s1, #0
0+01c <[^>]*> eefb0a40 	vcvt.f32.u16	s1, s1, #16
0+020 <[^>]*> eefb0a60 	vcvt.f32.u16	s1, s1, #15
0+024 <[^>]*> eefb0aef 	vcvt.f32.u32	s1, s1, #1
0+028 <[^>]*> eefb0ac0 	vcvt.f32.u32	s1, s1, #32
0+02c <[^>]*> eefb0ac8 	vcvt.f32.u32	s1, s1, #16
0+030 <[^>]*> eebf1b48 	vcvt.u16.f64	d1, d1, #0
0+034 <[^>]*> eebf1b40 	vcvt.u16.f64	d1, d1, #16
0+038 <[^>]*> eebf1b60 	vcvt.u16.f64	d1, d1, #15
0+03c <[^>]*> eebf1bef 	vcvt.u32.f64	d1, d1, #1
0+040 <[^>]*> eebf1bc0 	vcvt.u32.f64	d1, d1, #32
0+044 <[^>]*> eebf1bc8 	vcvt.u32.f64	d1, d1, #16
0+048 <[^>]*> eeff0a48 	vcvt.u16.f32	s1, s1, #0
0+04c <[^>]*> eeff0a40 	vcvt.u16.f32	s1, s1, #16
0+050 <[^>]*> eeff0a44 	vcvt.u16.f32	s1, s1, #8
0+054 <[^>]*> eeff0aef 	vcvt.u32.f32	s1, s1, #1
0+058 <[^>]*> eeff0ac0 	vcvt.u32.f32	s1, s1, #32
0+05c <[^>]*> eeff0ae7 	vcvt.u32.f32	s1, s1, #17
