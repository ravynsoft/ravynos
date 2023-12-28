#as: -march=generic64+rmpquery
#objdump: -dw
#name: 64-bit RMPQUERY insn
#source: rmpquery.s

.*: +file format .*


Disassembly of section \.text:

0+ <att>:
[ 	]*[a-f0-9]+:[ 	]+f3 0f 01 fd[ 	]+rmpquery[ 	]*
[ 	]*[a-f0-9]+:[ 	]+f3 0f 01 fd[ 	]+rmpquery[ 	]*
[ 	]*[a-f0-9]+:[ 	]+67 f3 0f 01 fd[ 	]+addr32 rmpquery[ 	]*

[0-9a-f]+ <intel>:
[ 	]*[a-f0-9]+:[ 	]+f3 0f 01 fd[ 	]+rmpquery[ 	]*
[ 	]*[a-f0-9]+:[ 	]+f3 0f 01 fd[ 	]+rmpquery[ 	]*
[ 	]*[a-f0-9]+:[ 	]+67 f3 0f 01 fd[ 	]+addr32 rmpquery[ 	]*
#pass
