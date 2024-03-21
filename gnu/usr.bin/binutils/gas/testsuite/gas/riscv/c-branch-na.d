#as: -march=rv32ic
#source: c-branch.s
#objdump: -drw -Mno-aliases

.*:[ 	]+file format .*


Disassembly of section .text:

0+ <target>:
[ 	]+[0-9a-f]+:[ 	]+c001[ 	]+c\.beqz[ 	]+s0,0 <target>[ 	]+0: R_RISCV_RVC_BRANCH	.*
[ 	]+[0-9a-f]+:[ 	]+dcfd[ 	]+c\.beqz[ 	]+s1,0 <target>[ 	]+2: R_RISCV_RVC_BRANCH	.*
[ 	]+[0-9a-f]+:[ 	]+fc75[ 	]+c\.bnez[ 	]+s0,0 <target>[ 	]+4: R_RISCV_RVC_BRANCH	.*
[ 	]+[0-9a-f]+:[ 	]+fced[ 	]+c\.bnez[ 	]+s1,0 <target>[ 	]+6: R_RISCV_RVC_BRANCH	.*
[ 	]+[0-9a-f]+:[ 	]+bfe5[ 	]+c\.j[ 	]+0 <target>[ 	]+8: R_RISCV_RVC_JUMP	.*
[ 	]+[0-9a-f]+:[ 	]+3fdd[ 	]+c\.jal[ 	]+0 <target>[ 	]+a: R_RISCV_RVC_JUMP	.*
[ 	]+[0-9a-f]+:[ 	]+9302[ 	]+c\.jalr[ 	]+t1
[ 	]+[0-9a-f]+:[ 	]+8382[ 	]+c\.jr[ 	]+t2
[ 	]+[0-9a-f]+:[ 	]+8082[ 	]+c\.jr[ 	]+ra
#...
