#name: NIOS2 R_NIOS2_IMM5
#source: imm5.s
#source: imm5_symbol.s
#ld:
#objdump: -dr --prefix-addresses

# Test the branch instructions.
.*: +file format elf32-littlenios2

Disassembly of section .text:
[0-9a-f]+ <[^>]*> roli	at,at,31
[0-9a-f]+ <.[^>]*> Address 0x[0-9a-f]+ is out of bounds.

