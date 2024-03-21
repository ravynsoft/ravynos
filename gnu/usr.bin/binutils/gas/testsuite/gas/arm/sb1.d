#objdump: -dr --prefix-addresses --show-raw-insn
#name: SB instruction
#source: sb.s
#as: -march=armv8.5-a

# Test SB Instructio

.*: *file format .*arm.*

Disassembly of section .text:
.*> f57ff070 	sb
