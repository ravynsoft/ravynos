#name: Valid v8-a+pan
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*


Disassembly of section .text:
0[0-9a-f]+ <.*> f1100000 	setpan	#0
0[0-9a-f]+ <.*> f1100200 	setpan	#1
0[0-9a-f]+ <.*> b610      	setpan	#0
0[0-9a-f]+ <.*> b618      	setpan	#1