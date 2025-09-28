#objdump: -dw
#name: 32-bit INVLPGB insn

.*: +file format .*

Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]+0f 01 fe[ 	]+invlpgb
[0-9a-f]+ <att32>:
[ 	]*[a-f0-9]+:[ 	]+0f 01 fe[ 	]+invlpgb
[0-9a-f]+ <att16>:
[ 	]*[a-f0-9]+:[ 	]+67 0f 01 fe[ 	]+addr16 invlpgb
[0-9a-f]+ <intel32>:
[ 	]*[a-f0-9]+:[ 	]+0f 01 fe[ 	]+invlpgb
[0-9a-f]+ <intel16>:
[ 	]*[a-f0-9]+:[ 	]+67 0f 01 fe[ 	]+addr16 invlpgb
#pass
