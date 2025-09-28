# name: MVE fp context sensitive .arch_extension
# as: -march=armv8.1-m.main
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
[^>]*> eea1 0fc0 	vshlc	q0, r0, #1
