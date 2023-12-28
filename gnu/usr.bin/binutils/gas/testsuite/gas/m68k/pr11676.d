#objdump: -d --prefix-addresses
#name: PR 11676

# Test disassembling of floating point constants.

.*: +file format .*

Disassembly of section .text:
0+000 <foo>[ 	]fmoves #0e1.23,%fp0
