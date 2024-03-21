#name: NIOS2 R_NIOS2_PCREL16
#source: pcrel16.s
#source: pcrel16_label.s
#ld:
#objdump: -dr --prefix-addresses

# Test the relative branch relocations.
.*: +file format elf32-littlenios2

Disassembly of section .text:

[0-9a-f]+ <[^>]*> br	[0-9a-f]+ <ext_label>
[0-9a-f]+ <[^>]*> br	[0-9a-f]+ <ext_label\+0x10>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> nop
