#source: shared-lib-nopic-04.s
#as:
#ld: -shared
#objdump: -d

.*:[ 	]+file format .*

#...
Disassembly of section \.text:
#...
.*:[ 	]+[0-9a-f]+[ 	]+jal[ 	]+.* <foo_default@plt>
.*:[ 	]+[0-9a-f]+[ 	]+jal[ 	]+.* <foo_default@plt>

0+[0-9a-f]+ <foo_default>:
#...
