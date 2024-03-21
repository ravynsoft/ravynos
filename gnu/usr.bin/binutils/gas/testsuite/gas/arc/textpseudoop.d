#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

[0-9a-f]+ <.text>:
   0:	39da 7010           	custom0.tst	mlx,mlx,r0
   4:	392f 701b           	custom1	mlx,r0
   8:	20aa 0041           	lr	r0,\[aux_test\]
