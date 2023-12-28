#name: PRU R_PRU_LDI32
#source: ldi32.s
#source: ldi32_symbol.s
#ld:
#objdump: -dr --prefix-addresses

# Test the ldi32 relocation
.*: +file format elf32-pru

Disassembly of section .text:
[0-9a-f]+ <[^>]*> ldi	r16.w2, 57005
[0-9a-f]+ <[^>]*> ldi	r16.w0, 48879
