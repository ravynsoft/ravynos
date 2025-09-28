# name: Load pseudo-operation for Cortex-M23
# as: -mcpu=cortex-m23
# objdump: -dr --prefix-addresses --show-raw-insn -M force-thumb
# source: load-pseudo.s

.*: +file format .*arm.*


Disassembly of section .text:
[^>]*> f240 0030 	movw	r0, #48	@ 0x30
[^>]*> 4800      	ldr	r0, \[pc, #0\]	@ \(00000008 [^>]*>\)
#...
