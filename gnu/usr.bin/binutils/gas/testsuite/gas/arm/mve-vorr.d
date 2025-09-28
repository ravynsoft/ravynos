# name: MVE vorr instructions
# as: -march=armv8.1-m.main+mve
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
[^>]*> ef20 0150 	vmov	q0, q0
[^>]*> ef20 0150 	vmov	q0, q0
[^>]*> ef20 0150 	vmov	q0, q0
[^>]*> ef20 0150 	vmov	q0, q0
[^>]*> ef20 0150 	vmov	q0, q0
[^>]*> ef20 0150 	vmov	q0, q0
[^>]*> ef20 0150 	vmov	q0, q0
[^>]*> ef20 0150 	vmov	q0, q0
[^>]*> ef20 0152 	vorr	q0, q0, q1
[^>]*> ef20 0152 	vorr	q0, q0, q1
[^>]*> ef20 0152 	vorr	q0, q0, q1
[^>]*> ef20 0152 	vorr	q0, q0, q1
[^>]*> ef20 0152 	vorr	q0, q0, q1
[^>]*> ef20 0152 	vorr	q0, q0, q1
[^>]*> ef20 0152 	vorr	q0, q0, q1
[^>]*> ef20 0152 	vorr	q0, q0, q1
[^>]*> ef20 0154 	vorr	q0, q0, q2
[^>]*> ef20 0154 	vorr	q0, q0, q2
[^>]*> ef20 0154 	vorr	q0, q0, q2
[^>]*> ef20 0154 	vorr	q0, q0, q2
[^>]*> ef20 0154 	vorr	q0, q0, q2
[^>]*> ef20 0154 	vorr	q0, q0, q2
[^>]*> ef20 0154 	vorr	q0, q0, q2
[^>]*> ef20 0154 	vorr	q0, q0, q2
[^>]*> ef20 0158 	vorr	q0, q0, q4
[^>]*> ef20 0158 	vorr	q0, q0, q4
[^>]*> ef20 0158 	vorr	q0, q0, q4
[^>]*> ef20 0158 	vorr	q0, q0, q4
[^>]*> ef20 0158 	vorr	q0, q0, q4
[^>]*> ef20 0158 	vorr	q0, q0, q4
[^>]*> ef20 0158 	vorr	q0, q0, q4
[^>]*> ef20 0158 	vorr	q0, q0, q4
[^>]*> ef20 015e 	vorr	q0, q0, q7
[^>]*> ef20 015e 	vorr	q0, q0, q7
[^>]*> ef20 015e 	vorr	q0, q0, q7
[^>]*> ef20 015e 	vorr	q0, q0, q7
[^>]*> ef20 015e 	vorr	q0, q0, q7
[^>]*> ef20 015e 	vorr	q0, q0, q7
[^>]*> ef20 015e 	vorr	q0, q0, q7
[^>]*> ef20 015e 	vorr	q0, q0, q7
[^>]*> ef22 0150 	vorr	q0, q1, q0
[^>]*> ef22 0150 	vorr	q0, q1, q0
[^>]*> ef22 0150 	vorr	q0, q1, q0
[^>]*> ef22 0150 	vorr	q0, q1, q0
[^>]*> ef22 0150 	vorr	q0, q1, q0
[^>]*> ef22 0150 	vorr	q0, q1, q0
[^>]*> ef22 0150 	vorr	q0, q1, q0
[^>]*> ef22 0150 	vorr	q0, q1, q0
[^>]*> ef22 0152 	vmov	q0, q1
[^>]*> ef22 0152 	vmov	q0, q1
[^>]*> ef22 0152 	vmov	q0, q1
[^>]*> ef22 0152 	vmov	q0, q1
[^>]*> ef22 0152 	vmov	q0, q1
[^>]*> ef22 0152 	vmov	q0, q1
[^>]*> ef22 0152 	vmov	q0, q1
[^>]*> ef22 0152 	vmov	q0, q1
[^>]*> ef22 0154 	vorr	q0, q1, q2
[^>]*> ef22 0154 	vorr	q0, q1, q2
[^>]*> ef22 0154 	vorr	q0, q1, q2
[^>]*> ef22 0154 	vorr	q0, q1, q2
[^>]*> ef22 0154 	vorr	q0, q1, q2
[^>]*> ef22 0154 	vorr	q0, q1, q2
[^>]*> ef22 0154 	vorr	q0, q1, q2
[^>]*> ef22 0154 	vorr	q0, q1, q2
[^>]*> ef22 0158 	vorr	q0, q1, q4
[^>]*> ef22 0158 	vorr	q0, q1, q4
[^>]*> ef22 0158 	vorr	q0, q1, q4
[^>]*> ef22 0158 	vorr	q0, q1, q4
[^>]*> ef22 0158 	vorr	q0, q1, q4
[^>]*> ef22 0158 	vorr	q0, q1, q4
[^>]*> ef22 0158 	vorr	q0, q1, q4
[^>]*> ef22 0158 	vorr	q0, q1, q4
[^>]*> ef22 015e 	vorr	q0, q1, q7
[^>]*> ef22 015e 	vorr	q0, q1, q7
[^>]*> ef22 015e 	vorr	q0, q1, q7
[^>]*> ef22 015e 	vorr	q0, q1, q7
[^>]*> ef22 015e 	vorr	q0, q1, q7
[^>]*> ef22 015e 	vorr	q0, q1, q7
[^>]*> ef22 015e 	vorr	q0, q1, q7
[^>]*> ef22 015e 	vorr	q0, q1, q7
[^>]*> ef24 0150 	vorr	q0, q2, q0
[^>]*> ef24 0150 	vorr	q0, q2, q0
[^>]*> ef24 0150 	vorr	q0, q2, q0
[^>]*> ef24 0150 	vorr	q0, q2, q0
[^>]*> ef24 0150 	vorr	q0, q2, q0
[^>]*> ef24 0150 	vorr	q0, q2, q0
[^>]*> ef24 0150 	vorr	q0, q2, q0
[^>]*> ef24 0150 	vorr	q0, q2, q0
[^>]*> ef24 0152 	vorr	q0, q2, q1
[^>]*> ef24 0152 	vorr	q0, q2, q1
[^>]*> ef24 0152 	vorr	q0, q2, q1
[^>]*> ef24 0152 	vorr	q0, q2, q1
[^>]*> ef24 0152 	vorr	q0, q2, q1
[^>]*> ef24 0152 	vorr	q0, q2, q1
[^>]*> ef24 0152 	vorr	q0, q2, q1
[^>]*> ef24 0152 	vorr	q0, q2, q1
[^>]*> ef24 0154 	vmov	q0, q2
[^>]*> ef24 0154 	vmov	q0, q2
[^>]*> ef24 0154 	vmov	q0, q2
[^>]*> ef24 0154 	vmov	q0, q2
[^>]*> ef24 0154 	vmov	q0, q2
[^>]*> ef24 0154 	vmov	q0, q2
[^>]*> ef24 0154 	vmov	q0, q2
[^>]*> ef24 0154 	vmov	q0, q2
[^>]*> ef24 0158 	vorr	q0, q2, q4
[^>]*> ef24 0158 	vorr	q0, q2, q4
[^>]*> ef24 0158 	vorr	q0, q2, q4
[^>]*> ef24 0158 	vorr	q0, q2, q4
[^>]*> ef24 0158 	vorr	q0, q2, q4
[^>]*> ef24 0158 	vorr	q0, q2, q4
[^>]*> ef24 0158 	vorr	q0, q2, q4
[^>]*> ef24 0158 	vorr	q0, q2, q4
[^>]*> ef24 015e 	vorr	q0, q2, q7
[^>]*> ef24 015e 	vorr	q0, q2, q7
[^>]*> ef24 015e 	vorr	q0, q2, q7
[^>]*> ef24 015e 	vorr	q0, q2, q7
[^>]*> ef24 015e 	vorr	q0, q2, q7
[^>]*> ef24 015e 	vorr	q0, q2, q7
[^>]*> ef24 015e 	vorr	q0, q2, q7
[^>]*> ef24 015e 	vorr	q0, q2, q7
[^>]*> ef28 0150 	vorr	q0, q4, q0
[^>]*> ef28 0150 	vorr	q0, q4, q0
[^>]*> ef28 0150 	vorr	q0, q4, q0
[^>]*> ef28 0150 	vorr	q0, q4, q0
[^>]*> ef28 0150 	vorr	q0, q4, q0
[^>]*> ef28 0150 	vorr	q0, q4, q0
[^>]*> ef28 0150 	vorr	q0, q4, q0
[^>]*> ef28 0150 	vorr	q0, q4, q0
[^>]*> ef28 0152 	vorr	q0, q4, q1
[^>]*> ef28 0152 	vorr	q0, q4, q1
[^>]*> ef28 0152 	vorr	q0, q4, q1
[^>]*> ef28 0152 	vorr	q0, q4, q1
[^>]*> ef28 0152 	vorr	q0, q4, q1
[^>]*> ef28 0152 	vorr	q0, q4, q1
[^>]*> ef28 0152 	vorr	q0, q4, q1
[^>]*> ef28 0152 	vorr	q0, q4, q1
[^>]*> ef28 0154 	vorr	q0, q4, q2
[^>]*> ef28 0154 	vorr	q0, q4, q2
[^>]*> ef28 0154 	vorr	q0, q4, q2
[^>]*> ef28 0154 	vorr	q0, q4, q2
[^>]*> ef28 0154 	vorr	q0, q4, q2
[^>]*> ef28 0154 	vorr	q0, q4, q2
[^>]*> ef28 0154 	vorr	q0, q4, q2
[^>]*> ef28 0154 	vorr	q0, q4, q2
[^>]*> ef28 0158 	vmov	q0, q4
[^>]*> ef28 0158 	vmov	q0, q4
[^>]*> ef28 0158 	vmov	q0, q4
[^>]*> ef28 0158 	vmov	q0, q4
[^>]*> ef28 0158 	vmov	q0, q4
[^>]*> ef28 0158 	vmov	q0, q4
[^>]*> ef28 0158 	vmov	q0, q4
[^>]*> ef28 0158 	vmov	q0, q4
[^>]*> ef28 015e 	vorr	q0, q4, q7
[^>]*> ef28 015e 	vorr	q0, q4, q7
[^>]*> ef28 015e 	vorr	q0, q4, q7
[^>]*> ef28 015e 	vorr	q0, q4, q7
[^>]*> ef28 015e 	vorr	q0, q4, q7
[^>]*> ef28 015e 	vorr	q0, q4, q7
[^>]*> ef28 015e 	vorr	q0, q4, q7
[^>]*> ef28 015e 	vorr	q0, q4, q7
[^>]*> ef2e 0150 	vorr	q0, q7, q0
[^>]*> ef2e 0150 	vorr	q0, q7, q0
[^>]*> ef2e 0150 	vorr	q0, q7, q0
[^>]*> ef2e 0150 	vorr	q0, q7, q0
[^>]*> ef2e 0150 	vorr	q0, q7, q0
[^>]*> ef2e 0150 	vorr	q0, q7, q0
[^>]*> ef2e 0150 	vorr	q0, q7, q0
[^>]*> ef2e 0150 	vorr	q0, q7, q0
[^>]*> ef2e 0152 	vorr	q0, q7, q1
[^>]*> ef2e 0152 	vorr	q0, q7, q1
[^>]*> ef2e 0152 	vorr	q0, q7, q1
[^>]*> ef2e 0152 	vorr	q0, q7, q1
[^>]*> ef2e 0152 	vorr	q0, q7, q1
[^>]*> ef2e 0152 	vorr	q0, q7, q1
[^>]*> ef2e 0152 	vorr	q0, q7, q1
[^>]*> ef2e 0152 	vorr	q0, q7, q1
[^>]*> ef2e 0154 	vorr	q0, q7, q2
[^>]*> ef2e 0154 	vorr	q0, q7, q2
[^>]*> ef2e 0154 	vorr	q0, q7, q2
[^>]*> ef2e 0154 	vorr	q0, q7, q2
[^>]*> ef2e 0154 	vorr	q0, q7, q2
[^>]*> ef2e 0154 	vorr	q0, q7, q2
[^>]*> ef2e 0154 	vorr	q0, q7, q2
[^>]*> ef2e 0154 	vorr	q0, q7, q2
[^>]*> ef2e 0158 	vorr	q0, q7, q4
[^>]*> ef2e 0158 	vorr	q0, q7, q4
[^>]*> ef2e 0158 	vorr	q0, q7, q4
[^>]*> ef2e 0158 	vorr	q0, q7, q4
[^>]*> ef2e 0158 	vorr	q0, q7, q4
[^>]*> ef2e 0158 	vorr	q0, q7, q4
[^>]*> ef2e 0158 	vorr	q0, q7, q4
[^>]*> ef2e 0158 	vorr	q0, q7, q4
[^>]*> ef2e 015e 	vmov	q0, q7
[^>]*> ef2e 015e 	vmov	q0, q7
[^>]*> ef2e 015e 	vmov	q0, q7
[^>]*> ef2e 015e 	vmov	q0, q7
[^>]*> ef2e 015e 	vmov	q0, q7
[^>]*> ef2e 015e 	vmov	q0, q7
[^>]*> ef2e 015e 	vmov	q0, q7
[^>]*> ef2e 015e 	vmov	q0, q7
[^>]*> ef20 2150 	vmov	q1, q0
[^>]*> ef20 2150 	vmov	q1, q0
[^>]*> ef20 2150 	vmov	q1, q0
[^>]*> ef20 2150 	vmov	q1, q0
[^>]*> ef20 2150 	vmov	q1, q0
[^>]*> ef20 2150 	vmov	q1, q0
[^>]*> ef20 2150 	vmov	q1, q0
[^>]*> ef20 2150 	vmov	q1, q0
[^>]*> ef20 2152 	vorr	q1, q0, q1
[^>]*> ef20 2152 	vorr	q1, q0, q1
[^>]*> ef20 2152 	vorr	q1, q0, q1
[^>]*> ef20 2152 	vorr	q1, q0, q1
[^>]*> ef20 2152 	vorr	q1, q0, q1
[^>]*> ef20 2152 	vorr	q1, q0, q1
[^>]*> ef20 2152 	vorr	q1, q0, q1
[^>]*> ef20 2152 	vorr	q1, q0, q1
[^>]*> ef20 2154 	vorr	q1, q0, q2
[^>]*> ef20 2154 	vorr	q1, q0, q2
[^>]*> ef20 2154 	vorr	q1, q0, q2
[^>]*> ef20 2154 	vorr	q1, q0, q2
[^>]*> ef20 2154 	vorr	q1, q0, q2
[^>]*> ef20 2154 	vorr	q1, q0, q2
[^>]*> ef20 2154 	vorr	q1, q0, q2
[^>]*> ef20 2154 	vorr	q1, q0, q2
[^>]*> ef20 2158 	vorr	q1, q0, q4
[^>]*> ef20 2158 	vorr	q1, q0, q4
[^>]*> ef20 2158 	vorr	q1, q0, q4
[^>]*> ef20 2158 	vorr	q1, q0, q4
[^>]*> ef20 2158 	vorr	q1, q0, q4
[^>]*> ef20 2158 	vorr	q1, q0, q4
[^>]*> ef20 2158 	vorr	q1, q0, q4
[^>]*> ef20 2158 	vorr	q1, q0, q4
[^>]*> ef20 215e 	vorr	q1, q0, q7
[^>]*> ef20 215e 	vorr	q1, q0, q7
[^>]*> ef20 215e 	vorr	q1, q0, q7
[^>]*> ef20 215e 	vorr	q1, q0, q7
[^>]*> ef20 215e 	vorr	q1, q0, q7
[^>]*> ef20 215e 	vorr	q1, q0, q7
[^>]*> ef20 215e 	vorr	q1, q0, q7
[^>]*> ef20 215e 	vorr	q1, q0, q7
[^>]*> ef22 2150 	vorr	q1, q1, q0
[^>]*> ef22 2150 	vorr	q1, q1, q0
[^>]*> ef22 2150 	vorr	q1, q1, q0
[^>]*> ef22 2150 	vorr	q1, q1, q0
[^>]*> ef22 2150 	vorr	q1, q1, q0
[^>]*> ef22 2150 	vorr	q1, q1, q0
[^>]*> ef22 2150 	vorr	q1, q1, q0
[^>]*> ef22 2150 	vorr	q1, q1, q0
[^>]*> ef22 2152 	vmov	q1, q1
[^>]*> ef22 2152 	vmov	q1, q1
[^>]*> ef22 2152 	vmov	q1, q1
[^>]*> ef22 2152 	vmov	q1, q1
[^>]*> ef22 2152 	vmov	q1, q1
[^>]*> ef22 2152 	vmov	q1, q1
[^>]*> ef22 2152 	vmov	q1, q1
[^>]*> ef22 2152 	vmov	q1, q1
[^>]*> ef22 2154 	vorr	q1, q1, q2
[^>]*> ef22 2154 	vorr	q1, q1, q2
[^>]*> ef22 2154 	vorr	q1, q1, q2
[^>]*> ef22 2154 	vorr	q1, q1, q2
[^>]*> ef22 2154 	vorr	q1, q1, q2
[^>]*> ef22 2154 	vorr	q1, q1, q2
[^>]*> ef22 2154 	vorr	q1, q1, q2
[^>]*> ef22 2154 	vorr	q1, q1, q2
[^>]*> ef22 2158 	vorr	q1, q1, q4
[^>]*> ef22 2158 	vorr	q1, q1, q4
[^>]*> ef22 2158 	vorr	q1, q1, q4
[^>]*> ef22 2158 	vorr	q1, q1, q4
[^>]*> ef22 2158 	vorr	q1, q1, q4
[^>]*> ef22 2158 	vorr	q1, q1, q4
[^>]*> ef22 2158 	vorr	q1, q1, q4
[^>]*> ef22 2158 	vorr	q1, q1, q4
[^>]*> ef22 215e 	vorr	q1, q1, q7
[^>]*> ef22 215e 	vorr	q1, q1, q7
[^>]*> ef22 215e 	vorr	q1, q1, q7
[^>]*> ef22 215e 	vorr	q1, q1, q7
[^>]*> ef22 215e 	vorr	q1, q1, q7
[^>]*> ef22 215e 	vorr	q1, q1, q7
[^>]*> ef22 215e 	vorr	q1, q1, q7
[^>]*> ef22 215e 	vorr	q1, q1, q7
[^>]*> ef24 2150 	vorr	q1, q2, q0
[^>]*> ef24 2150 	vorr	q1, q2, q0
[^>]*> ef24 2150 	vorr	q1, q2, q0
[^>]*> ef24 2150 	vorr	q1, q2, q0
[^>]*> ef24 2150 	vorr	q1, q2, q0
[^>]*> ef24 2150 	vorr	q1, q2, q0
[^>]*> ef24 2150 	vorr	q1, q2, q0
[^>]*> ef24 2150 	vorr	q1, q2, q0
[^>]*> ef24 2152 	vorr	q1, q2, q1
[^>]*> ef24 2152 	vorr	q1, q2, q1
[^>]*> ef24 2152 	vorr	q1, q2, q1
[^>]*> ef24 2152 	vorr	q1, q2, q1
[^>]*> ef24 2152 	vorr	q1, q2, q1
[^>]*> ef24 2152 	vorr	q1, q2, q1
[^>]*> ef24 2152 	vorr	q1, q2, q1
[^>]*> ef24 2152 	vorr	q1, q2, q1
[^>]*> ef24 2154 	vmov	q1, q2
[^>]*> ef24 2154 	vmov	q1, q2
[^>]*> ef24 2154 	vmov	q1, q2
[^>]*> ef24 2154 	vmov	q1, q2
[^>]*> ef24 2154 	vmov	q1, q2
[^>]*> ef24 2154 	vmov	q1, q2
[^>]*> ef24 2154 	vmov	q1, q2
[^>]*> ef24 2154 	vmov	q1, q2
[^>]*> ef24 2158 	vorr	q1, q2, q4
[^>]*> ef24 2158 	vorr	q1, q2, q4
[^>]*> ef24 2158 	vorr	q1, q2, q4
[^>]*> ef24 2158 	vorr	q1, q2, q4
[^>]*> ef24 2158 	vorr	q1, q2, q4
[^>]*> ef24 2158 	vorr	q1, q2, q4
[^>]*> ef24 2158 	vorr	q1, q2, q4
[^>]*> ef24 2158 	vorr	q1, q2, q4
[^>]*> ef24 215e 	vorr	q1, q2, q7
[^>]*> ef24 215e 	vorr	q1, q2, q7
[^>]*> ef24 215e 	vorr	q1, q2, q7
[^>]*> ef24 215e 	vorr	q1, q2, q7
[^>]*> ef24 215e 	vorr	q1, q2, q7
[^>]*> ef24 215e 	vorr	q1, q2, q7
[^>]*> ef24 215e 	vorr	q1, q2, q7
[^>]*> ef24 215e 	vorr	q1, q2, q7
[^>]*> ef28 2150 	vorr	q1, q4, q0
[^>]*> ef28 2150 	vorr	q1, q4, q0
[^>]*> ef28 2150 	vorr	q1, q4, q0
[^>]*> ef28 2150 	vorr	q1, q4, q0
[^>]*> ef28 2150 	vorr	q1, q4, q0
[^>]*> ef28 2150 	vorr	q1, q4, q0
[^>]*> ef28 2150 	vorr	q1, q4, q0
[^>]*> ef28 2150 	vorr	q1, q4, q0
[^>]*> ef28 2152 	vorr	q1, q4, q1
[^>]*> ef28 2152 	vorr	q1, q4, q1
[^>]*> ef28 2152 	vorr	q1, q4, q1
[^>]*> ef28 2152 	vorr	q1, q4, q1
[^>]*> ef28 2152 	vorr	q1, q4, q1
[^>]*> ef28 2152 	vorr	q1, q4, q1
[^>]*> ef28 2152 	vorr	q1, q4, q1
[^>]*> ef28 2152 	vorr	q1, q4, q1
[^>]*> ef28 2154 	vorr	q1, q4, q2
[^>]*> ef28 2154 	vorr	q1, q4, q2
[^>]*> ef28 2154 	vorr	q1, q4, q2
[^>]*> ef28 2154 	vorr	q1, q4, q2
[^>]*> ef28 2154 	vorr	q1, q4, q2
[^>]*> ef28 2154 	vorr	q1, q4, q2
[^>]*> ef28 2154 	vorr	q1, q4, q2
[^>]*> ef28 2154 	vorr	q1, q4, q2
[^>]*> ef28 2158 	vmov	q1, q4
[^>]*> ef28 2158 	vmov	q1, q4
[^>]*> ef28 2158 	vmov	q1, q4
[^>]*> ef28 2158 	vmov	q1, q4
[^>]*> ef28 2158 	vmov	q1, q4
[^>]*> ef28 2158 	vmov	q1, q4
[^>]*> ef28 2158 	vmov	q1, q4
[^>]*> ef28 2158 	vmov	q1, q4
[^>]*> ef28 215e 	vorr	q1, q4, q7
[^>]*> ef28 215e 	vorr	q1, q4, q7
[^>]*> ef28 215e 	vorr	q1, q4, q7
[^>]*> ef28 215e 	vorr	q1, q4, q7
[^>]*> ef28 215e 	vorr	q1, q4, q7
[^>]*> ef28 215e 	vorr	q1, q4, q7
[^>]*> ef28 215e 	vorr	q1, q4, q7
[^>]*> ef28 215e 	vorr	q1, q4, q7
[^>]*> ef2e 2150 	vorr	q1, q7, q0
[^>]*> ef2e 2150 	vorr	q1, q7, q0
[^>]*> ef2e 2150 	vorr	q1, q7, q0
[^>]*> ef2e 2150 	vorr	q1, q7, q0
[^>]*> ef2e 2150 	vorr	q1, q7, q0
[^>]*> ef2e 2150 	vorr	q1, q7, q0
[^>]*> ef2e 2150 	vorr	q1, q7, q0
[^>]*> ef2e 2150 	vorr	q1, q7, q0
[^>]*> ef2e 2152 	vorr	q1, q7, q1
[^>]*> ef2e 2152 	vorr	q1, q7, q1
[^>]*> ef2e 2152 	vorr	q1, q7, q1
[^>]*> ef2e 2152 	vorr	q1, q7, q1
[^>]*> ef2e 2152 	vorr	q1, q7, q1
[^>]*> ef2e 2152 	vorr	q1, q7, q1
[^>]*> ef2e 2152 	vorr	q1, q7, q1
[^>]*> ef2e 2152 	vorr	q1, q7, q1
[^>]*> ef2e 2154 	vorr	q1, q7, q2
[^>]*> ef2e 2154 	vorr	q1, q7, q2
[^>]*> ef2e 2154 	vorr	q1, q7, q2
[^>]*> ef2e 2154 	vorr	q1, q7, q2
[^>]*> ef2e 2154 	vorr	q1, q7, q2
[^>]*> ef2e 2154 	vorr	q1, q7, q2
[^>]*> ef2e 2154 	vorr	q1, q7, q2
[^>]*> ef2e 2154 	vorr	q1, q7, q2
[^>]*> ef2e 2158 	vorr	q1, q7, q4
[^>]*> ef2e 2158 	vorr	q1, q7, q4
[^>]*> ef2e 2158 	vorr	q1, q7, q4
[^>]*> ef2e 2158 	vorr	q1, q7, q4
[^>]*> ef2e 2158 	vorr	q1, q7, q4
[^>]*> ef2e 2158 	vorr	q1, q7, q4
[^>]*> ef2e 2158 	vorr	q1, q7, q4
[^>]*> ef2e 2158 	vorr	q1, q7, q4
[^>]*> ef2e 215e 	vmov	q1, q7
[^>]*> ef2e 215e 	vmov	q1, q7
[^>]*> ef2e 215e 	vmov	q1, q7
[^>]*> ef2e 215e 	vmov	q1, q7
[^>]*> ef2e 215e 	vmov	q1, q7
[^>]*> ef2e 215e 	vmov	q1, q7
[^>]*> ef2e 215e 	vmov	q1, q7
[^>]*> ef2e 215e 	vmov	q1, q7
[^>]*> ef20 4150 	vmov	q2, q0
[^>]*> ef20 4150 	vmov	q2, q0
[^>]*> ef20 4150 	vmov	q2, q0
[^>]*> ef20 4150 	vmov	q2, q0
[^>]*> ef20 4150 	vmov	q2, q0
[^>]*> ef20 4150 	vmov	q2, q0
[^>]*> ef20 4150 	vmov	q2, q0
[^>]*> ef20 4150 	vmov	q2, q0
[^>]*> ef20 4152 	vorr	q2, q0, q1
[^>]*> ef20 4152 	vorr	q2, q0, q1
[^>]*> ef20 4152 	vorr	q2, q0, q1
[^>]*> ef20 4152 	vorr	q2, q0, q1
[^>]*> ef20 4152 	vorr	q2, q0, q1
[^>]*> ef20 4152 	vorr	q2, q0, q1
[^>]*> ef20 4152 	vorr	q2, q0, q1
[^>]*> ef20 4152 	vorr	q2, q0, q1
[^>]*> ef20 4154 	vorr	q2, q0, q2
[^>]*> ef20 4154 	vorr	q2, q0, q2
[^>]*> ef20 4154 	vorr	q2, q0, q2
[^>]*> ef20 4154 	vorr	q2, q0, q2
[^>]*> ef20 4154 	vorr	q2, q0, q2
[^>]*> ef20 4154 	vorr	q2, q0, q2
[^>]*> ef20 4154 	vorr	q2, q0, q2
[^>]*> ef20 4154 	vorr	q2, q0, q2
[^>]*> ef20 4158 	vorr	q2, q0, q4
[^>]*> ef20 4158 	vorr	q2, q0, q4
[^>]*> ef20 4158 	vorr	q2, q0, q4
[^>]*> ef20 4158 	vorr	q2, q0, q4
[^>]*> ef20 4158 	vorr	q2, q0, q4
[^>]*> ef20 4158 	vorr	q2, q0, q4
[^>]*> ef20 4158 	vorr	q2, q0, q4
[^>]*> ef20 4158 	vorr	q2, q0, q4
[^>]*> ef20 415e 	vorr	q2, q0, q7
[^>]*> ef20 415e 	vorr	q2, q0, q7
[^>]*> ef20 415e 	vorr	q2, q0, q7
[^>]*> ef20 415e 	vorr	q2, q0, q7
[^>]*> ef20 415e 	vorr	q2, q0, q7
[^>]*> ef20 415e 	vorr	q2, q0, q7
[^>]*> ef20 415e 	vorr	q2, q0, q7
[^>]*> ef20 415e 	vorr	q2, q0, q7
[^>]*> ef22 4150 	vorr	q2, q1, q0
[^>]*> ef22 4150 	vorr	q2, q1, q0
[^>]*> ef22 4150 	vorr	q2, q1, q0
[^>]*> ef22 4150 	vorr	q2, q1, q0
[^>]*> ef22 4150 	vorr	q2, q1, q0
[^>]*> ef22 4150 	vorr	q2, q1, q0
[^>]*> ef22 4150 	vorr	q2, q1, q0
[^>]*> ef22 4150 	vorr	q2, q1, q0
[^>]*> ef22 4152 	vmov	q2, q1
[^>]*> ef22 4152 	vmov	q2, q1
[^>]*> ef22 4152 	vmov	q2, q1
[^>]*> ef22 4152 	vmov	q2, q1
[^>]*> ef22 4152 	vmov	q2, q1
[^>]*> ef22 4152 	vmov	q2, q1
[^>]*> ef22 4152 	vmov	q2, q1
[^>]*> ef22 4152 	vmov	q2, q1
[^>]*> ef22 4154 	vorr	q2, q1, q2
[^>]*> ef22 4154 	vorr	q2, q1, q2
[^>]*> ef22 4154 	vorr	q2, q1, q2
[^>]*> ef22 4154 	vorr	q2, q1, q2
[^>]*> ef22 4154 	vorr	q2, q1, q2
[^>]*> ef22 4154 	vorr	q2, q1, q2
[^>]*> ef22 4154 	vorr	q2, q1, q2
[^>]*> ef22 4154 	vorr	q2, q1, q2
[^>]*> ef22 4158 	vorr	q2, q1, q4
[^>]*> ef22 4158 	vorr	q2, q1, q4
[^>]*> ef22 4158 	vorr	q2, q1, q4
[^>]*> ef22 4158 	vorr	q2, q1, q4
[^>]*> ef22 4158 	vorr	q2, q1, q4
[^>]*> ef22 4158 	vorr	q2, q1, q4
[^>]*> ef22 4158 	vorr	q2, q1, q4
[^>]*> ef22 4158 	vorr	q2, q1, q4
[^>]*> ef22 415e 	vorr	q2, q1, q7
[^>]*> ef22 415e 	vorr	q2, q1, q7
[^>]*> ef22 415e 	vorr	q2, q1, q7
[^>]*> ef22 415e 	vorr	q2, q1, q7
[^>]*> ef22 415e 	vorr	q2, q1, q7
[^>]*> ef22 415e 	vorr	q2, q1, q7
[^>]*> ef22 415e 	vorr	q2, q1, q7
[^>]*> ef22 415e 	vorr	q2, q1, q7
[^>]*> ef24 4150 	vorr	q2, q2, q0
[^>]*> ef24 4150 	vorr	q2, q2, q0
[^>]*> ef24 4150 	vorr	q2, q2, q0
[^>]*> ef24 4150 	vorr	q2, q2, q0
[^>]*> ef24 4150 	vorr	q2, q2, q0
[^>]*> ef24 4150 	vorr	q2, q2, q0
[^>]*> ef24 4150 	vorr	q2, q2, q0
[^>]*> ef24 4150 	vorr	q2, q2, q0
[^>]*> ef24 4152 	vorr	q2, q2, q1
[^>]*> ef24 4152 	vorr	q2, q2, q1
[^>]*> ef24 4152 	vorr	q2, q2, q1
[^>]*> ef24 4152 	vorr	q2, q2, q1
[^>]*> ef24 4152 	vorr	q2, q2, q1
[^>]*> ef24 4152 	vorr	q2, q2, q1
[^>]*> ef24 4152 	vorr	q2, q2, q1
[^>]*> ef24 4152 	vorr	q2, q2, q1
[^>]*> ef24 4154 	vmov	q2, q2
[^>]*> ef24 4154 	vmov	q2, q2
[^>]*> ef24 4154 	vmov	q2, q2
[^>]*> ef24 4154 	vmov	q2, q2
[^>]*> ef24 4154 	vmov	q2, q2
[^>]*> ef24 4154 	vmov	q2, q2
[^>]*> ef24 4154 	vmov	q2, q2
[^>]*> ef24 4154 	vmov	q2, q2
[^>]*> ef24 4158 	vorr	q2, q2, q4
[^>]*> ef24 4158 	vorr	q2, q2, q4
[^>]*> ef24 4158 	vorr	q2, q2, q4
[^>]*> ef24 4158 	vorr	q2, q2, q4
[^>]*> ef24 4158 	vorr	q2, q2, q4
[^>]*> ef24 4158 	vorr	q2, q2, q4
[^>]*> ef24 4158 	vorr	q2, q2, q4
[^>]*> ef24 4158 	vorr	q2, q2, q4
[^>]*> ef24 415e 	vorr	q2, q2, q7
[^>]*> ef24 415e 	vorr	q2, q2, q7
[^>]*> ef24 415e 	vorr	q2, q2, q7
[^>]*> ef24 415e 	vorr	q2, q2, q7
[^>]*> ef24 415e 	vorr	q2, q2, q7
[^>]*> ef24 415e 	vorr	q2, q2, q7
[^>]*> ef24 415e 	vorr	q2, q2, q7
[^>]*> ef24 415e 	vorr	q2, q2, q7
[^>]*> ef28 4150 	vorr	q2, q4, q0
[^>]*> ef28 4150 	vorr	q2, q4, q0
[^>]*> ef28 4150 	vorr	q2, q4, q0
[^>]*> ef28 4150 	vorr	q2, q4, q0
[^>]*> ef28 4150 	vorr	q2, q4, q0
[^>]*> ef28 4150 	vorr	q2, q4, q0
[^>]*> ef28 4150 	vorr	q2, q4, q0
[^>]*> ef28 4150 	vorr	q2, q4, q0
[^>]*> ef28 4152 	vorr	q2, q4, q1
[^>]*> ef28 4152 	vorr	q2, q4, q1
[^>]*> ef28 4152 	vorr	q2, q4, q1
[^>]*> ef28 4152 	vorr	q2, q4, q1
[^>]*> ef28 4152 	vorr	q2, q4, q1
[^>]*> ef28 4152 	vorr	q2, q4, q1
[^>]*> ef28 4152 	vorr	q2, q4, q1
[^>]*> ef28 4152 	vorr	q2, q4, q1
[^>]*> ef28 4154 	vorr	q2, q4, q2
[^>]*> ef28 4154 	vorr	q2, q4, q2
[^>]*> ef28 4154 	vorr	q2, q4, q2
[^>]*> ef28 4154 	vorr	q2, q4, q2
[^>]*> ef28 4154 	vorr	q2, q4, q2
[^>]*> ef28 4154 	vorr	q2, q4, q2
[^>]*> ef28 4154 	vorr	q2, q4, q2
[^>]*> ef28 4154 	vorr	q2, q4, q2
[^>]*> ef28 4158 	vmov	q2, q4
[^>]*> ef28 4158 	vmov	q2, q4
[^>]*> ef28 4158 	vmov	q2, q4
[^>]*> ef28 4158 	vmov	q2, q4
[^>]*> ef28 4158 	vmov	q2, q4
[^>]*> ef28 4158 	vmov	q2, q4
[^>]*> ef28 4158 	vmov	q2, q4
[^>]*> ef28 4158 	vmov	q2, q4
[^>]*> ef28 415e 	vorr	q2, q4, q7
[^>]*> ef28 415e 	vorr	q2, q4, q7
[^>]*> ef28 415e 	vorr	q2, q4, q7
[^>]*> ef28 415e 	vorr	q2, q4, q7
[^>]*> ef28 415e 	vorr	q2, q4, q7
[^>]*> ef28 415e 	vorr	q2, q4, q7
[^>]*> ef28 415e 	vorr	q2, q4, q7
[^>]*> ef28 415e 	vorr	q2, q4, q7
[^>]*> ef2e 4150 	vorr	q2, q7, q0
[^>]*> ef2e 4150 	vorr	q2, q7, q0
[^>]*> ef2e 4150 	vorr	q2, q7, q0
[^>]*> ef2e 4150 	vorr	q2, q7, q0
[^>]*> ef2e 4150 	vorr	q2, q7, q0
[^>]*> ef2e 4150 	vorr	q2, q7, q0
[^>]*> ef2e 4150 	vorr	q2, q7, q0
[^>]*> ef2e 4150 	vorr	q2, q7, q0
[^>]*> ef2e 4152 	vorr	q2, q7, q1
[^>]*> ef2e 4152 	vorr	q2, q7, q1
[^>]*> ef2e 4152 	vorr	q2, q7, q1
[^>]*> ef2e 4152 	vorr	q2, q7, q1
[^>]*> ef2e 4152 	vorr	q2, q7, q1
[^>]*> ef2e 4152 	vorr	q2, q7, q1
[^>]*> ef2e 4152 	vorr	q2, q7, q1
[^>]*> ef2e 4152 	vorr	q2, q7, q1
[^>]*> ef2e 4154 	vorr	q2, q7, q2
[^>]*> ef2e 4154 	vorr	q2, q7, q2
[^>]*> ef2e 4154 	vorr	q2, q7, q2
[^>]*> ef2e 4154 	vorr	q2, q7, q2
[^>]*> ef2e 4154 	vorr	q2, q7, q2
[^>]*> ef2e 4154 	vorr	q2, q7, q2
[^>]*> ef2e 4154 	vorr	q2, q7, q2
[^>]*> ef2e 4154 	vorr	q2, q7, q2
[^>]*> ef2e 4158 	vorr	q2, q7, q4
[^>]*> ef2e 4158 	vorr	q2, q7, q4
[^>]*> ef2e 4158 	vorr	q2, q7, q4
[^>]*> ef2e 4158 	vorr	q2, q7, q4
[^>]*> ef2e 4158 	vorr	q2, q7, q4
[^>]*> ef2e 4158 	vorr	q2, q7, q4
[^>]*> ef2e 4158 	vorr	q2, q7, q4
[^>]*> ef2e 4158 	vorr	q2, q7, q4
[^>]*> ef2e 415e 	vmov	q2, q7
[^>]*> ef2e 415e 	vmov	q2, q7
[^>]*> ef2e 415e 	vmov	q2, q7
[^>]*> ef2e 415e 	vmov	q2, q7
[^>]*> ef2e 415e 	vmov	q2, q7
[^>]*> ef2e 415e 	vmov	q2, q7
[^>]*> ef2e 415e 	vmov	q2, q7
[^>]*> ef2e 415e 	vmov	q2, q7
[^>]*> ef20 8150 	vmov	q4, q0
[^>]*> ef20 8150 	vmov	q4, q0
[^>]*> ef20 8150 	vmov	q4, q0
[^>]*> ef20 8150 	vmov	q4, q0
[^>]*> ef20 8150 	vmov	q4, q0
[^>]*> ef20 8150 	vmov	q4, q0
[^>]*> ef20 8150 	vmov	q4, q0
[^>]*> ef20 8150 	vmov	q4, q0
[^>]*> ef20 8152 	vorr	q4, q0, q1
[^>]*> ef20 8152 	vorr	q4, q0, q1
[^>]*> ef20 8152 	vorr	q4, q0, q1
[^>]*> ef20 8152 	vorr	q4, q0, q1
[^>]*> ef20 8152 	vorr	q4, q0, q1
[^>]*> ef20 8152 	vorr	q4, q0, q1
[^>]*> ef20 8152 	vorr	q4, q0, q1
[^>]*> ef20 8152 	vorr	q4, q0, q1
[^>]*> ef20 8154 	vorr	q4, q0, q2
[^>]*> ef20 8154 	vorr	q4, q0, q2
[^>]*> ef20 8154 	vorr	q4, q0, q2
[^>]*> ef20 8154 	vorr	q4, q0, q2
[^>]*> ef20 8154 	vorr	q4, q0, q2
[^>]*> ef20 8154 	vorr	q4, q0, q2
[^>]*> ef20 8154 	vorr	q4, q0, q2
[^>]*> ef20 8154 	vorr	q4, q0, q2
[^>]*> ef20 8158 	vorr	q4, q0, q4
[^>]*> ef20 8158 	vorr	q4, q0, q4
[^>]*> ef20 8158 	vorr	q4, q0, q4
[^>]*> ef20 8158 	vorr	q4, q0, q4
[^>]*> ef20 8158 	vorr	q4, q0, q4
[^>]*> ef20 8158 	vorr	q4, q0, q4
[^>]*> ef20 8158 	vorr	q4, q0, q4
[^>]*> ef20 8158 	vorr	q4, q0, q4
[^>]*> ef20 815e 	vorr	q4, q0, q7
[^>]*> ef20 815e 	vorr	q4, q0, q7
[^>]*> ef20 815e 	vorr	q4, q0, q7
[^>]*> ef20 815e 	vorr	q4, q0, q7
[^>]*> ef20 815e 	vorr	q4, q0, q7
[^>]*> ef20 815e 	vorr	q4, q0, q7
[^>]*> ef20 815e 	vorr	q4, q0, q7
[^>]*> ef20 815e 	vorr	q4, q0, q7
[^>]*> ef22 8150 	vorr	q4, q1, q0
[^>]*> ef22 8150 	vorr	q4, q1, q0
[^>]*> ef22 8150 	vorr	q4, q1, q0
[^>]*> ef22 8150 	vorr	q4, q1, q0
[^>]*> ef22 8150 	vorr	q4, q1, q0
[^>]*> ef22 8150 	vorr	q4, q1, q0
[^>]*> ef22 8150 	vorr	q4, q1, q0
[^>]*> ef22 8150 	vorr	q4, q1, q0
[^>]*> ef22 8152 	vmov	q4, q1
[^>]*> ef22 8152 	vmov	q4, q1
[^>]*> ef22 8152 	vmov	q4, q1
[^>]*> ef22 8152 	vmov	q4, q1
[^>]*> ef22 8152 	vmov	q4, q1
[^>]*> ef22 8152 	vmov	q4, q1
[^>]*> ef22 8152 	vmov	q4, q1
[^>]*> ef22 8152 	vmov	q4, q1
[^>]*> ef22 8154 	vorr	q4, q1, q2
[^>]*> ef22 8154 	vorr	q4, q1, q2
[^>]*> ef22 8154 	vorr	q4, q1, q2
[^>]*> ef22 8154 	vorr	q4, q1, q2
[^>]*> ef22 8154 	vorr	q4, q1, q2
[^>]*> ef22 8154 	vorr	q4, q1, q2
[^>]*> ef22 8154 	vorr	q4, q1, q2
[^>]*> ef22 8154 	vorr	q4, q1, q2
[^>]*> ef22 8158 	vorr	q4, q1, q4
[^>]*> ef22 8158 	vorr	q4, q1, q4
[^>]*> ef22 8158 	vorr	q4, q1, q4
[^>]*> ef22 8158 	vorr	q4, q1, q4
[^>]*> ef22 8158 	vorr	q4, q1, q4
[^>]*> ef22 8158 	vorr	q4, q1, q4
[^>]*> ef22 8158 	vorr	q4, q1, q4
[^>]*> ef22 8158 	vorr	q4, q1, q4
[^>]*> ef22 815e 	vorr	q4, q1, q7
[^>]*> ef22 815e 	vorr	q4, q1, q7
[^>]*> ef22 815e 	vorr	q4, q1, q7
[^>]*> ef22 815e 	vorr	q4, q1, q7
[^>]*> ef22 815e 	vorr	q4, q1, q7
[^>]*> ef22 815e 	vorr	q4, q1, q7
[^>]*> ef22 815e 	vorr	q4, q1, q7
[^>]*> ef22 815e 	vorr	q4, q1, q7
[^>]*> ef24 8150 	vorr	q4, q2, q0
[^>]*> ef24 8150 	vorr	q4, q2, q0
[^>]*> ef24 8150 	vorr	q4, q2, q0
[^>]*> ef24 8150 	vorr	q4, q2, q0
[^>]*> ef24 8150 	vorr	q4, q2, q0
[^>]*> ef24 8150 	vorr	q4, q2, q0
[^>]*> ef24 8150 	vorr	q4, q2, q0
[^>]*> ef24 8150 	vorr	q4, q2, q0
[^>]*> ef24 8152 	vorr	q4, q2, q1
[^>]*> ef24 8152 	vorr	q4, q2, q1
[^>]*> ef24 8152 	vorr	q4, q2, q1
[^>]*> ef24 8152 	vorr	q4, q2, q1
[^>]*> ef24 8152 	vorr	q4, q2, q1
[^>]*> ef24 8152 	vorr	q4, q2, q1
[^>]*> ef24 8152 	vorr	q4, q2, q1
[^>]*> ef24 8152 	vorr	q4, q2, q1
[^>]*> ef24 8154 	vmov	q4, q2
[^>]*> ef24 8154 	vmov	q4, q2
[^>]*> ef24 8154 	vmov	q4, q2
[^>]*> ef24 8154 	vmov	q4, q2
[^>]*> ef24 8154 	vmov	q4, q2
[^>]*> ef24 8154 	vmov	q4, q2
[^>]*> ef24 8154 	vmov	q4, q2
[^>]*> ef24 8154 	vmov	q4, q2
[^>]*> ef24 8158 	vorr	q4, q2, q4
[^>]*> ef24 8158 	vorr	q4, q2, q4
[^>]*> ef24 8158 	vorr	q4, q2, q4
[^>]*> ef24 8158 	vorr	q4, q2, q4
[^>]*> ef24 8158 	vorr	q4, q2, q4
[^>]*> ef24 8158 	vorr	q4, q2, q4
[^>]*> ef24 8158 	vorr	q4, q2, q4
[^>]*> ef24 8158 	vorr	q4, q2, q4
[^>]*> ef24 815e 	vorr	q4, q2, q7
[^>]*> ef24 815e 	vorr	q4, q2, q7
[^>]*> ef24 815e 	vorr	q4, q2, q7
[^>]*> ef24 815e 	vorr	q4, q2, q7
[^>]*> ef24 815e 	vorr	q4, q2, q7
[^>]*> ef24 815e 	vorr	q4, q2, q7
[^>]*> ef24 815e 	vorr	q4, q2, q7
[^>]*> ef24 815e 	vorr	q4, q2, q7
[^>]*> ef28 8150 	vorr	q4, q4, q0
[^>]*> ef28 8150 	vorr	q4, q4, q0
[^>]*> ef28 8150 	vorr	q4, q4, q0
[^>]*> ef28 8150 	vorr	q4, q4, q0
[^>]*> ef28 8150 	vorr	q4, q4, q0
[^>]*> ef28 8150 	vorr	q4, q4, q0
[^>]*> ef28 8150 	vorr	q4, q4, q0
[^>]*> ef28 8150 	vorr	q4, q4, q0
[^>]*> ef28 8152 	vorr	q4, q4, q1
[^>]*> ef28 8152 	vorr	q4, q4, q1
[^>]*> ef28 8152 	vorr	q4, q4, q1
[^>]*> ef28 8152 	vorr	q4, q4, q1
[^>]*> ef28 8152 	vorr	q4, q4, q1
[^>]*> ef28 8152 	vorr	q4, q4, q1
[^>]*> ef28 8152 	vorr	q4, q4, q1
[^>]*> ef28 8152 	vorr	q4, q4, q1
[^>]*> ef28 8154 	vorr	q4, q4, q2
[^>]*> ef28 8154 	vorr	q4, q4, q2
[^>]*> ef28 8154 	vorr	q4, q4, q2
[^>]*> ef28 8154 	vorr	q4, q4, q2
[^>]*> ef28 8154 	vorr	q4, q4, q2
[^>]*> ef28 8154 	vorr	q4, q4, q2
[^>]*> ef28 8154 	vorr	q4, q4, q2
[^>]*> ef28 8154 	vorr	q4, q4, q2
[^>]*> ef28 8158 	vmov	q4, q4
[^>]*> ef28 8158 	vmov	q4, q4
[^>]*> ef28 8158 	vmov	q4, q4
[^>]*> ef28 8158 	vmov	q4, q4
[^>]*> ef28 8158 	vmov	q4, q4
[^>]*> ef28 8158 	vmov	q4, q4
[^>]*> ef28 8158 	vmov	q4, q4
[^>]*> ef28 8158 	vmov	q4, q4
[^>]*> ef28 815e 	vorr	q4, q4, q7
[^>]*> ef28 815e 	vorr	q4, q4, q7
[^>]*> ef28 815e 	vorr	q4, q4, q7
[^>]*> ef28 815e 	vorr	q4, q4, q7
[^>]*> ef28 815e 	vorr	q4, q4, q7
[^>]*> ef28 815e 	vorr	q4, q4, q7
[^>]*> ef28 815e 	vorr	q4, q4, q7
[^>]*> ef28 815e 	vorr	q4, q4, q7
[^>]*> ef2e 8150 	vorr	q4, q7, q0
[^>]*> ef2e 8150 	vorr	q4, q7, q0
[^>]*> ef2e 8150 	vorr	q4, q7, q0
[^>]*> ef2e 8150 	vorr	q4, q7, q0
[^>]*> ef2e 8150 	vorr	q4, q7, q0
[^>]*> ef2e 8150 	vorr	q4, q7, q0
[^>]*> ef2e 8150 	vorr	q4, q7, q0
[^>]*> ef2e 8150 	vorr	q4, q7, q0
[^>]*> ef2e 8152 	vorr	q4, q7, q1
[^>]*> ef2e 8152 	vorr	q4, q7, q1
[^>]*> ef2e 8152 	vorr	q4, q7, q1
[^>]*> ef2e 8152 	vorr	q4, q7, q1
[^>]*> ef2e 8152 	vorr	q4, q7, q1
[^>]*> ef2e 8152 	vorr	q4, q7, q1
[^>]*> ef2e 8152 	vorr	q4, q7, q1
[^>]*> ef2e 8152 	vorr	q4, q7, q1
[^>]*> ef2e 8154 	vorr	q4, q7, q2
[^>]*> ef2e 8154 	vorr	q4, q7, q2
[^>]*> ef2e 8154 	vorr	q4, q7, q2
[^>]*> ef2e 8154 	vorr	q4, q7, q2
[^>]*> ef2e 8154 	vorr	q4, q7, q2
[^>]*> ef2e 8154 	vorr	q4, q7, q2
[^>]*> ef2e 8154 	vorr	q4, q7, q2
[^>]*> ef2e 8154 	vorr	q4, q7, q2
[^>]*> ef2e 8158 	vorr	q4, q7, q4
[^>]*> ef2e 8158 	vorr	q4, q7, q4
[^>]*> ef2e 8158 	vorr	q4, q7, q4
[^>]*> ef2e 8158 	vorr	q4, q7, q4
[^>]*> ef2e 8158 	vorr	q4, q7, q4
[^>]*> ef2e 8158 	vorr	q4, q7, q4
[^>]*> ef2e 8158 	vorr	q4, q7, q4
[^>]*> ef2e 8158 	vorr	q4, q7, q4
[^>]*> ef2e 815e 	vmov	q4, q7
[^>]*> ef2e 815e 	vmov	q4, q7
[^>]*> ef2e 815e 	vmov	q4, q7
[^>]*> ef2e 815e 	vmov	q4, q7
[^>]*> ef2e 815e 	vmov	q4, q7
[^>]*> ef2e 815e 	vmov	q4, q7
[^>]*> ef2e 815e 	vmov	q4, q7
[^>]*> ef2e 815e 	vmov	q4, q7
[^>]*> ef20 e150 	vmov	q7, q0
[^>]*> ef20 e150 	vmov	q7, q0
[^>]*> ef20 e150 	vmov	q7, q0
[^>]*> ef20 e150 	vmov	q7, q0
[^>]*> ef20 e150 	vmov	q7, q0
[^>]*> ef20 e150 	vmov	q7, q0
[^>]*> ef20 e150 	vmov	q7, q0
[^>]*> ef20 e150 	vmov	q7, q0
[^>]*> ef20 e152 	vorr	q7, q0, q1
[^>]*> ef20 e152 	vorr	q7, q0, q1
[^>]*> ef20 e152 	vorr	q7, q0, q1
[^>]*> ef20 e152 	vorr	q7, q0, q1
[^>]*> ef20 e152 	vorr	q7, q0, q1
[^>]*> ef20 e152 	vorr	q7, q0, q1
[^>]*> ef20 e152 	vorr	q7, q0, q1
[^>]*> ef20 e152 	vorr	q7, q0, q1
[^>]*> ef20 e154 	vorr	q7, q0, q2
[^>]*> ef20 e154 	vorr	q7, q0, q2
[^>]*> ef20 e154 	vorr	q7, q0, q2
[^>]*> ef20 e154 	vorr	q7, q0, q2
[^>]*> ef20 e154 	vorr	q7, q0, q2
[^>]*> ef20 e154 	vorr	q7, q0, q2
[^>]*> ef20 e154 	vorr	q7, q0, q2
[^>]*> ef20 e154 	vorr	q7, q0, q2
[^>]*> ef20 e158 	vorr	q7, q0, q4
[^>]*> ef20 e158 	vorr	q7, q0, q4
[^>]*> ef20 e158 	vorr	q7, q0, q4
[^>]*> ef20 e158 	vorr	q7, q0, q4
[^>]*> ef20 e158 	vorr	q7, q0, q4
[^>]*> ef20 e158 	vorr	q7, q0, q4
[^>]*> ef20 e158 	vorr	q7, q0, q4
[^>]*> ef20 e158 	vorr	q7, q0, q4
[^>]*> ef20 e15e 	vorr	q7, q0, q7
[^>]*> ef20 e15e 	vorr	q7, q0, q7
[^>]*> ef20 e15e 	vorr	q7, q0, q7
[^>]*> ef20 e15e 	vorr	q7, q0, q7
[^>]*> ef20 e15e 	vorr	q7, q0, q7
[^>]*> ef20 e15e 	vorr	q7, q0, q7
[^>]*> ef20 e15e 	vorr	q7, q0, q7
[^>]*> ef20 e15e 	vorr	q7, q0, q7
[^>]*> ef22 e150 	vorr	q7, q1, q0
[^>]*> ef22 e150 	vorr	q7, q1, q0
[^>]*> ef22 e150 	vorr	q7, q1, q0
[^>]*> ef22 e150 	vorr	q7, q1, q0
[^>]*> ef22 e150 	vorr	q7, q1, q0
[^>]*> ef22 e150 	vorr	q7, q1, q0
[^>]*> ef22 e150 	vorr	q7, q1, q0
[^>]*> ef22 e150 	vorr	q7, q1, q0
[^>]*> ef22 e152 	vmov	q7, q1
[^>]*> ef22 e152 	vmov	q7, q1
[^>]*> ef22 e152 	vmov	q7, q1
[^>]*> ef22 e152 	vmov	q7, q1
[^>]*> ef22 e152 	vmov	q7, q1
[^>]*> ef22 e152 	vmov	q7, q1
[^>]*> ef22 e152 	vmov	q7, q1
[^>]*> ef22 e152 	vmov	q7, q1
[^>]*> ef22 e154 	vorr	q7, q1, q2
[^>]*> ef22 e154 	vorr	q7, q1, q2
[^>]*> ef22 e154 	vorr	q7, q1, q2
[^>]*> ef22 e154 	vorr	q7, q1, q2
[^>]*> ef22 e154 	vorr	q7, q1, q2
[^>]*> ef22 e154 	vorr	q7, q1, q2
[^>]*> ef22 e154 	vorr	q7, q1, q2
[^>]*> ef22 e154 	vorr	q7, q1, q2
[^>]*> ef22 e158 	vorr	q7, q1, q4
[^>]*> ef22 e158 	vorr	q7, q1, q4
[^>]*> ef22 e158 	vorr	q7, q1, q4
[^>]*> ef22 e158 	vorr	q7, q1, q4
[^>]*> ef22 e158 	vorr	q7, q1, q4
[^>]*> ef22 e158 	vorr	q7, q1, q4
[^>]*> ef22 e158 	vorr	q7, q1, q4
[^>]*> ef22 e158 	vorr	q7, q1, q4
[^>]*> ef22 e15e 	vorr	q7, q1, q7
[^>]*> ef22 e15e 	vorr	q7, q1, q7
[^>]*> ef22 e15e 	vorr	q7, q1, q7
[^>]*> ef22 e15e 	vorr	q7, q1, q7
[^>]*> ef22 e15e 	vorr	q7, q1, q7
[^>]*> ef22 e15e 	vorr	q7, q1, q7
[^>]*> ef22 e15e 	vorr	q7, q1, q7
[^>]*> ef22 e15e 	vorr	q7, q1, q7
[^>]*> ef24 e150 	vorr	q7, q2, q0
[^>]*> ef24 e150 	vorr	q7, q2, q0
[^>]*> ef24 e150 	vorr	q7, q2, q0
[^>]*> ef24 e150 	vorr	q7, q2, q0
[^>]*> ef24 e150 	vorr	q7, q2, q0
[^>]*> ef24 e150 	vorr	q7, q2, q0
[^>]*> ef24 e150 	vorr	q7, q2, q0
[^>]*> ef24 e150 	vorr	q7, q2, q0
[^>]*> ef24 e152 	vorr	q7, q2, q1
[^>]*> ef24 e152 	vorr	q7, q2, q1
[^>]*> ef24 e152 	vorr	q7, q2, q1
[^>]*> ef24 e152 	vorr	q7, q2, q1
[^>]*> ef24 e152 	vorr	q7, q2, q1
[^>]*> ef24 e152 	vorr	q7, q2, q1
[^>]*> ef24 e152 	vorr	q7, q2, q1
[^>]*> ef24 e152 	vorr	q7, q2, q1
[^>]*> ef24 e154 	vmov	q7, q2
[^>]*> ef24 e154 	vmov	q7, q2
[^>]*> ef24 e154 	vmov	q7, q2
[^>]*> ef24 e154 	vmov	q7, q2
[^>]*> ef24 e154 	vmov	q7, q2
[^>]*> ef24 e154 	vmov	q7, q2
[^>]*> ef24 e154 	vmov	q7, q2
[^>]*> ef24 e154 	vmov	q7, q2
[^>]*> ef24 e158 	vorr	q7, q2, q4
[^>]*> ef24 e158 	vorr	q7, q2, q4
[^>]*> ef24 e158 	vorr	q7, q2, q4
[^>]*> ef24 e158 	vorr	q7, q2, q4
[^>]*> ef24 e158 	vorr	q7, q2, q4
[^>]*> ef24 e158 	vorr	q7, q2, q4
[^>]*> ef24 e158 	vorr	q7, q2, q4
[^>]*> ef24 e158 	vorr	q7, q2, q4
[^>]*> ef24 e15e 	vorr	q7, q2, q7
[^>]*> ef24 e15e 	vorr	q7, q2, q7
[^>]*> ef24 e15e 	vorr	q7, q2, q7
[^>]*> ef24 e15e 	vorr	q7, q2, q7
[^>]*> ef24 e15e 	vorr	q7, q2, q7
[^>]*> ef24 e15e 	vorr	q7, q2, q7
[^>]*> ef24 e15e 	vorr	q7, q2, q7
[^>]*> ef24 e15e 	vorr	q7, q2, q7
[^>]*> ef28 e150 	vorr	q7, q4, q0
[^>]*> ef28 e150 	vorr	q7, q4, q0
[^>]*> ef28 e150 	vorr	q7, q4, q0
[^>]*> ef28 e150 	vorr	q7, q4, q0
[^>]*> ef28 e150 	vorr	q7, q4, q0
[^>]*> ef28 e150 	vorr	q7, q4, q0
[^>]*> ef28 e150 	vorr	q7, q4, q0
[^>]*> ef28 e150 	vorr	q7, q4, q0
[^>]*> ef28 e152 	vorr	q7, q4, q1
[^>]*> ef28 e152 	vorr	q7, q4, q1
[^>]*> ef28 e152 	vorr	q7, q4, q1
[^>]*> ef28 e152 	vorr	q7, q4, q1
[^>]*> ef28 e152 	vorr	q7, q4, q1
[^>]*> ef28 e152 	vorr	q7, q4, q1
[^>]*> ef28 e152 	vorr	q7, q4, q1
[^>]*> ef28 e152 	vorr	q7, q4, q1
[^>]*> ef28 e154 	vorr	q7, q4, q2
[^>]*> ef28 e154 	vorr	q7, q4, q2
[^>]*> ef28 e154 	vorr	q7, q4, q2
[^>]*> ef28 e154 	vorr	q7, q4, q2
[^>]*> ef28 e154 	vorr	q7, q4, q2
[^>]*> ef28 e154 	vorr	q7, q4, q2
[^>]*> ef28 e154 	vorr	q7, q4, q2
[^>]*> ef28 e154 	vorr	q7, q4, q2
[^>]*> ef28 e158 	vmov	q7, q4
[^>]*> ef28 e158 	vmov	q7, q4
[^>]*> ef28 e158 	vmov	q7, q4
[^>]*> ef28 e158 	vmov	q7, q4
[^>]*> ef28 e158 	vmov	q7, q4
[^>]*> ef28 e158 	vmov	q7, q4
[^>]*> ef28 e158 	vmov	q7, q4
[^>]*> ef28 e158 	vmov	q7, q4
[^>]*> ef28 e15e 	vorr	q7, q4, q7
[^>]*> ef28 e15e 	vorr	q7, q4, q7
[^>]*> ef28 e15e 	vorr	q7, q4, q7
[^>]*> ef28 e15e 	vorr	q7, q4, q7
[^>]*> ef28 e15e 	vorr	q7, q4, q7
[^>]*> ef28 e15e 	vorr	q7, q4, q7
[^>]*> ef28 e15e 	vorr	q7, q4, q7
[^>]*> ef28 e15e 	vorr	q7, q4, q7
[^>]*> ef2e e150 	vorr	q7, q7, q0
[^>]*> ef2e e150 	vorr	q7, q7, q0
[^>]*> ef2e e150 	vorr	q7, q7, q0
[^>]*> ef2e e150 	vorr	q7, q7, q0
[^>]*> ef2e e150 	vorr	q7, q7, q0
[^>]*> ef2e e150 	vorr	q7, q7, q0
[^>]*> ef2e e150 	vorr	q7, q7, q0
[^>]*> ef2e e150 	vorr	q7, q7, q0
[^>]*> ef2e e152 	vorr	q7, q7, q1
[^>]*> ef2e e152 	vorr	q7, q7, q1
[^>]*> ef2e e152 	vorr	q7, q7, q1
[^>]*> ef2e e152 	vorr	q7, q7, q1
[^>]*> ef2e e152 	vorr	q7, q7, q1
[^>]*> ef2e e152 	vorr	q7, q7, q1
[^>]*> ef2e e152 	vorr	q7, q7, q1
[^>]*> ef2e e152 	vorr	q7, q7, q1
[^>]*> ef2e e154 	vorr	q7, q7, q2
[^>]*> ef2e e154 	vorr	q7, q7, q2
[^>]*> ef2e e154 	vorr	q7, q7, q2
[^>]*> ef2e e154 	vorr	q7, q7, q2
[^>]*> ef2e e154 	vorr	q7, q7, q2
[^>]*> ef2e e154 	vorr	q7, q7, q2
[^>]*> ef2e e154 	vorr	q7, q7, q2
[^>]*> ef2e e154 	vorr	q7, q7, q2
[^>]*> ef2e e158 	vorr	q7, q7, q4
[^>]*> ef2e e158 	vorr	q7, q7, q4
[^>]*> ef2e e158 	vorr	q7, q7, q4
[^>]*> ef2e e158 	vorr	q7, q7, q4
[^>]*> ef2e e158 	vorr	q7, q7, q4
[^>]*> ef2e e158 	vorr	q7, q7, q4
[^>]*> ef2e e158 	vorr	q7, q7, q4
[^>]*> ef2e e158 	vorr	q7, q7, q4
[^>]*> ef2e e15e 	vmov	q7, q7
[^>]*> ef2e e15e 	vmov	q7, q7
[^>]*> ef2e e15e 	vmov	q7, q7
[^>]*> ef2e e15e 	vmov	q7, q7
[^>]*> ef2e e15e 	vmov	q7, q7
[^>]*> ef2e e15e 	vmov	q7, q7
[^>]*> ef2e e15e 	vmov	q7, q7
[^>]*> ef2e e15e 	vmov	q7, q7
[^>]*> ef80 0150 	vorr.i32	q0, #0	@ 0x00000000
[^>]*> ff87 015f 	vorr.i32	q0, #255	@ 0x000000ff
[^>]*> ff87 035f 	vorr.i32	q0, #65280	@ 0x0000ff00
[^>]*> ff87 075f 	vorr.i32	q0, #4278190080	@ 0xff000000
[^>]*> ff87 055f 	vorr.i32	q0, #16711680	@ 0x00ff0000
[^>]*> ef80 0950 	vorr.i16	q0, #0	@ 0x0000
[^>]*> ff87 095f 	vorr.i16	q0, #255	@ 0x00ff
[^>]*> ff87 0b5f 	vorr.i16	q0, #65280	@ 0xff00
[^>]*> fe71 ef4d 	vpstete
[^>]*> ef22 0154 	vorrt	q0, q1, q2
[^>]*> ef22 0154 	vorre	q0, q1, q2
[^>]*> ef80 0150 	vorrt.i32	q0, #0	@ 0x00000000
[^>]*> ff87 0b5f 	vorre.i16	q0, #65280	@ 0xff00
