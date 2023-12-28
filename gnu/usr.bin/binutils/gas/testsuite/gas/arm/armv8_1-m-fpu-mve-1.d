#name: Make sure .fpu does not reset MVE feature bits
#objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> ef22 0844 	vadd.i32	q0, q1, q2
