#as: -march=rv32ih_smstateen -mcsr-check -mpriv-spec=1.12
#source: ssstateen-csr.s
#objdump: -dr

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+10c02573[ 	]+csrr[ 	]+a0,sstateen0
[ 	]+[0-9a-f]+:[ 	]+10d02573[ 	]+csrr[ 	]+a0,sstateen1
[ 	]+[0-9a-f]+:[ 	]+10e02573[ 	]+csrr[ 	]+a0,sstateen2
[ 	]+[0-9a-f]+:[ 	]+10f02573[ 	]+csrr[ 	]+a0,sstateen3
[ 	]+[0-9a-f]+:[ 	]+60c02573[ 	]+csrr[ 	]+a0,hstateen0
[ 	]+[0-9a-f]+:[ 	]+60d02573[ 	]+csrr[ 	]+a0,hstateen1
[ 	]+[0-9a-f]+:[ 	]+60e02573[ 	]+csrr[ 	]+a0,hstateen2
[ 	]+[0-9a-f]+:[ 	]+60f02573[ 	]+csrr[ 	]+a0,hstateen3
[ 	]+[0-9a-f]+:[ 	]+61c02573[ 	]+csrr[ 	]+a0,hstateen0h
[ 	]+[0-9a-f]+:[ 	]+61d02573[ 	]+csrr[ 	]+a0,hstateen1h
[ 	]+[0-9a-f]+:[ 	]+61e02573[ 	]+csrr[ 	]+a0,hstateen2h
[ 	]+[0-9a-f]+:[ 	]+61f02573[ 	]+csrr[ 	]+a0,hstateen3h
