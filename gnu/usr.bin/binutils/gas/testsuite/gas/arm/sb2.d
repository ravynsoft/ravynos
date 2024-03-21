#objdump: -dr --prefix-addresses --show-raw-insn
#name: SB instruction with +sb
#source: sb.s
#as: -march=armv8-a+sb

# Test SB Instructio

.*: *file format .*arm.*

Disassembly of section .text:
.*> f57ff070 	sb
