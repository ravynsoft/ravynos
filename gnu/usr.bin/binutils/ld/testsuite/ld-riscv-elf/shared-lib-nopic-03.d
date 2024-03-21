#source: shared-lib-nopic-03.s
#as:
#ld: -shared
#objdump: -d

.*:[ 	]+file format .*

Disassembly of section \.text:

#...
.*:[ 	]+[0-9a-f]+[ 	]+jal[ 	]+.* <foo_default>

0+[0-9a-f]+ <foo_default>:
#...
