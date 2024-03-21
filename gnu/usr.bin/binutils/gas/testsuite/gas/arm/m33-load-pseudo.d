# name: Load pseudo-operation for Cortex-M33
# as: -mcpu=cortex-m33
# objdump: -dr --prefix-addresses --show-raw-insn -M force-thumb
# source: load-pseudo.s

.*: +file format .*arm.*


Disassembly of section .text:
[^>]*> f04f 0030 	mov.w	r0, #48	@ 0x30
[^>]*> f04f 40e0 	mov.w	r0, #1879048192	@ 0x70000000
