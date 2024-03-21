#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 andc
#as: -march=r2

# Test the and macro.

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 7fff211f 	andci	r4,r4,32767
0+0004 <[^>]*> 8000211f 	andci	r4,r4,32768
0+0008 <[^>]*> ffff211f 	andci	r4,r4,65535
0+000c <[^>]*> 0000211f 	andci	r4,r4,0
0+0010 <[^>]*> 7fff213f 	andchi	r4,r4,32767
0+0014 <[^>]*> 8000213f 	andchi	r4,r4,32768
0+0018 <[^>]*> ffff213f 	andchi	r4,r4,65535
0+001c <[^>]*> 0000213f 	andchi	r4,r4,0
