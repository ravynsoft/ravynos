#objdump: -dw
#name: 32-bit SNP insn

.*: +file format .*


Disassembly of section \.text:

0+ <att>:
[ 	]*[a-f0-9]+:[ 	]+f2 0f 01 ff[ 	]+pvalidate
[ 	]*[a-f0-9]+:[ 	]+f2 0f 01 ff[ 	]+pvalidate
[ 	]*[a-f0-9]+:[ 	]+67 f2 0f 01 ff[ 	]+addr16 pvalidate

[0-9a-f]+ <intel>:
[ 	]*[a-f0-9]+:[ 	]+f2 0f 01 ff[ 	]+pvalidate
[ 	]*[a-f0-9]+:[ 	]+f2 0f 01 ff[ 	]+pvalidate
[ 	]*[a-f0-9]+:[ 	]+67 f2 0f 01 ff[ 	]+addr16 pvalidate
#pass
