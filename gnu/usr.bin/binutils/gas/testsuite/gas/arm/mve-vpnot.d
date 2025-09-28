# name: MVE vpnot instructions
# as: -march=armv8.1-m.main+mve.fp
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
[^>]*> fe31 0f4d 	vpnot
[^>]*> fe71 8f4d 	vpste
[^>]*> fe31 0f4d 	vpnott
[^>]*> fe31 0f4d 	vpnote
