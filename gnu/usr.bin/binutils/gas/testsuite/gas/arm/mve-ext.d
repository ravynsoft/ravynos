# name: MVE context sensitive .arch_extension
# as: -march=armv8.1-m.main
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> ea52 136f 	asrl	r2, r3, #5
