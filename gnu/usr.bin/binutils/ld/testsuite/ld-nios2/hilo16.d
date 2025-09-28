#name: NIOS2 R_NIOS2_HI16,LO16,HIADJ16
#source: hilo16.s
#source: hilo16_symbol.s
#ld:
#objdump: -dr --prefix-addresses

# Test the %hi, %lo and %hiadi relocations
.*: +file format elf32-littlenios2

Disassembly of section .text:
[0-9a-f]+ <[^>]*> addi	at,at,-8531
[0-9a-f]+ <[^>]*> addi	at,at,-16657
[0-9a-f]+ <[^>]*> addi	at,at,-8530
