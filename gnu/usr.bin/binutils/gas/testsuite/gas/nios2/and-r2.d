#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 and
#as: -march=r2
#source: and.s

# Test the and macro.

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 38042120 	and	r4,r4,r4
0+0004 <[^>]*> 7fff210c 	andi	r4,r4,32767
0+0008 <[^>]*> 8000210c 	andi	r4,r4,32768
0+000c <[^>]*> ffff210c 	andi	r4,r4,65535
0+0010 <[^>]*> 0000210c 	andi	r4,r4,0
0+0014 <[^>]*> 7fff212c 	andhi	r4,r4,32767
0+0018 <[^>]*> 8000212c 	andhi	r4,r4,32768
0+001c <[^>]*> ffff212c 	andhi	r4,r4,65535
0+0020 <[^>]*> 0000212c 	andhi	r4,r4,0
