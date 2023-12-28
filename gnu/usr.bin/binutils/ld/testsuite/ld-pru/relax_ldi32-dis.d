#name: PRU LDI32 relaxation
#source: relax_ldi32.s
#source: relax_ldi32_symbol.s
#as: --mlink-relax
#ld: --relax
#objdump: -dr --prefix-addresses

# Test the LDI32 relaxation
.*: +file format elf32-pru

Disassembly of section .text:
..000000 <[^>]*> ldi	r16.w2, 57005
..000004 <[^>]*> ldi	r16.w0, 48879
..000008 <[^>]*> loop	..00003c <__end_loop>, r22
..00000c <[^>]*> ldi	r16.w2, 57005
..000010 <[^>]*> ldi	r16.w0, 48879
..000014 <[^>]*> ldi	r16, 52938
..000018 <[^>]*> ldi	r0, 52938
..00001c <[^>]*> ldi	r16.w2, 1
..000020 <[^>]*> ldi	r16.w0, 52938
..000024 <[^>]*> ldi	r16.w2, 57004
..000028 <[^>]*> ldi	r16.w0, 48879
..00002c <[^>]*> ldi	r16.w2, 4660
..000030 <[^>]*> ldi	r16.w0, 22136
..000034 <[^>]*> ldi	r16, 22136
..000038 <[^>]*> ldi	r16, 16
..00003c <[^>]*> qba	..000008 <__intermediate>
