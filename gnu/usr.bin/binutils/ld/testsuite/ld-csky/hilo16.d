#name: address hi16/lo16 relocations
#source: hilo16.s
#source: hilo16_symbol.s
#ld: -e __start
#objdump: -dr

.*:     file format .*


Disassembly of section .text:

[0-9a-f]+ <__start>:
\s*[0-9a-f]+:	ea21dead 	movih      	r1, 57005
\s*[0-9a-f]+:	ec21beef 	ori      	r1, r1, 48879
