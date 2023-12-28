# name: Thumb NOP
# objdump: -dr --prefix-addresses --show-raw-insn
#
# Both explicit nop and padding should not use Thumb-2 NOP for the
# default CPU.

.*: +file format .*arm.*

Disassembly of section \.text:
0+000 <[^>]+> 46c0      	nop			@ \(mov r8, r8\)
0+002 <[^>]+> 46c0      	nop			@ \(mov r8, r8\)
