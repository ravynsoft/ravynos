#name: NIOS2 R_NIOS2_U16
#source: u16.s
#source: u16_symbol.s
#ld:
#objdump: -s

# Test the unsigned 16-bit relocations.
.*: +file format elf32-littlenios2

Contents of section .text:
 [0-9a-f]+ 0c004408 0c004008 ccff7f08 4c004808  ..D...@.....L.H.
 [0-9a-f]+ 4c004008                             L.@.            
