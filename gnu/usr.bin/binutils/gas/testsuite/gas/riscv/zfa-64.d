#as: -march=rv64iq_zfa
#objdump: -d

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+e6108553[ 	]+fmvh\.x\.q[ 	]+a0,ft1
[ 	]+[0-9a-f]+:[ 	]+b6b500d3[ 	]+fmvp\.q\.x[ 	]+ft1,a0,a1
