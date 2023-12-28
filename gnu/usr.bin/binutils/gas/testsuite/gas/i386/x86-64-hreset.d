#as:
#objdump: -dw
#name: x86_64 HRESET insns
#source: hreset.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*f3 0f 3a f0 c0 08[ 	]*hreset \$0x8
#pass
