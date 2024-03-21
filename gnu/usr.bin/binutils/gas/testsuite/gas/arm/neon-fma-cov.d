# name: Neon FMA instruction coverage
# as: -mfpu=neon-vfpv4
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section \.text:
0[0-9a-f]+ <[^>]+> f2000c50 	vfma\.f32	q0, q0, q0
0[0-9a-f]+ <[^>]+> f2000c50 	vfma\.f32	q0, q0, q0
0[0-9a-f]+ <[^>]+> f2000c10 	vfma\.f32	d0, d0, d0
0[0-9a-f]+ <[^>]+> f2200c50 	vfms\.f32	q0, q0, q0
0[0-9a-f]+ <[^>]+> f2200c50 	vfms\.f32	q0, q0, q0
0[0-9a-f]+ <[^>]+> f2200c10 	vfms\.f32	d0, d0, d0
