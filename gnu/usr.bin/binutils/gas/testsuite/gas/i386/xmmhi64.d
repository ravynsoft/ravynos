#objdump: -dwr
#name: high XMM registers in 64-bit mode

.*: +file format .*

Disassembly of section .text:

0+ <xmm>:
[ 	]*[a-f0-9]+:	62 b1 74 08 58 c0[ 	]+vaddps %xmm16,%xmm1,%xmm0
[ 	]*[a-f0-9]+:	62 b1 74 28 58 c0[ 	]+vaddps %ymm16,%ymm1,%ymm0
[ 	]*[a-f0-9]+:	62 b1 74 48 58 c0[ 	]+vaddps %zmm16,%zmm1,%zmm0
#pass
