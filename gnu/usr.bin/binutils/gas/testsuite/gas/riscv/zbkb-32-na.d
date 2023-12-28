#as: -march=rv32i_zbkb
#source: zbkb-32.s
#objdump: -d -Mno-aliases

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+60c5d533[ 	]+ror[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+60c59533[ 	]+rol[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+6025d513[ 	]+rori[ 	]+a0,a1,0x2
[ 	]+[0-9a-f]+:[ 	]+40c5f533[ 	]+andn[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+40c5e533[ 	]+orn[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+40c5c533[ 	]+xnor[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+08c5c533[ 	]+pack[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+08c5f533[ 	]+packh[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+68755513[ 	]+brev8[ 	]+a0,a0
[ 	]+[0-9a-f]+:[ 	]+69855513[ 	]+rev8[ 	]+a0,a0
[ 	]+[0-9a-f]+:[ 	]+08f51513[ 	]+zip[ 	]+a0,a0
[ 	]+[0-9a-f]+:[ 	]+08f55513[ 	]+unzip[ 	]+a0,a0
#pass
