#as: -march=rv32i_svinval
#source: svinval.s
#objdump: -d

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <.text>:
[ 	]+0:[ 	]+16b50073[ 	]+sinval.vma[ 	]+a0,a1
[ 	]+4:[ 	]+18000073[ 	]+sfence.w.inval
[ 	]+8:[ 	]+18100073[ 	]+sfence.inval.ir
[ 	]+c:[ 	]+26b50073[ 	]+hinval.vvma[ 	]+a0,a1
[ 	]+10:[ 	]+66b50073[ 	]+hinval.gvma[ 	]+a0,a1
