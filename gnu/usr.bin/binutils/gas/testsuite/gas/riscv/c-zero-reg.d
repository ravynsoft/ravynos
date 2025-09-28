#as:
#objdump: -dr

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <.text>:
[ 	]+0:[ 	]+4005[ 	]+c.li[ 	]+zero,1
[ 	]+2:[ 	]+6009[ 	]+c.lui[ 	]+zero,0x2
[ 	]+4:[ 	]+000e[ 	]+c.slli[ 	]+zero,0x3
[ 	]+6:[ 	]+8006[ 	]+c.mv[ 	]+zero,ra
[ 	]+8:[ 	]+9006[ 	]+c.add[ 	]+zero,ra
[ 	]+a:[ 	]+00500013[ 	]+li[ 	]+zero,5
[ 	]+e:[ 	]+00006037[ 	]+lui[ 	]+zero,0x6
[ 	]+12:[ 	]+00701013[ 	]+sll[ 	]+zero,zero,0x7
[ 	]+16:[ 	]+00008013[ 	]+mv[ 	]+zero,ra
[ 	]+1a:[ 	]+00100033[ 	]+add[ 	]+zero,zero,ra
#...
