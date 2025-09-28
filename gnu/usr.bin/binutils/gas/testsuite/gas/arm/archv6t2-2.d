# name: ARMv6T2 ARM mode
# as: -march=armv6t2
# source: archv6t2-1.s
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> e320f001 	yield
0[0-9a-f]+ <[^>]+> e320f002 	wfe
0[0-9a-f]+ <[^>]+> e320f003 	wfi
0[0-9a-f]+ <[^>]+> e320f004 	sev
