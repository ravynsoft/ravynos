# name: VFP check vmov supports integer immediates
# as: -mcpu=cortex-a8 -mfpu=vfpv3
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> 4ef2da06 	vmovmi.f32	s27, #38	@ 0x41300000  11.0
0[0-9a-f]+ <[^>]+> 4ef2da06 	vmovmi.f32	s27, #38	@ 0x41300000  11.0
0[0-9a-f]+ <[^>]+> 4ef7da00 	vmovmi.f32	s27, #112	@ 0x3f800000  1.0
0[0-9a-f]+ <[^>]+> 4ef7da00 	vmovmi.f32	s27, #112	@ 0x3f800000  1.0
0[0-9a-f]+ <[^>]+> cebb1b04 	vmovgt.f64	d1, #180	@ 0xc1a00000 -20.0
0[0-9a-f]+ <[^>]+> ceb81b00 	vmovgt.f64	d1, #128	@ 0xc0000000 -2.0
0[0-9a-f]+ <[^>]+> eef0aa00 	vmov.f32	s21, #0	@ 0x40000000  2.0
0[0-9a-f]+ <[^>]+> eef97a07 	vmov.f32	s15, #151	@ 0xc0b80000 -5.750
0[0-9a-f]+ <[^>]+> eefc4a05 	vmov.f32	s9, #197	@ 0xbe280000 -0.1640625
