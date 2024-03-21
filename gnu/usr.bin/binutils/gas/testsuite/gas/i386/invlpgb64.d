#objdump: -dw
#name: 64-bit INVLPGB insn
#source: invlpgb.s

.*: +file format .*

Disassembly of section \.text:

0+000 <_start>:
[ 	]*[a-f0-9]+:[ 	]+0f 01 fe[ 	]+invlpgb
[0-9a-f]+ <att64>:
[ 	]*[a-f0-9]+:[ 	]+0f 01 fe[ 	]+invlpgb
[0-9a-f]+ <att32>:
[ 	]*[a-f0-9]+:[ 	]+67 0f 01 fe[ 	]+addr32 invlpgb
[0-9a-f]+ <intel64>:
[ 	]*[a-f0-9]+:[ 	]+0f 01 fe[ 	]+invlpgb
[0-9a-f]+ <intel32>:
[ 	]*[a-f0-9]+:[ 	]+67 0f 01 fe[ 	]+addr32 invlpgb
#pass
