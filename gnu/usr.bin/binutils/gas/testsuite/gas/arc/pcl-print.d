#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x00000000 1710 7001\s+ld\s+r1,\[pcl,16\]	;0x00000010
0x00000004 d005\s+ld_s\s+r0,\[pcl,0x14\]	;0x00000018
0x00000006 2630 7fc2 0000 0010\s+ld\s+r2,\[0x10,pcl\]	;0x00000014
