#name: PRU R_PRU_U16
#source: u16.s
#source: u16_symbol.s
#ld:
#objdump: -dr --prefix-addresses

# Test the regulard LDI relocation
.*: +file format elf32-pru

Disassembly of section .text:
[0-9a-f]+ <[^>]*> ldi	r16, 54321
