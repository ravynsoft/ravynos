#as:
#source: c-zero-imm.s
#objdump: -dr -Mno-aliases

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <.text>:
[ 	]+[0-9a-f]+:[ 	]+4501[ 	]+c\.li[ 	]+a0,0
[ 	]+[0-9a-f]+:[ 	]+4581[ 	]+c\.li[ 	]+a1,0
[ 	]+[0-9a-f]+:[ 	]+8a01[ 	]+c\.andi[ 	]+a2,0
[ 	]+[0-9a-f]+:[ 	]+8a81[ 	]+c\.andi[ 	]+a3,0
[ 	]+[0-9a-f]+:[ 	]+0001[ 	]+c\.addi[ 	]+zero,0
[ 	]+[0-9a-f]+:[ 	]+873a[ 	]+c\.mv[ 	]+a4,a4
[ 	]+[0-9a-f]+:[ 	]+0781[ 	]+c\.addi[ 	]+a5,0
[ 	]+[0-9a-f]+:[ 	]+00051513[ 	]+slli[ 	]+a0,a0,0x0
[ 	]+[0-9a-f]+:[ 	]+0005d593[ 	]+srli[ 	]+a1,a1,0x0
[ 	]+[0-9a-f]+:[ 	]+40065613[ 	]+srai[ 	]+a2,a2,0x0
[ 	]+[0-9a-f]+:[ 	]+0682[ 	]+c\.slli64[ 	]+a3
[ 	]+[0-9a-f]+:[ 	]+8301[ 	]+c\.srli64[ 	]+a4
[ 	]+[0-9a-f]+:[ 	]+8781[ 	]+c\.srai64[ 	]+a5
#...
