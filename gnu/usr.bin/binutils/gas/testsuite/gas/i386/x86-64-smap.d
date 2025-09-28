#source: smap.s
#objdump: -dw
#name: 64bit SMAP

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	0f 01 ca             	clac
[ 	]*[a-f0-9]+:	0f 01 cb             	stac
#pass
