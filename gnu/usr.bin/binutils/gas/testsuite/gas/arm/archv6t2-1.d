# name: ARMv6T2 THUMB mode
# as: -march=armv6t2 -mthumb
# source: archv6t2-1.s
# objdump: -dr --prefix-addresses --show-raw-insn
# skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> bf10      	yield
0[0-9a-f]+ <[^>]+> bf20      	wfe
0[0-9a-f]+ <[^>]+> bf30      	wfi
0[0-9a-f]+ <[^>]+> bf40      	sev
