#as: -march=rv64im -defsym zmmul=1 -defsym rv64=1
#source: m-ext.s
#objdump: -d

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+02c58533[ 	]+mul[  	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+02c59533[ 	]+mulh[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+02c5a533[ 	]+mulhsu[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+02c5b533[ 	]+mulhu[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+02c5853b[ 	]+mulw[ 	]+a0,a1,a2
