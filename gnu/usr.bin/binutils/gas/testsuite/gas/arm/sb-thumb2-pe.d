#objdump: -dr --prefix-addresses --show-raw-insn -M force-thumb
#name: SB instruction (Thumb) with +sb
#source: sb.s
#as: -march=armv8-a+sb -mthumb

# Test SB Instructio

.*: *file format .*arm.*

Disassembly of section .text:
.*> f3bf 8f70 	sb
