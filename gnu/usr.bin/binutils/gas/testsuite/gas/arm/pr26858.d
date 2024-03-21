# name: PR26858
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
[^>]*> ee266a87 	vmul.f32	s12, s13, s14
[^>]*> ee000a81 	vmla.f32	s0, s1, s2
