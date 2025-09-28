#as: -march=rv32id_zfa
#objdump: -d

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+e2108553[ 	]+fmvh\.x\.d[ 	]+a0,ft1
[ 	]+[0-9a-f]+:[ 	]+b2b500d3[ 	]+fmvp\.d\.x[ 	]+ft1,a0,a1
