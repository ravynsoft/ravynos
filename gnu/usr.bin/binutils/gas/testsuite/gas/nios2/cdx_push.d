#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX push.n
#as: -march=r2

# Test the push.n instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 8021      	push.n	{ra},0
0+0002 <[^>]*> a021      	push.n	{ra,fp},0
0+0004 <[^>]*> c021      	push.n	{ra,r16},0
0+0006 <[^>]*> e021      	push.n	{ra,fp,r16},0
0+0008 <[^>]*> fc21      	push.n	{ra,fp,r23,r22,r21,r20,r19,r18,r17,r16},0
0+000a <[^>]*> 8021      	push.n	{ra},0
0+000c <[^>]*> 83e1      	push.n	{ra},60
0+000e <[^>]*> ffe1      	push.n	{ra,fp,r23,r22,r21,r20,r19,r18,r17,r16},60
