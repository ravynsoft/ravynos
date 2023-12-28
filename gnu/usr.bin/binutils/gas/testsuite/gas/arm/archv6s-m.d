#name: Valid v6S-M
#as: -march=armv6s-m
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> df00      	svc	0
0[0-9a-f]+ <[^>]+> 46c0      	nop.+
