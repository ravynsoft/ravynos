#objdump: -dr

.*:     file format .*

Disassembly of section .text:

[0-9a-f]+ <f_a32>:
.*:	f57ff04c 	dfb

[0-9a-f]+ <f_t32>:
.*:	f3bf 8f4c 	dfb
.*:	bf18      	it	ne
.*:	f3bf 8f4c 	dfbne
.*:	bf08      	it	eq
.*:	f3bf 8f4c 	dfbeq
