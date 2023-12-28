#objdump: -dw
#name: i386 arch 14

.*:     file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[0-9a-f]+:[ 	]+0f 01 fe[ 	]+invlpgb
[ 	]*[0-9a-f]+:[ 	]+0f 01 ff[ 	]+tlbsync
[ 	]*[a-f0-9]+:[ 	]*f2 0f 01 ff[ 	]+pvalidate
[ 	]*[a-f0-9]+:[ 	]*0f 01 ee[ 	]+rdpkru
[ 	]*[a-f0-9]+:[ 	]*0f 01 ef[ 	]+wrpkru
#pass
