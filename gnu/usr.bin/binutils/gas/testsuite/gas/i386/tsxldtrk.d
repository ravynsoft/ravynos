#as:
#objdump: -dw
#name: TSXLDTRK insns
#source: tsxldtrk.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
 +[a-f0-9]+:	f2 0f 01 e8          	xsusldtrk
 +[a-f0-9]+:	f2 0f 01 e9          	xresldtrk
#pass
