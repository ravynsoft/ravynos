#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 custom

# Test the custom instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 10d7c032 	custom	0,r11,r2,r3
0+0004 <[^>]*> 10d7fff2 	custom	255,r11,r2,r3
0+0008 <[^>]*> 10c3a5b2 	custom	150,c1,r2,r3
0+000c <[^>]*> 10c28632 	custom	24,c1,c2,r3
0+0010 <[^>]*> 10c20e32 	custom	56,c1,c2,c3
