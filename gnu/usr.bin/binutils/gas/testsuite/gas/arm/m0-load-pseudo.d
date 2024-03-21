# name: Load pseudo-operation for Cortex-M0
# as: -mcpu=cortex-m0
# objdump: -dr --prefix-addresses --show-raw-insn -M force-thumb
# source: load-pseudo.s

.*: +file format .*arm.*


Disassembly of section .text:
[^>]*> 4800      	ldr	r0, \[pc, #0\]	@ \(00000004 [^>]*>\)
[^>]*> 4801      	ldr	r0, \[pc, #4\]	@ \(00000008 [^>]*>\)
#...
