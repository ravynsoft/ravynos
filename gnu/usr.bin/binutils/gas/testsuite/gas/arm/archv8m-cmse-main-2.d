#name: ARMv8-M Mainline Security Extensions instructions (2)
#source: archv8m-cmse-main.s
#as: -march=armv8-m.main
#objdump: -dr --prefix-addresses --show-raw-insn -M force-thumb

.*: +file format .*arm.*

Disassembly of section .text:
0+.* <[^>]*> ec31 0a00 	vlldm	r1
0+.* <[^>]*> ec22 0a00 	vlstm	r2
#...
