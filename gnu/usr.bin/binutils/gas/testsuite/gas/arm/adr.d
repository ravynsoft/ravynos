#as: -mthumb-interwork
#objdump: -dr --prefix-addresses --show-raw-insn
#name: ADR

# Test the `ADR' pseudo-op

.*: +file format .*arm.*

Disassembly of section .text:
0+ <.*> 824ff203 	subhi	pc, pc, #805306368	@ 0x30000000
