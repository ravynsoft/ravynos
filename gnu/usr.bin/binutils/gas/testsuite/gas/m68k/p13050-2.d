#name: p13050-2.d
#objdump: -dr
#as: -march=isab

.*:     file format .*

Disassembly of section .text:

0+ <.*>:
   0:	137c 0005 0001 	moveb #5,%a1@\(1\)
   6:	337c 0005 0001 	movew #5,%a1@\(1\)
