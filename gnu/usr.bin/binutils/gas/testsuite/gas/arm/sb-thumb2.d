#objdump: -dr --prefix-addresses --show-raw-insn
#name: SB instruction (Thumb) with +sb
#source: sb.s
#as: -march=armv8-a+sb -mthumb
#skip: *-*-pe *-*-wince

# Test SB Instructio

.*: *file format .*arm.*

Disassembly of section .text:
.*> f3bf 8f70 	sb
