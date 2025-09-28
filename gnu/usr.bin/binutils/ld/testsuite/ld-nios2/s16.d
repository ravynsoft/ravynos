#name: NIOS2 R_NIOS2_S16
#source: s16.s
#source: s16_symbol.s
#ld:
#objdump: -s

# Test the signed 16-bit relocations.
.*: +file format elf32-littlenios2

Contents of section .text:
 [0-9a-f]+ 04004408 04006008 c4ff5f08 44004808  ..D...`..._.D.H.
 [0-9a-f]+ 44004008                             D.@.            
