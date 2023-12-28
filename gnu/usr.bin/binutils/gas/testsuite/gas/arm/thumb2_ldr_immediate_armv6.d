# name: Ldr immediate on armv6
# as: -march=armv6
# objdump: -dr --prefix-addresses --show-raw-insn
# notarget: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> 4902      	ldr	r1, \[pc, #8\].*
0[0-9a-f]+ <[^>]+> 4903      	ldr	r1, \[pc, #12\]	.*
0[0-9a-f]+ <[^>]+> 4903      	ldr	r1, \[pc, #12\]	.*
0[0-9a-f]+ <[^>]+> 4a04      	ldr	r2, \[pc, #16\]	.*
0[0-9a-f]+ <[^>]+> 4a04      	ldr	r2, \[pc, #16\]	.*
0[0-9a-f]+ <[^>]+> 4a05      	ldr	r2, \[pc, #20\]	.*
0[0-9a-f]+ <[^>]+> 72727272 	.*
0[0-9a-f]+ <[^>]+> 63006300 	.*
0[0-9a-f]+ <[^>]+> 00510051 	.*
0[0-9a-f]+ <[^>]+> 00047000 	.*
0[0-9a-f]+ <[^>]+> ff320000 	.*
0[0-9a-f]+ <[^>]+> 000013f1 	.*

