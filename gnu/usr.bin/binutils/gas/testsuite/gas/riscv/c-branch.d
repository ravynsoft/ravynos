#as: -march=rv64ic
#objdump: -drw

.*:[ 	]+file format .*


Disassembly of section .text:

0+ <target>:
[ 	]+[0-9a-f]+:[ 	]+c001[ 	]+beqz[ 	]+s0,0 <target>[ 	]+0: R_RISCV_RVC_BRANCH	.*
[ 	]+[0-9a-f]+:[ 	]+dcfd[ 	]+beqz[ 	]+s1,0 <target>[ 	]+2: R_RISCV_RVC_BRANCH	.*
[ 	]+[0-9a-f]+:[ 	]+fc75[ 	]+bnez[ 	]+s0,0 <target>[ 	]+4: R_RISCV_RVC_BRANCH	.*
[ 	]+[0-9a-f]+:[ 	]+fced[ 	]+bnez[ 	]+s1,0 <target>[ 	]+6: R_RISCV_RVC_BRANCH	.*
[ 	]+[0-9a-f]+:[ 	]+bfe5[ 	]+j[ 	]+0 <target>[ 	]+8: R_RISCV_RVC_JUMP	.*
[ 	]+[0-9a-f]+:[ 	]+ff7ff0ef[ 	]+jal[ 	]+0 <target>[ 	]+a: R_RISCV_JAL	.*
[ 	]+[0-9a-f]+:[ 	]+9302[ 	]+jalr[ 	]+t1
[ 	]+[0-9a-f]+:[ 	]+8382[ 	]+jr[ 	]+t2
[ 	]+[0-9a-f]+:[ 	]+8082[ 	]+ret
#...
