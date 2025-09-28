#as: -march=rv32i_zbkx
#source: zbkx.s
#objdump: -d

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+28c5a533[ 	]+xperm4[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+28c5c533[ 	]+xperm8[ 	]+a0,a1,a2
