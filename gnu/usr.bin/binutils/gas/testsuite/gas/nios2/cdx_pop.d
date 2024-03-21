#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX pop.n
#as: -march=r2

# Test the pop.n instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 0021      	pop.n	{ra},0
0+0002 <[^>]*> 2021      	pop.n	{fp,ra},0
0+0004 <[^>]*> 4021      	pop.n	{r16,ra},0
0+0006 <[^>]*> 6021      	pop.n	{r16,fp,ra},0
0+0008 <[^>]*> 7c21      	pop.n	{r16,r17,r18,r19,r20,r21,r22,r23,fp,ra},0
0+000a <[^>]*> 0021      	pop.n	{ra},0
0+000c <[^>]*> 03e1      	pop.n	{ra},60
0+000e <[^>]*> 7fe1      	pop.n	{r16,r17,r18,r19,r20,r21,r22,r23,fp,ra},60
