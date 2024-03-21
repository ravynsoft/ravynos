#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 custom
#as: -march=r2
#source: custom.s

# Test the custom instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 00eb18b0 	custom	0,r11,r2,r3
0+0004 <[^>]*> ffeb18b0 	custom	255,r11,r2,r3
0+0008 <[^>]*> 966118b0 	custom	150,c1,r2,r3
0+000c <[^>]*> 184118b0 	custom	24,c1,c2,r3
0+0010 <[^>]*> 380118b0 	custom	56,c1,c2,c3
